#ifndef myHelper_H
#define myHelper_H

#include <atltime.h>	// CTime

bool dirExists(const wchar_t* dir);
bool fileExists(const wchar_t* str);

#define myDateStamp()	(CTime::GetCurrentTime().Format(_T("%Y%m%d")))


//static CString myDateStamp()
//{
//	return CTime::GetCurrentTime().Format(_T("%Y%m%d"));
//}

#define myTimeStamp()	(CTime::GetCurrentTime().Format(_T("%H:%M:%S")))

//static CString myTimeStamp()
//{
//	return CTime::GetCurrentTime().Format(_T("%H:%M:%S"));
//}
//---------------------------------------------------------------------------
int		__fastcall MyTrimCopy2(char *sOut, const int sOutBufSize, const char *sIn, const int sInBufSize = -1, bool bSemicolonEnd = true, bool bZeroLenthCopy = true);
bool	__fastcall myStr2Int2(int &nV, const char *s, const int n = -1, bool bHex = false);
bool	__fastcall myStr2Num2(int &dV, const char *s, const int n = -1); // support 0xNNNN for hex 16
//---------------------------------------------------------------------------

#endif