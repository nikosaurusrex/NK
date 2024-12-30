#pragma once

#include "PlatformContext.h"

#include "../General.h"
#include "../DataStructures/String.h"

typedef u64 OS_Handle;
typedef u32 OS_Flags;
enum {
    OS_READ = (1 << 0),
    OS_WRITE = (1 << 1),
    OS_CREATE = (1 << 2),
    OS_SHARED = (1 << 3),
};

struct OS_FileInfo {
    u64 Size;
};

typedef void (*ThreadFunc)(void *Args);

struct TaskQueue;
typedef void (*TaskFunc)(TaskQueue *Queue, void *Pointer);

struct Task {
    TaskFunc Function;
    void *Pointer;
};

struct TaskQueue {
    OS_Handle Semaphore;
    volatile u32 EnqueueIndex;
    volatile u32 DequeueIndex;
    volatile b32 Canceled;

    Task Tasks[256];
};

// Memory
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

// Print
void Print(const char *Format, ...);
void PrintLiteral(const char *Literal);

// Exit
void Exit(int Code);

// File Management
OS_Handle OpenFile(String Path, OS_Flags Flags);
void CloseFile(OS_Handle File);

OS_FileInfo GetFileInfo(OS_Handle File);
b32 IsValidFile(OS_Handle File);

String ReadHandle(OS_Handle File, u64 Size, void *Memory);

// Time
u64 GetTimeNowUs();
void SleepMs(u32 ms);

// Threading
OS_Handle StartThread(ThreadFunc Function, void *Args);
void JoinThread(OS_Handle Thread);

// Mutex
OS_Handle CreateMutex();
void DestroyMutex(OS_Handle Mutex);
void AcquireMutex(OS_Handle Mutex);
void ReleaseMutex(OS_Handle Mutex);

// Semaphores
OS_Handle CreateSemaphore(s32 Value, s32 Max);
void DestroySemaphore(OS_Handle Semaphore);
b32 TakeSemaphore(OS_Handle Semaphore);
void DropSemaphore(OS_Handle Semaphore);

// Atomic Operations
u32 AtomicIncrement(volatile u32 *Value);
u32 AtomicCompareExchange(volatile u32 *Dest, u32 Value, u32 Comperand);

// Defined in Platform.cpp - not OS specific
void _CopyMemory(u8 *Dest, u8 *Source, u64 Size);
void _MoveMemory(u8 *Dest, u8 *Source, u64 Size);
void _SetMemory(u8 *Pointer, u8 Byte, u64 Size);
int _CompareMemory(u8 *A, u8 *B, u64 Size);

#define CopyMemory(Dest, Source, Size) _CopyMemory((u8 *)(Dest), (u8 *)(Source), (Size))
#define MoveMemory(Dest, Source, Size) _MoveMemory((u8 *)(Dest), (u8 *)(Source), (Size))
#define SetMemory(Pointer, Byte, Size) _SetMemory((u8 *)(Pointer), (u8)(Byte), (Size))
#define CompareMemory(A, B, Size) _CompareMemory((u8 *)(A), (u8 *)(B), (Size))
#define ZeroMemory(Pointer, Size) _SetMemory((u8 *)(Pointer), 0, (Size))

String ReadFile(String Path);

void CreateTaskQueue(TaskQueue *Queue, u32 ThreadCount);
void EnqueueTask(TaskQueue *Queue, TaskFunc Function, void *Pointer);
void CancelTaskQueue(TaskQueue *Queue);
