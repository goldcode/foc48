// ---------------------------------------------------------------------------
//#include "stdafx.h"

#include "comList.h"
#include <windows.h>// RegOpenKeyExA
#include <stdio.h>	// sscanf

// ---------------------------------------------------------------------------
// Following is for class myREG_KEY
// ---------------------------------------------------------------------------
myREG_KEY::myREG_KEY() {
	myResetMEM();
}
myREG_KEY::~myREG_KEY() {
	myDelMEM();
}
void __fastcall myREG_KEY::myResetMEM(void) {
	sKEY_V_Name = NULL; ppKEY_V_Value = NULL; ppKEY_Sub = NULL;
	ppKEY_Name = NULL;
	nKEY_VCount = nKEY_SubCount = 0;
	nMAXCount = 0; nMAXLength = 0; nMAXLength2 = 0;
	sKey_Save[0] = 0;
	nMAXCountSub = nMAXLengthSub = 0;
}
void __fastcall myREG_KEY::myGetMEM(const int nCount, const int nLength, const int nLength2, const int nCountSub, const int nLengthSub) {
	myDelMEM();
	nMAXCount = nCount;	nMAXLength = nLength; nMAXLength2 = nLength2;
	nMAXCountSub = nCountSub; nMAXLengthSub = nLengthSub;
	long long nBufLen = (nMAXCount * (nMAXLength + nMAXLength2 + 2) + nMAXCountSub * (nMAXLengthSub + 1));
	long long nBufPLen = (nMAXCount * 2 + nMAXCountSub);
	if (nBufLen < 1 || nBufPLen < 1) { // no need mem, we reset
		myResetMEM();
	}
	else {
		sKEY_V_Name = new char[nBufLen];
		ppKEY_Name = new char* [nBufPLen];

		if (sKEY_V_Name != NULL && ppKEY_Name != NULL) {
			ppKEY_V_Value = ppKEY_Name + nMAXCount;
			ppKEY_Sub = ppKEY_Name + nMAXCount * 2;
			for (int i = 0; i < nMAXCount; i++) {
				ppKEY_Name[i] = sKEY_V_Name + (nMAXLength + nMAXLength2 + 2) * i;
				ppKEY_Name[i][0] = 0;
				ppKEY_V_Value[i] = ppKEY_Name[i] + nMAXLength + 1;
				ppKEY_V_Value[i][0] = 0;
			}
			char* p = sKEY_V_Name + nMAXCount * (nMAXLength + nMAXLength2 + 2);
			for (int i = 0; i < nMAXCountSub; i++) {
				ppKEY_Sub[i] = p + (nMAXLengthSub + 1) * i;
			}
		}
		else {
			myDelMEM();
		}
	}
}
void __fastcall myREG_KEY::myDelMEM(void) {
	if (ppKEY_Name != NULL) {
		delete[] ppKEY_Name;
		ppKEY_Name = NULL;
	}
	if (sKEY_V_Name != NULL) {
		delete[] sKEY_V_Name;
		sKEY_V_Name = NULL;
	}
}

