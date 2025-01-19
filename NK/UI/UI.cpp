#include <dwrite.h>
#include <dxgi.h>
#include <d2d1.h>

enum {
    FONT_ATLAS_WIDTH = 512,
    FONT_ATLAS_HEIGHT = 512,

    UI_RENDER_BUFFER_SIZE = 16368 // multiple of UIRenderCommand
};

enum {
    FONT_FIRST_CHAR = 32,
    FONT_LAST_CHAR = 126,
    FONT_NUM_CHARS = (FONT_LAST_CHAR - FONT_FIRST_CHAR) + 1
};

enum {
    UI_RECTANGLE = 0,
    UI_TEXT
};

struct UIRenderCommand {
    Vec2 Position;
    Vec2 Size;
    u32 Color;
    float Rounding;
    u32 Border; // 8bit thickness, 24bit rgb 
    u32 Type;
    Vec2 UVPosition;
    Vec2 UVSize;
};
#define UI_RENDER_GROUP_MAX_ELEMENTS (UI_RENDER_BUFFER_SIZE/sizeof(UIRenderCommand))

struct UIRenderGroup {
    UIRenderCommand *Commands;
    UIRenderGroup *Next;
    u32 CommandCount;
};

enum {
    UI_INTERACTION_TOGGLE = 0,
    UI_INTERACTION_PRESS = 1
};

enum {
    UI_ACTIVE = 0x1,
    UI_HOVERED = 0x2
};

enum {
    UI_DRAW_ACTIVE = 0x1,
    UI_DRAW_HOVERED = 0x2,
    UI_DRAW_BORDER = 0x4,
    UI_DRAW_ROUNDED = 0x8
};

struct UIBox {
    Vec2 Position;
    Vec2 Size;
};

struct UIWindowConstants {
    Vec2 Size;
    Vec2 Placeholder;
};

struct UIInteraction {
    union {
        b8 *Bool;
    };
    u32 Type;
};

struct GlyphAtlasEntry {
    Vec2 UVPosition;
    Vec2 UVSize;
    Vec2 GlyphSize;
    float Advance;
};

struct UIState {
    Arena RenderGroupArena;

    UIRenderGroup *FirstRenderGroup;
    UIRenderGroup *CurrentRenderGroup;

    GBuffer WindowConstantsBuffer;
    GBuffer RCmdBuffer;

    GShader Shader;

    GTexture FontAtlasTexture;

    float FontSize;
    float GlyphScale;

    GlyphAtlasEntry GlyphEntries[FONT_NUM_CHARS];
};

global UIState UI;

