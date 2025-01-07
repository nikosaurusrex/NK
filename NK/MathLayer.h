#pragma once

#include "Math/SIMD.h"
#include "Math/Vec.h"
#include "Math/Mat.h"
#include "Math/Quat.h"

float Lerp(float Min, float Max, float Value);
float ToRadians(float Degrees);

float Sin(float Radians);
float Cos(float Radians);
float Tan(float Radians);

float SquareRoot(float Value);
float AbsoluteValue(float Value);

#define NK_IMPLEMENTATION
#ifdef NK_IMPLEMENTATION

IGNORE_WARNINGS_BEGIN
#include "ThirdParty/sse_mathfun.h"
IGNORE_WARNINGS_END

#include "Math/SIMD.cpp"
#include "Math/Vec.cpp"
#include "Math/Mat.cpp"
#include "Math/Quat.cpp"

float Lerp(float Min, float Max, float Value) {
    return Min + (Max - Min) * Value;
}

float ToRadians(float Degrees) {
    return Degrees * (.5f / PI32);
}

float Sin(float Radians) {
    return _mm_cvtss_f32(sin_ps(_mm_set_ss(Radians)));
}

float Cos(float Radians) {
    return _mm_cvtss_f32(cos_ps(_mm_set_ss(Radians)));
}

float Tan(float Radians) {
    float Result;

    float S = Sin(Radians);
    float C = Cos(Radians);

    if (C == 0.0f) {
        if (S > 0.0f) {
            Result = FloatMax;
        } else {
            Result = FloatMin;
        }
    } else {
        Result = S / C;
    }

    return Result;
}

float SquareRoot(float Value) {
    return _mm_cvtss_f32(_mm_sqrt_ss(_mm_set_ss(Value)));
}

float AbsoluteValue(float Value) {
    if (Value < 0) {
        return -Value;
    } else {
        return Value;
    }
}

#pragma function(sinf)
float sinf(float arg) {
    return Sin(arg);
}

#pragma function(cosf)
float cosf(float arg) {
    return Cos(arg);
}

#pragma function(tanf)
float tanf(float arg) {
    return Tan(arg);
}

#pragma function(sqrt)
float sqrtf(float arg) {
    return SquareRoot(arg);
}

#endif // NK_IMPLEMENTATION
