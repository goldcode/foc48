#pragma once
//#include "afxwin.h"
#include "../res/resource.h"
//#include "Parameters.h"
#include <afxdialogex.h>

// AltSavDialog dialog

class myCListBox : public CListBox
{
	//DECLARE_DYNAMIC(myCListBox)

public:	
	//myCListBox(CWnd* pParent = NULL);   // standard constructor
	//virtual ~myCListBox();
	//virtual BOOL Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID);

	virtual int CompareItem(LPCOMPAREITEMSTRUCT lpCompareItemStruct);
	//
	CString exportPath;
	CString defName = L"20990101a"; 
	CString suffix  = L""; 
	CString name;
	CString GetName();
	CString updateExpList(CString exportPath, CString defName);
	CString updateMeaList(CString filePath);	
	//bool dirExists(CString dir);	
};

VOID CALLBACK myUpdateTitle(HWND hwnd, UINT message, unsigned long long idTimer, DWORD dwTime);
VOID CALLBACK myCloseDlg(HWND hwnd, UINT message, unsigned long long idTimer, DWORD dwTime);


class SaveDialogWn : public CDialogEx
{
	//DECLARE_DYNAMIC(SaveDialogWn)

public:
	
	static const UINT ID_CLOSE_TIMER  = 9900;
	static const UINT ID_UPDATE_TIMER = 9901;

	double currTime  = 0.0;
	double closeTime = 0.0;
	double duration  = 10;  //AutoSave after ## secs
	
	SaveDialogWn(CWnd* pParent = NULL, CString exportPath = L"", CString suffix = L"", CString expName = L"", CString meaName = L"");
	// standard constructor
	
	virtual ~SaveDialogWn();

	//void OnTimer(UINT_PTR nIDEvent);

	// Dialog Data
	enum { IDD = IDD_SAVEDIAL };

	double saveTime = 120.0;    //as default save 120sec
	bool   bAutoClose = false;   //close after save

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	//
	
	CString exportPath = L"C:\\Labhub\\FOC48_DATA"; 
	myCListBox m_ExpList;
	//myCListBox *pm_ExpList;
	myCListBox m_MeaList;
	CTime now = CTime::GetCurrentTime();
	
	bool bOK          = false;
	bool bCancel      = false;
	bool bInteractive = true;
	bool bSaveAll     = false;   //as default save 120sec
	

	//Parameters parameters;
	

	CString SaveCaption = "Save ## sec";
	CString TitleCaption = "Save to ";

	CString expName;
	CString meaName;
	CString defName = L"20190101a";
	CString suffix    = L"";
	

	//CString DateStamp(){ return now.Format(_T("%Y%m%d")); };
	//
	CString Suffix(CString sIn = L"") { if (!sIn.IsEmpty()) suffix = sIn;  return suffix; };
	CString ExpName(CString sIn = L"") ;
	CString MeaName(CString sIn = L"") ;
	CString ExportPath(CString sIn = L"") { if (!sIn.IsEmpty()) exportPath = sIn;  return exportPath; };
	//
	CString FilePath(){ return exportPath   + "\\" + ExpName(); };
	CString filePath;
	CString FileRoot(){ return FilePath()   + "\\" + MeaName(); };
	CString fileRoot;

	double SaveTime(double dIn = 0.0);
	bool   AutoClose(int iIn=-1);
	//	
	void	SetSaveDuration(double dIn) { duration = dIn; };
	
	
	void updateExpList();
	void updateMeaList();

	//void UpdateWindow();

	//afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
	
	virtual BOOL OnInitDialog();
	//afx_msg void OnStnClickedMeasurement();
	//afx_msg void OnStnClickedExperiment();
	afx_msg void OnStnClickedSavepath();
	//afx_msg void OnShowWindow(BOOL bShow,	UINT nStatus);	
	afx_msg void OnBnClickedAddExperiment();
	afx_msg void OnBnClickedButtonMea();
	afx_msg void OnBnClickedAutoClose();
	
	afx_msg void OnBnClickedSave();
	afx_msg void OnBnClickedSaveAll();
	afx_msg void OnBnClickedSaveLast10();
	afx_msg void OnBnClickedSaveLast30();
	afx_msg void OnBnClickedSaveLast60();
	afx_msg void OnBnClickedSaveLast120();

	afx_msg void OnBnClickedEditMea();
	
	
	
	afx_msg void OnLbnSelchangeMeaList();
	
	afx_msg void OnLbnSelchangeExpList();

	//bool dirExists(CString dir);
	
	afx_msg int OnCompareItem(
		int nIDCtl,
		LPCOMPAREITEMSTRUCT lpCompareItemStruct);

	
	
};

#pragma once