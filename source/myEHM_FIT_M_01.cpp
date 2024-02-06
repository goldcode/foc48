//---------------------------------------------------------------------------
#include "myEHM_FIT_M_01.h"
//---------------------------------------------------------------------------
#define MY_NO_FFT			 // used by myFIT_FUN_C2.hpp

//#include "D:\MyDocuments\Visual Studio 2013\Projects\EHM48\DLL\myEHMDLL\myFIT_FUN_C2.h"	//#include <math.h> #include <stdio.h> #include <stdlib.h>
#include "myFIT_FUN_C2.h"	//#include <math.h> #include <stdio.h> #include <stdlib.h>
//---------------------------------------------------------------------------
#include "myMutex0.h"
//---------------------------------------------------------------------------
// this line is used for matlab calling and debuging
//#define MY_DEBUG_PNT
//---------------------------------------------------------------------------
//#ifdef _DEBUG
//#pragma comment(linker, "/DEFAULTLIB:C:\\labhub\\FIT_TEST\\DOingF\\foc48\\lib\\myFIT_FUN_C2.lib")
//#pragma comment(linker, "/DEFAULTLIB:C:\\labhub\\FIT_TEST\\DOingF\\foc48\\lib\\myMutex0.lib")
//#pragma comment(linker, "/DEFAULTLIB:C:\\labhub\\FIT_TEST\\DOingF\\foc48\\lib\\myListManger.lib")
//#pragma comment(linker, "/DEFAULTLIB:C:\\labhub\\FIT_TEST\\DOingF\\foc48\\lib\\myFIT_FUN_C2.lib")
//#endif
//---------------------------------------------------------------------------
template<typename myT>
inline void myDel_ex(myT * &p){
	if (p != NULL)
		delete[] p;
	p = NULL;
}
inline void __fastcall myMemcpyEx(unsigned long long *A, const unsigned long long *B, int n)
{
	for (int i = 0; i < n; i++) {
		A[i] = B[i];
	}
}
void __fastcall myMemcpy(char *A, const char *B, int n){
	int nn = (n >> 3);
	myMemcpyEx((unsigned long long *)A, (const unsigned long long *)B, nn);
	for (int i = (nn << 3); i < n; i++) {
		A[i] = B[i];
	}
}
inline void __fastcall myMemsetEx(unsigned long long *A, const unsigned char B1, int n)
{
	union{
		struct{
			unsigned char c0;
			unsigned char c1;
			unsigned char c2;
			unsigned char c3;
			unsigned char c4;
			unsigned char c5;
			unsigned char c6;
			unsigned char c7;
		};
		unsigned long long cc8;
	};
	c7 = c6 = c5 = c4 = c3 = c2 = c1 = c0 = B1;
	for (int i = 0; i < n; i++) {
		A[i] = cc8;
	}
}
void __fastcall myMemset(char *A, const unsigned char B, int n){
	int nn = (n >> 3);
	myMemsetEx((unsigned long long *)A, B, nn);
	for (int i = (nn << 3); i < n; i++) {
		A[i] = B;
	}
}
//---------------------------------------------------------------------------
void __fastcall myInit_FIT_GL_MEM(sMY_FIT_GL_MEM &m, int gl_nIteration)
{	//gl_nIteration = 0
	m.pUser = NULL;
	m.gl_pOpt = NULL;
	m.gl_dlamdaFun = 0;
	m.gl_nOptCount = 0;
	m.gl_nIteration = gl_nIteration; // m.gl_nIteration = 0;
	m.gl_nIteration0 = 0;
	m.nThisID = 0x474c4d30;
	*((long long *)(m.nRes)) = 0;
}
void __fastcall myINIT_Tag(myOBJ48_Tag &m)
{
	m.ImgID = 0;
	m.pImgIn = NULL;	// The input image is stored
	m.pImgCut = NULL; // Image buffer, user can delete, but needs to set NULL after. This will be deleted by the program when quit
	m.pLine = NULL; // For each image, the 1d format data for fit, after fit, we can also delete and set to NULL
	m.pANA = new double[12 * 96];		// 12 x 96 array for ana, after, user can delete it
	myMemset((char *)(m.nTID), 0, 48);
	m.nState = 0;
	m.nMemUse = 0;
	m.nThisID = 0x573438;	// // 'W48') 573438 >>
}
void __fastcall myINIT_Tag(myOBJ48_Tag &m, long long nID, const unsigned char *pIn, const int nSize1, const int nSize2, const int nMASK = 0)
{	//nMASK = 0
	m.ImgID = nID;
	m.pImgIn = pIn;	// The input image is stored
	m.pImgCut = new unsigned char[nSize1]; // Image buffer, user can delete, but needs to set NULL after. This will be deleted by the program when quit
#ifdef	MY_USING_INTERNAL_ALLOC
	m.pLine = NULL; // For each image, the 1d format data for fit, after fit, we can also delete and set to NULL
	m.pANA = NULL;		// 12 x 96 array for ana, after, user can delete it
#else
	m.pLine = new double[nSize2]; // For each image, the 1d format data for fit, after fit, we can also delete and set to NULL
	m.pANA = new double[12 * 96];		// 12 x 96 array for ana, after, user can delete it
#endif
	myMemset((char *)(m.nTID), 0, 48);
	m.nState = 0;
	m.nMemUse = nMASK;
	m.nThisID = 0x573438;
}
// use after myINIT_Tag
// used when the size is different, hope we will not use this function in future
void __fastcall myReSet_Tag(myOBJ48_Tag &m, long long nID, const unsigned char *pIn, const int nSize1, const int nSize2)
{
	m.ImgID = nID;
	m.pImgIn = pIn;	// The input image is stored
	myDel_ex(m.pImgCut);
	m.pImgCut = new unsigned char[nSize1]; // Image buffer, user can delete, but needs to set NULL after. This will be deleted by the program when quit
	myDel_ex(m.pLine);
	m.pLine = new double[nSize2]; // For each image, the 1d format data for fit, after fit, we can also delete and set to NULL
	if (m.pANA == NULL)
		m.pANA = new double[12 * 96];		// 12 x 96 array for ana, after, user can delete it
	m.nThisID = 0x573438;
}
void __fastcall myDel_Tag_Mem(myOBJ48_Tag &m){
	myDel_ex(m.pANA);
	myDel_ex(m.pLine);
	myDel_ex(m.pImgCut);
}
//---------------------------------------------------------------------------
myOBJXs::myOBJXs(const int nSize1, const int nSize2){
	myInit(nSize1, nSize2);
}
myOBJXs::~myOBJXs(){
	ReMoveAll();
	//myDel_ex(pnXXYY);
}
void __fastcall myOBJXs::myInit(const int nSize1, const int nSize2, bool bRemoveAll)
{	// bRemoveAll = false;
	//pnXXYY = NULL;
	if (bRemoveAll)
		ReMoveAll();
	llFrameLast = llFrame = 0; sOBJ_Last = NULL; LastAdd = NULL; LastUpdate = NULL; nLastSave = 0; FirstFrame = NULL; nFirstFrameID = -1; nFrameTotalCount = 0;
	nSize_pImgCut = nSize1;
	nSize_pLine = nSize2;
}
myOBJX* __fastcall myOBJXs::NewItemX(int s, const unsigned char *pBuf, const int nMAKS)
{	//nMAKS = 0
	myOBJX *p = (myOBJX*)myLists::NewLists(1, sizeof(myOBJX));
	if (p != NULL){
		p->sOBJ = (myOBJ48_Tag *) new char[sizeof(myOBJ48_Tag)+0];
		if (p->sOBJ != NULL){
			myINIT_Tag(*(p->sOBJ), llFrame + 1, pBuf, nSize_pImgCut, nSize_pLine, nMAKS);
		}
	}
	return p;
}
myOBJX* __fastcall myOBJXs::NewItemX(int s, int pc)
{
	//int i = sizeof(myPIX3DX);
	//myOBJ_ItemX *p = NewItemX(s, s);
	//memcpy(p->sOBJ->p, PBuf, i*s);
	return NULL;
}
void __fastcall myOBJXs::ReMoveOne(const myListItem &A){
	myOBJX* p = (myOBJX*)&A;
	char *pp;
	if ((pp = (char *)(p->sOBJ)) != NULL)
	{
		if (p->sOBJ->pImgCut != NULL){
			nFrameTotalCount--; p->sOBJ->nMemUse |= MY_DEL_IMG;
			myDel_Tag_Mem(*(p->sOBJ));
		}
		delete[] pp;
	}
	myLists::ReMoveOne(A);
}
void __fastcall myOBJXs::ReMoveFirst(void)
{
	ReMoveOne(*pStart);
}
//---------------------------------------------------------------------------
// pOBJ->AddToList(*(pItem = pOBJ->NewItemX(j, j)));
// pItem = (myOBJ_ItemX *)pOBJ->myGetItemByIndex(0);
// i=pOBJ->nCount;
// pItem=(myOBJ_ItemX *)pItem->pNext;
//---------------------------------------------------------------------------


