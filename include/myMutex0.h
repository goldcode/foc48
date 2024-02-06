//---------------------------------------------------------------------------
#ifndef myMutex0H
#define myMutex0H
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
class my_aligned_malloc
{
protected:
	char *pRAW;
	char *pAligned;
	unsigned long long llSizeUser;
	unsigned long long llSizeReal;
	unsigned long long llSizeSave;

	int nAlignmentSave;
public:
	my_aligned_malloc( unsigned long long llSize=4, int nAlignment=16);
	~my_aligned_malloc();
	void *myGetMEM(void){return pAligned;}
	unsigned long long myGetMEMSize(void){return llSizeUser;}
};
//---------------------------------------------------------------------------
class myMutex {
public:
	explicit myMutex(bool bLock=false, long *nLockIn=0);  // NULL=0
	~myMutex();
	void lock(void);
    bool try_lock(void);
	void unlock(void);
	volatile long     nLockCount;
private:
	//void *hMutex;
	volatile char     cPad[4];
	//volatile long     nLock;
	long			  *nLock;
	volatile char     cPad2[4];
protected:
	my_aligned_malloc	*cLock;
};
//---------------------------------------------------------------------------
//class myMutexMEM {
//public:
//	explicit myMutexMEM(int *pMEM, bool bLock=false);
//	~myMutexMEM();
//	int	mySetLockMEM(int *pMEM);
//	void lock(void);
//	void unlock(void);
//	volatile long     nLockCount;
//private:
//	//void *hMutex;
//	volatile char     cPad[4];
//	volatile long     nLock;
//	volatile char     cPad2[4];
//};
//---------------------------------------------------------------------------
#endif

//BOOL g_fResourceInUse = FALSE;
//void ASCEFunc(){
////等待访问共享资源
//while(InterlockedExchange(&g_fResourceInUse, TRUE) == TRUE)
//sleep(0);
////访问共享资源
//...
////结束访问
//InterlockedExchange(&g_fResourceInUse, FALSE);
//}

