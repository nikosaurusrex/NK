#define NK_IMPLEMENTATION
#include "NK/PlatformLayer.h"
#include "NK/WindowLayer.h"
#include "NK/MathLayer.h"
#include "NK/DataStructuresLayer.h"
#include "NK/UILayer.h"

#include "Buffer.h"
#include "Pane.h"
#include "Editor.h"
#include "Buffer.cpp"
#include "Pane.cpp"
#include "Keymaps.cpp"

enum {
    MAX_LINE_LENGTH = 256
};

global TextEditor Editor;
global EditorConfig Config;

void LoadDefaultConfig() {
    Config.EditorPadding = 2;
    Config.StatusBarPadding = 2;
    Config.FontSize = 16;
    Config.TabSize = 4;
}

void UpdateConfigCalculations() {
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

    Config.SpaceWidth = CharMetrics.widthIncludingTrailingWhitespace;
    Config.TabWidth = Config.TabSize * CharMetrics.widthIncludingTrailingWhitespace;
    TextFormat->SetIncrementalTabStop(Config.TabWidth);

    DWRITE_LINE_METRICS LineMetrics;
    u32 LineCount = 0;
    TextLayout->GetLineMetrics(&LineMetrics, 1, &LineCount);

    Config.LineHeight = LineMetrics.height;

    Config.StatusBarHeight = LineMetrics.height + Config.StatusBarPadding * 2;

    TextLayout->Release();
}

