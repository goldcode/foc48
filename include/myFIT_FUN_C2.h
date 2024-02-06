//---------------------------------------------------------------------------
#ifndef myFIT_FUN_C_h
#define myFIT_FUN_C_h
//---------------------------------------------------------------------------
// This version Changed MyGaussJ
// Correct the G-J Inverse Function
// 2009.04.30 Bao Guboin
// 2009.12.11 Bao Guboin
// 2012.07.10 Correct old 2D fit, it also can be used for 1D
// 2012.12.12 Adding Converlution and FFT support
// 2012.12.13 Add LM class
// 2013.10.16 Find one error on fit if Par has some value fixed, line 699
// 2014.03.03 Add functions for only inverse: MyGaussJ_inv,  myGaussJ_ex_inv
//---------------------------------------------------------------------------
//#define MY_NO_FFT 1
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#ifndef MY_NO_FFT
#include "myFFT_Class.h"
#include "myFFT_Base.h"
#else
#   ifndef __fastcall
//#      define __fastcall  __attribute__((regparm(3)))
#   endif
#endif

//  #include <windows.h>
//---------------------------------------------------------------------------
#define MY_MAX              27
#define MY_MAX_X            27
#define MY_MAX_LAMDA        1e17
#define MY_MAX_TIMES_A      100
#define MY_MAX_TIMES_B      17
#define myABS(_a)         	((_a)>=(0)?(_a):(-(_a)))

extern double dM_LOG2E;//  = 1.4426950408889634073599246810019;
extern double dM_LN2;//    = 0.69314718055994530941723212145818;
extern double dONE;//      = 1;
extern double dPI;//       = 3.1415926535897932384626433832795;

extern double   *gl_pX1, *gl_pX2;       // the x array; or x1, x2, if x2 is used
extern int      gl_nIndex_i, gl_nCount; // the running index and total count
//---------------------------------------------------------------------------
bool DimensionCheck(int n);
//---------------------------------------------------------------------------
void MyGaussJ(double **a, int n, double **b, int m);
//Linear equation solution by Gauss-Jordan elimination.
//a[0..n-1][0..n-1] is the input matrix. b[0..n-1][0..m-1] is the m right-hand side vectors.
//On output, a is replaced by its matrix inverse, and b is replaced by the corresponding set
//---------------------------------------------------------------------------
int MyGaussJ_inv(double **a, int n);
// same as MyGaussJ, just remove right calculation part
//---------------------------------------------------------------------------
void MyGaussJ1(double **a, int n, double **b, int m);
//Help function: Linear equation solution call gate for fortran to c
// or to any Matrix that are not leading with 0 but 1 in the index
//---------------------------------------------------------------------------
extern void (*UserDef_funcs)(double x, double *para, double *y, double *dyda, int na);
// This is call gate for user define function
// x: incoming data point for x
//para[0] ~ para[na-1]: fit par
// y: outgoing data from the function
//dyda[0] ~ dyda[na-1]: outgoing data, the first partical derivative of each fit par
//---------------------------------------------------------------------------
extern void (*UserDef_Rage)(double *para, int na, int *ia, int nfit);
// This is user define range check function
//---------------------------------------------------------------------------
void MyCovsrt(double **covar, int ma, int *ia, int mfit);
//When end, we need to reshape the error matrix, if mfit != ma
//---------------------------------------------------------------------------
double MyMrqcof2(double *x, double *y, double *sig, int ndata, double *a, int *ia,
        int ma, double **alpha, double *beta );
// This is old version which used for 1D, we leave this for checking;
// help function to calculate the Chi square and H' matrix
// For Newton way, one need to calculate again the 2ed derivative to get a full Hession matrix
//---------------------------------------------------------------------------
double MyMrqcof(double *x, double *y, double *sig, int ndata, double *a, int *ia,
        int ma, double **alpha, double *beta );
// help function to calculate the Chi square and H' matrix
// For Newton way, one need to calculate again the 2ed derivative to get a full Hession matrix
//---------------------------------------------------------------------------
void MyMrqmin_1(double *x, double *y, double *sig, int ndata, double *a, int *ia,
        int ma, double **covar, double **alpha, double *chisq, double *alamda);
//---------------------------------------------------------------------------
int MyMrqmin(double *x, double *y, double *sig, int ndata, double *a, int *ia,
        int ma, double **covar, double **alpha, double *pdChiq, double *pdAlamdaq );
