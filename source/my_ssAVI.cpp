//---------------------------------------------------------------------------
#include <windows.h>
#include "my_ssAVI.h"
//---------------------------------------------------------------------------
#pragma comment(lib, "vfw32.lib")
//---------------------------------------------------------------------------
//typedef struct _AVISTREAMINFOA {
//	DWORD		fccType;
//	DWORD               fccHandler;
//	DWORD               dwFlags;        /* Contains AVITF_* flags */
//	DWORD		dwCaps;
//	WORD		wPriority;
//	WORD		wLanguage;
//	DWORD               dwScale;
//	DWORD               dwRate; /* dwRate / dwScale == samples/second */
//	DWORD               dwStart;
//	DWORD               dwLength; /* In units above... */
//	DWORD		dwInitialFrames;
//	DWORD               dwSuggestedBufferSize;
//	DWORD               dwQuality;
//	DWORD               dwSampleSize;
//	RECT                rcFrame;
//	DWORD		dwEditCount;
//	DWORD		dwFormatChangeCount;
//	char		szName[64];
//} AVISTREAMINFOA, FAR * LPAVISTREAMINFOA;

//typedef struct tagBITMAPINFOHEADER{
//	DWORD      biSize;
//	LONG       biWidth;
//	LONG       biHeight;
//	WORD       biPlanes;
//	WORD       biBitCount;
//	DWORD      biCompression;
//	DWORD      biSizeImage;
//	LONG       biXPelsPerMeter;
//	LONG       biYPelsPerMeter;
//	DWORD      biClrUsed;
//	DWORD      biClrImportant;
//} BITMAPINFOHEADER, FAR *LPBITMAPINFOHEADER, *PBITMAPINFOHEADER;

//typedef struct {
//	DWORD	fccType;		    /* stream type, for consistency */
//	DWORD       fccHandler;                 /* compressor */
//	DWORD       dwKeyFrameEvery;            /* keyframe rate */
//	DWORD       dwQuality;                  /* compress quality 0-10,000 */
//	DWORD       dwBytesPerSecond;           /* bytes per second */
//	DWORD       dwFlags;                    /* flags... see below */
//	LPVOID      lpFormat;                   /* save format */
//	DWORD       cbFormat;
//	LPVOID      lpParms;                    /* compressor options */
//	DWORD       cbParms;
//	DWORD       dwInterleaveEvery;          /* for non-video streams only */
//} AVICOMPRESSOPTIONS, FAR *LPAVICOMPRESSOPTIONS;

void __fastcall myMemcpy(char *A, const char *B, const int n);

//inline void __fastcall myMemcpyEx(unsigned long long *A, const unsigned long long *B, int n)
//{
//	for (int i = 0; i < n; i++) {
//		A[i] = B[i];
//	}
//}
//void __fastcall myMemcpy(char *A, const char *B, const int n)
//{
//	int nn = (n >> 3);
//	myMemcpyEx((unsigned long long *)A, (const unsigned long long *)B, nn);
//	for (int i = (nn << 3); i < n; i++) {
//		A[i] = B[i];
//	}
//}

#pragma pack(1)

typedef struct _MY_GRAY_BMP {
	BITMAPINFOHEADER sBMPH;
	RGBQUAD          RGBA[256];
}MY_GRAY_BMP;

//// Allocate for the BITMAPINFO structure and the color table.
// when used for 16 or 32 bit, we need to think of 12 bit mask case 12-bytes 0
//if ((bih.biBitCount == 16) || (bih.biBitCount == 32))
//dwDWMasks = sizeof(DWORD)* 3;
//hbi = GlobalAlloc(GPTR, (LONG)bih.biSize + nNumColors * sizeof(RGBQUAD) +dwDWMasks);

#pragma pack()

void __fastcall myINIT_BMP_HEAD(MY_GRAY_BMP &hBMP, const int nW, const int nH, const int nS)
{
	BITMAPINFOHEADER &sBMPH = hBMP.sBMPH;
	sBMPH.biSize = sizeof(BITMAPINFOHEADER);
	sBMPH.biWidth = nW;
	sBMPH.biHeight = nH;			//For uncompressed RGB bitmaps, if biHeight is positive, the bitmap is a bottom-up DIB with the origin at the lower left corner. If biHeight is negative, the bitmap is a top-down DIB with the origin at the upper left corner.
	sBMPH.biPlanes = 1;
	sBMPH.biBitCount = 8;
	sBMPH.biCompression = BI_RGB;
	sBMPH.biSizeImage = nS;
	sBMPH.biXPelsPerMeter = 0;
	sBMPH.biYPelsPerMeter = 0;
	sBMPH.biClrUsed = 256;
	sBMPH.biClrImportant = 0;		//Specifies the number of color indices that are considered important for displaying the bitmap. If this value is zero, all colors are important.
	for (int i = 0; i < 256; i++)
	{
		hBMP.RGBA[i].rgbRed = i;
		hBMP.RGBA[i].rgbGreen = i;
		hBMP.RGBA[i].rgbBlue = i;
		hBMP.RGBA[i].rgbReserved = 0;
	}
}

void __fastcall myReleaseStream(PAVISTREAM &hAVI)
{
	if (hAVI != NULL)
		AVIStreamRelease(hAVI);
	hAVI = NULL;
}

