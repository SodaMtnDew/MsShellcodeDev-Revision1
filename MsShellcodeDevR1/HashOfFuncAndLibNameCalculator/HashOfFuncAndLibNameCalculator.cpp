// HashOfFuncAndLibNameCalculator.cpp : Defines the entry point for the application.
//

#include "framework.h"
#include "HashOfFuncAndLibNameCalculator.h"
#include <Windows.h>
#include <Shlobj.h>
#include <Shellapi.h>
#include <shlwapi.h>
#include "..\Common\HashFunc.h"
#include "..\Common\HashFunc.cpp"

#pragma comment(lib, "Shlwapi")

// Global Variables:
HINSTANCE hInst; 

INT_PTR CALLBACK    HashBox(HWND, UINT, WPARAM, LPARAM);
BOOL                Browse4File(HWND, LPCWSTR, LPCWSTR, LPWSTR, BOOL);
BOOL                HashCalc(HWND);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);
    hInst = hInstance;
    DialogBoxW(hInst, MAKEINTRESOURCE(IDD_HASHBOX), NULL, HashBox);
    return 0;
}

INT_PTR CALLBACK HashBox(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	int wmId;
	WCHAR pwszPath[MAX_PATH] = { 0 };
	switch (message)
	{
	case WM_INITDIALOG:
#ifdef _DEBUG
		SetWindowTextA(GetDlgItem(hDlg, IDC_EDTDLL), "C:\\Windows\\SysWOW64\\kernel32.dll");
		SetWindowTextA(GetDlgItem(hDlg, IDC_EDTCSV), "C:\\kernel32.csv");
#endif
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		wmId = LOWORD(wParam);
		switch (wmId)
		{
		case IDC_BTNDLL:
			if (Browse4File(hDlg, L"Select the DLL File to Calculate Hash Values of Exported APIs....", L"dll", pwszPath, FALSE))
				SetWindowTextW(GetDlgItem(hDlg, IDC_EDTDLL), pwszPath);
			break;
		case IDC_BTNCSV:
			if (Browse4File(hDlg, L"Choose the Path to Save the Result of Calculation....", L"csv", pwszPath, TRUE))
				SetWindowTextW(GetDlgItem(hDlg, IDC_EDTCSV), pwszPath);
			break;
		case IDC_BTNCALC:
			if (HashCalc(hDlg))
				MessageBoxA(hDlg, "Finish the Calculation of API Hash Values in Selected DLL and Output as CSV File...", "Hint", MB_OK);
			break;
		case IDC_BTNABOUT:
			DialogBoxW(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hDlg, About);
			break;
		case IDCANCEL:
			DestroyWindow(hDlg);
			return (INT_PTR)TRUE;
			break;
		}
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	}
	return (INT_PTR)FALSE;
}

BOOL Browse4File(HWND hDlg, LPCWSTR pwszTitle, LPCWSTR pwszType, LPWSTR pwszPath, BOOL bNew)
{
	BOOL bRet = FALSE;
	IFileOpenDialog* pFileOperate;
	HRESULT hr;
	WCHAR pwszTypeSpec[10];
	lstrcatW(pwszTypeSpec, pwszType);
	if (bNew)
		hr = CoCreateInstance(CLSID_FileSaveDialog, NULL, CLSCTX_ALL, IID_IFileSaveDialog, reinterpret_cast<void**>(&pFileOperate));
	else
		hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL, IID_IFileOpenDialog, reinterpret_cast<void**>(&pFileOperate));
	if (SUCCEEDED(hr))
	{
		COMDLG_FILTERSPEC rgSpec[] =
		{
			{ NULL, NULL }
		};
		SHFILEINFOW sfi = { 0 };
		if (pwszType)
		{
			lstrcpyW(pwszTypeSpec, L"a.");
			lstrcatW(pwszTypeSpec, pwszType);
			SHGetFileInfoW(pwszTypeSpec, FILE_ATTRIBUTE_NORMAL, &sfi, sizeof(SHFILEINFOW), SHGFI_TYPENAME | SHGFI_USEFILEATTRIBUTES);
			pwszTypeSpec[0] = L'*';
			rgSpec[0].pszName = sfi.szTypeName;
			rgSpec[0].pszSpec = pwszTypeSpec;
			pFileOperate->SetFileTypes(1, rgSpec);
		}
		pFileOperate->SetTitle(pwszTitle);
		//pFileOpen->SetFileTypes()
		hr = pFileOperate->Show(hDlg);
		if (SUCCEEDED(hr))
		{
			IShellItem* pItem;
			hr = pFileOperate->GetResult(&pItem);
			if (SUCCEEDED(hr))
			{
				PWSTR pwszTmpPath;
				hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pwszTmpPath);
				if (SUCCEEDED(hr))
				{
					size_t lenTmp = wcslen(pwszTmpPath) + 1;
					lstrcpyW(pwszPath, pwszTmpPath);
					CoTaskMemFree(pwszTmpPath);
					bRet = TRUE;
				}
				pItem->Release();
			}
		}
		pFileOperate->Release();
	}
	return bRet;
}

