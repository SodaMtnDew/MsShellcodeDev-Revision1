#include "framework.h"
#include "GetSysInfo.h"
#include <shlwapi.h>
#include <LM.h>

#pragma comment(lib, "shlwapi")
#pragma comment(lib, "netapi32")
#pragma warning(disable: 4996)

LPWSTR GetSysInfo(HANDLE hHeap)
{
	WCHAR pwszStr[MAX_PATH];
	LPWSTR pwszRet;
	DWORD dwBufLen;
	LPSTREAM pStream = /*/shlwapi.dll#12/*/SHCreateMemStream(0, 0);
	/*/shlwapi.dll#212/*/IStream_Write(pStream, L"Computer Name: ", 30);
	dwBufLen = MAX_PATH;
	/*/kernel32.dll/*/GetComputerNameW(pwszStr, &dwBufLen);
	/*/shlwapi.dll#212/*/IStream_Write(pStream, pwszStr, dwBufLen * sizeof(WCHAR));
	NETSETUP_JOIN_STATUS dwJoinType;
	pwszRet = NULL;
	/*/netapi32.dll/*/NetGetJoinInformation(NULL, &pwszRet, &dwJoinType);
	if (dwJoinType >= NetSetupWorkgroupName)
	{
		if (dwJoinType == NetSetupWorkgroupName)
			/*/shlwapi.dll#212/*/IStream_Write(pStream, L"\r\nWorkgroup Name: ", 36);
		else
			/*/shlwapi.dll#212/*/IStream_Write(pStream, L"\r\nDomain Name: ", 30);
		dwBufLen = /*/kernel32.dll/*/lstrlenW(pwszRet);
		/*/shlwapi.dll#212/*/IStream_Write(pStream, pwszRet, dwBufLen * sizeof(WCHAR));
	}
	/*/netapi32.dll/*/NetApiBufferFree(pwszRet);
	pwszRet = NULL;
	dwBufLen = MAX_PATH;
	/*/advapi32.dll/*/GetUserNameW(pwszStr, &dwBufLen);
	/*/shlwapi.dll#212/*/IStream_Write(pStream, L"\r\nUser Name: ", 26);
	/*/shlwapi.dll#212/*/IStream_Write(pStream, pwszStr, dwBufLen * sizeof(WCHAR) - 2);
	UINT codePage = /*/kernel32.dll/*/GetACP();
	/*/user32.dll/*/wsprintfW(pwszStr, L"\r\nCodepage: %d", codePage);
	dwBufLen = /*/kernel32.dll/*/lstrlenW(pwszStr);
	/*/shlwapi.dll#212/*/IStream_Write(pStream, pwszStr, dwBufLen * sizeof(WCHAR));
	pwszRet = pwszStr + /*/kernel32.dll/*/GetSystemWindowsDirectoryW(pwszStr, MAX_PATH);
	/*/shlwapi.dll#212/*/IStream_Write(pStream, L"\r\nOS Arch: ", 22);
	/*/kernel32.dll/*/lstrcpyW(pwszRet, L"\\SyChpe32"); 
	if(/*/shlwapi.dll/*/PathFileExistsW(pwszStr))
		/*/shlwapi.dll#212/*/IStream_Write(pStream, L"arm64", 10);
	else
	{
		/*/kernel32.dll/*/lstrcpyW(pwszRet, L"\\SysWOW64");
		if (/*/shlwapi.dll/*/PathFileExistsW(pwszStr))
			/*/shlwapi.dll#212/*/IStream_Write(pStream, L"x64", 6);
		else
			/*/shlwapi.dll#212/*/IStream_Write(pStream, L"x86", 6);
	}
	pwszRet = NULL;
	OSVERSIONINFOEXW osVerInfo;
	osVerInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEXW);
	/*/kernel32.dll/*/GetVersionExW((LPOSVERSIONINFOW)&osVerInfo);
	/*/shlwapi.dll#212/*/IStream_Write(pStream, L"\r\nOS Version:\r\n\tWindows ", 48);
	LPBYTE pVerVonPEB =
#if defined(_M_X64)
		(LPBYTE)__readgsqword(0x60) + 0x0118;
#else
		(LPBYTE)__readfsdword(0x30) + 0xA4;
#endif
	switch (osVerInfo.wProductType)
	{
	case VER_NT_WORKSTATION:
		/*/shlwapi.dll#212/*/IStream_Write(pStream, L"Workstation\r\n\t", 28);
		break;
	case VER_NT_DOMAIN_CONTROLLER:
		/*/shlwapi.dll#212/*/IStream_Write(pStream, L"Domain Center\r\n\t", 32);
		break;
	case VER_NT_SERVER:
		/*/shlwapi.dll#212/*/IStream_Write(pStream, L"Server\r\n\t", 18);
		break;
	}
	osVerInfo.dwMajorVersion = *((LPDWORD)pVerVonPEB);
	osVerInfo.dwMinorVersion = *((LPDWORD)(pVerVonPEB + 4));
	osVerInfo.dwBuildNumber = *((LPWORD)(pVerVonPEB + 8));
	/*/user32.dll/*/wsprintfW(pwszStr, L"%d.%d Build %d", osVerInfo.dwMajorVersion, osVerInfo.dwMinorVersion, osVerInfo.dwBuildNumber);
	dwBufLen = /*/kernel32.dll/*/lstrlenW(pwszStr);
	/*/shlwapi.dll#212/*/IStream_Write(pStream, pwszStr, dwBufLen * sizeof(WCHAR));
	if (/*/kernel32.dll/*/lstrlenW(osVerInfo.szCSDVersion))
	{
		/*/shlwapi.dll#212/*/IStream_Write(pStream, L" ", sizeof(WCHAR));
		dwBufLen = /*/kernel32.dll/*/lstrlenW(osVerInfo.szCSDVersion);
		/*/shlwapi.dll#212/*/IStream_Write(pStream, osVerInfo.szCSDVersion, dwBufLen * sizeof(WCHAR));
	}
	ULARGE_INTEGER uiSize;
	/*/shlwapi.dll#214/*/IStream_Size(pStream, &uiSize);
	pwszRet = (LPWSTR)/*/kernel32.dll/*/HeapAlloc(hHeap, HEAP_ZERO_MEMORY, uiSize.LowPart + 2);
	/*/shlwapi.dll#213/*/IStream_Reset(pStream);
	/*/shlwapi.dll#184/*/IStream_Read(pStream, pwszRet, uiSize.LowPart);
	/*/shlwapi.dll#169/*/IUnknown_AtomicRelease((LPVOID*)&pStream);
	return pwszRet;
}