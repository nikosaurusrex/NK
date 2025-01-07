#include "NK/Test/Test.h"

TestGroup(Math) {
    Test(Vec2 Addition) {
        Vec2 A(1.0f, 2.0f);
        Vec2 B(3.0f, 4.0f);
        Vec2 C = A + B;
        Assert(C.X == 4.0f);
        Assert(C.Y == 6.0f);
    }

    Test(Vec2 Subtraction) {
        Vec2 A(5.0f, 7.0f);
        Vec2 B(3.0f, 4.0f);
        Vec2 C = A - B;
        Assert(C.X == 2.0f);
        Assert(C.Y == 3.0f);
    }

    Test(Vec2 Multiplication) {
        Vec2 A(2.0f, 3.0f);
        Vec2 B = A * 2.0f;
        Assert(B.X == 4.0f);
        Assert(B.Y == 6.0f);
    }

    Test(Vec2 Dot) {
        Vec2 A(1.0f, 2.0f);
        Vec2 B(3.0f, 4.0f);
        float DotResult = Dot(A, B);
        Assert(DotResult == 11.0f);
    }

    Test(Vec2 LengthSquared) {
        Vec2 A(3.0f, 4.0f);
        float LenSq = LengthSquared(A);
        Assert(LenSq == 25.0f);
    }

    Test(Vec2 Length) {
        Vec2 A(3.0f, 4.0f);
        float Len = Length(A);
        Assert(Len == 5.0f);
    }

    Test(Vec2 Normalize) {
        Vec2 A(3.0f, 4.0f);
        Vec2 Norm = Normalize(A);
        Assert(Length(Norm) == 1.0f);
    }

    Test(Vec3 Addition) {
        Vec3 A(1.0f, 2.0f, 3.0f);
        Vec3 B(4.0f, 5.0f, 6.0f);
        Vec3 C = A + B;
        Assert(C.X == 5.0f);
        Assert(C.Y == 7.0f);
        Assert(C.Z == 9.0f);
    }

    Test(Vec3 CrossProduct) {
        Vec3 A(1.0f, 0.0f, 0.0f);
        Vec3 B(0.0f, 1.0f, 0.0f);
        Vec3 C = Cross(A, B);
        Assert(C.X == 0.0f);
        Assert(C.Y == 0.0f);
        Assert(C.Z == 1.0f);
    }

    Test(Vec3 Length) {
        Vec3 A(1.0f, 2.0f, 2.0f);
        float Len = Length(A);
        Assert(Len == 3.0f);
    }

    Test(Vec3 Normalize) {
        Vec3 A(1.0f, 2.0f, 2.0f);
        Vec3 Norm = Normalize(A);
        Assert(Length(Norm) == 1.0f);
    }

    Test(SquareRoot) {
        float Value = 16.0f;
        Assert(SquareRoot(Value) == 4.0f);
    }

    Test(Sin/Cos) {
        AssertFloat(Sin(0), 0.0f);
        AssertFloat(Sin(PI32 / 2.f), 1.0f);
        AssertFloat(Sin(PI32 / 4.f), 0.707107f);

        AssertFloat(Cos(0), 1.0f);
        AssertFloat(Cos(PI32 / 2.f), 0.0f);
        AssertFloat(Cos(PI32), -1.0f);
        AssertFloat(Cos(PI32 / 4.f), 0.707107f);
    }
}

