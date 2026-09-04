#include "HashFunc.cpp"

LPBYTE WINAPI GetFuncAddrByForwardStr(LPSTR);

HMODULE WINAPI GetModuleByHash(DWORD dwMod2Find)
{
    HMODULE modRet = 0;
    BYTE byteOpt = 2;
    PPEB pPEB = (PPEB)
#if defined(_M_X64)
        __readgsqword(0x60);
#else
        __readfsdword(0x30);
#endif
    PLIST_ENTRY pListEnd = &pPEB->Ldr->InInitializationOrderModuleList,pListNow = pListEnd->Flink;
    while(pListNow!=pListEnd)
    {
        PLDR_DATA_TABLE_ENTRY_INIT pEntryInit=(PLDR_DATA_TABLE_ENTRY_INIT)pListNow;
        DWORD mod2Comp = GetStrHash((LPSTR)pEntryInit->BaseDllName.Buffer, (PHASH_OPT)&byteOpt);
        if (dwMod2Find == mod2Comp)
        {
            modRet = (HMODULE)pEntryInit->DllBase;
            break;
        }
        pListNow=pListNow->Flink;
    }
    return modRet;
}

LPBYTE WINAPI GetFuncAddrByForwardStr(LPSTR pszDLLdotAPI)
{
    LPBYTE pRet = 0;
    CHAR pszDLL[MAX_PATH], chCmp;
    LPSTR pszDst = pszDLL;
    __stosb((LPBYTE)pszDst, 0, MAX_PATH);
    do
    {
        chCmp = *pszDLLdotAPI;
        *pszDst = chCmp;
        pszDLLdotAPI += 1;
        pszDst += 1;
    } while (chCmp != 0x2e);
    *((LPDWORD)pszDst) = 0x6c6c64;
    HMODULE dwDLL = GetModuleByName(pszDLL);
    if (dwDLL)
    {
        if (*pszDLLdotAPI != 0x23)
            pRet = GetFuncAddrByName(dwDLL, pszDLLdotAPI);
        else
        {
            pszDLLdotAPI += 1;
            USHORT wOrd = 0;
            while (*pszDLLdotAPI)
            {
                wOrd *= 10;
                wOrd += (*pszDLLdotAPI - 0x30);
                pszDLLdotAPI += 1;
            }
            pRet = GetFuncAddrByHashOrd(dwDLL, 0, wOrd);
        }
    }
    return pRet;
}

LPBYTE WINAPI GetFuncAddrByHashOrd(HMODULE module, DWORD dwApi2Find, WORD wOrdinal)
{
    BYTE byteOpt = 1;
    LPBYTE pRet = 0;
    LPBYTE pModBase = (LPBYTE)module;
    PIMAGE_NT_HEADERS ntHeaders = (PIMAGE_NT_HEADERS)(pModBase + ((PIMAGE_DOS_HEADER)module)->e_lfanew);
    PIMAGE_DATA_DIRECTORY expDir = &ntHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT];
    PIMAGE_EXPORT_DIRECTORY ied = (PIMAGE_EXPORT_DIRECTORY)(pModBase + expDir->VirtualAddress);
    LPBYTE pExpHead = pModBase + expDir->VirtualAddress, pExpTail = pExpHead + expDir->Size;
    LPDWORD pAddresOfFunction = (LPDWORD)(pModBase + ied->AddressOfFunctions);
    if (dwApi2Find)
    {
        LPDWORD pAddressName = (LPDWORD)(pModBase + ied->AddressOfNames);
        LPWORD pAddressOfNameOrdinals = (LPWORD)(pModBase + ied->AddressOfNameOrdinals);
        for (DWORD index = 0; index < (ied->NumberOfNames); index++)
        {
            LPSTR pFunc = (LPSTR)(pModBase + *pAddressName);
            if (dwApi2Find == GetStrHash(pFunc, (PHASH_OPT)&byteOpt))
            {
                wOrdinal = *pAddressOfNameOrdinals;
                break;
            }
            pAddressName++;
            pAddressOfNameOrdinals++;
        }
    }
    else
        wOrdinal -= (USHORT)ied->Base;
    pRet = pModBase + pAddresOfFunction[wOrdinal];
    if ((pRet >= pExpHead) && (pRet < pExpTail))
    {
        LPSTR pszDLLdotAPI = (LPSTR)pRet;
        pRet = GetFuncAddrByForwardStr(pszDLLdotAPI);
    }
    return pRet;
}

LPBYTE WINAPI GetFuncAddrByName(HMODULE module, LPSTR funcName)
{
    BYTE byteOpt = 1;
    return GetFuncAddrByHashOrd(module, GetStrHash(funcName, (PHASH_OPT)&byteOpt),0);
}

HMODULE WINAPI GetModuleByName(LPSTR pLibName)
{
    BYTE byteOpt = 0;
    HMODULE modRet = NULL;
    modRet = GetModuleByHash(GetStrHash(pLibName, (PHASH_OPT)&byteOpt));
    if (!modRet)
    {
        HMODULE dwK32 = GetModuleByHash(0x63d37afa);
        decltype(&LoadLibraryA) fn_LoadLibraryA = (decltype(&LoadLibraryA))GetFuncAddrByHashOrd(dwK32,0xd2f7afac,0);
        modRet = fn_LoadLibraryA(pLibName);
    }
    return modRet;
}