// return n is the real copyed char, Out real memory shall be n + 1
// In = NULL should also work;
int __fastcall myStrcpy(char *Out, const char * In, const int nMaxOutLen = -1)
{	// nMaxOutLen = -1,  if this is <0, no use
	int n = (int) strlen(In);
	if (nMaxOutLen >= 0){
		if (n >= nMaxOutLen)
			n = nMaxOutLen - 1;
	}
	if (n >= 0){
		if (n > 0)
			myMemcpy(Out, In, n);
		Out[n] = 0;
	}
	return n;
}
//---------------------------------------------------------------------------
// Following are for class ssAVI
//---------------------------------------------------------------------------
ssAVI::ssAVI(const char *fn, const int nWidth, const int nHigh, unsigned long ulCodec, unsigned long ulFrameRate, unsigned long ulQulity)
{	// const char *fn, unsigned long ulCodec = mmioFOURCC('M', 'P', 'G', '4'), unsigned long ulFrameRate = 1, unsigned long ulQulity = 10000
	int nBitPerPixel = 8;
	nFrameWidth = (nWidth < MY_MAX_FRAME_WIDTH) ? nWidth : MY_MAX_FRAME_WIDTH; if (nFrameWidth < 1)nFrameWidth = 1;
	nFrameHeight = (nHigh < MY_MAX_FRAME_HEIGHT) ? nHigh : MY_MAX_FRAME_HEIGHT; if (nFrameHeight < 1)nFrameHeight = 1;
	ulBytesInLine = ((((nFrameWidth)* nBitPerPixel + 31) & ~31) / 8);
	ulFrameSizeInByte = ulBytesInLine * nHigh;

	ulBytesInLine24 = ((((nFrameWidth)* 24 + 31) & ~31) / 8);
	ulFrameSizeInByte24 = ulBytesInLine * nHigh;

	//                                                        fRate
	AVISTREAMINFOA sInfo = { streamtypeVIDEO, 0, 0, 0, 0, 0, 1, 30, 0, 0, 0, 0, (unsigned long)-1, 0, { 0, 0, 1, 1 }, 0, 0, "EHM_Img_Cut" };
	//sInfo.fccHandler = ulCodec; // mmioFOURCC('M', 'P', 'G', '4')
	sInfo.dwRate = ulFrameRate;
	sInfo.rcFrame.right = nFrameWidth;
	sInfo.rcFrame.bottom = nFrameHeight;

	MY_GRAY_BMP gBMP;
	myINIT_BMP_HEAD(gBMP, nFrameWidth, nFrameHeight, ulFrameSizeInByte);

	//BITMAPINFOHEADER sBMPH = { sizeof(BITMAPINFOHEADER), 1, 1, 1, 24, BI_RGB, 4, 0, 0, 0, 0 };
	//sBMPH.biWidth = nFrameWidth;
	//sBMPH.biHeight = nFrameHeight;
	//sBMPH.biSizeImage = ulFrameSizeInByte;

	//                                                                                dwQuality
	AVICOMPRESSOPTIONS sCompOpt0 = { streamtypeVIDEO, mmioFOURCC('D', 'I', 'B', ' '), 1, 10000, 0, AVICOMPRESSF_KEYFRAMES | AVICOMPRESSF_VALID, NULL, 0, NULL, 0, 0 };
	AVICOMPRESSOPTIONS sCompOpt;
	memcpy(&sCompOpt, &sCompOpt0, sizeof(sCompOpt0));	//memset(&sCompOpt, 0, sizeof(AVICOMPRESSOPTIONS));
	//sCompOpt.fccType = streamtypeVIDEO;
	sCompOpt.fccHandler = ulCodec; // mmioFOURCC('D', 'I', 'V', 'X'); // Compressor RIFF //  fccHandler = mmioFOURCC('M', 'R', 'L', 'E'); 
	sCompOpt.dwKeyFrameEvery = 0;
	sCompOpt.dwQuality = 10000; //Quality 0 - 10000
	//sCompOpt.dwBytesPerSecond = 0;
	sCompOpt.dwFlags = 0;//|AVICOMPRESSF_DATARATE;

	////AVICOMPRESSF_INTERLEAVE		0x00000001    // interleave
	////AVICOMPRESSF_DATARATE			0x00000002    // use a data rate
	////AVICOMPRESSF_KEYFRAMES		0x00000004    // use keyframes
	////AVICOMPRESSF_VALID			0x00000008    // has valid data?

	//sCompOpt.lpFormat = NULL; //Save format
	//sCompOpt.cbFormat = 0;
	//sCompOpt.lpParms = NULL; //Compressor options
	//sCompOpt.cbParms = 0;
	//sCompOpt.dwBytesPerSecond = 0; //For non-video streams only

	nError = 0; ulFrameCount = 0;
	hAVI_S = NULL; hAVI_CompS = NULL;
	hAVI_F = NULL;
	pFrameBuf = NULL;
	pLineBuf = NULL;

	myStrcpy(sFileName, fn, MAX_PATH);

	AVIFileInit();
	while (1){

		if (AVIFileOpenA(&hAVI_F, sFileName, OF_CREATE | OF_WRITE | OF_SHARE_DENY_NONE, NULL) != 0){
			nError++; break;
		}

		if (AVIFileCreateStreamA(hAVI_F, &hAVI_S, &sInfo) != 0){
			nError++; break;
		}

		//// test a Dialog
		//AVICOMPRESSOPTIONS *psCompOpt[1]; psCompOpt[0] = &sCompOpt;
		//int nRes = AVISaveOptions(NULL, 0, 1, &hAVI_S, psCompOpt);		
		//if (nRes == 1){ 
		//	if (AVIMakeCompressedStream(&hAVI_CompS, hAVI_S, psCompOpt[0], NULL) != 0){
		//		myReleaseStream(hAVI_CompS);
		//	}
		//}
		//else{ // user press Cancel or there is an error
		//	myReleaseStream(hAVI_CompS);
		//}
		//AVISaveOptionsFree(1, psCompOpt);

		if (hAVI_CompS == NULL){
			if (AVIMakeCompressedStream(&hAVI_CompS, hAVI_S, &sCompOpt, NULL) != 0){
				myReleaseStream(hAVI_CompS);
			}
		}
		if (hAVI_CompS == NULL){
			if (AVIMakeCompressedStream(&hAVI_CompS, hAVI_S, &sCompOpt0, NULL) != 0){
				myReleaseStream(hAVI_CompS);
				nError++; break;
			}
		}

		if (AVIStreamSetFormat(hAVI_CompS, 0, &gBMP.sBMPH, gBMP.sBMPH.biSize + gBMP.sBMPH.biClrUsed * sizeof(RGBQUAD)) != 0) {
			nError++; break;
		}

		pFrameBuf = new unsigned char[ulFrameSizeInByte];
		if (pFrameBuf == NULL){
			nError++; break;
		}

		pLineBuf = new unsigned char *[nFrameHeight];
		if (pLineBuf == NULL){
			nError++; break;
		}
		else{
			unsigned char *pFrameBuf1 = pFrameBuf + ulBytesInLine * (nFrameHeight - 1);
			for (int i = 0; i < nFrameHeight; i++)
			{
				pLineBuf[i] = pFrameBuf1;
				pFrameBuf1 -= ulBytesInLine;
			}
		}
		break;
	}

}
ssAVI::~ssAVI()
{
	if (hAVI_CompS != NULL)
		AVIStreamRelease(hAVI_CompS);
	if (hAVI_S != NULL)
		AVIStreamRelease(hAVI_S);
	if (hAVI_F != NULL)
		AVIFileRelease(hAVI_F);
	AVIFileExit();

	if (pLineBuf != NULL)
		delete[] pLineBuf;

	if (pFrameBuf != NULL)
		delete[] pFrameBuf;
}
//---------------------------------------------------------------------------
unsigned char * __fastcall ssAVI::myGray2RGB(const unsigned char *pBuf){
	for (int i = 0; i < nFrameHeight; i++){
		myMemcpy((char *)(pLineBuf[i]), (const char *)pBuf, nFrameWidth);
		pBuf += nFrameWidth;
		//unsigned char *pLine1 = pLineBuf[i];
		//for (int j = 0; j < nFrameWidth; j++)
		//{
		//	unsigned char cOne = *pBuf++;
		//	*pLine1++ = cOne; //*pLine1++ = cOne; *pLine1++ = cOne;
		//}
	}
	return pFrameBuf;
}
bool __fastcall ssAVI::AppendNewFrame(void * pBuf)
{
	if (nError == 0){

		if (AVIStreamWrite(hAVI_CompS, ulFrameCount++, 1, myGray2RGB((unsigned char *)pBuf), ulFrameSizeInByte, 0, NULL, NULL) != 0){ // AVIIF_KEYFRAME (ulFrameCount == 0) ? AVIIF_KEYFRAME : 0
			nError++;
		}
		//ulFrameCount++;
	}
	return (nError == 0);
}

