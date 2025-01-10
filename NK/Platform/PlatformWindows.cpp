#include "Platform.h"

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#define _NO_CRT_STDIO_INLINE
#include <Windows.h>

#include "../General.h"

#define STB_SPRINTF_IMPLEMENTATION
#include "../ThirdParty/stb_sprintf.h"

struct WindowsState {
    u64 usResolution;
    b8 LargePagesEnabled;
};

global WindowsState PlatformState;

internal void InitPlatform() {
    LARGE_INTEGER LI = {};

    if (!QueryPerformanceFrequency(&LI)) {
        PrintLiteral("Failed QueryPerformanceFrequency\n");
        Exit(1);
    }

    PlatformState.usResolution = LI.QuadPart;
    PlatformState.LargePagesEnabled = b8(EnableLargePages());
}

b32 EnableLargePages() {
    b32 Result = 0;

    HANDLE Token;
    if (OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &Token)) {
        TOKEN_PRIVILEGES Privileges;
        Privileges.PrivilegeCount = 1;

        if (LookupPrivilegeValueA(0, SE_LOCK_MEMORY_NAME, &Privileges.Privileges[0].Luid)) {
            Privileges.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

            if (AdjustTokenPrivileges(Token, 0, &Privileges, sizeof(Privileges), 0, 0)) {
                Result = 1;
            }
        }

        CloseHandle(Token);
    }

    return Result;
}

u64 GetPageSize() {
    SYSTEM_INFO SystemInfo;
    GetSystemInfo(&SystemInfo);

    return u64(SystemInfo.dwPageSize);
}

u64 GetLargePageSize() {
    SIZE_T Size = GetLargePageMinimum();

    return u64(Size);
}

void *ReserveMemory(u64 Size) {
    return VirtualAlloc(0, Size, MEM_RESERVE, PAGE_READWRITE);
}

b32 CommitMemory(void *Pointer, u64 Size) {
    return VirtualAlloc(Pointer, Size, MEM_COMMIT, PAGE_READWRITE) != 0;
}

void *ReserveMemoryLarge(u64 Size) {
    return VirtualAlloc(0, Size, MEM_RESERVE | MEM_COMMIT | MEM_LARGE_PAGES, PAGE_READWRITE);
}

void *ReserveMemoryLargeIfPossible(u64 Size) {
    void *Result = 0;

    /*
    if (PlatformState.LargePagesEnabled) {
        Result = ReserveMemoryLarge(Size);
    } else {*/
        Result = ReserveMemory(Size);
        CommitMemory(Result, Size);
    // }

    return Result;
}

void DecommitMemory(void *Pointer, u64 Size) {
    VirtualFree(Pointer, Size, MEM_DECOMMIT);
}

void ReleaseMemory(void *Pointer, u64 Size) {
    VirtualFree(Pointer, 0, MEM_RELEASE);
}

void *HeapAlloc(u64 Size) {
    return HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, Size);
}

void HeapFree(void *Pointer) {
    HeapFree(GetProcessHeap(), 0, Pointer);
}

internal HANDLE GetStdOutHandle() {
    HANDLE Result = GetStdHandle(STD_OUTPUT_HANDLE);

    if (Result == INVALID_HANDLE_VALUE || Result == 0) {
        if (!AttachConsole(ATTACH_PARENT_PROCESS)) {
            AllocConsole();
        }

        Result = GetStdHandle(STD_OUTPUT_HANDLE);
    }

    return Result;
}

void Print(const char *Format, ...) {
    if (Format) {
        HANDLE Console = GetStdOutHandle();

        va_list Args;
        va_start(Args, Format);
        int Needed = stbsp_vsnprintf(0, 0, Format, Args);
        va_end(Args);

        if (Needed <= 0) {
            return;
        }

        char *Buffer = (char *) HeapAlloc(Needed + 1);

        va_start(Args, Format);
        stbsp_vsnprintf(Buffer, Needed + 1, Format, Args);
        va_end(Args);

        WriteConsoleA(Console, Buffer, Needed, 0, 0);

        HeapFree(Buffer);
    }
}

void PrintLiteral(const char *Literal) {
    if (Literal) {
        HANDLE Console = GetStdOutHandle();

        WriteConsoleA(Console, Literal, lstrlenA(Literal), 0, 0);
    }
}

void Exit(int Code) {
    ExitProcess(u32(Code));
}

