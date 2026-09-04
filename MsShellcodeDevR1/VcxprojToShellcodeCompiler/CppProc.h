#pragma once

typedef struct mod_VarMap
{
	LPSTR                 pszModName, pszStrName, pszDllName;
	DWORD                 dwLenStr, dwHash;
	struct mod_VarMap   * pNext;
} VAR_MODITEM, *PVAR_MODITEM;

typedef struct api_VarMap
{
	LPSTR                 pszFncName, pszApiName;
	PVAR_MODITEM          pVarModItem;
	DWORD                 dwHash;
	DWORD                 dwOrd;
	struct api_VarMap   * pNext;
} VAR_APIITEM, *PVAR_APIITEM;

typedef struct raw_VarMap
{
	LPSTR                 pszRawStr;
	struct raw_VarMap   * pNext;
} VAR_RAWITEM, *PVAR_RAWITEM;

typedef struct str_VarItemA
{
	CHAR                  pszStrName[16];
	LPBYTE                pDataParsed;
	DWORD                 dwLenData;
	DWORD                 dwNumRaw;
	PVAR_RAWITEM          pVarRaw;
	struct str_VarItemA * pNext;
} VAR_STRITEMA, *PVAR_STRITEMA;

typedef struct str_VarItemW
{
	CHAR                  pszStrName[16];
	LPBYTE                pDataParsed;
	DWORD                 dwLenData;
	DWORD                 dwNumRaw;
	PVAR_RAWITEM          pVarRaw;
	struct str_VarItemW * pNext;
} VAR_STRITEMW, *PVAR_STRITEMW;

typedef struct
{
	DWORD                 dwNumModItem;
	PVAR_MODITEM          pVar_ModItem;
	DWORD                 dwNumApiItem;
	PVAR_APIITEM          pVar_ApiItem;
	DWORD                 dwNumStrItemA;
	PVAR_STRITEMA         pVar_StrItemA;
	DWORD                 dwNumStrItemW;
	PVAR_STRITEMW         pVar_StrItemW;
} VARS_MAP, *PVARS_MAP;

typedef struct str_RepMap
{
	LPSTR                 pszOri;
	LPSTR                 pszNew;
	struct str_RepMap *   pNext;
} STR_REPMAP, *PSTR_REPMAP;

// Used to convert each c source file
extern DWORD ConvertCppFile(HANDLE, LPWSTR, LPWSTR);
// Used to convert c function block to shellcoded function block
extern DWORD ShellcodingBlock(HANDLE, LPSTR, LPSTR *);
extern DWORD ParseModsAPIs(HANDLE, PVARS_MAP, LPSTR);
extern DWORD ParseStrs(HANDLE, PVARS_MAP, LPSTR);
extern LPBYTE ConvertRaw2Bin(HANDLE, LPSTR, LPDWORD);
extern LPSTR GenDwordArrayAndMore(HANDLE, PVARS_MAP, PSTR_REPMAP *);
extern VOID ReleaseVarMap(HANDLE, PVARS_MAP, PSTR_REPMAP *);
extern PVAR_MODITEM FindModInVarMap(PVARS_MAP, LPSTR);
extern PVAR_APIITEM FindApiInVarMap(PVARS_MAP, LPSTR);
extern PVAR_STRITEMA FindStrAInVarMapRaw(PVARS_MAP, LPSTR);
extern PVAR_STRITEMA FindStrAInVarMapParsed(PVARS_MAP, LPBYTE, DWORD);
extern PVAR_STRITEMW FindStrWInVarMapRaw(PVARS_MAP, LPSTR);
extern PVAR_STRITEMW FindStrWInVarMapParsed(PVARS_MAP, LPBYTE, DWORD);

#define HEAD_WINMAIN \
	"#ifndef _DEBUG\r\n"	\
	"extern \"C\" int myWinMain();\r\n"	\
	"#pragma alloc_text(\".text\", myWinMain)\r\n"	\
	"int myWinMain()\r\n"	\
	"#else\r\n"
#define TAIL_WINMAIN "\r\n#endif"