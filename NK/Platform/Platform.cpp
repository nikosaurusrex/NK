#include "Platform.h"

#if ARCH_X64

#pragma function(memcpy)
void *memcpy(void *Dest, void const *Source, size_t Size) {
    __asm__ volatile (
        "rep movsb"
        : "=D"(Dest), "=S"(Source), "=c"(Size)
        : "D"(Dest), "S"(Source), "c"(Size)
        : "memory"
    );
    return Dest;
}

#pragma function(memset)
void *memset(void *Pointer, int Byte, size_t Size) {
    __asm__ volatile (
        "rep stosb"
        : "=D"(Pointer), "=a"(Byte), "=c"(Size)
        : "D"(Pointer), "a"(Byte), "c"(Size)
        : "memory"
    );
    return Pointer;
}

void _MoveMemory(u8 *Dest, u8 *Source, u64 Size) {
    if (Dest < Source) {
        __asm__ volatile (
            "rep movsb"
            : "=D"(Dest), "=S"(Source), "=c"(Size)
            : "D"(Dest), "S"(Source), "c"(Size)
            : "memory"
        );
    } else if (Dest > Source) {
        __asm__ volatile (
            "std\n"
            "rep movsb\n"
            "cld"
            : "=D"(Dest), "=S"(Source), "=c"(Size)
            : "D"(Dest + Size - 1), "S"(Source + Size - 1), "c"(Size)
            : "memory"
        );
    }
}

int _CompareMemory(u8 *A, u8 *B, u64 Size) {
    if (Size == 0) {
        return 1;
    }

    int Result = 1;

    __asm__ volatile (
        "repe cmpsb\n"
        "jne 1f\n"
        "movl $1, %[Result]\n"
        "jmp 2f\n"
    "1:\n"
        "movl $0, %[Result]\n"
    "2:\n"
        : [Result] "=r"(Result)
        : "D"(A), "S"(B), "c"(Size)
        : "cc", "memory"
    );

    return Result;
}

#else

#pragma function(memcpy)
void *memcpy(void *Dest, void const *Source, size_t Size) {
    for (u64 i = 0; i < Size; i++) {
        Dest[i] = Source[i];
    }
    return Dest;
}

#pragma function(memset)
void *memset(void *Pointer, int Byte, size_t Size) {
    for (u64 i = 0; i < Size; i++) {
        Pointer[i] = Byte;
    }
    return Pointer;
}

void _MoveMemory(u8 *Dest, u8 *Source, u64 Size) {
    if (Dest < Source) {
        for (u64 i = 0; i < Size; i++) {
            Dest[i] = Source[i];
        }
    } else {
        for (u64 i = Size; i > 0; i--) {
            Dest[i - 1] = Source[i - 1];
        }
    }
}

int _CompareMemory(u8 *A, u8 *B, u64 Size) {
    int Result = 1;

    for (u64 i = 0; i < Size; i++) {
        if (A[i] != B[i]) {
            Result = 0;
            break;
        }
    }

    return Result;
}

#endif

void _CopyMemory(u8 *Dest, u8 *Source, u64 Size) {
    memcpy(Dest, Source, Size);
}

void _SetMemory(u8 *Pointer, u8 Byte, u64 Size) {
    memset(Pointer, Byte, Size);
}
