#include "Platform.h"

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#define _NO_CRT_STDIO_INLINE
#include <Windows.h>

#include "../General.h"

#define STB_SPRINTF_IMPLEMENTATION
#include "../ThirdParty/stb_sprintf.h"

#pragma comment (lib, "kernel32")
#pragma comment (lib, "user32")

void *ReserveMemory(u64 Size) {
    return VirtualAlloc(0, Size, MEM_RESERVE, PAGE_READWRITE);
}

b32 CommitMemory(void *Pointer, u64 Size) {
    return VirtualAlloc(Pointer, Size, MEM_COMMIT, PAGE_READWRITE) != 0;
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

extern void NKMain();

void WinMainCRTStartup() {
    NKMain();

    Exit(0);
}

void mainCRTStartup() {
    NKMain();

    Exit(0);
}

extern "C" int _fltused = 0;
