#include "NK/Test/Test.h"

TestGroup(Platform) {
    Test(Print Literal) {
        PrintLiteral("This is a literal\n");
    }
    Test(Formatted Printing) {
        Print("This is a formatted String: %d + %d = %d\n", 2, 4, 2 + 4);
    }
}
