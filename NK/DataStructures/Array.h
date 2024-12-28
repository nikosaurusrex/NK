#pragma once

#include "../General.h"

template<typename T>
struct Array {
    T *Pointer;
    u64 Length;
    u64 Capacity;

    Array() {
        Pointer = 0;
        Length = 0;
        Capacity = 0;
    }

    Array(u64 ReserveCount) {
        Pointer = 0;
        Capacity = 0;
        Length = 0;

        Reserve(*this, ReserveCount);
    }

    T &operator[](u64 Index) {
        return Pointer[Index];
    }

    const T operator[](u64 Index) const {
        return Pointer[Index];
    }
};

template<typename T>
void Reserve(Array<T> &A, u64 Count);

template<typename T>
void Append(Array<T> &A, T Element);

template<typename T>
void UnorderedRemove(Array<T> &A, u64 Index);
template<typename T>
void OrderedRemove(Array<T> &A, u64 Index);

template<typename T>
T Pop(Array<T> &A);

template<typename T>
void Clear(Array<T> &A);
template<typename T>
void Free(Array<T> &A);