void InitDirectWrite() {
	IDWriteFactory *DWriteFactory = 0;
    CheckFail(DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), (IUnknown **) &DWriteFactory));

    ID2D1Factory *D2Factory = 0;
    D2D1_FACTORY_OPTIONS Options = {};
    CheckFail(D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, __uuidof(ID2D1Factory), &Options, (void **)&D2Factory));

    UI.FontAtlasTexture = CreateTexture(
        FONT_ATLAS_WIDTH,
        FONT_ATLAS_HEIGHT,
        TEXTURE_FORMAT_BGRA_UNORM,
        TEXTURE_RENDER_TARGET
    );

    IDXGISurface *DxgiSurface;
    CheckFail(UI.FontAtlasTexture.Handle->QueryInterface(IID_PPV_ARGS(&DxgiSurface)));

    D2D1_RENDER_TARGET_PROPERTIES Props =
        D2D1::RenderTargetProperties(
                                     D2D1_RENDER_TARGET_TYPE_DEFAULT,
                                     D2D1::PixelFormat(
                                                       DXGI_FORMAT_B8G8R8A8_UNORM,
                                                       D2D1_ALPHA_MODE_PREMULTIPLIED
                                                      ),
                                     0, 0
                                    );
    ID2D1RenderTarget *RenderTarget;
    CheckFail(D2Factory->CreateDxgiSurfaceRenderTarget(
        DxgiSurface,
        &Props,
        &RenderTarget
    ));

    ID2D1SolidColorBrush *FillBrush = 0;
    RenderTarget->CreateSolidColorBrush(D2D1::ColorF(1.0f, 1.0f, 1.0f, 1.0f), &FillBrush);

    UI.FontSize = 12.0f;

    const wchar_t *FontName = L"Roboto";
    IDWriteTextFormat *TextFormat;
    CheckFail(DWriteFactory->CreateTextFormat(
        FontName,
        0,
        DWRITE_FONT_WEIGHT_REGULAR,
        DWRITE_FONT_STYLE_NORMAL,
        DWRITE_FONT_STRETCH_NORMAL,
        UI.FontSize,
        L"en-us",
        &TextFormat
    ));

    IDWriteFontCollection *FontCollection = 0;
    CheckFail(DWriteFactory->GetSystemFontCollection(&FontCollection));

    UINT32 FontIndex;
    BOOL Exists;
    CheckFail(FontCollection->FindFamilyName(FontName, &FontIndex, &Exists));
    if (!Exists) {
        Print("Font not found.\n");
        Exit(1);
    }

    IDWriteFontFamily *FontFamily = 0;
    CheckFail(FontCollection->GetFontFamily(FontIndex, &FontFamily));

    IDWriteFont *Font = 0;
    CheckFail(FontFamily->GetFont(0, &Font));

    IDWriteFontFace *FontFace = 0;
    CheckFail(Font->CreateFontFace(&FontFace));

    u16 GlyphIndices[FONT_NUM_CHARS];
    DWRITE_GLYPH_METRICS GlyphMetrics[FONT_NUM_CHARS]; 
    for (u32 Char = FONT_FIRST_CHAR; Char <= FONT_LAST_CHAR; ++Char) {
        u16 GlyphIndex = 0;
        CheckFail(FontFace->GetGlyphIndices(&Char, 1, &GlyphIndex));
        GlyphIndices[Char - FONT_FIRST_CHAR] = GlyphIndex;
    }
    
    CheckFail(FontFace->GetDesignGlyphMetrics(
        GlyphIndices,
        FONT_NUM_CHARS,
        GlyphMetrics
    ));

    DWRITE_FONT_METRICS FontMetrics;
    FontFace->GetMetrics(&FontMetrics);
    float DesignUnitsPerEm = (float)FontMetrics.designUnitsPerEm;
    float FontScale = UI.FontSize / DesignUnitsPerEm;
    
    RenderTarget->BeginDraw();
    RenderTarget->Clear();

    float PenX = 0;
    float PenY = 0;
    float RowHeight = 0;
    const float Padding = 2.0f;
    for (int i = 0; i < FONT_NUM_CHARS; ++i) {
        wchar_t Char = FONT_FIRST_CHAR + i;
        IDWriteTextLayout *TextLayout;
        CheckFail(DWriteFactory->CreateTextLayout(
            &Char, 1, TextFormat, FONT_ATLAS_WIDTH, FONT_ATLAS_HEIGHT, &TextLayout));
        
        DWRITE_TEXT_METRICS Metrics;
        TextLayout->GetMetrics(&Metrics);

        DWRITE_GLYPH_METRICS GMetrics = GlyphMetrics[i];

        float GlyphWidth = Metrics.widthIncludingTrailingWhitespace;
        float GlyphHeight = UI.FontSize;

        if (PenX + GlyphWidth + Padding > FONT_ATLAS_WIDTH) {
            PenX = 0;
            PenY += RowHeight + Padding;
            RowHeight = 0;
        }
        
        if (GlyphHeight > RowHeight) {
            RowHeight = GlyphHeight;
        }

        D2D1_RECT_F LayoutRect;
        LayoutRect.left = PenX;
        LayoutRect.right = PenX + GlyphWidth;
        LayoutRect.top = PenY;
        LayoutRect.bottom = PenY + GlyphHeight;

        RenderTarget->DrawText(
            &Char,
            1,
            TextFormat,
            &LayoutRect,
            FillBrush,
            D2D1_DRAW_TEXT_OPTIONS_ENABLE_COLOR_FONT,
            DWRITE_MEASURING_MODE_NATURAL
        );

        GlyphAtlasEntry *GlyphEntry = UI.GlyphEntries + i;
        GlyphEntry->UVPosition = Vec2(
            PenX / FONT_ATLAS_WIDTH, PenY / FONT_ATLAS_HEIGHT
        );
        GlyphEntry->UVSize = Vec2(
            GlyphWidth / FONT_ATLAS_WIDTH, Metrics.height / FONT_ATLAS_HEIGHT
        );
        GlyphEntry->GlyphSize = Vec2(
            GlyphWidth, Metrics.height
        );
        GlyphEntry->Advance = GMetrics.advanceWidth * FontScale;

        PenX += GlyphWidth + Padding;
    }

    RenderTarget->EndDraw();
}

