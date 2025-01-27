#pragma once

#include "UI/UI.h"

#ifdef NK_IMPLEMENTATION


#if OS_WINDOWS
#include "UI/UIWindows.cpp"
#else
#error UI Rendering not implemented for OS
#endif

#include "UI/UI.cpp"

#endif // NK_IMPLEMENTATION
