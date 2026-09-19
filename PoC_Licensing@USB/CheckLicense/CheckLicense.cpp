// CheckLicense.cpp : Defines the entry point for the application.
//

#include "framework.h"
#include "CheckLicense.h"
#include "GenerateLicenseInfo.h"
#include <commdlg.h>
#include <shlwapi.h>

#pragma comment(lib, "shlwapi")

BOOL GetOpenFilePath(HWND, HANDLE, LPWSTR*);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
	LPWSTR pwszIniPath = NULL;
	HANDLE hHeap = /*/kernel32.dll/*/GetProcessHeap();
	if (GetOpenFilePath(NULL, hHeap, &pwszIniPath))
	{
		LPWSTR
			pwszFirst = /*/shlwapi.dll/*/StrChrW(pwszIniPath, L'\\'),
			pwszLast = /*/shlwapi.dll/*/StrRChrW(pwszIniPath, NULL, L'\\');
		if (pwszFirst == pwszLast)
		{
			WCHAR pwszDrv2Chk[4];
			/*/kernel32.dll/*/lstrcpynW(pwszDrv2Chk, pwszIniPath, 4);
			if (/*/kernel32.dll/*/GetDriveTypeW(pwszDrv2Chk) == DRIVE_REMOVABLE)
			{
				DWORD dwLenSN = 0, dwLen = 0;
				LPSTR pszDriveSN = NULL;
				if (GetDriveSN(hHeap, pwszDrv2Chk[0], &dwLenSN, &pszDriveSN) == NO_ERROR)
				{
					dwLen = /*/kernel32.dll/*/lstrlenW(pwszIniPath);
					LPSTR pszIniPath = (LPSTR)/*/kernel32.dll/*/HeapAlloc(hHeap, HEAP_ZERO_MEMORY, dwLen + 1);
					/*/user32.dll/*/wsprintfA(pszIniPath, "%S", pwszIniPath);
					CHAR pszStrRead[MAX_PATH];
					/*/kernel32.dll/*/GetPrivateProfileStringA("License", "Username", NULL, pszStrRead, MAX_PATH, pszIniPath);
					dwLen =/*/kernel32.dll/*/lstrlenA(pszStrRead);
					LPSTR pszKey = GenKeyHexHmac(hHeap, (LPBYTE)pszDriveSN, dwLenSN, (LPBYTE)pszStrRead, dwLen);
					/*/kernel32.dll/*/GetPrivateProfileStringA("License", "Key", NULL, pszStrRead, MAX_PATH, pszIniPath);
					if (/*/kernel32.dll/*/lstrcmpA(pszKey, pszStrRead) == 0)
						/*/user32.dll/*/MessageBoxA(NULL, "License in USB Thumb Drive Checked: Valid!", NULL, MB_OK);
					else
						/*/user32.dll/*/MessageBoxA(NULL, "License in USB Thumb Drive Checked: Invalid!", NULL, MB_OK);
					/*/kernel32.dll/*/HeapFree(hHeap, 0, pszKey);
					/*/kernel32.dll/*/HeapFree(hHeap, 0, pszIniPath);
					if(pszDriveSN)
						/*/kernel32.dll/*/HeapFree(hHeap, 0, pszDriveSN);
				}
				else
					/*/user32.dll/*/MessageBoxA(NULL, "Invalid USB Thumb Drive Picked...", NULL, MB_OK);
			}
			else
				/*/user32.dll/*/MessageBoxA(NULL, "License.ini Should be Stored in USB Thumb Drive...", NULL, MB_OK);
		}
		else
			/*/user32.dll/*/MessageBoxA(NULL, "License.ini Should be Located in Root Directory of a Logical Drive...", NULL, MB_OK);
	}
	else
		/*/user32.dll/*/MessageBoxA(NULL, "Cancel License Checking Process...", NULL, MB_OK);
	if (pwszIniPath)
		/*/kernel32.dll/*/HeapFree(hHeap, 0, pwszIniPath);
	/*/kernel32.dll/*/ExitProcess(0);
    return 0;
}

BOOL GetOpenFilePath(HWND hDlg, HANDLE hHeap, LPWSTR* pwszPath)
{
	WCHAR pwszStr[260];
	/*/ntdll.dll/*/RtlZeroMemory(pwszStr, 520);
	OPENFILENAMEW InfOpenFile;
	// Initialize OPENFILENAMEW
	/*/ntdll.dll/*/RtlZeroMemory(&InfOpenFile, sizeof(OPENFILENAMEW));
	InfOpenFile.lStructSize = sizeof(OPENFILENAMEW);
	InfOpenFile.hwndOwner = hDlg;
	InfOpenFile.lpstrFile = pwszStr;
	// Set lpstrFile[0] to '\0' so that GetOpenFileName does not use the contents of szFile to initialize itself.
	InfOpenFile.nMaxFile = MAX_PATH;
	InfOpenFile.lpstrFilter = L"MS ini file (*.ini)\0*.ini\0";
	InfOpenFile.lpstrTitle = L"Pick the License.ini File to Check";
	InfOpenFile.nFilterIndex = 1;
	InfOpenFile.lpstrFileTitle = NULL;
	InfOpenFile.nMaxFileTitle = 0;
	InfOpenFile.lpstrInitialDir = NULL;
	InfOpenFile.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
	// Display the Open dialog box. 
	BOOL bRet = /*/Comdlg32.dll/*/GetOpenFileNameW(&InfOpenFile);
	if (bRet)
	{
		if (*pwszPath)
			/*/kernel32.dll/*/HeapFree(hHeap, 0, *pwszPath);
		size_t lenTmp = /*/kernel32.dll/*/lstrlenW(InfOpenFile.lpstrFile) + 1;
		*pwszPath = (LPWSTR)/*/kernel32.dll/*/HeapAlloc(hHeap, HEAP_ZERO_MEMORY, lenTmp * 2);
		/*/kernel32.dll/*/lstrcpyW(*pwszPath, InfOpenFile.lpstrFile);
	}
	return bRet;
}
