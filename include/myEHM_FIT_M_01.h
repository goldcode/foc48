//---------------------------------------------------------------------------
#ifndef myEHM_FIT_M_01H
#define myEHM_FIT_M_01H
//---------------------------------------------------------------------------
#include "myThread_Help.h"
#define	MY_THREAD_NUMBER	8
//class myThreads;
//union uMY_INFO_DATA;
class MyLMFIT;
//---------------------------------------------------------------------------
#include "myListManger.h"
//---------------------------------------------------------------------------
#define MY_MAX_ITEM_COUNT			500000000
#define	MY_DEL_IMG					0x0001
#define MY_NO_DEL_ANA				0x0010			// thread will not delete pANA
#define MY_NO_DEL_LINE				0x0100			// thread will not delete pLine

//---------------------------------------------------------------------------
// Using internal memory can avoid new memory not enough problem.
// In most computer, internal allocated memory is slower than the new allocated memory.
// Suggest to use this, when computer memory is smaller than 4GB.

//#define	MY_USING_INTERNAL_ALLOC		1
//---------------------------------------------------------------------------
#pragma pack(1)

typedef struct _myOBJ48_Tag
{
	long long ImgID;
	const unsigned char *pImgIn;	// The input image is stored
	unsigned char *pImgCut;			// Image buffer, user can delete, but needs to set NULL after. This will be deleted by the program when quit
	double *pLine;			// For each image, the 1d format data for fit, after fit, we can also delete and set to NULL
	double *pANA;					// 12 x 96 array for ana, after, user can delete it
	double dResult[48];				// 48 ana distances
	volatile char nTID[48];			// threadID, >0 means done, <0 means is doing =0 means still waiting
	volatile int nState;			// 0 means nothing is done, if all done, this is 48, if read, this is -48
	int     nMemUse;				//
	int		nThisID;
	int nRev;
}myOBJ48_Tag, *pmyOBJ48_Tag;
// 48 byte

typedef struct _MY_FIT_GL_MEM{
	void	*pUser;
	double(*gl_pOpt)[2];
	double	gl_dlamdaFun;
	int		gl_nOptCount, gl_nIteration, gl_nIteration0;
	int		nThisID;		// = GLM0 = 474c4d30
	int		nRes[2];
}sMY_FIT_GL_MEM, *pMY_FIT_GL_MEM;

// 48 byte
#pragma pack()


class myOBJX : public myListItem
{
public:
	pmyOBJ48_Tag sOBJ;				// here, we still can increase user data, if last tag can be resized
	//myOBJ48_Tag sOBJ;				// here, we use fixed size, to encrease a bit the speed
private:	// User declarations
};

class myOBJXs : public myLists
{
public:		// User declarations
	explicit myOBJXs(const int nSize1 = 60 * 8 * 240 * 6, const int nSize2 = 120 * 96);
	virtual ~myOBJXs();
	virtual myOBJX * __fastcall NewItemX(int s = 63, int pc = 0);
	virtual myOBJX * __fastcall NewItemX(int s, const unsigned char *pBuf, const int nMAKS = 0);
	virtual void __fastcall ReMoveOne(const myListItem &A);
	//     myOBJ_Item(int n=63);
	//     virtual ~myOBJ_Item();

	volatile long long llFrame;
	volatile long long llFrameLast;
	volatile int nLastSave;
	int  nSize_pImgCut;
	int  nSize_pLine;
	//myMutex mx;

	virtual void __fastcall ReMoveFirst(void);

	virtual bool __fastcall HasNew(void);
	virtual myOBJ48_Tag * __fastcall myFindFirstNew(void);
	virtual myOBJ48_Tag * __fastcall myFindID(const int nID, const bool bNoRead = true);

	virtual void __fastcall myInit(const int nSize1, const int nSize2, bool bRemoveAll = false);
	virtual int  __fastcall myKeepFrame(const int n);

	myOBJX *LastAdd;
	myOBJX *LastUpdate;
	myOBJX *FirstFrame;
	int nFirstFrameID;
	int nFrameTotalCount;

	pmyOBJ48_Tag sOBJ_Last;

};

//---------------------------------------------------------------------------
// Please do not change the following defined numbers.
// If must change, please also change the same values in DLL project and re-compile the DLL files.
//---------------------------------------------------------------------------
#ifndef MY_CUT_IMAGE_X

#define	MY_CUT_IMAGE_X		100
#define	MY_CUT_IMAGE_Y		56
#define	MY_CUT_IMAGE_Y_REAL	28

#endif // !MY_CUT_IMAGE_X
//---------------------------------------------------------------------------
#define	MY_SAMLL_IMG_X	MY_CUT_IMAGE_X
#define	MY_SAMLL_IMG_Y	MY_CUT_IMAGE_Y
#define	MY_OUT_IMG_X	(MY_SAMLL_IMG_X * 2) * 6
#define	MY_OUT_IMG_Y	(MY_SAMLL_IMG_Y * 8)

