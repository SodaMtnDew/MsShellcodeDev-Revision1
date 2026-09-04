// ShellcodePrototypeRev1.cpp : Defines the entry point for the application.
//

#include "framework.h"
#include "..\Common\ScFunc.h"
extern "C" LPBYTE _scMain();
#pragma alloc_text(".text", _scMain)

LPBYTE _scMain()
{
	DWORD pdwArray[27] =
	{
		0x72657375, 0x642e3233, 0x63006c6c, 0x2e636c61,
		0x00657865, 0x00720050, 0x0063006f, 0x00730065,
		0x00200073, 0x0061006c, 0x006e0075, 0x00680063,
		0x00640065, 0x00730020, 0x00630075, 0x00650063,
		0x00730073, 0x00750066, 0x006c006c, 0x002c0079,
		0x00500020, 0x00440049, 0x0025003d, 0x00000064,
		0x00690048, 0x0074006e, 0x00000000
	};
	LPBYTE pByteDword = (LPBYTE)pdwArray;
	HMODULE mod_kernel32_dll = GetModuleByHash(0x63d37afa);
	LPSTR psz_user32_dll = (LPSTR)pByteDword;
	pByteDword += 11;
	HMODULE mod_user32_dll = GetModuleByName(psz_user32_dll);
	decltype(&CreateProcessA) fn_CreateProcessA = (decltype(&CreateProcessA))
		GetFuncAddrByHashOrd(mod_kernel32_dll, 0x2e287319, 0);
	decltype(&wsprintfW) fn_wsprintfW = (decltype(&wsprintfW))
		GetFuncAddrByHashOrd(mod_user32_dll, 0x13ddbe62, 0);
	decltype(&MessageBoxW) fn_MessageBoxW = (decltype(&MessageBoxW))
		GetFuncAddrByHashOrd(mod_user32_dll, 0xf217fcb1, 0);
	LPSTR pszTmp0001 = (LPSTR)pByteDword;
	pByteDword += 9;
	LPWSTR pwszTmp0001 = (LPWSTR)pByteDword;
	pByteDword += 76;
	LPWSTR pwszTmp0002 = (LPWSTR)pByteDword;
	pByteDword += 10;

	STARTUPINFOA siA;
	PROCESS_INFORMATION pi;
	WCHAR pwszMsg[100];
	__stosb((LPBYTE)&siA, 0, sizeof(STARTUPINFOA));
	__stosb((LPBYTE)&pi, 0, sizeof(PROCESS_INFORMATION));
	/*/kernel32.dll/*/fn_CreateProcessA(NULL, pszTmp0001, NULL, NULL, FALSE, NULL, NULL, NULL, &siA, &pi);
	/*/user32.dll/*/fn_wsprintfW(pwszMsg, pwszTmp0001, pi.dwProcessId);
	/*/user32.dll/*/fn_MessageBoxW(NULL, pwszMsg, pwszTmp0002, MB_OK);
    return NULL;
}

#include "..\Common\ScFunc.cpp"