// ---------------------------------------------------------------------------
bool __fastcall myGetRegInfo(HKEY hKey, MY_REG_KEY& info) {
	info.achClass[0] = 0;
	info.cchClassName = MAX_PATH;
	info.cSubKeys = 0;
	FILETIME ftLastWriteTime;
	// Get the class name and the value count.
	unsigned long retCode = RegQueryInfoKeyA(
		hKey,                    // key handle
		info.achClass,           // buffer for class name
		&info.cchClassName,      // size of class string
		NULL,                    // reserved
		&info.cSubKeys,               // number of subkeys
		&info.cbMaxSubKey,            // longest subkey size
		&info.cchMaxClass,            // longest class string
		&info.cValues,                // number of values for this key
		&info.cchMaxValue,            // longest value name
		&info.cbMaxValueData,         // longest value data
		&info.cbSecurityDescriptor,   // security descriptor
		&ftLastWriteTime);       // last write time
	info.ftLastWriteTime.dwHighDateTime = ftLastWriteTime.dwHighDateTime;
	info.ftLastWriteTime.dwLowDateTime = ftLastWriteTime.dwLowDateTime;
	//f the function succeeds, the return value is ERROR_SUCCESS.
	return(retCode == ERROR_SUCCESS);
}
int __fastcall myCopy(char* pA, const char* pB, const int n) {
	memcpy(pA, pB, n);
	return n;
}
int __fastcall myStrCpy(char* pA, const char* pB, const int nMaxA)
{	// nMaxA = -1
	int nB = (int)strlen(pB); // 4G
	int nA = (nMaxA > 0) ? nMaxA - 1 : nB;
	if (nB < nA)
		nA = nB;
	myCopy(pA, pB, nA);
	pA[nA] = 0;
	return nA;
}
// ---------------------------------------------------------------------------
inline HKEY myRoot2Key(int rootKey) {
	switch (rootKey)
	{
	case MY_HKCR:
		return HKEY_CLASSES_ROOT;
	case MY_HKCU:
		return HKEY_CURRENT_USER;
	case MY_HKLM:
		return HKEY_LOCAL_MACHINE;
	case MY_HKUS:
		return HKEY_USERS;
	default:
		break;
	}
	return HKEY_LOCAL_MACHINE;
}
bool __fastcall myREG_KEY::myGET_KEY(int rootKey, const char* sKey) {

	HKEY hKey;
	//char * sKey = "SYSTEM\\CurrentControlSet\\Enum\\USB\\";
	if (RegOpenKeyExA(myRoot2Key(rootKey), sKey, 0, KEY_READ, &hKey) != ERROR_SUCCESS) {
		return false;
	}

	if (myGetRegInfo(hKey, iKey)) {
		if (iKey.cValues > 0 || iKey.cSubKeys > 0) {
			unsigned long nLen = MY_MAX_KEY_NAME_LEN;
			if (nLen > iKey.cchMaxValue)
				nLen = iKey.cchMaxValue;
			unsigned long nLen2 = MY_MAX_VALUE_NAME;
			if (nLen2 > iKey.cbMaxValueData)
				nLen2 = iKey.cbMaxValueData;
			// mem for sub-key name
			unsigned long nLen3 = MY_MAX_KEY_NAME_LEN;
			if (nLen3 > iKey.cbMaxSubKey)
				nLen3 = iKey.cbMaxSubKey;

			myGetMEM(iKey.cValues, nLen, nLen2, iKey.cSubKeys, nLen3);

			while (sKEY_V_Name != NULL && ppKEY_Name != NULL) {

				// nMAXLength
				int i = (int)strlen(sKey); if (i >= MY_MAX_KEY_NAME_LEN)i = MY_MAX_KEY_NAME_LEN - 1; // 4G 
				memcpy(sKey_Save, sKey, i);	sKey_Save[i] = 0;
				i = 0;
				char sName[MY_MAX_KEY_NAME_LEN];
				unsigned long dwName;
				unsigned char sValue[MY_MAX_KEY_NAME_LEN];
				unsigned long dwValue;
				long Status;
				unsigned long dwIndex = 0;
				unsigned long dwType;
				// get all values
				if (nMAXCount > 0)
					do
					{
						dwValue = sizeof(sValue);	dwName = sizeof(sName);
						Status = RegEnumValueA(hKey, dwIndex++, sName, &dwName, NULL, &dwType, sValue, &dwValue);
						if ((Status == ERROR_SUCCESS) || (Status == ERROR_MORE_DATA))
						{
							int nCopy;

							if (dwName > nLen)nCopy = nLen;
							else nCopy = dwName;
							myCopy(ppKEY_Name[i], sName, nCopy); ppKEY_Name[i][nCopy] = 0;

							if (dwValue > nLen2)nCopy = nLen2;
							else nCopy = dwValue;
							myCopy(ppKEY_V_Value[i], (char*)sValue, nCopy); ppKEY_V_Value[i][nCopy] = 0;

							i++;// serial port count
						}
					} while ((Status == ERROR_SUCCESS) || (Status == ERROR_MORE_DATA) && i < nMAXCount);
					nKEY_VCount = i;

					// get all subkeys
					i = 0; dwIndex = 0;
					if (nMAXCountSub > 0)
						do
						{
							dwValue = sizeof(sValue);	dwName = sizeof(sName);
							Status = RegEnumKeyExA(hKey, dwIndex++, sName, &dwName, NULL, NULL, NULL, (PFILETIME)&iKey.ftLastWriteTime);
							// Status!=ERROR_NO_MORE_ITEMS
							if ((Status == ERROR_SUCCESS) || (Status == ERROR_MORE_DATA))
							{
								int nCopy;

								if (dwName > nLen)nCopy = nLen3;
								else nCopy = dwName;
								myCopy(ppKEY_Sub[i], sName, nCopy); ppKEY_Sub[i][nCopy] = 0;

								i++;// serial port count
							}
						} while ((Status == ERROR_SUCCESS) || (Status == ERROR_MORE_DATA) && i < nMAXCountSub);
						nKEY_SubCount = i;
						break;
			};
		}
		else { // no values in this key
			myGetMEM(iKey.cValues, 0, 0); // this will clear all
		}

	}
	RegCloseKey(hKey);
	return true;
}

