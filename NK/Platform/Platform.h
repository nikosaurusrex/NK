#pragma once

#include "PlatformContext.h"

#include "../General.h"

b32 EnableLargePages();
u64 GetPageSize();
u64 GetLargePageSize();

void *ReserveMemory(u64 Size);
b32 CommitMemory(void *Pointer, u64 Size);
void *ReserveMemoryLarge(u64 Size);
void *ReserveMemoryLargeIfPossible(u64 Size);
void DecommitMemory(void *Pointer, u64 Size);
void ReleaseMemory(void *Pointer, u64 Size);

void *HeapAlloc(u64 Size);
void HeapFree(void *Pointer);

void Print(const char *Format, ...);
void PrintLiteral(const char *Literal);

void Exit(int Code);

// Defined in Platform.cpp - not really OS specific
void _CopyMemory(u8 *Dest, u8 *Source, u64 Size);
void _MoveMemory(u8 *Dest, u8 *Source, u64 Size);
void _SetMemory(u8 *Pointer, u8 Byte, u64 Size);
int _CompareMemory(u8 *A, u8 *B, u64 Size);

#define CopyMemory(Dest, Source, Size) _CopyMemory((u8 *)(Dest), (u8 *)(Source), (Size))
#define MoveMemory(Dest, Source, Size) _MoveMemory((u8 *)(Dest), (u8 *)(Source), (Size))
#define SetMemory(Pointer, Byte, Size) _SetMemory((u8 *)(Pointer), (u8)(Byte), (Size))
#define CompareMemory(A, B, Size) _CompareMemory((u8 *)(A), (u8 *)(B), (Size))
#define ZeroMemory(Pointer, Size) _SetMemory((u8 *)(Pointer), 0, (Size))
