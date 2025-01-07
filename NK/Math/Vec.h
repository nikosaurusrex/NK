#pragma once

union Vec2 {
    struct {
        float X;
        float Y;
    };
    float E[2];

    Vec2() {
        X = 0;
        Y = 0;
    }

    Vec2(float V) {
        X = V;
        Y = V;
    }

    Vec2(float _X, float _Y) {
        X = _X;
        Y = _Y;
    }
};

Vec2 operator+(Vec2 A, Vec2 B);
Vec2 operator-(Vec2 A, Vec2 B);
Vec2 operator*(Vec2 A, float S);
Vec2 operator*(float S, Vec2 A);
Vec2 operator/(Vec2 A, float S);

float Dot(Vec2 A, Vec2 B);
float LengthSquared(Vec2 V);
float Length(Vec2 V);
Vec2 Normalize(Vec2 V);

union Vec3 {
    struct {
        float X, Y, Z;
    };
    struct {
        float R, G, B;
    };
    float E[3];

    Vec3() {
        X = 0;
        Y = 0;
        Z = 0;
    }

    Vec3(float V) {
        X = V;
        Y = V;
        Z = V;
    }

    Vec3(float _X, float _Y, float _Z) {
        X = _X;
        Y = _Y;
        Z = _Z;
    }
};

Vec3 operator+(Vec3 A, Vec3 B);
Vec3 operator-(Vec3 A, Vec3 B);
Vec3 operator*(Vec3 A, float S);
Vec3 operator*(float S, Vec3 A);
Vec3 operator/(Vec3 A, float S);

float Dot(Vec3 A, Vec3 B);
Vec3 Cross(Vec3 A, Vec3 B);
float LengthSquared(Vec3 V);
float Length(Vec3 V);
Vec3 Normalize(Vec3 V);

struct Vec4 {
    struct {
        float X, Y, Z, W;
    };

    float E[4];
    f32x4 EW;

    Vec4() {
        EW = f32x4();
    }

    Vec4(float V) {
        EW = f32x4(V);
    }

    Vec4(float X, float Y, float Z, float W) {
        EW = f32x4(W, Z, Y, X);
    }

    Vec4(f32x4 _W) {
        EW = _W;
    }
};