//---------------------------------------------------------------------------
class MyLMFIT;
typedef void (*UserDef_Fit)  (double, double*, double *, double *, MyLMFIT *);
typedef void (*UserDef_Range)(double *,  MyLMFIT *);
//---------------------------------------------------------------------------
void myFree(void *p1, void *p2);
//---------------------------------------------------------------------------
void myFree(void *p1);
//---------------------------------------------------------------------------
#define MY_ERROR_XX_MAX 0x00000001
#define MY_ERROR_PAR_01 0x00000002          //if(pdAlpha==NULL || pdCovar==NULL)
#define MY_ERROR_PAR_02 0x00000004          //if(ppAlpha==NULL || ppCovar==NULL||pPara==NULL || nPara==NULL){
#define MY_ERROR_PAR_03 0x00000008          //if(pdWeight == NULL)
#define MY_ERROR_WEIGHT 0x00000010          // Input Weight is wrong
#define MY_ERROR_MEM_01 0x00000100
#define MY_ERROR_pUSER  0x00001000
//---------------------------------------------------------------------------
#define MY_MEM_MAX_DOUBLE   4
#define MY_MEM_MAX_INT      4
#define MY_MEM_MAX_DOUBLE_P 4
extern int gl_TEST; // =0
//---------------------------------------------------------------------------
class myLMFIT_MEM{
public:
    explicit myLMFIT_MEM(int n, MyLMFIT *p){
        pD_ss = NULL; ppD_ss = NULL; pInt_ss = NULL; nSave = 0; pTagSave = p;
        myGetMem(n);mySetWorkingMem();
    }
    ~myLMFIT_MEM(){ myFreeMem(); }
    virtual void __fastcall mySetError(int n){nError |= n;}
    virtual void __fastcall myFreeMem(void){ myFree(pInt_ss);myFree(ppD_ss);myFree(pD_ss);nSave = 0; }
    virtual void __fastcall mySetWorkingMem(void);
    virtual bool __fastcall myGetMem(int n){
        if(n > nSave){
            myFreeMem();
            pD_ss   = (double *) malloc(sizeof(double) * n * MY_MEM_MAX_DOUBLE);
            ppD_ss  = (double **)malloc(sizeof(double*)* n * MY_MEM_MAX_DOUBLE_P);
            pInt_ss = (int *)    malloc(sizeof(int)    * n * MY_MEM_MAX_INT);
            nSave = n;
            if(pD_ss == NULL||ppD_ss == NULL||pInt_ss == NULL){
                mySetError(MY_ERROR_MEM_01);
                myFreeMem();
            }
            if(n<=nSave){
                int i;
                for(i = 0; i<MY_MEM_MAX_DOUBLE;   i++)pD[i]   = pD_ss   + i * n;
                for(i = 0; i<MY_MEM_MAX_DOUBLE_P; i++)ppD[i]  = ppD_ss  + i * n;
                for(i = 0; i<MY_MEM_MAX_INT;      i++)pInt[i] = pInt_ss + i * n;
            }
        }
        return true;
    }
    double  *pD[4],    *pD_ss;
    double **ppD[4],  **ppD_ss;
    int     *pInt[4],  *pInt_ss;
    int      nSave,     nError;
    MyLMFIT *pTagSave;
    // double fAtry[MY_MAX], fBeta[MY_MAX], da[MY_MAX],  double fdyda[MY_MAX];
    //    double *fpL[MY_MAX],*fpR[MY_MAX],  *pOneda[MY_MAX];
    //    int nIndex[MY_MAX],nIndexT[MY_MAX];
    //    int nIndeC[MY_MAX],nIndeR[MY_MAX];
};
//---------------------------------------------------------------------------
class MyLMFIT{
public:
    explicit MyLMFIT(const int nPar_Count = MY_MAX, UserDef_Fit pFit=NULL, UserDef_Range pRange=NULL){
        pdIn_XX     = pdIn_XX_ss; ppCovar=ppCovar_ss, ppAlpha = ppAlpha_ss; pPara = pPara_ss; nPara = nPara_ss;
        mySetWorkDefault();
        
        pdCovar     = pdAlpha   = NULL; pWorkingMem = NULL;   pdWeight = NULL; bDelpdWeight = false;
        nCountXXSave= nCountPar = MY_MAX;
        nCountXX    = nError    = nCountParSave = nCountWeightSave = 0; // to start the allocate immediately
        mySetParaCount(nPar_Count); mySetFitFun(pFit, pRange);
    }
    ~MyLMFIT(){
        myFreeMem();
        if(pWorkingMem != NULL)delete pWorkingMem;
        if(bDelpdWeight)myFree(pdWeight);
    }
    virtual void __fastcall mySetWork2Mem(double  **pD, double ***ppD, int **pInt){
        pAtry=pD[0];
        pBeta=pD[1];
        pValdA=pD[2];
        pDyda=pD[3];
        
        ppValpL=ppD[0];
        ppValpR=ppD[1];
        ppOnedA=ppD[2];
        
        pnIndex=pInt[0];
        pnIndexT=pInt[1];
        pnIndeC=pInt[2];
        pnIndeR=pInt[3];
    }
    virtual void __fastcall mySetWorkDefault(void){
        pAtry=pAtry_ss;
        pBeta=pBeta_ss;
        pValdA=pValdA_ss;
        pDyda=pDyda_ss;
        
        ppValpL=ppValpL_ss;
        ppValpR=ppValpR_ss;
        ppOnedA=ppOnedA_ss;
        
        pnIndex=pnIndex_ss;
        pnIndexT=pnIndexT_ss;
        pnIndeC=pnIndeC_ss;
        pnIndeR=pnIndeR_ss;
    }
    virtual void __fastcall mySetError(int n){nError |= n;}
    
