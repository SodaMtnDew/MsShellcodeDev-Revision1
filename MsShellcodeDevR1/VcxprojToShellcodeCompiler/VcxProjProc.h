#pragma once

typedef struct err_code
{
	LPSTR                 pszFile;
	DWORD                 dwErr;
	struct err_code* pNext;
} ERR_CODE, * PERR_CODE;

typedef struct
{
	DWORD                 dwErrNo, dwNumErr;
	PERR_CODE             pErrCode;
} ERR_MAP, * PERR_MAP;

typedef struct file2Add
{
	LPSTR pszName;		//File Name to be Added to Project, could be Header or Source file
	DWORD dwErrCode;	//Error code of this file after processing
	struct file2Add * pNext;
} FILE2ADD, *PFILE2ADD;

typedef struct
{
	LPSTR pszRootName;
	PFILE2ADD pHeaders, pSources;
} PROP_VCXPROJ, *PPROP_VCXPROJ;

// Used to convert prototype vcxproj to shellcodized vcxproj
extern DWORD ParseVcxproj(HANDLE, PPROP_VCXPROJ, LPWSTR);
extern VOID  ConvertVcxproj(HANDLE, DWORD, PERR_MAP, LPSTR, LPWSTR, LPWSTR, LPWSTR, LPWSTR, BOOL, BOOL);
extern VOID  ExportVcxproj(HANDLE, PPROP_VCXPROJ, DWORD, LPSTR, LPWSTR, LPSTR, LPSTR);