// ---------------------------------------------------------------------------
// Following is for class myCOM_LIST
// ---------------------------------------------------------------------------



myCOM_LIST::myCOM_LIST() {
	sCOM_Name = NULL;
	ppCOM_Name = NULL;
	myReflash();
}
myCOM_LIST::~myCOM_LIST() {
	myDelMEM();
}
void __fastcall myCOM_LIST::myGetMEM(const int nCount, const int nLength) {
	myDelMEM();
	nMAXCount = nCount;	nMAXLength = nLength;
	sCOM_Name = new char[nMAXCount * nMAXLength + nMAXCount];
	ppCOM_Name = new char* [nMAXCount];

	if (sCOM_Name != NULL && ppCOM_Name != NULL) {
		sCOM_Num = sCOM_Name + nMAXCount * nMAXLength;
		for (int i = 0; i < nMAXCount; i++) {
			ppCOM_Name[i] = sCOM_Name + nMAXLength * i;
			ppCOM_Name[i][0] = 0;
			sCOM_Num[i] = 0;
		}
	}
	else {
		myDelMEM();
	}
}
void __fastcall myCOM_LIST::myDelMEM(void) {
	if (ppCOM_Name != NULL) {
		delete[] ppCOM_Name;
		ppCOM_Name = NULL;
	}
	if (sCOM_Name != NULL) {
		delete[] sCOM_Name;
		sCOM_Name = NULL;
	}
}

unsigned long long myStrMerge(char* Out, const unsigned long long  nOutBufLen, const char** In, const unsigned long long nCount) {
	// add 0 always in the end, so output length will <nOutBufLen, MAX is nOutBufLen-1, and truncate
	unsigned long long  n = 0;
	unsigned long long  nMax = nOutBufLen - 1;
	unsigned long long  i = 0;
	while (n < nMax && i < nCount) {
		if (In[i] != NULL) {
			long long nLen = strlen(In[i]);
			if (nLen + n > nMax) nLen = nMax - n;
			memcpy(Out + n, In[i], nLen);
			n += nLen;
		}
		i++;
	}
	Out[n] = 0;
	return n;
}
unsigned long long myStrMerge(char* Out, const unsigned long long  nOutBufLen, const char* In1, const char* In2, const char* In3 = NULL) {
	const char* ppIn[3]; ppIn[0] = In1; ppIn[1] = In2; ppIn[2] = In3;
	return myStrMerge(Out, nOutBufLen, ppIn, 3);
}

int myMatchCount(const char* s1, const char* s2, const int n) {
	int i = 0;
	while (*s1++ == *s2++ && i < n)
		i++;
	return i;
}

int myFind(const char* s1, const char* s2) {
	int n1 = (int)strlen(s1);	// 4G
	int n2 = (int)strlen(s2);  // 4G
	int nS = n1 - n2;
	for (int i = n1 - n2; i >= 0; i--) {
		if (myMatchCount(s1 + i, s2, n2) == n2) return i;
	}
	return -1;
}


