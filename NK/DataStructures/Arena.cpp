Arena CreateArena(u64 Size) {
    Arena Result = {};

    Size = AlignPow2(Size, GetLargePageSize());

    Result.Size = Size;
    Result.Top = 0;
    Result.Pointer = (u8 *)ReserveMemoryLargeIfPossible(Size);

    return Result;
}

void FreeArena(Arena *A) {
    ReleaseMemory(A->Pointer, A->Size);
}

Arena CreateSubArena(Arena *Parent, u64 Size) {
    Arena Result = {};

    Result.Size = Size;
    Result.Top = 0;
    Result.Pointer = PushSize(Parent, Size);

    return Result;
}

void ResetArena(Arena *A) {
    A->Top = 0;
}

TempArena BeginTempArena(Arena *Owning) {
    TempArena Result = {};

    Result.Owning = Owning;
    Result.SavedTop = Owning->Top;

    return Result;
}

void EndTempArena(TempArena TA) {
    TA.Owning->Top = TA.SavedTop;
}

u8 *PushSize(Arena *A, u64 Size, u64 Align) {
    u8 *Result = 0;

    u64 Base = (u64) (A->Pointer + A->Top);
    u64 Offset = 0;

    u64 Mask = Align - 1;
    if (Base & Mask) {
        Offset = Align - (Base & Mask);
    }

    Size += Offset;

    Result = (u8 *) (Base + Offset);

    A->Top += Size;

    return Result;
}
