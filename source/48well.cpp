//#include <afxwin.h>         // MFC core and standard components
//#include <afxext.h>         // MFC extensions
//#include <afxdialogex.h>
//#include <iostream>
//#include <sstream>

#include "48well.h"		// //#include <afxext.h>         // MFC extensions
#include "MainFrm.h"
#include "myHelper.h"	// dirExists
#include "SaveDialogWn.h"

#include "myPlot.h"

#ifdef _DEBUG
#include "DBG_print.h"
#endif


//using namespace std;
//using std::cout;
//using std::endl;
//
//#define _CRTDBG_MAP_ALLOC
//#include <stdlib.h>
//#include <crtdbg.h>


/////////////////////////////////////////////////////////////////////////////
// CDemoApp

BEGIN_MESSAGE_MAP(CDemoApp, CWinApp)
	ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
	ON_COMMAND(ID_FILE_SAVE, OnAppSave)
	ON_COMMAND(ID_START_MEASUREMENT, OnAppStartMeasurement)
	ON_COMMAND(ID_STOP_MEASUREMENT, OnAppStopMeasurement)
	ON_COMMAND(ID_CAMERA_START, OnAppStartCamera)
	ON_COMMAND(ID_CAMERA_STOP, OnAppStopCamera)
	
	//ON_COMMAND(ID_APP_EXIT, OnAppExit)
	//ON_COMMAND(ID_FILE_NEW, CWinApp::OnFileNew)
	//ON_COMMAND(ID_FILE_OPEN, CWinApp::OnFileOpen)
	// Standard print setup command
	//ON_COMMAND(ID_FILE_PRINT_SETUP, CWinApp::OnFilePrintSetup)	
	ON_COMMAND(ID_VIEW_ROIPREVIEW, OnRoiPreview)
	ON_COMMAND(SC_MAXIMIZE, OnMaximize)
	//WM_SYSCOMMAND(SC_MAXIMIZE, OnMaximize);
	//ON_WM_LBUTTONDOWN()
	//ON_WM_RBUTTONDOWN()
	//ON_WM_CLOSE()
	ON_COMMAND(ID_LIGHT_ON,         OnAppUvOn)
	ON_COMMAND(ID_LIGHT_OFF,        OnAppUvOff)
	ON_COMMAND(ID_LIGHT_FLASH,      OnAppUvFlash)
	ON_COMMAND(ID_STIMULATION_X_ON, OnAppXon)
	ON_COMMAND(ID_STIMULATION_X_OFF, OnAppXoff)
	ON_COMMAND(ID_STIMULATION_ON,   OnAppStimOn)
	ON_COMMAND(ID_STIMULATION_PERM, OnAppStimPerm)
	ON_COMMAND(ID_STIMULATION_OFF,  OnAppStimOff)
	ON_COMMAND(ID_TRIGGER_ON,       OnAppTrigOn)
	ON_COMMAND(ID_TRIGGER_OFF,      OnAppTrigOff)
	ON_COMMAND(ID_RR_500,			OnAppRR500)
	ON_COMMAND(ID_RR_32000,         OnAppRR32000)
	ON_COMMAND(ID_STIMULATION_N1,   OnAppN1)
	ON_COMMAND(ID_STIMULATION_N5,   OnAppN5)
	ON_COMMAND(ID_STIMULATION_N160, OnAppN160)
	ON_COMMAND(ID_STIMULATION_W1,   OnAppW1)
	ON_COMMAND(ID_RE_CAL,			OnReCal)
	ON_COMMAND(ID_VIEW_TAG,			OnViewTag)
	ON_COMMAND(ID_USE_NEW_MED,		OnNewMethod)
	ON_COMMAND(ID_VIEW_POLE,		OnDspPole)
	ON_COMMAND(ID_VIEW_NO_TRAP,		OnMouseTrap)
END_MESSAGE_MAP()



/////////////////////////////////////////////////////////////////////////////
// CDemoApp construction

CDemoApp::CDemoApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
	pMainFrame = NULL;
}



/////////////////////////////////////////////////////////////////////////////
// The one and only CDemoApp object

CDemoApp theApp;

CMyCmdLineInfo::CMyCmdLineInfo()
{
	bExpName  = FALSE;
	sExpName  = "";
	sParmFile = "";
}

CMyCmdLineInfo::~CMyCmdLineInfo()
{

}

