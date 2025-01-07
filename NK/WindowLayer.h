#pragma once

#include "Platform/PlatformContext.h"
#include "Window/Window.h"

#ifdef NK_IMPLEMENTATION

#if OS_WINDOWS
#include "Window/WindowWindows.cpp"
#include "Window/Window.cpp"
#else
#error Unsupported platform layer.
#endif

#endif // NK_IMPLEMENTATION