    virtual void __fastcall mySetData(double *pX, double *pY, const int nCount, double *pWeight=NULL){
        pdIn_X=pX; pdIn_Y=pY; nCountDat = nCount; if(pWeight!=NULL){ mySetDataWeight(pWeight); nCountWeightSave = nCountDat;}
    }
    //---------------------------------------------------------------------------
    // Following is for weight fact
    // To increase the speed, please use weight[i] = 1.0/(sig[i]*sig[i]);  sig is the individal standard deviations
    // for Chi square calcuation
    //---------------------------------------------------------------------------
    virtual void __fastcall myAllocteWeight(const int n){
        if(bDelpdWeight){ if(n!= nCountWeightSave){ myFreeMen_WT(); myAllocteWeight_ex(n); }}
        else            { myAllocteWeight_ex(n);  }
    }
    virtual void __fastcall mySetDataWeight(double *pWeight=NULL, int nCount = -1, bool bCopy = false){
        if(bCopy){
            if(pWeight!=NULL&&nCount>0){
                myAllocteWeight(nCount);  // if error, nCountWeightSave = 0;
                if(nCountWeightSave == nCount){
                    for(int i=0;i<nCount;i++)pdWeight[i] = pWeight[i];
                }
            }else{
                mySetError(MY_ERROR_WEIGHT);
            }
        }else{
            if(pWeight!=pdWeight){
                myFreeMen_WT(); // same as  if(bDelpdWeight)myFree(pdWeight);
                pdWeight = pWeight; bDelpdWeight = false;
            }
            if(pWeight==NULL)nCountWeightSave = 0;
            else{
                if(nCount == -1)nCountWeightSave = nCountDat;
                else            nCountWeightSave = nCount;
            }
        }
    }
    virtual void __fastcall mySetWeightToOne(void){
        if(nCountDat != nCountWeightSave){ myAllocteWeight(nCountDat); }
        if(nCountDat <= nCountWeightSave){ for(int i=0;i<nCountDat;i++)pdWeight[i] = 1; }
    }
    //---------------------------------------------------------------------------
    virtual void __fastcall mySetFitFun(UserDef_Fit pFit, UserDef_Range pRange){UserFun_Fit = pFit; UserFun_Range = pRange;}
    virtual int  __fastcall mySetParaCount(const int n){
        if(n>nCountParSave){ // if larger than already saved
            //free already saved, re-allocate
            //myFreeMem_Par();
            myAllocte(n); // nParCountSave
        }
        if(n<=nCountParSave) nCountPar = n;
        else                 nCountPar = nCountParSave;
        return nCountPar;
    }
    virtual int __fastcall mySet_XX_Count(const int n){
        if(n > nCountXXSave){
            myFreeMem_XX(); nCountXXSave = MY_MAX_X;
            if(n > nCountXXSave){
                pdIn_XX = (double **) malloc( n * sizeof(double *));
                if(pdIn_XX == NULL){ pdIn_XX = pdIn_XX_ss;
                nCountXXSave = nCountXX = MY_MAX_X;
                mySetError(MY_ERROR_XX_MAX);
                }else{
                    nCountXXSave = nCountXX = n;
                }
            }else{
                nCountXX = n;
            }
        }else{
            nCountXX = n;
        }
        return nCountXX;
    }
    virtual void __fastcall myFreeMem    (void){ myFreeMem_Par(); myFreeMem_XX(); }
    virtual void __fastcall myFreeMem_N1(void){ myFree(pdCovar); myFree(pdAlpha); nCountParSave = 0; }
    virtual void __fastcall myFreeMem_Par(void){ myFree(ppAlpha, ppAlpha_ss); myFree(ppCovar, ppCovar_ss);
    myFree(pPara,   pPara_ss  ); myFree(nPara,   nPara_ss  );           }
    virtual void __fastcall myFreeMem_XX(void){ myFree(pdIn_XX, pdIn_XX_ss); }
    virtual void __fastcall myFreeMen_WT(void){ if(bDelpdWeight)myFree(pdWeight); nCountWeightSave = 0;}
    virtual int  __fastcall myAllocte(int n){
        int i;
        if(n != nCountParSave){
            if(nCountParSave > 0){
                myFreeMem_N1(); myFreeMem_Par();
            }
            i = n*n*sizeof(double);
            pdAlpha = (double *) malloc(i); pdCovar = (double *) malloc(i);
            if(n > MY_MAX){
                ppAlpha = (double **) malloc( n * sizeof(double *));
                ppCovar = (double **) malloc( n * sizeof(double *));
                pPara   = (double *)  malloc( n * sizeof(double));
                nPara   = (int *)     malloc( n * sizeof(int));
                if(pWorkingMem==NULL){
                    pWorkingMem = new myLMFIT_MEM(n, this);
                }else{
                    pWorkingMem->myGetMem(n); pWorkingMem->mySetWorkingMem();
                }
                if((i = pWorkingMem->nError)!=0) mySetError(i);
            }else{
                if(pnIndeR!=pnIndeR_ss){
                    if(pWorkingMem != NULL){ pWorkingMem->myFreeMem(); }
                    mySetWorkDefault();
                }
            }
            nCountParSave = n;
            if(pdAlpha==NULL || pdCovar==NULL){
                myFreeMem_N1();  nCountParSave = 0; mySetError(MY_ERROR_PAR_01);
            }
            if(ppAlpha==NULL || ppCovar==NULL||pPara==NULL || nPara==NULL){
                myFreeMem_Par(); nCountParSave = 0; mySetError(MY_ERROR_PAR_02);
            }
        }
        if(n <= nCountParSave){
            for(i=0; i<n; i++){ ppCovar[i]=&(pdCovar[i*n]);  ppAlpha[i]=&(pdAlpha[i*n]); }
            //             for(int nj=0;nj<5;nj++) for(int ni=0;ni<5;ni++)ppCovar[nj][ni]= nj*100+ni;
            //             printf("memory alloance\n");
            //             for(int nj=0;nj<5;nj++) {for(int ni=0;ni<5;ni++)printf("%lf\t", ppCovar[nj][ni]);printf("\n");}
            //             printf("\n\n");
        }
        return nCountParSave;
    }
    virtual void __fastcall mySetPara     (const int n, double dSt, bool bFit = true){ int i=n%nCountPar; pPara[i]=dSt; nPara[i]=bFit?1:0; }
    virtual void __fastcall mySetParaAll  (const int n, double *pSt, int *pFitSwitch){
        if(pFitSwitch!=NULL&&pSt!=NULL){ mySetParaCount(n); for(int i=0;i<nCountPar;i++){ mySetPara(i, pSt[i], (pFitSwitch[i]!=0)); }}
    }
    virtual void __fastcall mySetParaAll  (const int n, double *pSt, double *pFitSwitch){
        if(pFitSwitch!=NULL&&pSt!=NULL){ mySetParaCount(n); for(int i=0;i<nCountPar;i++){ mySetPara(i, pSt[i], (pFitSwitch[i]!=0)); }}
    }
    
