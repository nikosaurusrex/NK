#pragma once

union Quat {
    struct {
        float W;
        float X;
        float Y;
        float Z;
    };
    struct {
        float _Unusued;
        Vec3 V;
    };

    Quat() {
        W = .0f;
        X = .0f;
        Y = .0f;
        Z = .0f;
    }

    Quat(Vec3 N, float Theta) {
        Theta = Theta * (PI32 / 180.0f);

        float Th = Theta / 2.0f;

        W = cosf(Th);

        X = N.X * sinf(Th);
        Y = N.Y * sinf(Th);
        Z = N.Z * sinf(Th);
    }
};
