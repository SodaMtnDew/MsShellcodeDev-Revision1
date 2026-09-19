#pragma once

#include <shlwapi.h>

#ifdef RtlMoveMemory
#undef RtlMoveMemory
extern "C" void NTAPI RtlMoveMemory(void*, const void*, size_t);
#endif 

#ifdef RtlZeroMemory
#undef RtlZeroMemory
extern "C" void NTAPI RtlZeroMemory(void*, size_t);
#endif