#pragma pack(1)
typedef union _CC4
{
	struct{
		unsigned char c0;
		unsigned char c1;
		unsigned char c2;
		unsigned char c3;
	};
	unsigned long cc4;
}uCC4;
#pragma pack()

bool __fastcall ssAVI::myAddInfo(const char *sRIFF, char *pBuf, const int nBuf){

	uCC4 id4;
	id4.c0 = sRIFF[0]; id4.c1 = sRIFF[1]; id4.c2 = sRIFF[2]; id4.c3 = sRIFF[3];

	if (hAVI_F == NULL){
		if (AVIFileOpenA(&hAVI_F, sFileName, OF_CREATE | OF_WRITE | OF_SHARE_DENY_NONE, NULL) != 0){
			return false;
		}
	}

	long lRes = AVIFileWriteData(hAVI_F, id4.cc4, pBuf, nBuf);

	//if (lRes != 0){
	//	// AVIERR_NODATA indicates that data with the requested chunk identifier does not exist.
	//	return false;
	//}
	return (lRes == 0);
}
// add data
//myAVI.myAddInfo("EHMW", (char *)nXXYY_Save, 48 * 4 * sizeof(int));
//---------------------------------------------------------------------------
int __fastcall myReadInfo(const char * sFN, const char *sRIFF, char *pBuf, const int nBuf)
{
	int nRead = 0;

	AVIFileInit();
	while (true){

		IAVIFile *pAVI;
		long lRes = AVIFileOpenA(&pAVI, sFN, OF_SHARE_DENY_WRITE, 0L);
		if (lRes != 0){
			//Return code				Description
			//AVIERR_BADFORMAT			//The file couldn't be read, indicating a corrupt file or an unrecognized format.
			//AVIERR_MEMORY				//The file could not be opened because of insufficient memory.
			//AVIERR_FILEREAD			//A disk error occurred while reading the file.
			//AVIERR_FILEOPEN			//A disk error occurred while opening the file.
			//REGDB_E_CLASSNOTREG		//According to the registry, the type of file specified in AVIFileOpen does not have a handler to process it.
			break;
		}

		// 
		// Place functions here that interact with the open file. 
		// 

		uCC4 id4;
		id4.c0 = sRIFF[0]; id4.c1 = sRIFF[1]; id4.c2 = sRIFF[2]; id4.c3 = sRIFF[3];

		long pBufSize = nBuf;
		lRes = AVIFileReadData(pAVI, id4.cc4, pBuf, &pBufSize);

		if (lRes != 0){
			// AVIERR_NODATA indicates that data with the requested chunk identifier does not exist.
			break;
		}

		nRead = pBufSize;

		AVIFileRelease(pAVI);  // closes the file 
		break;
	}
	AVIFileExit();
	return nRead;
}
//---------------------------------------------------------------------------
//#define CREATE_NEW          1
//#define CREATE_ALWAYS       2
//#define OPEN_EXISTING       3
//#define OPEN_ALWAYS         4
//Opens a file, always. If the specified file exists, the function succeeds and the last - error code is set to ERROR_ALREADY_EXISTS(183).
//If the specified file does not exist and is a valid path to a writable location, the function creates a file and the lasterror code is set to zero.

