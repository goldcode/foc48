#pragma once


// GenIdWn dialog
#include <afxdialogex.h>
#include "../res/resource.h"       // enum { IDD = IDD_Add_ID };


class GenIdWn : public CDialogEx
{
	DECLARE_DYNAMIC(GenIdWn)

public:
	GenIdWn(CWnd* pParent = NULL);   // standard constructor
	virtual ~GenIdWn();

// Dialog Data
	enum { IDD = IDD_Add_ID };

	CString Year  = L"2020";
	CString Month = L"01";
	CString Day10 = L"0";
	CString Day1  = L"1";
	CString Index = L"a";
	
	afx_msg void OnBnClickedYear(UINT unID);
	afx_msg void OnBnClickedMonth(UINT unID);
	afx_msg void OnBnClickedDay10(UINT unID);
	afx_msg void OnBnClickedDay1(UINT unID);
	afx_msg void OnBnClickedIndex(UINT unID);

	void UpdateWindow();

	CString Name() { return Year + Month + Day10 + Day1 + Index; };
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
};
