
//#include <stdafx.h>


#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions
#include <stdio.h>


//#include "DBG_print.h"

unsigned long long hDBG = 0L;
long long gl_llSize;

//const wchar_t	* __fastcall Char2WChar(const char    *pStr, long long &llSize = gl_llSize);
//const char * __fastcall WChar2Char(const wchar_t *pStrW, long long &llSize);

// ---------------------------------------------------------------------------

int __fastcall myAddText(HWND hh, const char *pStr);

int myDBG_print(const char* format, ...) {
	//{
	static char s_printf_buf[2048];
	va_list args;
	va_start(args, format);
	int nSize = _vsnprintf_s(s_printf_buf, sizeof(s_printf_buf) - 1, format, args);
	va_end(args);
	if (nSize > 0)
		s_printf_buf[nSize] = 0;
	s_printf_buf[2047] = 0;
	if (hDBG != 0)
		myAddText(reinterpret_cast<HWND>(hDBG), s_printf_buf);
	else
		OutputDebugStringA(s_printf_buf);
	return 0;
};
const wchar_t * __fastcall Char2WChar(const char *pStr, long long &llSize)
{	//llSize = gl_llSize
	//const wchar_t * __fastcall Char2WChar(const char *pStr, long long &llSize){
	llSize = strlen(pStr) + 1;
	wchar_t *pStrW = new wchar_t[llSize];
	long long llSize2 = MultiByteToWideChar(0, 0, pStr, (int)llSize, pStrW, (int)llSize);
	//mbstowcs(pStrW, pStr, llSize);
	return pStrW;
};
const char * __fastcall WChar2Char(const wchar_t *pStrW, long long &llSize)
{	//long long &llSize = gl_llSize
		llSize = wcslen(pStrW) + 1;
		char* szRes;
		int i = WideCharToMultiByte(CP_ACP, 0, pStrW, -1, NULL, 0, NULL, NULL);
		szRes = new char[(int)(i + 1)];
		if (szRes != NULL)
			WideCharToMultiByte(CP_ACP, 0, pStrW, -1, szRes, i, NULL, NULL);
		return szRes;
}
int __fastcall myAddText(HWND hh, const char *pStr){
	unsigned long unPosS, unPosE;
	SendMessage(hh, EM_GETSEL, reinterpret_cast<WPARAM>(&unPosS), reinterpret_cast<WPARAM>(&unPosE));
	int nLength = GetWindowTextLength(hh);
	SendMessage(hh, EM_SETSEL, nLength, nLength);
	const wchar_t *pStrW = Char2WChar(pStr, gl_llSize);
	//SendMessage(hh, EM_REPLACESEL, TRUE, reinterpret_cast<LPARAM>(pStr));
	SendMessage(hh, EM_REPLACESEL, TRUE, reinterpret_cast<LPARAM>(pStrW));
	SendMessage(hh, EM_SETSEL, unPosS, unPosE);
	nLength = GetWindowTextLength(hh);
	delete[] pStrW;
	return nLength;
};
