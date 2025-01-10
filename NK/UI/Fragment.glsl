#version 410 core

#extension GL_ARB_explicit_uniform_location : enable

flat in vec3 PassColor;
flat in vec3 PassBorderColor;
in vec2 PassRectCenter;
in vec2 PassRectHalfSize;
flat in float PassRounding;
flat in float PassBorder;
flat in uint PassType;
in vec2 PassUVCoords;

layout(location=2) uniform sampler2D Texture;

layout(location=0) out vec4 FragColor;

float RoundedBoxSDF(vec2 Center, vec2 Size, float Radius) {
    vec2 Q = abs(Center) - Size + Radius;
    return length(max(Q, 0.0)) + min(max(Q.x, Q.y), 0.0) - Radius;
}

void main() {
    vec2 SmoothedSize = PassRectHalfSize - 0.5;
    float CornerDistance = RoundedBoxSDF(PassRectCenter, SmoothedSize, PassRounding);
    float CornerFactor = 1.0 - smoothstep(0, 2, CornerDistance);

    if (CornerFactor < 0.001) {
        discard;
    }

    FragColor = vec4(PassColor, 1.0);

    vec4 Border = vec4(1.0);
    if (PassBorder > 0.0) {
        vec2 InnerSize = SmoothedSize - PassBorder;
        float InnerRadiusFactor = min(InnerSize.x/PassRectHalfSize.x, InnerSize.y/PassRectHalfSize.y);
        float InnerRounding = PassRounding * (InnerRadiusFactor * InnerRadiusFactor);
        float InnerDistance = RoundedBoxSDF(PassRectCenter, InnerSize, InnerRounding);
        float InnerFactor = 1.0 - smoothstep(0, 2, InnerDistance);
        float BorderFactor = CornerFactor * (1.0 - InnerFactor);
        
        FragColor = vec4(mix(PassColor, PassBorderColor, BorderFactor), 1.0) * CornerFactor;
    }

    if (PassType == 1) {
        // Text
        FragColor = vec4(texture(Texture, PassUVCoords).r);
    }

    FragColor *= CornerFactor;
    FragColor *= Border;
}
