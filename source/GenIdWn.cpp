// GenIdWn.cpp : implementation file
//

//#include "stdafx.h"
//#include <afxwin.h>         // MFC core and standard components
//#include <afxext.h>         // MFC extensions
//#include <afxdialogex.h>
//#include "48well.h"
#include "GenIdWn.h"


// GenIdWn dialog

IMPLEMENT_DYNAMIC(GenIdWn, CDialogEx)

GenIdWn::GenIdWn(CWnd* pParent /*=NULL*/)	: CDialogEx(GenIdWn::IDD, pParent)
{
	
}

BOOL GenIdWn::OnInitDialog() 
{
	BOOL bResult = CDialogEx::OnInitDialog(); 
	
	CString str;
	CWnd* pButton;
	//	
	Year.Format(L"%d", time.GetYear());
	//UPDATE Buttons
	
	str.Format(L"%d", time.GetYear() - 1);

	pButton = GetDlgItem(IDC_BUTTON_LAST);
	pButton->SetWindowTextW(str);

	pButton = GetDlgItem(IDC_BUTTON_THIS);
	pButton->SetWindowTextW(Year);

	str.Format(L"%d", time.GetYear() + 1);
	pButton = GetDlgItem(IDC_BUTTON_NEXT);
	pButton->SetWindowTextW(str);

	return bResult;
}

GenIdWn::~GenIdWn()
{
}

void GenIdWn::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(GenIdWn, CDialogEx)
	ON_COMMAND_RANGE(IDC_BUTTON_LAST,   IDC_BUTTON_LAST, GenIdWn::OnBnClickedYear)
	ON_COMMAND_RANGE(IDC_BUTTON_THIS,   IDC_BUTTON_THIS, GenIdWn::OnBnClickedYear)
	ON_COMMAND_RANGE(IDC_BUTTON_NEXT,   IDC_BUTTON_NEXT, GenIdWn::OnBnClickedYear)
	ON_COMMAND_RANGE(IDC_BUTTON_m1,     IDC_BUTTON_m12, GenIdWn::OnBnClickedMonth)
	ON_COMMAND_RANGE(IDC_BUTTON_d10,    IDC_BUTTON_d30, GenIdWn::OnBnClickedDay10)
	ON_COMMAND_RANGE(IDC_BUTTON_d1,     IDC_BUTTON_d9, GenIdWn::OnBnClickedDay1)
	ON_COMMAND_RANGE(IDC_BUTTON_A,      IDC_BUTTON_O, GenIdWn::OnBnClickedIndex)	
END_MESSAGE_MAP()


void GenIdWn::OnBnClickedYear(UINT unID)
{	
	GetDlgItem(unID)->GetWindowTextW(Year);
	UpdateWindow();
}

void GenIdWn::OnBnClickedMonth(UINT unID)
{
	CWnd * pButton = GetDlgItem(unID);
	CString str;
	//
	pButton->GetWindowTextW(str);
	if (str.GetLength() == 1) str = L"0" + str;
	//
	Month = str;
	UpdateWindow();
}

void GenIdWn::OnBnClickedDay10(UINT unID)
{
	CWnd * pButton = GetDlgItem(unID);
	CString str;
	//
	pButton->GetWindowTextW(str);
	str = str.GetAt(0);
	//
	CT2A ascii(str);
	//
	Day10 = (str == Day10) ? "0" : ascii.m_psz;
	UpdateWindow();
}

void GenIdWn::OnBnClickedDay1(UINT unID)
{
	CWnd * pButton = GetDlgItem(unID);
	CString str;
	pButton->GetWindowTextW(str);
	//
	CT2A ascii(str);
	//
	Day1 = (str == Day1) ? "0" : ascii.m_psz;
	//Day1 = (str == Day1) ? "0" : str;
	UpdateWindow();
}

void GenIdWn::OnBnClickedIndex(UINT unID)
{
	CWnd * pButton = GetDlgItem(unID);
	pButton->GetWindowTextW(Index);
	UpdateWindow();
}

void GenIdWn::UpdateWindow()
{
	GetDlgItem(IDC_MEASUREMENT)->SetWindowTextW(Name());	
}

