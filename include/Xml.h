#pragma once

#ifndef XML_H
#define XML_H


//#include "tinyxml2.h"

//using namespace tinyxml2;

#include <afxcoll.h>  //CStringArray
#include <atlstr.h>  // for CString


class Xml {
public:
	//
	CStringArray Element;
	//
	Xml(CString& fName, const char* mode = "w") {
		//
		CStringA safName = fName;
		const char* sfName = safName;
		printf("write to %s in mode %s\n", sfName, mode);
		if (fopen_s(&pFile, sfName, mode) != 0)
		{
			close();
		}
		//
		if (pFile == NULL) MessageBox(NULL, L"Failed to open ", fName, NULL);
		//
		if (mode == "w") fprintf(pFile, "<?xml version=\"1.1\" encoding=\"UTF-8\"?>\n");
	}

	void closeElement() {
		int nEle = (int)Element.GetCount(); // 4G
		if (nEle > 0) {
			int nEle1 = nEle - 1;
			CString str = Element.ElementAt(nEle1);
			const wchar_t* pstr = str;
			fprintf(pFile, "</%ls>\n", pstr);
			Element.RemoveAt(nEle1);
		}
	}
	
	Xml() {
		pFile = NULL;
	}

	void close() { if (pFile != NULL) fclose(pFile); pFile = NULL; };

	~Xml() {
		for (int i = (int)Element.GetCount(); i > 0; i--) closeElement(); // 4G
		close();
	}

	void openElement(CString sIn) {
		Element.Add(sIn);
		fprintf(pFile, CStringA(L"<" + sIn + L">\n"));
	}
		
	static void print(FILE* pFileX, CString tag, CString fmt, double val)
	{
		fmt = L"<" + tag + L">" + fmt + L"< / " + tag + L">\n";
		fprintf(pFileX, (CStringA)fmt, val);
	}
	
	void print(CString tag, CString fmt, double val, CString comment)
	{
		fmt = L"<" + tag + L">" + fmt + L"</" + tag + L"> <!--" + comment + "-->\n"; 
		fprintf(pFile, (CStringA)fmt, val);
	}

	void print(CString tag, CString fmt, double val)
	{
		fmt = L"<" + tag + L">" + fmt + L"</" + tag + L">\n";
		fprintf(pFile, (CStringA)fmt, val);
	}

	void print(CString tag, CString fmt, CString str)
	{
		fmt = L"<" + tag + L">" + fmt + L"</" + tag + L">\n";
		fprintf(pFile, (CStringA)fmt, str);
	}

	FILE* pFile;

};

#endif