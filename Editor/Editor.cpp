#define NK_IMPLEMENTATION
#include "NK/PlatformLayer.h"
#include "NK/WindowLayer.h"
#include "NK/MathLayer.h"
#include "NK/DataStructuresLayer.h"

#include "Buffer.h"
#include "Buffer.cpp"

#include <dwrite.h>
#include <d2d1.h>

#define CheckFail(Call) do {         \
    HRESULT HResult = Call;           \
    if (FAILED(HResult)) {            \
         Print("DWrite Call Failed: 0x%x\n", HResult); NK_TRAP(); Exit(1); \
    }                                 \
} while (0);

enum {
    MAX_LINE_LENGTH = 256
};

global IDWriteFactory *DWriteFactory;
global ID2D1Factory *D2DFactory;
global ID2D1HwndRenderTarget *RenderTarget;
global IDWriteTextFormat *TextFormat;
global ID2D1SolidColorBrush *FillBrush;

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

    u64 RenderCursor = 0;
    while (RenderCursor < Buffer->Length) {
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
            CaretBounds.bottom = CaretBounds.top + CaretMetrics.height;
            RenderTarget->DrawRectangle(CaretBounds, FillBrush);
        }

        DWRITE_LINE_METRICS LineMetrics;
        u32 LineCount = 0;
        TextLayout->GetLineMetrics(&LineMetrics, 1, &LineCount);

        Bounds.top += LineMetrics.height;

        TextLayout->Release();
    }
}

void NKMain() {
    Window MainWindow = {};
    MainWindow.Title = "Editor";
    MainWindow.Size.X = 1280;
    MainWindow.Size.Y = 720;

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
    CheckFail(D2DFactory->CreateHwndRenderTarget(D2D1::RenderTargetProperties(), D2D1::HwndRenderTargetProperties(MainWindow.Handle, WinSize), &RenderTarget));

    CheckFail(DWriteFactory->CreateTextFormat(
        L"Roboto",
        0,
        DWRITE_FONT_WEIGHT_REGULAR,
        DWRITE_FONT_STYLE_NORMAL,
        DWRITE_FONT_STRETCH_NORMAL,
        16.0f,
        L"en-us",
        &TextFormat
    ));

    RenderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Black), &FillBrush);

    Pane InitialPane = CreatePane(KiloBytes(16), 256, 60, 20, 20, 600, 400);
    u64 Cursor = 0;
    Cursor = InsertString(&InitialPane.Buffer, "Lmao, no way this actually works!", 0);
    Cursor = InsertLine(&InitialPane.Buffer, Cursor, 0);
    Cursor = InsertString(&InitialPane.Buffer, "There is actually no way", Cursor);
    InitialPane.Cursor = 6;

    while (MainWindow.Running) {
        UpdateWindow(&MainWindow);

        if (MainWindow.Resized) {
            RenderTarget->Release();
            D2D1_SIZE_U WinSize = {};
            WinSize.width = MainWindow.Size.X;
            WinSize.height = MainWindow.Size.Y;
            CheckFail(D2DFactory->CreateHwndRenderTarget(D2D1::RenderTargetProperties(), D2D1::HwndRenderTargetProperties(MainWindow.Handle, WinSize), &RenderTarget));
        }

        RenderTarget->BeginDraw();
        RenderTarget->Clear(D2D1::ColorF(D2D1::ColorF::White));

        DrawPane(&InitialPane);
        RenderTarget->EndDraw();
    }

    DestroyPane(InitialPane);

    TextFormat->Release();
    FillBrush->Release();
    RenderTarget->Release();
    DWriteFactory->Release();
    D2DFactory->Release();

    DestroyWindow(&MainWindow);
}
