#pragma once

union Mat4 {
    float M[4][4];
    Vec4 Columns[4];

    Mat4() {
        SetMemory(M, 0, sizeof(M));
    }

    Mat4(float Value) {
        SetMemory(M, 0, sizeof(M));
        M[0][0] = Value;
        M[1][1] = Value;
        M[2][2] = Value;
        M[3][3] = Value;
    }

    Vec4 &operator[](u32 Index) {
        return Columns[Index];
    }

    const Vec4 &operator[](u32 Index) const {
        return Columns[Index];
    }
};
