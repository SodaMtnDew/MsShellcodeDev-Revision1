// header.h : include file for standard system include files,
// or project specific include files
//

#pragma once

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files
#include <windows.h>
#include <intrin.h>
// Defining ERROR NUMBER USED in COMPILER
#define ERSC_NOERROR	0x0000
#define ERSC_OLDMSDEV	0x0001
#define ERSC_NOMSDEV	0x0002
#define ERSC_NOMSVC		0x0003
#define ERSC_NOSDK		0x0004
#define ERSC_BADVCXPROJ	0x0005
#define ERSC_HAVINGRSRC	0x0006
#define ERSC_USEGLOBAL	0x0007
#define ERSC_BADCPP		0x0008