void UpdateScroll(Pane *Target) {
    GapBuffer *Buffer = &Target->Buffer;

    u64 CursorLine = 0;

    for (u64 i = 0; i < Target->Cursor; ++i) {
        if ((*Buffer)[i] == '\n') {
            CursorLine++;
        }
    }

    float ContentHeight = Target->Height - Config.EditorPadding * 2;
    u64 LinesVisible = IFloor(ContentHeight / Config.LineHeight);

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

void DrawStatusBar(Pane *P) {
    Vec2 Position = Vec2(P->X + Config.StatusBarPadding, P->Y + P->Height - Config.StatusBarHeight);
    Vec2 Size = Vec2(P->Width, Config.StatusBarHeight);

    DrawTextCenteredVertically(P->File, Position, Size.Y, UI_BLACK);
}

// For now, all the D2D1 stuff is defined and setup in UIWindows.cpp
void DrawTheActualEditor(Pane *ToDraw) {
    GapBuffer *Buffer = &ToDraw->Buffer;

    char Line8[MAX_LINE_LENGTH];
    wchar_t Line16[MAX_LINE_LENGTH];

    D2D1_RECT_F Bounds;
    Bounds.left = ToDraw->X;
    Bounds.top = ToDraw->Y;
    Bounds.right = ToDraw->X + ToDraw->Width;
    Bounds.bottom = ToDraw->Y + ToDraw->Height - Config.StatusBarHeight;

    RenderTarget->PushAxisAlignedClip(Bounds, D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
    RenderTarget->DrawRectangle(Bounds, Brushes[UI_BLACK], 2, 0);

    // Padding
    float Padding = Config.EditorPadding;
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
        RenderCursor += Line8Length;

        IDWriteTextLayout *TextLayout;
        CheckFail(DWriteFactory->CreateTextLayout(
            Line16,
            Line16Length,
            TextFormat,
            Bounds.right - Bounds.left,
            Bounds.bottom - Bounds.top,
            &TextLayout
        ));

        // Draw selection
        if (Editor.Mode == ED_VISUAL) {
            u32 VisualStart;
            u32 VisualEnd;
            if (ToDraw->Cursor > ToDraw->VisualCursor) {
                VisualStart = ToDraw->VisualCursor;
                VisualEnd = ToDraw->Cursor;
            } else {
                VisualStart = ToDraw->Cursor;
                VisualEnd = ToDraw->VisualCursor;
            }

            u32 SelectionStart = Max(PreviousRenderCursor, VisualStart);
            u32 SelectionEnd = Min(RenderCursor, VisualEnd);

            if (SelectionStart < SelectionEnd) {
                u32 SelectionStartOffset = SelectionStart - PreviousRenderCursor;
                u32 SelectionEndOffset = SelectionEnd - PreviousRenderCursor;

                DWRITE_HIT_TEST_METRICS SelectionStartMetrics, SelectionEndMetrics;
                float SelectionStartX, SelectionEndX;
                float SelectionY;
                TextLayout->HitTestTextPosition(SelectionStartOffset, 0, &SelectionStartX, &SelectionY, &SelectionStartMetrics);
                TextLayout->HitTestTextPosition(SelectionEndOffset, 0, &SelectionEndX, &SelectionY, &SelectionEndMetrics);

                D2D1_RECT_F SelectionRect;
                SelectionRect.left = Bounds.left + SelectionStartMetrics.left;
                SelectionRect.top = Bounds.top + SelectionStartMetrics.top;
                SelectionRect.right = Bounds.left + SelectionEndMetrics.left;
                SelectionRect.bottom = SelectionRect.top + SelectionStartMetrics.height;

                RenderTarget->FillRectangle(&SelectionRect, Brushes[UI_LIGHT_GRAY]);
            }
        }

        // Draw Cursor
        if (PreviousRenderCursor <= ToDraw->Cursor && ToDraw->Cursor <= RenderCursor) {
            u32 CursorStart = ToDraw->Cursor - PreviousRenderCursor;
            
            DWRITE_HIT_TEST_METRICS CaretMetrics;
            float CaretX;
            float CaretY;
            TextLayout->HitTestTextPosition(CursorStart, 0, &CaretX, &CaretY, &CaretMetrics);

            D2D1_RECT_F CaretBounds;
            CaretBounds.left = Bounds.left + CaretMetrics.left;
            CaretBounds.top = Bounds.top + CaretMetrics.top;
            CaretBounds.right = CaretBounds.left;
            CaretBounds.bottom = CaretBounds.top + CaretMetrics.height;

            if (Editor.Mode != ED_INSERT) {
                CaretBounds.right += CaretMetrics.width;
                if (CaretMetrics.width == 0) {
                    CaretBounds.right += Config.SpaceWidth;
                }

                RenderTarget->FillRectangle(CaretBounds, Brushes[UI_BLACK]);

                DWRITE_TEXT_RANGE TextRange = {CursorStart, 1};
                TextLayout->SetDrawingEffect(Brushes[UI_WHITE], TextRange);
            } else{
                RenderTarget->DrawRectangle(CaretBounds, Brushes[UI_BLACK]);
            }
        }

        D2D1_POINT_2F Location;
        Location.x = Bounds.left;
        Location.y = Bounds.top;
        RenderTarget->DrawTextLayout(Location, TextLayout, Brushes[UI_BLACK], D2D1_DRAW_TEXT_OPTIONS_CLIP);

        DWRITE_LINE_METRICS LineMetrics;
        u32 LineCount = 0;
        TextLayout->GetLineMetrics(&LineMetrics, 1, &LineCount);

        Bounds.top += LineMetrics.height * LineCount;

        RenderCursor++; // skip \n - has to be down after rendering cursor

        TextLayout->Release();
    }

    RenderTarget->PopAxisAlignedClip();
}

void DrawPane(Pane *ToDraw) {
    DrawTheActualEditor(ToDraw);
    DrawStatusBar(ToDraw);
}

void OnKeyPress(u32 Codepoint) {
    u32 ModBits = 0;

    if (IsKeyDown(KEY_CONTROL)) {
        ModBits |= KEYMAP_CTRL;
    }
    if (IsKeyDown(KEY_SHIFT)) {
        ModBits |= KEYMAP_SHIFT;
    }
    if (IsKeyDown(KEY_MENU)) {
        ModBits |= KEYMAP_ALT;
    }

    if (!(ModBits & KEYMAP_CTRL) || Codepoint > KEY_Z) {
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

    HandleShortcut(&Editor, char(Codepoint & 0xFF), ModBits);
}

void OnCharInput(char Char) {
    u32 ModBits = 0;

    if (IsKeyDown(KEY_CONTROL)) {
        ModBits |= KEYMAP_CTRL;
    }
    if (IsKeyDown(KEY_SHIFT)) {
        ModBits |= KEYMAP_SHIFT;
    }
    if (IsKeyDown(KEY_MENU)) {
        ModBits |= KEYMAP_ALT;
    }

    if (Char < 32 || Char > 127) {
        return;
    }

    HandleShortcut(&Editor, Char, ModBits);
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
    // MaximizeWindow(&MainWindow);

    Arena KeymapArena = CreateArena(MegaBytes(1));
    CreateKeymaps(&KeymapArena);

    LoadDefaultConfig();
    InitUI(&MainWindow, Config.FontSize);
    UpdateConfigCalculations(); // requires font to be set up

    Pane InitialPane = CreatePane(KiloBytes(16), 0, 0, MainWindow.Size.X, MainWindow.Size.Y);

    Editor.Mode = ED_NORMAL;
    Editor.Panes[0] = InitialPane;
    Editor.ActivePane = &Editor.Panes[0];
    Editor.Directory = "D:\\dev\\NK";

    PaneLoadFile(Editor.ActivePane, "Editor/Editor.cpp");

    double CPUTimeAverage = 0.0;
    while (MainWindow.Running) {
        u64 CPUTimeBegin = GetTimeNowUs();

        UpdateWindow(&MainWindow);

        if (MainWindow.Resized) {
            Editor.ActivePane->Width = MainWindow.Size.X;
            Editor.ActivePane->Height = MainWindow.Size.Y;
        }

        BeginUIFrame(&MainWindow);

        DrawPane(Editor.ActivePane);

        EndUIFrame();

        u64 CPUTimeEnd = GetTimeNowUs();
        double CPUTimeDeltaMS = double(CPUTimeEnd - CPUTimeBegin) / 1000.0;
        
        CPUTimeAverage = CPUTimeAverage * 0.95 + CPUTimeDeltaMS * 0.05;

        char PerfTitle[128];
        stbsp_snprintf(PerfTitle, sizeof(PerfTitle), "cpu: %.2fmss", CPUTimeAverage);
        SetWindowTitle(&MainWindow, PerfTitle);
    }

    DestroyUI();

    FreeArena(&KeymapArena);
    DestroyPane(InitialPane);
    DestroyWindow(&MainWindow);
}
