#include "../General.h"

#define STBTT_ifloor(x) IFloor(x)
#define STBTT_iceil(x) ICeil(x)
#define STBTT_sqrt(x) SquareRoot(x)
#define STBTT_pow(x, y) Power(x, y)
#define STBTT_fmod(x, y) FMod(x, y)
#define STBTT_cos(x) Cos(x)
#define STBTT_acos(x) ACos(x)
#define STBTT_fabs(x) AbsoluteValue(x)
#define STBTT_malloc(x, u) ((void)(u),HeapAlloc(x))
#define STBTT_free(x, u) ((void)(u),HeapFree(x))
#define STBTT_assert(x) Assert(x)
#define STBTT_strlen(x) CStringLength(x)
#define STBTT_memcpy CopyMemory
#define STBTT_memset SetMemory

IGNORE_WARNINGS_BEGIN
#define STB_TRUETYPE_IMPLEMENTATION
#include "../ThirdParty/stb_truetype.h"