#define MY_WELL_COUNT	48	// this value defines the lock number and thread looking number

myMutex mxWell[MY_WELL_COUNT];
myMutex mxUpdate;
myMutex myThreadFit;
int nHaveDone;
//---------------------------------------------------------------------------
//      User Define Function Asymtrica
//      Y = A0 * exp(-0.5 * (((X-A1)*(X-A1)/A2/A2) + ((Y-A3)*(Y-A3)/A4/A4))) + A5; if X<A1
//      Y = A0 * exp(-0.5 * (((X-A1)*(X-A1)/A6/A6) + ((Y-A3)*(Y-A3)/A4/A4))) + A5; if X>A1
//      dy/dA5 = 1;
//      dy/dA0 = exp(-0.5 * (((X-A1)*(X-A1)/A2/A2) + ((Y-A3)*(Y-A3)/A4/A4)));
//      dy/dA1 = A0 * dy/dA0 * (X-A1)/A2/A2; dy/dA2 = dy/dA1 * (X-A1)/A2;
//		dy/dA3 = A0 * dy/dA0 * (X-A3)/A4(A4; dy/dA4 = dy/dA3 * (X-A3)/A4;
//---------------------------------------------------------------------------
void myGauss2D_Asym_tt(double x, double *par, double *y, double *dyda, MyLMFIT *pThis)
{	// myFitFun_Old(x, par, y, dyda, pThis->nCountPar);
	double dtmpA, dtmpB, dtmpD, dA2, dA4;
	if (x > par[1]){
		dtmpA = (x - par[1]) / (dA2 = par[6]);
		dtmpB = (pThis->pdIn_XX[1][pThis->nCurIndex] - par[3]) / (dA4 = par[4]);
		dtmpD = (dyda[0] = exp((dtmpA * dtmpA + dtmpB * dtmpB) * (-0.5)))*par[0];
		dyda[5] = 1;
		dyda[6] = (dyda[1] = dtmpA / dA2 * dtmpD) * dtmpA;
		dyda[4] = (dyda[3] = dtmpB / dA4 * dtmpD) * dtmpB;
		dyda[2] = 0;
	}
	else if (x < par[1]){
		dtmpA = (x - par[1]) / (dA2 = par[2]);
		dtmpB = (pThis->pdIn_XX[1][pThis->nCurIndex] - par[3]) / (dA4 = par[4]);
		dtmpD = (dyda[0] = exp((dtmpA * dtmpA + dtmpB * dtmpB) * (-0.5)))*par[0];
		dyda[5] = 1;
		dyda[2] = (dyda[1] = dtmpA / dA2 * dtmpD) * dtmpA;
		dyda[4] = (dyda[3] = dtmpB / dA4 * dtmpD) * dtmpB;
		dyda[6] = 0;
	}
	else{
		//dtmpA = 0;
		dtmpB = (pThis->pdIn_XX[1][pThis->nCurIndex] - par[3]) / (dA4 = par[4]);
		dtmpD = (dyda[0] = exp((dtmpB * dtmpB) * (-0.5)))*par[0];
		dyda[5] = 1;
		//dyda[2] = (dyda[1] = dtmpA / dA2 * dtmpD) * dtmpA;
		dyda[2] = dyda[1] = dyda[6] = 0;
		dyda[4] = (dyda[3] = dtmpB / dA4 * dtmpD) * dtmpB;
	}

	*y = dtmpD + par[5];
	//printf("x=%f, y=%f,dyda=%f,%f,%f,%f,%f,%f\n", x, *y, dyda[0], dyda[1], dyda[2], dyda[3], dyda[4], dyda[5]);
}
//---------------------------------------------------------------------------
void myGauss2D_Asym_Range_tt(double *par, MyLMFIT *pThis)
{	//pThis->nCountPar;   // this is number of par
	//pThis->nCountFit;   // real fitting numbers
	//myRangeCheck_Old(par, pThis->nCountPar, pThis->nPara, pThis->nCountFit);

	bool bSet = false;
	sMY_FIT_GL_MEM *sGL = (sMY_FIT_GL_MEM *)(pThis->pdIn_XX[4]);
	if (sGL != NULL){
		if (sGL->nThisID == 0x474c4d30){
			double(*gl_pOpt)[2] = sGL->gl_pOpt;
			int gl_nOptCount = sGL->gl_nOptCount;
			if (gl_nOptCount > 0 && gl_pOpt != NULL)
			{
				int *nPara = pThis->nPara;
				for (int i = 0; i<gl_nOptCount; i++){
					if (nPara[i] > 0)
					if (par[i]<gl_pOpt[i][0])par[i] = gl_pOpt[i][0];
					else if (par[i]>gl_pOpt[i][1])par[i] = gl_pOpt[i][1];
				}
				if (gl_nOptCount < 3)
				if (par[2] <= 0)par[2] = 1e-17;
				if (gl_nOptCount < 5)
				if (par[4] <= 0)par[4] = 1e-17;
				if (gl_nOptCount < 7)
				if (par[6] <= 0)par[6] = 1e-17;
				bSet = true;
			}

			// dlamdaFun can be used to control the break of it
			// after UserFun_Range, save dlamdaFun, and set dlamdaFun = MY_MAX_LAMDA will stop
			// next time in the last UserFun_Fit, restore dlamdaFun
			if (sGL->gl_nIteration != MY_MAX_TIMES_A && sGL->gl_nIteration > 0){
				if (sGL->gl_nIteration0++ >= sGL->gl_nIteration){
					sGL->gl_dlamdaFun = pThis->dlamdaFun;
					pThis->dlamdaFun = MY_MAX_LAMDA * 100;
				}
			}
		}
	}
	if (bSet == false){
		if (par[0] <= 0)par[0] = 1e-17;
		if (par[2] <= 0)par[2] = 1e-17;
		//if(par[3]<=0)par[0]=1e-17;
		if (par[4] <= 0)par[4] = 1e-17;
		if (par[6] <= 0)par[6] = 1e-17;
	}
}
//---------------------------------------------------------------------------
inline void __fastcall myMemcpyEx_tt(unsigned long long *A, const unsigned long long *B, int n)
{
	for (int i = 0; i < n; i++) {
		A[i] = B[i];
	}
}
void __fastcall myMemcpy_tt(char *A, const char *B, int n){
	int nn = (n >> 3);
	myMemcpyEx_tt((unsigned long long *)A, (const unsigned long long *)B, nn);
	for (int i = (nn << 3); i < n; i++) {
		A[i] = B[i];
	}
}
// pIn2D is already starting, n1 is contiguous, n1,n2 are small image size
// s1 s2 is sum lines, used for 1D, 0-based, sum [s1, s2], both are < n2
//
inline double myCut2D_tt(unsigned char *pOut2D, const int nJumpOut, const unsigned char *pIn2D, const int n1, const int n2, const int nJumpIn, double *pOut1D, const int s1, const int s2)
{

	for (int i = 0; i < s1; i++)
	{
		myMemcpy_tt((char*)pOut2D, (char*)pIn2D, n1);
		pOut2D += nJumpOut; pIn2D += nJumpIn;
	}
	for (int j = 0; j < n1; j++)
		pOut1D[j] = pIn2D[j];
	myMemcpy_tt((char*)pOut2D, (char*)pIn2D, n1);
	pOut2D += nJumpOut; pIn2D += nJumpIn;
	for (int i = s1 + 1; i <= s2; i++)
	{
		for (int j = 0; j < n1; j++)
			pOut1D[j] += pIn2D[j];
		myMemcpy_tt((char*)pOut2D, (char*)pIn2D, n1);
		pOut2D += nJumpOut; pIn2D += nJumpIn;
	}
	for (int i = s2 + 1; i < n2; i++)
	{
		myMemcpy_tt((char*)pOut2D, (char*)pIn2D, n1);
		pOut2D += nJumpOut; pIn2D += nJumpIn;
	}
	double dd = s2 - s1 + 1; dd = 1 / dd;
	double dm = 0;
	for (int j = 0; j < n1; j++){
		pOut1D[j] *= dd;
		dm += pOut1D[j];
	}
	return dm;
}

