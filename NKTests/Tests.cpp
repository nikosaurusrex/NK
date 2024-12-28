#define NK_IMPLEMENTATION
#include "NK/PlatformLayer.h"
#include "NK/DataStructuresLayer.h"

#define TEST_NAME Platform
#include "PlatformTests.cpp"
#undef TEST_NAME
#define TEST_NAME Array
#include "ArrayTests.cpp"

void NKMain() {
    RunAllTestsPlatform();
    RunAllTestsArray();
}
