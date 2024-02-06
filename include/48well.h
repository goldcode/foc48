
#if !defined(AFX_DEMO_H__BB278705_A140_11D1_BEB4_006008918F1C__INCLUDED_)
#define AFX_DEMO_H__BB278705_A140_11D1_BEB4_006008918F1C__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000


//#include "../res/resource.h"       // main symbols
//#include "MainFrm.h"
//#include "Trace.h"
//
//#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions

class CSampleCameraEventHandler;


class CMyCmdLineInfo : public CCommandLineInfo
{
public:
	CMyCmdLineInfo();
	bool bExpName=false;
	CString sExpName = "";
	CString sMeaName = "";
	CString sParmFile = "";
	virtual ~CMyCmdLineInfo();
	//virtual void ParseParam(LPCSTR pszParam, BOOL bFlag, BOOL bLast);
	virtual void ParseParam(const TCHAR* pszParam, BOOL bFlag, BOOL bLast); 
	//virtual void ParseParam(const TCHAR* pszParam, BOOL bFlag, BOOL bLast);
};

class CMainFrame;

class CDemoApp : public CWinApp
{
public:
	CDemoApp();
	~CDemoApp() {
		int i = 1;		
	};

public:
	CMainFrame* pMainFrame; 
	
	virtual BOOL InitInstance();
	
	afx_msg void OnAppNew();
	afx_msg void OnRoiPreview();
	afx_msg void OnAppSave();
	afx_msg void OnAppStartMeasurement();
	afx_msg void OnAppStopMeasurement();
	afx_msg void OnAppStartCamera();
	afx_msg void OnAppStopCamera(); 
	afx_msg void OnAppAbout();
	afx_msg void OnMaximize();
	//
	afx_msg void OnAppUvOn();
	afx_msg void OnAppUvOff();
	afx_msg void OnAppXoff();
	afx_msg void OnAppXon();
	afx_msg void OnAppUvFlash();
	afx_msg void OnAppStimOn();
	afx_msg void OnAppStimOff();
	afx_msg void OnAppStimPerm();
	afx_msg void OnAppTrigOn();
	afx_msg void OnAppTrigOff();
	afx_msg void OnAppRR500();
	afx_msg void OnAppRR32000();
	afx_msg void OnAppN1(); 
	afx_msg void OnAppN5();
	afx_msg void OnAppN160();
	afx_msg void OnAppW1();
	afx_msg void OnAppW2();
	afx_msg void OnAppW5();
	afx_msg void OnAppSendUSB(const char* sCmd);

	afx_msg void OnReCal();
	afx_msg void OnViewTag();
	afx_msg void OnNewMethod();
	afx_msg void OnDspPole();
	afx_msg void OnMouseTrap();

	DECLARE_MESSAGE_MAP()
private:
	
};


#endif