inline void myGetS1S2(const int nP, int &s1, int &s2, const int nL = 11, const int n2 = 60)
{
	s1 = nP - nL; if (s1 < 0) s1 = 0;
	s2 = nP + nL; if (s2 >= n2) s2 = n2 - 1;
}

inline double myDD(const double X, const double Y, const double X2, const double Y2)
{
	double dx = (X - X2); double dy = (Y - Y2);
	return sqrt(dx * dx + dy * dy);
}

// vv=myCorrection(AF,GS,17,2);
//function vv=myCorrection(AF, GS, dAD, opt)
//if nargin<4;opt=1;end
//    %    1.Top          2 Top,X    3.Poll-Top   4 Sigma1    5sigma2.   6,7   8ll    delta   real_X .
//    vv=[AF(1)+AF(6),    AF(2),      0,            AF(3),      AF(7),   0, 0, 0,      0 ,     0];
//    vv(3)=GS(floor(vv(2)+0.5));
//    vv([6,7])=myGauss2D_Cal([vv(2)-vv(4),0;vv(2)+vv(5),0],AF,1);
//    if opt(1)==1
//        vv(8)=(vv(1)-vv(3)+dAD)/(vv(1)-vv(6))*(vv(4)+vv(5));
//    else
//        vv(8)=dAD;
//    end
//    vv(9)=vv(8)/2 - vv(8)/(vv(4)+vv(5))*vv(4);
//    vv(10)=vv(2)+vv(9);
//end

// dd / 2 - dd / (AF3 + AF7) * AF3 + AF2
// dd * ( 0.5 - AF3 / (AF3 + AF7) ) + AF2
//