int __fastcall myCOM_LIST::myReflash(void) {
	myGetMEM(); // including reset all buffer
	nCOMCount = 0;
	while (sCOM_Name != NULL && ppCOM_Name != NULL) {

		HKEY hKey;
		char* sKey = "HARDWARE\\DEVICEMAP\\SERIALCOMM\\";
		if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, sKey, 0, KEY_READ, &hKey) != ERROR_SUCCESS) {
			break;
		}
		int i = 0;
		char sName[MY_MAX_COM_NAME_LEN];
		unsigned long dwName;
		unsigned char sPortName[MY_MAX_COM_NAME_LEN];
		unsigned long dwPortName;

		long Status;
		unsigned long dwIndex = 0;
		unsigned long dwType;

		do
		{
			dwPortName = sizeof(sPortName);	dwName = sizeof(sName);
			Status = RegEnumValueA(hKey, dwIndex++, sName, &dwName, NULL, &dwType, sPortName, &dwPortName);
			if ((Status == ERROR_SUCCESS) || (Status == ERROR_MORE_DATA))
			{
				myCopy(ppCOM_Name[i], sName, dwName); ppCOM_Name[i][dwName] = 0;
				int j;
				int k = sscanf_s((const char*)sPortName, "COM%d", &j);
				if (k > 0)
					sCOM_Num[i] = j;
				i++;// serial port count
			}
		} while ((Status == ERROR_SUCCESS) || (Status == ERROR_MORE_DATA) && i < MY_MAX_COM_CONUT);
		RegCloseKey(hKey);
		nCOMCount = i;
		break;
	}

	// we want to match the name in COM port with the FriendlyName name in USB port
	while (sCOM_Name != NULL && ppCOM_Name != NULL && nCOMCount > 0) {

		myREG_KEY rKey, rSubKey, rSubKey2;
		char* sKey = "SYSTEM\\CurrentControlSet\\Enum\\USB\\";
		// open USB key
		if (!rKey.myGET_KEY(MY_HKLM, sKey))
			break;

		int n = rKey.nMAXCountSub;
		// loop all sub key, which is like VID_067B&PID_2731
		for (int i = 0; i < n; i++) {
			//	2nd subkeys of the 1st subkey, which is like ABCDEF0123456789AB
			myStrMerge(rSubKey.sKey_Save, MY_MAX_KEY_NAME_LEN, rKey.sKey_Save, rKey.ppKEY_Sub[i], "\\");
			if (!rSubKey.myGET_KEY(MY_HKLM, rSubKey.sKey_Save))
				continue;
			int n2 = rSubKey.nMAXCountSub;
			// loop all this 2nd subkeys
			for (int i2 = 0; i2 < n2; i2++) {
				myStrMerge(rSubKey2.sKey_Save, MY_MAX_KEY_NAME_LEN, rSubKey.sKey_Save, rSubKey.ppKEY_Sub[i2], "\\");
				// open the second subkey
				if (!rSubKey2.myGET_KEY(MY_HKLM, rSubKey2.sKey_Save))
					continue;
				int n3 = rSubKey2.nMAXCount;
				// loop all name-value pairs, find if name is "FriendlyName"
				for (int i3 = 0; i3 < n3; i3++) {
					if (strcmp(rSubKey2.ppKEY_Name[i3], "FriendlyName") != 0)
						continue;
					char* pF = rSubKey2.ppKEY_V_Value[i3]; //int  n4 = strlen(pF);
					// search if in the value there is COM port
					int  n5 = myFind(pF, "COM");
					if (n5 < 0)
						continue;
					// found, we get the port number                    
					int n6 = 0; pF = pF + (n5 + 3);
					if (pF[0] >= '0' && pF[0] <= '9')
						n6 = pF[0] - '0';
					if (pF[1] >= '0' && pF[1] <= '9')
						n6 = n6 * 10 + (pF[1] - '0');
					// loop all found COM in the COM list, replace the correct name, nCOMCount, sCOM_Num
					for (int nn = 0; nn < nCOMCount; nn++) {
						if (sCOM_Num[nn] == n6) {
							myStrCpy(ppCOM_Name[nn], rSubKey2.ppKEY_V_Value[i3], nMAXLength); //strcpy(ppCOM_Name[nn], rSubKey2.ppKEY_V_Value[i3]);
						}
					}
				}
			}
		}
		break;
	}
	return nCOMCount;
}