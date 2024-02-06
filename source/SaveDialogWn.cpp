//

//#include "stdafx.h"
//#include <afxwin.h>         // MFC core and standard components
//#include <afxext.h>         // MFC extensions
//#include <afxdialogex.h>
#include "SaveDialogWn.h"
#include "GenIdWn.h"
//#include "myHelper.h"
//#include "48well.h"
//#include "../res/resource.h"

//#include <vector>
//#include <string>
#include "Parameters.h"   //#include "tinyxml2.h", #include "myHelper.h"

using namespace tinyxml2;

//
/*
BOOL myCListBox::Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID){

	BOOL i=CListBox::Create(dwStyle, rect, pParentWnd, nID);
	int j = dwStyle&LBS_SORT;
	return i;
}
*/

int myCListBox::CompareItem(LPCOMPAREITEMSTRUCT lpCompareItemStruct)
{
	//int ret = CWnd::OnCompareItem();

	ASSERT(lpCompareItemStruct->CtlType == ODT_LISTBOX);
	LPCTSTR lpszText1 = (LPCTSTR)lpCompareItemStruct->itemData1;
	ASSERT(lpszText1 != NULL);
	LPCTSTR lpszText2 = (LPCTSTR)lpCompareItemStruct->itemData2;
	ASSERT(lpszText2 != NULL);

	if ( lstrlen(lpszText1) != lstrlen(lpszText2)) {
		return lstrlen(lpszText2) - lstrlen(lpszText1);
	}

	return _tcscmp(lpszText2, lpszText1);
}

void __fastcall RemoveXML(CString& A) {
	int i;
	if ((i = A.GetLength()) > 3) {
		char cL = (char)A.GetAt(i - 1) & 0xDF;
		char cM = (char)A.GetAt(i - 2) & 0xDF;
		char cX = (char)A.GetAt(i - 3) & 0xDF;
		char cD = (char)A.GetAt(i - 4);

		if (cL == 'L' && cX == 'X' && cM == 'M' && cD == '.') {
			A = A.Left(i - 4);
		}
	}
}

CString myCListBox::GetName() {
	// To ensure the code does not brake if window is closed
	//
	CString str, str_o;
	int sel = GetCurSel();
	GetText(sel, str);
	str_o = str;
	str.TrimLeft('[');
	str.TrimRight(']');
	RemoveXML(str);
	printf("Old %ls became %ls\n", str_o, str);
	return str;
}

	

CString myCListBox::updateExpList(CString inPath, CString inName)
{
	//	
	exportPath = inPath;
	int idx = 0;
	//
	SetCurrentDirectory(exportPath);
	//
	ResetContent();
	//
	//Get Sorted Content
	ModifyStyle(NULL, LBS_SORT, 0);
	Dir(DDL_DIRECTORY, _T("20???????"));
	ModifyStyle(LBS_SORT, NULL, 0);
	//inverse Sorting
	//CString str;
	int iLast = GetCount();
	for (int i = iLast - 2; i >= 0; i--) {
		GetText(i, name);
		DeleteString(i);
		InsertString(iLast - 1, name);
	}
	if (inName) {
		idx = FindString(-1, inName);
		if (idx == LB_ERR) {
			idx = 0;
		}
	}
	//	
	SetCurSel(idx);
	
	return name;
}

CString myCListBox::updateMeaList(CString dir)
{
	//CString name;
	char Index1 = ' ';
	char Index2 = 'a';
	int  nFiles = 0;
	int  nModulus;
	int nAlphabet = 26;
	CString LastMea;
	ResetContent();
	//
	if (dirExists(dir)) {
		SetCurrentDirectory(dir);
		int i = GetStyle();
		int j = i & LBS_SORT;
		//m_ExpList.ModifyStyle(NULL, LBS_SORT, 0); 
		Dir(DDL_READWRITE, (myDateStamp() + "?.xml"));       // 20201209a  .. 20201209z
		Dir(DDL_READWRITE, (myDateStamp() + "??.xml"));      // 20201209aa .. 20201209zz
		//m_ExpList.ModifyStyle(LBS_SORT, NULL, 0);
		nFiles = GetCount();
		if (nFiles > 0) {
			GetText(0, LastMea);
		}
	}
	//
	nModulus = nFiles % nAlphabet;
	//
	if (nModulus)
	{
		Index1 = (char) nModulus-1 + 'a';
		Index2 = (char) nFiles - nModulus * nAlphabet + 'a';
		name = myDateStamp() + (CString)Index1 + (CString)Index2;
	}
	else
	{
		Index2 = (char)nFiles - nModulus * nAlphabet + 'a';
		name = myDateStamp() +  (CString)Index2;
	}

	if (! suffix.IsEmpty()) {
		name = name + "_" + suffix;
	}

	printf("Modulus %.1f nAlphabet %.1f nFile %.1f\n", nModulus, nAlphabet, nFiles);

	int ret = InsertString(0, name);
	SetCurSel(0);
	return name;
}

