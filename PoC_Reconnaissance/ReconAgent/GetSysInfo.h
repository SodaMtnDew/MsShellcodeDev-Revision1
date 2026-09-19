#pragma once

#ifndef NTAPI
#define NTAPI WINAPI
#endif

#define WINGDIPAPI __stdcall
#define GDIPCONST const

typedef enum
{
	Ok = 0
}GpStatus;

typedef struct {
	UINT32 GdiplusVersion;
	LPVOID DebugEventCallback;
	BOOL   SuppressBackgroundThread;
	BOOL   SuppressExternalCodecs;
} GdiplusStartupInput;

typedef struct {
	LPVOID NotificationHook;
	LPVOID NotificationUnhook;
} GdiplusStartupOutput;

typedef struct
{
	CLSID Clsid;
	GUID  FormatID;
	const WCHAR* CodecName;
	const WCHAR* DllName;
	const WCHAR* FormatDescription;
	const WCHAR* FilenameExtension;
	const WCHAR* MimeType;
	DWORD Flags;
	DWORD Version;
	DWORD SigCount;
	DWORD SigSize;
	const BYTE* SigPattern;
	const BYTE* SigMask;
}ImageCodecInfo;

typedef struct
{
	GUID   Guid;
	ULONG  NumberOfValues;
	ULONG  Type;
	LPVOID Value;
}EncoderParameter;

typedef struct
{
	UINT Count;
	EncoderParameter Parameter[1];
}EncoderParameters;

typedef void GpImage;
typedef void GpBitmap;

extern "C" GpStatus WINGDIPAPI GdiplusStartup(ULONG_PTR*, const GdiplusStartupInput*, GdiplusStartupOutput*);
extern "C" GpStatus WINGDIPAPI GdipCreateBitmapFromHICON(HICON, GpBitmap**);
extern "C" GpStatus WINGDIPAPI GdipGetImageEncodersSize(UINT*, UINT*);
extern "C" GpStatus WINGDIPAPI GdipGetImageEncoders(UINT, UINT, ImageCodecInfo*);
extern "C" GpStatus WINGDIPAPI GdipLoadImageFromStream(IStream*, GpImage**);
extern "C" GpStatus WINGDIPAPI GdipSaveImageToStream(GpImage*, IStream*, GDIPCONST CLSID*, GDIPCONST EncoderParameters*);
extern "C" GpStatus WINGDIPAPI GdipDisposeImage(GpImage*);
extern "C" void     WINGDIPAPI GdiplusShutdown(ULONG_PTR);
extern "C" void     GetIcons(HANDLE, DWORD, LPSTREAM, LPCSTR);
extern "C" DWORD    GetInfo(HANDLE, LPSTREAM);