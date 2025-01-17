IGNORE_WARNINGS_BEGIN
#define STB_IMAGE_WRITE_IMPLEMENTATION
// #include "../ThirdParty/stb_image_write.h"
IGNORE_WARNINGS_END

#include <dwrite.h>
#include <dxgi.h>
#include <d3d11.h>
#include <d3dcompiler.h>

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

struct UIState {
    Arena RenderGroupArena;

    UIRenderGroup *FirstRenderGroup;
    UIRenderGroup *CurrentRenderGroup;

    ID3D11Buffer *WindowConstantsBuffer;
    ID3D11Buffer *RCmdBuffer;
    ID3D11ShaderResourceView *RCmdBufferView;
    ID3D11VertexShader *VertexShader;
    ID3D11PixelShader *PixelShader;

    u16 GlyphIndices[FONT_NUM_CHARS];
    float GlyphAdvances[FONT_NUM_CHARS];
    DWRITE_GLYPH_METRICS GlyphMetrics[FONT_NUM_CHARS];
};

global UIState UI;
global ID3D11Device *Device;
global ID3D11DeviceContext *DeviceContext;
global IDXGISwapChain *SwapChain;
global ID3D11RenderTargetView *RenderTargetView;

#define DWriteCall(Call) do {         \
    HRESULT HResult = Call;           \
    if (FAILED(HResult)) {            \
         Print("DWrite Call Failed: 0x%x\n", HResult); NK_TRAP(); Exit(1); \
    }                                 \
} while (0);

void InitD3D11(Window *Win) {
    DXGI_SWAP_CHAIN_DESC SwapChainDesc = {};
    SwapChainDesc.BufferCount = 2;
    SwapChainDesc.BufferDesc.Width = Win->Size.X;
    SwapChainDesc.BufferDesc.Height = Win->Size.Y;
    SwapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    SwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    SwapChainDesc.OutputWindow = Win->Handle;
    SwapChainDesc.SampleDesc.Count = 1;
    SwapChainDesc.Windowed = TRUE;
    SwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;

    // TODO: change flags based on build
    u32 Flags = D3D11_CREATE_DEVICE_SINGLETHREADED | D3D11_CREATE_DEVICE_DEBUG;
    D3D_FEATURE_LEVEL Features[] = {
        D3D_FEATURE_LEVEL_11_1,
    };

    DWriteCall(D3D11CreateDeviceAndSwapChain(
        0,
        D3D_DRIVER_TYPE_HARDWARE,
        0, Flags, Features, ArrayCount(Features),
        D3D11_SDK_VERSION,
        &SwapChainDesc,
        &SwapChain,
        &Device,
        0,
        &DeviceContext
    ));

    ID3D11Texture2D* BackBuffer = 0;
    DWriteCall(SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&BackBuffer));

    Device->CreateRenderTargetView(BackBuffer, 0, &RenderTargetView);
    BackBuffer->Release();

    DeviceContext->OMSetRenderTargets(1, &RenderTargetView, 0);

    D3D11_VIEWPORT Viewport = {};
    Viewport.Width = float(Win->Size.X);
    Viewport.Height = float(Win->Size.Y);
    Viewport.MinDepth = 0.0f;
    Viewport.MaxDepth = 1.0f;
    DeviceContext->RSSetViewports(1, &Viewport);
    
    DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
}

void CompileShader(LPCWSTR FilePath, LPCSTR EntryPoint, LPCSTR Target, ID3DBlob **Blob) {
    ID3DBlob *Error = 0;
    HRESULT HResult = D3DCompileFromFile(FilePath, 0, 0, EntryPoint, Target, 0, 0, Blob, &Error);

    if (FAILED(HResult)) {
        if (Error) {
            PrintLiteral((char *) Error->GetBufferPointer());
            Error->Release();
        }
    }
    if (Error) {
        Error->Release();
    }
}

