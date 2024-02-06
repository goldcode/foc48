
#include "myHelper.h"

bool dirExists(const wchar_t *dir)
{
	//
	unsigned long ftyp = GetFileAttributes(dir);
	//
	if (ftyp == INVALID_FILE_ATTRIBUTES)
		return false;  //something is wrong with your path!

	if (ftyp & FILE_ATTRIBUTE_DIRECTORY)
		return true;   // this is a directory!

	return false;    // this is not a directory!
}

bool fileExists(const wchar_t *str)
{
	return((GetFileAttributes(str) == INVALID_FILE_ATTRIBUTES)
		? false  //lesen nicht moeglich -> Datei existiert nicht!
		: true); //die Datei existiert
}
//---------------------------------------------------------------------------
int __fastcall myStrLen(const char *In){
	if (In == NULL)return 0;
	int n = 0;
	while (In[n] != 0)n++;
	return n;
}
const char * __fastcall myStrChr(const char *A, const char B){
	if (A == NULL)return NULL;
	int n = 0;
	while (A[n] != 0 && A[n] != B)n++;
	if (A[n] == B)return (A + n);
	return NULL;
}

// correct for self trim to self
int __fastcall MyTrimCopy2(char *sOut, const int sOutBufSize, const char *sIn, const int sInBufSize, bool bSemicolonEnd, bool bZeroLenthCopy)
{	// sInBufSize=-1, bSemicolonEnd = true, bZeroLenthCopy = true, if set to false, when sIn is not avaible, sOut will not be touched
	// "1234 56 7; fa "  ->"1234567"   remove spaces if not in "" remove anyting after ';'
	int nLen = 0;
	while (1){
		if (sOut == NULL || sOutBufSize < 1) break;
		if (sIn == NULL){
			if (bZeroLenthCopy) *sOut = 0; break; // this is for self trim to self
		}
		int nLenM = sOutBufSize - 1;
		int nIn = (sInBufSize < 0) ? myStrLen(sIn) : sInBufSize;
		if (nIn < 1){
			if (bZeroLenthCopy) *sOut = 0; break;
		}
		bool bQuotationOn = false;
		for (int i = 0; i < nIn; i++)
		{
			char cOne = sIn[i];
			if (bQuotationOn){
				if (cOne == '=') bQuotationOn = false;
				else if (cOne == ';') bQuotationOn = false;
				else if (cOne == ',') bQuotationOn = false;
			}
			if (cOne == ' ' && bQuotationOn == false) continue;
			if (cOne == ';' && bSemicolonEnd) break;
			if (cOne == '"'){
				bQuotationOn = bQuotationOn ? false : true;
			}
			sOut[nLen++] = cOne;
			if (nLen >= nLenM) break;
		}
		sOut[nLen] = 0;	break;
	}
	return nLen;
}
int __fastcall myChar2Int(const char cOne){
	if (cOne <= 'Z'&&cOne >= 'A')return (cOne - 'A' + 10);
	if (cOne <= 'z'&&cOne >= 'a')return (cOne - 'a' + 10);
	if (cOne <= '9'&&cOne >= '0')return (cOne - '0');
	return 0;
}
int __fastcall myStr2Int(const char *s, const int n, bool bHex) // bHex = false
{
	if (s == NULL)return 0;
	int nRet = 0; int nF = bHex ? 16 : 10;
	for (int i = 0; i < n; i++) {
		nRet *= nF;
		nRet += myChar2Int(s[i]);   //myShowMsg("s[%d]=%c,myChar2Int(s[%d])=%d",i,s[i],i,myChar2Int(s[i]));
	}
	return ((*s == '-') ? (-nRet) : nRet);
}
int __fastcall myIsHEX(const char cOne){
	if ('0' <= cOne&&cOne <= '9') return 1;
	if ('A' <= cOne&&cOne <= 'F') return 2;
	if ('a' <= cOne&&cOne <= 'f') return 3;
	return 0;
}
bool __fastcall isNumber2(const char *pIn, const int n, bool bHex) // n = -1, bHex = false
{
	int nB = (n < 0) ? myStrLen(pIn) : n;
	for (int i = 0; i < nB; i++) {
		char cOne = pIn[i];
		if ((cOne <= '9'&&cOne >= '0') || (myIsHEX(cOne) && bHex)) {
			continue;
		}return false;
	}
	return true;
}
bool __fastcall myStr2Int2(int &nV, const char *s, const int n, bool bHex) //  n = -1, bHex = false
{
	int ns = (n < 0) ? myStrLen(s) : n;
	bool bRes = false;
	if (n > 0){
		if (isNumber2(s, n, bHex)){
			nV = myStr2Int(s, n, bHex);
			bRes = true;
		}
	}
	return bRes;
}

bool __fastcall myStr2Num2(int &dV, const char *s, const int n)
{
	int ns = (n < 0) ? myStrLen(s) : n; const char *s2 = s;
	if (ns >= 3){
		if (s[0] == '0'&&s[1] == 'x'){
			ns -= 2; s2 = s + 2;
			int nV = 0;
			if (myStr2Int2(nV, s2, ns, true)){
				dV = nV; return true;
			}
		}
	}
	return myStr2Int2(dV, s2, ns);
}
//---------------------------------------------------------------------------
