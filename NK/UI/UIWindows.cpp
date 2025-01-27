#include <dwrite.h>
#include <d2d1.h>

#define CheckFail(Call) do { \
    HRESULT HResult = Call; \
    if (FAILED(HResult)) { \
         Print("DWrite Call Failed: 0x%x\n", HResult); NK_TRAP(); Exit(1); \
    } \
} while (0);

global IDWriteFactory *DWriteFactory;
global ID2D1Factory *D2DFactory;
global ID2D1HwndRenderTarget *RenderTarget;
global IDWriteTextFormat *TextFormat;

global ID2D1SolidColorBrush *Brushes[UI_COLORS];

void InitUIRendering(Window *Win, float FontSize) {
    CheckFail(DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), (IUnknown **) &DWriteFactory));

    D2D1_FACTORY_OPTIONS D2DFactoryOptions = {};
    CheckFail(D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, __uuidof(ID2D1Factory), &D2DFactoryOptions, (void **)&D2DFactory));

    D2D1_SIZE_U WinSize = {};
    WinSize.width = Win->Size.X;
    WinSize.height = Win->Size.Y;
    CheckFail(D2DFactory->CreateHwndRenderTarget(
        D2D1::RenderTargetProperties(),
        D2D1::HwndRenderTargetProperties(Win->Handle, WinSize, D2D1_PRESENT_OPTIONS_NONE),
        &RenderTarget)
    );

    CheckFail(DWriteFactory->CreateTextFormat(
        L"Roboto",
        0,
        DWRITE_FONT_WEIGHT_NORMAL,
        DWRITE_FONT_STYLE_NORMAL,
        DWRITE_FONT_STRETCH_NORMAL,
        FontSize,
        L"en-us",
        &TextFormat
    ));

    RenderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Black), &Brushes[UI_BLACK]);
    RenderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::White), &Brushes[UI_WHITE]);
    RenderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::LightGray), &Brushes[UI_LIGHT_GRAY]);

    RenderTarget->CreateSolidColorBrush(D2D1::ColorF(0x161616, 1.0f), &Brushes[UI_BUTTON_BG]);
    RenderTarget->CreateSolidColorBrush(D2D1::ColorF(0x222222, 1.0f), &Brushes[UI_BUTTON_HOVERED_BG]);
    RenderTarget->CreateSolidColorBrush(D2D1::ColorF(0x292929, 1.0f), &Brushes[UI_BUTTON_ACTIVE_BG]);
    RenderTarget->CreateSolidColorBrush(D2D1::ColorF(0x353535, 1.0f), &Brushes[UI_BUTTON_ACTIVE_HOVERED_BG]);
}

void DeinitUIRendering() {
    for (int i = 0; i < UI_COLORS; ++i) {
        Brushes[i]->Release();
    }

    RenderTarget->Release();
    DWriteFactory->Release();
    D2DFactory->Release();
}

void BeginUIRendering(Window *Win) {
    RenderTarget->BeginDraw();
    RenderTarget->Clear(D2D1::ColorF(D2D1::ColorF::White));

    if (Win->Resized) {
        D2D1_SIZE_U WinSize = {};
        WinSize.width = Win->Size.X;
        WinSize.height = Win->Size.Y;

        RenderTarget->Resize(WinSize);
    }
}

void EndUIRendering() {
    RenderTarget->EndDraw();
}

void DrawRectangle(Vec2 Position, Vec2 Size, u32 Color) {
    D2D1_RECT_F Rect;
    Rect.left = Position.X;
    Rect.top = Position.Y;
    Rect.right = Position.X + Size.X;
    Rect.bottom = Position.Y + Size.Y;
    RenderTarget->FillRectangle(Rect, Brushes[Color]);
}

void DrawText(String Text, Vec2 Position, u32 Color) {
    wchar_t WideBuffer[1024];
    int Length16 = MultiByteToWideChar(CP_UTF8, 0, (char *) Text.Pointer, Text.Length, WideBuffer, sizeof(WideBuffer) / sizeof(wchar_t));

    IDWriteTextLayout *Layout = 0;
    float MaxWidth  = 10000.0f;
    float MaxHeight = 10000.0f;

    CheckFail(DWriteFactory->CreateTextLayout(
        WideBuffer,
        Length16,
        TextFormat,
        MaxWidth,
        MaxHeight,
        &Layout
    ));

    Layout->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);
    Layout->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_NEAR);

    RenderTarget->DrawTextLayout(
        D2D1::Point2F(Position.X, Position.Y),
        Layout,
        Brushes[Color]
    );

    Layout->Release();
}

void DrawTextCentered(String Text, Vec2 Position, Vec2 Size, u32 Color) {
    wchar_t WideBuffer[1024];
    int Length16 = MultiByteToWideChar(CP_UTF8, 0, (char *) Text.Pointer, Text.Length, WideBuffer, sizeof(WideBuffer) / sizeof(wchar_t));

    IDWriteTextLayout *Layout = 0;
    CheckFail(DWriteFactory->CreateTextLayout(
        WideBuffer,
        Length16,
        TextFormat,
        Size.X,
        Size.Y,
        &Layout
    ));

    Layout->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
    Layout->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);

    RenderTarget->DrawTextLayout(
        D2D1::Point2F(Position.X, Position.Y),
        Layout,
        Brushes[Color]
    );

    Layout->Release();
}

void DrawTextCenteredVertically(String Text, Vec2 Position, float Height, u32 Color) {
    wchar_t WideBuffer[1024];
    int Length16 = MultiByteToWideChar(CP_UTF8, 0, (char *) Text.Pointer, Text.Length, WideBuffer, sizeof(WideBuffer) / sizeof(wchar_t));

    IDWriteTextLayout *Layout = 0;
    float MaxWidth  = 10000.0f;
    CheckFail(DWriteFactory->CreateTextLayout(
        WideBuffer,
        Length16,
        TextFormat,
        MaxWidth,
        Height,
        &Layout
    ));

    Layout->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);

    RenderTarget->DrawTextLayout(
        D2D1::Point2F(Position.X, Position.Y),
        Layout,
        Brushes[Color]
    );

    Layout->Release();
}
