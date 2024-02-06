//---------------------------------------------------------------------------
#ifndef myListMangerH
#define myListMangerH
//---------------------------------------------------------------------------
#include "myBufGet.h"    //include #define_MY_LINUX  #define_MY_NO_MEMORY_CHECK
//---------------------------------------------------------------------------
class myListItem
{
public:		// User declarations
    myListItem *pPre;
    myListItem *pNext;
    void *pUser;
private:	// User declarations

};
extern myListItem *pNULL;
class myLists
{
public:		// User declarations
        myLists();
        virtual ~myLists();

        int nCount;
        myListItem *pStart, *pEnd;
        //virtual myListItem *NewLists(int n=1);
        virtual myListItem * __fastcall NewList(int s=0);
        virtual myListItem * __fastcall NewLists(int n=1, int s=0);
        //virtual void AddToList(myListItem &A, myListItem &B=*(myListItem *)NULL);
        virtual void __fastcall AddToList(myListItem &A, myListItem &B=*pNULL);
        // Add A as a previous listitem of B, is B=NULL, A will be the last one
        virtual void __fastcall ReMoveOne(const myListItem &A);
        virtual void __fastcall ReMoveOneLink(const myListItem &A);
        virtual void __fastcall ReMoveAllFrom(const myListItem &A); // remove all after pNow
        virtual void __fastcall ReMoveAll(void); // remove all after pNow
        virtual void __fastcall JoinAB(myListItem &A, myListItem &B);
        virtual int __fastcall getItemCountFrom(const myListItem &A);  //Include A
        virtual myListItem *__fastcall getEndItemFrom(const myListItem &A, int &i);
        virtual void __fastcall myGetInfo(pMY_BUF_USE_INFO pBuf);
        virtual myListItem * __fastcall myGetItemByIndex(int i); // i= 0 ~ nCount-1
        virtual int __fastcall myGetItemIndex(const myListItem &A);  // Start Form 0
private:	// User declarations        
};

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
#endif  //static