    UserDef_Fit         UserFun_Fit;       // void (*UserFun_Fit)  (double x, double *para, double *y, double *dyda, MyLMFIT *pThis);
    //---------------------------------------------------------------------------
    // This is call gate for user define a function
    // x: incoming data point for x
    // para[0] ~ para[na-1]: fit par,      // na = pThis->nCountPar;
    // y: outgoing data from the function
    // dyda[0] ~ dyda[na-1]: outgoing data, the first partical derivative of each fit par
    //---------------------------------------------------------------------------
    UserDef_Range       UserFun_Range;     // void (*UserFun_Range)(double *para, MyLMFIT *pThis);
    //---------------------------------------------------------------------------
    // This is call gate for user define function
    // para[0] ~ para[na-1]: fit par       // na = pThis->nCountPar;
    // pThis->nPara     is the fitting switch
    // pThis->nCountFit is the real fit para number
    //---------------------------------------------------------------------------
    int                 nCurIndex;    // current running index
    //---------------------------------------------------------------------------
    virtual void __fastcall myCovReshape(double **ppCov, int nCount, int *nFitSwitch, int nFitCount){
        //When end, we need to reshape the error matrix, if mfit != ma
        int i, j, k;
        double fTemp;
        for (i=nFitCount;i<nCount;i++)
            for (j=0;j<i;j++) ppCov[i][j]=ppCov[j][i]=0.0;
        k=nFitCount-1;
        for (j=nCount-1;j>=0;j--) {
            if (nFitSwitch[j]) {
                for (i=0;i<nCount;i++){
                    fTemp=ppCov[i][k]; ppCov[i][k]= ppCov[i][j]; ppCov[i][j]=fTemp;
                }
                for (i=0;i<nCount;i++){
                    fTemp=ppCov[k][i]; ppCov[k][i]= ppCov[j][i]; ppCov[j][i]=fTemp;
                }
                k--;
            }
        }
    }
    //---------------------------------------------------------------------------
    virtual void __fastcall myGaussJ_ex(double **a, int n, double **b, int m){
        //Linear equation solution by Gauss-Jordan elimination.
        //a[0..n-1][0..n-1] is the input matrix. b[0..n-1][0..m-1] is the m right-hand side vectors.
        //On output, a is replaced by its matrix inverse, and b is replaced by the corresponding set
        
        //double *fpL[MY_MAX],*fpR[MY_MAX]; int nIndex[MY_MAX],nIndexT[MY_MAX]; int nIndeC[MY_MAX],nIndeR[MY_MAX];
        //    ppValpL      ppValpR             pnIndex        pnIndexT             pnIndeC        pnIndeR
        
        int i, j, k, l, nRow, nCol, nTemp;
        double dBig, dTemp;
        //if (gl_TEST++<2)
        //   for (j=0;j<n;j++){
        //	 for (k=0;k<n;k++) {
        //		printf("%f ", a[j][k]);//printf("a[j=%d][k=%d]=%f ", j,k,a[j][k]);
        //   }
        //    printf("  b=%f \n",b[j][0]);
        //}
        
        for(i=0;i<n;i++){
            ppValpL[i]=a[i]; ppValpR[i]=b[i];
            pnIndex[i]=i; pnIndexT[i]=i;
        }
        for(i=0;i<n;i++){
            //dBig=1e5;//
            dBig=myABS(ppValpL[i][pnIndex[i]]);
            //printf("Here\n");
            pnIndeR[i]=pnIndexT[nRow=i]; pnIndeC[i]=pnIndex[nCol=i];  //dBig=0; nRow = nCol = i;
            for (j=i;j<n;j++)
                for (k=i;k<n;k++)
                    if ((dTemp=myABS(ppValpL[j][pnIndex[k]])) > dBig){
                        dBig = dTemp;
                        pnIndeR[i]=pnIndexT[nRow=j]; pnIndeC[i]=pnIndex[nCol=k];
                    }
            if (nRow != i){
                double *pdTemp;
                pdTemp=ppValpL[nRow]; ppValpL[nRow]=ppValpL[i]; ppValpL[i]= pdTemp;
                pdTemp=ppValpR[nRow]; ppValpR[nRow]=ppValpR[i]; ppValpR[i]= pdTemp;
                nTemp = pnIndexT[nRow]; pnIndexT[nRow] = pnIndexT[i]; pnIndexT[i]= nTemp;
            }
            if (nCol != i){
                nTemp = pnIndex[nCol]; pnIndex[nCol] = pnIndex[i]; pnIndex[i]= nTemp;
            }
            if ( (dTemp = ppValpL[i][pnIndex[i]]) == 0.0){
                ////MessageBox(NULL, "All 0 in array!", "All 0 in Array!",MB_OK | MB_ICONWARNING );
                //printf("myGaussJ_ex: Big one = 0 \n");
                //for (j=0;j<n;j++){
                //   for (k=0;k<n;k++) {
                //      printf("%f ", a[j][k]);//printf("a[j=%d][k=%d]=%f ", j,k,a[j][k]);
                //   }
                //   printf("  b=%f \n",b[j][0]);
                //}
                //i=n; // skip,
                continue; //return;
            }
            dTemp=1.0/dTemp;  ppValpL[i][nTemp = pnIndex[i]]=1;
            for (j=0;j<n;j++) ppValpL[i][pnIndex[j]] *= dTemp;
            for (j=0;j<m;j++) ppValpR[i][j] *= dTemp;
            for (j=0;j<n;j++){
                if(j==i)continue;
                dTemp = ppValpL[j][nTemp]; ppValpL[j][nTemp]=0.0;
                for (k=0;k<n;k++){
                    l=pnIndex[k];
                    ppValpL[j][l] -= ppValpL[i][l] * dTemp;
                }
                for (k=0;k<m;k++)
                    ppValpR[j][k] -= ppValpR[i][k] * dTemp;
            }
        }
        for(i=0;i<n;i++){
            pnIndex[i]=i; pnIndexT[i]=i;
        }
        // Until Here, we have all the data, if we donot need an inv, we can quit at this point
        // Now, we reshape the matrix to fit the real INV of the input
        for (i=0;i<n;i++){
            if((k = pnIndex[pnIndeR[i]])!= (l = pnIndeC[i])){
                nTemp = pnIndexT[l];  pnIndexT[l]=pnIndexT[k];  pnIndexT[k]=nTemp;
                pnIndex[pnIndexT[l]]=l;
                pnIndex[pnIndexT[k]]=k;
                for (j=0;j<n;j++){
                    dTemp=a[k][j]; a[k][j]=a[l][j]; a[l][j]= dTemp;
                }
                for (j=0;j<m;j++){
                    dTemp=b[k][j]; b[k][j]=b[l][j]; b[l][j]= dTemp;
                }
            }
            pnIndeC[i]=l;
            pnIndeR[i]=k;
        }
        for (i=n-1;i>=0;i--){
            if((k = pnIndeR[i])!= (l = pnIndeC[i])){
                for (j=0;j<n;j++){
                    dTemp=a[j][k]; a[j][k]=a[j][l]; a[j][l]= dTemp;
                }
            }
        }
        //for (j=0;j<n;j++){
        //	for (k=0;k<n;k++) {
        //		printf("%f ", a[j][k]);//printf("a[j=%d][k=%d]=%f ", j,k,a[j][k]);
        //          }
        //          printf("  b=%f \n",b[j][0]);
        //      }
        
    }
    virtual int __fastcall myGaussJ_ex_inv(double **a, int n){
        //same as myGaussJ_ex, just used for inverse
        
        //double *fpL[MY_MAX],*fpR[MY_MAX]; int nIndex[MY_MAX],nIndexT[MY_MAX]; int nIndeC[MY_MAX],nIndeR[MY_MAX];
        //    ppValpL      ppValpR             pnIndex        pnIndexT             pnIndeC        pnIndeR
        
        int i, j, k, l, nRow, nCol, nTemp, nRR=0;
        double dBig, dTemp;
        //if (gl_TEST++<2)
        //   for (j=0;j<n;j++){
        //	 for (k=0;k<n;k++) {
        //		printf("%f ", a[j][k]);//printf("a[j=%d][k=%d]=%f ", j,k,a[j][k]);
        //   }
        //    printf("  b=%f \n",b[j][0]);
        //}
        
        for(i=0;i<n;i++){
            ppValpL[i]=a[i]; pnIndex[i]=i; pnIndexT[i]=i;
        }
        for(i=0;i<n;i++){
            //dBig=1e5;//
            dBig=myABS(ppValpL[i][pnIndex[i]]);
            //printf("Here\n");
            pnIndeR[i]=pnIndexT[nRow=i]; pnIndeC[i]=pnIndex[nCol=i];  //dBig=0; nRow = nCol = i;
            for (j=i;j<n;j++)
                for (k=i;k<n;k++)
                    if ((dTemp=myABS(ppValpL[j][pnIndex[k]])) > dBig){
                        dBig = dTemp;
                        pnIndeR[i]=pnIndexT[nRow=j]; pnIndeC[i]=pnIndex[nCol=k];
                    }
            if (nRow != i){
                double *pdTemp;
                pdTemp=ppValpL[nRow]; ppValpL[nRow]=ppValpL[i]; ppValpL[i]= pdTemp;
                nTemp = pnIndexT[nRow]; pnIndexT[nRow] = pnIndexT[i]; pnIndexT[i]= nTemp;
            }
            if (nCol != i){
                nTemp = pnIndex[nCol]; pnIndex[nCol] = pnIndex[i]; pnIndex[i]= nTemp;
            }
            if ( (dTemp = ppValpL[i][pnIndex[i]]) == 0.0){
                ////MessageBox(NULL, "All 0 in array!", "All 0 in Array!",MB_OK | MB_ICONWARNING );
                //printf("myGaussJ_ex: Big one = 0 \n");
                //for (j=0;j<n;j++){
                //   for (k=0;k<n;k++) {
                //      printf("%f ", a[j][k]);//printf("a[j=%d][k=%d]=%f ", j,k,a[j][k]);
                //   }
                //   printf("  b=%f \n",b[j][0]);
                //}
                //i=n;    // skip
                nRR=-1; continue;
            }
            dTemp=1.0/dTemp;  ppValpL[i][nTemp = pnIndex[i]]=1;
            for (j=0;j<n;j++) ppValpL[i][pnIndex[j]] *= dTemp;
            //for (j=0;j<m;j++) ppValpR[i][j] *= dTemp;
            for (j=0;j<n;j++){
                if(j==i)continue;
                dTemp = ppValpL[j][nTemp]; ppValpL[j][nTemp]=0.0;
                for (k=0;k<n;k++){
                    l=pnIndex[k];
                    ppValpL[j][l] -= ppValpL[i][l] * dTemp;
                }
                //for (k=0;k<m;k++)
                //ppValpR[j][k] -= ppValpR[i][k] * dTemp;
            }
        }
        for(i=0;i<n;i++){
            pnIndex[i]=i; pnIndexT[i]=i;
        }
        // Until Here, we have all the data, if we donot need an inv, we can quit at this point
        // Now, we reshape the matrix to fit the real INV of the input
        for (i=0;i<n;i++){
            if((k = pnIndex[pnIndeR[i]])!= (l = pnIndeC[i])){
                nTemp = pnIndexT[l];  pnIndexT[l]=pnIndexT[k];  pnIndexT[k]=nTemp;
                pnIndex[pnIndexT[l]]=l;
                pnIndex[pnIndexT[k]]=k;
                for (j=0;j<n;j++){
                    dTemp=a[k][j]; a[k][j]=a[l][j]; a[l][j]= dTemp;
                }
                //for (j=0;j<m;j++){
                //    dTemp=b[k][j]; b[k][j]=b[l][j]; b[l][j]= dTemp;
                //}
            }
            pnIndeC[i]=l;
            pnIndeR[i]=k;
        }
        for (i=n-1;i>=0;i--){
            if((k = pnIndeR[i])!= (l = pnIndeC[i])){
                for (j=0;j<n;j++){
                    dTemp=a[j][k]; a[j][k]=a[j][l]; a[j][l]= dTemp;
                }
            }
        }
        //for (j=0;j<n;j++){
        //	for (k=0;k<n;k++) {
        //		printf("%f ", a[j][k]);//printf("a[j=%d][k=%d]=%f ", j,k,a[j][k]);
        //          }
        //          printf("  b=%f \n",b[j][0]);
        //      }
        
        return nRR;
    }
    //---------------------------------------------------------------------------
    