//---------------------------------------------------------------------------
// Following defined numbers can be changed
//---------------------------------------------------------------------------

// This is the saving image numbers, 1000 frames costs ~500Mb, please change according to the real memory size and free disk space.
// When input images are less than this value, the real inputed images will be saved by default.
#define	MY_MAX_MEM_FRAME	1000

// This is the max saving distance. Following is 1-day data.
#define	MY_MAX_MEM_DATA		50 * 3600 * 24

class myEHM_FIT{
public:

	explicit myEHM_FIT(const int nThreadNumber = MY_THREAD_NUMBER);
	~myEHM_FIT();

	// n1, n2 is input image size, n1 is contiguous mem-length
	// sets for small block      n1    n2   d-n1   d-n2    start_n1    Start_n2
	// nSSB[6] =                [258, 103,  258,   155,    0,             0          ];  % 0 based
	virtual void __fastcall myINIT_48(const unsigned char *pImg, const int n1, const int n2, const int *nSSB);
	virtual bool __fastcall isINIT_OK(void);
	virtual bool __fastcall myDllTestLoad(void);

	// dOut is 48 distances
	// bAdd will add the RAW images into MEM_File, for future to read the raw data if exe file crashed e.g.
	// return is the ID, one can use this ID to get the data by myGetANA(const int nID, double *dOut);
	// if dOut = NULL, function will return immediately. Otherwise, will return till data is OK
	// sFrameOut will be used together with dOut
	// nMASK is used to set nMemUse, is user need pANA for pLine information.
	virtual int __fastcall myANA_48(const unsigned char *pImg, double *dOut, bool bAdd = false, myOBJ48_Tag **sFrameOut = NULL, const int nMASK = 0);

	// return 1 when data is good, return 0 when data is too old and out of mem.
	// function will block, till data is avalible, if nID is new added
	// if nID = -1, return >0, data of earest ID that is not read.
	// dOut is 48 distances
	virtual int __fastcall myGetANA(const int nID, double *dOut, myOBJ48_Tag **sFrameOut = NULL);
	
	// pdRawXY is double *[4]
	// bDel will delete pANA, after using
	virtual void __fastcall myGetRaw(myOBJ48_Tag *sFrameOut, double *pdRawXY, bool bDel = true);

	// check if ANA for nID is ready, return is count that is calculated, best will be 48, if read, -48
	// if ( myIsANAOK(nID) == 48 ) read;
	virtual int __fastcall myIsANAOK(const int nID);

	// by default, this class will hold last 1000 images in terms of the cut small parts
	virtual void __fastcall mySetSavingCount(const int nCount = MY_MAX_MEM_FRAME, const int nCount2 = MY_MAX_MEM_DATA);
	virtual int  __fastcall myGetSavingCount(void);
	virtual int  __fastcall myGetSavingCount2(void);

	// save the internal images to an avi file, this avi file has s RIFF chunk identifier, "EHMW"
	// which is xy list of unsigned int values for 48 windows, each has 4 values as x_Left, x_Right, y_Left, y_Right 0-based
	// these values indicate the position where the small blocks are cut from.
	virtual void __fastcall mySaveToAVI(const char *fn);

	// read the EHMW information from the file, return 48 if correct, if fail, return 0.
	virtual int  __fastcall myGetInfo(const char *fn, unsigned int(*pXXYY)[4], const int nCount = 48);

	virtual void __fastcall FF48(const int nTID, volatile char *cThreadWork, const unsigned char *pIn2D, unsigned char *pOut2D, double *pOut1D, double *pOutDD, MyLMFIT &lm, double *pFitOut, const int nEx = 2);
	
	// T1 is for main interval, running set to 0, sleep set to 3000
	// T2 is for thread 7 8, running set to 57-77, sleep set to 7000
	virtual void __fastcall mySetThreadSleep(const bool bSleep = false);

	virtual int __fastcall myRunningThread(void);

	virtual bool __fastcall mySetMEMFile(const int n1, const int n2, const int nBlockCount = MY_MAX_MEM_FRAME);
	virtual bool __fastcall myCloseMEMFile(void);
	virtual bool __fastcall myAddData(const int nIndex, const char *pBuf);
	virtual void __fastcall mySetUser(const int* p, const int nCount = 0);


	volatile int *pThreadState;
	int nThreadCount;
	volatile int nThreadRunning;

