#pragma once

#if ARCH_X64
#include <immintrin.h>
#endif

struct f32x4 {
    float E[4];

#if ARCH_X64
    __m128 SSE;
#endif

    f32x4();
    f32x4(float Value);
    f32x4(float X, float Y, float Z, float W);
    f32x4(float *Address);
};

void Store(f32x4 V, float *Address);

f32x4 operator+(f32x4 A, f32x4 B);
f32x4 operator-(f32x4 A, f32x4 B);
f32x4 operator*(f32x4 A, f32x4 B);
f32x4 operator/(f32x4 A, f32x4 B);

f32x4 operator-(f32x4 A);

f32x4 &operator+=(f32x4 &A, f32x4 B);
f32x4 &operator-=(f32x4 &A, f32x4 B);
f32x4 &operator*=(f32x4 &A, f32x4 B);
f32x4 &operator/=(f32x4 &A, f32x4 B);

f32x4 operator<(f32x4 A, f32x4 B);
f32x4 operator<=(f32x4 A, f32x4 B);
f32x4 operator>(f32x4 A, f32x4 B);
f32x4 operator>=(f32x4 A, f32x4 B);
f32x4 operator==(f32x4 A, f32x4 B);
f32x4 operator!=(f32x4 A, f32x4 B);

f32x4 operator&(f32x4 A, f32x4 B);
f32x4 operator|(f32x4 A, f32x4 B);

f32x4 &operator&=(f32x4 &A, f32x4 B);
f32x4 &operator|=(f32x4 &A, f32x4 B);

f32x4 Minimum(f32x4 A, f32x4 B);
f32x4 Maximum(f32x4 A, f32x4 B);

float HorizontalAdd(f32x4 A);

float LowestFloat(f32x4 A);

b32 AnyTrue(f32x4 Cond);
b32 AllTrue(f32x4 Cond);
b32 AllFalse(f32x4 Cond);