void LoadShaders() {
    ID3DBlob *VertexBlob = 0;
    ID3DBlob *PixelBlob = 0;

    CompileShader(L"NK/UI/Shader.hlsl", "VSMain", "vs_5_0", &VertexBlob);
    CompileShader(L"NK/UI/Shader.hlsl", "PSMain", "ps_5_0", &PixelBlob);

    Device->CreateVertexShader(VertexBlob->GetBufferPointer(), VertexBlob->GetBufferSize(), 0, &UI.VertexShader);
    Device->CreatePixelShader(PixelBlob->GetBufferPointer(), PixelBlob->GetBufferSize(), 0, &UI.PixelShader);

    D3D11_BUFFER_DESC RCmdBufferDesc = {};
    RCmdBufferDesc.ByteWidth = UI_RENDER_BUFFER_SIZE;
    RCmdBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    RCmdBufferDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    RCmdBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    RCmdBufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
    RCmdBufferDesc.StructureByteStride = sizeof(UIRenderCommand);

    DWriteCall(Device->CreateBuffer(&RCmdBufferDesc, 0, &UI.RCmdBuffer));

    D3D11_SHADER_RESOURCE_VIEW_DESC RCmdBufferViewDesc = {};
    RCmdBufferViewDesc.Format = DXGI_FORMAT_UNKNOWN;
    RCmdBufferViewDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
    RCmdBufferViewDesc.Buffer.FirstElement = 0;
    RCmdBufferViewDesc.Buffer.NumElements = UI_RENDER_GROUP_MAX_ELEMENTS;

    DWriteCall(Device->CreateShaderResourceView(UI.RCmdBuffer, &RCmdBufferViewDesc, &UI.RCmdBufferView));

    D3D11_BUFFER_DESC WindowConstantsBufferDesc = {};
    WindowConstantsBufferDesc.ByteWidth = sizeof(UIWindowConstants);
    WindowConstantsBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    WindowConstantsBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    WindowConstantsBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    DWriteCall(Device->CreateBuffer(&WindowConstantsBufferDesc, 0, &UI.WindowConstantsBuffer));

    VertexBlob->Release();
    PixelBlob->Release();
}

void InitDirectWrite() {
	IDWriteFactory *DWriteFactory = 0;
    DWriteCall(DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), (IUnknown **) &DWriteFactory));

    IDWriteFontCollection *FontCollection = 0;
    DWriteCall(DWriteFactory->GetSystemFontCollection(&FontCollection));

    UINT32 FontIndex;
    BOOL Exists;
    DWriteCall(FontCollection->FindFamilyName(L"Roboto", &FontIndex, &Exists));
    if (!Exists) {
        Print("Font not found.\n");
        Exit(1);
    }

    IDWriteFontFamily *FontFamily = 0;
    DWriteCall(FontCollection->GetFontFamily(FontIndex, &FontFamily));

    IDWriteFont *Font = 0;
    DWriteCall(FontFamily->GetFont(0, &Font));

    IDWriteFontFace *FontFace = 0;
    DWriteCall(Font->CreateFontFace(&FontFace));

    for (u32 Char = FONT_FIRST_CHAR; Char <= FONT_LAST_CHAR; ++Char) {
        u16 GlyphIndex = 0;
        DWriteCall(FontFace->GetGlyphIndices(&Char, 1, &GlyphIndex));
        UI.GlyphIndices[Char - FONT_FIRST_CHAR] = GlyphIndex;
    }
    
    DWriteCall(FontFace->GetDesignGlyphMetrics(
        UI.GlyphIndices,
        FONT_NUM_CHARS,
        UI.GlyphMetrics
    ));

    u8 *AtlasData = (u8 *) HeapAlloc(FONT_ATLAS_WIDTH * 3 * FONT_ATLAS_HEIGHT);

    int PenX = 0;
    int PenY = 0;
    int RowHeight = 0;
    for (int i = 0; i < FONT_NUM_CHARS; ++i) {
        DWRITE_GLYPH_RUN GlyphRun = {};
        GlyphRun.fontFace = FontFace;
        GlyphRun.fontEmSize = 32.0f;
        GlyphRun.glyphCount = 1;
        GlyphRun.glyphIndices = &UI.GlyphIndices[i];
        GlyphRun.glyphAdvances = &UI.GlyphAdvances[i];

        IDWriteGlyphRunAnalysis *GlyphRunAnalysis = 0;
        DWriteCall(DWriteFactory->CreateGlyphRunAnalysis(
            &GlyphRun, 1.0f, 0,
            DWRITE_RENDERING_MODE_CLEARTYPE_NATURAL,
            DWRITE_MEASURING_MODE_NATURAL,
            0.0f, 0.0f, &GlyphRunAnalysis
        ));

        RECT TextureBounds;
        DWriteCall(GlyphRunAnalysis->GetAlphaTextureBounds(DWRITE_TEXTURE_CLEARTYPE_3x1, &TextureBounds));

        int TextureWidth = (TextureBounds.right - TextureBounds.left) * 3;
        int TextureHeight = TextureBounds.bottom - TextureBounds.top;

        u8 *TextureData = (u8 *) HeapAlloc(TextureWidth * TextureHeight);

        GlyphRunAnalysis->CreateAlphaTexture(
            DWRITE_TEXTURE_CLEARTYPE_3x1,
            &TextureBounds,
            TextureData,
            TextureWidth * TextureHeight
        );

        if (PenX + TextureWidth > FONT_ATLAS_WIDTH * 3) {
            PenX = 0;
            PenY += RowHeight;
            RowHeight = 0;
        }

        if (TextureHeight > RowHeight) {
            RowHeight = TextureHeight;
        }

        for (int Row = 0; Row < TextureHeight; ++Row) {
            for (int Column = 0; Column < TextureWidth; ++Column) {
                int Index = (PenY + Row) * (FONT_ATLAS_WIDTH * 3) + (PenX + Column);
                u8 D = TextureData[Row * TextureWidth + Column];
                AtlasData[Index] = D;
            }
        }

        PenX += TextureWidth;
        HeapFree(TextureData);
    }

    HeapFree(AtlasData);
}