/*
void SaveDialogWn::OnTimer(UINT_PTR nIDEvent)
{
	if (nIDEvent == ID_CLOSE_TIMER)
	{
		EndDialog(IDOK);
	}	
}
*/


//pParent = NULL, CString exportPath = L"", CString suffix = L"", CString expName = L"", CString meaName = L""

//SaveDialogWn::SaveDialogWn(CWnd* pParent /*=NULL*/, CWnd*)	: CDialogEx(SaveDialogWn::IDD, pParent)
SaveDialogWn::SaveDialogWn(CWnd* pParent, CString _exportPath, CString _suffix, CString _expName, CString _meaName) : CDialogEx(SaveDialogWn::IDD, pParent)
{	
	//	
	if (! _exportPath.IsEmpty())
			ExportPath(_exportPath);
	if (! _suffix.IsEmpty())
			Suffix(_suffix);
	if (! _expName.IsEmpty())
			ExpName(_expName);
	if (!_meaName.IsEmpty())
		MeaName(_meaName);
	
	//pm_ExpList = NULL;


}

SaveDialogWn::~SaveDialogWn()
{
	
}

void SaveDialogWn::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EXP_LIST, m_ExpList);
	DDX_Control(pDX, IDC_MEA_LIST, m_MeaList);
}


BEGIN_MESSAGE_MAP(SaveDialogWn, CDialogEx)
	
	ON_WM_TIMER()
	
	//ON_BN_CLICKED(IDOK, SaveDialogWn::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, SaveDialogWn::OnBnClickedCancel)
	ON_STN_CLICKED(IDC_SAVEPATH, &SaveDialogWn::OnStnClickedSavepath)

	ON_BN_CLICKED(IDC_Add_Experiment, &SaveDialogWn::OnBnClickedAddExperiment)
	ON_BN_CLICKED(IDC_Button_MEA,     &SaveDialogWn::OnBnClickedButtonMea)
	ON_LBN_SELCHANGE(IDC_MEA_LIST,    &SaveDialogWn::OnLbnSelchangeMeaList)
	ON_LBN_SELCHANGE(IDC_EXP_LIST,    &SaveDialogWn::OnLbnSelchangeExpList)
	ON_WM_COMPAREITEM()

	ON_BN_CLICKED(ID_SAVE,       &SaveDialogWn::OnBnClickedSave)
	ON_BN_CLICKED(ID_SAVE_ALL,   &SaveDialogWn::OnBnClickedSaveAll)
	ON_BN_CLICKED(ID_SAVE_range, &SaveDialogWn::OnBnClickedSave)
	ON_BN_CLICKED(IDC_AUTOCLOSE, &SaveDialogWn::OnBnClickedAutoClose)

	//ON_BN_CLICKED(ID_SAVE_10,  &SaveDialogWn::OnBnClickedSaveLast10)
	//ON_BN_CLICKED(ID_SAVE_30,  &SaveDialogWn::OnBnClickedSaveLast30)
	//ON_BN_CLICKED(ID_SAVE_60,  &SaveDialogWn::OnBnClickedSaveLast60)
	//ON_BN_CLICKED(ID_SAVE_120, &SaveDialogWn::OnBnClickedSaveLast120)

	ON_BN_CLICKED(ID_EDIT_MEA, &SaveDialogWn::OnBnClickedEditMea)

END_MESSAGE_MAP()


CString SaveDialogWn::ExpName(CString sIn){
	//
	// Set expName
    //
	if (!sIn.IsEmpty()) 
	{
		expName = sIn;
		return expName;
	}
	//
	// To ensure the code does not brake if window is closed
	//
	if (m_ExpList.m_hWnd == NULL)
		return expName;
	//
    int sel = m_ExpList.GetCurSel();
	if (sel == LB_ERR){
		expName = defName;
	} else {
		m_ExpList.GetText(sel, expName);
	}
	expName.TrimLeft('[');
	expName.TrimRight(']');

	return expName;
}





CString SaveDialogWn::MeaName(CString sIn){
	//
	// Set Function
	//
	if (!sIn.IsEmpty()) {
		meaName = sIn;
		return meaName;
	}	
	//
	// To ensure the code does not brake if window is closed
	//
	if (m_MeaList.m_hWnd == NULL)
		return meaName; 
	//
	// generate Measurement Name
	// 
	int sel = m_MeaList.GetCurSel();
	//
	if (sel == LB_ERR){
		meaName = myDateStamp() + L"a";
	}
	else {
		m_MeaList.GetText(sel, meaName);
	}
	RemoveXML(meaName);
	//
	return meaName;
}

