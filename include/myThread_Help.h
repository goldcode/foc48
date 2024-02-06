//---------------------------------------------------------------------------
#ifndef myThread_HelpH
#define myThread_HelpH
//---------------------------------------------------------------------------
// Shift between VC and Linux
//---------------------------------------------------------------------------

#ifndef _MSC_VER
#define     MY_USE_pthread
#endif

//---------------------------------------------------------------------------
// some constant values
//---------------------------------------------------------------------------

//#define MY_LCC              // use under matlab for dll
#define MY_MAX_THREAD		1000
#define MY_DEFAULT_THREAD   1
#define MY_t_STACK_SIZE		2048


//---------------------------------------------------------------------------
#ifndef MY_USE_pthread      // Following code is for windows
//---------------------------------------------------------------------------
#if (_MSC_VER <= 1600)   // ... Do VC10/Visual Studio 2010 specific stuff
#include <windows.h>
#include <process.h>
#include <stdio.h>

typedef struct _myThreadWrap{
    void *(*start_routine) (void *);
    void *arg;
}myThreadWrap;

static unsigned __stdcall threadMain(void *arg){
    myThreadWrap *p=static_cast<myThreadWrap *>(arg);
    if (p!=NULL)
        p->start_routine(p->arg);
    return 0;
}

typedef struct myEasy_pthread_t{
    void  *p;            /* Pointer to actual object */
    unsigned int x;             /* Extra information - reuse count etc */
    unsigned int id;
    myThreadWrap pp;
}pthread_t;

typedef struct myEasy_pthread_attr_t{
    unsigned int x;
}pthread_attr_t;

int pthread_create (pthread_t *tt, const pthread_attr_t *attr, void *(*start_routine) (void *), void *arg);
int pthread_join   (pthread_t &tt, void **retval);

//---------------------------------------------------------------------------
#else                   // for VS 2011 and later, or for Linux with std:thread supports
//---------------------------------------------------------------------------
#ifndef MY_LCC
#include <windows.h>

#endif
#ifdef MY_FORCE_WIN

#include <stdio.h>

typedef struct _myThreadWrap{
	void *(*start_routine) (void *);
	void *arg;
}myThreadWrap;

static unsigned long __stdcall threadMain(void *arg){
	myThreadWrap *p = static_cast<myThreadWrap *>(arg);
	if (p != NULL)
		p->start_routine(p->arg);
	return 0;
}

typedef struct myEasy_pthread_t{
	void  *p;            /* Pointer to actual object */
	unsigned int x;             /* Extra information - reuse count etc */
	unsigned int id;
	myThreadWrap pp;
}pthread_t;

typedef struct myEasy_pthread_attr_t{
	unsigned int x;
}pthread_attr_t;

#else

#include <thread>

typedef struct myEasy_pthread_t{
    std::thread  *p;            /* Pointer to actual object */
    unsigned int x;             /* Extra information - reuse count etc */
}pthread_t;

typedef struct myEasy_pthread_attr_t{
    unsigned int x;
}pthread_attr_t;

#endif // MY_FORCE_WIN

int pthread_create(pthread_t *tt, const pthread_attr_t *attr, void *(*start_routine) (void *), void *arg);
int pthread_join  (pthread_t &tt, void **retval);

#endif
//---------------------------------------------------------------------------
#else                       // Following code is using pthread.h for example in linux
//---------------------------------------------------------------------------
#include <pthread.h>
#ifndef _SC_NPROCESSORS_ONLN
#include <sys/sysinfo.h>
#endif
//---------------------------------------------------------------------------
#endif                      // end of different system
//---------------------------------------------------------------------------

int myThreadNumber_All(unsigned int &nC);

typedef struct _MY_INFO_DD{
    int nType, nRev;
    const double *pIn;
    double *pOut;
}sMY_INFO_DD;
typedef struct _MY_INFO_UI{
    int nType, nRev;
    const unsigned int *pIn;
    unsigned int *pOut;
}sMY_INFO_UI;
typedef struct _MY_INFO_II{
    int nType, nRev;
    const int *pIn;
    int *pOut;
}sMY_INFO_II;
typedef struct _MY_INFO_US{
    int nType, nRev;
    const unsigned short *pIn;
    unsigned short *pOut;
}sMY_INFO_US;
typedef struct _MY_INFO_SS{
    int nType, nRev;
    const short *pIn;
    short *pOut;
}sMY_INFO_SS;
typedef union _MY_INFO_DATA{
    sMY_INFO_DD dd;
    sMY_INFO_UI ui;
    sMY_INFO_II ii;
    sMY_INFO_US us;
    sMY_INFO_SS ss;
    int nType;
}uMY_INFO_DATA;

