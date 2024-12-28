#include "NK/Test/Test.h"

TestGroup(Array) {
    Test(Reserve Capacity) {
        Array<int> A(10);
        Assert(A.Capacity == 10);
        Assert(A.Length == 0);
    }
    Test(Add IncreasesLength) {
        Array<int> A = {};
        Append(A, 42);
        Assert(A.Length == 1);
        Assert(A[0] == 42);
        Free(A);
    }
    Test(UnorderedRemove RemovesElement) {
        int Temp[] = {1, 2, 3};

        Array<int> A = {};
        A.Capacity = 3;
        A.Length = 3;
        A.Pointer = (int *)&Temp;
        UnorderedRemove(A, 1);
        Assert(A.Length == 2);
        Assert(A[0] == 1);
        Assert(A[1] == 3);
    }
    Test(OrderedRemove ShiftsElements) {
        int Temp[] = {1, 2, 3, 4};

        Array<int> A = {};
        A.Capacity = 4;
        A.Length = 4;
        A.Pointer = (int *)&Temp;
        OrderedRemove(A, 1);
        Assert(A.Length == 3);
        Assert(A[0] == 1);
        Assert(A[1] == 3);
        Assert(A[2] == 4);
    }
    Test(Clear EmptiesArray) {
        int Temp[] = {1, 2, 3, 4, 5};

        Array<int> A = {};
        A.Capacity = 10;
        A.Length = 5;
        A.Pointer = (int *)&Temp;
        Clear(A);
        Assert(A.Length == 0);
    }
    Test(Pop RemovesAndReturnsLastElement) {
        int Temp[] = {10, 20, 30};

        Array<int> A = {};
        A.Capacity = 3;
        A.Length = 3;
        A.Pointer = (int *)&Temp;
        int Last = Pop(A);
        Assert(Last == 30);
        Assert(A.Length == 2);
    }
    Test(IndexOperatorAccess) {
        int Temp[] = {5, 10, 15};

        Array<int> A = {};
        A.Capacity = 3;
        A.Length = 3;
        A.Pointer = (int *)&Temp;
        Assert(A[0] == 5);
        Assert(A[1] == 10);
        Assert(A[2] == 15);
        A[1] = 20;
        Assert(A[1] == 20);
    }
    Test(Add ResizesWhenCapacityExceeded) {
        Array<int> A = {};
        Append(A, 3);
        Append(A, 5);
        Append(A, 1);
        Append(A, 2);

        Assert(A[0] == 3);
        Assert(A[1] == 5);
        Assert(A[2] == 1);
        Assert(A[3] == 2);
        Assert(A.Length == 4);

        Free(A);
   }
}