	int nXXYY[48][8];	//XXYY are pore_X_Left, X_Right, Y_Left, Y_Right, XY_corner_Left, right.
	double preFit[96][12]; // AF1_Amp, AF2_X0, AF3_Sigma, AF6_Offset, AF7_Sigma2, Error_5_Values, BK, Corrected value	
	double preVal[48][4];
	double *preFitLast;
	double *pANAIn;
	double *pLineIn;
	unsigned char ImgCutIn[MY_OUT_IMG_X * MY_OUT_IMG_Y];

	int nOffsetIn[48][2];
	int nOffsetOut[48][2];
	//volatile char *cThreadWork;
	myOBJXs sFrames;
	int nFrameSize[3]; // 240 * 6, 60 * 8
	int nImageSize[3];

	double DataXY[MY_SAMLL_IMG_X * 2];

	volatile int nStartEnable;
	volatile long long llFrameLastT;
	volatile myOBJ48_Tag * sOBJ_LastT;

protected:

	myThreads *tt;
	uMY_INFO_DATA ttData; //uMY_INFO_DATA *ttData;

	int nMaxImgCount;
	int nMaxDatCount;
	int nFrameRate;
	volatile bool bLoad_OK;
};
//---------------------------------------------------------------------------
void __fastcall myMemcpy(char *A, const char *B, int n);
//---------------------------------------------------------------------------
//
//
//---------------------------------------------------------------------------
// Following is the example to use this class
//---------------------------------------------------------------------------
//#include "myEHM_FIT_M_01.h"
//
//unsigned char *pucA; // point to the memory of the image buf
//int nSSB[6] = { 258, 103, 258, 155, 0, 0 }; //Sets for Small Block: n1, n2, d-n1, d-n2, start_n1, Start_n2; n1 is contiguous in memory, usually X-dir.
//int nAdimY, nAdimX; // image X and Y pixel size
//
//void Test_Example(void){
//
//	myEHM_FIT ff;
//	ff.myINIT_48(pucA, nAdimX, nAdimY, nSSB);  // precalulation, if error, user shall stop adding.
//
//	if (ff.isINIT_OK()){
//		int n; // predefine saving size
//		unsigned long long pnDIM[4]; pnDIM[0] = ff.nFrameSize[0]; pnDIM[1] = ff.nFrameSize[1]; pnDIM[2] = n;
//		long long nLen2 = ff.nFrameSize[0] * ff.nFrameSize[1];
//		//plhs[0] = mxCreateNumericArray(nn, pnDIM, mxUINT8_CLASS, mxREAL); // raw frames in small blocks
//		char *pImgOut;
//
//		pnDIM[0] = 48; pnDIM[1] = n;
//		//plhs[1] = mxCreateNumericArray(2, pnDIM, mxDOUBLE_CLASS, mxREAL); // distances
//		double *pdOut;
//
//		pnDIM[0] = 48; pnDIM[1] = n;
//		//plhs[2] = mxCreateNumericArray(2, pnDIM, mxINT8_CLASS, mxREAL); // threads ID using
//		char *pthOut;
//
//		int nIDs[10], *nID;
//		if (n > 10) nID = new int[n];
//		else nID = nIDs;
//
//		long long ll = 0, nLen = nAdimX*nAdimY;
//
//		for (int i = 0; i < n; i++)
//		{
//			nID[i] = ff.myANA_48(pucA + ll, NULL); ll += nLen;   // this line is the main line to add the images
//			//if (i < n - 1) continue;
//			//printf("%d. add ID = %d. State:", i, nID[i]);
//			//for (int j = 0; j < i; j++){
//			//	printf("%d ", ff.myIsANAOK(nID[j]));
//			//}
//			//printf("\n");
//		}
//
//		for (int i = 0; i < n; i++)
//		{
//			int nID2 = ff.myGetANA(nID[i], pdOut + 48 * i); //(double *)mxGetData(plhs[1])) + 48 * i // this line is the main line to read the results
//			//int nState = ff.myIsANAOK(nID[i]);
//			//printf("%d nID = %d, nID2 = %d, nState = %d.\n", i, nID[i], nID2, nState);
//			myOBJ48_Tag * pp = ff.sFrames.myFindID(nID[i], false);  // this line is the main line to read the raw image
//			//
//			if (pp != NULL){
//				if (pp->pImgCut != NULL)
//					myMemcpy(pImgOut + nLen2 * i, (const char *)pp->pImgCut, nLen2); //(char *)mxGetData(plhs[0]) + nLen2 * i // example to copy the raw data
//				myMemcpy(pthOut + 48 * i, (const char *)pp->nTID, 48); //(char *)mxGetData(plhs[2]) + 48 * i // example to copy thread using information
//			}
//		}
//		if (nID != nIDs && nID != NULL)
//			delete[] nID;
//		ff.mySaveToAVI("ttTest.avi"); // example to save the last some images to a file
//	}
//}
#endif
//---------------------------------------------------------------------------