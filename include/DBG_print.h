

#ifndef DBG_PRINT_INCLUDED_
#define DBG_PRINT_INCLUDED_

#include <stdio.h>	// sscanf, //#include <stdlib.h>

//int myDBG_print(const char* format, ...);

#define printf(format, ...) \
myDBG_print(format, __VA_ARGS__)

extern long long gl_llSize;
extern unsigned long long hDBG;

int __fastcall myDBG_print(const char* format, ...);

const wchar_t* __fastcall Char2WChar(const char* pStr, long long& llSize = gl_llSize);
const char* __fastcall WChar2Char(const wchar_t* pStrW, long long& llSize = gl_llSize);

	
#endif // !defined(DBG_PRINT_INCLUDED_)