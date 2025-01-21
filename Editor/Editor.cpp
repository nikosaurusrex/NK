#define NK_IMPLEMENTATION
#include "NK/PlatformLayer.h"
#include "NK/WindowLayer.h"
#include "NK/MathLayer.h"
#include "NK/DataStructuresLayer.h"

#include "Buffer.h"
#include "Editor.h"
#include "Buffer.cpp"
#include "Keymaps.cpp"

#include <dwrite.h>
#include <d2d1.h>

#define CheckFail(Call) do { \
    HRESULT HResult = Call; \
    if (FAILED(HResult)) { \
         Print("DWrite Call Failed: 0x%x\n", HResult); NK_TRAP(); Exit(1); \
    } \
} while (0);

enum {
    MAX_LINE_LENGTH = 256
};

global IDWriteFactory *DWriteFactory;
global ID2D1Factory *D2DFactory;
global ID2D1HwndRenderTarget *RenderTarget;
global IDWriteTextFormat *TextFormat;
global ID2D1SolidColorBrush *FillBrush;

global TextEditor Editor;
global float LineHeight;
global float Padding = 2;

void UpdateScroll(Pane *Target) {
    GapBuffer *Buffer = &Target->Buffer;

    u64 CursorLine = 0;

    for (u64 i = 0; i < Target->Cursor; ++i) {
        if ((*Buffer)[i] == '\n') {
            CursorLine++;
        }
    }

    float ContentHeight = Target->Height - Padding * 2;
    u64 LinesVisible = IFloor(ContentHeight / LineHeight);

    if (CursorLine < Target->Scroll) {
        Target->Scroll = CursorLine;
    } else if (CursorLine >= Target->Scroll + LinesVisible) {
        Target->Scroll = CursorLine - LinesVisible + 1;
    }

    u64 RenderIndex = 0;
    u64 CurrentLine = 0;
    while (CurrentLine < Target->Scroll && RenderIndex < Buffer->Length) {
        RenderIndex = CursorNextLineBegin(Buffer, RenderIndex);
        CurrentLine++;
    }
    Target->ScrollRenderIndex = RenderIndex;
}

void DrawPane(Pane *ToDraw) {
    GapBuffer *Buffer = &ToDraw->Buffer;

    char Line8[MAX_LINE_LENGTH];
    wchar_t Line16[MAX_LINE_LENGTH];

    D2D1_RECT_F Bounds;
    Bounds.left = ToDraw->X;
    Bounds.top = ToDraw->Y;
    Bounds.right = ToDraw->X + ToDraw->Width;
    Bounds.bottom = ToDraw->Y + ToDraw->Height;
    RenderTarget->DrawRectangle(Bounds, FillBrush, 2, 0);

    // Padding
    Bounds.left += Padding;
    Bounds.right -= Padding;
    Bounds.top += Padding;
    Bounds.bottom += Padding;

    if (ToDraw == Editor.ActivePane) {
        UpdateScroll(ToDraw);
    }

    u64 RenderCursor = ToDraw->ScrollRenderIndex;
    while (RenderCursor < Buffer->Length && Bounds.top < Bounds.bottom) {
        int Line8Length = GetLineFromGapBuffer(Buffer, RenderCursor, Line8, sizeof(Line8));
        int Line16Length = MultiByteToWideChar(CP_UTF8, 0, Line8, Line8Length, Line16, sizeof(Line16) / sizeof(wchar_t));

        u64 PreviousRenderCursor = RenderCursor;
        RenderCursor += Line8Length + 1;

        IDWriteTextLayout *TextLayout;
        CheckFail(DWriteFactory->CreateTextLayout(
            Line16,
            Line16Length,
            TextFormat,
            Bounds.right - Bounds.left,
            Bounds.bottom - Bounds.top,
            &TextLayout
        ));

        D2D1_POINT_2F Location;
        Location.x = Bounds.left;
        Location.y = Bounds.top;
        RenderTarget->DrawTextLayout(Location, TextLayout, FillBrush, D2D1_DRAW_TEXT_OPTIONS_CLIP);

        if (PreviousRenderCursor <= ToDraw->Cursor && ToDraw->Cursor <= RenderCursor) {
            DWRITE_HIT_TEST_METRICS CaretMetrics;
            float CaretX;
            float CaretY;
            TextLayout->HitTestTextPosition(ToDraw->Cursor - PreviousRenderCursor, 0, &CaretX, &CaretY, &CaretMetrics);

            D2D1_RECT_F CaretBounds;
            CaretBounds.left = Bounds.left + CaretMetrics.left;
            CaretBounds.top = Bounds.top + CaretMetrics.top;
            CaretBounds.right = CaretBounds.left;
            if (Editor.Mode != ED_INSERT) {
                CaretBounds.right += CaretMetrics.width;
            }
            CaretBounds.bottom = CaretBounds.top + CaretMetrics.height;
            RenderTarget->DrawRectangle(CaretBounds, FillBrush);
        }

        DWRITE_LINE_METRICS LineMetrics;
        u32 LineCount = 0;
        TextLayout->GetLineMetrics(&LineMetrics, 1, &LineCount);

        Bounds.top += LineMetrics.height * LineCount;

        TextLayout->Release();
    }
}

