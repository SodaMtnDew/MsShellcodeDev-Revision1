// ShellcodeSampleRev1.cpp : Defines the entry point for the application.
//

#include "framework.h"
#include "ShellcodeSample.h"
#include "GetSysInfo.h"

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	HANDLE hHeap = /*/kernel32.dll/*/GetProcessHeap();
	LPWSTR pwszInfo = GetSysInfo(hHeap);
	/*/user32.dll/*/MessageBoxW(NULL, pwszInfo, L"SysInfo", MB_OK);
	/*/kernel32.dll/*/HeapFree(hHeap, 0, pwszInfo);
	return 0;
}