int __fastcall myD2I(const double d);

BOOL SaveDialogWn::OnInitDialog()
{
	//
	BOOL bResult = CDialogEx::OnInitDialog();
	//
	SaveTime(saveTime);
	SaveCaption.Format(L"Save %.0f sec", saveTime);
	SetDlgItemText(ID_SAVE, SaveCaption);
	SetDlgItemText(IDC_SAVEPATH, exportPath);
	SetDlgItemText(IDC_SUFFIX, suffix);
	m_MeaList.suffix = suffix;
	//
	CButton* m_ctlCheck = (CButton*)GetDlgItem(IDC_AUTOCLOSE);
	m_ctlCheck->SetCheck(bAutoClose);


	updateExpList();
	updateMeaList();

	if (! bInteractive) {
		SetTimer(ID_CLOSE_TIMER, myD2I(duration * 1000) ,  myCloseDlg); // 10000ms = 10 secs
		SetTimer(ID_UPDATE_TIMER,          100,  myUpdateTitle); // 10000ms = 10 secs
		currTime = (double)clock() / CLOCKS_PER_SEC;
		closeTime = currTime + duration;
	}
	//
	
	//
	return 	bResult;
}

double SaveDialogWn::SaveTime(double dIn)
{
	if (dIn > 0) saveTime = dIn;
	return saveTime;
}


bool SaveDialogWn::AutoClose(int iIn)
{
	//CButton* m_ctlCheck = (this->m_hWnd != NULL) ? (CButton*)this->GetDlgItem(IDC_AUTOCLOSE): NULL;
	//
	if (iIn >= 0)
		bAutoClose = iIn;
	//	if (m_ctlCheck!=NULL)
	//		m_ctlCheck->SetCheck(bAutoClose);
	//}
	//	
	//if (m_ctlCheck != NULL)
	//	bAutoClose = (m_ctlCheck->GetCheck() == BST_UNCHECKED) ? false : true;	
	//
	return bAutoClose;
}



//extern CDemoApp theApp;

VOID CALLBACK myUpdateTitle(HWND hwnd, UINT message, unsigned long long idTimer, DWORD dwTime)
{
	
	SaveDialogWn *theDlg = dynamic_cast< SaveDialogWn *> (CWnd::FromHandle(hwnd));
	theDlg->currTime = (double)clock() / CLOCKS_PER_SEC;
	theDlg->TitleCaption.Format(L"Saving in %4.1f sec to ", theDlg->closeTime - theDlg->currTime);	
	//PostMessage(hwnd, WM_SETTEXT, theDlg.currTime = (double)clock() / CLOCKS_PER_SEC;	
	theDlg->SetWindowText(theDlg->TitleCaption);
}

VOID CALLBACK myCloseDlg(HWND hwnd, UINT message, unsigned long long idTimer, DWORD dwTime)
{
	SaveDialogWn* theDlg = dynamic_cast<SaveDialogWn*> (CWnd::FromHandle(hwnd)); 
	theDlg->OnBnClickedSave();
	//PostMessage(hwnd, WM_QUIT, IDOK,0);
}

void SaveDialogWn::OnBnClickedSave()
{
	filePath = FilePath();
	fileRoot = FileRoot();
	bOK = true;
	bCancel = false;	
	CDialogEx::OnOK();
}

void SaveDialogWn::OnBnClickedSaveAll()
{
	saveTime = -1; 
	OnBnClickedSave();	
}

void SaveDialogWn::OnBnClickedSaveLast10()
{
	saveTime = 10;
	OnBnClickedSave();
}

void SaveDialogWn::OnBnClickedSaveLast30()
{
	saveTime = 30;
	OnBnClickedSave();
}

void SaveDialogWn::OnBnClickedSaveLast60()
{
	saveTime = 60;
	OnBnClickedSave();
}

void SaveDialogWn::OnBnClickedSaveLast120()
{
	saveTime = 120;
	OnBnClickedSave();
}





void SaveDialogWn::OnBnClickedCancel()
{
	bCancel = true;
	bOK = false;
	CDialogEx::OnCancel();
}


void SaveDialogWn::OnStnClickedSavepath()
{
	// TODO: Add your control notification handler code here
	CWnd * savePath = GetDlgItem(IDC_SAVEPATH);
	savePath->SetWindowText(exportPath);	

}

void SaveDialogWn::OnBnClickedEditMea()
{


}

