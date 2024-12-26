#include "NK/Test/Test.h"

TestGroup(Platform) {
    Test(Print Literal) {
        PrintLiteral("This is a literal\n");

        Assert(1);
    }
    Test(Formatted Printing) {
        Print("This is a formatted String: %d + %d = %d\n", 2, 4, 2 + 4);

        Assert(1);
    }
    Test(CopyMemory Basic) {
        u8 Source[] = {1, 2, 3, 4, 5};
        u8 Dest[5] = {0};
        CopyMemory(Dest, Source, 5);
        for (u64 i = 0; i < 5; i++) {
            Assert(Dest[i] == Source[i]);
        }
    }
    Test(CopyMemory Empty) {
        u8 Source[] = {};
        u8 Dest[0] = {};
        CopyMemory(Dest, Source, 0);
        Assert(1);
    }
    Test(MoveMemory NonOverlapping) {
        u8 Source[] = {10, 20, 30, 40, 50};
        u8 Dest[5] = {0};
        MoveMemory(Dest, Source, 5);
        for (u64 i = 0; i < 5; i++) {
            Assert(Dest[i] == Source[i]);
        }
    }
    Test(MoveMemory Overlapping_Forward) {
        u8 Buffer[] = {1, 2, 3, 4, 5};
        MoveMemory(&Buffer[1], Buffer, 4);
        Assert(Buffer[1] == 1);
        Assert(Buffer[2] == 2);
        Assert(Buffer[3] == 3);
        Assert(Buffer[4] == 4);
    }
    Test(MoveMemory Overlapping_Backward) {
        u8 Buffer[] = {1, 2, 3, 4, 5};
        MoveMemory(Buffer, &Buffer[1], 4);
        Assert(Buffer[0] == 2);
        Assert(Buffer[1] == 3);
        Assert(Buffer[2] == 4);
        Assert(Buffer[3] == 5);
    }
    Test(SetMemory Basic) {
        u8 Buffer[5] = {0};
        SetMemory(Buffer, 42, 5);
        for (u64 i = 0; i < 5; i++) {
            Assert(Buffer[i] == 42);
        }
    }
    Test(SetMemory Empty) {
        u8 Buffer[0] = {};
        SetMemory(Buffer, 42, 0);
        Assert(1);
    }
    Test(CompareMemory Equal) {
        u8 BufferA[] = {1, 2, 3};
        u8 BufferB[] = {1, 2, 3};
        int Result = CompareMemory(BufferA, BufferB, 3);
        Assert(Result == 1);
    }
    Test(CompareMemory NotEqual) {
        u8 BufferA[] = {1, 2, 3};
        u8 BufferB[] = {1, 2, 4};
        int Result = CompareMemory(BufferA, BufferB, 3);
        Assert(Result == 0);
    }
    Test(CompareMemory Empty) {
        u8 BufferA[] = {};
        u8 BufferB[] = {};
        int Result = CompareMemory(BufferA, BufferB, 0);
        Assert(Result == 1);
    }
}