void InitUI(Window *Win) {
    UI.RenderGroupArena = CreateArena(Megabytes(16));

    InitD3D11(Win);
    LoadShaders();
    InitDirectWrite();
}

void DestroyUI() {
    if (RenderTargetView) {
        RenderTargetView->Release();
    }
    if (SwapChain) {
        SwapChain->Release();
    }
    if (DeviceContext) {
        DeviceContext->Release();
    }
    if (Device) {
        Device->Release();
    }
    FreeArena(&UI.RenderGroupArena);
}

void BeginUIFrame(Window *Win) {
    int Width = Win->Size.X;
    int Height = Win->Size.Y;

    if (Win->Resized) {
        if (!DeviceContext || !SwapChain) return;

        if (RenderTargetView) {
            RenderTargetView->Release();
            RenderTargetView = 0;
        }

        DWriteCall(SwapChain->ResizeBuffers(2, Width, Height, DXGI_FORMAT_UNKNOWN, 0));

        ID3D11Texture2D* BackBuffer = 0;
        DWriteCall(SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&BackBuffer));

        DWriteCall(Device->CreateRenderTargetView(BackBuffer, 0, &RenderTargetView));
        BackBuffer->Release();

        DeviceContext->OMSetRenderTargets(1, &RenderTargetView, 0);

        D3D11_VIEWPORT Viewport = {};
        Viewport.Width = float(Width);
        Viewport.Height = float(Height);
        Viewport.MinDepth = 0.0f;
        Viewport.MaxDepth = 1.0f;

        DeviceContext->RSSetViewports(1, &Viewport);
    }

    float ClearColor[4] = { 0.0f, 0.2f, 0.4f, 1.0f };
    DeviceContext->ClearRenderTargetView(RenderTargetView, ClearColor);

    UIWindowConstants WindowConstants = {};
    WindowConstants.Size.X = Win->Size.X;
    WindowConstants.Size.Y = Win->Size.Y;

    D3D11_MAPPED_SUBRESOURCE Mapped;
    DeviceContext->Map(UI.WindowConstantsBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &Mapped);
    CopyMemory(Mapped.pData, &WindowConstants, sizeof(UIWindowConstants));
    DeviceContext->Unmap(UI.WindowConstantsBuffer, 0);

    ResetArena(&UI.RenderGroupArena);
    UI.FirstRenderGroup = 0;
    UI.CurrentRenderGroup = 0;
}

void EndUIFrame() {
    DeviceContext->OMSetRenderTargets(1, &RenderTargetView, 0);

    DeviceContext->VSSetShaderResources(0, 1, &UI.RCmdBufferView);
    DeviceContext->VSSetConstantBuffers(0, 1, &UI.WindowConstantsBuffer);

    DeviceContext->VSSetShader(UI.VertexShader, 0, 0);
    DeviceContext->PSSetShader(UI.PixelShader, 0, 0);

    UIRenderGroup *RenderGroup = UI.FirstRenderGroup;
    while (RenderGroup) {
        u64 Size = RenderGroup->CommandCount * sizeof(UIRenderCommand);

        D3D11_MAPPED_SUBRESOURCE Mapped;
        DeviceContext->Map(UI.RCmdBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &Mapped);
        CopyMemory(Mapped.pData, RenderGroup->Commands, Size);
        DeviceContext->Unmap(UI.RCmdBuffer, 0);

        DeviceContext->DrawInstanced(6, RenderGroup->CommandCount, 0, 0);

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

}

void PushTextCentered(String Text, UIBox Box, u32 Color) {
    Vec2 Position = Box.Position;
    Vec2 Size = Box.Size;

    Vec2 TextPos = Vec2();

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
    UIInteraction Interaction = {&Result, UI_INTERACTION_PRESS}; 

    u32 InteractionFlags = HandleUIInteraction(Box, Interaction);
    u32 RenderFlags = UI_DRAW_HOVERED | UI_DRAW_BORDER;

    DrawBox(Box, InteractionFlags, RenderFlags);
    PushTextCentered(Text, Box, 0xFFFFFF);

    return Result;
}

b8 UIToggleButton(b8 *Value, String Text, Vec2 Position, Vec2 Size) {
    b8 Result = 0;

    UIBox Box = {Position, Size};
    UIInteraction Interaction = {Value, UI_INTERACTION_TOGGLE}; 

    u32 InteractionFlags = HandleUIInteraction(Box, Interaction);
    u32 RenderFlags = UI_DRAW_ACTIVE | UI_DRAW_HOVERED | UI_DRAW_BORDER;

    DrawBox(Box, InteractionFlags, RenderFlags);
    PushTextCentered(Text, Box, 0xFFFFFF);

    Result = *Value;
    return Result;
}

