#pragma once

#include <wincrypt.h>
#include <WinSock2.h>

#pragma pack(push, 1)
typedef union
{
	CHAR chVal[4];
	DWORD dwVal;
} TYPE_NET, * LPTYPE_NET;

typedef struct
{
	DWORD dwMagic[4], dwDummy[4];
} PACKET_HEADER;
#pragma pack(pop)

extern "C" BOOL    SendInfo(HANDLE, LPSTR, LPBYTE, DWORD);