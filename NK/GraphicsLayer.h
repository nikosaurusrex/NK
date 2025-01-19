#pragma once

#include "Platform/PlatformContext.h"
#include "Graphics/Graphics.h"

#ifdef NK_IMPLEMENTATION

#if OS_WINDOWS
#include "Graphics/GraphicsWindows.cpp"
#else
#error Unsupported platform layer.
#endif

#endif // NK_IMPLEMENTATION