void InitUI(Window *Win) {
    UI.RenderGroupArena = CreateArena(Megabytes(16));

    UI.Shader = LoadShader(MakeNativeString("NK/UI/Shader.hlsl"), MakeNativeString("NK/UI/Shader.hlsl"));

    UI.WindowConstantsBuffer = CreateBuffer(
        BUFFER_CONSTANT,
        sizeof(UIWindowConstants),
        0, 0, 0, 0
    );
    UI.RCmdBuffer = CreateBuffer(
        BUFFER_STRUCTURED,
        UI_RENDER_BUFFER_SIZE,
        sizeof(UIRenderCommand),
        0,
        UI_RENDER_GROUP_MAX_ELEMENTS,
        0
    );

    InitDirectWrite();
}

void DestroyUI() {
    ReleaseBuffer(UI.WindowConstantsBuffer);
    ReleaseBuffer(UI.RCmdBuffer);
    
    ReleaseShader(UI.Shader);

    ReleaseTexture(UI.FontAtlasTexture);

    FreeArena(&UI.RenderGroupArena);
}

void BeginUIFrame(Window *Win) {
    UIWindowConstants WindowConstants = {};
    WindowConstants.Size.X = Win->Size.X;
    WindowConstants.Size.Y = Win->Size.Y;

    UpdateBuffer(UI.WindowConstantsBuffer, &WindowConstants, sizeof(UIWindowConstants));

    ResetArena(&UI.RenderGroupArena);
    UI.FirstRenderGroup = 0;
    UI.CurrentRenderGroup = 0;
}

void EndUIFrame() {
    BindBuffer(UI.RCmdBuffer, SHADER_VERTEX);
    BindBuffer(UI.WindowConstantsBuffer, SHADER_VERTEX);

    BindTexture(UI.FontAtlasTexture, SHADER_PIXEL);

    BindShader(UI.Shader);

    UIRenderGroup *RenderGroup = UI.FirstRenderGroup;
    while (RenderGroup) {
        u64 Size = RenderGroup->CommandCount * sizeof(UIRenderCommand);

        UpdateBuffer(UI.RCmdBuffer, RenderGroup->Commands, Size);

        DrawInstanced(RenderGroup->CommandCount, 6);

        RenderGroup = RenderGroup->Next;
    }

    SwapChain->Present(1, 0);
}

UIRenderGroup *PushRenderGroup() {
    UIRenderGroup *Result = PushStruct(&UI.RenderGroupArena, UIRenderGroup);
    Result->Commands = PushArray(&UI.RenderGroupArena, UIRenderCommand, UI_RENDER_GROUP_MAX_ELEMENTS); 
    Result->Next = 0;
    Result->CommandCount = 0;

    return Result;
}

void PushRenderCommand(UIRenderCommand Command) {
    UIRenderGroup *RenderGroup = UI.CurrentRenderGroup;

    if (!RenderGroup) {
        RenderGroup = PushRenderGroup();

        UI.FirstRenderGroup = RenderGroup;
        UI.CurrentRenderGroup = RenderGroup;
    }

    if (RenderGroup->CommandCount >= UI_RENDER_GROUP_MAX_ELEMENTS) {
        RenderGroup = PushRenderGroup();

        UI.CurrentRenderGroup->Next = RenderGroup;
        UI.CurrentRenderGroup = RenderGroup;
    }

    RenderGroup->Commands[RenderGroup->CommandCount] = Command;
    RenderGroup->CommandCount++;
}

void PushRectangle(UIBox Box, u32 Color, float Rounding, u32 Border) {
    UIRenderCommand Command = {};
    Command.Position = Box.Position;
    Command.Size = Box.Size;
    Command.Color = Color;
    Command.Rounding = Rounding;
    Command.Border = Border;
    Command.Type = UI_RECTANGLE;
    Command.UVPosition = Vec2();
    Command.UVSize = Vec2();

    PushRenderCommand(Command);
}

void PushText(String Text, Vec2 Position, u32 Color) {
    float PosX = Position.X;
    float PosY = Position.Y;

    for (u64 i = 0; i < Text.Length; ++i) {
        char Character = Text[i];

        int GlyphIndex = Character - FONT_FIRST_CHAR;
        GlyphAtlasEntry GlyphEntry = UI.GlyphEntries[GlyphIndex];

        float GlyphOriginX = PosX;
        float GlyphOriginY = PosY;

        UIRenderCommand Command = {};
        Command.Position = Vec2(GlyphOriginX, GlyphOriginY);
        Command.Size = GlyphEntry.GlyphSize;
        Command.Color = Color;
        Command.Rounding = 0;
        Command.Border = 0;
        Command.Type = UI_TEXT;
        Command.UVPosition = GlyphEntry.UVPosition;
        Command.UVSize = GlyphEntry.UVSize;

        PushRenderCommand(Command);

        PosX += GlyphEntry.Advance;
    }
}

