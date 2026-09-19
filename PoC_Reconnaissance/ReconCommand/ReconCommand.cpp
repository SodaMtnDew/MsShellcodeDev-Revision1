// ReconCommand.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN // Prevents conflicts between windows.h and winsock2.h
#endif
#include <Shlwapi.h>
#include <Windows.h>
#include <WinSock2.h>

#pragma comment(lib, "shlwapi")
#pragma comment(lib, "ws2_32")

#pragma pack(push, 1)
typedef union
{
	CHAR chVal[4];
	DWORD dwVal;
} TYPE_NET, * LPTYPE_NET;

typedef struct packet_Header
{
	DWORD dwMagic[4], dwDummy[4];
} PACKET_HEADER;
#pragma pack(pop)

int main()
{
	DWORD dwSize, dwLen = 0;
	TYPE_NET valPort;
	WCHAR pwszPath[MAX_PATH], pwszFolder[MAX_PATH], pwszFile[MAX_PATH], pwszStr[MAX_PATH];
	GetModuleFileNameW(NULL, pwszPath, MAX_PATH);
	PathRemoveExtensionW(pwszPath);
	lstrcatW(pwszPath, L".ini");
	valPort.dwVal =  GetPrivateProfileIntW(L"Setting", L"ListenPort", 8080, pwszPath);
	PathRemoveFileSpecW(pwszPath);
	HANDLE hHeap = GetProcessHeap(), hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
	if ((valPort.dwVal == 0) || (valPort.dwVal > 65535))
	{
		wsprintfW(pwszStr, L"Error:\n\tAssigned Listening Port %d out of range!\n", valPort.dwVal);
		dwLen = lstrlenW(pwszStr);
		WriteConsoleW(hStdOut, pwszStr, dwLen, &dwLen, NULL);
	}
	else
	{
		WSADATA wsaData;
		if (WSAStartup(MAKEWORD(2, 2), &wsaData) == 0)
		{
			SOCKET sockListen = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
			if (sockListen != INVALID_SOCKET)
			{
				SOCKADDR addrListen =
				{
					AF_INET,{ valPort.chVal[1], valPort.chVal[0], 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
				};
				if (bind(sockListen, &addrListen, sizeof(SOCKADDR)) != SOCKET_ERROR)
				{
					if (listen(sockListen, SOMAXCONN) != SOCKET_ERROR)
					{
						wsprintfW(pwszStr, L"Listening on Port %d\n\n", valPort.dwVal);
						dwLen = lstrlenW(pwszStr);
						WriteConsoleW(hStdOut, pwszStr, dwLen, &dwLen, NULL);
						while (1)
						{
							SOCKADDR addrConnect;
							int addrLen = sizeof(SOCKADDR);
							SOCKET sockAccept = accept(sockListen, &addrConnect, &addrLen);
							if (sockAccept != INVALID_SOCKET)
							{
								PACKET_HEADER pktHdr;
								int iLen = recv(sockAccept, (LPSTR)&pktHdr, sizeof(PACKET_HEADER), 0);
								if (iLen < sizeof(PACKET_HEADER))
								{
									wsprintfW(pwszStr, L"Error:\n\tNot Enough Bytes of Packet Header Received!\n");
									dwLen = lstrlenW(pwszStr);
									WriteConsoleW(hStdOut, pwszStr, dwLen, &dwLen, NULL);
								}
								else
								{
									for (int i = 0;i < 4;i++)
										for (int j = i;j < 4;j++)
											pktHdr.dwMagic[i] ^= pktHdr.dwDummy[j];
									if (pktHdr.dwMagic[2] != 0xdcaf4a32)
									{
										wsprintfW(pwszStr, L"Error:\n\tSignature of Packet Header Not Matched!\n");
										dwLen = lstrlenW(pwszStr);
										WriteConsoleW(hStdOut, pwszStr, dwLen, &dwLen, NULL);
									}
									else
									{
										LPTYPE_NET
											pNetWan = (LPTYPE_NET)(addrConnect.sa_data + 2),
											pNetLan = (LPTYPE_NET)&pktHdr.dwMagic[1];
										SYSTEMTIME st;
										GetSystemTime(&st);
										LPBYTE pBufRecv = (LPBYTE)HeapAlloc(hHeap, HEAP_ZERO_MEMORY, pktHdr.dwMagic[0]);
										ULARGE_INTEGER ulSize;
										int iLen;
										LPSTREAM pStrm = SHCreateMemStream(0, 0);
										do
										{
											iLen = recv(sockAccept, (LPSTR)pBufRecv, 8192, 0);
											if (iLen > 0)
												IStream_Write(pStrm, pBufRecv, iLen);
										} while (iLen > 0);
										IStream_Size(pStrm, &ulSize);
										if(ulSize.LowPart != pktHdr.dwMagic[0])
										{
											//wsprintfW(pwszStr, L"Error:\n\tNot Enough Bytes of Packet Data Received!\n");
											wsprintfW(pwszStr, L"Error:\n\t%u Bytes to Receive, %u Bytes Received!\n", pktHdr.dwMagic[0], ulSize.LowPart);
											dwLen = lstrlenW(pwszStr);
											WriteConsoleW(hStdOut, pwszStr, dwLen, &dwLen, NULL);
										}
										else
										{
											wsprintfW(pwszFolder, L"%u.%u.%u.%u(%u.%u.%u.%u)@%04d%02d%02d-%02d%02d%02d",
												(BYTE)pNetWan->chVal[0], (BYTE)pNetWan->chVal[1], (BYTE)pNetWan->chVal[2], (BYTE)pNetWan->chVal[3],
												(BYTE)pNetLan->chVal[0], (BYTE)pNetLan->chVal[1], (BYTE)pNetLan->chVal[2], (BYTE)pNetLan->chVal[3],
												st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
											IStream_Reset(pStrm);
											IStream_Read(pStrm, pBufRecv, pktHdr.dwMagic[0]);
											wsprintfW(pwszFile, L"%s\\%s", pwszPath, pwszFolder);
											CreateDirectoryW(pwszFile, NULL);
											dwLen = pktHdr.dwMagic[0];
											LPBYTE pBufNow = pBufRecv;
											while (dwLen > 0)
											{
												wsprintfW(pwszFile, L"%s\\%S", pwszFile, (LPSTR)pBufNow);
												pBufNow += 12;
												dwSize = *((LPDWORD)pBufNow);
												pBufNow += 4;
												dwLen -= 16;
												HANDLE hFile = CreateFileW(pwszFile, GENERIC_WRITE, NULL, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
												WriteFile(hFile, pBufNow, dwSize, &dwSize, NULL);
												CloseHandle(hFile);
												pBufNow += dwSize;
												dwLen -= dwSize;
												PathRemoveFileSpecW(pwszFile);
											}
											wsprintfW(pwszStr, L"Result:\n\tReceived Data Save in Folder [%s]\n", pwszFolder);
											dwLen = lstrlenW(pwszStr);
											WriteConsoleW(hStdOut, pwszStr, dwLen, &dwLen, NULL);
										}
										IUnknown_AtomicRelease((LPVOID*)&pStrm);
										HeapFree(hHeap, 0, pBufRecv);
									}
								}
							}
						}
					}
					else
					{
						wsprintfW(pwszStr, L"Error:\n\tPort listening error!\n");
						dwLen = lstrlenW(pwszStr);
						WriteConsoleW(hStdOut, pwszStr, dwLen, &dwLen, NULL);
					}
				}
				else
				{
					wsprintfW(pwszStr, L"Error:\n\tPort binding error!\n");
					dwLen = lstrlenW(pwszStr);
					WriteConsoleW(hStdOut, pwszStr, dwLen, &dwLen, NULL);
				}
				closesocket(sockListen);
			}
			else
			{
				wsprintfW(pwszStr, L"Error:\n\tSocket initilization error!\n");
				dwLen = lstrlenW(pwszStr);
				WriteConsoleW(hStdOut, pwszStr, dwLen, &dwLen, NULL);
			}
			WSACleanup();
		}
		else
		{
			wsprintfW(pwszStr, L"Error:\n\tWSAStartup initilization error!\n");
			dwLen = lstrlenW(pwszStr);
			WriteConsoleW(hStdOut, pwszStr, dwLen, &dwLen, NULL);
		}
	}
	ExitProcess(0);
	return 0;
}