void SaveDialogWn::OnBnClickedAddExperiment()
{
	//
	GenIdWn idDlg(NULL);
	//
	idDlg.DoModal();
	CString str = idDlg.Name();
	//
	int ret = m_ExpList.InsertString(0, str);
	//m_ExpList.AddString(str);
	//m_ExpList.SetSel(0);
	m_ExpList.SetCurSel(0);
	updateMeaList();
}


void SaveDialogWn::OnBnClickedButtonMea()
{
	// TODO: Add your control notification handler code here
}

void SaveDialogWn::OnBnClickedAutoClose()
{
	CButton* m_ctlCheck =  (CButton*)this->GetDlgItem(IDC_AUTOCLOSE);
	bAutoClose = (m_ctlCheck->GetCheck() == BST_UNCHECKED) ? false : true;
}


void SaveDialogWn::OnLbnSelchangeMeaList()
{
	// TODO: Add your control notification handler code here
}


void SaveDialogWn::OnLbnSelchangeExpList()
{
	updateMeaList();
}

void SaveDialogWn::updateMeaList()
{
	CString  dir = FilePath();
	char Index0 = 'a';
	char Index1 = 'a';

	int nFiles = 0;
	int nModulus = 0;
	int nAlphabet = 26;
	//
	m_MeaList.ResetContent();
	//
	if (dirExists(dir)){
		SetCurrentDirectory(dir);
		//int i = m_ExpList.GetStyle();
		//int j = i&LBS_SORT;
		m_MeaList.ModifyStyle(NULL, LBS_SORT, 0); 
		//m_MeaList.Dir(DDL_READWRITE, (DateStamp() + "?.xml"));     //20201209a  ..  20201209z		
		//m_MeaList.Dir(DDL_READWRITE, (DateStamp() + "??.xml"));    //20201209aa ..  20201209zz		
		//
		m_MeaList.Dir(DDL_READWRITE, (myDateStamp() + "*.xml"));    //20201209aa ..  20201209zz
		m_MeaList.ModifyStyle(LBS_SORT, NULL, 0);
		//

		nFiles = m_MeaList.GetCount();

		CString str, lpszBuffer;
		int n, nIndex;
		for (int i = 0; i < m_MeaList.GetCount(); i++)
		{
			//m_ExpList.GetText(nIndex, lpszBuffer);
			n = m_MeaList.GetTextLen(i);
			m_MeaList.GetText(i, str.GetBuffer(n));
			int icount = 0;
			char *ptr = CT2A(str);
			while ((ptr = strchr(ptr, '.')) != NULL)
			{
				icount++;
				ptr++;
			}
			if (icount != 1)
				m_MeaList.DeleteString(i);
		}
		nFiles = m_MeaList.GetCount();		
	} 
	//
	nModulus = nFiles / nAlphabet;
	//
	if (nFiles == 0) {		
		meaName = myDateStamp() + (CString) 'a';
	} else if (nModulus == 0) {
		meaName = myDateStamp() + (CString) (char) (nFiles + 'a');
	}	else {
		meaName = myDateStamp() + (CString)(char) (nModulus-1 + 'a') + (CString) (char) (nFiles % nAlphabet + 'a');
	}
	//
	if (!suffix.IsEmpty()) {
		meaName = meaName + "_" + suffix;
	}
	//
	int ret = m_MeaList.InsertString(0, meaName);

	m_MeaList.SetCurSel(0);
}




void SaveDialogWn::updateExpList()
{	
	//	
	int idx = 0;
	//
	SetCurrentDirectory(exportPath);
	//
	m_ExpList.ResetContent(); 
	//
	//Get Sorted Content
	m_ExpList.ModifyStyle(NULL,LBS_SORT,0);
	m_ExpList.Dir(DDL_DIRECTORY, _T("20???????"));
	m_ExpList.ModifyStyle(LBS_SORT,NULL,0); 	
	//inverse Sorting
	CString str;
	int iLast = m_ExpList.GetCount();
	for (int i = iLast-2; i >= 0; i--){
		m_ExpList.GetText(i, str);
		m_ExpList.DeleteString(i);
		m_ExpList.InsertString(iLast-1, str);
	}
	if (expName){
		idx = m_ExpList.FindString(-1, expName);
		if (idx == LB_ERR) {
			idx = 0;
		}
	}
	//	
	m_ExpList.SetCurSel(idx);
	//
	updateMeaList();
}

afx_msg int SaveDialogWn::OnCompareItem(
	int nIDCtl,
	LPCOMPAREITEMSTRUCT lpCompareItemStruct){
	myCListBox *p = reinterpret_cast<myCListBox *>(GetDlgItem(nIDCtl));
	if (p != NULL)
		return p->CompareItem(lpCompareItemStruct);
	else
		return CDialogEx::OnCompareItem(nIDCtl, lpCompareItemStruct);

}