//void CMyCmdLineInfo::ParseParam(LPCSTR pszParam, BOOL bFlag, BOOL bLast)
void CMyCmdLineInfo::ParseParam(const TCHAR* pszParam, BOOL bFlag, BOOL bLast)
{
	if (bFlag)
	{
		//if (!strnicmp("e:", pszParam, 2)) //Erster Parameter danach folgt noch was
		if (!_tcscmp(L"e", pszParam)) {
			bExpName = TRUE;
			sExpName = (pszParam + 2);
		}
		//if (!strnicmp("m:", pszParam, 2)) //Erster Parameter danach folgt noch was
		if (!_tcscmp(L"m", pszParam))
			sMeaName = (pszParam + 2);

	}
	CCommandLineInfo::ParseParam(pszParam, bFlag, bLast);
}


/////////////////////////////////////////////////////////////////////////////
// CDemoApp initialization

void myError(HWND hWnd, const char* lpFunction, int dw);

BOOL CDemoApp::InitInstance()
{
	
//#ifdef _AFXDLL
//	Enable3dControls();			// Call this when using MFC in a shared DLL
//#else
//	Enable3dControlsStatic();	// Call this when linking to MFC statically
//#endif

	CWinApp::InitInstance();

	CMyCmdLineInfo rCmdInfo; //CCommandLineInfo rCmdInfo;

	ParseCommandLine(rCmdInfo);

	//pMainFrame->parameters.setEnvironmentC("my2Out_xml.xml");
	//system("cmd /V:ON /C set FOC_OUTFILE=myNextTryToSet.xml ^&^& echo !FOC_OUTFILE!");	

	CString inParFile = rCmdInfo.m_strFileName;
	CString defParFile = _T("C:\\Labhub\\import\\myrPlate.xml");
	CString header     = "FOC48: Header Line";

	//rCmdInfo.m_nShellCommand;

	if (inParFile.IsEmpty()) inParFile = defParFile;
	if (! PathFileExists(inParFile))
	{
		MessageBox(NULL, inParFile, L"Parameter File does not exist", 1);
		return FALSE;
	}

	LoadStdProfileSettings();  // Load standard INI file options (including MRU)

	// create main Frame window
	//
	pMainFrame = new CMainFrame(this);
	
	// ugly way to import the second commandline Argument
	
	if (!pMainFrame->LoadFrame(IDR_MAINFRAME) || !pMainFrame->myDllTestLoad())
	{
		MessageBox(NULL, inParFile, L"Failed to load LoadFrame or my48WellFF_vF.dll", 1);
		delete pMainFrame;
		return FALSE;
	}

	// keep this line
	m_pMainWnd = pMainFrame;	// after this line, we do not need delete pMainFrame;

	pMainFrame->ShowWindow(SW_SHOWMAXIMIZED);	
	//pMainFrame->FindStimulationBoard();
	// The main window has been initialized, so show and update it.	
	pMainFrame->UpdateWindow();
	//HWND btn = GetDlgItem(pMainFrame->m_hWnd, ID_SAVE);

	//  Only after show, title can be seen
	CControlBar* m_cbar = pMainFrame->GetControlBar(0);

	header.Format(L"FOC48: Import Parameters from %ls", inParFile);
	pMainFrame->SetWindowText(header);
	//pMainFrame->myShowInStatusBar(header);


	if ( ! pMainFrame->parameters.read(inParFile)) {
		//
		//MessageBox(NULL, L"Failed to load Parameter File, continue with defaults", inParFile, MB_OK | MB_ICONWARNING);
		MessageBox(NULL, inParFile, L"Parameter File could not be read", 1);
		//exit(1);
		//delete(this);
		return FALSE;
	}
	
	if (rCmdInfo.bExpName) {
		pMainFrame->parameters.ExpName(rCmdInfo.sExpName);
		pMainFrame->expName=rCmdInfo.sExpName;
		pMainFrame->filePath = pMainFrame->parameters.FilePath();
		pMainFrame->bFilePathSet = true;
		if (! dirExists(pMainFrame->filePath) && ! CreateDirectory(pMainFrame->filePath, NULL))
		{
				MessageBox(NULL, pMainFrame->filePath, L"Failed to create",1);
				return FALSE;
		}
	}
	
	pMainFrame->importParameters();

#ifndef MY_NO_pylon


	if (pMainFrame->bSimulate)
		pMainFrame->checkFrameInterval = 30;

#else
	pMainFrame->checkFrameInterval = 57;
#endif

	//

#ifndef MY_NO_DB
	if (pMainFrame->parameters.UseDb()) {
		pMainFrame->SetWindowText(L"Initialize Database");
		//pMainFrame->Db.connect();
		//pMainFrame->Db.myTest();
		//pMainFrame->Db.getPersons();
	}
#endif

	//
	//connect to camera	
	//

	pMainFrame->checkFrameInterval = 57;
	
	pMainFrame->SetWindowText(L"Initialize Camera");

	if (!pMainFrame->InitCamera()){
		MessageBox(NULL, L"Camera Initialization error!", L"Failed to Initialize the Camera", 1 | 16); // MB_ICONERROR = 16
		return FALSE;
	}
	// initialize Stimulation board.
	// Important: the Camera should be running so we can check the UV Illumination

#ifndef MY_NO_USB
	pMainFrame->SetWindowText(L"Initialize Stimulation Board");	
	if (!pMainFrame->InitStimulationBoard()) {
		return MessageBox(0, L"Failed to Initialize Stimulation Board", L"Abort Execution", MB_OK);
		pMainFrame->OnClose();
		return FALSE;
	}
#endif

	pMainFrame->SetWindowText(L"Initialize Buffer and Plots");
	pMainFrame->InitBuffer();   //initialize Buffer after Camera since buffer size depends on fps
	printf("Init Plots\n"); 
	pMainFrame->InitPlots();
	printf("Init ROIs\n");
	pMainFrame->InitROIs();
	//
	if (pMainFrame->bAutoSave && ! pMainFrame->bFilePathSet)
		pMainFrame->SetSavePath();
		
	//printf("Set Timer\n"); 
	//pMainFrame->bMeasuring = true;

	pMainFrame->mySetTimer();
	//
	pMainFrame->SetWindowText(L"Start Measurement");
	pMainFrame->StartMeasurement();
	//
	pMainFrame->canSize = TRUE;
	//
	return TRUE;
}

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
		// No message handlers
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);	
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)

