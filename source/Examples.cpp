
#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions
#include <afxdialogex.h>


class CMFCListViewApp : public CWinApp
{
public:
	CMFCListViewApp();
	~CMFCListViewApp();
	virtual BOOL InitInstance();
};

class CMainFrame : public CFrameWnd
{
public:
	CMainFrame();
	~CMainFrame();
};


BOOL CMFCListViewApp::InitInstance()
{
	//AfxSetAmbientActCtx(FALSE);
	// Remainder of function definition omitted.

	CWinApp::InitInstance();

	// Initialize OLE libraries
	//if (!AfxOleInit())
	//{
	//	AfxMessageBox(_T("OleInit failed."));
	//	return FALSE;
	//}

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	// of your final executable, you should remove from the following
	// the specific initialization routines you do not need
	// Change the registry key under which our settings are stored
	// TODO: You should modify this string to be something appropriate
	// such as the name of your company or organization
	
	//SetRegistryKey(_T("Local AppWizard-Generated Applications"));
	
	LoadStdProfileSettings(4);  // Load standard INI file options (including MRU)
	
	// Register the application's document templates.  Document templates
	//  serve as the connection between documents, frame windows and views

	//CMultiDocTemplate* pDocTemplate;
	//pDocTemplate = new CMultiDocTemplate(IDR_MFCListViewTYPE,
	//	RUNTIME_CLASS(CMFCListViewDoc),
	//	RUNTIME_CLASS(CChildFrame), // custom MDI child frame
	//	RUNTIME_CLASS(CMyListView));
	//if (!pDocTemplate)
	//	return FALSE;
	//AddDocTemplate(pDocTemplate);

	// create main MDI Frame window
	CMainFrame* pMainFrame = new CMainFrame;
	if (!pMainFrame || !pMainFrame->LoadFrame(IDR_MAINFRAME))
	{
		delete pMainFrame;
		return FALSE;
	}
	m_pMainWnd = pMainFrame;
	// call DragAcceptFiles only if there's a suffix
	//  In an MDI app, this should occur immediately after setting m_pMainWnd


	// Parse command line for standard shell commands, DDE, file open
	CCommandLineInfo cmdInfo;
	ParseCommandLine(cmdInfo);


	// Dispatch commands specified on the command line.  Will return FALSE if
	// app was launched with /RegServer, /Register, /Unregserver or /Unregister.
	if (!ProcessShellCommand(cmdInfo))
		return FALSE;
	// The main window has been initialized, so show and update it
	pMainFrame->ShowWindow(m_nCmdShow);
	pMainFrame->UpdateWindow();

	return TRUE;
}


//MK_CONTROL   Set if the CTRL key is down.
//MK_LBUTTON   Set if the left mouse button is down.
//MK_MBUTTON   Set if the middle mouse button is down.
//MK_RBUTTON   Set if the right mouse button is down.
//MK_SHIFT   Set if the SHIFT key is down.

//#define MK_LBUTTON          0x0001
//#define MK_RBUTTON          0x0002
//#define MK_SHIFT            0x0004
//#define MK_CONTROL          0x0008
//#define MK_MBUTTON          0x0010
//#if(_WIN32_WINNT >= 0x0500)
//#define MK_XBUTTON1         0x0020
//#define MK_XBUTTON2         0x0040

//fx_msg void CMainFrame::OnLButtonDown(UINT nFlags, CPoint point)


//CWnd::OnClose
//The framework calls this member function as a signal that the CWnd or an application is to terminate.
//Remarks
//The default implementation calls DestroyWindow.
