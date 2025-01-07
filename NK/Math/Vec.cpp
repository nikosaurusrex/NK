Vec2 operator+(Vec2 A, Vec2 B) {
    return Vec2(A.X + B.X, A.Y + B.Y);
}

Vec2 operator-(Vec2 A, Vec2 B) {
    return Vec2(A.X - B.X, A.Y - B.Y);
}

Vec2 operator*(Vec2 A, float S) {
    return Vec2(A.X * S, A.Y * S);
}

Vec2 operator*(float S, Vec2 A) {
    return Vec2(A.X * S, A.Y * S); 
}

Vec2 operator/(Vec2 A, float S) {
    return Vec2(A.X / S, A.Y / S);
}

float Dot(Vec2 A, Vec2 B) {
    return A.X * B.X + A.Y * B.Y;
}

float LengthSquared(Vec2 V) {
    return Dot(V, V);
}

float Length(Vec2 V) {
    return sqrtf(LengthSquared(V));
}

Vec2 Normalize(Vec2 V) {
    return V / Length(V);
}

Vec3 operator+(Vec3 A, Vec3 B) {
    return Vec3(A.X + B.X, A.Y + B.Y, A.Z + B.Z);
}

Vec3 operator-(Vec3 A, Vec3 B) {
    return Vec3(A.X - B.X, A.Y - B.Y, A.Z - B.Z);
}

Vec3 operator*(Vec3 A, float S) {
    return Vec3(A.X * S, A.Y * S, A.Z * S);
}

Vec3 operator*(float S, Vec3 A) {
    return Vec3(A.X * S, A.Y * S, A.Z * S);
}

Vec3 operator/(Vec3 A, float S) {
    return Vec3(A.X / S, A.Y / S, A.Z / S);
}

float Dot(Vec3 A, Vec3 B) {
    return A.X * B.X + A.Y * B.Y + A.Z * B.Z;
}

Vec3 Cross(Vec3 A, Vec3 B) {
    return Vec3(
        A.Y * B.Z - A.Z * B.Y,
        A.Z * B.X - A.X * B.Z,
        A.X * B.Y - A.Y * B.X
    );
}

float LengthSquared(Vec3 V) {
    return Dot(V, V);
}

float Length(Vec3 V) {
    return sqrtf(LengthSquared(V));
}

Vec3 Normalize(Vec3 V) {
    return V / Length(V);
}

Vec4 operator+(Vec4 A, Vec4 B) {
    return Vec4(A.EW + A.EW);
}

Vec4 operator-(Vec4 A, Vec4 B) {
    return Vec4(A.EW - A.EW);
}

Vec4 operator*(Vec4 A, Vec4 B) {
    return Vec4(A.EW * A.EW);
}

Vec4 operator/(Vec4 A, Vec4 B) {
    return Vec4(A.EW / A.EW);
}

float Dot(Vec4 A, Vec4 B) {
    f32x4 Result = A.EW * B.EW;
    return HorizontalAdd(Result);
}

float LengthSquared(Vec4 V) {
    return Dot(V, V);
}

float Length(Vec4 V) {
    return sqrtf(LengthSquared(V));
}

Vec4 Normalize(Vec4 V) {
    float Len = Length(V);
    f32x4 InvLen = f32x4(1.0f / Len);
    return Vec4(V.EW * InvLen);
}