void __fastcall myEHM_FIT::FF48(const int nTID, volatile char *cThreadWork, const unsigned char *pIn2D, unsigned char *pOut2D, double *pOut1D, double *pOutDD, MyLMFIT &lm, double *pFitOut, const int nEx)
{	// const int nEx = 2
	//  pOut2D is 240 * 6, 60 * 8
	int n1 = nImageSize[0];
	double *pOut1DR = pOut1D + MY_SAMLL_IMG_X * 48; // 120 * 48
	int nFit2 = 48 * (10 + nEx);

	sMY_FIT_GL_MEM *sGL = (sMY_FIT_GL_MEM *)(lm.pdIn_XX[4]);
	//volatile char *cThreadWork = sFrames.sOBJ_Last->nTID;

	for (int i = 0; i < 48; i++){
		if (mxWell[i].try_lock() == false)continue;
		if (cThreadWork[i] != 0){
			mxWell[i].unlock();	continue;
		}
		cThreadWork[i] = -nTID;

		// i is 0-based well ID
		// get image, get data for fit, save fit result

		int s1, s2, i2;
		myGetS1S2(nXXYY[i][2] - 1, s1, s2);
		double *pDataZ = pOut1D + (i2 = i * MY_SAMLL_IMG_X);
		double dm = myCut2D_tt(pOut2D + nOffsetOut[i][0], MY_OUT_IMG_X, pIn2D + nOffsetIn[i][0], MY_SAMLL_IMG_X, MY_SAMLL_IMG_Y, n1, pDataZ, s1, s2);
		dm *= 1.0 / MY_SAMLL_IMG_X;

		lm.mySetData(DataXY, pDataZ, MY_SAMLL_IMG_X); double *preFit1 = preFit[i];
		int imx = (int)(preFit1[1] + 0.5);
		lm.pPara[0] = pDataZ[imx];
		lm.pPara[1] = imx;
		lm.pPara[2] = preFit1[2];
		lm.pPara[5] = preFit1[3] + dm - preFit1[10];
		lm.pPara[6] = preFit1[4];

		double dChisq, dAlamda; sGL->gl_nIteration0 = 0;
		int j = lm.myLM_Min2(dChisq, dAlamda);
		double pFitAOut[7];
		double p2[7];
		lm.myGetFitResult(pFitAOut, p2);

		double *pFitAEOut1 = pFitOut + i * (10 + nEx);
		pFitAEOut1[0] = pFitAOut[0]; // AF1 A
		pFitAEOut1[1] = pFitAOut[1]; // AF2 X
		pFitAEOut1[2] = pFitAOut[2]; // AF3 Sx
		pFitAEOut1[3] = pFitAOut[5]; // AF6 Off
		pFitAEOut1[4] = pFitAOut[6]; // AF7
		pFitAEOut1[5] = p2[0];
		pFitAEOut1[6] = p2[1];
		pFitAEOut1[7] = p2[2];
		pFitAEOut1[8] = p2[5];
		pFitAEOut1[9] = p2[6];
		pFitAEOut1[10] = dm;
		double dd1 = pFitAEOut1[11] = 17 * (0.5 - pFitAOut[2] / (pFitAOut[2] + pFitAOut[6])) + pFitAOut[1];

		myGetS1S2(nXXYY[i][3] - 1, s1, s2);
		pDataZ = pOut1DR + i2;
		dm = myCut2D_tt(pOut2D + nOffsetOut[i][1], MY_OUT_IMG_X, pIn2D + nOffsetIn[i][1], MY_SAMLL_IMG_X, MY_SAMLL_IMG_Y, n1, pDataZ, s1, s2);
		dm *= 1.0 / MY_SAMLL_IMG_X;

		lm.mySetData(DataXY, pDataZ, MY_SAMLL_IMG_X); preFit1 = preFit[i + 48];
		imx = (int)(preFit1[1] + 0.5);
		lm.pPara[0] = pDataZ[imx];
		lm.pPara[1] = imx;
		lm.pPara[2] = preFit1[2];
		lm.pPara[5] = preFit1[3] + dm - preFit1[10];
		lm.pPara[6] = preFit1[4];

		sGL->gl_nIteration0 = 0;
		j = lm.myLM_Min2(dChisq, dAlamda);
		lm.myGetFitResult(pFitAOut, p2);

		pFitAEOut1 += nFit2;
		pFitAEOut1[0] = pFitAOut[0];
		pFitAEOut1[1] = pFitAOut[1];
		pFitAEOut1[2] = pFitAOut[2];
		pFitAEOut1[3] = pFitAOut[5];
		pFitAEOut1[4] = pFitAOut[6];
		pFitAEOut1[5] = p2[0];
		pFitAEOut1[6] = p2[1];
		pFitAEOut1[7] = p2[2];
		pFitAEOut1[8] = p2[5];
		pFitAEOut1[9] = p2[6];
		pFitAEOut1[10] = dm;
		pFitAEOut1[11] = 17 * (0.5 - pFitAOut[2] / (pFitAOut[2] + pFitAOut[6])) + pFitAOut[1];

		pOutDD[i] = myDD(nXXYY[i][4] + dd1, nXXYY[i][5] + nXXYY[i][2], nXXYY[i][6] + pFitAEOut1[11], nXXYY[i][7] + nXXYY[i][3]);

		cThreadWork[i] = nTID;
		mxWell[i].unlock();
	}
}

// using uMY_INFO_DATA -> dd for class

volatile bool gl_bSleep = false;
void __fastcall myEHM_FIT::mySetThreadSleep(const bool bSleep)
{	//const int T1 = 0, const int T2 = 77
	// bSleep = false
	//gl_nTime1_us = T1;
	//gl_nTime2_us = T2;
	gl_bSleep = bSleep;
}

