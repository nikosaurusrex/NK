#pragma once

#include "../General.h"

struct Arena {
    u64 Size;
    u64 Top;
    u8 *Pointer;
};

struct TempArena {
    Arena *Owning;
    u64 SavedTop;
};

Arena CreateArena(u64 Size);
void FreeArena(Arena *A, u64 Size);
Arena CreateSubArena(Arena *Parent, u64 Size);

TempArena BeginTempArena(Arena *Owning);
void EndTempArena(TempArena TA);

#define PushArray(A, Ty, N, ...) (ty *) PushSize(A, N * sizeof(Ty), ##__VA_ARGS__)
#define PushStruct(A, Ty, ...) (ty *) PushSize(A, sizeof(Ty), ##__VA_ARGS__)
u8 *PushSize(Arena *A, u64 Size, u64 Align = 4);
