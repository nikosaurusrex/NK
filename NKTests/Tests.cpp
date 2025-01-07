#define NK_IMPLEMENTATION
#include "NK/PlatformLayer.h"
#include "NK/DataStructuresLayer.h"
#include "NK/MathLayer.h"

#define TEST_NAME Platform
#include "PlatformTests.cpp"
#undef TEST_NAME
#define TEST_NAME Array
#include "ArrayTests.cpp"
#undef TEST_NAME
#define TEST_NAME Math
#include "MathTests.cpp"

void NKMain() {
    RunAllTestsPlatform();
    RunAllTestsArray();
    RunAllTestsMath();
}