END_MESSAGE_MAP()

// App command to run the dialog
void CDemoApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}
void CDemoApp::OnAppSave()
{ 
	pMainFrame->Save(TRUE);
	//
	
}

void CDemoApp::OnAppUvOn()
{
	pMainFrame->StimulationBoard_WR("U");
}


void CDemoApp::OnAppUvOff()
{
	//pMainFrame->stimulationBoard.setUV(FALSE);
	pMainFrame->StimulationBoard_WR("u");
	
}

void CDemoApp::OnAppXoff()
{
	//pMainFrame->stimulationBoard.setUV(FALSE);
	pMainFrame->stimulationBoard.write("x\0", 2);
}

void CDemoApp::OnAppXon()
{
	//pMainFrame->stimulationBoard.setUV(FALSE);
	pMainFrame->stimulationBoard.write("X\0", 2);
}


void CDemoApp::OnAppUvFlash()
{
	//pMainFrame->stimulationBoard.setUV(TRUE, TRUE);
	pMainFrame->stimulationBoard.write("F\0", 2);
}

void CDemoApp::OnAppStimOn()
{
	pMainFrame->stimulationBoard.write("S\0", 2);
	//CWnd* wnd = GetMessageBar();//get pointer to status bar
	//wnd - &gt;
	//SetWindowText("Running...");
}

void CDemoApp::OnAppStimOff()
{
	pMainFrame->stimulationBoard.write("s\0", 2);
}

void CDemoApp::OnAppStimPerm()
{
	pMainFrame->stimulationBoard.write("P\0", 2);
}

void CDemoApp::OnAppTrigOn()
{
	//pMainFrame->stimulationBoard.setTrigger(TRUE, FALSE);
	pMainFrame->stimulationBoard.write("T\0", 2);
}

void CDemoApp::OnAppTrigOff()
{
	//pMainFrame->stimulationBoard.setTrigger(FALSE, FALSE);
	pMainFrame->stimulationBoard.write("t\0", 2);
}

void CDemoApp::OnAppRR500()
{
	//pMainFrame->stimulationBoard.setTrigger(FALSE, FALSE);
	pMainFrame->stimulationBoard.set("r" , 500);
}

void CDemoApp::OnAppRR32000()
{
	//pMainFrame->stimulationBoard.setTrigger(FALSE, FALSE);
	pMainFrame->stimulationBoard.set("r", 32000);
}


void CDemoApp::OnAppN1()
{
	//pMainFrame->stimulationBoard.setTrigger(FALSE, FALSE);
	pMainFrame->stimulationBoard.set("N", 1);
}

