// GenerateLicenseAtUSB.cpp : Defines the entry point for the application.
//

#include "framework.h"
#include "GenerateLicenseAtUSB.h"
#include "..\CheckLicense\GenerateLicenseInfo.h"

INT_PTR CALLBACK    MainBox(HWND, UINT, WPARAM, LPARAM);
int	                RefreshThumbdrive(HWND);

HANDLE hHeap;

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
	hHeap = GetProcessHeap();
	HINSTANCE hInst = GetModuleHandleW(NULL);
    DialogBox(hInst, MAKEINTRESOURCE(IDD_MAINDIALOG), NULL, MainBox);
    return 0;
}

INT_PTR CALLBACK MainBox(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId;
	DWORD dwLenStr;
	HWND hCtrl;
	switch (message)
	{
	case WM_INITDIALOG:
		if (RefreshThumbdrive(hDlg) == 0)
			MessageBoxA(hDlg, "No Thumb Drive Detected, Please Insert 1 or More & Mount Them...", "Error", MB_OK);
		return (INT_PTR)TRUE;
		break;

	case WM_COMMAND:
		wmId = LOWORD(wParam);
		switch (wmId)
		{
		case IDCANCEL:
			if (MessageBoxA(hDlg, "Ready to Close the Application?", "Hint", MB_YESNO) == IDYES)
				DestroyWindow(hDlg);
			break;
		case IDC_BTN_REFRESH:
			if (RefreshThumbdrive(hDlg) == 0)
				MessageBoxA(hDlg, "No Thumb Drive Detected, Please Insert 1 or More & Mount Them...", "Error", MB_OK);
			break;
		case IDC_BTN_GENERATE:
			hCtrl = GetDlgItem(hDlg, IDC_EDT_USERNAME);
			dwLenStr = GetWindowTextLengthA(hCtrl);
			if (dwLenStr > 0)
			{
				WCHAR pwszPath[MAX_PATH];
				__stosb((LPBYTE)pwszPath, 0, MAX_PATH * sizeof(WCHAR));
				GetWindowTextW(GetDlgItem(hDlg, IDC_CMB_DRIVE), pwszPath, MAX_PATH);
				if (lstrlenW(pwszPath))
				{
					LPSTR pszUserName = (LPSTR)HeapAlloc(hHeap, HEAP_ZERO_MEMORY, dwLenStr + 1);
					GetWindowTextA(hCtrl, pszUserName, dwLenStr + 1);
					DWORD dwLenSN = 0;
					LPSTR pszDriveSN = NULL;
					GetDriveSN(hHeap, pwszPath[0], &dwLenSN, &pszDriveSN);
					if (!dwLenSN)
						if (MessageBoxA(hDlg, "Thumb Drive Picked Has NO iSerialNumber, Still Want to Generate License Key?", NULL, MB_YESNO) == IDYES)
							dwLenSN = 1;
					if (dwLenSN)
					{
						if (!pszDriveSN)
							dwLenSN = 0;
						DWORD dwLenKey = 0;
						LPSTR pszKey = GenKeyHexHmac(hHeap, (LPBYTE)pszDriveSN, dwLenSN, (LPBYTE)pszUserName, dwLenStr);
						if (lstrlenA(pszKey) > 0)
						{
							CHAR pszIniFile[MAX_PATH];
							SetWindowTextA(GetDlgItem(hDlg, IDC_EDT_KEY), pszKey);
							wsprintfA(pszIniFile, "%SLicense.ini", pwszPath);
							WritePrivateProfileStringA("License", "Username", pszUserName, pszIniFile);
							WritePrivateProfileStringA("License", "Key", pszKey, pszIniFile);
							wsprintfW(pwszPath, L"License Info Saved to \"%S\" Successfullly...", pszIniFile);
							MessageBoxW(hDlg, pwszPath, L"Hint", MB_OK);
							HeapFree(hHeap, 0, pszKey);
						}
						else
							MessageBoxA(hDlg, "Could NOT Generate License Key to be Saved...", "Error", MB_OK);
						if (pszDriveSN)
							HeapFree(hHeap, 0, pszDriveSN);
					}
					else
						MessageBoxA(hDlg, "Thumb Drive Picked Has NO iSerialNumber, Could NOT be Used to Save the License File...", "Error", MB_OK);
					HeapFree(hHeap, 0, pszUserName);
				}
				else
					MessageBoxA(hDlg, "Should Pick a Thumb Drive to Save the License File...", "Error", MB_OK);
			}
			else
				MessageBoxA(hDlg, "Username Should be Given to Generate License...", "Error", MB_OK);
			break;
		}
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	}
	return (INT_PTR)FALSE;
}

int	RefreshThumbdrive(HWND hDlg)
{
	int i = 0;
	HWND hCtrl = GetDlgItem(hDlg, IDC_CMB_DRIVE);
	SendMessageW(hCtrl, CB_RESETCONTENT, 0, 0);
	DWORD dwDrives = GetLogicalDrives();
	WCHAR pwszDrv[4] = { L'A',L':',L'\\',L'\0' };
	while (dwDrives)
	{
		if (dwDrives & 1)
		{
			if (GetDriveTypeW(pwszDrv) == DRIVE_REMOVABLE)
			{
				SendMessageW(hCtrl, CB_ADDSTRING, 0, (LPARAM)pwszDrv);
				i += 1;
			}
		}
		dwDrives >>= 1;
		pwszDrv[0] += 1;
	}
	if (i)
		SendMessageA(hCtrl, CB_SETCURSEL, 0, 0);
	return i;
}