//#define TRUNCATE_EXISTING   5

//#define FILE_BEGIN           0		//The starting point is zero or the beginning of the file.
//#define FILE_CURRENT         1		//The starting point is the current value of the file pointer.
//#define FILE_END             2


typedef union _MY_LL {
	long long ll;
	struct {
		unsigned long LowPart;
		long HighPart;
	} u;
}MY_LL;

long long __fastcall myFileSeek(void *hf, long long llPos, unsigned long ulMethod)
{
	MY_LL uPos;
	uPos.ll = llPos;
	uPos.u.LowPart = SetFilePointer(hf, uPos.u.LowPart, &uPos.u.HighPart, ulMethod);
	if (uPos.u.LowPart == INVALID_SET_FILE_POINTER && GetLastError() != 0)    // NO_ERROR = 0
	{
		uPos.ll = -1;
	}
	return uPos.ll;
}

bool __fastcall myMEM_File_Open(void *&hf, const long long llSize = -1, bool bForceTrancate = false)
{
	bool bRes = false;
	// void * HANDLE
	while (1){
		hf = CreateFileA("MY_MEM_DATA.tmp", GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_ALWAYS,
			FILE_ATTRIBUTE_NORMAL | FILE_FLAG_RANDOM_ACCESS,
			NULL);
		if (INVALID_HANDLE_VALUE == hf){
			break;
		}
		if (llSize > 0){			// user set the size
			long long llSizeF;
			if ((llSizeF = myFileSeek(hf, 0, FILE_END)) == -1) break; // get size
			bool bReSize = true;
			if (bForceTrancate == false && llSizeF > llSize && llSizeF < llSize * 2){
				bReSize = false;
			}
			if (bReSize){
				if ((llSizeF = myFileSeek(hf, llSize, FILE_BEGIN)) == -1) // Set and get size
					break;

				if (SetEndOfFile(hf) == 0)
					break;
			}
		}
		bRes = true; break;
	}
	return bRes;
}


bool __fastcall myMEM_File_OpenRD(void *&hf)
{
	bool bRes = false;
	// void * HANDLE
	while (1){
		hf = CreateFileA("MY_MEM_DATA.tmp", GENERIC_READ, FILE_SHARE_WRITE, NULL, OPEN_EXISTING,
			FILE_ATTRIBUTE_READONLY | FILE_ATTRIBUTE_NORMAL | FILE_FLAG_RANDOM_ACCESS,
			NULL);
		if (INVALID_HANDLE_VALUE == hf){
			break;
		}
		bRes = true; break;
	}
	return bRes;
}

unsigned long __fastcall myMEM_File_Read_ex(void *&hf, unsigned char *pOut, const int nLen, const long long llStart = -1)
{
	if (llStart >= 0){
		if (myFileSeek(hf, llStart, FILE_BEGIN) == -1) return 0;
	}
	unsigned long ulIO;
	if (ReadFile(hf, pOut, nLen, &ulIO, NULL) == 0) return 0;
	return ulIO;
}

unsigned long __fastcall myMEM_File_Read(void *&hf, char *pOut, const int nLen, const long long llStart = -1)
{
	if (INVALID_HANDLE_VALUE == hf){
		if (!myMEM_File_Open(hf))
			return 0;
	}
	if (llStart >= 0){
		if (myFileSeek(hf, llStart, FILE_BEGIN) == -1) return 0;
	}
	unsigned long ulIO;
	if (ReadFile(hf, pOut, nLen, &ulIO, NULL) == 0) return 0;
	return ulIO;
}

unsigned long __fastcall myMEM_File_Write(void *&hf, const char *pIn, const int nLen, const long long llStart = -1)
{
	if (INVALID_HANDLE_VALUE == hf){
		if (!myMEM_File_Open(hf))
			return 0;
	}
	if (llStart >= 0){
		if (myFileSeek(hf, llStart, FILE_BEGIN) == -1) return 0;
	}
	unsigned long ulIO;
	if (WriteFile(hf, pIn, nLen, &ulIO, NULL) == 0) return 0;
	return ulIO;
}
//#include "DBG_print.h"
bool __fastcall myMEM_File_Close(void *&hf)
{
	if (INVALID_HANDLE_VALUE == hf || hf == NULL) return true;
	bool bRes = false;
	while (1){
		int nn = CloseHandle(hf);
		if (!nn){
			// Handle the error.
			//printf("CloseHandle failed (%d)\n", GetLastError()); // close error
			hf = INVALID_HANDLE_VALUE;
			break;
		}
		hf = INVALID_HANDLE_VALUE;
		bRes = true;
		break;
	}
	return bRes;
}