void PushTextCentered(String Text, UIBox Box, u32 Color) {
    Vec2 Position = Box.Position;
    Vec2 Size = Box.Size;

    float TextWidth = 0;
    float TextHeight = UI.FontSize;

    for (u64 i = 0; i < Text.Length; ++i) {
        char Character = Text[i];

        int GlyphIndex = Character - FONT_FIRST_CHAR;
        if (GlyphIndex < 0 || GlyphIndex >= FONT_NUM_CHARS) continue;

        GlyphAtlasEntry GlyphEntry = UI.GlyphEntries[GlyphIndex];
        TextWidth += GlyphEntry.Advance;
    }

    // Center the text in the box
    Vec2 TextPos = {};
    TextPos.X = Position.X + (Size.X - TextWidth) * 0.5f;
    TextPos.Y = Position.Y + (Size.Y - TextHeight) * 0.5f;

    PushText(Text, TextPos, Color);
}

void DrawBox(UIBox Box, u32 InteractionFlags, u32 DrawFlags) {
    b32 Active = (InteractionFlags & UI_ACTIVE) && (DrawFlags & UI_DRAW_ACTIVE);
    b32 Hovered = (InteractionFlags & UI_HOVERED) && (DrawFlags & UI_DRAW_HOVERED);

    u32 Border = 0;
    if (DrawFlags & UI_DRAW_BORDER) {
        if (Hovered) {
            Border = 0x02999999;
        } else {
            Border = 0x01666666;
        }
    }

    float Rounding = 0;
    if (DrawFlags & UI_DRAW_ROUNDED) {
        Rounding = 10;
    }

    u32 Color = 0x161616;
    if (Active) {
        if (Hovered) {
            Color = 0x353535; 
        } else {
            Color = 0x292929; 
        }
    } else if (Hovered) {
        Color = 0x222222;
    }

    PushRectangle(Box, Color, Rounding, Border);
}

b32 IsHovered(UIBox Box) {
    Int2 MousePos = GetMousePosition();
    return MousePos.X > Box.Position.X &&
            MousePos.Y > Box.Position.Y &&
            MousePos.X < Box.Position.X + Box.Size.X &&
            MousePos.Y < Box.Position.Y + Box.Size.Y;
}

u32 HandleUIInteraction(UIBox Box, UIInteraction Interaction) {
    u32 Result = 0;

    b32 Hovered = IsHovered(Box);
    b32 Clicked = WasButtonPressed(MOUSE_BUTTON_LEFT);

    if (Hovered) {
        Result |= UI_HOVERED;
    }

    switch (Interaction.Type) {
    case UI_INTERACTION_TOGGLE: {
        b8 Value = *Interaction.Bool;
        if (Hovered && Clicked) {
            *Interaction.Bool = !Value;
        }
        if (Value) {
            Result |= UI_ACTIVE;
        }
    } break;
    case UI_INTERACTION_PRESS: {
        if (Hovered && Clicked) {
            *Interaction.Bool = 1;
            Result |= UI_ACTIVE;
        }
    } break;
    }

    return Result;
}

b8 UIButton(String Text, Vec2 Position, Vec2 Size) {
    b8 Result = 0;

    UIBox Box = {Position, Size};
    UIInteraction Interaction = {{&Result}, UI_INTERACTION_PRESS}; 

    u32 InteractionFlags = HandleUIInteraction(Box, Interaction);
    u32 RenderFlags = UI_DRAW_HOVERED | UI_DRAW_BORDER;

    DrawBox(Box, InteractionFlags, RenderFlags);
    PushTextCentered(Text, Box, 0xFFFFFF);

    return Result;
}

b8 UIToggleButton(b8 *Value, String Text, Vec2 Position, Vec2 Size) {
    b8 Result = 0;

    UIBox Box = {Position, Size};
    UIInteraction Interaction = {{Value}, UI_INTERACTION_TOGGLE}; 

    u32 InteractionFlags = HandleUIInteraction(Box, Interaction);
    u32 RenderFlags = UI_DRAW_ACTIVE | UI_DRAW_HOVERED | UI_DRAW_BORDER;

    DrawBox(Box, InteractionFlags, RenderFlags);
    PushTextCentered(Text, Box, 0xFFFFFF);

    Result = *Value;
    return Result;
}

