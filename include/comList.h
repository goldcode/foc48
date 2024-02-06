#pragma once

#ifndef comList_H
#define comList_H

#define MY_MAX_KEY_CONUT	17
#define MY_MAX_KEY_NAME_LEN	270
#define MY_MAX_KEY_LENGTH	255
#define MY_MAX_VALUE_NAME	16383
#define MY_MAX_PATH         260

#define MY_MAX_COM_CONUT	17
#define MY_MAX_COM_NAME_LEN	270

#define MY_HKCR				01	//HKEY_CLASSES_ROOT
#define MY_HKCU				02	//HKEY_CURRENT_USER
#define MY_HKLM				03	//HKEY_LOCAL_MACHINE
#define MY_HKUS				04	//HKEY_USERS
// ---------------------------------------------------------------------------
typedef struct _MY_FILETIME {
	unsigned long dwLowDateTime;
	unsigned long dwHighDateTime;
} MY_FILETIME, * LP_MY_FILETIME;

typedef struct _MY_REG_KEY
{
	char			achKey[MY_MAX_KEY_LENGTH];	// buffer for subkey name
	unsigned long   cbName;                 // size of name string 
	char			achClass[MY_MAX_PATH];	// buffer for class name	// = TEXT("")
	unsigned long   cchClassName;			// size of class string		// = MAX_PATH;
	unsigned long   cSubKeys;               // number of subkeys		// = 0
	unsigned long   cbMaxSubKey;            // longest subkey size 
	unsigned long   cchMaxClass;            // longest class string 
	unsigned long   cValues;				// number of values for key 
	unsigned long   cchMaxValue;			// longest value name 
	unsigned long   cbMaxValueData;			// longest value data 
	unsigned long   cbSecurityDescriptor;	// size of security descriptor 
	MY_FILETIME		ftLastWriteTime;		// last write time 
}MY_REG_KEY, * LP_MY_REG_KEY;

class myREG_KEY {
public:
	myREG_KEY();
	~myREG_KEY();

	virtual bool __fastcall myGET_KEY(int rootKey, const char* sKey);
	virtual void __fastcall myGetMEM(const int nCount = MY_MAX_KEY_CONUT, const int nLength = MY_MAX_KEY_NAME_LEN, const int nLength2 = MY_MAX_VALUE_NAME, const int nCountSub = 0, const int nLengthSub = 0);
	virtual void __fastcall myDelMEM(void);
	virtual void __fastcall myResetMEM(void);
	MY_REG_KEY	iKey;
	char** ppKEY_Name;
	char* sKEY_V_Name;
	char** ppKEY_V_Value;
	char** ppKEY_Sub;
	int		nKEY_VCount; // # correctly read value count ppKEY_Name ppKEY_V_Value
	int     nKEY_SubCount;
	int		nMAXCount;
	int		nMAXLength;
	int		nMAXLength2;
	int		nMAXCountSub;
	int		nMAXLengthSub;
	char    sKey_Save[MY_MAX_KEY_NAME_LEN + 1];
};

class myCOM_LIST {
public:
	myCOM_LIST();
	~myCOM_LIST();

	virtual int __fastcall myReflash(void);
	virtual void __fastcall myGetMEM(const int nCount = MY_MAX_COM_CONUT, const int nLength = MY_MAX_COM_NAME_LEN);
	virtual void __fastcall myDelMEM(void);


	char** ppCOM_Name;
	char* sCOM_Num;
	char* sCOM_Name;
	int		nCOMCount;
	int		nMAXCount;
	int		nMAXLength;
};

int myFind(const char* s1, const char* s2);
#endif