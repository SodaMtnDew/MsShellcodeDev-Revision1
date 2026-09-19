#pragma once

#ifndef NTAPI
#define NTAPI WINAPI
#endif

#ifdef RtlZeroMemory
#undef RtlZeroMemory
extern "C" void NTAPI RtlZeroMemory(void*, size_t);
#endif

extern LPSTR GenKeyHexHmac(HANDLE, LPBYTE, DWORD, LPBYTE, DWORD);
extern DWORD GetDriveSN(HANDLE, WCHAR, LPDWORD, LPSTR *);