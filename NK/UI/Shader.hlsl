struct UIRenderCommand {
    float2 Position;
    float2 Size;
    uint Color;
    float Rounding;
    uint Border;
    uint Type;
    float2 UVPosition;
    float2 UVSize;
};

StructuredBuffer<UIRenderCommand> RenderCommands : register(t0);

cbuffer WindowConstants : register(b0) {
    float2 WindowSize;
    float2 Placeholder;
};

struct VSOutput {
    float4 Position : SV_POSITION;
    float3 Color : COLOR0;
    float3 BorderColor : COLOR1;
    float2 RectCenter : TEXCOORD0;
    float2 RectHalfSize : TEXCOORD1;
    float Rounding : TEXCOORD2;
    float Border : TEXCOORD3;
    uint Type : TEXCOORD4;
    float2 UVCoords : TEXCOORD5;
};

static const float2 VertexPositions[6] = {
    float2(0, 1),
    float2(1, 0),
    float2(0, 0),
    float2(0, 1),
    float2(1, 1),
    float2(1, 0)
};

static const float2 UVPositions[6] = {
    float2(0, 1),
    float2(0, 0),
    float2(1, 0),
    float2(0, 1),
    float2(1, 0),
    float2(1, 1)
};

float3 UnpackColor(uint Hex) {
    uint R = (Hex >> 16) & 0xFF;
    uint G = (Hex >> 8) & 0xFF;
    uint B = Hex & 0xFF;

    return float3(R, G, B) * (1.0 / 255.0);
}

float RoundedBoxSDF(float2 Center, float2 Size, float Radius) {
    float2 Q = abs(Center) - Size + Radius;
    return length(max(Q, 0.0)) + min(max(Q.x, Q.y), 0.0) - Radius;
}

VSOutput VSMain(uint VertexID : SV_VertexID, uint InstanceID : SV_InstanceID) {
    VSOutput Output = (VSOutput) 0;
    
    UIRenderCommand Command = RenderCommands[InstanceID];

    uint VertexIndex = VertexID % 6;
    float2 VertexPosition = VertexPositions[VertexIndex];
    float2 UVPosition = UVPositions[VertexIndex];

    float2 Position = mad(VertexPosition, Command.Size, Command.Position);
    float2 WindowPosition = float2(
        (Position.x / WindowSize.x) * 2.0 - 1.0,
        1.0 - (Position.y / WindowSize.y) * 2.0
    );

    Output.Position = float4(WindowPosition, 0.0, 1.0);

    float2 UVCoords = mad(UVPosition, Command.UVSize, Command.UVPosition);

    Output.Color = UnpackColor(Command.Color);
    Output.BorderColor = UnpackColor(Command.Border);
    Output.RectCenter = (VertexPosition - 0.5) * Command.Size;
    Output.RectHalfSize = Command.Size * 0.5;
    Output.Rounding = Command.Rounding;
    Output.Border = float((Command.Border >> 24) & 0xFF);
    Output.Type = Command.Type;
    Output.UVCoords = UVCoords;

    return Output;
}

float4 PSMain(VSOutput Input) : SV_TARGET {
    float2 SmoothedSize = Input.RectHalfSize - 0.5;
    float CornerDistance = RoundedBoxSDF(Input.RectCenter, SmoothedSize, Input.Rounding);
    float CornerFactor = 1.0 - smoothstep(0, 2, CornerDistance);

    if (CornerFactor < 0.001) {
        discard;
    }

    float4 FragColor = float4(Input.Color, 1.0);

    if (Input.Border > 0.0) {
        float2 InnerSize = SmoothedSize - Input.Border;
        float InnerRadiusFactor = min(InnerSize.x / Input.RectHalfSize.x, InnerSize.y / Input.RectHalfSize.y);
        float InnerRounding = Input.Rounding * (InnerRadiusFactor * InnerRadiusFactor);
        float InnerDistance = RoundedBoxSDF(Input.RectCenter, InnerSize, InnerRounding);
        float InnerFactor = 1.0 - smoothstep(0, 2, InnerDistance);
        float BorderFactor = CornerFactor * (1.0 - InnerFactor);

        FragColor = float4(lerp(Input.Color, Input.BorderColor, BorderFactor), 1.0) * CornerFactor;
    }

    if (Input.Type == 1) {
        // Text rendering
        // FragColor = float4(Texture.Sample(Sampler, Input.PassUVCoords).r, 1.0, 1.0, 1.0);
    }

    FragColor *= CornerFactor;

    return FragColor;
}
