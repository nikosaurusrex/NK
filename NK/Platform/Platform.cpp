#include "Platform.h"

#if ARCH_X64 && COMPILER_CLANG

void *memcpy(void *Dest, void const *Source, size_t Size) {
    __asm__ volatile (
        "rep movsb"
        : "=D"(Dest), "=S"(Source), "=c"(Size)
        : "D"(Dest), "S"(Source), "c"(Size)
        : "memory"
    );
    return Dest;
}

void *memset(void *Pointer, int Byte, size_t Size) {
    __asm__ volatile (
        "rep stosb"
        : "=D"(Pointer), "=a"(Byte), "=c"(Size)
        : "D"(Pointer), "a"(Byte), "c"(Size)
        : "memory"
    );
    return Pointer;
}

void *memmove(void *_Dest, const void *_Source, size_t Size) {
    u8 *Dest = (u8 *)_Dest;
    u8 *Source = (u8 *)_Source;

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
    return Dest;
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

void *memcpy(void *_Dest, void const *_Source, size_t Size) {
    u8 *Dest = (u8 *)_Dest;
    u8 *Source = (u8 *)_Source;
    for (u64 i = 0; i < Size; i++) {
        Dest[i] = Source[i];
    }
    return Dest;
}

void *memset(void *_Pointer, int Byte, size_t Size) {
    u8 *Pointer = (u8 *)_Pointer;
    for (u64 i = 0; i < Size; i++) {
        Pointer[i] = Byte;
    }
    return Pointer;
}

void *memmove(void *_Dest, const void *_Source, size_t Size) {
    u8 *Dest = (u8 *)_Dest;
    u8 *Source = (u8 *)_Source;
    if (Dest < Source) {
        for (u64 i = 0; i < Size; i++) {
            Dest[i] = Source[i];
        }
    } else {
        for (u64 i = Size; i > 0; i--) {
            Dest[i - 1] = Source[i - 1];
        }
    }

    return Dest;
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

void _MoveMemory(u8 *Dest, u8 *Source, u64 Size) {
    memmove(Dest, Source, Size);
}

String ReadFile(String Path) {
    OS_Handle Handle = OpenFile(Path, OS_READ | OS_SHARED);

    if (!IsValidFile(Handle)) {
        return String();
    }

    OS_FileInfo FileInfo = GetFileInfo(Handle);
    void *Memory = HeapAlloc(FileInfo.Size + 1);
    String contents = ReadHandle(Handle, FileInfo.Size, Memory);
    CloseFile(Handle);

    return contents;
}

internal u32 RunTaskQueue(void *Args) {
    TaskQueue *Queue = (TaskQueue *) Args;

    while (!Queue->Canceled) {
        u32 CurrentIndex = Queue->DequeueIndex;
        u32 NextIndex = (Queue->DequeueIndex + 1) % ArrayCount(Queue->Tasks);
        if (CurrentIndex != Queue->EnqueueIndex) {
            u32 Index = AtomicCompareExchange(&Queue->DequeueIndex, NextIndex, CurrentIndex);

            if (Index == CurrentIndex) {
                Task ToExecute = Queue->Tasks[Index];
                ToExecute.Function(Queue, ToExecute.Pointer);
            }
        } else {
            TakeSemaphore(Queue->Semaphore);
        }
    }

    return 0;
}

void CreateTaskQueue(TaskQueue *Queue, u32 ThreadCount) {
    Queue->Semaphore = CreateSemaphore(0, ThreadCount);
    Queue->EnqueueIndex = 0;
    Queue->DequeueIndex = 0;
    Queue->Canceled = false;

    for (u32 i = 0; i < ThreadCount; ++i) {
        StartThread(RunTaskQueue, Queue);
    }
}

void EnqueueTask(TaskQueue *Queue, TaskFunc Function, void *Pointer) {
    u32 Index = (Queue->EnqueueIndex + 1) % ArrayCount(Queue->Tasks);

    Task *New = Queue->Tasks + Index;
    New->Function = Function;
    New->Pointer = Pointer;

    // TODO: Write barrier
    Queue->EnqueueIndex = Index;

    DropSemaphore(Queue->Semaphore);
}

void CancelTaskQueue(TaskQueue *Queue) {
    Queue->Canceled = true;
}
