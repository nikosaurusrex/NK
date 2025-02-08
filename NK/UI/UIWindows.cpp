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
global Font UIFont;

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

    UIFont = CreateFont(MakeNativeString("Roboto"), 14.0f);

    RenderTarget->SetTextAntialiasMode(D2D1_TEXT_ANTIALIAS_MODE_DEFAULT);

    RenderTarget->CreateSolidColorBrush(D2D1::ColorF(0x252829, 1.0f), &Brushes[UI_PRIMARY_BG]);
    RenderTarget->CreateSolidColorBrush(D2D1::ColorF(0xfafafa, 1.0f), &Brushes[UI_PRIMARY_FG]);
    RenderTarget->CreateSolidColorBrush(D2D1::ColorF(0x202223, 1.0f), &Brushes[UI_SECONDARY_BG]);
    RenderTarget->CreateSolidColorBrush(D2D1::ColorF(0x797a7b, 1.0f), &Brushes[UI_SECONDARY_FG]);
    RenderTarget->CreateSolidColorBrush(D2D1::ColorF(0x181a1c, 1.0f), &Brushes[UI_BORDER]);
    RenderTarget->CreateSolidColorBrush(D2D1::ColorF(0x363b4f, 1.0f), &Brushes[UI_SELECTION]);
}

void DeinitUIRendering() {
    for (int i = 0; i < UI_COLORS; ++i) {
        Brushes[i]->Release();
    }

    DestroyFont(UIFont);

    RenderTarget->Release();
    DWriteFactory->Release();
    D2DFactory->Release();
}

Font CreateFont(NativeString Name, float Size) {
    Font Result = {};

    CheckFail(DWriteFactory->CreateTextFormat(
        Name,
        0,
        DWRITE_FONT_WEIGHT_NORMAL,
        DWRITE_FONT_STYLE_NORMAL,
        DWRITE_FONT_STRETCH_NORMAL,
        Size,
        L"en-us",
        &Result.Format
    ));

    return Result;
}

void DestroyFont(Font Handle) {
    if (Handle.Format) {
        Handle.Format->Release();
    }
}

void BeginUIRendering(Window *Win) {
    if (Win->Resized) {
        D2D1_SIZE_U WinSize = {};
        WinSize.width = Win->Size.X;
        WinSize.height = Win->Size.Y;

        CheckFail(RenderTarget->Resize(WinSize));
    }

    RenderTarget->BeginDraw();
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

void DrawBorderedRectangle(Vec2 Position, Vec2 Size, u32 Color) {
    D2D1_RECT_F Rect;
    Rect.left = Position.X;
    Rect.top = Position.Y;
    Rect.right = Position.X + Size.X;
    Rect.bottom = Position.Y + Size.Y;

    RenderTarget->FillRectangle(Rect, Brushes[Color]);
    RenderTarget->DrawRectangle(Rect, Brushes[UI_BORDER], 2);
}

void DrawText(String Text, Vec2 Position, u32 Color) {
    wchar_t WideBuffer[1024];
    Assert(Text.Length < 1024);
    int Length16 = MultiByteToWideChar(CP_UTF8, 0, (char *) Text.Pointer, Text.Length, WideBuffer, sizeof(WideBuffer) / sizeof(wchar_t));

    IDWriteTextLayout *Layout = 0;
    float MaxWidth  = 10000.0f;
    float MaxHeight = 10000.0f;

    CheckFail(DWriteFactory->CreateTextLayout(
        WideBuffer,
        Length16,
        UIFont.Format,
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
    Assert(Text.Length < 1024);
    int Length16 = MultiByteToWideChar(CP_UTF8, 0, (char *) Text.Pointer, Text.Length, WideBuffer, sizeof(WideBuffer) / sizeof(wchar_t));

    IDWriteTextLayout *Layout = 0;
    CheckFail(DWriteFactory->CreateTextLayout(
        WideBuffer,
        Length16,
        UIFont.Format,
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
    Assert(Text.Length < 1024);
    int Length16 = MultiByteToWideChar(CP_UTF8, 0, (char *) Text.Pointer, Text.Length, WideBuffer, sizeof(WideBuffer) / sizeof(wchar_t));

    IDWriteTextLayout *Layout = 0;
    float MaxWidth  = 10000.0f;
    CheckFail(DWriteFactory->CreateTextLayout(
        WideBuffer,
        Length16,
        UIFont.Format,
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