    virtual double __fastcall myLM_Min_exx(double *pPara_ex, double **ppAlpha_ex, double *pBeta_ex ){
        //MyMrqcof(double *x, double *y, double *sig, int ndata, double *a, int *ia,int ma, double **alpha, double *beta ){
        // help function to calculate the Chi square and H' matrix
        // For Newton way, one need to calculate again the 2ed derivative to get a full Hession matrix
        
        int i, j, k, l, m;
        double dModOut, dWT, dSig, dDelta;
        double dChiX = 0;
        
        for (j=0;j<nCountFit;j++) {                     // Initialize a symmetric alpha, beta.
            for (k=0; k<=j; k++) ppAlpha_ex[j][k]=0.0;
            pBeta_ex[j]=0.0;
        }
        for (i=0; i<nCountDat; i++) {      //loop over all data and cal Hession' .
            (*UserFun_Fit)(pdIn_X[(nCurIndex = i)], pPara_ex , &dModOut, pDyda, this); // (*UserDef_funcs)(x[(gl_nIndex_i = i)], a , &ymod, fdyda, ma );
            dSig   = pdWeight[i];
            dDelta = pdIn_Y[i] - dModOut;
            for (j=0, l=0;l<nCountPar;l++) { // this is the error: //for (j=0, l=0;l<nCountFit;l++) {
                if (nPara[l]) {
                    dWT = pDyda[l] * dSig;
                    for (k=0, m=0; m<=l; m++)
                        if (nPara[m])
                            ppAlpha_ex[j][k++] += dWT * pDyda[m];
                    pBeta_ex[j++] += dDelta * dWT;
                }
            }
            dChiX += dDelta*dDelta*dSig; // Cal Chi square.
        }
        for (j=1;j<nCountFit;j++)        //We suppose a symmetric Hession'
            for (k=0;k<j;k++) ppAlpha_ex[k][j]=ppAlpha_ex[j][k];
        //printf("dChiX_Cpp = %f\n", dChiX);
        return dChiX;
    }
    