bool __fastcall myMEM_File_OpenRD(void *&hf, const char *fn)
{
	bool bRes = false;
	// void * HANDLE
	while (1){
		hf = CreateFileA(fn, GENERIC_READ, FILE_SHARE_WRITE, NULL, OPEN_EXISTING,
			FILE_ATTRIBUTE_READONLY | FILE_ATTRIBUTE_NORMAL | FILE_FLAG_RANDOM_ACCESS,
			NULL);
		if (INVALID_HANDLE_VALUE == hf){
			break;
		}
		bRes = true; break;
	}
	return bRes;
}

#define	MY_FIRST_BLOCK	64

void __fastcall myInit_MY_MEMF_HEAD(MY_MEMF_HEAD &A)
{
	int i = sizeof(A);
	memset(&A, 0, i);
	A.unHeadID = 0x4d454d46;
	A.usVer = 1;
	A.usVer2 = 3;
	//A.nBlockSize = 0;
	//A.nBlockCount = 0;
	//A.nType = 0;
	//A.nRev = 0;
	A.llFirst = 64;
}

bool __fastcall my_IsOK_MEMF_HEAD(const MY_MEMF_HEAD &A)
{
	if (A.unHeadID == 0x4d454d46 && A.usVer == 1 && A.usVer2 == 3){
		if (A.nBlockSize >= A.nRev2[0] * A.nRev2[1])
			return true;
	}
	return false;
}

//---------------------------------------------------------------------------
// Following are for class myMEMF
//---------------------------------------------------------------------------
myMEMF::myMEMF(const int nBlockCount, const int nBlockSize)
{

	pfHandle = INVALID_HANDLE_VALUE;
	llSizeCurrent = nBlockCount; llSizeCurrent *= nBlockSize;

	if (myMEM_File_OpenRD(pfHandle)){
		myMEM_File_Read(pfHandle, (char *)&ssHeadRead, sizeof(ssHeadRead), 0);
		llSizeRead = myFileSeek(pfHandle, 0, FILE_END);
		if (llSizeCurrent > 0)
			myMEM_File_Close(pfHandle);
	}
	else{
		myInit_MY_MEMF_HEAD(ssHeadRead);
	}

	memcpy(&ssHead, &ssHeadRead, sizeof(ssHeadRead));

	if (llSizeCurrent > 0){
		llSizeCurrent += MY_FIRST_BLOCK;
		if (myMEM_File_Open(pfHandle, llSizeCurrent)){
			//myMEM_File_Open(pfHandle);
			ssHead.nBlockCount = nBlockCount;
			ssHead.nBlockSize = nBlockSize;
			myMEM_File_Write(pfHandle, (char *)&ssHead, sizeof(ssHead), 0);
			myMEM_File_Close(pfHandle);
		}
	}
}

myMEMF::~myMEMF()
{
	if (llSizeCurrent > 0)
		myMEM_File_Write(pfHandle, (char *)&ssHead, sizeof(ssHead), 0);
	myMEM_File_Close(pfHandle);
}

bool __fastcall myMEMF::mySetBlock(const int nIndex, const char *pBuf, const int nLength)
{	// nLength = -1
	int nLen = (nLength<0) ? ssHead.nBlockSize : nLength;
	bool bRes = false;
	while (nLen>0)
	{
		long long llStart = (long long)(nIndex % ssHead.nBlockCount) * ssHead.nBlockSize + ssHead.llFirst;
		int nWR = myMEM_File_Write(pfHandle, pBuf, nLen, llStart);
		if (!myMEM_File_Close(pfHandle)) break;
		bRes = (nWR == nLen); break;
	}
	if (bRes)
		ssHead.nRev = nIndex;
	return bRes;
}

