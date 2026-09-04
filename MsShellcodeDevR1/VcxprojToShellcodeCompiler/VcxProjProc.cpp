#include "framework.h"
#include "VcxprojToShellcodeCompiler.h"
#include "VcxProjProc.h"
#include "CppProc.h"
#include "DevEnvDetect.h"
#include <Shlwapi.h>

extern HINSTANCE hInst;
LPSTR ExtractRsrc2Buffer(HANDLE, WORD, LPDWORD);

DWORD ParseVcxproj(HANDLE hHeap, PPROP_VCXPROJ pProp_VcxProj, LPWSTR pwszPathVcxproj)
{
	DWORD dwRet = ERSC_NOERROR;
	HANDLE hFile = CreateFileW(pwszPathVcxproj, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	DWORD dwSize = GetFileSize(hFile, NULL);
	LPSTR pszHead, pszTail, pszContent = (LPSTR)HeapAlloc(hHeap, HEAP_ZERO_MEMORY, dwSize + 1);
	PFILE2ADD pFileHdr = NULL, pFileSrc = NULL;
	ReadFile(hFile, pszContent, dwSize, &dwSize, NULL);
	CloseHandle(hFile);
	if (StrStrA(pszContent, "<ResourceCompile Include=\""))
		dwRet = ERSC_HAVINGRSRC;
	else
	{
		pszHead = NULL;
		pszTail = NULL;
		pszHead = StrStrA(pszContent, "<RootNamespace>");
		if (pszHead)
		{
			pszHead += 15;
			pszTail = StrStrA(pszHead, "</RootNamespace>");
			dwSize = (DWORD)(pszTail - pszHead) + 1;
			pProp_VcxProj->pszRootName = (LPSTR)HeapAlloc(hHeap, HEAP_ZERO_MEMORY, dwSize);
			lstrcpynA(pProp_VcxProj->pszRootName, pszHead, dwSize);
		}
		pszHead = NULL;
		pszTail = NULL;
		pszHead = StrStrA(pszContent, "<ClInclude Include=\"");
		while (pszHead)
		{
			pszHead += 20;
			pszTail = StrStrA(pszHead, "\"");
			dwSize = (DWORD)(pszTail - pszHead) + 1;
			if (StrCmpNIA(pszHead, "stdafx.h", 8) &&
				StrCmpNIA(pszHead, "Resource.h", 10) &&
				StrCmpNIA(pszHead, "targetver.h", 11) &&
				StrCmpNIA(pszHead, "pch.h", 5) &&
				StrCmpNIA(pszHead, "framework.h", 11))
			{
				if (pFileHdr)
				{
					pFileHdr->pNext = (PFILE2ADD)HeapAlloc(hHeap, HEAP_ZERO_MEMORY, sizeof(FILE2ADD));
					pFileHdr = pFileHdr->pNext;
				}
				else
				{
					pFileHdr = (PFILE2ADD)HeapAlloc(hHeap, HEAP_ZERO_MEMORY, sizeof(FILE2ADD));
					pProp_VcxProj->pHeaders = pFileHdr;
				}
				pFileHdr->pszName = (LPSTR)HeapAlloc(hHeap, HEAP_ZERO_MEMORY, dwSize);
				lstrcpynA(pFileHdr->pszName, pszHead, dwSize);
			}
			pszHead = StrStrA(pszHead, "<ClInclude Include=\"");
		}
		pszHead = NULL;
		pszTail = NULL;
		pszHead = StrStrA(pszContent, "<ClCompile Include=\"");
		while (pszHead)
		{
			pszHead += 20;
			pszTail = StrStrA(pszHead, "\"");
			dwSize = (DWORD)(pszTail - pszHead) + 1;
			if (StrCmpNIA(pszHead, "stdafx.cpp", 10) &&
				StrCmpNIA(pszHead, "pch.cpp", 7))
			{
				if (pFileSrc)
				{
					pFileSrc->pNext = (PFILE2ADD)HeapAlloc(hHeap, HEAP_ZERO_MEMORY, sizeof(FILE2ADD));
					pFileSrc = pFileSrc->pNext;
				}
				else
				{
					pFileSrc = (PFILE2ADD)HeapAlloc(hHeap, HEAP_ZERO_MEMORY, sizeof(FILE2ADD));
					pProp_VcxProj->pSources = pFileSrc;
				}
				pFileSrc->pszName = (LPSTR)HeapAlloc(hHeap, HEAP_ZERO_MEMORY, dwSize);
				lstrcpynA(pFileSrc->pszName, pszHead, dwSize);
			}
			pszHead = StrStrA(pszHead, "<ClCompile Include=\"");
		}
	}
	HeapFree(hHeap, 0, pszContent);
	return dwRet;
}

VOID ConvertVcxproj(HANDLE hHeap, DWORD dwVerDevEnv, PERR_MAP pErrMap, LPSTR pszSDK, LPWSTR pwszSrc, LPWSTR pwszDst, LPWSTR pwszBatch, LPWSTR pwszOut, BOOL bChkX86, BOOL bChkX64)
{
	DWORD dwLen = 0, dwErr;
	PROP_VCXPROJ propVcxProj;
	PFILE2ADD pFileNow, pFileNext;
	__stosb((LPBYTE)&propVcxProj, 0, sizeof(PROP_VCXPROJ));
	pErrMap->dwErrNo = ParseVcxproj(hHeap, &propVcxProj, pwszSrc);
	if (!pErrMap->dwErrNo)
	{
		if (propVcxProj.pszRootName && propVcxProj.pHeaders && propVcxProj.pSources)
		{
			HANDLE hFile;
			PERR_CODE pErrCodeNow = NULL;
			LPSTR pszContent;
			DWORD dwSizeContent = 0;
			dwLen = lstrlenW(pwszOut);
			if (dwLen)
			{
				if (propVcxProj.pszRootName)
					HeapFree(hHeap, 0, propVcxProj.pszRootName);
				propVcxProj.pszRootName = (LPSTR)HeapAlloc(hHeap, HEAP_ZERO_MEMORY, dwLen + 1);
				wsprintfA(propVcxProj.pszRootName, "%S", pwszOut);
			}
			else
				wsprintfW(pwszOut, L"%S", propVcxProj.pszRootName);
			PathRemoveFileSpecW(pwszSrc);
			DeletePath(hHeap, pwszDst);
			CreateDirectoryW(pwszDst, NULL);
			{
				//Export vcxproj & vcxproj.filters
				LPSTR
					pszVcxproj = ExtractRsrc2Buffer(hHeap, IDR_VCXPROJ, &dwSizeContent),
					pszFilters = ExtractRsrc2Buffer(hHeap, IDR_VCXPROJ_FILTERS, &dwSizeContent);
				ExportVcxproj(hHeap, &propVcxProj, dwVerDevEnv, pszSDK, pwszDst, pszVcxproj, pszFilters);
				HeapFree(hHeap, 0, pszFilters);
				HeapFree(hHeap, 0, pszVcxproj);
			}
			// Save framework.h
			lstrcatW(pwszDst, L"\\framework.h");
			hFile = CreateFileW(pwszDst, GENERIC_WRITE, NULL, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
			pszContent = ExtractRsrc2Buffer(hHeap, IDR_FRAMEWORK_H, &dwSizeContent);
			WriteFile(hFile, pszContent, dwSizeContent, &dwLen, NULL);
			CloseHandle(hFile);
			HeapFree(hHeap, 0, pszContent);
			PathRemoveFileSpecW(pwszDst);
			// Save HashFunc.h HashFunc.cpp
			lstrcatW(pwszDst, L"\\HashFunc.h");
			hFile = CreateFileW(pwszDst, GENERIC_WRITE, NULL, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
			pszContent = ExtractRsrc2Buffer(hHeap, IDR_HASH_H, &dwSizeContent);
			WriteFile(hFile, pszContent, dwSizeContent, &dwLen, NULL);
			CloseHandle(hFile);
			HeapFree(hHeap, 0, pszContent);
			PathRemoveExtensionW(pwszDst);
			lstrcatW(pwszDst, L".cpp");
			hFile = CreateFileW(pwszDst, GENERIC_WRITE, NULL, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
			pszContent = ExtractRsrc2Buffer(hHeap, IDR_HASH_CPP, &dwSizeContent);
			WriteFile(hFile, pszContent, dwSizeContent, &dwLen, NULL);
			CloseHandle(hFile);
			HeapFree(hHeap, 0, pszContent);
			PathRemoveFileSpecW(pwszDst);
			// Save ScFunc.h ScFunc.cpp
			lstrcatW(pwszDst, L"\\ScFunc.h");
			hFile = CreateFileW(pwszDst, GENERIC_WRITE, NULL, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
			pszContent = ExtractRsrc2Buffer(hHeap, IDR_SCFUNC_H, &dwSizeContent);
			WriteFile(hFile, pszContent, dwSizeContent, &dwLen, NULL);
			CloseHandle(hFile);
			HeapFree(hHeap, 0, pszContent);
			PathRemoveExtensionW(pwszDst);
			lstrcatW(pwszDst, L".cpp");
			hFile = CreateFileW(pwszDst, GENERIC_WRITE, NULL, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
			pszContent = ExtractRsrc2Buffer(hHeap, IDR_SCFUNC_CPP, &dwSizeContent);
			WriteFile(hFile, pszContent, dwSizeContent, &dwLen, NULL);
			CloseHandle(hFile);
			HeapFree(hHeap, 0, pszContent);
			PathRemoveFileSpecW(pwszDst);
			// Copy header files
			pFileNow = propVcxProj.pHeaders;
			while (pFileNow)
			{
				lstrcatW(pwszSrc, L"\\");
				dwLen = lstrlenW(pwszSrc);
				wsprintfW(pwszSrc + dwLen, L"%S", pFileNow->pszName);
				lstrcatW(pwszDst, L"\\");
				dwLen = lstrlenW(pwszDst);
				wsprintfW(pwszDst + dwLen, L"%S", pFileNow->pszName);
				CopyFileW(pwszSrc, pwszDst, FALSE);
				PathRemoveFileSpecW(pwszSrc);
				PathRemoveFileSpecW(pwszDst);
				pFileNow = pFileNow->pNext;
			}
			// Convert each cpp file for shellcode ready
			pFileNow = propVcxProj.pSources;
			while (pFileNow)
			{
				lstrcatW(pwszSrc, L"\\");
				dwLen = lstrlenW(pwszSrc);
				wsprintfW(pwszSrc + dwLen, L"%S", pFileNow->pszName);
				lstrcatW(pwszDst, L"\\");
				dwLen = lstrlenW(pwszDst);
				wsprintfW(pwszDst + dwLen, L"%S", pFileNow->pszName);
				dwErr = ConvertCppFile(hHeap, pwszSrc, pwszDst);
				if (dwErr)
				{
					if (pErrMap->dwNumErr)
					{
						pErrCodeNow->pNext = (PERR_CODE)HeapAlloc(hHeap, HEAP_ZERO_MEMORY, sizeof(ERR_CODE));
						pErrCodeNow = pErrCodeNow->pNext;
					}
					else
					{
						pErrCodeNow = (PERR_CODE)HeapAlloc(hHeap, HEAP_ZERO_MEMORY, sizeof(ERR_CODE));
						pErrMap->pErrCode = pErrCodeNow;
					}
					pErrMap->dwNumErr += 1;
					pErrCodeNow->pszFile = (LPSTR)HeapAlloc(hHeap, HEAP_ZERO_MEMORY, lstrlenA(pFileNow->pszName) + 1);
					lstrcpyA(pErrCodeNow->pszFile, pFileNow->pszName);
					pErrCodeNow->dwErr = dwErr;
				}
				PathRemoveFileSpecW(pwszSrc);
				PathRemoveFileSpecW(pwszDst);
				pFileNow = pFileNow->pNext;
			}
			// Create Bat to Build Exe & Extract text section
			lstrcatW(pwszDst, L"\\VcxBuild.bat");
			hFile = CreateFileW(pwszDst, GENERIC_WRITE, NULL, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
			PathRemoveFileSpecW(pwszDst);
			LPCSTR psz2Write = "@call \"";
			WriteFile(hFile, psz2Write, lstrlenA(psz2Write), &dwLen, NULL);
			LPSTR pszBatch = (LPSTR)HeapAlloc(hHeap, HEAP_ZERO_MEMORY, lstrlenW(pwszBatch) + 1);
			wsprintfA(pszBatch, "%S", pwszBatch);
			WriteFile(hFile, pszBatch, lstrlenA(pszBatch), &dwLen, NULL);
			HeapFree(hHeap, 0, pszBatch);
			psz2Write = "\"\r\n\r\n";
			WriteFile(hFile, psz2Write, lstrlenA(psz2Write), &dwLen, NULL);
			if (bChkX86)
			{
				psz2Write = "msbuild ";
				WriteFile(hFile, psz2Write, lstrlenA(psz2Write), &dwLen, NULL);
				WriteFile(hFile, propVcxProj.pszRootName, lstrlenA(propVcxProj.pszRootName), &dwLen, NULL);
				psz2Write = ".vcxproj /p:Configuration=Release;Platform=Win32\r\n";
				WriteFile(hFile, psz2Write, lstrlenA(psz2Write), &dwLen, NULL);
			}
			if (bChkX64)
			{
				psz2Write = "msbuild ";
				WriteFile(hFile, psz2Write, lstrlenA(psz2Write), &dwLen, NULL);
				WriteFile(hFile, propVcxProj.pszRootName, lstrlenA(propVcxProj.pszRootName), &dwLen, NULL);
				psz2Write = ".vcxproj /p:Configuration=Release;Platform=x64\r\n";
				WriteFile(hFile, psz2Write, lstrlenA(psz2Write), &dwLen, NULL);
			}
			CloseHandle(hFile);
		}
		else
			pErrMap->dwErrNo = ERSC_BADVCXPROJ;
		if (propVcxProj.pszRootName)
			HeapFree(hHeap, 0, propVcxProj.pszRootName);
		if (propVcxProj.pHeaders)
		{
			pFileNow = propVcxProj.pHeaders, pFileNext = propVcxProj.pHeaders->pNext;
			while (pFileNow)
			{
				HeapFree(hHeap, 0, pFileNow->pszName);
				HeapFree(hHeap, 0, pFileNow);
				pFileNow = pFileNext;
				if (pFileNow)
					pFileNext = pFileNow->pNext;
			}
		}
		if (propVcxProj.pSources)
		{
			pFileNow = propVcxProj.pSources, pFileNext = propVcxProj.pSources->pNext;
			while (pFileNow)
			{
				HeapFree(hHeap, 0, pFileNow->pszName);
				HeapFree(hHeap, 0, pFileNow);
				pFileNow = pFileNext; if (pFileNow)
					pFileNext = pFileNow->pNext;
			}
		}
	}
}

VOID ExportVcxproj(HANDLE hHeap, PPROP_VCXPROJ pProp_VcxProj, DWORD dwVerEnvEnv, LPSTR pszSDK, LPWSTR pwszOutPath, LPSTR pszVcxProj, LPSTR pszVcxFilter)
{
	LPCSTR 
		pszTagEnd = "</",
		pszFilesEnd[2]=
	{
		"\" />",
		"\">",
	},
		pszFilterEnd[2]=
	{
		"      <Filter>Header Files</Filter>\r\n    </ClInclude>\r\n",
		"      <Filter>Source Files</Filter>\r\n    </ClCompile>\r\n"
	},
		pszTag2Scan[7] =
	{
		"<VCProjectVersion>",				//idx=0
		"<PlatformToolset>",				//idx=1
		"<RootNamespace>",					//idx=2
		"<ProjectName>",					//idx=3
		"<WindowsTargetPlatformVersion>",	//idx=4, bNeedSDK
		"<ClInclude Include=\"",			//idx=5, Append Header Files (Keep framework.h)
		"<ClCompile Include=\""				//idx=6, Append Source Files (Keep Nothing)
	};
	CHAR pszTsVerPrj[2][5];
	BOOL bNeedSDK = dwVerEnvEnv > 14 ? TRUE : FALSE;
	wsprintfA(pszTsVerPrj[0], "%d.0", dwVerEnvEnv);
	wsprintfA(pszTsVerPrj[1], "v%d0", dwVerEnvEnv < 14 ? dwVerEnvEnv : 14);
	if (dwVerEnvEnv > 14)
	{
		pszTsVerPrj[1][3] += (dwVerEnvEnv - 14);
		if (dwVerEnvEnv == 18)
			pszTsVerPrj[1][3] += 1;
	}
	DWORD dwSize;
	LPSTR pszNow, pszToken, pszScanned, pszTail;
	PFILE2ADD pFileNow;
	lstrcatW(pwszOutPath, L"\\");
	dwSize = lstrlenW(pwszOutPath);
	//Export vcxproj File using given config
	wsprintfW(pwszOutPath + dwSize, L"%S.vcxproj", pProp_VcxProj->pszRootName);
	HANDLE hFile = CreateFileW(pwszOutPath, GENERIC_WRITE, NULL, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	pszNow = pszVcxProj;
	BOOL bAddCrLf;
	while (1)
	{
		pFileNow = NULL;
		bAddCrLf = TRUE;
		pszToken = StrStrA(pszNow, "\r\n");
		if (pszToken)
		{
			pszToken[0] = '\0';
			pszToken += 2;
		}
		else
			bAddCrLf = FALSE;
		int idxScan;
		for (idxScan = 0; idxScan < 7; idxScan++)
			if (pszScanned = StrStrA(pszNow, pszTag2Scan[idxScan]))
				break;
		if(pszScanned)
			pszScanned += lstrlenA(pszTag2Scan[idxScan]);
		switch (idxScan)
		{
		case 0:
		case 1:
			pszScanned[0] = '\0';
			WriteFile(hFile, pszNow, lstrlenA(pszNow), &dwSize, NULL);
			pszNow = pszScanned + 1;
			WriteFile(hFile, pszTsVerPrj[idxScan], lstrlenA(pszTsVerPrj[idxScan]), &dwSize, NULL);
			pszNow = StrStrA(pszNow, pszTagEnd);
			WriteFile(hFile, pszNow, lstrlenA(pszNow), &dwSize, NULL);
			break;
		case 2:
		case 3:
			pszScanned[0] = '\0';
			WriteFile(hFile, pszNow, lstrlenA(pszNow), &dwSize, NULL);
			pszNow = pszScanned + 1;
			WriteFile(hFile, pProp_VcxProj->pszRootName, lstrlenA(pProp_VcxProj->pszRootName), &dwSize, NULL);
			pszNow = StrStrA(pszNow, pszTagEnd);
			WriteFile(hFile, pszNow, lstrlenA(pszNow), &dwSize, NULL);
			break;
		case 4:
			if (bNeedSDK && pszSDK)
			{
				pszScanned[0] = '\0';
				WriteFile(hFile, pszNow, lstrlenA(pszNow), &dwSize, NULL);
				pszNow = pszScanned + 1;
				WriteFile(hFile, pszSDK, lstrlenA(pszSDK), &dwSize, NULL);
				pszNow = StrStrA(pszNow, pszTagEnd);
				WriteFile(hFile, pszNow, lstrlenA(pszNow), &dwSize, NULL);
			}
			else
				bAddCrLf = FALSE;
			break;
		case 5:
			WriteFile(hFile, pszNow, lstrlenA(pszNow), &dwSize, NULL);
			WriteFile(hFile, "\r\n", 2, &dwSize, NULL);
			pFileNow = pProp_VcxProj->pHeaders;
		case 6:
			if(!pFileNow)
				pFileNow = pProp_VcxProj->pSources;
			pszScanned[0] = '\0';
			pszTail = pszScanned + 1;
			pszTail = StrStrA(pszTail, pszFilesEnd[0]);
			while (pFileNow)
			{
				WriteFile(hFile, pszNow, lstrlenA(pszNow), &dwSize, NULL);
				WriteFile(hFile, pFileNow->pszName, lstrlenA(pFileNow->pszName), &dwSize, NULL);
				WriteFile(hFile, pszTail, lstrlenA(pszTail), &dwSize, NULL);
				WriteFile(hFile, "\r\n", 2, &dwSize, NULL);
				pFileNow = pFileNow->pNext;
			}
			bAddCrLf = FALSE;
			break;
		default:
			WriteFile(hFile, pszNow, lstrlenA(pszNow), &dwSize, NULL);
			break;
		}
		if(bAddCrLf)
			WriteFile(hFile, "\r\n", 2, &dwSize, NULL);
		if (pszToken)
			pszNow = pszToken;
		else
			break;
	}
	CloseHandle(hFile);
	//Export vcxproj.filters File using given config
	int numAdded = 0;
	lstrcatW(pwszOutPath, L".filters");
	hFile = CreateFileW(pwszOutPath, GENERIC_WRITE, NULL, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	pszNow = pszVcxFilter;
	bAddCrLf;
	while (1)
	{
		pFileNow = NULL;
		bAddCrLf = TRUE;
		pszToken = StrStrA(pszNow, "\r\n");
		if (pszToken)
		{
			pszToken[0] = '\0';
			pszToken += 2;
		}
		else
			bAddCrLf = FALSE;
		int idxScan;
		for (idxScan = 5; idxScan < 7; idxScan++)
			if (pszScanned = StrStrA(pszNow, pszTag2Scan[idxScan]))
				break;
		if (pszScanned)
			pszScanned += lstrlenA(pszTag2Scan[idxScan]);
		switch (idxScan)
		{
		case 5:
			WriteFile(hFile, pszNow, lstrlenA(pszNow), &dwSize, NULL);
			WriteFile(hFile, "\r\n", 2, &dwSize, NULL);
			numAdded += 1;
			pFileNow = pProp_VcxProj->pHeaders;
		case 6:
			if (!pFileNow)
				pFileNow = pProp_VcxProj->pSources;
			pszScanned[0] = '\0';
			pszTail = pszScanned + 1;
			pszTail = StrStrA(pszTail, pszFilesEnd[1]);
			while (pFileNow)
			{
				if (numAdded)
					WriteFile(hFile, pszFilterEnd[idxScan - 5], lstrlenA(pszFilterEnd[idxScan - 5]), &dwSize, NULL);
				WriteFile(hFile, pszNow, lstrlenA(pszNow), &dwSize, NULL);
				WriteFile(hFile, pFileNow->pszName, lstrlenA(pFileNow->pszName), &dwSize, NULL);
				WriteFile(hFile, pszTail, lstrlenA(pszTail), &dwSize, NULL);
				WriteFile(hFile, "\r\n", 2, &dwSize, NULL);
				pFileNow = pFileNow->pNext;
				numAdded += 1;
			}
			bAddCrLf = FALSE;
			break;
		default:
			WriteFile(hFile, pszNow, lstrlenA(pszNow), &dwSize, NULL);
			break;
		}
		if (bAddCrLf)
			WriteFile(hFile, "\r\n", 2, &dwSize, NULL);
		if (pszToken)
			pszNow = pszToken;
		else
			break;
	}
	CloseHandle(hFile);
	PathRemoveFileSpecW(pwszOutPath);
	return;
}

LPSTR ExtractRsrc2Buffer(HANDLE hHeap, WORD wRscID, LPDWORD pdwSize)
{
	HRSRC hTextRsrc = FindResourceW(NULL, MAKEINTRESOURCEW(wRscID), L"PROJ_SRC");
	*pdwSize = SizeofResource(NULL, hTextRsrc);
	HGLOBAL hgTextRsrc = LoadResource(NULL, hTextRsrc);
	LPSTR pszTextRsrc = (LPSTR)LockResource(hgTextRsrc);
	LPSTR pszRet = (LPSTR)HeapAlloc(hHeap, HEAP_ZERO_MEMORY, *pdwSize + 1);
	__movsb((LPBYTE)pszRet, (LPBYTE)pszTextRsrc, *pdwSize);
	return pszRet;
}