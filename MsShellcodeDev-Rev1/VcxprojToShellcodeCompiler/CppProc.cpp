#include "framework.h"
#include "CppProc.h"
#include <Shlwapi.h>
#include "..\Common\HashFunc.h"
#include "..\Common\HashFunc.cpp"

BOOL IsCharHexA(CHAR ch)
{
	BOOL bRet = TRUE;
	if (IsCharAlphaNumericA(ch))
	{
		if (ch > 'f')
			bRet = FALSE;
		else if (IsCharUpperA(ch) && (ch > 'F'))
			bRet = FALSE;
	}
	else
		bRet = FALSE;
	return bRet;
}

DWORD ConvertCppFile(HANDLE hHeap, LPWSTR pwszSrc, LPWSTR pwszDst)
{
	DWORD dwRet = ERSC_NOERROR, dwSize;
	HANDLE hFile;
	BOOL bAddHeader = FALSE, bIsMain = FALSE;
	hFile = CreateFileW(pwszSrc, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	dwSize = GetFileSize(hFile, NULL);
	LPSTR pszNow, pszToken, pszOutput = NULL, pszContent = (LPSTR)HeapAlloc(hHeap, HEAP_ZERO_MEMORY, dwSize + 3);
	ReadFile(hFile, pszContent, dwSize, &dwSize, NULL);
	CloseHandle(hFile);
	lstrcatA(pszContent, "\r\n");
	pszNow = pszContent;
	LPSTREAM pStrm = SHCreateMemStream(NULL, 0);;
	if (((BYTE)pszNow[0]) > 0x7e)
	{
		if (!StrCmpNA(pszNow, "\xEF\xBB\xBF", 3))
		{
			IStream_Write(pStrm, pszNow, 3);
			pszNow += 3;
		}
		else
			dwRet = ERSC_BADCPP;
	}
	if (dwRet == ERSC_BADCPP)
	{
		IUnknown_AtomicRelease((LPVOID *)&pStrm);
		HeapFree(hHeap, 0, pszContent);
		return dwRet;
	}
	while (1)
	{
		pszToken = StrStrA(pszNow, "\r\n");
		while ((*(pszToken - 1) == L'\\') || (*(pszToken - 1) == L','))
			pszToken = StrStrA(pszToken + 2, "\r\n");
		pszToken[0] = '\0';
		dwSize = (DWORD)(pszToken - pszNow);
		if (IsCharAlphaA(pszNow[0]))	// Normal lines, check and do more things
		{
			if (StrChrA(pszNow, '(') && StrChrA(pszNow, ')'))	// Function prototype or Function Block
			{
				BOOL bFuncBlk = FALSE;
				if (*(pszToken - 1) != L';')	// Function prototype or block
				{
					pszToken[0] = '\r';
					pszToken = StrStrA(pszToken, "\r\n");
					while ((pszToken[2] != L'{') || ((pszToken[2] == L';')))
						pszToken = StrStrA(pszToken + 2, "\r\n");
					if (pszToken[2] == '{')	// Function block
					{
						bFuncBlk = TRUE;
						pszToken[2] = '\0';
						if (StrStrA(pszNow, "wWinMain"))
						{
							LPCSTR pszEntry = "extern \"C\" int _scMain();\r\n#pragma alloc_text(\".text\", _scMain)\r\n\r\nint _scMain()";
							IStream_Write(pStrm, pszEntry, lstrlenA(pszEntry));
							bIsMain = TRUE;
						}
						else
							IStream_Write(pStrm, pszNow, dwSize);
						IStream_Write(pStrm, "\r\n", 2);
						pszNow = pszToken + 3;
						pszToken = StrStrA(pszNow, "\r\n}");
						pszToken[2] = '\0';
						pszToken += 3;
						// Call function to process function block, head
						LPSTR pszConverted = NULL;
						ShellcodingBlock(hHeap, pszNow, &pszConverted);
						dwSize = lstrlenA(pszConverted);
						IStream_Write(pStrm, pszConverted, dwSize);
						HeapFree(hHeap, 0, pszConverted);
						// Call function to process function block, tail
						IStream_Write(pStrm, "\r\n", 2);
					}
					else
					{
						IStream_Write(pStrm, pszNow, dwSize);
						IStream_Write(pStrm, ";\r\n", 3);
						pszToken += 1;
					}
				}
				else	//Function prototype
				{
					IStream_Write(pStrm, pszNow, dwSize);
					IStream_Write(pStrm, "\r\n", 2);
				}
			}
			else	// Should be global variable defined
			{
				IStream_Write(pStrm, pszNow, dwSize);
				IStream_Write(pStrm, "\r\n", 2);
				dwRet = ERSC_USEGLOBAL;
			}
		}
		else	//Comment or compiler option lines, just write to IStream
		{
			if (StrCmpNA(pszNow, "#include \"", 10) == 0)
			{
				if (!bAddHeader)
				{
					bAddHeader = TRUE;
					LPCSTR pszIncAtHead = "#include \"framework.h\"\r\n#include \"ScFunc.h\"\r\n";
					IStream_Write(pStrm, pszIncAtHead, lstrlenA(pszIncAtHead));
				}
				if (!StrStrA(pszNow, "stdafx.h") &&
					!StrStrA(pszNow, "Resource.h") &&
					!StrStrA(pszNow, "targetver.h") &&
					!StrStrA(pszNow, "pch.h") &&
					!StrStrA(pszNow, "framework.h"))
				{
					IStream_Write(pStrm, pszNow, dwSize);
					IStream_Write(pStrm, "\r\n", 2);
				}
			}
			else
			{
				IStream_Write(pStrm, pszNow, dwSize);
				IStream_Write(pStrm, "\r\n", 2);
			}
		}
		if (!pszToken[2])
			break;
		pszNow = pszToken + 2;
	}
	if (bIsMain)
	{
		LPCSTR pszIncAtTail = "#include \"ScFunc.cpp\"\r\n";
		IStream_Write(pStrm, pszIncAtTail, lstrlenA(pszIncAtTail));
	}
	ULARGE_INTEGER ulSize;
	IStream_Size(pStrm, &ulSize);
	pszOutput = (LPSTR)HeapAlloc(hHeap, HEAP_ZERO_MEMORY, ulSize.LowPart + 1);
	IStream_Reset(pStrm);
	IStream_Read(pStrm, pszOutput, ulSize.LowPart);
	IUnknown_AtomicRelease((LPVOID *)&pStrm);
	hFile = CreateFileW(pwszDst, GENERIC_WRITE, NULL, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	WriteFile(hFile, pszOutput, ulSize.LowPart, &dwSize, NULL);
	CloseHandle(hFile);
	HeapFree(hHeap, 0, pszOutput);
	HeapFree(hHeap, 0, pszContent);
	return dwRet;
}

DWORD ShellcodingBlock(HANDLE hHeap, LPSTR pszSrc, LPSTR * ppszDst)
{
	DWORD dwRet = ERSC_NOERROR;
	VARS_MAP varsMap;
	__stosb((LPBYTE)&varsMap, 0, sizeof(VARS_MAP));
	PSTR_REPMAP pStrRepMap = NULL;//, pStrVarNowW = NULL, pStrVarFoundW = NULL;
	// Searching /*/dllname.dll#Ord/*/apiname( for dll & api name string in LPSTR format
	ParseModsAPIs(hHeap, &varsMap, pszSrc);
	ParseStrs(hHeap, &varsMap, pszSrc);
	// Use parsed varsMap to convert function block
	LPSTREAM pStrm = SHCreateMemStream(0, 0);
	IStream_Write(pStrm, "{\r\n", 3);
	LPSTR pszInit = GenDwordArrayAndMore(hHeap, &varsMap, &pStrRepMap);
	IStream_Write(pStrm, pszInit, lstrlenA(pszInit));
	HeapFree(hHeap, 0, pszInit);
	//Replace string accroding to pStrRepMap
	LPSTR pszHead = pszSrc, pszTail = pszSrc;
	PSTR_REPMAP pStr2Replace;
	DWORD dwNum2Cmp, dwNum2Copy;
	while (*pszTail)
	{
		pStr2Replace = pStrRepMap;
		while (pStr2Replace)
		{
			dwNum2Cmp = lstrlenA(pStr2Replace->pszOri);
			if (StrCmpNA(pszTail, pStr2Replace->pszOri, dwNum2Cmp) == 0)
				break;
			pStr2Replace = pStr2Replace->pNext;
		}
		if (pStr2Replace)
		{
			BOOL bRep = TRUE;
			if ((pStr2Replace->pszOri[0] != 0x22) && ((pStr2Replace->pszOri[0] != 0x4C) && (pStr2Replace->pszOri[1] != 0x22)))	//Function Name Replace, check if the replaced string in the middle of a variable
			{
				CHAR c2Chk1 = *(pszTail - 1), c2Chk2 = *(pszTail + lstrlenA(pStr2Replace->pszOri));
				if (IsCharAlphaNumericA(c2Chk1) || c2Chk1 == 0x5f || (IsCharAlphaNumericA(c2Chk2) > 0) || c2Chk1 == 0x5f)
					bRep = FALSE;
			}
			if (bRep)
			{
				dwNum2Copy = (DWORD)(pszTail - pszHead);
				IStream_Write(pStrm, pszHead, dwNum2Copy);
				IStream_Write(pStrm, pStr2Replace->pszNew, lstrlenA(pStr2Replace->pszNew));
				pszHead = pszTail + dwNum2Cmp;
				pszTail = pszHead;
			}
			else
				pszTail += 1;
		}
		else
			pszTail += 1;
	}
	dwNum2Copy = (DWORD)(pszTail - pszHead);
	IStream_Write(pStrm, pszHead, dwNum2Copy);
	IStream_Write(pStrm, "}", 1);
	if (*ppszDst)
		HeapFree(hHeap, 0, *ppszDst);
	ULARGE_INTEGER ulSize;
	IStream_Size(pStrm, &ulSize);
	*ppszDst = (LPSTR)HeapAlloc(hHeap, HEAP_ZERO_MEMORY, ulSize.LowPart + 1);
	IStream_Reset(pStrm);
	IStream_Read(pStrm, *ppszDst, ulSize.LowPart);
	IUnknown_AtomicRelease((LPVOID *)&pStrm);
	// After conversion, release the tables used
	ReleaseVarMap(hHeap, &varsMap, &pStrRepMap);

	return dwRet;
}

DWORD ParseModsAPIs(HANDLE hHeap, PVARS_MAP pvars_Map, LPSTR pszSrc)
{
	DWORD dwRet = 0;
	LPSTR pszNow = pszSrc;
	BYTE btHashOptDLLA = 0, btHashOptAPIA = 1;
	while (*pszNow)
	{
		LPSTR pszTail = StrStrA(pszNow, "\r\n");
		DWORD dwLen;
		LPSTR pszLine;
		if (pszTail)
		{
			dwLen = (DWORD)(pszTail - pszNow);
			pszLine = (LPSTR)HeapAlloc(hHeap, HEAP_ZERO_MEMORY, dwLen + 1);
			lstrcpynA(pszLine, pszNow, dwLen + 1);
			pszNow = pszTail + 2;
		}
		else
		{
			dwLen = lstrlenA(pszNow);
			pszLine = (LPSTR)HeapAlloc(hHeap, HEAP_ZERO_MEMORY, dwLen + 1);
			lstrcpyA(pszLine, pszNow);
			pszNow += dwLen;
		}
		LPSTR pszDLL = NULL, pszOrd = NULL, pszAPI = NULL, pszTailAPI = NULL;
		pszDLL = StrStrA(pszLine, "/*/");
		if (pszDLL)
		{
			pszDLL += 3;
			pszAPI = StrStrA(pszDLL, "/*/");
			if (pszAPI)
			{
				DWORD dwOrd = 0;
				pszAPI[0] = '\0';
				pszAPI += 3;
				pszTailAPI = StrChrA(pszAPI, '(');
				if (pszTailAPI)
				{
					pszTailAPI[0] = '\0';
					//All parsed, add
					pszOrd = StrChrA(pszDLL, '#');
					if (pszOrd)
					{
						pszOrd[0] = '\0';
						pszOrd += 1;
						StrTrimA(pszOrd, "\x20\t");
						dwOrd = StrToIntA(pszOrd);
					}
					StrTrimA(pszDLL, "\x20\t");
					StrTrimA(pszAPI, "\x20\t");
					CharLowerA(pszDLL);
					PVAR_MODITEM pVarModItem = FindModInVarMap(pvars_Map, pszDLL);
					if (!pVarModItem)
					{
						pVarModItem = (PVAR_MODITEM)HeapAlloc(hHeap, HEAP_ZERO_MEMORY, sizeof(VAR_MODITEM));
						dwLen = lstrlenA(pszDLL);
						pVarModItem->pszDllName = (LPSTR)HeapAlloc(hHeap, HEAP_ZERO_MEMORY, dwLen + 1);
						lstrcpyA(pVarModItem->pszDllName, pszDLL);
						*StrChrA(pszDLL, '.') = '_';
						if ((lstrcmpA(pVarModItem->pszDllName, "ntdll.dll") == 0) ||
							(lstrcmpA(pVarModItem->pszDllName, "kernel32.dll") == 0) ||
							(lstrcmpA(pVarModItem->pszDllName, "kernelbase.dll") == 0))
							pVarModItem->dwHash = GetStrHash(pVarModItem->pszDllName, (PHASH_OPT)&btHashOptDLLA);
						else
						{
							pVarModItem->pszStrName = (LPSTR)HeapAlloc(hHeap, HEAP_ZERO_MEMORY, dwLen + 5);
							wsprintfA(pVarModItem->pszStrName, "psz_%s", pszDLL);
							pVarModItem->dwLenStr = lstrlenA(pszDLL) + 1;
						}
						pVarModItem->pszModName = (LPSTR)HeapAlloc(hHeap, HEAP_ZERO_MEMORY, dwLen + 5);
						wsprintfA(pVarModItem->pszModName, "mod_%s", pszDLL);
						PVAR_MODITEM pVarModTail = NULL;
						if (pvars_Map->pVar_ModItem)
						{
							pVarModTail = pvars_Map->pVar_ModItem;
							while (pVarModTail->pNext)
								pVarModTail = pVarModTail->pNext;
							pVarModTail->pNext = pVarModItem;
						}
						else
							pvars_Map->pVar_ModItem = pVarModItem;
						pvars_Map->dwNumModItem += 1;
					}
					PVAR_APIITEM pVarApiItem = FindApiInVarMap(pvars_Map, pszAPI);
					if (!pVarApiItem)
					{
						pVarApiItem = (PVAR_APIITEM)HeapAlloc(hHeap, HEAP_ZERO_MEMORY, sizeof(VAR_APIITEM));
						dwLen = lstrlenA(pszAPI);
						pVarApiItem->pszApiName = (LPSTR)HeapAlloc(hHeap, HEAP_ZERO_MEMORY, dwLen + 1);
						lstrcpyA(pVarApiItem->pszApiName, pszAPI);
						if(!pszOrd)
							pVarApiItem->dwHash = GetStrHash(pVarApiItem->pszApiName, (PHASH_OPT)&btHashOptAPIA);
						pVarApiItem->pszFncName = (LPSTR)HeapAlloc(hHeap, HEAP_ZERO_MEMORY, dwLen + 4);
						wsprintfA(pVarApiItem->pszFncName, "fn_%s", pszAPI);
						pVarApiItem->dwOrd = dwOrd;
						pVarApiItem->pVarModItem = pVarModItem;
						PVAR_APIITEM pVarApiTail = NULL;
						if (pvars_Map->pVar_ApiItem)
						{
							pVarApiTail = pvars_Map->pVar_ApiItem;
							while (pVarApiTail->pNext)
								pVarApiTail = pVarApiTail->pNext;
							pVarApiTail->pNext = pVarApiItem;
						}
						else
							pvars_Map->pVar_ApiItem = pVarApiItem;
						pvars_Map->dwNumApiItem += 1;
					}
				}
			}
		}
		HeapFree(hHeap, 0, pszLine);
	}
	return dwRet;
}

DWORD ParseStrs(HANDLE hHeap, PVARS_MAP pvars_Map, LPSTR pszSrc)
{
	DWORD dwRet = 0;
	LPSTR pszNow = pszSrc;
	while (1)
	{
		pszNow = StrChrA(pszNow, 0x22);
		if (pszNow)
		{
			LPSTR pszTail;
			DWORD dwLenHd = 1;
			if (*(pszNow - 1) == 'L')
			{
				pszNow -= 1;
				dwLenHd += 1;
			}
			pszTail = pszNow + dwLenHd + StrCSpnA(pszNow + dwLenHd, "\\\"");
			while (*pszTail != 0x22)
				pszTail = pszTail + 2 + StrCSpnA(pszTail + 2, "\\\"");
			pszTail += 1;
			DWORD dwLen = (DWORD)(pszTail - pszNow);
			LPSTR pszRaw = (LPSTR)HeapAlloc(hHeap, HEAP_ZERO_MEMORY, dwLen + 1);
			lstrcpynA(pszRaw, pszNow, dwLen + 1);
			pszNow = pszTail;
			dwLen = 0;
			LPBYTE pBufData = ConvertRaw2Bin(hHeap, pszRaw, &dwLen);
			if (pszRaw[0] == 'L')
			{
				PVAR_STRITEMW pVarStrItemW = FindStrWInVarMapRaw(pvars_Map, pszRaw);
				if (!pVarStrItemW)
				{
					pVarStrItemW = FindStrWInVarMapParsed(pvars_Map, pBufData, dwLen);
					if (pVarStrItemW)
					{
						PVAR_RAWITEM pVarRaw = pVarStrItemW->pVarRaw;
						while (pVarRaw->pNext)
							pVarRaw = pVarRaw->pNext;
						pVarRaw->pNext = (PVAR_RAWITEM)HeapAlloc(hHeap, HEAP_ZERO_MEMORY, sizeof(VAR_RAWITEM));
						pVarRaw = pVarRaw->pNext;
						pVarRaw->pszRawStr = (LPSTR)HeapAlloc(hHeap, HEAP_ZERO_MEMORY, lstrlenA(pszRaw) + 1);
						lstrcpyA(pVarRaw->pszRawStr, pszRaw);
					}
				}
				if (!pVarStrItemW)
				{
					pVarStrItemW = (PVAR_STRITEMW)HeapAlloc(hHeap, HEAP_ZERO_MEMORY, sizeof(VAR_STRITEMW));
					pVarStrItemW->dwLenData = dwLen;
					pVarStrItemW->pDataParsed = (LPBYTE)HeapAlloc(hHeap, HEAP_ZERO_MEMORY, dwLen);
					__movsb(pVarStrItemW->pDataParsed, pBufData, dwLen);
					pVarStrItemW->dwNumRaw += 1;
					pVarStrItemW->pVarRaw = (PVAR_RAWITEM)HeapAlloc(hHeap, HEAP_ZERO_MEMORY, sizeof(VAR_RAWITEM));
					pVarStrItemW->pVarRaw->pszRawStr = (LPSTR)HeapAlloc(hHeap, HEAP_ZERO_MEMORY, lstrlenA(pszRaw) + 1);
					lstrcpyA(pVarStrItemW->pVarRaw->pszRawStr, pszRaw);
					PVAR_STRITEMW pVarStrTailW = NULL;
					if (pvars_Map->pVar_StrItemW)
					{
						pVarStrTailW = pvars_Map->pVar_StrItemW;
						while (pVarStrTailW->pNext)
							pVarStrTailW = pVarStrTailW->pNext;
						pVarStrTailW->pNext = pVarStrItemW;
					}
					else
						pvars_Map->pVar_StrItemW = pVarStrItemW;
					pvars_Map->dwNumStrItemW += 1;
					wsprintfA(pVarStrItemW->pszStrName, "pwszTmp%04d", pvars_Map->dwNumStrItemW);
				}
			}
			else
			{
				PVAR_STRITEMA pVarStrItemA = FindStrAInVarMapRaw(pvars_Map, pszRaw);
				if (!pVarStrItemA)
				{
					pVarStrItemA = FindStrAInVarMapParsed(pvars_Map, pBufData, dwLen);
					if (pVarStrItemA)
					{
						PVAR_RAWITEM pVarRaw = pVarStrItemA->pVarRaw;
						while (pVarRaw->pNext)
							pVarRaw = pVarRaw->pNext;
						pVarRaw->pNext = (PVAR_RAWITEM)HeapAlloc(hHeap, HEAP_ZERO_MEMORY, sizeof(VAR_RAWITEM));
						pVarRaw = pVarRaw->pNext;
						pVarRaw->pszRawStr = (LPSTR)HeapAlloc(hHeap, HEAP_ZERO_MEMORY, lstrlenA(pszRaw) + 1);
						lstrcpyA(pVarRaw->pszRawStr, pszRaw);
					}
				}
				if (!pVarStrItemA)
				{
					pVarStrItemA = (PVAR_STRITEMA)HeapAlloc(hHeap, HEAP_ZERO_MEMORY, sizeof(VAR_STRITEMA));
					pVarStrItemA->dwLenData = dwLen;
					pVarStrItemA->pDataParsed = (LPBYTE)HeapAlloc(hHeap, HEAP_ZERO_MEMORY, dwLen);
					__movsb(pVarStrItemA->pDataParsed, pBufData, dwLen);
					pVarStrItemA->dwNumRaw += 1;
					pVarStrItemA->pVarRaw = (PVAR_RAWITEM)HeapAlloc(hHeap, HEAP_ZERO_MEMORY, sizeof(VAR_RAWITEM));
					pVarStrItemA->pVarRaw->pszRawStr = (LPSTR)HeapAlloc(hHeap, HEAP_ZERO_MEMORY, lstrlenA(pszRaw) + 1);
					lstrcpyA(pVarStrItemA->pVarRaw->pszRawStr, pszRaw);
					PVAR_STRITEMA pVarStrTailA = NULL;
					if (pvars_Map->pVar_StrItemA)
					{
						pVarStrTailA = pvars_Map->pVar_StrItemA;
						while (pVarStrTailA->pNext)
							pVarStrTailA = pVarStrTailA->pNext;
						pVarStrTailA->pNext = pVarStrItemA;
					}
					else
						pvars_Map->pVar_StrItemA = pVarStrItemA;
					pvars_Map->dwNumStrItemA += 1;
					wsprintfA(pVarStrItemA->pszStrName, "pszTmp%04d", pvars_Map->dwNumStrItemA);
				}
			}
			HeapFree(hHeap, 0, pszRaw);
			pszNow = pszTail;
		}
		else
			break;
	}
	return dwRet;
}

LPBYTE ConvertRaw2Bin(HANDLE hHeap, LPSTR pszRaw, LPDWORD pdwLen)
{
	LPBYTE pBufRet = NULL;
	ULARGE_INTEGER ulStrSize;
	WORD wVal;
	LPSTREAM pStrStrm = SHCreateMemStream(0, 0);
	BOOL bWide = (pszRaw[0] == 'L');
	LPSTR pszHead = pszRaw + (bWide ? 2 : 1);
	DWORD dwLen = lstrlenA(pszHead);
	LPSTR psz2Parse = (LPSTR)HeapAlloc(hHeap, HEAP_ZERO_MEMORY, dwLen);
	lstrcpynA(psz2Parse, pszHead, dwLen);
	pszHead = psz2Parse;
	while (*pszHead)
	{
		wVal = (WORD)*pszHead;
		if (wVal == 0x5c)	//Escape Character, Translate
		{
			pszHead += 1;
			wVal = (WORD)*pszHead;
			switch (wVal)
			{
			case 0x61:	//\a
				wVal = 0x07;
				break;
			case 0x62:	//\b
				wVal = 0x08;
				break;
			case 0x65:	//\e
				wVal = 0x1b;
				break;
			case 0x66:	//\f
				wVal = 0x0c;
				break;
			case 0x6e:	//\n
				wVal = 0x0a;
				break;
			case 0x72:	//\r
				wVal = 0x0d;
				break;
			case 0x74:	//\t
				wVal = 0x09;
				break;
			case 0x76:	//\v
				wVal = 0x0b;
				break;
			case 0x78:	//\x
				wVal = 0;
				while (IsCharHexA(*(pszHead + 1)))
				{
					pszHead += 1;
					wVal <<= 4;
					wVal += (WORD)(IsCharAlphaA(*pszHead) ? (IsCharLowerA(*pszHead) ? *pszHead - 0x57 : *pszHead - 0x37) : *pszHead - 0x30);
				}
				break;
			case 0x30:
				wVal = 0;
				break;
			}
		}
		IStream_Write(pStrStrm, &wVal, bWide ? 2 : 1);
		pszHead += 1;
	}
	wVal = 0;
	IStream_Write(pStrStrm, &wVal, bWide ? 2 : 1);
	IStream_Size(pStrStrm, &ulStrSize);
	if (ulStrSize.LowPart)
	{
		*pdwLen = ulStrSize.LowPart;
		pBufRet = (LPBYTE)HeapAlloc(hHeap, HEAP_ZERO_MEMORY, *pdwLen);
		IStream_Reset(pStrStrm);
		IStream_Read(pStrStrm, pBufRet, *pdwLen);
	}
	IUnknown_AtomicRelease((LPVOID *)&pStrStrm);
	HeapFree(hHeap, 0, psz2Parse);
	return pBufRet;
}

LPSTR GenDwordArrayAndMore(HANDLE hHeap, PVARS_MAP pvars_Map, PSTR_REPMAP * ppstr_RepMap)	//Modify this next to Initialize DWORD array that stored LPSTR & LPWSTR
{
	CHAR pszStr[MAX_PATH];
	DWORD dwIdx, dwNum;
	LPSTR pszRet = NULL;
	PSTR_REPMAP pStr2RepRoot = NULL, pStr2RepNow = NULL;
	LPSTREAM pStrm = SHCreateMemStream(0, 0);
	PVAR_MODITEM pVarModItem = pvars_Map->pVar_ModItem;
	// for pVarModItem, no string replacement needed
	for (dwIdx = 0; dwIdx < pvars_Map->dwNumModItem; dwIdx++)
	{
		if (pVarModItem->pszStrName)
			IStream_Write(pStrm, pVarModItem->pszDllName, pVarModItem->dwLenStr);
		pVarModItem = pVarModItem->pNext;
	}
	PVAR_APIITEM pVarApiItem = pvars_Map->pVar_ApiItem;
	// for pVarApiItem, only string replacement needed
	for (dwIdx = 0; dwIdx < pvars_Map->dwNumApiItem; dwIdx++)
	{
		if (pStr2RepRoot)
		{
			pStr2RepNow->pNext = (PSTR_REPMAP)HeapAlloc(hHeap, HEAP_ZERO_MEMORY, sizeof(STR_REPMAP));
			pStr2RepNow = pStr2RepNow->pNext;
		}
		else
		{
			pStr2RepNow = (PSTR_REPMAP)HeapAlloc(hHeap, HEAP_ZERO_MEMORY, sizeof(STR_REPMAP));
			pStr2RepRoot = pStr2RepNow;
		}
		pStr2RepNow->pszOri = pVarApiItem->pszApiName;
		pStr2RepNow->pszNew = pVarApiItem->pszFncName;
		pVarApiItem = pVarApiItem->pNext;
	}
	// for pVarStrItemA, both array initializtion & string replacement needed
	PVAR_STRITEMA pVarStrItemA = pvars_Map->pVar_StrItemA;
	for (dwIdx = 0; dwIdx < pvars_Map->dwNumStrItemA; dwIdx++)
	{
		IStream_Write(pStrm, pVarStrItemA->pDataParsed, pVarStrItemA->dwLenData);
		PVAR_RAWITEM pVarRawItem = pVarStrItemA->pVarRaw;
		while (pVarRawItem)
		{
			if (pStr2RepRoot)
			{
				pStr2RepNow->pNext = (PSTR_REPMAP)HeapAlloc(hHeap, HEAP_ZERO_MEMORY, sizeof(STR_REPMAP));
				pStr2RepNow = pStr2RepNow->pNext;
			}
			else
			{
				pStr2RepNow = (PSTR_REPMAP)HeapAlloc(hHeap, HEAP_ZERO_MEMORY, sizeof(STR_REPMAP));
				pStr2RepRoot = pStr2RepNow;
			}
			pStr2RepNow->pszOri = pVarRawItem->pszRawStr;
			pStr2RepNow->pszNew = pVarStrItemA->pszStrName;
			pVarRawItem = pVarRawItem->pNext;
		}
		pVarStrItemA = pVarStrItemA->pNext;
	}
	// for pVarStrItemW, both array initializtion & string replacement needed
	PVAR_STRITEMW pVarStrItemW = pvars_Map->pVar_StrItemW;
	for (dwIdx = 0; dwIdx < pvars_Map->dwNumStrItemW; dwIdx++)
	{
		IStream_Write(pStrm, pVarStrItemW->pDataParsed, pVarStrItemW->dwLenData);
		PVAR_RAWITEM pVarRawItem = pVarStrItemW->pVarRaw;
		while (pVarRawItem)
		{
			if (pStr2RepRoot)
			{
				pStr2RepNow->pNext = (PSTR_REPMAP)HeapAlloc(hHeap, HEAP_ZERO_MEMORY, sizeof(STR_REPMAP));
				pStr2RepNow = pStr2RepNow->pNext;
			}
			else
			{
				pStr2RepNow = (PSTR_REPMAP)HeapAlloc(hHeap, HEAP_ZERO_MEMORY, sizeof(STR_REPMAP));
				pStr2RepRoot = pStr2RepNow;
			}
			pStr2RepNow->pszOri = pVarRawItem->pszRawStr;
			pStr2RepNow->pszNew = pVarStrItemW->pszStrName;
			pVarRawItem = pVarRawItem->pNext;
		}
		pVarStrItemW = pVarStrItemW->pNext;
	}
	// Read buffer for dword array initialization
	ULARGE_INTEGER ulSize;
	IStream_Size(pStrm, &ulSize);
	DWORD dwSize = ulSize.LowPart;
	while (dwSize % 4)
		dwSize += 1;
	LPDWORD pdwData = (LPDWORD)HeapAlloc(hHeap, HEAP_ZERO_MEMORY, dwSize);
	IStream_Reset(pStrm);
	IStream_Read(pStrm, pdwData, ulSize.LowPart);
	IUnknown_AtomicRelease((LPVOID *)&pStrm);
	//Convert DWORD array to c code, and calculate psz or pwsz offset according to STR_REPMAP
	dwNum = dwSize / 4;
	pStrm = SHCreateMemStream(0, 0);
	wsprintfA(pszStr, "\tDWORD pdwArray[%d]=\r\n\t{", dwNum);
	IStream_Write(pStrm, pszStr, lstrlenA(pszStr));
	for (dwIdx = 0; dwIdx<dwNum; dwIdx++)
	{
		wsprintfA(pszStr, "%s%s0x%08x", dwIdx ? "," : "", dwIdx % 4 ? " " : "\r\n\t\t", pdwData[dwIdx]);
		IStream_Write(pStrm, pszStr, lstrlenA(pszStr));
	}
	wsprintfA(pszStr, "\r\n\t};\r\n\tLPBYTE pByteDword = (LPBYTE)pdwArray;\r\n");
	IStream_Write(pStrm, pszStr, lstrlenA(pszStr));
	pVarModItem = pvars_Map->pVar_ModItem;
	// for pVarModItem, string initialization needed (only 3 excepted)
	for (dwIdx = 0; dwIdx < pvars_Map->dwNumModItem; dwIdx++)
	{
		if (pVarModItem->pszStrName)
		{
			wsprintfA(pszStr, "\tLPSTR %s = (LPSTR)pByteDword;\r\n\tpByteDword += %d;\r\n", pVarModItem->pszStrName, pVarModItem->dwLenStr);
			IStream_Write(pStrm, pszStr, lstrlenA(pszStr));
			wsprintfA(pszStr, "\tHMODULE %s = GetModuleByName(%s);\r\n", pVarModItem->pszModName, pVarModItem->pszStrName);
			IStream_Write(pStrm, pszStr, lstrlenA(pszStr));
		}
		else
		{
			wsprintfA(pszStr, "\tHMODULE %s = GetModuleByHash(0x%08x);\r\n", pVarModItem->pszModName, pVarModItem->dwHash);
			IStream_Write(pStrm, pszStr, lstrlenA(pszStr));
		}
		pVarModItem = pVarModItem->pNext;
	}
	pVarApiItem = pvars_Map->pVar_ApiItem;
	// for pVarApiItem, only string replacement needed
	for (dwIdx = 0; dwIdx < pvars_Map->dwNumApiItem; dwIdx++)
	{
		wsprintfA(pszStr, "\tdecltype(&%s) %s = (decltype(&%s))\r\n\t\tGetFuncAddrByHashOrd(%s, 0x%08x, %d);\r\n",
			pVarApiItem->pszApiName, pVarApiItem->pszFncName, pVarApiItem->pszApiName, pVarApiItem->pVarModItem->pszModName, pVarApiItem->dwHash, pVarApiItem->dwOrd);
		IStream_Write(pStrm, pszStr, lstrlenA(pszStr));
		pVarApiItem = pVarApiItem->pNext;
	}
	// for pVarStrItemA, both array initializtion & string replacement needed
	pVarStrItemA = pvars_Map->pVar_StrItemA;
	for (dwIdx = 0; dwIdx < pvars_Map->dwNumStrItemA; dwIdx++)
	{
		wsprintfA(pszStr, "\tLPSTR %s = (LPSTR)pByteDword;\r\n\tpByteDword += %d;\r\n", pVarStrItemA->pszStrName, pVarStrItemA->dwLenData);
		IStream_Write(pStrm, pszStr, lstrlenA(pszStr));
		pVarStrItemA = pVarStrItemA->pNext;
	}
	// for pVarStrItemW, both array initializtion & string replacement needed
	pVarStrItemW = pvars_Map->pVar_StrItemW;
	for (dwIdx = 0; dwIdx < pvars_Map->dwNumStrItemW; dwIdx++)
	{
		wsprintfA(pszStr, "\tLPWSTR %s = (LPWSTR)pByteDword;\r\n\tpByteDword += %d;\r\n", pVarStrItemW->pszStrName, pVarStrItemW->dwLenData);
		IStream_Write(pStrm, pszStr, lstrlenA(pszStr));
		pVarStrItemW = pVarStrItemW->pNext;
	}
	IStream_Size(pStrm, &ulSize);
	pszRet = (LPSTR)HeapAlloc(hHeap, HEAP_ZERO_MEMORY, ulSize.LowPart + 1);
	IStream_Reset(pStrm);
	IStream_Read(pStrm, pszRet, ulSize.LowPart);
	IUnknown_AtomicRelease((LPVOID *)&pStrm);
	// After 
	*ppstr_RepMap = pStr2RepRoot;
	return pszRet;
}

VOID ReleaseVarMap(HANDLE hHeap, PVARS_MAP pvars_Map, PSTR_REPMAP * ppstr_RepMap)
{
	LPVOID pBuf2Release;
	PSTR_REPMAP pStr2Rep=*ppstr_RepMap;
	while (pStr2Rep)
	{
		pBuf2Release = (LPVOID)pStr2Rep;
		pStr2Rep = pStr2Rep->pNext;
		HeapFree(hHeap, 0, pBuf2Release);
	}
	*ppstr_RepMap = NULL;
	PVAR_STRITEMW pStrW = pvars_Map->pVar_StrItemW;
	while (pStrW)
	{
		pBuf2Release = (LPVOID)pStrW;
		pStrW = pStrW->pNext;
		HeapFree(hHeap, 0, pBuf2Release);
	}
	PVAR_STRITEMA pStrA = pvars_Map->pVar_StrItemA;
	while (pStrA)
	{
		pBuf2Release = (LPVOID)pStrA;
		pStrA = pStrA->pNext;
		HeapFree(hHeap, 0, pBuf2Release);
	}
	PVAR_APIITEM pApi = pvars_Map->pVar_ApiItem;
	while (pApi)
	{
		pBuf2Release = (LPVOID)pApi;
		pApi = pApi->pNext;
		HeapFree(hHeap, 0, pBuf2Release);
	}
	PVAR_MODITEM pMod = pvars_Map->pVar_ModItem;
	while (pMod)
	{
		pBuf2Release = (LPVOID)pMod;
		pMod = pMod->pNext;
		HeapFree(hHeap, 0, pBuf2Release);
	}
	__stosb((LPBYTE)pvars_Map, 0, sizeof(VARS_MAP));
}

PVAR_MODITEM FindModInVarMap(PVARS_MAP pvars_Map, LPSTR pszDllName)
{
	PVAR_MODITEM pFound = pvars_Map->pVar_ModItem;
	while (pFound)
	{
		if (!lstrcmpA(pFound->pszDllName, pszDllName))
			break;
		pFound = pFound->pNext;
	}
	return pFound;
}

PVAR_APIITEM FindApiInVarMap(PVARS_MAP pvars_Map, LPSTR pszApiName)
{
	PVAR_APIITEM pFound = pvars_Map->pVar_ApiItem;
	while (pFound)
	{
		if (!lstrcmpA(pFound->pszApiName, pszApiName))
			break;
		pFound = pFound->pNext;
	}
	return pFound;
}

PVAR_STRITEMA FindStrAInVarMapRaw(PVARS_MAP pvars_Map, LPSTR pszRawStr)
{
	PVAR_STRITEMA pFound = pvars_Map->pVar_StrItemA;
	BOOL bFound = FALSE;
	while (pFound)
	{
		PVAR_RAWITEM pRawItem = pFound->pVarRaw;
		while (pRawItem)
		{
			if (!lstrcmpA(pRawItem->pszRawStr, pszRawStr))
			{
				bFound = TRUE;
				break;
			}
			pRawItem = pRawItem->pNext;
		}
		if (bFound)
			break;
		pFound = pFound->pNext;
	}
	return pFound;
}

PVAR_STRITEMA FindStrAInVarMapParsed(PVARS_MAP pvars_Map, LPBYTE pBufData, DWORD dwLen)
{
	PVAR_STRITEMA pFound = pvars_Map->pVar_StrItemA;
	while (pFound)
	{
		if (!memcmp(pFound->pDataParsed, pBufData, dwLen))
			break;
		pFound = pFound->pNext;
	}
	return pFound;
}

PVAR_STRITEMW FindStrWInVarMapRaw(PVARS_MAP pvars_Map, LPSTR pszRawStr)
{
	PVAR_STRITEMW pFound = pvars_Map->pVar_StrItemW;
	BOOL bFound = FALSE;
	while (pFound)
	{
		PVAR_RAWITEM pRawItem = pFound->pVarRaw;
		while (pRawItem)
		{
			if (!lstrcmpA(pRawItem->pszRawStr, pszRawStr))
			{
				bFound = TRUE;
				break;
			}
			pRawItem = pRawItem->pNext;
		}
		if (bFound)
			break;
		pFound = pFound->pNext;
	}
	return pFound;
}

PVAR_STRITEMW FindStrWInVarMapParsed(PVARS_MAP pvars_Map, LPBYTE pBufData, DWORD dwLen)
{
	PVAR_STRITEMW pFound = pvars_Map->pVar_StrItemW;
	while (pFound)
	{
		if (!memcmp(pFound->pDataParsed, pBufData, dwLen))
			break;
		pFound = pFound->pNext;
	}
	return pFound;
}