    virtual void __fastcall myLM_Min_ex(){
        int j, k, l;
        //static double  fAtry[MY_MAX], fBeta[MY_MAX], da[MY_MAX], *pOneda[MY_MAX];
        double dTemp;
        if (dlamdaFun < 0.0){ // We use a 0 value to Initialize the starting point
            for (nCountFit=0, j=0;j<nCountPar;j++)if(nPara[j]) nCountFit++;
            dlamdaFun = dlamda_ex = 1;
            dChiFun = dChi_ex = myLM_Min_exx(pPara, ppAlpha, pBeta);   //MyMrqcof(x,y,sig,ndata,a,ia,ma,alpha, fBeta);
            for (j=0;j<nCountPar;j++){ pAtry[j]=pPara[j]; ppOnedA[j]=&(pValdA[j]);}
        }
        // Alter linearized fitting matrix, by augmenting diagonal elements.
        for (j=0;j<nCountFit;j++) {
            for (k=0;k<nCountFit;k++) ppCovar[j][k]=ppAlpha[j][k];
#ifndef MY_LCC
            try{
#endif
                dTemp = ((double)(ppAlpha[j][j]))*(1.0+(dlamdaFun));
#ifndef MY_LCC
            }   catch(...){
                dTemp = ppAlpha[j][j];
            }
#endif
            ppCovar[j][j]= dTemp;
            pValdA[j]=pBeta[j];
        }
        myGaussJ_ex(ppCovar, nCountFit, ppOnedA, 1);
        if (dlamdaFun == 0.0) { myCovReshape(ppCovar, nCountPar, nPara, nCountFit);  return; }// Once converged, evaluate covariance matrix.
        
        //Did the trial succeed?
        for (j=0, l=0;l<nCountPar;l++)
            if (nPara[l]) pAtry[l] = pPara[l] + pValdA[j++];
        ///////////////////////////////////////////////////////////////////////////////
        (*UserFun_Range)(pAtry, this);
        ///////////////////////////////////////////////////////////////////////////////
        dChiFun= myLM_Min_exx(pAtry, ppCovar, pValdA); //MyMrqcof(x,y,sig,ndata,fAtry,ia,ma,covar,da);
        if (dChiFun < dChi_ex) {   // Success, accept the new solution.
            dlamda_ex = dlamdaFun;
            dlamdaFun *= 0.2; dChi_ex = dChiFun;
            //dlamdaFun *= 0.1; dChi_ex = dChiFun;
            for (j=0;j<nCountFit;j++) {
                for (k=0;k<nCountFit;k++) ppAlpha[j][k] = ppCovar[j][k];
                pBeta[j] = pValdA[j];
            }
            for (l=0;l<nCountPar;l++) pPara[l]=pAtry[l];
        }else{// Failure, increase alamda and return.
            if(dlamdaFun > dlamda_ex){ // last time is here      AAA
                dlamda_ex = dlamdaFun; dlamdaFun *= 10;
            }else if(dlamdaFun < dlamda_ex){ // last time is in //Success, or in CCC,    BBB
                if( dlamdaFun * 4 > dlamda_ex){  //last time is in CCC,
                    dlamda_ex = dlamdaFun; dlamdaFun *= 5;
                }else{   // last time is in //Success,
                    dlamda_ex = dlamdaFun; dlamdaFun *= 5;
                }
            }else{ // first time or last time, it is wrong, maybe dlamdaFun is too big,  CCC
                dlamda_ex = dlamdaFun; dlamdaFun /= 2;
            }
            //dlamdaFun *= 10;
            dChiFun = dChi_ex;
        }
    }
    virtual void __fastcall myLM_Min_ex2(){
        int j, k, l;
        //static double  fAtry[MY_MAX], fBeta[MY_MAX], da[MY_MAX], *pOneda[MY_MAX];
        double dTemp;
        //bool   bDSP = false;
        if (dlamdaFun < 0.0){ // We use a 0 value to Initialize the starting point
            for (nCountFit=0, j=0;j<nCountPar;j++)if(nPara[j]) nCountFit++;
            dlamdaFun = dlamda_ex = 1; //bDSP = true;
            dChiFun = dChi_ex = myLM_Min_exx(pPara, ppAlpha, pBeta);   //MyMrqcof(x,y,sig,ndata,a,ia,ma,alpha, fBeta);
            for (j=0;j<nCountPar;j++){ pAtry[j]=pPara[j]; ppOnedA[j]=&(pValdA[j]);}
            //printf("dChi_ex=%f\n",dChi_ex); //Same
        }
        // Alter linearized fitting matrix, by augmenting diagonal elements.
//         if(bDSP){
//             for (j=0;j<nCountFit;j++) {
//                 for (k=0;k<nCountFit;k++) {
//                     printf("ppAlpha[j=%d][k=%d]=%f ", j,k,ppAlpha[j][k]);
//                 }
//                 printf("\n");
//             }
//         }
        for (j=0;j<nCountFit;j++) {
            for (k=0;k<nCountFit;k++) ppCovar[j][k]=ppAlpha[j][k];
#ifndef MY_LCC
            try{
#endif
                dTemp = ((double)(ppAlpha[j][j]))*(1.0+(dlamdaFun));
#ifndef MY_LCC
            }   catch(...){
                dTemp = ppAlpha[j][j];
            }
#endif
            ppCovar[j][j]= dTemp;
            pValdA[j]=pBeta[j];
        }
        myGaussJ_ex(ppCovar, nCountFit, ppOnedA, 1);
        if (dlamdaFun == 0.0) { myCovReshape(ppCovar, nCountPar, nPara, nCountFit);  return; }// Once converged, evaluate covariance matrix.
        
        //Did the trial succeed?
        for (j=0, l=0;l<nCountPar;l++)
            if (nPara[l]) pAtry[l] = pPara[l] + pValdA[j++];
        ///////////////////////////////////////////////////////////////////////////////
        (*UserFun_Range)(pAtry, this);
        ///////////////////////////////////////////////////////////////////////////////
        dChiFun= myLM_Min_exx(pAtry, ppCovar, pValdA); //MyMrqcof(x,y,sig,ndata,fAtry,ia,ma,covar,da);
        if (dChiFun < dChi_ex) {   // Success, accept the new solution.
            dlamda_ex = dlamdaFun;
            //dlamdaFun *= 0.2; dChi_ex = dChiFun;
            dlamdaFun *= 0.1; dChi_ex = dChiFun;
            for (j=0;j<nCountFit;j++) {
                for (k=0;k<nCountFit;k++) ppAlpha[j][k] = ppCovar[j][k];
                pBeta[j] = pValdA[j];
            }
            for (l=0;l<nCountPar;l++) pPara[l]=pAtry[l];
        }else{// Failure, increase alamda and return.
            //            if(dlamdaFun > dlamda_ex){ // last time is here      AAA
            //                dlamda_ex = dlamdaFun; dlamdaFun *= 10;
            //            }else if(dlamdaFun < dlamda_ex){ // last time is in //Success, or in CCC,    BBB
            //                if( dlamdaFun * 4 > dlamda_ex){  //last time is in CCC,
            //                    dlamda_ex = dlamdaFun; dlamdaFun *= 5;
            //                }else{   // last time is in //Success,
            //                    dlamda_ex = dlamdaFun; dlamdaFun *= 5;
            //                }
            //            }else{ // first time or last time, it is wrong, maybe dlamdaFun is too big,  CCC
            //                dlamda_ex = dlamdaFun; dlamdaFun /= 2;
            //            }
            dlamdaFun *= 10;
            dChiFun = dChi_ex;
        }
    }
    virtual int __fastcall myLM_Min(){ double dChi, dLamda; return myLM_Min(dChi, dLamda); }
    
