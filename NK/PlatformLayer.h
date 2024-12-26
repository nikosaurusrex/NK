#pragma once

#include "Platform/PlatformContext.h"

#include "Platform/Platform.h"

#ifdef NK_IMPLEMENTATION

#if OS_WINDOWS
#include "Platform/PlatformWindows.cpp"
#include "Platform/Platform.cpp"
#else
#error Unsupported platform layer.
#endif

#endif // NK_IMPLENENTATION