#ifdef MY_USING_MEMFR
//---------------------------------------------------------------------------
// Following are for class myMEMFR
//---------------------------------------------------------------------------
void __fastcall myMEMFR::myStop0(void)
{

}
void __fastcall myMEMFR::myStart0()
{

}
void __fastcall myAssign(MY_BITMAP &A, const BITMAP &B){
	A.bmType = B.bmType;
	A.bmWidth = B.bmWidth;
	A.bmHeight = B.bmHeight;
	A.bmWidthBytes = B.bmWidthBytes;
	A.bmPlanes = B.bmPlanes;
	A.bmBitsPixel = B.bmBitsPixel;
	A.bmBits = B.bmBits;
}
myMEMFR::myMEMFR(const char *FN)	// in BOOL CDemoApp::InitInstance() -> new CMainFrame -> new this
{	// *FN = NULL
	bHeadOK = false;
	mySetFileName_ex(FN);
	if (myMEM_File_OpenRD(pfHandle, sFileName)){
		myMEM_File_Read_ex(pfHandle, (unsigned char *)&ssHeadRead, sizeof(ssHeadRead), 0);
		llSizeRead = myFileSeek(pfHandle, 0, FILE_END);
		if ((bHeadOK = my_IsOK_MEMF_HEAD(ssHeadRead))){
			llCurrent = myFileSeek(pfHandle, ssHeadRead.llFirst, FILE_BEGIN);
		}
		else{
			llCurrent = myFileSeek(pfHandle, 0, FILE_BEGIN);
		}
	}
	else llCurrent = 0;

	pFrameBuf = NULL; myHBMP = NULL; pLineBuf = NULL; pLineBufBMP = NULL; nIndex0 = nCopyWidth = nCopyHeight = 0;

	if (bHeadOK){
		pMEM = new unsigned char[ssHeadRead.nBlockSize];
		// hbmp
		int nBitPerPixel = 8;
		nIndex0 = (ssHeadRead.nRev + 1) % ssHeadRead.nBlockCount;
		nFrameWidth = ssHeadRead.nRev2[0];
		nFrameHeight = ssHeadRead.nRev2[1];
		unsigned long ulBytesInLine8 = ((((nFrameWidth)* nBitPerPixel + 31) & ~31) / 8);
		unsigned long ulFrameSizeInByte = ulBytesInLine8 * nFrameHeight;
		pBMP = new unsigned char[sizeof(MY_GRAY_BMP)+ulFrameSizeInByte];
		if (pBMP != NULL){
			myINIT_BMP_HEAD(*((MY_GRAY_BMP*)pBMP), nFrameWidth, nFrameHeight, ulFrameSizeInByte);

			// mem line pos
			pFrameBuf = pBMP + sizeof(MY_GRAY_BMP);
			pLineBuf = new unsigned char *[nFrameHeight];
			if (pLineBuf != NULL){
				//unsigned char *pFrameBuf1 = pFrameBuf;
				//unsigned char *pFrameBuf1 = pFrameBuf + ulBytesInLine8 * (nFrameHeight - 1);   // reverse

				unsigned char *pFrameBuf1 = pMEM;
				for (int i = 0; i < nFrameHeight; i++)
				{
					pLineBuf[i] = pFrameBuf1;
					//pFrameBuf1 += ulBytesInLine8;
					//pFrameBuf1 -= ulBytesInLine8; // reverse
					pFrameBuf1 += nFrameWidth;
				}
			}
			// create a hBitmap
			if (myHBMP == NULL){
				//HDC hDC = GetDC(NULL);
				//HDC memDC = CreateCompatibleDC(hDC); 
				HDC hDC = CreateCompatibleDC(NULL);
				void *pBitBuf = NULL;
				myHBMP = CreateDIBSection(hDC, (BITMAPINFO*)pBMP, DIB_RGB_COLORS, &pBitBuf, NULL, 0); // DIB_PAL_COLORS 1       DIB_RGB_COLORS	0
				//myHBMP = CreateCompatibleBitmap(memDC, nFrameWidth, nFrameHeight);
				//HBITMAP hBMPOld = (HBITMAP)SelectObject(memDC, myHBMP);		
				//myHBMP = CreateDIBitmap(hDC, &((MY_GRAY_BMP *)pBMP)->sBMPH, CBM_INIT, pFrameBuf, (BITMAPINFO*)pBMP, DIB_RGB_COLORS);
				//if (hPal) SelectPalette(hDC, hPalOld, FALSE);

				// get mem
				while (myHBMP != NULL){
					BITMAP  sBMP0;
					if (!GetObjectA(myHBMP, sizeof(sBMP0), &sBMP0))  //返回值是存储在缓冲区中的字节数。
					{
						OutputDebugStringA("GetObjectA Error.\n"); break;
					}
					myAssign(sBMP, sBMP0);
					unsigned char* pBufBMP = (unsigned char*)(sBMP.bmBits);  // pBitBuf == pBufBMP
					if (pBufBMP != NULL){
						//long long nSize = ((long long)sBMP.bmWidth * sBMP.bmHeight * sBMP.bmBitsPixel) >> 3;
						nCopyHeight = (sBMP.bmHeight < nFrameHeight) ? (sBMP.bmHeight) : nFrameHeight;
						nCopyWidth = (sBMP.bmWidth < nFrameWidth) ? (sBMP.bmWidth) : nFrameWidth;
						pLineBufBMP = new unsigned char *[nCopyHeight];
						if (pLineBufBMP != NULL){
							//unsigned char *pFrameBuf1 = pBufBMP;
							unsigned char *pFrameBuf1 = pBufBMP + sBMP.bmWidthBytes * (sBMP.bmHeight - 1);   // reverse
							for (int i = 0; i < nCopyHeight; i++)
							{
								pLineBufBMP[i] = pFrameBuf1;
								//pFrameBuf1 += sBMP.bmWidthBytes;
								pFrameBuf1 -= sBMP.bmWidthBytes;; // reverse
							}
						}
					}
					break;
				}
				//SelectObject(memDC, hBMPOld);
				DeleteDC(hDC);//DeleteDC(memDC);
				//ReleaseDC(NULL, hDC);
			} // end_if (myHBMP == NULL)
		} // end_if (pBMP != NULL)
	}
	else{
		nFrameWidth = 0; nFrameHeight = 0;
		pMEM = NULL; pBMP = NULL;
	}
	
}
myMEMFR::~myMEMFR()
{
	myMEM_File_Close(pfHandle);

	if (myHBMP != NULL){
		int nR = DeleteObject(myHBMP);	//如果该函数成功，则返回值为非零值。
		if (nR == 0){
			OutputDebugStringA("DeleteObject Error\n");
		}
	}
	if (pLineBuf != NULL)
		delete[] pLineBuf;
	
	if (pBMP != NULL)
		delete[] pBMP;
	
	if (pMEM != NULL)
		delete[] pMEM;
}
int __fastcall myMEMFR::mySetFileName_ex(const char *FN)
{
	return myStrcpy(sFileName, FN, MAX_PATH);
}
bool __fastcall myMEMFR::myGetBlock(const int nIndex)  // in timer, called myCheckFrame.
{
	bool bRes = false;
	while (pMEM != NULL){
		int nLen;
		long long llStart = (long long)((nIndex + nIndex0) % ssHeadRead.nBlockCount) * (nLen = ssHeadRead.nBlockSize) + ssHeadRead.llFirst;
		if (llStart != llCurrent){
			llCurrent = myFileSeek(pfHandle, llStart, FILE_BEGIN);
		}
		if (llStart != llCurrent || nLen < 1)break;
		int nWR = myMEM_File_Read_ex(pfHandle, pMEM, nLen);
		if (nWR != nLen)break;
		bRes = true; break;
	}
	return bRes;
}
int __fastcall myMEMFR::mySetFileName(const char *FN)
{

	return 0;
}
unsigned char * __fastcall myMEMFR::myGray2RGB(const unsigned char *pBuf){
	for (int i = 0; i < nFrameHeight; i++){
		myMemcpy((char *)(pLineBuf[i]), (const char *)pBuf, nFrameWidth);
		pBuf += nFrameWidth;
		//unsigned char *pLine1 = pLineBuf[i];
		//for (int j = 0; j < nFrameWidth; j++)
		//{
		//	unsigned char cOne = *pBuf++;
		//	*pLine1++ = cOne; //*pLine1++ = cOne; *pLine1++ = cOne;
		//	
		//}
	}
	return pFrameBuf;
}
void __fastcall myGray2Pixel32(const unsigned char *pIn, unsigned int *pOut, const int nLen)
{
	for (int i = 0; i < nLen; i++){
		unsigned int n = pIn[i];
		n |= (n << 8) | (n << 16);
	}
}
HBITMAP __fastcall myMEMFR::myGetBMP(void)
{
	if (nCopyHeight > 0)  // when this matched, all memory shall be correctly set.
	{
		if (sBMP.bmBitsPixel == 32)
		{
			for (int i = 0; i < nCopyHeight; i++)
			{
				myGray2Pixel32(pLineBuf[i], (unsigned int *)pLineBufBMP[i], nCopyWidth);
			}
		}
		else if (sBMP.bmBitsPixel == 8){
			for (int i = 0; i < nCopyHeight; i++)
			{
				//myGray2Pixel8(pLineBuf[i], (unsigned int *)pLineBufBMP[i], nCopyWidth);
				myMemcpy((char *)pLineBufBMP[i], (const char *)pLineBuf[i], nCopyWidth);
			}
		}
	}
	return myHBMP;
}
// Following 2 functions, first call in bool CMainFrame::InitCamera()
int __fastcall myMEMFR::GetHeight(void)
{
	return nFrameHeight;
}
int __fastcall myMEMFR::GetWidth(void)
{
	return nFrameWidth;
}
#endif //MY_USING_MEMFR