void OnKeyPress(u32 Codepoint) {
    u32 ModBits = 0;

    if (IsKeyDown(KEY_CONTROL)) {
        ModBits |= CTRL;
    }
    if (IsKeyDown(KEY_SHIFT)) {
        ModBits |= SHIFT;
    }
    if (IsKeyDown(KEY_MENU)) {
        ModBits |= ALT;
    }

    if (!(ModBits & CTRL) || Codepoint > KEY_Z) {
        if (KEY_SPACE <= Codepoint && Codepoint <= KEY_Z) {
            return;
        }
    }

    switch (Codepoint) {
        case KEY_CONTROL:
        case KEY_MENU:
        case KEY_SHIFT:
            return;
        default:
            break;
    }

    u32 KeyCombination = Codepoint | ModBits;

    Editor.InputEvent.Char = char(Codepoint & 0xFF);
    Editor.InputEvent.KeyCombination = KeyCombination;

    EKeymap *Keymap = Editor.Keymaps + Editor.Mode;
    EShortcut *Shortcut = KeymapGetShortcut(Keymap, KeyCombination);
    Shortcut->Function(&Editor);
}

void OnCharInput(char Char) {
    u32 ModBits = 0;

    if (IsKeyDown(KEY_CONTROL)) {
        ModBits |= CTRL;
    }
    if (IsKeyDown(KEY_SHIFT)) {
        ModBits |= SHIFT;
    }
    if (IsKeyDown(KEY_MENU)) {
        ModBits |= ALT;
    }

    if (Char < 32 || Char > 127) {
        return;
    }

    u32 KeyCombination = Char;
    if ('a' <= KeyCombination && KeyCombination <= 'z') {
        KeyCombination -= 32;
    }

    KeyCombination |= ModBits;

    Editor.InputEvent.Char = Char;
    Editor.InputEvent.KeyCombination = KeyCombination;

    EKeymap *Keymap = Editor.Keymaps + Editor.Mode;
    EShortcut *Shortcut = KeymapGetShortcut(Keymap, KeyCombination);
    Shortcut->Function(&Editor);
}

void AdjustTabWidthAndSetLineHeight() {
    IDWriteTextLayout *TextLayout;
    CheckFail(DWriteFactory->CreateTextLayout(
        L" ",
        1,
        TextFormat,
        128.0f,
        128.0f,
        &TextLayout
    ));

    DWRITE_TEXT_METRICS CharMetrics;
    TextLayout->GetMetrics(&CharMetrics);

    TextFormat->SetIncrementalTabStop(TAB_SIZE * CharMetrics.widthIncludingTrailingWhitespace);

    DWRITE_LINE_METRICS LineMetrics;
    u32 LineCount = 0;
    TextLayout->GetLineMetrics(&LineMetrics, 1, &LineCount);

    LineHeight = LineMetrics.height;

    TextLayout->Release();
}

