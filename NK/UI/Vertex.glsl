#version 410 core

#extension GL_ARB_explicit_uniform_location : enable

struct UIRenderCommand {
    float Px, Py;
    float Sx, Sy;
    uint Color;
    float Rounding;
    uint Border;
    uint Type;
    float UVx, UVy;
    float UVw, UVh;
};

flat out vec3 PassColor;
flat out vec3 PassBorderColor;
out vec2 PassRectCenter;
out vec2 PassRectHalfSize;
flat out float PassRounding;
flat out float PassBorder;
flat out uint PassType;
out vec2 PassUVCoords;

layout(std140) uniform RenderCommands {
    UIRenderCommand Commands[512];
};

layout(location=1) uniform vec2 WindowSize;

const vec2 VertexPositions[] = vec2[](
    vec2(0, 1),
    vec2(0, 0),
    vec2(1, 0),
    vec2(0, 1),
    vec2(1, 0),
    vec2(1, 1)
);

const vec2 UVPositions[] = vec2[](
    vec2(0, 1),
    vec2(0, 0),
    vec2(1, 0),
    vec2(0, 1),
    vec2(1, 0),
    vec2(1, 1)
);

vec3 UnpackColor(uint Hex) {
    uint R = (Hex >> 16) & 0xFFu;
    uint G = (Hex >> 8) & 0xFFu;
    uint B = Hex & 0xFFu;

    return vec3(R, G, B) * (1.0/255.0);
}

void main() {
    UIRenderCommand Command = Commands[gl_InstanceID];

    uint VertexIndex = gl_VertexID % 6;
    vec2 VertexPosition = VertexPositions[VertexIndex];
    vec2 UVPosition = UVPositions[VertexIndex];

    vec2 Pos = vec2(Command.Px, Command.Py);
    vec2 Size = vec2(Command.Sx, Command.Sy);
    
    vec2 Position = fma(VertexPosition, Size, Pos);
    vec2 WindowPosition = vec2(
        (Position.x / WindowSize.x) * 2.0 - 1.0,
        1.0 - (Position.y / WindowSize.y) * 2.0
    );

    gl_Position = vec4(WindowPosition, 0.0, 1.0);

    vec2 UVPos = vec2(Command.UVx, Command.UVy);
    vec2 UVSize = vec2(Command.UVw, Command.UVh);
    vec2 UVCoords = fma(UVPosition, UVSize, UVPos);

    PassColor = UnpackColor(Command.Color);
    PassBorderColor = UnpackColor(Command.Border);
    PassRectCenter = vec2(VertexPosition - 0.5) * Size;
    PassRectHalfSize = Size * 0.5;
    PassRounding = Command.Rounding;
    PassBorder = float((Command.Border >> 24) & 0xFFu);
    PassType = Command.Type;
    PassUVCoords = UVCoords;
}
