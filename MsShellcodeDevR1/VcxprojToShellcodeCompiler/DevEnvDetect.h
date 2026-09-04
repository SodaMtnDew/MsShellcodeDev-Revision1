#pragma once

typedef struct
{
	LPWSTR pwszVerVS;
	DWORD dwVerVS;
} MSDEV_VERINFO, *PMSDEV_VERINFO;

typedef struct _sdk_Item
{
	LPSTR pszSDK;
	struct _sdk_Item * pNext;
} MSSDK_ITEM, *PMSSDK_ITEM;

typedef struct
{
	DWORD dwNumSDKs;
	PMSSDK_ITEM pMsSdk_Item;
} MSSDK_INFO, *PMSSDK_INFO;

// Used to get devenv & sdk versions
extern DWORD GetVcVars32Path(HANDLE, LPBOOL, LPDWORD, LPWSTR *, LPWSTR *);
extern DWORD GetSDKsNumStr(HANDLE, PMSSDK_INFO);
extern DWORD GetFileProperties(HANDLE, LPWSTR, LPWSTR *);
// Used to pick existing file path
extern BOOL GetOpenFilePath(HANDLE, HWND, LPWSTR *);
// Used in deleting TmpPrj folder
extern VOID DeletePath(HANDLE, LPWSTR);