void * myMT_Fit_EX(uMY_INFO_DATA *pI, int *nI, volatile int &nUserCancel)
{
	myEHM_FIT * ehmFit = dynamic_cast<myEHM_FIT *>((myEHM_FIT *)((void *)(pI->dd.pIn)));
	while (1){
		if (ehmFit == NULL) break;
		// nI[1] is 1-based ID
		if (nI[1] > ehmFit->nThreadCount) break;
		ehmFit->pThreadState[nI[1] - 1] = nI[1];

		////double 		 	*pOut = pI->dd.pOut;
		////int             x0 = nI[0], x1 = nI[1];

		MyLMFIT lm(7, myGauss2D_Asym_tt, myGauss2D_Asym_Range_tt);
		sMY_FIT_GL_MEM sGL;
		myInit_FIT_GL_MEM(sGL, MY_MAX_TIMES_A); //sGL.gl_nOptCount = 0; sGL.gl_nIteration = MY_MAX_TIMES_A; sGL.gl_dlamdaFun = 0; sGL.gl_nIteration0 = 0;

		//                       0          1                          2            3                           4           5           6
		double dOpt[7][2] = { { 0, 255 }, { 0, MY_CUT_IMAGE_Y - 1 }, { 0.7, 20 }, { 0, MY_CUT_IMAGE_Y - 1 }, { 0.7, 20 }, { 0, 255 }, { 0.7, 20 } };
		dOpt[1][1] = MY_CUT_IMAGE_X - 1; dOpt[3][1] = MY_CUT_IMAGE_X - 1;

		sGL.gl_pOpt = dOpt; sGL.gl_nOptCount = 7;

		sGL.gl_nIteration = 33;

		lm.pdIn_XX[0] = ehmFit->DataXY;
		lm.pdIn_XX[1] = ehmFit->DataXY + MY_SAMLL_IMG_X;	//lm.pdIn_XX[2] = xData + (nCountD << 1);
		lm.pdIn_XX[4] = (double *)((void *)(&sGL));

		//double *pDataW = NULL;

		//Set fitting data, Check if weight is used
		//if (pDataW != NULL) lm.mySetData(pDataXY, pDataZ, nCountD, pDataW);
		//else lm.mySetData(pDataXY, pDataZ, nCountD);

		//                    0  1   2  3  4  5  6
		//                    1  1   1  0  0  1  1
		double dFitAIn[8] = { 0, 30, 9, 0, 1, 0, 9, 0 };
		int nFitSwitch[7] = { 1, 1, 1, 0, 0, 0, 1 };
		lm.mySetParaAll(7, dFitAIn, nFitSwitch); //lm.mySetPara(0, 111);
		//myBuildGroup(nFit_fit, nCountA, nFit_gX); // build nFit_gIndex;
		//for(i=0; i<nCountA; i++)printf("nFit_gX[%d]=%d\n",i,nFit_gX[i]);

		while (nUserCancel == 0){
			// if there are new
			// fit
			volatile myOBJ48_Tag * sFrame = NULL;
			if (nI[1] == 1){ // thread 1 will run this
				if (ehmFit->sFrames.HasNew()){
					if ((sFrame = ehmFit->sFrames.myFindFirstNew()) != NULL){
						// known bug, following could not be safe, if program is deleteing this item, e.g. others release the object by quit the thread
						// But also, this bug could not happen, ObjX is deleteing if class is quiting, before that thready is ready stopped, this code will not run.
						// If user do not release, e.g. if reached the limit of max frames and new items is adding, and we want to ANA this old one.
						// usually will not happen, for ana ready start when it is adding.
#ifdef	MY_USING_INTERNAL_ALLOC
						if (sFrame->pImgIn != NULL && (sFrame->pImgCut != NULL || (sFrame->nMemUse & MY_DEL_IMG) != 0) && ehmFit->pANAIn != NULL && ehmFit->pLineIn != NULL && sFrame->nState == 0){
#else
						if (sFrame->pImgIn != NULL && (sFrame->pImgCut != NULL || (sFrame->nMemUse & MY_DEL_IMG) != 0) && sFrame->pANA != NULL && sFrame->pLine != NULL && sFrame->nState == 0){
#endif
							mxUpdate.lock();
							ehmFit->llFrameLastT = ehmFit->sFrames.llFrameLast; // this is to save a ID for checking //sFrame->ImgID is 1 higher;
							ehmFit->sOBJ_LastT = sFrame;
							mxUpdate.unlock();

							myThreadFit.lock();
							ehmFit->nThreadRunning = 0;
							ehmFit->nStartEnable = ehmFit->nThreadCount;
							myThreadFit.unlock();

						}
						else sFrame = NULL;
					}
				}
			}
			else{ // all other thread will run this
				if (ehmFit->nStartEnable > 0){
					//mxUpdate.lock();
					sFrame = ehmFit->sOBJ_LastT; //ehmFit->sFrames.sOBJ_Last;
					//mxUpdate.unlock();
				}
			}
			if (sFrame != NULL){
				bool bFit;
				myThreadFit.lock();
				//if (nI[1] == 1) bFit = true;
				//else{
				//	if (sFrame->ImgID == ehmFit->llFrameLastT + 1 && sFrame->nState == 0)
				//		bFit = true;
				//	else bFit = false;
				//}
				ehmFit->nThreadRunning++; bFit = true;
				myThreadFit.unlock();
				if (bFit)
#ifdef	MY_USING_INTERNAL_ALLOC
					ehmFit->FF48(nI[1], sFrame->nTID, sFrame->pImgIn, ((sFrame->pImgCut == NULL) ? ehmFit->ImgCutIn : sFrame->pImgCut), ehmFit->pLineIn, (double *)(sFrame->dResult), lm, ehmFit->pANAIn, 2);
#else
					ehmFit->FF48(nI[1], sFrame->nTID, sFrame->pImgIn, ((sFrame->pImgCut == NULL) ? ehmFit->ImgCutIn : sFrame->pImgCut), sFrame->pLine, (double *)(sFrame->dResult), lm, sFrame->pANA, 2);
#endif
				myThreadFit.lock();
				if (ehmFit->nStartEnable >= ehmFit->nThreadCount)
				{	// this is the first one
					mxUpdate.lock();
					if (ehmFit->llFrameLastT == ehmFit->sFrames.llFrameLast){
						ehmFit->sOBJ_LastT = NULL;
						ehmFit->sFrames.llFrameLast++;
					}
					mxUpdate.unlock();
					ehmFit->nStartEnable = ehmFit->nThreadCount - 1; // this ensures later thread will not enter here.
				}
				else{ ehmFit->nStartEnable--; }


				ehmFit->nThreadRunning--;
				if (ehmFit->nThreadRunning == 0)
				{	// this is the last one, we can update and delete the middle step memory.
					if (sFrame->nState == 0){
						int j = 0;
						for (int i = 0; i < 48; i++)
						{
							if (sFrame->nTID[i]>0)j++;
							else break;
						}
						sFrame->nState = j;
						if (sFrame->pANA != NULL && (sFrame->nMemUse & MY_NO_DEL_ANA) == 0){
							delete[] sFrame->pANA; sFrame->pANA = NULL; // set to NULL is need, we will not release again in ~()
						}
						if (sFrame->pLine != NULL && (sFrame->nMemUse & MY_NO_DEL_LINE) == 0){
							delete[] sFrame->pLine; sFrame->pLine = NULL;
						}
					}
					if (ehmFit->nStartEnable != 0)
						ehmFit->nStartEnable = 0;
				}
				myThreadFit.unlock();
				while (ehmFit->nStartEnable != 0 && nUserCancel == 0) // wait for others to finish
					Sleep(0);
			}
			else{
				if (gl_bSleep)
					Sleep(1);
				else if (nI[1] < 6){
					//Sleep(0);
					std::this_thread::sleep_for(std::chrono::microseconds(0)); //std::chrono::microseconds(0), std::chrono::nanoseconds(0)
				}
				else{
					//	//if (ehmFit->nStartEnable == 0)
					std::this_thread::sleep_for(std::chrono::microseconds(77));
					// std::chrono::microseconds(7) CPU 100%
					// std::chrono::microseconds(57) CPU 75 %
					// std::chrono::microseconds(77) CPU 75-77 %
					//	//else
					//	//while (ehmFit->nStartEnable != 0 && nUserCancel == 0) // wait for others to finish, this can be reach when other is fast and  sFrame = NULL;
					//	//	Sleep(0);
				}
			}
		}
		ehmFit->pThreadState[nI[1] - 1] = -nI[1]; // set to - mean finished
		break;
	}
	return NULL;  // this line must be have, but no need to return NULL
}
// we suppose X is contiguous
// each buffer is nX * nY size
void __fastcall myGrid2D_XY(double *pX, double *pY, const int nX, const int nY)
{
	for (int j = 0; j < nY; j++)
	for (int i = 0; i < nX; i++){
		*pX++ = i;
		*pY++ = j;
	}
}
//---------------------------------------------------------------------------
// Following is for class myEHM_FIT
//---------------------------------------------------------------------------
myEHM_FIT::myEHM_FIT(const int nThreadNumber)
{
	nStartEnable = 0; nThreadRunning = 0; llFrameLastT = 0; sOBJ_LastT = NULL; bLoad_OK = false;
#ifdef	MY_USING_INTERNAL_ALLOC
	pLineIn = new double[MY_SAMLL_IMG_X * 96];
	pANAIn = new double[96 * 12];
	preFitLast = pANAIn;
#else
	pLineIn = NULL;
	pANAIn = NULL;
	preFitLast = (double *)preFit;
#endif
	myGrid2D_XY(DataXY, DataXY + MY_SAMLL_IMG_X, MY_SAMLL_IMG_X, 1);

	//ttData = new uMY_INFO_DATA;
	nThreadCount = nThreadNumber;
	if (nThreadCount < 1) nThreadCount = 1;
	else if (nThreadCount>100) nThreadCount = 100;

	pThreadState = new volatile int[nThreadCount];
	if (pThreadState != NULL){
		for (int i = 0; i < nThreadCount; i++)
		{
			pThreadState[i] = 0;  // 0 not started, after atart, it will be ID
		}
		tt = new myThreads(nThreadNumber);
	}
	else{ // mem problem, we have no thread
		tt = NULL;
	}

	nFrameSize[0] = MY_OUT_IMG_X; nFrameSize[1] = MY_OUT_IMG_Y;
	mySetSavingCount(); // const int nCount = MY_MAX_MEM_FRAME, const int nCount2 = MY_MAX_MEM_DATA

}
myEHM_FIT::~myEHM_FIT(){

	while (tt != NULL)
	{
		tt->SetUserCancel(1);
		delete tt; break;
	}
	if (pThreadState != NULL)
		delete[] pThreadState;
	myDel_ex(pANAIn);
	myDel_ex(pLineIn);
	myCloseMEMFile();
}
int __fastcall myEHM_FIT::myRunningThread(void)
{
	int n = 0;
	for (int i = 0; i < nThreadCount; i++)
	{
		if (pThreadState[i]>0) n++;
	}
	return n;
}
//---------------------------------------------------------------------------

#ifdef MY_USING_FUNCTION
//#include "myEHM_ANA_01.h"
extern "C" __declspec(dllexport)
int mySTEP1_48e(const unsigned char *pImg, const int n1, const int n2, const int *nSSB, int *pnXXYY_MEM, double *fitP12_MEM, double *val4_MEM = NULL);
extern "C" __declspec(dllexport)
int __fastcall mySTEP1_48(const unsigned char *pImg, const int n1, const int n2, const int *nSSB, int *pnXXYY_MEM, double *fitP12_MEM);
#else
typedef int(*_mySTEP1_48e)(const unsigned char *pImg, const int n1, const int n2, const int *nSSB, int *pnXXYY_MEM, double *fitP12_MEM, double *val4_MEM);
typedef int(*_mySTEP1_48)(const unsigned char *pImg, const int n1, const int n2, const int *nSSB, int *pnXXYY_MEM, double *fitP12_MEM);
typedef int(*DLL_myTest)(const unsigned short *pIn, const int m, const int n, const int z, unsigned short *pOut);
#endif // MY_USING_DLL

HMODULE __fastcall myLoadLibrary(void)
{
	HMODULE hDLL0 = LoadLibraryA("my48WellFF_vF.dll");
	if (hDLL0 == NULL) {
		hDLL0 = LoadLibraryA("C:\\labhub\\my48WellFF_vF.dll");
		if (hDLL0 == NULL) {
			hDLL0 = LoadLibraryA("C:\\labhub\\import\\my48WellFF_vF.dll");
			if (hDLL0 == NULL) {
				hDLL0 = LoadLibraryA("D:\\MyDocuments\\Visual Studio 2013\\Projects\\EHM48\\DLL\\x64\\Release\\my48WellFF_vF.dll");
			}
		}
	}
	return hDLL0;
}

// n1, n2 is input image size, n1 is contiguous mem-length
// sets for small block      n1    n2   d-n1   d-n2    start_n1    Start_n2
// nSSB[6] =                [258, 103,  258,   155,    0,             0          ];  % 0 based
void __fastcall myEHM_FIT::myINIT_48(const unsigned char *pImg, const int n1, const int n2, const int *nSSB)
{
	; // nSize1 = 60 * 8 * 240 * 6, const int nSize2 = 120 * 96

	nImageSize[0] = n1; nImageSize[1] = n2; nImageSize[2] = 1;
#ifdef MY_USING_FUNCTION
	int nCount = mySTEP1_48(pImg, n1, n2, nSSB, (int *)nXXYY, (double *)preFit); //XXYY are pore_X_Left, X_Right, Y_Left, Y_Right, XY_corner_Left, right.
	for (int i = 0; i < 48; i++)
	{
		nOffsetIn[i][0] = nXXYY[i][4] + nXXYY[i][5] * n1;
		nOffsetIn[i][1] = nXXYY[i][6] + nXXYY[i][7] * n1;
		int xi = i % 6;
		int yi = i / 6;
		nOffsetOut[i][0] = xi * (MY_SAMLL_IMG_X * 2) + yi * MY_SAMLL_IMG_Y * MY_OUT_IMG_X;  //// 240 * 6, 60 * 8
		nOffsetOut[i][1] = nOffsetOut[i][0] + MY_SAMLL_IMG_X; // 120
	}

	if (sFrames.nSize_pImgCut != MY_OUT_IMG_X * MY_OUT_IMG_Y && sFrames.nSize_pLine != MY_SAMLL_IMG_X * 96){
		sFrames.myInit(MY_OUT_IMG_X * MY_OUT_IMG_Y, MY_SAMLL_IMG_X * 96, true);
		//myDel_ex(pLineIn);
	}

	if (myRunningThread() == 0) // if not start
	{
		ttData.dd.pIn = (double *)((void *)this);
		ttData.dd.pOut = NULL;
		tt->SetUserCancel(0);
		int nThreadCount = tt->mySetN(MY_THREAD_NUMBER, MY_THREAD_NUMBER); // using internal memory for parallel transfer, same value means the internal mem is 0-based ID
		tt->Start(0, myMT_Fit_EX, &ttData, tt->pIntBuf, nThreadCount);
		bLoad_OK = true;
	}
#else
	int nDLL = 0;
	while (1) {
		HMODULE hDLL0 = myLoadLibrary();
		if (hDLL0 == NULL) { break; }
		while (1) {
			DLL_myTest myTest = (DLL_myTest)GetProcAddress(hDLL0, "myTest");
			if (myTest == NULL) { break; }
			else {
				int i = myTest(NULL, 3, 14, 0, NULL);
				if (i != 17)break;
			}
			_mySTEP1_48e mySTEP1_48e = (_mySTEP1_48e)GetProcAddress(hDLL0, "mySTEP1_48e");
			if (mySTEP1_48e == NULL) { break; }
			else {															//                                  0      1        2        3     4 5             6 7
				int nCount = mySTEP1_48e(pImg, n1, n2, nSSB, (int*)nXXYY, (double*)preFit, (double*)preVal); //XXYY are pore_X_Left, X_Right, Y_Left, Y_Right, XY_corner_Left, right.
				nDLL += 10;
			}
			break;
		}
		if (FreeLibrary(hDLL0) == 0) { break; }
		else nDLL += 1;
		break;
	}
	if (nDLL >= 10){
		for (int i = 0; i < 48; i++)
		{
			nOffsetIn[i][0] = nXXYY[i][4] + nXXYY[i][5] * n1;
			nOffsetIn[i][1] = nXXYY[i][6] + nXXYY[i][7] * n1;
			int xi = i % 6;
			int yi = i / 6;
			nOffsetOut[i][0] = xi * (MY_SAMLL_IMG_X * 2) + yi * MY_SAMLL_IMG_Y * MY_OUT_IMG_X;  //// 240 * 6, 60 * 8
			nOffsetOut[i][1] = nOffsetOut[i][0] + MY_SAMLL_IMG_X; // 120
		}

		if (sFrames.nSize_pImgCut != MY_OUT_IMG_X * MY_OUT_IMG_Y && sFrames.nSize_pLine != MY_SAMLL_IMG_X * 96){
			sFrames.myInit(MY_OUT_IMG_X * MY_OUT_IMG_Y, MY_SAMLL_IMG_X * 96, true);
		}
		if (nDLL > 10){
			if (myRunningThread() == 0) // if not start
			{
				ttData.dd.pIn = (double *)((void *)this);
				ttData.dd.pOut = NULL;
				tt->SetUserCancel(0);
				int nThreadCount = tt->mySetN(MY_THREAD_NUMBER, MY_THREAD_NUMBER); // using internal memory for parallel transfer, same value means the internal mem is 0-based ID
				tt->Start(0, myMT_Fit_EX, &ttData, tt->pIntBuf, nThreadCount);
				bLoad_OK = true;
			}
		}
	}
	else bLoad_OK = false;
#endif // MY_USING_DLL
}

bool __fastcall myEHM_FIT::myDllTestLoad()
{
#ifdef MY_USING_FUNCTION
	return true;
#else
	int nDLL = 0;
	while (1){
		HMODULE hDLL0 = myLoadLibrary();
		if (hDLL0 == NULL) { break; }
		while (1){
			DLL_myTest myTest = (DLL_myTest)GetProcAddress(hDLL0, "myTest");
			if (myTest == NULL){ break; }
			else{
				int i = myTest(NULL, 3, 14, 0, NULL);
				if (i != 17)break;
			}
			_mySTEP1_48e mySTEP1_48e = (_mySTEP1_48e)GetProcAddress(hDLL0, "mySTEP1_48e");
			if (mySTEP1_48e == NULL){ break; }
			else{
				nDLL++;
			}
			break;
		}
		if (FreeLibrary(hDLL0) == 0){ break; }
		else nDLL += 1;
		break;
	}
	return (nDLL == 2);
#endif // MY_USING_DLL
}

bool __fastcall myEHM_FIT::isINIT_OK(void)
{
	return bLoad_OK;
}

bool __fastcall myOBJXs::HasNew(void)
{
	bool bHas;
	mxUpdate.lock();
	bHas = (llFrame > llFrameLast);
	mxUpdate.unlock();
	return bHas;
}

// after calling, sOBJ_Last is the new one, if find, otherwise NULL
myOBJ48_Tag * __fastcall myOBJXs::myFindFirstNew(void)
{
	mxUpdate.lock();
	myOBJ48_Tag * sOBJT = sOBJ_Last;
	while (1){
		long long llFrameLastNew = llFrameLast + 1;
		if (sOBJT != NULL){
			if (llFrameLastNew == sOBJT->ImgID) break;
		}

		sOBJT = NULL;
		myOBJX * pp;
		if ((pp = LastUpdate) != NULL){
			while (pp != NULL){
				if (pp->sOBJ != NULL){
					if (llFrameLastNew == pp->sOBJ->ImgID){
						sOBJT = pp->sOBJ; LastUpdate = pp;
						break;
					}
				}
				pp = dynamic_cast<myOBJX *>((myOBJX *)(pp->pNext));
			}
		}
		else{
			pp = dynamic_cast<myOBJX *>((myOBJX *)pEnd);
			while (pp != NULL){
				if (pp->sOBJ != NULL){
					if (llFrameLastNew == pp->sOBJ->ImgID){
						sOBJT = pp->sOBJ; LastUpdate = pp;
						break;
					}
				}
				pp = dynamic_cast<myOBJX *>((myOBJX *)(pp->pPre));
			}
		}
		break;
	}
	sOBJ_Last = sOBJT;
	mxUpdate.unlock();
	return sOBJT;
}
// find first not read one
myOBJ48_Tag * __fastcall myOBJXs::myFindID(const int nID, const bool bNoRead)
{	// bNoRead = true

	myOBJX * pp;
	mxUpdate.lock();
	int nIDL = (nID <= 0) ? nLastSave : nID;
	// try to find
	if ((pp = LastUpdate) == NULL)
		pp = dynamic_cast<myOBJX *>((myOBJX *)pEnd);

	int nDir = 0;
	while (pp != NULL){
		if ((int)(pp->sOBJ->ImgID) == nIDL){
			if (pp->sOBJ->nState < 0 && bNoRead){  // == -48
				nIDL++;
				pp = dynamic_cast<myOBJX *>((myOBJX *)(pp->pNext)); nDir = 1; continue;
			}
			//return  pp->sOBJ;
			// find, copy data
			//dd = pp->sOBJ->dResult;
			break;
		}
		else if ((int)(pp->sOBJ->ImgID) > nIDL){
			pp = dynamic_cast<myOBJX *>((myOBJX *)(pp->pPre)); if (nDir == 1) break; nDir = -1;
		}
		else {
			pp = dynamic_cast<myOBJX *>((myOBJX *)(pp->pNext)); if (nDir == -1) break; nDir = 1;
		}
	}
	mxUpdate.unlock();
	if (pp == NULL) return NULL;
	if (nID < 0)
		nLastSave = nIDL;
	return  pp->sOBJ;
}
// delete all frames that are earlier than the maxID
int __fastcall myOBJXs::myKeepFrame(const int n){
	myOBJX * pp;
	if (FirstFrame == NULL){
		FirstFrame = pp = (myOBJX *)myGetItemByIndex(0);
	}
	else{
		pp = FirstFrame;
	}
	while (nFrameTotalCount > n && pp != NULL){
		myOBJ48_Tag *p48W = (myOBJ48_Tag *)pp->sOBJ;
		if (p48W != NULL){
			if (p48W->pImgCut != NULL){
				delete[] p48W->pImgCut; p48W->pImgCut = NULL; p48W->nMemUse |= MY_DEL_IMG;
				nFrameTotalCount--;
			}
		}
		pp = (myOBJX *)pp->pNext;
	}
	FirstFrame = pp; // if pp = NULL, next round, it will also start from 0 and scan all
	return nFrameTotalCount;
}

int __fastcall myEHM_FIT::myANA_48(const unsigned char *pImg, double *dOut, bool bAdd, myOBJ48_Tag **sFrameOut, const int nMASK)
{	// bAdd = true, sFrameOut = NULL, nMASK = 0
	int nID = 0;
	if (bLoad_OK){
		myOBJX* pItem = sFrames.NewItemX(0, pImg, nMASK);

		mxUpdate.lock();
		if (sFrames.LastUpdate == NULL){
			sFrames.LastUpdate = pItem;
			sFrames.sOBJ_Last = pItem->sOBJ;
		}
		sFrames.LastAdd = pItem;
		if (sFrames.nCount == nMaxDatCount)
		{
			sFrames.ReMoveFirst(); // this has nFrameTotalCount-- if indeed remove a non-NULL point
		}
		sFrames.AddToList(*pItem); sFrames.nFrameTotalCount++;
		if (sFrames.nFrameTotalCount > nMaxImgCount){
			sFrames.myKeepFrame(nMaxImgCount);
		}
		sFrames.llFrame++;
		nID = (int)(sFrames.llFrame);
		mxUpdate.unlock();

		if (dOut != NULL){
			myGetANA(nID, dOut, sFrameOut);
		}
	}
	if (bAdd){

	}
	return nID;
}
// return nID when data is good, return 0 when data is too old and out of mem or not added.
// function will block, till data is avalible, if nID is new added
// if nID = -1, return >0, data of earest ID that is not read.
// dOut is 48 distances
int __fastcall myEHM_FIT::myGetANA(const int nID, double *dOut, myOBJ48_Tag **sFrameOut)
{	//sFrameOut = NULL
	int nIDL;
	mxUpdate.lock();
	nIDL = (int)(sFrames.llFrame);
	mxUpdate.unlock();
	if (nID > nIDL) // not exist
		return 0;

	myOBJ48_Tag * sFrame = sFrames.myFindID(nID, nID < 0);
	if (sFrame == NULL) return 0;

	nIDL = (int)(sFrame->ImgID);

	while (sFrame->nState == 0)
	{	// if data is not OK, we wait
		Sleep(1);
	}

	myThreadFit.lock();
	if (sFrame->nState > 0)
		sFrame->nState = -(sFrame->nState);
	myThreadFit.unlock();
	myMemcpy_tt((char *)dOut, (const char *)(sFrame->dResult), 48 * sizeof(double));
	if (sFrameOut != NULL)
		*sFrameOut = sFrame;
	return nIDL;
}

// pdRawXY is double *[4]
void __fastcall myEHM_FIT::myGetRaw(myOBJ48_Tag *sFrame, double *pdRawXY, bool bDel)
{	// bDel = true
	double *pDXY = NULL;
#ifdef	MY_USING_INTERNAL_ALLOC
	pDXY = pANAIn;
#else
	if (sFrame!=NULL)
		pDXY = sFrame->pANA;
#endif
	if (pDXY != NULL)
	{
		int nEx = 2;
		int nFit2 = 48 * (10 + nEx);
		double *pdRawXY1 = pdRawXY;
		for (int i = 0; i < 48; i++)
		{	//pOutDD[i] = myDD(nXXYY[i][4] + dd1, nXXYY[i][5] + nXXYY[i][2], nXXYY[i][6] + pFitAEOut1[11], nXXYY[i][7] + nXXYY[i][3]);
			double *pFitAEOut1 = pDXY + i * (10 + nEx);
			double dd1 = pFitAEOut1[11];
			pdRawXY1[0] = dd1 + nXXYY[i][4];
			pdRawXY1[1] = nXXYY[i][5] + nXXYY[i][2] + 1;
			pFitAEOut1 += nFit2;
			pdRawXY1[2] = nXXYY[i][6] + pFitAEOut1[11];
			pdRawXY1[3] = nXXYY[i][7] + nXXYY[i][3] + 1;
			pdRawXY1 += 4;
		}
	}
#ifdef	MY_USING_INTERNAL_ALLOC
	
#else
	if (bDel){
		if (sFrame != NULL){
			if (sFrame->pANA != NULL){
				delete[] sFrame->pANA; sFrame->pANA = NULL; // set to NULL is need, we will not release again in ~()
			}
		}
	}
#endif
	return;
}

// check if ANA for nID is ready, return is count that is calculated, best will be 48, if read, -48
// if ( myIsANAOK(nID) == 48 ) read;
int __fastcall  myEHM_FIT::myIsANAOK(const int nID){

	int nIDL;
	mxUpdate.lock();
	nIDL = (int)(sFrames.llFrame);
	mxUpdate.unlock();
	if (nID > nIDL) // not exist
		return 0;

	myOBJ48_Tag * sFrame = sFrames.myFindID(nID, nID < 0);
	if (sFrame == NULL) return 0;

	if (nID != (int)(sFrame->ImgID)) return 0;

	return sFrame->nState;

}

// by default, this class will hold last 1000 images in terms of the cut small parts
void __fastcall  myEHM_FIT::mySetSavingCount(const int nCount, const int nCount2)
{	// const int nCount = MY_MAX_MEM_FRAME, const int nCount2 = MY_MAX_MEM_DATA
	nMaxImgCount = nCount; nMaxDatCount = nCount2;
}
int __fastcall myEHM_FIT::myGetSavingCount(void)
{
	return nMaxImgCount;
}
int __fastcall myEHM_FIT::myGetSavingCount2(void)
{
	return nMaxDatCount;
}
#include "my_ssAVI.h"
void __fastcall myEHM_FIT::mySaveToAVI(const char *fn)
{
	ssAVI myAVI(fn, nFrameSize[0], nFrameSize[1], mmioFOURCC('M', 'P', 'G', '4'), 50); //mmioFOURCC('M', 'P', 'G', '4') //mmioFOURCC('M', 'R', 'L', 'E') //mmioFOURCC('D', 'I', 'B', ' ')
	int n = sFrames.nCount;
	myOBJX* pItem = (myOBJX*)sFrames.myGetItemByIndex(0);
	int i = 0;
	while (pItem != NULL && i < n){
		myOBJ48_Tag *p48W = (myOBJ48_Tag *)pItem->sOBJ;
		if (p48W->pImgCut != NULL)
			myAVI.AppendNewFrame(p48W->pImgCut);
		pItem = (myOBJX *)pItem->pNext; i++;
	}

	unsigned nXXYY_Save[48][4];
	for (int i = 0; i < 48; i++){
		for (int j = 0; j < 4; j++)
			nXXYY_Save[i][j] = nXXYY[i][j + 4];
	}
	// add data
	myAVI.myAddInfo("EHMW", (char *)nXXYY_Save, 48 * 4 * sizeof(int));
}

int __fastcall myEHM_FIT::myGetInfo(const char *fn, unsigned int(*pXXYY)[4], const int nCount)
{ // nCount = 48
	int n = myReadInfo(fn, "EHMW", (char *)pXXYY, nCount * 2 * 2 * sizeof(int));
	return (n / 16);
}
myMEMF	*mFF = NULL;

bool __fastcall myEHM_FIT::mySetMEMFile(const int n1, const int n2, const int nBlockCount)
{	//nBlockSize = MY_MAX_MEM_FRAME
	
	int nBlockSize = n1*n2;
	if (mFF != NULL){
		if (mFF->ssHead.nBlockCount == nBlockCount && mFF->ssHead.nBlockSize == nBlockSize)
			return true;
		myCloseMEMFile();
	}
	mFF = new myMEMF(nBlockCount, nBlockSize);
	mFF->ssHead.nRev2[0] = n1;	mFF->ssHead.nRev2[1] = n2;
	return (mFF != NULL);
}
bool __fastcall myEHM_FIT::myCloseMEMFile(void)
{
	if (mFF != NULL){
		delete mFF;
		mFF = NULL;
	}
	return true;
}
bool __fastcall myEHM_FIT::myAddData(const int nIndex, const char *pBuf)
{
	return mFF->mySetBlock(nIndex, pBuf);
}
void __fastcall myEHM_FIT::mySetUser(const int* p, const int nCount)
{	// nCount = 0, MY_MAX_USER_DATA
	int j = (nCount < MY_MAX_USER_DATA) ? nCount : MY_MAX_USER_DATA;
	for (int i = 0; i < j; i++)
	{
		mFF->ssHead.nRev2[i + 2] = p[i];
	}
}
//---------------------------------------------------------------------------