void CDemoApp::OnAppN5()
{
	//pMainFrame->stimulationBoard.setTrigger(FALSE, FALSE);
	pMainFrame->stimulationBoard.set("N", 5);
}

void CDemoApp::OnAppN160()
{
	//pMainFrame->stimulationBoard.setTrigger(FALSE, FALSE);
	pMainFrame->stimulationBoard.set("N", 160);
}

void CDemoApp::OnAppW1()
{
	//pMainFrame->stimulationBoard.setTrigger(FALSE, FALSE);
	pMainFrame->stimulationBoard.set("W", 1);
}

void CDemoApp::OnAppW2()
{
	//pMainFrame->stimulationBoard.setTrigger(FALSE, FALSE);
	pMainFrame->stimulationBoard.set("W", 2);
}

void CDemoApp::OnAppW5()
{
	//pMainFrame->stimulationBoard.setTrigger(FALSE, FALSE);
	pMainFrame->stimulationBoard.set("W", 5);
}



void CDemoApp::OnAppSendUSB(const char* sCmd)
{
    pMainFrame->stimulationBoard.write(sCmd);
}




void CDemoApp::OnAppStartMeasurement()
{
	pMainFrame->StartMeasurement();
}

void CDemoApp::OnAppStopMeasurement()
{
	pMainFrame->StopMeasurement();
}

void CDemoApp::OnAppStartCamera()
{
	pMainFrame->StartCamera();
}

void CDemoApp::OnAppStopCamera()
{
	pMainFrame->StopCamera();
}


void CDemoApp::OnMaximize()
{	
	pMainFrame->ShowWindow(SW_SHOWMAXIMIZED);
	CRect Rect0;
	pMainFrame->GetClientRect(Rect0);	
	//pMainFrame->canSize = true;
	pMainFrame->OnSize(SIZE_MAXIMIZED, Rect0.Width() - 1, Rect0.Height() - 1);	
	//PostMessage(pMainFrame->m_hWnd, WM_SIZE,)
}

void CDemoApp::OnRoiPreview()
{
	pMainFrame->bDrawROIs = pMainFrame->bDrawROIs ? FALSE :TRUE;
	int myShow = pMainFrame->bDrawROIs ? SW_HIDE : SW_SHOW;
	for (int i = 0; i < pMainFrame->nWell; i++)		pMainFrame->m_Plot[i].ShowWindow(myShow);	
	//
	if (pMainFrame != NULL){
#ifndef MY_NO_pylon
		if (pMainFrame->bDrawROIs){
			if (pMainFrame->h_RoiPreview == NULL){
				pMainFrame->h_RoiPreview = new CPylonImageWindow();
				pMainFrame->h_RoiPreview->Create(1);
				pMainFrame->h_RoiPreview->Show();
			}			
		}
		else{
			if (pMainFrame->h_RoiPreview != NULL){
				pMainFrame->h_RoiPreview->Close();
				delete pMainFrame->h_RoiPreview;
			}
			pMainFrame->h_RoiPreview = NULL;
		}
#endif
	}
	
}

void CDemoApp::OnAppNew()
{
	CWinApp::OnFileNew();
}

void CDemoApp::OnReCal()
{
	CMainFrame *pMain = dynamic_cast<CMainFrame *>(theApp.m_pMainWnd);
	if (pMain != NULL)
	{
		pMain->myCal_0();
	}
}
void CDemoApp::OnViewTag()
{
	CMainFrame *pMain = dynamic_cast<CMainFrame *>(theApp.m_pMainWnd);
	if (pMain != NULL)
	{
		pMain->myViewTagToggle();
	}
}
void CDemoApp::OnNewMethod()
{
	CMainFrame *pMain = dynamic_cast<CMainFrame *>(theApp.m_pMainWnd);
	if (pMain != NULL)
	{
		pMain->myNewMethodToggle();
	}
}
void CDemoApp::OnDspPole()
{
	CMainFrame *pMain = dynamic_cast<CMainFrame *>(theApp.m_pMainWnd);
	if (pMain != NULL)
	{
		pMain->myDspPoleToggle();
	}
}
void CDemoApp::OnMouseTrap()
{
	CMainFrame *pMain = dynamic_cast<CMainFrame *>(theApp.m_pMainWnd);
	if (pMain != NULL)
	{
		pMain->myMouseTrapToggle();
	}
}
/////////////////////////////////////////////////////////////////////////////
// CDemoApp commands
