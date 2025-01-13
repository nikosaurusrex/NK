template<typename T>
void Reserve(Array<T> &A, u64 Count) {
    if (Count == 0) {
        Count = 4;
    }

    if (Count <= A.Capacity) {
        return;
    }

    T *NewPointer = (T *)HeapAlloc(Count * sizeof(T));

    if (A.Pointer) {
        CopyMemory(NewPointer, A.Pointer, A.Length * sizeof(T));
        HeapFree(A.Pointer);
    }

    A.Pointer = NewPointer;
    A.Capacity = Count;
}

template<typename T>
void Append(Array<T> &A, T Element) {
    if (A.Length + 1 >= A.Capacity) {
        Reserve(A, A.Capacity * 2);
    }

    A.Pointer[A.Length] = Element;
    A.Length++;
}

template<typename T>
void UnorderedRemove(Array<T> &A, u64 Index) {
    T Last = Pop(A);

    if (Index < A.Length) {
        A.Pointer[Index] = Last;
    }
}

template<typename T>
void OrderedRemove(Array<T> &A, u64 Index) {
    MoveMemory(A.Pointer + Index, A.Pointer + Index + 1, ((A.Length - Index) - 1) * sizeof(T));

    A.Length--;
}

template<typename T>
T Pop(Array<T> &A) {
    T Result = A.Pointer[A.Length - 1];

    A.Length--;

    return Result;
}

template<typename T>
void Clear(Array<T> &A) {
    A.Length = 0;
}

template<typename T>
void Free(Array<T> &A) {
    A.Length = 0;
    A.Capacity = 0;

    if (A.Pointer) {
        HeapFree(A.Pointer);
        A.Pointer = 0;
    }
}
