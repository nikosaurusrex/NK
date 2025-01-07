Quat Invert(Quat Q) {
    Quat P = {};

    P.W = Q.W;
    P.X = -Q.X;
    P.Y = -Q.Y;
    P.Z = -Q.Z;

    return P;
}

Quat operator*(Quat Q, Quat P) {
    Quat Result = {};

    Result.W = Q.W * P.W - Dot(P.V, Q.V);
    Result.V = Q.W * P.V + P.W * Q.V + Cross(Q.V, P.V);

    return Result;
}

Vec3 operator*(Quat Q, Vec3 V) {
    Vec3 qvXv = Cross(Q.V, V);

    return V + qvXv * (Q.W * 2.0f) + Cross(Q.V, qvXv) * 2.0f;
}
