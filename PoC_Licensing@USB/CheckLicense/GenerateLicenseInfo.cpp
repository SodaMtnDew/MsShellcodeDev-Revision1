#include "framework.h"
#include "GenerateLicenseInfo.h"
#include <commdlg.h>
#include <wincrypt.h>
#include <Setupapi.h>
#include <Winioctl.h>

LPSTR GenKeyHexHmac(HANDLE hHeap, LPBYTE pBufSN, DWORD dwLenSN, LPBYTE pBufSrc, DWORD dwLenSrc)
{
	LPBYTE pBufDst = NULL;
	LPSTR pszRet = NULL;
	BOOL bOK = FALSE;
	HCRYPTPROV hProv;
	HCRYPTKEY hHmacKey = NULL;
	HCRYPTHASH hHash = NULL, hHmacHash = NULL;
	HMAC_INFO HmacInfo;
	DWORD dwLenHash = 64;
	DWORD dwKeyBuf[35] =
	{
		0x00000208, CALG_RC2, 0x00000080,
		0xfac2ac7b, 0x8d2d29b4, 0x2297a305, 0x1e8b8613,
		0xfe43f120, 0x9c99751d, 0x0b155c51, 0xaeb192de,
		0x2d35c1b5, 0xa733b84d, 0xfaaaa947, 0x77ad61e2,
		0x9681aab9, 0x18e49815, 0x038f2e23, 0x6a60b932,
		0x11766798, 0x55ea08c7, 0xa306006c, 0x51e7a422,
		0x5c00b806, 0x2262c18a, 0x47129237, 0x145393a6,
		0xc9eb9f46, 0x55c41e59, 0xedda5e23, 0xe5080e3d,
		0x727bf7b7, 0x7974a960, 0xb3bb68be, 0xe27b1150
	};
	bOK = /*/advapi32.dll/*/CryptAcquireContextA(&hProv, NULL, NULL, PROV_RSA_AES, CRYPT_VERIFYCONTEXT);
	if (bOK)
	{
		if (dwLenSN)
		{
			bOK = /*/advapi32.dll/*/CryptCreateHash(hProv, CALG_SHA_512, NULL, 0, &hHash);
			if (bOK)
			{
				/*/advapi32.dll/*/CryptHashData(hHash, pBufSN, dwLenSN, 0);
				dwLenHash = 64;
				/*/advapi32.dll/*/CryptGetHashParam(hHash, HP_HASHVAL, (LPBYTE)(dwKeyBuf + 19), &dwLenHash, 0);
				/*/advapi32.dll/*/CryptDestroyHash(hHash);
			}
		}
		bOK = /*/advapi32.dll/*/CryptImportKey(hProv, (LPBYTE)dwKeyBuf, 140, 0, CRYPT_IPSEC_HMAC_KEY, &hHmacKey);
		if (bOK)
		{
			bOK = /*/advapi32.dll/*/CryptCreateHash(hProv, CALG_HMAC, hHmacKey, 0, &hHmacHash);
			if (bOK)
			{
				/*/ntdll.dll/*/RtlZeroMemory(&HmacInfo, sizeof(HMAC_INFO));
				HmacInfo.HashAlgid = CALG_SHA_512;
				/*/advapi32.dll/*/CryptSetHashParam(hHmacHash, HP_HMAC_INFO, (LPBYTE)&HmacInfo, 0);
				/*/advapi32.dll/*/CryptHashData(hHmacHash, pBufSrc, dwLenSrc, 0);
				dwLenHash = 64;
				pBufDst = (LPBYTE)/*/kernel32.dll/*/HeapAlloc(hHeap, HEAP_ZERO_MEMORY, dwLenHash);
				/*/advapi32.dll/*/CryptGetHashParam(hHmacHash, HP_HASHVAL, pBufDst, &dwLenHash, 0);
				/*/advapi32.dll/*/CryptDestroyHash(hHmacHash);
				pszRet = (LPSTR)HeapAlloc(hHeap, HEAP_ZERO_MEMORY, dwLenHash * 2 + 1);
				for (DWORD dwIdx = 0;dwIdx < dwLenHash;dwIdx++)
					/*/user32.dll/*/wsprintfA(pszRet + 2 * dwIdx, "%02x", pBufDst[dwIdx]);
				/*/kernel32.dll/*/HeapFree(hHeap, 0, pBufDst);
			}
			/*/advapi32.dll/*/CryptDestroyKey(hHmacKey);
		}
	}
	if (hProv)
		/*/advapi32.dll/*/CryptReleaseContext(hProv, 0);
	return pszRet;
}

