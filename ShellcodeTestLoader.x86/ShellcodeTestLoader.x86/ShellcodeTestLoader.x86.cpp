// ShellcodeTestLoader.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "ShellcodeTestLoader.x86.h"
#include <commdlg.h>

typedef LPBYTE (*ptFnSC)();							//Prototype of Shellcodes' Main Function  
BOOL GetOpenFilePath(HWND, LPWSTR*);	//Function to Pick Shellcode File Path to be Loaded

HANDLE hHeap;

int APIENTRY winMain()
{
	hHeap = GetProcessHeap();
	LPWSTR pwszSc2Exec = NULL;
	if (GetOpenFilePath(NULL, &pwszSc2Exec))
	{
		HANDLE hFile = CreateFileW(pwszSc2Exec, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		DWORD dwSize = GetFileSize(hFile, NULL);
		ptFnSC pFnSC = (ptFnSC)VirtualAlloc(NULL, dwSize, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
		ReadFile(hFile, pFnSC, dwSize, &dwSize, NULL);
		CloseHandle(hFile);
		pFnSC();
		VirtualFree(pFnSC, dwSize, MEM_RELEASE);
		HeapFree(hHeap, 0, pwszSc2Exec);
	}
	ExitProcess(0);
	return 0;
}

BOOL GetOpenFilePath(HWND hDlg, LPWSTR* pwszPath)
{
	WCHAR pwszStr[260];
	LPOPENFILENAMEW pInfOpenFile;
	// Initialize OPENFILENAMEW
	pInfOpenFile = (LPOPENFILENAMEW)HeapAlloc(hHeap, HEAP_ZERO_MEMORY, sizeof(OPENFILENAMEW));
	pInfOpenFile->lStructSize = sizeof(OPENFILENAMEW);
	pInfOpenFile->hwndOwner = hDlg;
	pInfOpenFile->lpstrFile = pwszStr;
	// Set lpstrFile[0] to '\0' so that GetOpenFileName does not use the contents of szFile to initialize itself.
	pInfOpenFile->lpstrFile[0] = L'\0';
	pInfOpenFile->nMaxFile = MAX_PATH;
	pInfOpenFile->lpstrFilter = L"Binary file\0*.bin\0";
	pInfOpenFile->nFilterIndex = 1;
	pInfOpenFile->lpstrFileTitle = NULL;
	pInfOpenFile->nMaxFileTitle = 0;
	pInfOpenFile->lpstrInitialDir = NULL;
	pInfOpenFile->Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
	// Display the Open dialog box. 
	BOOL bRet = GetOpenFileNameW(pInfOpenFile);
	if (bRet)
	{
		if (*pwszPath)
			HeapFree(hHeap, 0, *pwszPath);
		size_t lenTmp = lstrlenW(pInfOpenFile->lpstrFile) + 1;
		*pwszPath = (LPWSTR)HeapAlloc(hHeap, HEAP_ZERO_MEMORY, lenTmp * 2);
		lstrcpyW(*pwszPath, pInfOpenFile->lpstrFile);
	}
	HeapFree(hHeap, 0, pInfOpenFile);
	return bRet;
}