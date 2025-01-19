#pragma once

#include <d3d11.h>
#include <d3dcompiler.h>

enum {
    SHADER_VERTEX = 0,
    SHADER_PIXEL = 1,
    SHADER_FRAGMENT = 1
};

enum {
    BUFFER_STRUCTURED,
    BUFFER_CONSTANT
};

enum {
    TEXTURE_FORMAT_RGBA_UNORM,
    TEXTURE_FORMAT_BGRA_UNORM,
};

enum {
    TEXTURE_RENDER_TARGET = 0x1
};

#if OS_WINDOWS
struct GShader {
    ID3D11VertexShader *D3VS;
    ID3D11PixelShader *D3PS;
};

struct GBuffer {
    ID3D11Buffer *Handle;
    ID3D11ShaderResourceView *View;
    u32 Type;
};

struct GTexture {
    ID3D11Texture2D *Handle;
    ID3D11ShaderResourceView *View;
    ID3D11SamplerState *Sampler;
};
#else
struct GShader {
    u32 Handle;
};

struct GBuffer {
    u32 Handle;
    u32 Type;
};

struct GTexture {
    u32 Handle;
};
#endif

void InitGraphics(Window *Win);
void ReleaseGraphics();

void UpdateGraphics(Window *Win);

GShader LoadShader(NativeString VertexPath, NativeString PixelPath);
void ReleaseShader(GShader Shader);
void BindShader(GShader Shader);

GBuffer CreateBuffer(u32 Type, u32 Size, u32 Stride, b8 IsStatic, u32 NumElements, void *Data);
void ReleaseBuffer(GBuffer Buffer);
void UpdateBuffer(GBuffer Buffer, void *Data, u32 Size);
void BindBuffer(GBuffer Buffer, u32 Stage);

GTexture CreateTexture(int Width, int Height, int Format, u32 Flags);
void ReleaseTexture(GTexture Texture);
void BindTexture(GTexture Texture, u32 Stage);

void DrawInstanced(int Instances, int VerticesPerInstance);
