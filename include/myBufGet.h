//---------------------------------------------------------------------------
#ifndef MY_GET_BUF
#define MY_GET_BUF
//---------------------------------------------------------------------------

//#define MY_LINUX              //This is very important in linux system
//#define MY_NO_MEMORY_CHECK    //This is for debug use

//---------------------------------------------------------------------------
#ifndef MY_MEMORY_CHECK
   #ifndef  MY_NO_MEMORY_CHECK
      #define MY_MEMORY_CHECK
   #endif
#endif
//---------------------------------------------------------------------------
#include <stdio.h>    // printf
#include <stdlib.h>   // NULL
#include <string.h>     //ready #include <mem.h>

#define MY_MAX_ARRAY 1000
#define MY_DEFAULT_LENGTH       1000
//---------------------------------------------------------------------------
typedef struct _MY_BUF_USE_INFO{
   int nTotalBuf;
   int nPreBufUse;
   int nPreBufFree;
   int nAllcoUse;
   int nAllcoFree;
   int nListCount;
   int nRev[10];
}MY_BUF_USE_INFO, *pMY_BUF_USE_INFO;

void myGetBuf_KEEP(char *&pBuf, int nNow, int &nSave);

//---------------------------------------------------------------------------
class myBuf{
public:
   myBuf(int nBufLen, int nPreLocate);
   virtual ~myBuf();
   unsigned char *myGetBuf();
   void myPutBuf(unsigned char *pBuf);
   void BufReset(void);
   virtual void myGetInfo(pMY_BUF_USE_INFO pBuf);
protected:
   int nFreeCount, nPreCount;
   int nTotalCount;
   int nBufLenth;
   int nBufRealLegnth;
   unsigned char *pBufArray[MY_MAX_ARRAY];
};
//---------------------------------------------------------------------------
class myRoundBuf{
public:
   myRoundBuf(int nL=MY_DEFAULT_LENGTH);
   virtual ~myRoundBuf();
   virtual int Add(const char *pIn, int nL=-1);
   virtual int GetBuf(char *pOut, char *pInDel=NULL); // Get all
   virtual int GetBuf_Remove(char *pOut, char *pInDel=NULL);
   virtual void GetNextChar(char*&pNow);
   virtual char *mySearchBuf(char *pInDel, int *pOutL=NULL);
   virtual int myDoCopy(char *pOut, char *pCopyEnd=NULL);  // All
protected:
   char *pCycBuf;
   char *pCycBuf_End;
   int nStart, nEnd, nCount;
   int nBufLength;       
};
//---------------------------------------------------------------------------
/*--------              Sample Code           -------------------------------
myRoundBuf P(30);
P.Add("1234567890123456789012345678901234567890",29);
i=P.GetBuf(sBuf, "345");
i=P.GetBuf_Remove(sBuf, "345");
P.Add("1234567890123456789012345678901234567890",1);
i = P.Add("abc45678901234567890");
i = P.GetBuf_Remove(sBuf, "abc");
i = P.GetBuf_Remove(sBuf, "123");
i = P.GetBuf_Remove(sBuf, "1");
---------------------------------------------------------------------------*/
//---------------------------------------------------------------------------
#endif