DWORD GetDriveSN(HANDLE hHeap, WCHAR wchDrvLetter, LPDWORD dwLenSN, LPSTR* pwszSN)
{
	DWORD dwRet = NO_ERROR;
	WCHAR pwszDrivePath[MAX_PATH];
	/*/user32.dll/*/wsprintfW(pwszDrivePath, L"\\\\.\\%c:", wchDrvLetter);
	HANDLE hDevice = /*/kernel32.dll/*/CreateFileW(pwszDrivePath, 0, FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL, OPEN_EXISTING, 0, NULL);
	if (INVALID_HANDLE_VALUE == hDevice)
		return /*/kernel32.dll/*/GetLastError();
	STORAGE_PROPERTY_QUERY storagePropertyQuery;
	/*/ntdll.dll/*/RtlZeroMemory(&storagePropertyQuery, sizeof(STORAGE_PROPERTY_QUERY));
	storagePropertyQuery.PropertyId = StorageDeviceProperty;
	storagePropertyQuery.QueryType = PropertyStandardQuery;
	STORAGE_DESCRIPTOR_HEADER storageDescriptorHeader;
	/*/ntdll.dll/*/RtlZeroMemory(&storageDescriptorHeader, sizeof(STORAGE_DESCRIPTOR_HEADER));
	DWORD dwBytesReturned = 0;
	BOOL bDevIoOK = /*/kernel32.dll/*/DeviceIoControl(hDevice, IOCTL_STORAGE_QUERY_PROPERTY,
		&storagePropertyQuery, sizeof(STORAGE_PROPERTY_QUERY),
		&storageDescriptorHeader, sizeof(STORAGE_DESCRIPTOR_HEADER),
		&dwBytesReturned, NULL);
	if (!bDevIoOK)
	{
		dwRet = /*/kernel32.dll/*/GetLastError();
		/*/kernel32.dll/*/CloseHandle(hDevice);
		return dwRet;
	}
	const DWORD dwOutBufferSize = storageDescriptorHeader.Size;
	LPBYTE pOutBuffer = (LPBYTE)/*/kernel32.dll/*/HeapAlloc(hHeap, HEAP_ZERO_MEMORY, dwOutBufferSize);
	bDevIoOK = /*/kernel32.dll/*/DeviceIoControl(hDevice, IOCTL_STORAGE_QUERY_PROPERTY,
		&storagePropertyQuery, sizeof(STORAGE_PROPERTY_QUERY),
		pOutBuffer, dwOutBufferSize,
		&dwBytesReturned, NULL);
	if (!bDevIoOK)
	{
		dwRet = /*/kernel32.dll/*/GetLastError();
		/*/kernel32.dll/*/HeapFree(hHeap, 0, pOutBuffer);
		/*/kernel32.dll/*/CloseHandle(hDevice);
		return dwRet;
	}
	STORAGE_DEVICE_DESCRIPTOR* pDeviceDescriptor = (STORAGE_DEVICE_DESCRIPTOR*)pOutBuffer;
	DWORD dwSerialNumberOffset = pDeviceDescriptor->SerialNumberOffset;
	if (dwSerialNumberOffset != 0)
	{
		//*bRemovable = pDeviceDescriptor->RemovableMedia;
		while (pOutBuffer[dwSerialNumberOffset] == 0x20)
			dwSerialNumberOffset += 1;
		*dwLenSN = /*/kernel32.dll/*/lstrlenA((LPSTR)(pOutBuffer + dwSerialNumberOffset));
		*pwszSN = (LPSTR)/*/kernel32.dll/*/HeapAlloc(hHeap, HEAP_ZERO_MEMORY, *dwLenSN + 1);
		/*/kernel32.dll/*/lstrcpyA(*pwszSN, (LPSTR)(pOutBuffer + dwSerialNumberOffset));
	}
	/*/kernel32.dll/*/HeapFree(hHeap, 0, pOutBuffer);
	/*/kernel32.dll/*/CloseHandle(hDevice);
	return dwRet;
}