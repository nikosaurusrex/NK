#pragma once

#include "PlatformContext.h"

#include "../General.h"

void *ReserveMemory(u64 Size);
b32 CommitMemory(void *Pointer, u64 Size);
void DecommitMemory(void *Pointer, u64 Size);
void ReleaseMemory(void *Pointer, u64 Size);

void *HeapAlloc(u64 Size);
void HeapFree(void *Pointer);

void Print(const char *Format, ...);
void PrintLiteral(const char *Literal);

void Exit(int Code);

/*
#define CopyMemory(dst, src, size) memcpy((u8 *)(dst), (u8 *)(src), (size))
#define MoveMemory(dst, src, size) memmove((u8 *)(dst), (u8 *)(src), (size))
#define SetMemory(dst, byte, size) memset((u8 *)(dst), (u8 *)(byte), (size))
#define CompareMemory(a, b, size) memcmp((u8 *)(a), (b), (size))
#define ZeroMemory(s, z) memset((s), 0, (z))
*/
