f32x4::f32x4() {
    SSE = _mm_setzero_ps();
}

f32x4::f32x4(float Value) {
    SSE = _mm_set1_ps(Value);
}

f32x4::f32x4(float X, float Y, float Z, float W) {
    SSE = _mm_set_ps(X, Y, Z, W);
}

f32x4::f32x4(float *Address) {
    SSE = _mm_loadu_ps(Address);
}

void Store(f32x4 V, float *Address) {
    _mm_storeu_ps(Address, V.SSE);
}

internal f32x4 FromSSE(__m128 SSE) {
    f32x4 Result = {};

    Result.SSE = SSE;

    return Result;
}

f32x4 operator+(f32x4 A, f32x4 B) {
    return FromSSE(_mm_add_ps(A.SSE, B.SSE));
}

f32x4 operator-(f32x4 A, f32x4 B) {
    return FromSSE(_mm_sub_ps(A.SSE, B.SSE));
}

f32x4 operator*(f32x4 A, f32x4 B) {
    return FromSSE(_mm_mul_ps(A.SSE, B.SSE));
}

f32x4 operator/(f32x4 A, f32x4 B) {
    return FromSSE(_mm_div_ps(A.SSE, B.SSE));
}

f32x4 operator-(f32x4 A) {
    return f32x4() - A;
}

f32x4 &operator+=(f32x4 &A, f32x4 B) {
    A = A + B;

    return A;
}

f32x4 &operator-=(f32x4 &A, f32x4 B) {
    A = A - B;

    return A;
}

f32x4 &operator*=(f32x4 &A, f32x4 B) {
    A = A * B;

    return A;
}

f32x4 &operator/=(f32x4 &A, f32x4 B) {
    A = A / B;

    return A;
}

f32x4 operator<(f32x4 A, f32x4 B) {
    return FromSSE(_mm_cmplt_ps(A.SSE, B.SSE));
}

f32x4 operator<=(f32x4 A, f32x4 B) {
    return FromSSE(_mm_cmple_ps(A.SSE, B.SSE));
}

f32x4 operator>(f32x4 A, f32x4 B) {
    return FromSSE(_mm_cmpgt_ps(A.SSE, B.SSE));
}

f32x4 operator>=(f32x4 A, f32x4 B) {
    return FromSSE(_mm_cmpge_ps(A.SSE, B.SSE));
}

f32x4 operator==(f32x4 A, f32x4 B) {
    return FromSSE(_mm_cmpeq_ps(A.SSE, B.SSE));
}

f32x4 operator!=(f32x4 A, f32x4 B) {
    return FromSSE(_mm_cmpneq_ps(A.SSE, B.SSE));
}

f32x4 operator&(f32x4 A, f32x4 B) {
    return FromSSE(_mm_and_ps(A.SSE, B.SSE));
}

f32x4 operator|(f32x4 A, f32x4 B) {
    return FromSSE(_mm_or_ps(A.SSE, B.SSE));
}

f32x4 &operator&=(f32x4 &A, f32x4 B) {
    A = A & B;

    return A;
}

f32x4 &operator|=(f32x4 &A, f32x4 B) {
    A = A | B;

    return A;
}

f32x4 Minimum(f32x4 A, f32x4 B) {
    return FromSSE(_mm_min_ps(A.SSE, B.SSE));
}

f32x4 Maximum(f32x4 A, f32x4 B) {
    return FromSSE(_mm_max_ps(A.SSE, B.SSE));
}

float HorizontalAdd(f32x4 A) {
    return A.E[0] + A.E[1] + A.E[2] + A.E[3];
}

float LowestFloat(f32x4 A) {
    return _mm_cvtss_f32(A.SSE);
}

b32 AnyTrue(f32x4 Cond) {
    return _mm_movemask_ps(Cond.SSE);
}

b32 AllTrue(f32x4 Cond) {
    return _mm_movemask_ps(Cond.SSE) == 15;
}

b32 AllFalse(f32x4 Cond) {
    return _mm_movemask_ps(Cond.SSE) == 0;
}
