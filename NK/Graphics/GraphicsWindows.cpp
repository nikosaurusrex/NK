global b8 GraphicsInitialized = 0;
global ID3D11Device *Device;
global ID3D11DeviceContext *DeviceContext;
global IDXGISwapChain *SwapChain;
global ID3D11RenderTargetView *RenderTargetView;
global ID3D11BlendState *BlendState;

#define CheckFail(Call) do {         \
    HRESULT HResult = Call;           \
    if (FAILED(HResult)) {            \
         Print("DWrite Call Failed: 0x%x\n", HResult); NK_TRAP(); Exit(1); \
    }                                 \
} while (0);

void InitGraphics(Window *Win) {
    if (GraphicsInitialized) return;
    GraphicsInitialized = 1;

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

    u32 Flags = D3D11_CREATE_DEVICE_SINGLETHREADED | D3D11_CREATE_DEVICE_BGRA_SUPPORT;

#ifndef NDEBUG
    Flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    D3D_FEATURE_LEVEL Features[] = {
        D3D_FEATURE_LEVEL_11_1,
    };

    CheckFail(D3D11CreateDeviceAndSwapChain(
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
    CheckFail(SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&BackBuffer));

    Device->CreateRenderTargetView(BackBuffer, 0, &RenderTargetView);
    BackBuffer->Release();

    D3D11_BLEND_DESC BlendDesc = {};
    BlendDesc.RenderTarget[0].BlendEnable = TRUE;
    BlendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
    BlendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
    BlendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
    BlendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
    BlendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
    BlendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
    BlendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

    CheckFail(Device->CreateBlendState(&BlendDesc, &BlendState));

    float BlendFactor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
    u32 SampleMask = 0xFFFFFFFF;
    DeviceContext->OMSetBlendState(BlendState, BlendFactor, SampleMask);
    DeviceContext->OMSetRenderTargets(1, &RenderTargetView, 0);

    D3D11_VIEWPORT Viewport = {};
    Viewport.Width = float(Win->Size.X);
    Viewport.Height = float(Win->Size.Y);
    Viewport.MinDepth = 0.0f;
    Viewport.MaxDepth = 1.0f;
    DeviceContext->RSSetViewports(1, &Viewport);
    
    DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
}

void ReleaseGraphics() {
    GraphicsInitialized = 0;

    if (BlendState) BlendState->Release();
    if (RenderTargetView) RenderTargetView->Release();
    if (SwapChain) SwapChain->Release();
    if (DeviceContext) DeviceContext->Release();
    if (Device) Device->Release();
}

void UpdateGraphics(Window *Win) {
    int Width = Win->Size.X;
    int Height = Win->Size.Y;

    if (Win->Resized) {
        if (!DeviceContext || !SwapChain) return;

        if (RenderTargetView) {
            RenderTargetView->Release();
            RenderTargetView = 0;
        }

        CheckFail(SwapChain->ResizeBuffers(2, Width, Height, DXGI_FORMAT_UNKNOWN, 0));

        ID3D11Texture2D* BackBuffer = 0;
        CheckFail(SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&BackBuffer));

        CheckFail(Device->CreateRenderTargetView(BackBuffer, 0, &RenderTargetView));
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
    float BlendFactor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
    u32 SampleMask = 0xFFFFFFFF;
    DeviceContext->OMSetBlendState(BlendState, BlendFactor, SampleMask);
    DeviceContext->OMSetRenderTargets(1, &RenderTargetView, 0);
}

void CompileShader(NativeString FilePath, LPCSTR EntryPoint, LPCSTR Target, ID3DBlob **Blob) {
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

GShader LoadShader(NativeString VertexPath, NativeString PixelPath) {
    GShader Result = {};

    ID3DBlob *VSBlob;
    ID3DBlob *PSBlob;

    CompileShader(VertexPath, "VSMain", "vs_5_0", &VSBlob);
    CompileShader(PixelPath, "PSMain", "ps_5_0", &PSBlob);

    Device->CreateVertexShader(VSBlob->GetBufferPointer(), VSBlob->GetBufferSize(), 0, &Result.D3VS);
    Device->CreatePixelShader(PSBlob->GetBufferPointer(), PSBlob->GetBufferSize(), 0, &Result.D3PS);

    VSBlob->Release();
    PSBlob->Release();

    return Result;
}

void ReleaseShader(GShader Shader) {
    Shader.D3VS->Release();
    Shader.D3PS->Release();
}

void BindShader(GShader Shader) {
    DeviceContext->VSSetShader(Shader.D3VS, 0, 0);
    DeviceContext->PSSetShader(Shader.D3PS, 0, 0);
}

GBuffer CreateBuffer(u32 Type, u32 Size, u32 Stride, b8 IsStatic, u32 NumElements, void *Data) {
    GBuffer Result = {};
    Result.Type = Type;

    u32 MiscFlags = 0;
    u32 UsageFlags = 0;

    if (IsStatic) {
        UsageFlags |= D3D11_USAGE_IMMUTABLE;
    } else {
        UsageFlags |= D3D11_USAGE_DYNAMIC;
    }

    if (Type == BUFFER_STRUCTURED) {
        MiscFlags |= D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
    }

    D3D11_BUFFER_DESC BufferDesc = {};
    BufferDesc.ByteWidth = Size;
    BufferDesc.Usage = (D3D11_USAGE) UsageFlags;
    BufferDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    BufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    BufferDesc.StructureByteStride = Stride;
    BufferDesc.MiscFlags = MiscFlags;

    if (Data) {
        D3D11_SUBRESOURCE_DATA DataDesc = {};
        DataDesc.pSysMem = Data;

        CheckFail(Device->CreateBuffer(&BufferDesc, &DataDesc, &Result.Handle));
    } else {
        CheckFail(Device->CreateBuffer(&BufferDesc, 0, &Result.Handle));
    }

    if (Type != BUFFER_CONSTANT) {
        D3D11_SHADER_RESOURCE_VIEW_DESC ViewDesc = {};
        ViewDesc.Format = DXGI_FORMAT_UNKNOWN;
        ViewDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
        ViewDesc.Buffer.FirstElement = 0;
        ViewDesc.Buffer.NumElements = NumElements;

        CheckFail(Device->CreateShaderResourceView(Result.Handle, &ViewDesc, &Result.View));
    }
    
    return Result;
}

void ReleaseBuffer(GBuffer Buffer) {
    Buffer.Handle->Release();
    if (Buffer.View) Buffer.View->Release();
}

void UpdateBuffer(GBuffer Buffer, void *Data, u32 Size) {
    D3D11_MAPPED_SUBRESOURCE Mapped;
    DeviceContext->Map(Buffer.Handle, 0, D3D11_MAP_WRITE_DISCARD, 0, &Mapped);
    CopyMemory(Mapped.pData, Data, Size);
    DeviceContext->Unmap(Buffer.Handle, 0);
}

void BindBuffer(GBuffer Buffer, u32 Stage) {
    if (Buffer.Type == BUFFER_STRUCTURED) {
        if (Stage == SHADER_VERTEX) {
            DeviceContext->VSSetShaderResources(0, 1, &Buffer.View);
        } else if (Stage == SHADER_PIXEL) {
            DeviceContext->PSSetShaderResources(0, 1, &Buffer.View);
        }
    } else if (Buffer.Type == BUFFER_CONSTANT) {
        if (Stage == SHADER_VERTEX) {
            DeviceContext->VSSetConstantBuffers(0, 1, &Buffer.Handle);
        } else if (Stage == SHADER_PIXEL) {
            DeviceContext->PSSetConstantBuffers(0, 1, &Buffer.Handle);
        }
    }
}

GTexture CreateTexture(int Width, int Height, int Format, u32 Flags) {
    GTexture Result = {};

    u32 D3Format = 0;
    switch (Format) {
    case TEXTURE_FORMAT_RGBA_UNORM: D3Format = DXGI_FORMAT_R8G8B8A8_UNORM; break;
    case TEXTURE_FORMAT_BGRA_UNORM: D3Format = DXGI_FORMAT_B8G8R8A8_UNORM; break;
    }

    u32 BindFlags = D3D11_BIND_SHADER_RESOURCE;
    if (Flags & TEXTURE_RENDER_TARGET) {
        BindFlags |= D3D11_BIND_RENDER_TARGET;
    }

    D3D11_TEXTURE2D_DESC TextureDesc = {};
    TextureDesc.Width = Width;
    TextureDesc.Height = Height;
    TextureDesc.MipLevels = 1;
    TextureDesc.ArraySize = 1;
    TextureDesc.Format = (DXGI_FORMAT) D3Format;
    TextureDesc.SampleDesc.Count = 1;
    TextureDesc.Usage = D3D11_USAGE_DEFAULT;
    TextureDesc.BindFlags = BindFlags;

    CheckFail(Device->CreateTexture2D(&TextureDesc, 0, &Result.Handle));
    CheckFail(Device->CreateShaderResourceView(Result.Handle, 0, &Result.View));

    D3D11_SAMPLER_DESC SamplerDesc = {};
    SamplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
    SamplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
    SamplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
    SamplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
    SamplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    SamplerDesc.MinLOD = 0;
    SamplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

    CheckFail(Device->CreateSamplerState(&SamplerDesc, &Result.Sampler));
    
    return Result;
}

void ReleaseTexture(GTexture Texture) {
    Texture.Handle->Release();
    Texture.View->Release();
    Texture.Sampler->Release();
}

void BindTexture(GTexture Texture, u32 Stage) {
    if (Stage == SHADER_VERTEX) {
        DeviceContext->VSSetShaderResources(0, 1, &Texture.View);
        DeviceContext->VSSetSamplers(0, 1, &Texture.Sampler);
    } else if (Stage == SHADER_PIXEL) {
        DeviceContext->PSSetShaderResources(0, 1, &Texture.View);
        DeviceContext->PSSetSamplers(0, 1, &Texture.Sampler);
    }
}

void DrawInstanced(int Instances, int VerticesPerInstance) {
    DeviceContext->DrawInstanced(VerticesPerInstance, Instances, 0, 0);
}
