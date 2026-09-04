// VcxprojToShellcodeCompilerRev1.cpp : Defines the entry point for the application.
//

#include "framework.h"
#include "VcxprojToShellcodeCompiler.h"
#include "DevEnvDetect.h"
#include "VcxProjProc.h"
#include <windowsx.h>
#include <Shellapi.h>
#include <Shlwapi.h>

#pragma comment(lib,"shlwapi")

INT_PTR CALLBACK    ConfigBox(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
DWORD               ExtractCodeSection(LPWSTR, LPWSTR, BOOL);

HINSTANCE hInst;
HANDLE hHeap;
WCHAR pwszHomePath[MAX_PATH], pwszPrjPath[MAX_PATH];
LPWSTR pwszMsDevVersion, pwszVcVars32Bat;
MSSDK_INFO msSDK_Info;
BOOL bNeedSDK;
DWORD dwVerMsDev;


int APIENTRY wWinMain(
	_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow
)
{
	hInst = GetModuleHandleW(NULL);
	DWORD dwErr = ERSC_NOERROR;
	hHeap = GetProcessHeap();
	GetModuleFileNameW(NULL, pwszHomePath, MAX_PATH);
	PathRemoveFileSpecW(pwszHomePath);
	dwErr = GetVcVars32Path(hHeap, &bNeedSDK, &dwVerMsDev, &pwszMsDevVersion, &pwszVcVars32Bat);
	if (!dwErr)
	{
		if (bNeedSDK)
			dwErr = GetSDKsNumStr(hHeap, &msSDK_Info);
	}
	switch (dwErr)
	{
	case ERSC_OLDMSDEV:
		MessageBoxA(NULL, "Visual Studio Version TOO OLD, Only Support 2015 or Higher Version!", "Error", MB_OK);
		break;
	case ERSC_NOMSDEV:
		MessageBoxA(NULL, "Couldn't find compatible Visual Studio Environment!", "Error", MB_OK);
		break;
	case ERSC_NOMSVC:
		MessageBoxA(NULL, "Couldn't find compatible Visual C++ installed!", "Error", MB_OK);
		break;
	case ERSC_NOSDK:
		MessageBoxA(NULL, "Couldn't find SDK installed when needed!", "Error", MB_OK);
		break;
	default:
		lstrcpyW(pwszPrjPath, pwszHomePath);
		lstrcatW(pwszPrjPath, L"\\PicProj");
		DialogBoxW(hInst, MAKEINTRESOURCEW(IDD_CONFIGBOX), NULL, ConfigBox);
		break;
	}
	dwErr = GetLastError();
	if (msSDK_Info.dwNumSDKs)
	{
		PMSSDK_ITEM pItemNow = msSDK_Info.pMsSdk_Item;
		while (pItemNow)
		{
			PMSSDK_ITEM pItemNext = pItemNow->pNext;
			HeapFree(hHeap, 0, pItemNow->pszSDK);
			HeapFree(hHeap, 0, pItemNow);
			pItemNow = pItemNext;
		}
	}
	if (pwszVcVars32Bat)
		HeapFree(hHeap, 0, pwszVcVars32Bat);
	if (pwszMsDevVersion)
		HeapFree(hHeap, 0, pwszMsDevVersion);
	return 0;
}

INT_PTR CALLBACK ConfigBox(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, numRemove = 2;
	LPWSTR pwszFile = NULL;
	BOOL bRet, bChkX64, bChkX86;
	HWND hCtrl;
	DWORD dwIdx, dwLen;
	ERR_MAP errMap;
	WCHAR pwszPath[MAX_PATH], pwszBatch[MAX_PATH], pwszOut[MAX_PATH];
	CHAR pszMsg[MAX_PATH];
	switch (message)
	{
	case WM_INITDIALOG:
		SetWindowTextW(GetDlgItem(hDlg, IDC_EDT_VIDE), pwszMsDevVersion);
		if (bNeedSDK)
		{
			hCtrl = GetDlgItem(hDlg, IDC_CMB_VSDK);
			PMSSDK_ITEM pItemNow = msSDK_Info.pMsSdk_Item;
			dwIdx = 0;
			while (pItemNow)
			{
				SendMessageA(hCtrl, CB_ADDSTRING, NULL, (LPARAM)pItemNow->pszSDK);
				dwIdx += 1;
				pItemNow = pItemNow->pNext;
			}
			SendMessageW(hCtrl, CB_SETCURSEL, dwIdx - 1, NULL);
		}
		else
			EnableWindow(GetDlgItem(hDlg, IDC_CMB_VSDK), FALSE);
		SetWindowTextW(GetDlgItem(hDlg, IDC_EDT_BAT), pwszVcVars32Bat);
		return (INT_PTR)TRUE;
		break;

	case WM_COMMAND:
		wmId = LOWORD(wParam);
		switch (wmId)
		{
		case IDCANCEL:
			DestroyWindow(hDlg);
			break;
		case IDC_BTN_SRC:
			bRet = GetOpenFilePath(hHeap, hDlg, &pwszFile);
			if (bRet)
			{
				SetWindowTextW(GetDlgItem(hDlg, IDC_EDT_SRC), pwszFile);
				HeapFree(hHeap, 0, pwszFile);
			}
			break;
		case IDC_BTN_ABOUT:
			DialogBoxW(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hDlg, About);
			break;
		case IDC_BTN_COMPILE:
			bChkX86 = (Button_GetCheck(GetDlgItem(hDlg, IDC_CHK_X86)) == BST_CHECKED);
			bChkX64 = (Button_GetCheck(GetDlgItem(hDlg, IDC_CHK_AMD64)) == BST_CHECKED);
			if (bChkX86 || bChkX64)
			{
				GetWindowTextW(GetDlgItem(hDlg, IDC_EDT_SRC), pwszPath, MAX_PATH);
				GetWindowTextW(GetDlgItem(hDlg, IDC_EDT_BAT), pwszBatch, MAX_PATH);
				GetWindowTextW(GetDlgItem(hDlg, IDC_EDT_OUT), pwszOut, MAX_PATH);
				if (lstrlenW(pwszPath) > 0)
				{
					DWORD dwLenOut = lstrlenW(pwszOut);
					LPSTR pszSDK = NULL, pszErrMsgs = NULL;
					LPSTREAM pStrm = NULL;
					hCtrl = GetDlgItem(hDlg, IDC_CMB_VSDK);
					if (IsWindowEnabled(hCtrl))
					{
						dwLen = GetWindowTextLengthA(hCtrl);
						pszSDK = (LPSTR)HeapAlloc(hHeap, HEAP_ZERO_MEMORY, dwLen + 1);
						GetWindowTextA(hCtrl, pszSDK, dwLen + 1);
					}
					__stosb((LPBYTE)&errMap, 0, sizeof(ERR_MAP));
					ConvertVcxproj(hHeap, dwVerMsDev, &errMap, pszSDK, pwszPath, pwszPrjPath, pwszBatch, pwszOut, bChkX86, bChkX64);
					if (!dwLenOut)
						SetWindowTextW(GetDlgItem(hDlg, IDC_EDT_OUT), pwszOut);
					if (errMap.dwNumErr == ERSC_BADVCXPROJ)
						MessageBoxA(hDlg, "Invalid vcxproj file format, please check ...", "Error", MB_OK);
					else if (errMap.dwNumErr == ERSC_HAVINGRSRC)
						MessageBoxA(hDlg, "Having resource(s) in vcxproj, please remove ...", "Error", MB_OK);
					else if (errMap.dwNumErr > 0)
					{
						pStrm = SHCreateMemStream(0, 0);
						lstrcpyA(pszMsg, "Error when converting source file(s) in vcxproj:");
						IStream_Write(pStrm, pszMsg, lstrlenA(pszMsg));
						PERR_CODE pErrCodeNow = errMap.pErrCode;
						while (pErrCodeNow)
						{
							if (pErrCodeNow->dwErr == ERSC_USEGLOBAL)
								wsprintfA(pszMsg, "\r\n\t%s: Using Global Variable(s)", pErrCodeNow->pszFile);
							else
								wsprintfA(pszMsg, "\r\n\t%s: Bad CPP File Format", pErrCodeNow->pszFile);
							IStream_Write(pStrm, pszMsg, lstrlenA(pszMsg));
							pErrCodeNow = pErrCodeNow->pNext;
						}
						ULARGE_INTEGER ulSize;
						IStream_Size(pStrm, &ulSize);
						pszErrMsgs = (LPSTR)HeapAlloc(hHeap, HEAP_ZERO_MEMORY, ulSize.LowPart + 1);
						IStream_Reset(pStrm);
						IStream_Read(pStrm, pszErrMsgs, ulSize.LowPart);
						IUnknown_AtomicRelease((LPVOID*)&pStrm);
						MessageBoxA(hDlg, pszErrMsgs, "Error", MB_OK);
						HeapFree(hHeap, 0, pszErrMsgs);
						pErrCodeNow = errMap.pErrCode;
						while (pErrCodeNow)
						{
							PERR_CODE pErrCode2Del = pErrCodeNow;
							pErrCodeNow = pErrCodeNow->pNext;
							HeapFree(hHeap, 0, pErrCode2Del->pszFile);
							HeapFree(hHeap, 0, pErrCode2Del);
						}
					}
					else
					{
						//start to call batch & extract .text section if no error
						LPCSTR pszErrMsgs[6] = {
							"32-bit EXE NOT Built, Check Generated Project!",					//ErrNo= 5
							"32-bit EXE Entrypoint != VA of .text, Check Selected Project!",	//ErrNo= 6
							"32-bit EXE with Import Table, Check Selected Project!",			//ErrNo= 7
							"64-bit EXE NOT Built, Check Generated Project!",					//ErrNo= 8
							"64-bit EXE Entrypoint != VA of .text, Check Selected Project!",	//ErrNo= 9
							"64-bit EXE with Import Table, Check Selected Project!"				//ErrNo=10
						};
						PROCESS_INFORMATION pi;
						STARTUPINFOW siW;
						__stosb((LPBYTE)&pi, 0, sizeof(PROCESS_INFORMATION));
						__stosb((LPBYTE)&siW, 0, sizeof(STARTUPINFOW));
						DWORD dwErr[2] = { 0,0 }, dwLenMsg;
						pszMsg[0] = '\0';
						//CHAR pszData2Insert[MAX_PATH];
						lstrcpyW(pwszBatch, pwszPrjPath);
						lstrcatW(pwszBatch, L"\\VcxBuild.bat");
						CreateProcessW(NULL, pwszBatch, NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, pwszPrjPath, &siW, &pi);
						WaitForSingleObject(pi.hProcess, INFINITE);
						if (bChkX86)
						{
							wsprintfW(pwszPath, L"%s\\Release\\%s-x86.exe", pwszPrjPath, pwszOut);
							wsprintfW(pwszBatch, L"%s\\%s.x86.bin", pwszHomePath, pwszOut);
							dwErr[0] = ExtractCodeSection(pwszPath, pwszBatch, FALSE);
							dwLenMsg = lstrlenA(pszMsg);
							if (dwErr[0])
								wsprintfA(pszMsg + dwLenMsg, "Error when Compiling %S in x86 Release Config: %s\r\n", pwszOut, pszErrMsgs[dwErr[0] - 5]);
						}
						if (bChkX64)
						{

							wsprintfW(pwszPath, L"%s\\Release\\%s-x64.exe", pwszPrjPath, pwszOut);
							wsprintfW(pwszBatch, L"%s\\%s.x64.bin", pwszHomePath, pwszOut);
							dwErr[1] = ExtractCodeSection(pwszPath, pwszBatch, TRUE);
							dwLenMsg = lstrlenA(pszMsg);
							if (dwErr[1])
								wsprintfA(pszMsg + dwLenMsg, "%sError when Compiling %S in x64 Release Config: %s", dwLenMsg ? "\r\n" : "", pwszOut, pszErrMsgs[dwErr[1] - 5]);
						}
						dwLenMsg = lstrlenA(pszMsg);
						if (dwLenMsg)
							MessageBoxA(hDlg, pszMsg, "Error", MB_OK);
						else
						{
							lstrcpyA(pszMsg, "EXE(s) Compiled & Shellcode File(s) Extracted Successfully, You Could Now Use ShellcodeTestLoader to Test the Exported Binary File(s)...");
							MessageBoxA(hDlg, pszMsg, "Hint", MB_OK);
						}
					}
				}
			}
			else
				MessageBoxA(hDlg, "Should Check At Least 1 Output Architecture to Continue...", "Error", MB_OK);
			break;
		}
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	}
	return (INT_PTR)FALSE;
}

DWORD ExtractCodeSection(LPWSTR pwszExe, LPWSTR pwszBin, BOOL bIsX64)
{
	DWORD dwRet = 0;
	HANDLE hSrc = CreateFileW(pwszExe, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hSrc != INVALID_HANDLE_VALUE)
	{
		DWORD dwSize = GetFileSize(hSrc, NULL);
		LPBYTE pBufPE = (LPBYTE)HeapAlloc(hHeap, HEAP_ZERO_MEMORY, dwSize);
		ReadFile(hSrc, pBufPE, dwSize, &dwSize, NULL);
		CloseHandle(hSrc);
		DWORD dwEntry = 0, dwBase = 0;
		BOOL bHasImport = FALSE;
		PIMAGE_NT_HEADERS ntHeaders = (PIMAGE_NT_HEADERS)(pBufPE + ((PIMAGE_DOS_HEADER)pBufPE)->e_lfanew);
		PIMAGE_SECTION_HEADER secHdrs = (PIMAGE_SECTION_HEADER)((LPBYTE)ntHeaders + sizeof(DWORD) + sizeof(IMAGE_FILE_HEADER) + ntHeaders->FileHeader.SizeOfOptionalHeader);
		if (bIsX64)
		{
			PIMAGE_OPTIONAL_HEADER64 optHdr64 = (PIMAGE_OPTIONAL_HEADER64)((LPBYTE)ntHeaders + sizeof(DWORD) + sizeof(IMAGE_FILE_HEADER));
			bHasImport = (optHdr64->DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress > 0);
			dwEntry = optHdr64->AddressOfEntryPoint;
			dwBase = optHdr64->BaseOfCode;
		}
		else
		{
			PIMAGE_OPTIONAL_HEADER32 optHdr32 = (PIMAGE_OPTIONAL_HEADER32)((LPBYTE)ntHeaders + sizeof(DWORD) + sizeof(IMAGE_FILE_HEADER));
			bHasImport = (optHdr32->DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress > 0);
			dwEntry = optHdr32->AddressOfEntryPoint;
			dwBase = optHdr32->BaseOfCode;
		}
		if (dwBase != dwEntry)
			dwRet = 6 + (bIsX64 ? 3 : 0);
		else if (bHasImport)
			dwRet = 7 + (bIsX64 ? 3 : 0);
		else
		{
			LPBYTE pBufCode = pBufPE + secHdrs[0].PointerToRawData;
			DWORD dwSizeCode = secHdrs[0].Misc.VirtualSize;
			WCHAR pwszBinPath[MAX_PATH];
			lstrcpyW(pwszBinPath, pwszBin);
			HANDLE hDst = CreateFileW(pwszBinPath, GENERIC_WRITE, NULL, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
			if (hDst != INVALID_HANDLE_VALUE)
			{
				WriteFile(hDst, pBufCode, dwSizeCode, &dwSize, NULL);
				CloseHandle(hDst);
			}
			HeapFree(hHeap, 0, pBufPE);
		}
	}
	else
		dwRet = 5 + (bIsX64 ? 3 : 0);
	return dwRet;
}

INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}