//#define WIN32_LEAN_AND_MEAN
//#include <windows.h>
//#include <cstddef>
//#include <cstdlib>
//#include <algorithm>
//#include <iterator>
//#include <string>
//#include <iostream>
//int myMain(int argc, char* argv[])
//{
//	if (argc == 1)
//	{
//		std::cerr << argv[0] << " error usage: " << argv[0] << " <bmp-file-name>\n";
//		return EXIT_FAILURE;
//	}
//
//	HBITMAP hbm = static_cast<HBITMAP>(LoadImage(GetModuleHandle(0), argv[1], IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION));
//	if (!hbm) {
//		std::cerr << argv[0] << " error could not open file.\n";
//		return EXIT_FAILURE;
//	}
//	
//	BITMAP  bm;
//	if (!GetObject(hbm, sizeof(bm), &bm)) {
//		std::cerr << argv[0] << " error could not load file\n";
//		return EXIT_FAILURE;
//	}
//
//	unsigned char* buf = reinterpret_cast<unsigned char*>(bm.bmBits);
//	
//	if (!buf) {
//		DeleteObject(hbm);
//		std::cerr << argv[0] << " error image data is invalid\n";
//		return EXIT_FAILURE;
//	}
//	size_t size = bm.bmWidth * bm.bmHeight * static_cast<size_t>(bm.bmBitsPixel * 0.125f);
//	//dump to console out, for you, you can do your manip here
//	std::copy(buf, buf + size, std::ostream_iterator<unsigned int>(std::cout, ", "));
//	DeleteObject(hbm);// don't delete until you've finished manipulation
//	return EXIT_SUCCESS;
//}

//#define COLORREF2RGB(Color) (Color & 0xff00) | ((Color >> 16) & 0xff) | ((Color << 16) & 0xff0000)
//-------------------------------------------------------------------------------
// ReplaceColor
//
// Author    : Dimitri Rochette drochette@coldcat.fr
// Specials Thanks to Joe Woodbury for his comments and code corrections
//
// Includes  : Only <windows.h>

