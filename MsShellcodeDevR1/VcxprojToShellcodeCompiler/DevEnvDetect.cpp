#include "framework.h"
#include "DevEnvDetect.h"
#include <commdlg.h>
#include <shellapi.h>
#include <shlwapi.h>
//#include <Shobjidl.h>
//#include <Propkey.h>

#pragma comment(lib, "version")

DWORD GetVcVars32Path(HANDLE hHeap, LPBOOL pbNeedSDK, LPDWORD pdwVerMsDev,  LPWSTR * ppwszMsDev, LPWSTR * ppwszVcBat)
{
	DWORD dwRet = ERSC_NOERROR, dwType = REG_SZ, dwSize = MAX_PATH * sizeof(WCHAR);
	WCHAR pwszPath[MAX_PATH];
	*pdwVerMsDev = 0;
	pwszPath[0] = L'\0';
	HKEY hKey;
	LONG lRet = RegOpenKeyExW(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\App Paths\\devenv.exe", 0, KEY_READ, &hKey);
	if (!lRet)
	{
		RegQueryValueExW(hKey, NULL, NULL, &dwType, (LPBYTE)pwszPath, &dwSize);
		RegCloseKey(hKey);
		if (lstrlenW(pwszPath))
		{
			if (pwszPath[0] == L'\"')
			{
				pwszPath[lstrlenW(pwszPath) - 1] = L'\0';
				lstrcpyW(pwszPath, pwszPath + 1);
			}
			*pdwVerMsDev = GetFileProperties(hHeap, pwszPath, ppwszMsDev);
			if (*pdwVerMsDev)
			{
				if (*pdwVerMsDev <= 11)
					dwRet = ERSC_OLDMSDEV;
				else
				{
					if (*pdwVerMsDev > 14)
						*pbNeedSDK = TRUE;
					else
						*pbNeedSDK = FALSE;
					lstrcpyW(StrRStrIW(pwszPath, NULL, L"Common7"), L"VC\\");
					lstrcatW(pwszPath, *pbNeedSDK ? L"Auxiliary\\Build\\vcvars32.bat" : L"bin\\vcvars32.bat");
					if (PathFileExistsW(pwszPath))
					{
						dwSize = lstrlenW(pwszPath) * sizeof(WCHAR) + 2;
						*ppwszVcBat = (LPWSTR)HeapAlloc(hHeap, HEAP_ZERO_MEMORY, dwSize);
						lstrcpyW(*ppwszVcBat, pwszPath);
					}
					else
						dwRet = ERSC_NOMSVC;
				}
			}
			else
				dwRet = ERSC_OLDMSDEV;
		}
		else
			dwRet = ERSC_NOMSDEV;
	}
	if (dwRet)
	{
		HeapFree(hHeap, 0, *ppwszMsDev);
		*ppwszMsDev = NULL;
		HeapFree(hHeap, 0, *ppwszVcBat);
		*ppwszVcBat = NULL;
	}
	return dwRet;
}

DWORD GetSDKsNumStr(HANDLE hHeap, PMSSDK_INFO pMsSDK_Info)
{
	DWORD dwRet = ERSC_NOERROR, dwType = REG_SZ, dwSize = MAX_PATH;
	CHAR pszPath[MAX_PATH];
	LPSTREAM pStrm = NULL;
	HANDLE hFind;
	WIN32_FIND_DATAA findData;
	pszPath[0] = '\0';
	HKEY hKey;
	LONG lRet = RegOpenKeyExA(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Microsoft SDKs\\Windows\\v10.0", 0, KEY_READ | KEY_WOW64_32KEY, &hKey);
	if (!lRet)
	{
		RegQueryValueExA(hKey, "InstallationFolder", NULL, &dwType, (LPBYTE)pszPath, &dwSize);
		RegCloseKey(hKey);
		if (lstrlenA(pszPath))
		{
			lstrcatA(pszPath, "Include\\*");
			__stosb((LPBYTE)&findData, 0, sizeof(WIN32_FIND_DATAA));
			hFind = FindFirstFileA(pszPath, &findData);
			dwType = 0;
			if (hFind != INVALID_HANDLE_VALUE)
			{
				PMSSDK_ITEM pItemNow = NULL;
				do
				{
					if (lstrlenA(findData.cFileName) > 2)
					{
						if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
						{
							pMsSDK_Info->dwNumSDKs += 1;
							PMSSDK_ITEM pItemNew = (PMSSDK_ITEM)HeapAlloc(hHeap, HEAP_ZERO_MEMORY, sizeof(MSSDK_ITEM));
							if (pMsSDK_Info->pMsSdk_Item)
							{
								pItemNow->pNext = pItemNew;
								pItemNow = pItemNew;
							}
							else
							{
								pItemNow = pItemNew;
								pMsSDK_Info->pMsSdk_Item = pItemNow;
							}
							pItemNow->pszSDK = (LPSTR)HeapAlloc(hHeap, HEAP_ZERO_MEMORY, lstrlenA(findData.cFileName) + 1);
							lstrcpyA(pItemNow->pszSDK, findData.cFileName);
						}
					}
					__stosb((LPBYTE)&findData, 0, sizeof(WIN32_FIND_DATAA));
				} while (FindNextFileA(hFind, &findData));
				FindClose(hFind);
			}
		}
	}
	if (!pMsSDK_Info->dwNumSDKs)
		dwRet = ERSC_NOSDK;

	return dwRet;
}

DWORD GetFileProperties(HANDLE hHeap, LPWSTR pwszPath, LPWSTR * ppwszMsDev)
{
	DWORD dwVerRet = 0, dwBufSize = GetFileVersionInfoSizeW(pwszPath, &dwVerRet);
	dwVerRet = 0;
	LPVOID pInfoBlock = HeapAlloc(hHeap, HEAP_ZERO_MEMORY, dwBufSize);
	GetFileVersionInfoW(pwszPath, dwVerRet, dwBufSize, pInfoBlock);
	DWORD dwBytes;
	struct LANGANDCODEPAGE {
		WORD wLanguage;
		WORD wCodePage;
	} *lpTranslate;
	WCHAR pwszStr[50];
	LPWSTR pwszDesc = NULL, pwszVer = NULL;
	VerQueryValueW(pInfoBlock, L"\\VarFileInfo\\Translation", (LPVOID *)&lpTranslate, (PUINT)&dwBytes);
	wsprintfW(pwszStr, L"\\StringFileInfo\\%04x%04x\\FileDescription",lpTranslate[0].wLanguage,lpTranslate[0].wCodePage);
	VerQueryValueW(pInfoBlock, pwszStr, (LPVOID *)&pwszDesc, (PUINT)&dwBytes);
	wsprintfW(pwszStr, L"\\StringFileInfo\\%04x%04x\\ProductVersion", lpTranslate[0].wLanguage, lpTranslate[0].wCodePage);
	VerQueryValueW(pInfoBlock, pwszStr, (LPVOID*)&pwszVer, (PUINT)&dwBytes);
	dwVerRet = (DWORD)StrToIntW(pwszVer);
	*ppwszMsDev = (LPWSTR)HeapAlloc(hHeap, HEAP_ZERO_MEMORY, (lstrlenW(pwszDesc) + lstrlenW(pwszVer) + 4) * 2);
	wsprintfW(*ppwszMsDev, L"%s (%s)", pwszDesc, pwszVer);
	HeapFree(hHeap, 0, pInfoBlock);
	return dwVerRet;
}

BOOL GetOpenFilePath(HANDLE hHeap, HWND hDlg, LPWSTR* pwszPath)
{
	BOOL bRet = FALSE;
	WCHAR pwszStr[260];
	__stosb((LPBYTE)pwszStr, 0, 520);
	OPENFILENAMEW InfOpenFile;
	__stosb((LPBYTE)&InfOpenFile, 0, sizeof(OPENFILENAMEW));
	InfOpenFile.lStructSize = sizeof(OPENFILENAMEW);
	InfOpenFile.hwndOwner = hDlg;
	InfOpenFile.lpstrFile = pwszStr;
	// Set lpstrFile[0] to '\0' so that GetOpenFileName does not use the contents of szFile to initialize itself.
	InfOpenFile.nMaxFile = MAX_PATH;
	InfOpenFile.lpstrFilter = L"VC++ Project files (*.vcxproj)\0*.vcxproj\0";
	InfOpenFile.lpstrTitle = L"Pick the VC++ Project to be Converted & Compiled...";
	InfOpenFile.nFilterIndex = 1;
	InfOpenFile.lpstrFileTitle = NULL;
	InfOpenFile.nMaxFileTitle = 0;
	InfOpenFile.lpstrInitialDir = NULL;
	InfOpenFile.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
	// Display the Open dialog box. 
	bRet = GetOpenFileNameW(&InfOpenFile);
	if (bRet)
	{
		if (*pwszPath)
			HeapFree(hHeap, 0, *pwszPath);
		size_t lenTmp = lstrlenW(InfOpenFile.lpstrFile) + 1;
		*pwszPath = (LPWSTR)HeapAlloc(hHeap, HEAP_ZERO_MEMORY, lenTmp * 2);
		lstrcpyW(*pwszPath, InfOpenFile.lpstrFile);
	}
	return bRet;
}

VOID DeletePath(HANDLE hHeap, LPWSTR pwszPath)
{
	if (PathFileExistsW(pwszPath))
	{
		int lenPath = lstrlenW(pwszPath) + 2;
		LPWSTR pwszFrom = (LPWSTR)HeapAlloc(hHeap, HEAP_ZERO_MEMORY, lenPath * 2);
		lstrcpyW(pwszFrom, pwszPath);
		SHFILEOPSTRUCTW stSHFile = { 0 };
		stSHFile.wFunc = FO_DELETE;
		stSHFile.pFrom = pwszFrom;
		stSHFile.fFlags = FOF_NOCONFIRMATION | FOF_SILENT;
		stSHFile.fAnyOperationsAborted = FALSE;
		SHFileOperationW(&stSHFile);
		HeapFree(hHeap, 0, pwszFrom);
	}
}