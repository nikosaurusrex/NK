Vec4 LinearCombination(Vec4 V, Mat4 M) {
    Vec4 Result = {};

    Result.X = V.X * M[0].X;
    Result.Y = V.X * M[0].Y;
    Result.Z = V.X * M[0].Z;
    Result.W = V.X * M[0].W;

    Result.X += V.Y * M[1].X;
    Result.Y += V.Y * M[1].Y;
    Result.Z += V.Y * M[1].Z;
    Result.W += V.Y * M[1].W;

    Result.X += V.Z * M[2].X;
    Result.Y += V.Z * M[2].Y;
    Result.Z += V.Z * M[2].Z;
    Result.W += V.Z * M[2].W;

    Result.X += V.W * M[3].X;
    Result.Y += V.W * M[3].Y;
    Result.Z += V.W * M[3].Z;
    Result.W += V.W * M[3].W;

    return Result;
}

Mat4 operator*(Mat4 A, Mat4 B) {
    Mat4 Result = {};

    Result[0] = LinearCombination(B[0], A);
    Result[1] = LinearCombination(B[1], A);
    Result[2] = LinearCombination(B[2], A);
    Result[3] = LinearCombination(B[3], A);

    return Result;
}

Mat4 Ortho(float Left, float Right, float Bottom, float Top) {
    Mat4 Result(1.0f);

    Result.M[0][0] = 2.0f / (Right - Left);
    Result.M[1][1] = 2.0f / (Top - Bottom);
    Result.M[2][2] = -1.0f;
    Result.M[3][0] = -(Right + Left) / (Right - Left);
    Result.M[3][1] = -(Top + Bottom) / (Top - Bottom);

    return Result;
}

Mat4 Perspective(float FOV, float Aspect, float Near, float Far) {
    Mat4 Result(1.0f);

    float THFov = tanf(FOV / 2.0f);

    Result.M[0][0] = 1.0f / (Aspect * THFov);
    Result.M[1][1] = -1.0f / THFov;
    Result.M[2][2] = Far / (Near - Far);
    Result.M[2][3] = -1.0f;
    Result.M[3][2] = -(Far * Near) / (Far - Near);

    return Result;
}

Mat4 LookAt(Vec3 Eye, Vec3 Center, Vec3 Up) {
    Mat4 Result(1.0f);

    Vec3 F = Normalize(Center - Eye);
    Vec3 S = Normalize(Cross(F, Up));
    Vec3 U = Cross(S, F);

    Result.M[0][0] = S.X;
    Result.M[1][0] = S.Y;
    Result.M[2][0] = S.Z;
    Result.M[0][1] = U.X;
    Result.M[1][1] = U.Y;
    Result.M[2][1] = U.Z;
    Result.M[0][2] = -F.X;
    Result.M[1][2] = -F.Y;
    Result.M[2][2] = -F.Z;
    Result.M[3][0] = -Dot(S, Eye);
    Result.M[3][1] = -Dot(U, Eye);
    Result.M[3][2] = Dot(F, Eye);

    return Result;
}

Mat4 Scale(Mat4 M, Vec3 V) {
    Mat4 Result = M;

    Result.M[0][0] *= V.X;
    Result.M[1][0] *= V.X;
    Result.M[2][0] *= V.X;
    Result.M[3][0] *= V.X;

    Result.M[0][1] *= V.Y;
    Result.M[1][1] *= V.Y;
    Result.M[2][1] *= V.Y;
    Result.M[3][1] *= V.Y;

    Result.M[0][2] *= V.Z;
    Result.M[1][2] *= V.Z;
    Result.M[2][2] *= V.Z;
    Result.M[3][2] *= V.Z;

    return Result;
}

Mat4 Translate(Mat4 M, Vec3 V) {
    Mat4 Result = M;

    Result.M[0][3] = M.M[0][0] * V.X + M.M[0][1] * V.Y + M.M[0][2] * V.Z + M.M[0][3];
    Result.M[1][3] = M.M[1][0] * V.X + M.M[1][1] * V.Y + M.M[1][2] * V.Z + M.M[1][3];
    Result.M[2][3] = M.M[2][0] * V.X + M.M[2][1] * V.Y + M.M[2][2] * V.Z + M.M[2][3];
    Result.M[3][3] = M.M[3][0] * V.X + M.M[3][1] * V.Y + M.M[3][2] * V.Z + M.M[3][3];

    return Result;
}

Mat4 Rotate(Mat4 M, float Angle, Vec3 V) {
    Mat4 Result;

    float C = Cos(Angle);
    float S = Sin(Angle);

    Vec3 Axis = Normalize(V);
    Vec3 Temp = (1.0f - C) * Axis;

    Mat4 Rotation = {};
    Rotation.M[0][0] = C + Temp.X * Axis.X;
    Rotation.M[0][1] = Temp.X * Axis.Y + S * Axis.Z;
    Rotation.M[0][2] = Temp.X * Axis.Z - S * Axis.Y;

    Rotation.M[1][0] = Temp.Y * Axis.X - S * Axis.Z;
    Rotation.M[1][1] = C + Temp.Y * Axis.Y;
    Rotation.M[1][2] = Temp.Y * Axis.Z + S * Axis.X;

    Rotation.M[2][0] = Temp.Z * Axis.X + S * Axis.Y;
    Rotation.M[2][1] = Temp.Z * Axis.Y - S * Axis.X;
    Rotation.M[2][2] = C + Temp.Z * Axis.Z;

    Result = Rotation * M;

    return Result;
}

Mat4 Transpose(Mat4 M) {
    Mat4 Result = {};

    Result.M[0][0] = M.M[0][0];
    Result.M[0][1] = M.M[1][0];
    Result.M[0][2] = M.M[2][0];
    Result.M[0][3] = M.M[3][0];
    Result.M[1][0] = M.M[0][1];
    Result.M[1][1] = M.M[1][1];
    Result.M[1][2] = M.M[2][1];
    Result.M[1][3] = M.M[3][1];
    Result.M[2][0] = M.M[0][2];
    Result.M[2][1] = M.M[1][2];
    Result.M[2][2] = M.M[2][2];
    Result.M[2][3] = M.M[3][2];
    Result.M[3][0] = M.M[0][3];
    Result.M[3][1] = M.M[1][3];
    Result.M[3][2] = M.M[2][3];
    Result.M[3][3] = M.M[3][3];

    return Result;
}

void PrintMat(Mat4 M) {
    Print("| %.2f, %.2f, %.2f, %.2f |\n", M.M[0][0], M.M[0][1], M.M[0][2], M.M[0][3]);
    Print("| %.2f, %.2f, %.2f, %.2f |\n", M.M[1][0], M.M[1][1], M.M[1][2], M.M[1][3]);
    Print("| %.2f, %.2f, %.2f, %.2f |\n", M.M[2][0], M.M[2][1], M.M[2][2], M.M[2][3]);
    Print("| %.2f, %.2f, %.2f, %.2f |\n\n", M.M[3][0], M.M[3][1], M.M[3][2], M.M[3][3]);
}


