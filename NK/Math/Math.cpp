IGNORE_WARNINGS_BEGIN
#include "ThirdParty/sse_mathfun.h"
IGNORE_WARNINGS_END

#if ARCH_X64
#include <smmintrin.h>

float Sin(float Radians) {
    return _mm_cvtss_f32(sin_ps(_mm_set_ss(Radians)));
}

float Cos(float Radians) {
    return _mm_cvtss_f32(cos_ps(_mm_set_ss(Radians)));
}

float SquareRoot(float Value) {
    return _mm_cvtss_f32(_mm_sqrt_ss(_mm_set_ss(Value)));
}

int IFloor(float X) {
    return _mm_cvtss_si32(_mm_floor_ss(_mm_setzero_ps(), _mm_set_ss(X)));
}

int ICeil(float X) {
    return _mm_cvtss_si32(_mm_ceil_ss(_mm_setzero_ps(), _mm_set_ss(X)));
}
#else
#error Unsupported math for architecture
#endif

float Lerp(float Min, float Max, float Value) {
    return Min + (Max - Min) * Value;
}

float FMod(float X, float Y) {
    int N = (int)(X / Y);
    return X - Y * N;
}

float ToRadians(float Degrees) {
    return Degrees * (.5f / PI32);
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

float AbsoluteValue(float Value) {
    if (Value < 0) {
        return -Value;
    } else {
        return Value;
    }
}

float Power(float Base, float Exponent) {
    float Result = 1;

    for (int i = 0; i < Exponent; ++i) {
        Result *= Base;
    }

    return Result;
}

// TODO: replace some time
//https://stackoverflow.com/questions/3380628/fast-arc-cos-algorithm
float ACos(float X) {
    float negate = float(X < 0);
    X = AbsoluteValue(X);
    float ret = -0.0187293;
    ret = ret * X;
    ret = ret + 0.0742610;
    ret = ret * X;
    ret = ret - 0.2121144;
    ret = ret * X;
    ret = ret + 1.5707288;
    ret = ret * SquareRoot(1.0-X);
    ret = ret - 2 * negate * ret;
    return negate * 3.14159265358979 + ret;
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
