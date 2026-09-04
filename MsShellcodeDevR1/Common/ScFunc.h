#include "HashFunc.h"

typedef struct _UNICODE_STRING
{
    USHORT Length;
    USHORT MaximumLength;
    PWSTR Buffer;
} UNICODE_STRING, *PUNICODE_STRING;

typedef struct _PEB_LDR_DATA
{
    ULONG Length;
    UCHAR Initialized[4];
    PVOID SsHandle;
    LIST_ENTRY InLoadOrderModuleList;
    LIST_ENTRY InMemoryOrderModuleList;
    LIST_ENTRY InInitializationOrderModuleList;
} PEB_LDR_DATA, *PPEB_LDR_DATA;

typedef struct _PEB
{
    UCHAR InheritedAddressSpace;
    UCHAR ReadImageFileExecOptions;
    UCHAR BeingDebugged;
    UCHAR BitField;
#ifdef _WIN64
    ULONG Dummy;
#endif
    UINT_PTR Mutant;
    UINT_PTR ImageBaseAddress;
    PPEB_LDR_DATA Ldr;
    UINT_PTR ProcessParameters;
    UINT_PTR SubSystemData;
    HANDLE ProcessHeap;
} PEB, *PPEB;

typedef struct _LDR_DATA_TABLE_ENTRY_LOAD
{
    LIST_ENTRY InLoadOrderLinks;
    LIST_ENTRY InMemoryOrderModuleList;
    LIST_ENTRY InInitializationOrderModuleList;
    UINT_PTR DllBase;
    UINT_PTR EntryPoint;
    ULONG SizeOfImage;
    UNICODE_STRING FullDllName;
    UNICODE_STRING BaseDllName;
} LDR_DATA_TABLE_ENTRY_LOAD, *PLDR_DATA_TABLE_ENTRY_LOAD;

typedef struct _LDR_DATA_TABLE_ENTRY_MEM
{
    LIST_ENTRY InMemoryOrderModuleList;
    LIST_ENTRY InInitializationOrderModuleList;
    UINT_PTR DllBase;
    UINT_PTR EntryPoint;
    ULONG SizeOfImage;
    UNICODE_STRING FullDllName;
    UNICODE_STRING BaseDllName;
} LDR_DATA_TABLE_ENTRY_MEM, *PLDR_DATA_TABLE_ENTRY_MEM;

typedef struct _LDR_DATA_TABLE_ENTRY_INIT
{
    LIST_ENTRY InInitializationOrderModuleList;
    UINT_PTR DllBase;
    UINT_PTR EntryPoint;
    ULONG SizeOfImage;
    UNICODE_STRING FullDllName;
    UNICODE_STRING BaseDllName;
} LDR_DATA_TABLE_ENTRY_INIT, *PLDR_DATA_TABLE_ENTRY_INIT;

extern "C" HMODULE WINAPI GetModuleByHash(DWORD dwMod2Find);
extern "C" HMODULE WINAPI GetModuleByName(LPSTR pLibName);
extern "C" LPBYTE WINAPI GetFuncAddrByName(HMODULE module, LPSTR funcName);
extern "C" LPBYTE WINAPI GetFuncAddrByHashOrd(HMODULE module, DWORD dwApi2Find, WORD wOrdinal);
