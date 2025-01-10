#version 410 core

#extension GL_ARB_explicit_uniform_location : enable

struct UIRenderCommand {
    vec2 Position;
    vec2 Size;
    uint Color;
    float Rounding;
    uint Border;
    uint Type;
};

flat out vec3 PassColor;
flat out vec3 PassBorderColor;
out vec2 PassRectCenter;
out vec2 PassRectHalfSize;
flat out float PassRounding;
flat out float PassBorder;
flat out uint PassType;

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
    
    vec2 Position = fma(VertexPosition, Command.Size, Command.Position);
    vec2 WindowPosition = vec2(
        (Position.x / WindowSize.x) * 2.0 - 1.0,
        1.0 - (Position.y / WindowSize.y) * 2.0
    );

    gl_Position = vec4(WindowPosition, 0.0, 1.0);

    PassColor = UnpackColor(Command.Color);
    PassBorderColor = UnpackColor(Command.Border);
    PassRectCenter = vec2(VertexPosition - 0.5) * Command.Size;
    PassRectHalfSize = Command.Size * 0.5;
    PassRounding = Command.Rounding;
    PassBorder = float((Command.Border >> 24) & 0xFFu);
    PassType = Command.Type;
}
