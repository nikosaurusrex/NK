#pragma once

#include "../General.h"

inline u64 CStringLength(const char *CStr) {
    u64 Length = 0;

    if (CStr) {
        while (CStr[Length] != 0) {
            Length++;
        }
    }

    return Length;
}

struct String {
    u8 *Pointer;
    u64 Length;

    String() {
        Pointer = 0;
        Length = 0;
    }

    String(const char *CStr) {
        Pointer = (u8 *) CStr;
        Length = CStringLength(CStr);
    }

    String(u8 *_Pointer, u64 _Length) {
        Pointer = _Pointer;
        Length = _Length;
    }

    u8 operator[](u64 Index) const {
        Assert(Index < Length);
        return Pointer[Index];
    }

    u8 &operator[](u64 Index) {
        Assert(Index < Length);
        return Pointer[Index];
    }

    b8 operator==(const char *CStr) {
        if ((const char *) Pointer == CStr)
            return 1;
        if (!Pointer)
            return 0;
        if (!CStr)
            return 0;

        u64 CStrLength = CStringLength(CStr);
        if (CStrLength != Length)
            return 0;

        for (u64 i = 0; i < Length; ++i) {
            if (Pointer[i] != CStr[i]) {
                return 0;
            }
        }

        return 1;
    }

    b8 operator!=(const char *CStr) {
        return !operator==(CStr);
    }

    b8 operator==(String Other) {
        if (Other.Pointer == Pointer)
            return 1;
        if (!Pointer)
            return 0;
        if (!Other.Pointer)
            return 0;

        if (Other.Length != Length)
            return 0;

        for (u64 i = 0; i < Length; ++i) {
            if (Pointer[i] != Other.Pointer[i]) {
                return 0;
            }
        }

        return 1;
    }

    b8 operator!=(String Str) {
        return !operator==(Str);
    }
};
