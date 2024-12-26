#include "../General.h"
#include "../PlatformLayer.h"

#ifndef MAX_ASSERTS_PER_TEST
#define MAX_ASSERTS_PER_TEST 32
#endif

#ifndef MAX_TESTS
#define MAX_TESTS 128
#endif

#ifndef TEST_NAME
#define TEST_NAME
#endif

#define __Func NK_STRING_JOIN(__Test__Group_Func, TEST_NAME)
#define __Group NK_STRING_JOIN(__Group, TEST_NAME)
#define __Tag NK_STRING_JOIN(__Test__Group_Tag, TEST_NAME)
#define __PrintResults NK_STRING_JOIN(__Test__Print_Results, TEST_NAME)

#ifndef TEST_H
#define TEST_H
struct Test {
    const char *Tag;
    u32 Count;
    b8 Results[MAX_ASSERTS_PER_TEST];
    const char *Exprs[MAX_ASSERTS_PER_TEST];
};

struct TestGroup {
    u32 Count;
    Test Tests[MAX_TESTS];
};
#endif

TestGroup __Group = {};

#define TestGroup(Name)                                                                            \
    const char *__Tag = #Name;                                                                     \
    void __Func()

#define Test(Name)                                                                                 \
    {                                                                                              \
        Test *__T = &__Group.Tests[__Group.Count++];                                               \
        __T->Tag = #Name;                                                                          \
        if (__T->Count >= MAX_TESTS) {                                                             \
            Print("Ran out of max tests: %s:%d", __FILE__, __LINE__);                              \
            Exit(1);                                                                               \
        }                                                                                          \
    }

#undef Assert
#define Assert(Condition)                                                                          \
    {                                                                                              \
        do {                                                                                       \
            Test *__T = &__Group.Tests[__Group.Count - 1];                                         \
            __T->Results[__t->Count] = Condition;                                                  \
            __T->Exprs[__t->Count] = #Condition;                                                   \
            __T->Count++;                                                                          \
            if (__T->Count >= MAX_ASSERTS_PER_TEST) {                                              \
                Print("Ran out of max asserts per test: %s:%d", __FILE__, __LINE__);               \
                Exit(1);                                                                           \
            }                                                                                      \
        } while (0);                                                                               \
    }

extern const char *__Tag;
extern void __Func();
inline void __PrintResults() {
    int TotalFails = 0;

    Print("\x1B[1m%s\x1B[0m\n", __Tag);

    for (u32 i = 0; i < __Group.Count; ++i) {
        Test *__T = &__Group.Tests[i];

        const char *Result = "\x1B[32mPASS\x1B[0m";

        b8 Failed = 0;
        for (u32 j = 0; j < __T->Count; ++j) {
            if (!__T->Results[j]) {
                Result = "\x1B[31mFAIL\x1B[0m";
                Failed = 1;
                TotalFails++;
                break;
            }
        }

        Print("  %s %s\n", __T->Tag, Result);

        if (Failed) {
            for (u32 j = 0; j < __T->Count; ++j) {
                if (!__T->Results[j]) {
                    Print("    Condition \x1B[35m%s\x1B[0m failed.\n", __T->Exprs[j]);
                }
            }
        }
    }

    if (TotalFails > 0) {
        Print("\n\x1B[31m%d Tests Failed\x1B[0m\n\n", TotalFails);
    } else {
        PrintLiteral("\n\x1B[32mAll Tests Passed!\x1B[0m\n");
    }
}

inline void NK_STRING_JOIN(RunAllTests, TEST_NAME)() {
    __Func();
    __PrintResults();
}