OS_Handle OpenFile(String Path, OS_Flags Flags) {
    char *CPath = (char *) HeapAlloc(Path.Length + 1);
    CopyMemory(CPath, Path.Pointer, Path.Length);
    CPath[Path.Length] = 0;

    DWORD Access = 0;
    if (Flags & OS_READ)
        Access |= GENERIC_READ;
    if (Flags & OS_WRITE)
        Access |= GENERIC_WRITE;

    DWORD Shared = 0;
    if (Flags & OS_SHARED)
        Shared = FILE_SHARE_READ;

    SECURITY_ATTRIBUTES SecurityAttributes = {sizeof(SECURITY_ATTRIBUTES), 0, 0};

    DWORD CreationDisposition = 0;
    if (!(Flags & OS_CREATE)) {
        CreationDisposition = OPEN_EXISTING;
    }

    DWORD FlagsAndAttributes = 0;
    HANDLE TemplateFile = 0;
    HANDLE Result = CreateFileA(CPath,
                                Access,
                                Shared,
                                &SecurityAttributes,
                                CreationDisposition,
                                FlagsAndAttributes,
                                TemplateFile);

    HeapFree(CPath);

    return (OS_Handle) Result;
}

void CloseFile(OS_Handle File) {
    CloseHandle((HANDLE) File);
}

OS_FileInfo GetFileInfo(OS_Handle File) {
    OS_FileInfo Result = {};

    u32 HighBits = 0;
    u32 LowBits = GetFileSize((HANDLE) File, (DWORD *) &HighBits);

    Result.Size = u64(LowBits) | ((u64(HighBits)) << 32);

    return Result;
}

b32 IsValidFile(OS_Handle File) {
    return ((HANDLE) File) != INVALID_HANDLE_VALUE;
}

String ReadHandle(OS_Handle File, u64 Size, void *Memory) {
    String Result = {};

    LARGE_INTEGER LI = {};
    LI.QuadPart = 0;

    if (SetFilePointerEx((HANDLE) File, LI, 0, FILE_BEGIN)) {
        Result.Pointer = (u8 *) Memory;

        u8 *Pointer = Result.Pointer;
        u8 *End = Pointer + Size;

        for (;;) {
            u64 Unread = (u64) (End - Pointer);
            DWORD ToRead = (DWORD) ClampTop(Unread, MaxU32);
            DWORD Read = 0;
            if (!ReadFile((HANDLE) File, Pointer, ToRead, &Read, 0)) {
                break;
            }

            Pointer += Read;
            Result.Length += Read;

            if (Pointer >= End) {
                break;
            }
        }

        *End = 0;
    }

    return Result;
}

u64 GetTimeNowUs() {
    LARGE_INTEGER LI = {};

    if (QueryPerformanceCounter(&LI)) {
        return (LI.QuadPart * 1000000) / PlatformState.usResolution;
    }

    return 0;
}

void SleepMs(u32 ms) {
    Sleep(ms);
}

OS_Handle StartThread(ThreadFunc Function, void *Args) {
    DWORD ID;
    HANDLE Handle = CreateThread(0, 0, (LPTHREAD_START_ROUTINE) Function, Args, 0, &ID);

    return (OS_Handle) Handle;
}

void JoinThread(OS_Handle Thread) {
    HANDLE Handle = (HANDLE) Thread;

    WaitForSingleObject(Handle, INFINITE);
    CloseHandle(Handle);
}

OS_Handle CreateMutex() {
    return (OS_Handle) CreateMutex(0, FALSE, 0);
}

void DestroyMutex(OS_Handle Mutex) {
    CloseHandle((HANDLE) Mutex);
}

void AcquireMutex(OS_Handle Mutex) {
    WaitForSingleObject((HANDLE) Mutex, INFINITE);
}

void ReleaseMutex(OS_Handle Mutex) {
    ReleaseMutex((HANDLE) Mutex);
}

OS_Handle CreateSemaphore(s32 Value, s32 Max) {
    return (OS_Handle) CreateSemaphoreA(0, Value, Max, 0);
}

void DestroySemaphore(OS_Handle Semaphore) {
    CloseHandle((HANDLE) Semaphore);
}

b32 TakeSemaphore(OS_Handle Semaphore) {
    return WaitForSingleObject((HANDLE) Semaphore, INFINITE) == WAIT_OBJECT_0;
}

void DropSemaphore(OS_Handle Semaphore) {
    ReleaseSemaphore((HANDLE) Semaphore, 1, 0);
}

u32 AtomicIncrement(volatile u32 *Value) {
    return InterlockedIncrement(Value);
}

u32 AtomicCompareExchange(volatile u32 *Dest, u32 Value, u32 Comperand) {
    return InterlockedCompareExchange(Dest, Value, Comperand);
}

extern void NKMain();

void WinMainCRTStartup() {
    InitPlatform();

    NKMain();

    Exit(0);
}

void mainCRTStartup() {
    InitPlatform();

    NKMain();

    Exit(0);
}

extern "C" int _fltused = 0;
