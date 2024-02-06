//---------------------------------------------------------------------------
#ifndef my_ssAVIH
#define my_ssAVIH
//---------------------------------------------------------------------------
#include <vfw.h>
//---------------------------------------------------------------------------
#define MY_USING_MEMFR	1
//---------------------------------------------------------------------------
#define	MY_MAX_FRAME_WIDTH	20000
#define	MY_MAX_FRAME_HEIGHT	18000

class ssAVI
{
public://  mmioFOURCC('M', 'P', 'G', '4')
	ssAVI(const char *fn, const int nWidth, const int nHigh, unsigned long ulCodec = mmioFOURCC('M', 'P', 'G', '4'), unsigned long ulFrameRate = 50, unsigned long ulQulity = 10000);
	~ssAVI();

	virtual bool __fastcall AppendNewFrame(void * pBuf);
	virtual bool __fastcall myAddInfo(const char *sRIFF, char *pBuf, const int nBuf);
	virtual unsigned char * __fastcall myGray2RGB(const unsigned char *pBuf);


private:

	PAVIFILE hAVI_F;
	PAVISTREAM hAVI_S;
	PAVISTREAM hAVI_CompS;
	int nError;
	unsigned long ulFrameCount;
	unsigned long ulFrameSizeInByte;
	unsigned long ulBytesInLine;
	unsigned long ulFrameSizeInByte24;
	unsigned long ulBytesInLine24;
	int nFrameWidth;
	int nFrameHeight;
	char	sFileName[MAX_PATH];
	unsigned char * pFrameBuf;
	unsigned char **pLineBuf;

};
// please do not change this value
#define MY_MAX_USER_DATA	6

#pragma pack(1)
// a MEM-File has a head
typedef struct _MY_MEMF_HEAD
{
	unsigned int unHeadID;		// alwasy = 0x4d454d46    MEMF
	unsigned short usVer;		// = 1
	unsigned short usVer2;		// 3
	int			nBlockCount;
	int			nBlockSize;		// if not the same, this is the max size, current we only support the same BLK size
	int			nType;
	int			nRev;
	long long	llFirst;		// always = 64;
	int			nRev2[8];
}MY_MEMF_HEAD;

typedef struct _MY_BITMAP
{
	long        bmType;
	long        bmWidth;
	long        bmHeight;
	long        bmWidthBytes;
	unsigned short        bmPlanes;
	unsigned short        bmBitsPixel;
	void*      bmBits;
}MY_BITMAP;

#pragma pack()

class myMEMF
{
public:
	myMEMF(const int nBlockCount = 0, const int nBlockSize = 0);   // if both are 0, we use this to read the mem file
	~myMEMF();						// llSizeCurrent >0 the head will be updated

	// write the data to the index -> block, will be cycled, if write OK, the ssHead.nRev = nIndex; nIndex is 0-based
	virtual bool __fastcall mySetBlock(const int nIndex, const char *pBuf, const int nLength = -1);

	MY_MEMF_HEAD	ssHead;
	void *pfHandle;
	long long		llSizeCurrent;

private:
	MY_MEMF_HEAD	ssHeadRead;
	long long		llSizeRead;
};

#ifdef MY_USING_MEMFR
class myMEMFR
{
public:
	myMEMFR(const char *FN);
	~myMEMFR();
	virtual int __fastcall mySetFileName(const char *FN);
	void *pfHandle;

	virtual bool __fastcall myGetBlock(const int nIndex);
	virtual unsigned char * __fastcall myGray2RGB(const unsigned char *pBuf);
	virtual HBITMAP __fastcall myGetBMP(void);
	virtual int __fastcall GetHeight(void);
	virtual int __fastcall GetWidth(void);


	long long		llCurrent;

	unsigned char *pMEM;
	unsigned char *pBMP;
	bool bHeadOK;
	int  nIndex0;
	HBITMAP myHBMP;
	MY_BITMAP sBMP;

protected:
	virtual int __fastcall mySetFileName_ex(const char *FN);
	virtual void __fastcall myStart0(void);
	virtual void __fastcall myStop0(void);
	//int nBlockCountSave;
	//int nBlockSizeSave;


private:
	
	MY_MEMF_HEAD	ssHeadRead;
	long long		llSizeRead;

	int nFrameWidth;
	int nFrameHeight;
	int nCopyHeight;
	int nCopyWidth;

	char	sFileName[MAX_PATH];
	unsigned char * pFrameBuf;
	unsigned char **pLineBuf;
	unsigned char **pLineBufBMP;
	
};
#endif //MY_USING_MEMFR

//---------------------------------------------------------------------------
int __fastcall myReadInfo(const char * sFN, const char *sRIFF, char *pBuf, const int nBuf);
//---------------------------------------------------------------------------
#endif
//---------------------------------------------------------------------------