//
// hBmp         : Source Bitmap
// cOldColor : Color to replace in hBmp
// cNewColor : Color used for replacement
// hBmpDC    : DC of hBmp ( default NULL ) could be NULL if hBmp is not selected
//
// Retcode   : HBITMAP of the modified bitmap or NULL for errors
//
//-------------------------------------------------------------------------------
//HBITMAP ReplaceColor(HBITMAP hBmp, COLORREF cOldColor, COLORREF cNewColor, HDC hBmpDC)
//{
//	HBITMAP RetBmp = NULL;
//	if (hBmp)
//	{
//		HDC BufferDC = CreateCompatibleDC(NULL);    // DC for Source Bitmap
//		if (BufferDC)
//		{
//			HBITMAP hTmpBitmap = (HBITMAP)NULL;
//			if (hBmpDC)
//			if (hBmp == (HBITMAP)GetCurrentObject(hBmpDC, OBJ_BITMAP))
//			{
//				hTmpBitmap = CreateBitmap(1, 1, 1, 1, NULL);
//				SelectObject(hBmpDC, hTmpBitmap);
//			}
//
//			HGDIOBJ PreviousBufferObject = SelectObject(BufferDC, hBmp);
//			// here BufferDC contains the bitmap
//
//			HDC DirectDC = CreateCompatibleDC(NULL); // DC for working
//			if (DirectDC)
//			{
//				// Get bitmap size
//				BITMAP bm;
//				GetObject(hBmp, sizeof(bm), &bm);
//
//				// create a BITMAPINFO with minimal initilisation 
//				// for the CreateDIBSection
//				BITMAPINFO RGB32BitsBITMAPINFO;
//				ZeroMemory(&RGB32BitsBITMAPINFO, sizeof(BITMAPINFO));
//				RGB32BitsBITMAPINFO.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
//				RGB32BitsBITMAPINFO.bmiHeader.biWidth = bm.bmWidth;
//				RGB32BitsBITMAPINFO.bmiHeader.biHeight = bm.bmHeight;
//				RGB32BitsBITMAPINFO.bmiHeader.biPlanes = 1;
//				RGB32BitsBITMAPINFO.bmiHeader.biBitCount = 32;
//
//				// pointer used for direct Bitmap pixels access
//				UINT * ptPixels;
//
//				HBITMAP DirectBitmap = CreateDIBSection(DirectDC,
//					(BITMAPINFO *)&RGB32BitsBITMAPINFO,
//					DIB_RGB_COLORS,
//					(void **)&ptPixels,
//					NULL, 0);
//				if (DirectBitmap)
//				{
//					// here DirectBitmap!=NULL so ptPixels!=NULL no need to test
//					HGDIOBJ PreviousObject = SelectObject(DirectDC, DirectBitmap);
//					BitBlt(DirectDC, 0, 0,
//						bm.bmWidth, bm.bmHeight,
//						BufferDC, 0, 0, SRCCOPY);
//
//					// here the DirectDC contains the bitmap
//
//					// Convert COLORREF to RGB (Invert RED and BLUE)
//					cOldColor = COLORREF2RGB(cOldColor);
//					cNewColor = COLORREF2RGB(cNewColor);
//
//					// After all the inits we can do the job : Replace Color
//					for (int i = ((bm.bmWidth*bm.bmHeight) - 1); i >= 0; i--)
//					{
//						if (ptPixels[i] == cOldColor) ptPixels[i] = cNewColor;
//					}
//					// little clean up
//					// Don't delete the result of SelectObject because it's 
//					// our modified bitmap (DirectBitmap)
//					SelectObject(DirectDC, PreviousObject);
//
//					// finish
//					RetBmp = DirectBitmap;
//				}
//				// clean up
//				DeleteDC(DirectDC);
//			}
//			if (hTmpBitmap)
//			{
//				SelectObject(hBmpDC, hBmp);
//				DeleteObject(hTmpBitmap);
//			}
//			SelectObject(BufferDC, PreviousBufferObject);
//			// BufferDC is now useless
//			DeleteDC(BufferDC);
//		}
//	}
//	return RetBmp;
//}

//PBITMAPINFO CreateBitmapInfoStruct(HWND hwnd, HBITMAP hBmp)
//{
//	BITMAP bmp;
//	PBITMAPINFO pbmi;
//	WORD    cClrBits;
//
//	// Retrieve the bitmap color format, width, and height.  
//	if (!GetObject(hBmp, sizeof(BITMAP), (LPSTR)&bmp))
//		OutputDebugStringA("GetObject");
//
//	// Convert the color format to a count of bits.  
//	cClrBits = (WORD)(bmp.bmPlanes * bmp.bmBitsPixel);
//	if (cClrBits == 1)
//		cClrBits = 1;
//	else if (cClrBits <= 4)
//		cClrBits = 4;
//	else if (cClrBits <= 8)
//		cClrBits = 8;
//	else if (cClrBits <= 16)
//		cClrBits = 16;
//	else if (cClrBits <= 24)
//		cClrBits = 24;
//	else cClrBits = 32;
//
//	// Allocate memory for the BITMAPINFO structure. (This structure  
//	// contains a BITMAPINFOHEADER structure and an array of RGBQUAD  
//	// data structures.)  
//
//	if (cClrBits < 24)
//		pbmi = (PBITMAPINFO)LocalAlloc(LPTR,
//		sizeof(BITMAPINFOHEADER)+
//		sizeof(RGBQUAD)* (1 << cClrBits));
//
//	// There is no RGBQUAD array for these formats: 24-bit-per-pixel or 32-bit-per-pixel 
//
//	else
//		pbmi = (PBITMAPINFO)LocalAlloc(LPTR,
//		sizeof(BITMAPINFOHEADER));
//
//	// Initialize the fields in the BITMAPINFO structure.  
//
//	pbmi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
//	pbmi->bmiHeader.biWidth = bmp.bmWidth;
//	pbmi->bmiHeader.biHeight = bmp.bmHeight;
//	pbmi->bmiHeader.biPlanes = bmp.bmPlanes;
//	pbmi->bmiHeader.biBitCount = bmp.bmBitsPixel;
//	if (cClrBits < 24)
//		pbmi->bmiHeader.biClrUsed = (1 << cClrBits);
//
//	// If the bitmap is not compressed, set the BI_RGB flag.  
//	pbmi->bmiHeader.biCompression = BI_RGB;
//
//	// Compute the number of bytes in the array of color  
//	// indices and store the result in biSizeImage.  
//	// The width must be DWORD aligned unless the bitmap is RLE 
//	// compressed. 
//	pbmi->bmiHeader.biSizeImage = ((pbmi->bmiHeader.biWidth * cClrBits + 31) & ~31) / 8
//		* pbmi->bmiHeader.biHeight;
//	// Set biClrImportant to 0, indicating that all of the  
//	// device colors are important.  
//	pbmi->bmiHeader.biClrImportant = 0;
//	return pbmi;
//}