void NKMain() {
    Window MainWindow = {};
    MainWindow.Title = "Editor";
    MainWindow.Size.X = 1280;
    MainWindow.Size.Y = 720;
    MainWindow.KeyCallback = OnKeyPress;
    MainWindow.CharCallback = OnCharInput;

    if (!InitWindow(&MainWindow)) {
        Print("Failed to initialize window!\n");
        Exit(1);
    }

    CheckFail(DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), (IUnknown **) &DWriteFactory));

    D2D1_FACTORY_OPTIONS D2DFactoryOptions = {};
    CheckFail(D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, __uuidof(ID2D1Factory), &D2DFactoryOptions, (void **)&D2DFactory));

    D2D1_SIZE_U WinSize = {};
    WinSize.width = MainWindow.Size.X;
    WinSize.height = MainWindow.Size.Y;
    CheckFail(D2DFactory->CreateHwndRenderTarget(
        D2D1::RenderTargetProperties(),
        D2D1::HwndRenderTargetProperties(MainWindow.Handle, WinSize, D2D1_PRESENT_OPTIONS_NONE),
        &RenderTarget)
    );

    CheckFail(DWriteFactory->CreateTextFormat(
        L"Roboto",
        0,
        DWRITE_FONT_WEIGHT_NORMAL,
        DWRITE_FONT_STYLE_NORMAL,
        DWRITE_FONT_STRETCH_NORMAL,
        16.0f,
        L"en-us",
        &TextFormat
    ));

    AdjustTabWidthAndSetLineHeight();

    RenderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Black), &FillBrush);

    Editor.Mode = ED_NORMAL;

    Pane InitialPane = CreatePane(KiloBytes(16), 0, 0, MainWindow.Size.X, MainWindow.Size.Y);

    Editor.Panes[0] = InitialPane;
    Editor.ActivePane = &Editor.Panes[0];

    LoadSourceFile(&Editor.ActivePane->Buffer, "Editor/Editor.cpp");

    CreateDefaultKeymaps(&Editor);

    double CPUTimeAverage = 0.0;
    while (MainWindow.Running) {
        u64 CPUTimeBegin = GetTimeNowUs();

        UpdateWindow(&MainWindow);

        if (MainWindow.Resized) {
            RenderTarget->Release();
            D2D1_SIZE_U WinSize = {};
            WinSize.width = MainWindow.Size.X;
            WinSize.height = MainWindow.Size.Y;

            CheckFail(D2DFactory->CreateHwndRenderTarget(
                D2D1::RenderTargetProperties(),
                D2D1::HwndRenderTargetProperties(MainWindow.Handle, WinSize, D2D1_PRESENT_OPTIONS_NONE),
                &RenderTarget
            ));

            Editor.ActivePane->Width = WinSize.width;
            Editor.ActivePane->Height = WinSize.height;
        }

        RenderTarget->BeginDraw();
        RenderTarget->Clear(D2D1::ColorF(D2D1::ColorF::White));

        DrawPane(Editor.ActivePane);

        RenderTarget->EndDraw();

        u64 CPUTimeEnd = GetTimeNowUs();
        double CPUTimeDeltaMS = double(CPUTimeEnd - CPUTimeBegin) / 1000.0;
        
        CPUTimeAverage = CPUTimeAverage * 0.95 + CPUTimeDeltaMS * 0.05;

        char PerfTitle[128];
        stbsp_snprintf(PerfTitle, sizeof(PerfTitle), "cpu: %.2fmss", CPUTimeAverage);
        SetWindowTitle(&MainWindow, PerfTitle);
    }

    DestroyPane(InitialPane);

    TextFormat->Release();
    FillBrush->Release();
    RenderTarget->Release();
    DWriteFactory->Release();
    D2DFactory->Release();

    DestroyWindow(&MainWindow);
}