typedef void *(*MyRUN)(void *); //define a thread type function
typedef void *(*MyThreadFunH)(uMY_INFO_DATA *pUserData, int *pInternalBuffer, volatile int &nUserCancel); //define a thread type function
typedef void * MyThreadDataH; //define a thread type function

typedef struct _MY_THREAD_INFO{
    pthread_t tid;
    uMY_INFO_DATA *pData;
    int *pBuf;
    MyThreadFunH pFun;
    volatile int *pUserCancel;
	void *pCHK;
}sMY_THREAD_INFO;

class myThreads;
class myMutex;

typedef struct _MY_CHK_THREAD_INFO{
    pthread_t tid;
	pthread_t tid2;
    myThreads *pData;
    sMY_THREAD_INFO *pThread;
    unsigned int nThreadCount;
    volatile int *pUserCancel;
	volatile int *pPermit;
	myMutex *mxCheck;
    volatile bool *pbStopped;
}MY_CHK_THREAD_INFO;

class myThreads{
public:
	// automatically use CPU threads as max
    myThreads(int nMaxCount = -1);
    ~myThreads();

    // this fit for each thread is not using the same data or function
    virtual void __fastcall AddThreadFunction (MyThreadFunH pFun, MyThreadDataH pData=NULL, int *pIntBuf = NULL);

    // this will modify the prestored function and data
    virtual void __fastcall SetThreadFunction (MyThreadFunH pFun, const int nID, MyThreadDataH pData=NULL, int *pIntBuf = NULL);

    // this fit for same function with different data in a batch way
    virtual void __fastcall SetThreadFunctions(MyThreadFunH pFun, const int nCount, MyThreadDataH *ppData, int **ppIntBuf = NULL);

    // this fit for different functions with same data in a batch way
    virtual void __fastcall SetThreadFunctions(MyThreadFunH *ppFun, const int nCount, MyThreadDataH pData=NULL, int *pIntBuf = NULL);

    virtual bool __fastcall Start(int nPriority);
    virtual bool __fastcall Start(int nPriority, MyThreadFunH pFun, MyThreadDataH pData, int *pIntBuf=NULL);
    // in this spetical case, pIntBuf is 1d must have nCount values
    virtual bool __fastcall Start(int nPriority, MyThreadFunH pFun, MyThreadDataH pData, int *pIntBuf, const int nCount);

    virtual bool __fastcall IsStop(void){return bStopped;};

    // user in there thread function can check this state, descide with to do
    // e.g. if this value is 0, user can run and set to 1, stop
    virtual int  __fastcall GetUserCancel(void){return nUserCancel;};
    virtual int  __fastcall SetUserCancel(const int n=1){nUserCancel=n; return nUserCancel;};

    virtual int  __fastcall Get_CPU_Number(void){return nCPUThreadCount;};
    virtual int  __fastcall Get_Threads_Number(void){return nThreadCount;};
    virtual int  __fastcall Get_Max_Threads_Number(void){return nThreadCountMax;};

    // for parallel calculation, split n to different parts with nThread number of threads
    // if nThread <0, it will min(nThreadCountMax, nCPUThreadCount) as total threads
    virtual int  __fastcall mySetN(int n, int nThread=-1);

    int	*pIntBuf;
    static void * Run  (void *pA);
	myMutex *mxCheck;

protected:

    static void * Check(void *pA);
    virtual void __fastcall myEndThreads(void);
    virtual void __fastcall myMemINIT(unsigned int nMax);
    virtual void __fastcall myMemDel (void);
    virtual void __fastcall myMemGet (unsigned int nMax);

    sMY_THREAD_INFO *pThread;
    unsigned int nThreadCountMax, nThreadCount, nCPUCount, nCPUThreadCount;
    volatile bool bStopped;
    volatile int  nUserCancel; // if nUserCancel > 0, means user want to terminate the threads
	volatile int  nPermit; // nPermit=0, new thread shall be suspended. Used for Priority setting. Priority set will get error, if thread is done.
    MY_CHK_THREAD_INFO tThis;
    myThreads *pThis;
	char * pHeapMem;
};


//---------------------------------------------------------------------------
#endif
//---------------------------------------------------------------------------