    virtual int __fastcall myLM_Min(double &dChi, double &dLamda){
        if(pdIn_Y == NULL || pdIn_X == NULL || nCountDat <=0 ||nCountPar <=0)return -1;
        if(pdWeight == NULL)mySetWeightToOne();
        if(nCountDat > nCountWeightSave) return -1;
        // calling
        int i, j;
        dlamdaFun = -1; //gl_nCount = ndata; gl_pX1 = x;
        myLM_Min_ex();
        dChi = dChiFun; dLamda = dlamdaFun; i=j=0;
        while(dlamdaFun < MY_MAX_LAMDA && i<MY_MAX_TIMES_A && j<MY_MAX_TIMES_B){
            //printf("i=%d, j=%d, dlamdaFun=%lg\t dChiFun=%lg \n", i, j, dlamdaFun, dChiFun);
            myLM_Min_ex(); //  MyMrqmin_1(x, y, sig, ndata, a, ia, ma, covar, alpha, &dChi, &dAlamda);
            if(dLamda < dlamdaFun){ // On success, dlamda_ex will reduce 1/10 and we also monitor this
                j++;
            }else{
                j=0;
            }
            dLamda = dlamdaFun;
            i++;
        }
        //printf("End: i=%d j=%d, dlamdaFun=%lg\t dChiFun=%lg \n", i, j, dlamdaFun, dChiFun);
        dChi = dChiFun; dLamda = dlamdaFun; dlamdaFun = 0;
        myLM_Min_ex();// MyMrqmin_1(x, y, sig, ndata, a, ia, ma, covar, alpha, &dChi, &dAlamda);
        return i;
    }
    virtual int __fastcall myLM_Min2(double &dChi, double &dLamda){
        if(pdIn_Y == NULL || pdIn_X == NULL || nCountDat <=0 ||nCountPar <=0)return -1;
        if(pdWeight == NULL)mySetWeightToOne();
        if(nCountDat > nCountWeightSave) return -1;
        // calling
        int i, j;
        dlamdaFun = -1; //gl_nCount = ndata; gl_pX1 = x;
        myLM_Min_ex2();
        dChi = dChiFun; dLamda = dlamdaFun; i=j=0;
        while(dlamdaFun < MY_MAX_LAMDA && i<MY_MAX_TIMES_A && j<MY_MAX_TIMES_B){
            //printf("i=%d, j=%d, dlamdaFun=%lg\t dChiFun=%lg \n", i, j, dlamdaFun, dChiFun);
            myLM_Min_ex2(); //  MyMrqmin_1(x, y, sig, ndata, a, ia, ma, covar, alpha, &dChi, &dAlamda);
            if(dLamda < dlamdaFun){ // On success, dlamda_ex will reduce 1/10 and we also monitor this
                j++;
            }else{
                j=0;
            }
            dLamda = dlamdaFun;
            i++;
        }
        //printf("End2: i=%d j=%d, dlamdaFun=%lg\t dChiFun=%lg \n", i, j, dlamdaFun, dChiFun);
        dChi = dChiFun; dLamda = dlamdaFun; dlamdaFun = 0;
        myLM_Min_ex2();// MyMrqmin_1(x, y, sig, ndata, a, ia, ma, covar, alpha, &dChi, &dAlamda);
        return i;
    }
    virtual int __fastcall myLM_Min(double *pX, double *pY, double *pWeight, int nDataCount,
            double *Par, int *nFitSwitch, int nParCount, double &dChi, double &dLamda){
        mySetData(pX, pY, nDataCount, pWeight);   mySetParaAll(nParCount, Par, nFitSwitch);  return myLM_Min(dChi, dLamda);
    }
    
    virtual void __fastcall myGetFitResult(double *pFitA, double *pFitErr = NULL){
        int i; double j = double(nCountDat - nCountFit);
        if(pFitErr == NULL){
            for(i=0; i < nCountPar; i++){   // Save data and error matrix to matlab
                pFitA[i] = pPara[i];
            }
        }else{
            for(i=0; i < nCountPar; i++){   // Save data and error matrix to matlab
                pFitA[i] = pPara[i];
                if(nPara[i]!=0 && j>0) pFitErr[i] = sqrt( myABS(dChiFun * ppCovar[i][i]/j) );
                else                   pFitErr[i] = 0;
            }
        }
    }
    