BOOL HashCalc(HWND hDlg)
{
	BOOL bRet = FALSE;
	WCHAR pwszPathDLL[MAX_PATH], pwszPathCSV[MAX_PATH];
	GetWindowTextW(GetDlgItem(hDlg, IDC_EDTDLL), pwszPathDLL, MAX_PATH);
	GetWindowTextW(GetDlgItem(hDlg, IDC_EDTCSV), pwszPathCSV, MAX_PATH);
	if (lstrlenW(pwszPathDLL) == 0)
		MessageBoxA(hDlg, "Please Select the DLL File....", "Error", MB_OK);
	else if (lstrlenW(pwszPathCSV) == 0)
		MessageBoxA(hDlg, "Please Choose the Path to Save the Result...", "Error", MB_OK);
	else if (MessageBoxA(hDlg, "Do You Want to Calculate the Exported API Hash Values of Selected DLL and Output as Assigned CSV?", "Hint", MB_YESNO) == IDYES)
	{
		HMODULE hDLL = LoadLibraryW(pwszPathDLL);
		if (hDLL == NULL)
			MessageBoxA(hDlg, "Couldn't Open Selected DLL File....", "Error", MB_OK);
		else
		{
			HANDLE hCSV = CreateFileW(pwszPathCSV, GENERIC_WRITE, NULL, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
			if (hCSV == INVALID_HANDLE_VALUE)
			{
				MessageBoxA(hDlg, "Could'nt Create the File to Save Calculated Result....", "Error", MB_OK);
				FreeLibrary(hDLL);
			}
			else
			{
				DWORD dwSize;
				DWORD offPE = (DWORD)hDLL + 0x3c;
				DWORD addPE = *((DWORD*)offPE);
				DWORD offExp = (DWORD)hDLL + addPE + 0x78;
				DWORD addExp = *((DWORD*)offExp) + (DWORD)hDLL;
				DWORD numExp = *((DWORD*)(addExp + 0x18));
				DWORD addNTab = *((DWORD*)(addExp + 0x20)) + (DWORD)hDLL;
				DWORD* addFName = new DWORD[numExp];
				CHAR strLine[1000], strTmp[100], * strChar;
				DWORD i, j;
				LPWSTR pwszFileDLL = StrRChrW(pwszPathDLL, NULL, L'\\') + 1;
				BYTE byteHashOpt = 2;
				wsprintfA(strLine, "DLL Name,%S,,HashThisSC,0x%08x\r\n\r\n", pwszFileDLL, GetStrHash((LPSTR)pwszFileDLL, (PHASH_OPT)&byteHashOpt));
				WriteFile(hCSV, strLine, lstrlenA(strLine), &dwSize, NULL);
				for (i = 0; i < numExp; i++)
					addFName[i] = (DWORD)hDLL + *((DWORD*)(addNTab + 4 * i));
				wsprintfA(strLine, "API Name,HashThisSC,0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F,0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,0x19,0x1A,0x1B,0x1C,0x1D,0x1E,0x1F\r\n");
				WriteFile(hCSV, strLine, strlen(strLine), &dwSize, NULL);
				byteHashOpt = 1;
				for (i = 0; i < numExp; i++)
				{
					DWORD valHash = 0;
					wsprintfA(strLine, "%s", (BYTE*)(addFName[i]));
					//WriteFile(hCSV, strLine, lstrlenA(strLine), &dwSize, NULL);
					wsprintfA(strTmp, "%s", (BYTE*)(addFName[i]));
					wsprintfA(strTmp, ",0x%08x", GetStrHash(strTmp, (PHASH_OPT)&byteHashOpt));
					lstrcatA(strLine, strTmp);
					for (j = 0; j < 32; j++)
					{
						valHash = 0;
						wsprintfA(strTmp, "%s", (BYTE*)(addFName[i]));
						strChar = strTmp;
						while (*strChar)
						{
							valHash = (valHash >> j) | (valHash << (32 - j));
							valHash += *strChar;
							strChar++;
						}
						wsprintfA(strTmp, ",0x%08x", valHash);
						lstrcatA(strLine, strTmp);
					}
					lstrcatA(strLine, "\r\n");
					WriteFile(hCSV, strLine, strlen(strLine), &dwSize, NULL);
				}
				CloseHandle(hCSV);
				FreeLibrary(hDLL);
				bRet = TRUE;
			}
		}
	}
	return bRet;
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