    int          nCountDat, nCountPar, nCountFit, nCountParSave;
    double      *pdIn_Y, *pdIn_X, **pdIn_XX, *pdIn_XX_ss[MY_MAX_X];   // pdIn_XX point to user data, if more than one input X is used
    int          nCountXXSave, nCountXX;
    myLMFIT_MEM *pWorkingMem;
    int          nError,    nRev;
    double       dChiFun,    dlamdaFun;
    
    int         *nPara,    nPara_ss[MY_MAX];
    double      *pPara,    pPara_ss[MY_MAX];
    void        *pUser;
protected:
    virtual void __fastcall myAllocteWeight_ex(const int n){
        pdWeight = (double *)malloc(n*sizeof(double));
        if(pdWeight == NULL){ nCountWeightSave = 0; bDelpdWeight = false; mySetError(MY_ERROR_PAR_03); }
        else                { nCountWeightSave = n; bDelpdWeight = true; }
    }
    
    bool         bDelpdWeight;
    double      *pdWeight, *pdCovar, *pdAlpha;
    int          nCountWeightSave;
    double     **ppCovar,  *ppCovar_ss[MY_MAX], **ppAlpha, *ppAlpha_ss[MY_MAX];
    double       dChi_ex,    dlamda_ex;
    
    double      *pAtry,    pAtry_ss[MY_MAX], *pBeta,    pBeta_ss[MY_MAX], *pValdA,   pValdA_ss[MY_MAX];
    double      *pDyda,    pDyda_ss[MY_MAX];
    double     **ppValpL, *ppValpL_ss[MY_MAX], **ppValpR, *ppValpR_ss[MY_MAX], **ppOnedA, *ppOnedA_ss[MY_MAX];
    int         *pnIndex,  pnIndex_ss[MY_MAX],  *pnIndexT, pnIndexT_ss[MY_MAX];
    int         *pnIndeC,  pnIndeC_ss[MY_MAX],  *pnIndeR, pnIndeR_ss[MY_MAX];
    
};
//---------------------------------------------------------------------------
// //---------------------------------------------------------------------------
// //
// //  Following is the example for using the MyLMFIT class
// //
// //---------------------------------------------------------------------------
// //  We suppose old LM fitting functions myFitFun_Old and myRangeCheck_Old/
// //---------------------------------------------------------------------------
// void myFitFun_Old(double x, double *para, double *y, double *dyda, int na){
//     // Suppose an old function for LM fit.
//     // eg. In old EXP fitting function
//     
//     //---------------------------------------------------------------------------
//     //  *User Define Function
//     //      Y = A1 * exp(- t / T1)+ A2 * exp(- t / T2) + A3;
//     //      dy/dA1 = exp(- t / T1);   dy/dA2 = exp(- t / T2); dy/dA3 = 1;
//     //      dy/dT1 = A1 * (t / T1 / T1) * exp(- t / T1);
//     //      dy/dT2 = A2 * (t / T2 / T2) * exp(- t / T2);
//     //  *para 0 ~ 4 : A1 T1 A2 T2 A3
//     //---------------------------------------------------------------------------
//     double dtmpA, dtmpB;//, dtmpC, dtmpD, dA2;
//     
//     *y = dtmpA = para[0] *  (dyda[0] = exp( -(dtmpB = x / para[1])) );
//     dyda[1] = dtmpA * dtmpB / para[1];
//     *y+= dtmpA = para[2] *  (dyda[2] = exp( -(dtmpB = x / para[3])) );
//     *y+= para[4];
//     dyda[3] = dtmpA * dtmpB / para[3];
//     dyda[4] = 1; int i = na; na=i;
// }
// void myRangeCheck_Old(double *para, int ma, int *ia, int mfit){
//     // Suppose an old function for Range Check.
//     if(para[1]<=0)para[1]=1e-17;
//     if(para[3]<=0)para[3]=1e-17;
//     int i = ma; i = *ia; mfit = i; i= mfit;
// }
// //---------------------------------------------------------------------------
// //  We call from the new function like following
// //---------------------------------------------------------------------------
// void myFitFun(double x, double *par, double *y, double *dyda, MyLMFIT *pThis){
//     myFitFun_Old(x, par, y, dyda, pThis->nCountPar);
// }
// void myRangeCheck(double *par, MyLMFIT *pThis){
//     //pThis->nCountPar;   // this is number of par
//     //pThis->nCountFit;   // real fitting numbers
//     myRangeCheck_Old(par, pThis->nCountPar, pThis->nPara, pThis->nCountFit);
// }
// //---------------------------------------------------------------------------
// //  here is some useful methods for this class
// //---------------------------------------------------------------------------
// void myTesting(void){
//     double xData[20], yData[20]; int nFitSwitch[5]; double dFit[5], dFitError[5];
//     int    nCountA = 5, nCountD = 20, j;
//     double dChisq, dAlamda;
//     
//     MyLMFIT lm(nCountA, myFitFun, myRangeCheck);
//     lm.mySetData(xData, yData, nCountD);
//     lm.mySetParaAll(5, dFit, nFitSwitch);
//     j=lm.myLM_Min(dChisq, dAlamda); j++;
//     lm.myGetFitResult(dFit, dFitError);
//     
//     //same as
//     //MyLMFIT lm;
//     lm.pdIn_X  = xData; lm.pdIn_Y  = xData; lm.nCountDat = nCountA; // set the x, y, and number
//     lm.mySetParaCount(3);  lm.mySetPara(0, 111); lm.mySetPara(1, 123, false); lm.mySetPara(2, 134, true);// if set to false, this value will be fixed
//     // same as
//     nFitSwitch[0]=1; nFitSwitch[1]=0; nFitSwitch[2]=1;
//     dFit[0] = 111;   dFit[1] = 123;   dFit[3] = 134;   lm.mySetParaAll(3, dFit, nFitSwitch);
//     
//     lm.mySetFitFun(myFitFun, myRangeCheck);
//     // same as
//     lm.UserFun_Fit = myFitFun; lm.UserFun_Range = myRangeCheck; // select our fit and rang-check function
//     lm.myLM_Min();
// }
//---------------------------------------------------------------------------
#endif
//---------------------------------------------------------------------------