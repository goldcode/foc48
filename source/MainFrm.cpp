// MainFrm.cpp : implementation of the CMainFrame class
//

//#include "stdafx.h"
//#include <afx.h>


//#include <iostream>
//#include <fstream>
//#include <afxwin.h>         // MFC core and standard components
//#include <afxext.h>         // MFC extensions
//#include <afxdialogex.h>

#include "MainFrm.h"
#include "48well.h"		// //#include <afxext.h>         // MFC extensions
#include "SaveDialogWn.h"
//#include "SerialPort.h"

// #include "comList.h"
//#include "USBstimulationBoard.h"

#include "myHelper.h"
#include "DBG_print.h"

#include "myPlot.h"
#include "trace.h"

#include "myEHM_FIT_M_01.h"

//#include "mySerial.h"
//#include "SerialClass.h"
//#include "..\libXML\libXML\libXML.h"
//#pragma comment(lib,"..\\libXML\\release\\libXMLw32.lib")

#ifndef MY_NO_pylon
using namespace GenApi;
using namespace Basler_UsbCameraParams;
#endif
// Namespace for using cout.
using namespace std;

//using std::cout;
//using std::endl;

bool bCameraEvent;
myEHM_FIT* eFit = NULL;

#ifndef MY_NO_pylon

//CSampleCameraEventHandler* pEventHandler;

//Enumeration used for distinguishing different events.
enum MyEvents
{
	eMyExposureEndEvent = 100,
	eMyEventOverrunEvent = 200
};

class CSampleCameraEventHandler : public CBaslerUsbCameraEventHandler
{
public:
	// Only very short processing tasks should be performed by this method. Otherwise, the event notification will block the
	// processing of images.
	virtual void OnCameraEvent(CBaslerUsbInstantCamera & camera, intptr_t userProvidedId, GenApi::INode* /* pNode */)
	{
		switch (userProvidedId)
		{
		case eMyExposureEndEvent: // Exposure End event
			//cout << "Exposure End event. FrameID: " << m_camera->EventExposureEndFrameID.GetValue() << " Timestamp: " << m_camera->EventExposureEndTimestamp.GetValue() << std::endl << std::endl;
			break;
		case eMyEventOverrunEvent:  // Event Overrun event, shall be  EventOverrunEventFrameID, EventOverrunEventTimestamp
			//cout << "Event Overrun event. FrameID: " << camera.EventFrameStartOvertriggerFrameID.GetValue() << " Timestamp: " << camera.EventFrameStartOvertriggerTimestamp.GetValue() << std::endl << std::endl;
			break;
		}
	}
};

//Example of an image event handler.
class CSampleImageEventHandler : public CImageEventHandler
{
public:
	virtual void OnImageGrabbed(CInstantCamera& camera, const CGrabResultPtr& ptrGrabResult)
	{
		CBaslerUsbInstantCamera *pCam = dynamic_cast<CBaslerUsbInstantCamera *>(&camera);
		if (pCam != NULL)
		{
			//printf("ChunkTimestamp=%d,\tEventExposureEndTimestamp=%d\n", (unsigned long long)(pCam->ChunkTimestamp.GetValue(false, true)), (unsigned long long)(pCam->EventExposureEndTimestamp.GetValue(false, true)));
			if (bCameraEvent)
				printf("EventExposureEndFrameID=%d\n", (unsigned long long)(pCam->EventExposureEndFrameID.GetValue()));
			else
				printf("CBaslerUsbInstantCamera OnImageGrabbed called.\n");
		}

	}
};
#else

#include "my_ssAVI.h"
myMEMFR	*mFF2 = NULL;

#endif
/////////////////////////////////////////////////////////////////////////////resize
// CMainFrame

IMPLEMENT_DYNAMIC(CMainFrame, CFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWnd)
	ON_WM_SIZE()	
	//ON_WM_TIMER()
	ON_WM_CREATE()
	ON_WM_LBUTTONDOWN()
	ON_WM_RBUTTONDOWN()
	ON_WM_MOUSEMOVE()
	ON_WM_CLOSE()
	ON_WM_SYSCOMMAND()
	//ON_UPDATE_COMMAND_UI(ID_INDICATOR_SCRL, &CMainFrame::OnUpdatePane)
	//ON_WM_PAINT()
	//ON_WM_ERASEBKGND()
END_MESSAGE_MAP()

static UINT indicators[] =
{
	ID_SEPARATOR,           // status line indicator
	ID_INDICATOR_CAPS,
	ID_INDICATOR_NUM,
	ID_INDICATOR_SCRL,
};

/////////////////////////////////////////////////////////////////////////////
// CMainFrame construction/destruction
CMainFrame::CMainFrame(CWinApp *p)
{
	pParent = p;
	// TODO: add member initialization code here
#ifndef MY_NO_pylon
	m_camera = NULL;
	h_RoiPreview = NULL;

#else
	if (mFF2 == NULL)
		mFF2 = new myMEMFR("C:\\labhub\\FIT_TEST\\MY_MEM_DATA.tmp");
#endif
	nWell = nRowCount*nColCount;
	m_trace=NULL;
	m_Plot=NULL;
	roiPixOffset=NULL;  //array of 96
	roiXpos=NULL;   //array of roiPix
	RowSumR=NULL;
	if (eFit == NULL)
		eFit = new myEHM_FIT;
}
CMainFrame::~CMainFrame()
{
#ifndef MY_NO_pylon
	if (m_camera != NULL) {
		if (m_camera->IsGrabbing())
			m_camera->StopGrabbing();
	}
#endif
	delete[] m_trace;
	delete[] m_Plot;
	delete[] roiPixOffset;  //array of 96
	delete[] roiXpos;   //array of roiPix
	delete[] RowSumR;
	//SetStimulationBoardStatus('b');
#ifndef MY_NO_pylon
	if (m_camera != NULL) {
		if (m_camera->IsOpen())
			m_camera->Close();
		try {
			delete m_camera;
		}
		catch (...) {}
	}
	PylonTerminate();
#endif
	if (eFit != NULL) {
		delete eFit;
		eFit = NULL;
	}
	//delete[] wRowSumR;
	//delete[] RowSumL;
	//delete[] wRowSumL;
}

ULONG CMainFrame::GetGestureStatus(CPoint /*ptTouch*/) { return 0; }

void CMainFrame::StartCamera()
{
#ifndef MY_NO_pylon
	if (!m_camera->IsOpen())
		m_camera->Open();
	//
	if (!m_camera->IsGrabbing())
		m_camera->StartGrabbing();
	//
#endif
	stimulationBoard.setUV(1, bUvFlash); 
	bGrabbing = true;

}

void CMainFrame::StimulationBoard_WR(const char * cIn)
{
	CStringA strShow; 
	strShow.Format("Sending %s to StimulationBoard ...", cIn);
	m_wndStatusBar.SetPaneText(m_wndStatusBar.CommandToIndex(ID_SEPARATOR), CString(strShow));
	m_wndStatusBar.UpdateWindow(); 
	//
	char * cOut = stimulationBoard.write_read(cIn);
	
	strShow.Format("Sending %s to StimulationBoard ... returns ", cIn, cOut);
	m_wndStatusBar.SetPaneText(m_wndStatusBar.CommandToIndex(ID_SEPARATOR), CString(strShow));
	m_wndStatusBar.UpdateWindow();
}

void __fastcall CMainFrame::myShowInStatusBar(const wchar_t *pStr, const int nID)
{	// nID = 0

	//m_wndStatusBar.SetPaneText(m_wndStatusBar.CommandToIndex(ID_SEPARATOR), ss);
	//m_wndStatusBar.UpdateWindow();
	//int n = m_wndStatusBar.CommandToIndex(ID_INDICATOR_SCRL);
	m_wndStatusBar.GetStatusBarCtrl().SetText(pStr, nID, SBT_NOBORDERS); //m_wndStatusBar.SetPaneText(n, pStr); //(nID == 0) ? SBT_NOBORDERS : SBT_OWNERDRAW
}

bool __fastcall CMainFrame::myDllTestLoad(void)
{
#ifndef MY_NO_pylon
	if (eFit == NULL)return false;
#else
	if (eFit == NULL || mFF2 == NULL)return false;
#endif
	return eFit->myDllTestLoad();
}

bool __fastcall CMainFrame::mySetMEMFile(void)
{
	if (eFit == NULL)return false;
	nFrameAddIndex = 0;
#ifndef USE_MY_MEM_FILE
	return true;
#else
	return eFit->mySetMEMFile((int)imageWidth, (int)imageHeight); // 4G
#endif	//USE_MY_MEM_FILE
}

bool __fastcall CMainFrame::mySetMEMData(const char *pBuf)
{
	if (eFit == NULL)return false;
#ifndef USE_MY_MEM_FILE
	return true;
#else
	return eFit->myAddData(nFrameAddIndex++, pBuf);
#endif	//USE_MY_MEM_FILE
}

void __fastcall CMainFrame::mySetMEMUser(void)
{
#ifndef USE_MY_MEM_FILE
	return;
#else
	if (eFit != NULL) {
		int n[6];
		n[0] = xGrid; n[1] = yGrid; n[2] = roiCols; n[3] = roiRows; n[4] = 0; n[5] = 0;
		eFit->mySetUser(n, 6);
	}
	return;
#endif	//USE_MY_MEM_FILE
}

// enable the next incoming frame trigger the recalculating routing
void __fastcall CMainFrame::myCal_0(void)
{
	if ((mskEHM & MY_CAL_MASK) == 0)
	{
		mskEHM |= MY_CAL_MASK;
	}
}
void __fastcall CMainFrame::myCal_nSSB(int *nSSB)
{
	// ssB=[info.nRev2(6),info.nRev2(5)*2,info.nRev2(4)-info.nRev2(6),info.nRev2(3)-info.nRev2(5)*2,0,  0];  % 0 based
	nSSB[0] = roiCols * 2; nSSB[1] = roiRows; nSSB[2] = xGrid - nSSB[0]; nSSB[3] = yGrid - nSSB[1];	nSSB[4] = 0; nSSB[5] = 0;
}
// real recalculating routing
void __fastcall CMainFrame::myCal_1(void)
{
	if (eFit != NULL)
	{
		myCal_nSSB(nSSB_0);
		const unsigned char *pucA;
		int nAdimX, nAdimY;
#ifndef MY_NO_pylon
		pucA = (const unsigned char *)m_image.GetBuffer();
		nAdimX = imageWidth;
		nAdimY = imageHeight;
#else
		pucA = (const unsigned char*)mFF2->pMEM;
		nAdimX = mFF2->GetWidth();
		nAdimY = mFF2->GetHeight();
#endif
		eFit->myINIT_48(pucA, nAdimX, nAdimY, nSSB_0);
	}
}

int __fastcall CMainFrame::myToggle_Ex(int &mskVal, const unsigned int nID, const int nMSK)
{
	//#define MF_UNCHECKED        0x00000000L
	//#define MF_CHECKED          0x00000008L
	//#define MF_USECHECKBITMAPS  0x00000200L
	CMenu *pMenu = GetMenu();
	unsigned int unState = pMenu->GetMenuState(nID, MF_BYCOMMAND); //#define MF_BYCOMMAND        0x00000000L
	if (unState != 0xFFFFFFFF){
		if (unState & MF_CHECKED){
			pMenu->CheckMenuItem(nID, MF_UNCHECKED | MF_BYCOMMAND);
			mskVal &= ~nMSK;	// clear mask
		}
		else{
			pMenu->CheckMenuItem(nID, MF_CHECKED | MF_BYCOMMAND);
			mskVal |= nMSK;	// set mask
		}
	}
	return (mskVal & nMSK); // return mask bit, if return != 0 means, we need OBJ
}

// in the ROI view, display the block from the image recognition
int __fastcall CMainFrame::myViewTagToggle(void)
{
	return myToggle_Ex(mskEHM, ID_VIEW_TAG, MY_SHOW_OBJ_KASK);
}
int __fastcall CMainFrame::myNewMethodToggle(void)
{
	if (eFit == NULL)
		return 0;
	int i = myToggle_Ex(mskEHM, ID_USE_NEW_MED, MY_NEW_METHOD_MASK);
	if ((mskEHM & MY_NEW_METHOD_MASK) == 0)
		eFit->mySetThreadSleep(true);
	else
		eFit->mySetThreadSleep();
	return i;
}
int __fastcall CMainFrame::myDspPoleToggle(void)
{
	mskEHM &= ~MY_DSP_POLE_OK;
	return myToggle_Ex(mskEHM, ID_VIEW_POLE, MY_DSP_POLE_ENABLE);
}
int __fastcall CMainFrame::myMouseTrapToggle(void)
{
	return myToggle_Ex(mskEHM, ID_VIEW_NO_TRAP, MY_MOUSE_TRAP_EX);
}
void CMainFrame::StopCamera() {
	//
	printf("Close Camera\n");
#ifndef MY_NO_pylon	
	if (m_camera->IsOpen())
		m_camera->Close();
#endif
	// first stop camera so that last image in buffer is stil lit
	stimulationBoard.setUV(0);
	bGrabbing = false;
}
void __fastcall  CMainFrame::myPreSetMenu(const int mskEHM_in)
{
	while (mskEHM_in != 0)
	{
		if ((mskEHM_in & MY_CAL_MASK) != 0)
		{
			myCal_0();	// set mask
		}
		if ((mskEHM_in & MY_SHOW_OBJ_KASK) != 0)
		{
			myViewTagToggle(); // will do a toggle
		}
		if ((mskEHM_in & MY_NEW_METHOD_MASK) != 0)
		{
			myNewMethodToggle(); // will do a toggle
		}
		if ((mskEHM_in & MY_DSP_POLE_ENABLE) != 0)
		{
			myDspPoleToggle(); // will do a toggle
		}	
		if ((mskEHM_in & MY_MOUSE_TRAP_EX) != 0)
		{
			myMouseTrapToggle(); // will do a toggle
		}
		break;
	}
}

void CMainFrame::StartMeasurement()
{	
	//
	resetBuffer();   // remove this line will solve the HEAP error, Bao
	//
	iFrame = 0;
	//stimulationBoard.setUV(1);	// no need, StartCamera() has setUV
	//m_camera->AcquisitionMode.SetValue(AcquisitionMode_Continuous);
	//m_camera->AcquisitionStart.Execute();
	StartCamera(); // has -> stimulationBoard.setUV(1, bUvFlash); 
	//
	bMeasuring = true;
	measurementStartTime = (double) clock() / CLOCKS_PER_SEC;
	// Add 10 Second to stop time. Save will keep the last measurementDuration seconds and noise from start will be cut
	SetTimer(measurementStopTimer, (int) (measurementDuration+1) * 1000, myStopMeasurement );
}

void CMainFrame::StopMeasurement()
{
	//lights Off
	
	bMeasuring = false;
	//
	measurementStartTime = measurementStartTime + measurementInterval;
	//
	StopCamera();
	//	m_camera->Close();
	//
	printf("ShutDown UV\n");
	//setTimer
	stimulationBoard.setUV(0);
	//
	//SetStimulationBoardStatus(L"b");
	KillTimer(measurementStopTimer);
    //
	if (bAutoSave)
		Save(FALSE);
	//
	if (iMeasurement >= nMeasurement)
		OnClose();
	else
		iMeasurement = iMeasurement + 1;
	//
	//
}

void CMainFrame::mySetTimer()
{
	SetTimer(checkFrameTimer,		checkFrameInterval,					myCheckFrame);
	SetTimer(updateScreenTimer,		(int)updateScreenInterval,			myUpdateScreen);
	SetTimer(measurementStartTimer, (int)(measurementInterval * 1000),	myStartMeasurement);
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	//// create a view to occupy the client area of the frame
	//if (!m_wndView.Create(NULL, NULL, AFX_WS_DEFAULT_VIEW, CRect(0, 0, 0, 0), this, AFX_IDW_PANE_FIRST, NULL))
	//{
	//	TRACE0("Failed to create view window\n");
	//	return -1;
	//}

 //   //Create ToolBar
	//if (!m_wndToolBar.Create(this) ||
	//	!m_wndToolBar.LoadToolBar(IDR_MAINFRAME))
	//{
	//	TRACE0("Failed to create toolbar\n");
	//	return -1;      // fail to create
	//}

	if (!m_wndStatusBar.Create(this))
	{
		TRACE0("Failed to create status bar\n");
		return -1;      // fail to create
	}
	m_wndStatusBar.SetIndicators(indicators, sizeof(indicators) / sizeof(UINT));

	unsigned int nID, nStyle;
	int nWidth;
	for (int i = 1; i < 3; i++){
		m_wndStatusBar.GetPaneInfo(i, nID, nStyle, nWidth);
		nWidth = 57;//nWidth *= 2;
		m_wndStatusBar.SetPaneInfo(i, nID, nStyle, nWidth);
	}
	//int timerID = SetTimer(MY_TIMER_UPDATE_ID, MY_TIMER_UPDATE_LAG, myTimerUpdate);
	return 0;
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if (!CFrameWnd::PreCreateWindow(cs))
		return FALSE;
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	//cs.dwExStyle &= ~WS_EX_CLIENTEDGE;
	//cs.lpszClass = AfxRegisterWndClass(0);
	return TRUE;
}

//#define vTimer2_TICK	48

//MK_CONTROL Set if the CTRL key is down.
//MK_LBUTTON Set if the left mouse button is down.
//MK_MBUTTON Set if the middle mouse button is down.
//MK_RBUTTON Set if the right mouse button is down.
//MK_SHIFT Set if the SHIFT key is down.

afx_msg void CMainFrame::OnLButtonDown(UINT nFlags, CPoint point)
{
	bDrawROIs = bDrawROIs ? FALSE : TRUE;
	int myShow = bDrawROIs ? SW_HIDE : SW_SHOW;

	if (myShow)
		drawROIs(true);	// this will clear the right part of the image
	
	for (int i = 0; i < nWell; i++)	m_Plot[i].ShowWindow(myShow);

	if (!myShow){
		drawROIs();       // here we draw ROI, this will not get any delay!
		if (nDrawROIs==0)
			nDrawROIs++; // skip the next timer event based update
	}

	if ((mskEHM & MY_MOUSE_TRAP_EX) == 0)
	{
		if ((int)nFlags == -2)   // 0xfffffffe
			SetCapture();
		else
			::ReleaseCapture();
	}
	//CFrameWnd::OnLButtonDown(nFlags, point);
}

#define MY_LButtonDown		0x01
#define MY_RButtonDown		0x02
#define MY_MButtonDown		0x010
wchar_t * __fastcall myMOUSEKEY(int nFlags, wchar_t *wSTin = NULL)
{
	wchar_t wST[10] = { L'L', L' ', L' ', L'M', L' ', L' ', L'R', L' ', L' ', 0 };
	//                    0     1     2     3     4     5     6     7     9
	if ((nFlags&MY_LButtonDown) != 0)
		wST[1] = L'*';
	if ((nFlags&MY_MButtonDown) != 0)
		wST[4] = L'*';
	if ((nFlags&MY_RButtonDown) != 0)
		wST[7] = L'*';
	wchar_t *ww;
	if (wSTin != NULL){
		memcpy(wSTin, wST, sizeof(wST));
		ww = wSTin;
	}
	else wSTin = wST;
	return ww;
}
#define	MY_DSP_L1		170
#define	MY_DSP_W		20
#define	MY_DSP_L2		(MY_DSP_L1 - MY_DSP_W)

afx_msg void CMainFrame::OnMouseMove(UINT nFlags, CPoint point) 
{
	if (bDrawROIs) {
		// during move, display the image co-ordinate
		wchar_t wST[MY_DSP_L1];
		swprintf_s(wST, MY_DSP_L1 - 1, L"OnMouseMove: nFlags=0x%08x, x=%d,y=%d %ls", nFlags, point.x, point.y, myMOUSEKEY(nFlags, wST + MY_DSP_L2));
		m_wndStatusBar.GetStatusBarCtrl().SetText(wST, 0, SBT_NOBORDERS);

		CFrameWnd::OnMouseMove(nFlags, point);
		// 5,5
		RECT irr;
		GetWindowRect(&irr);
		// remap to image
		//printf("%d,%d\n",point.x, point.y);
	}
}

bool CMainFrame::InitROIs()
{
	//the camera records only the net ROIs
	//so the first Pixel points to the lop left corner of ROI[1] 
	//and the last Pixel to  the bottom wight corner of ROI[48]
	
	//imageHeight = m_camera->Height();
	//imageWidth  = m_camera->Width();

	ratioRows = parameters.RatioRows();
	ratioCols = parameters.RatioCols();

	yGrid = (int)(((double)imageHeight) / ((double)nRowCount - 1 + ratioRows));
	xGrid = (int)(((double)imageWidth) / ((double)nColCount - 1 + ratioCols));

	pix2mm = xGrid / xGrid_mm;

	printf(" imageWidth X imageHeight = %d X %d\n", imageWidth, imageHeight); 
	printf("      xGrid X yGrid       = %.1f X %.1f\n", xGrid, yGrid);

	roiRows = (int)(yGrid * ratioRows);
	// roiCols is divided by 2 bacause we separate left and right Poles
	roiCols = (int)(xGrid * ratioCols / 2);
	//
	nRoiPix = roiRows * roiCols;
	//
	roiPixList = new int[nRoiPix];                //array of roiPix
	roiXpos = new int[nRoiPix];
	//
	printf("roiRows   X roiCols   = %d X %d\n", roiRows, roiCols);
	printf("nRowCount X nColCount = %d X %d\n", nRowCount, nColCount);
	//printf("dRowCount X dColCount = %.2f X %.2f\n", dRowCount, dColCount);
	//
	// make a list of offsets per ROI
	//The referenc is the top left pixel
	//
	int k;
	//int xPosR, xPosL;
	//
	//
	k = 0;
	for (int i = 0; i < roiRows; i++) {
		//xPosL = roiCols;
		//xPosR = 0;
		for (int j = 0; j < roiCols; j++) {
			roiPixList[k] = i * (int)imageWidth + j;
			roiXpos[k] = j;
			k++;
		}
	}
	//
	// make a list of first pointer for 48 roi pairs
	//
	nWell = nRowCount*nColCount;
	roiPixOffset = new int[nWell];
	roiPixOffsetX = new int[nColCount];
	roiPixOffsetY = new int[nRowCount];
	//
	RowSumR = new int[nWell*4];
	wRowSumR = RowSumR + nWell;
	RowSumL = wRowSumR + nWell;
	wRowSumL = RowSumL + nWell;

	printf("roiCols X roiRows = %d X %d\n", roiCols, roiRows);
	// The list should contain the first top left pixel of each ROI
	//int	firstRoiPix = (int) yGrid * (1.0 - ratioRows)/2 +  xGrid * (1.0 - ratioCols) / 2;
	//
	//firstRoiPix -= 222;
	//
	for (int i = 0; i < nRowCount; i++) {
		roiPixOffsetY[i] =  yGrid * (int)imageWidth * i;
	}

	for (int j = 0; j < nColCount; j++) {
		roiPixOffsetX[j] = (int)xGrid * j;
	}

	k = 0;
	// before change, RAW and COL are not correct, now it is correct //AAA
	//for (int j = 0; j < nColCount; j++) {
	//for (int i = 0; i < nRowCount; i++) {
	sWell.RemoveAll();
	char *P ="ABCDEFGH" ;
	char *N = "123456";
	char buf[3] = { 0 };
	for (int i = 0; i < nRowCount; i++) {
		for (int j = 0; j < nColCount; j++) {
			roiPixOffset[k] = roiPixOffsetX[j] + roiPixOffsetY[i]; nWellXY[k][0] = roiPixOffsetX[j]; nWellXY[k][1] = yGrid * i;
			buf[0] = P[i % 8];
			buf[1] = N[j % 6]; 
			sWell.Add(CString(buf));
			k++;
		}
	}
	//
	if (mySetMEMFile()) {
		mySetMEMUser();
	}
	return true;
}

void CMainFrame::OnClose()
{
	//m_camera->AcquisitionStop.Execute();
	//delete(m_camera);
	stimulationBoard.setUV(0);	
	//SetStimulationBoardStatus(L"b");
	CFrameWnd::OnClose();
}

bool CMainFrame::SetSavePath()
{
	SaveDialogWn saveDlg(NULL);	
	
	saveDlg.ExportPath(parameters.ExportPath());
	saveDlg.SaveCaption = L"Save Path";
	//	
	if ( saveDlg.DoModal() == IDOK) {
		expName = saveDlg.ExpName();
		filePath = saveDlg.FilePath();
		bFilePathSet = true;
	}
	//
	return true;
}

void myError(HWND hWnd, const char *lpFunction, int dw)
{
	// Retrieve the system error message for the last-error code

	void *lpMsgBuf;
	void *lpDisplayBuf;

	FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_SYSTEM|FORMAT_MESSAGE_IGNORE_INSERTS,NULL,dw,MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),(LPSTR)&lpMsgBuf,0, NULL);

	int n = (int)(strlen((char*)lpMsgBuf) + strlen(lpFunction) + 60);
	lpDisplayBuf = LocalAlloc(LMEM_ZEROINIT, n);

	int n2 = sprintf_s((char *)lpDisplayBuf, n, "%s failed with error %d: %s", lpFunction, dw, (char *)lpMsgBuf);

	printf((char*)lpDisplayBuf);
	//MessageBoxA(hWnd, (char*)lpDisplayBuf, "Error", MB_OK);

	LocalFree(lpMsgBuf);
	LocalFree(lpDisplayBuf);
	
}

bool CMainFrame::Save(bool bInteractive)
{
	bool bRes = false;
	while (1){
		//
		INT_PTR nRet = -1;
		//
		SaveDialogWn saveDlg(NULL);
		//	
		//saveDlg.SaveCaption.Format(L"Save %.0f sec", measurementDuration);

		bAutoClose = parameters.AutoClose();

		saveDlg.SaveTime(saveTime);
		saveDlg.AutoClose(bAutoClose);


		//saveDlg.SaveCaption.Format(L"Save %.0f sec", saveTime);

		saveDlg.ExportPath(parameters.ExportPath());
		saveDlg.ExpName(expName);
		saveDlg.Suffix(suffix);
		//
		if (!bInteractive && !bFilePathSet) {
			MessageBox(L"Automatic Saving requires FilePath be set", filePath, MB_OKCANCEL | MB_ICONWARNING);
			break;
		}
		//	
		saveDlg.bInteractive = bInteractive;

		int ret = (int)saveDlg.DoModal();

		if (ret == IDOK) {
			expName = saveDlg.ExpName();
			meaName = saveDlg.MeaName();
			filePath = saveDlg.FilePath();
			rootName = saveDlg.FileRoot();
			//bSaveAll     = saveDlg.bSaveAll;
			saveTime = saveDlg.SaveTime();
			bAutoClose = saveDlg.AutoClose();

			bFilePathSet = true;

			//expName = saveDlg.GetExpName();
			//
			CString outFileTrace = rootName + L".csv";
			CString outFileParam = rootName + L".xml";
			CString outFileStats = rootName + L".stats.xml";
			CString outFileTraceStats = rootName + L".trace.csv";
			CString outFileImg = rootName + L".bmp";
			CString outFileVideo = rootName + L".avi";
			//
			printf("Write Traces to %ls\n", outFileTrace);
			//
			//
			if (!dirExists(filePath)) {
				if (IDCANCEL == MessageBox(L"Create New Directory?", filePath, MB_OKCANCEL | MB_ICONWARNING))
					break;

				if (!CreateDirectory(filePath, NULL)) {
					MessageBox(L"Failed to create", filePath, MB_OK | MB_ICONWARNING);
					break;
				}
			}

			if (fileExists(outFileTrace)) {
				if (IDCANCEL == MessageBox(L"Overwrite Existing Outfile?", outFileTrace, MB_OKCANCEL | MB_ICONWARNING))
					break;

				if (DeleteFileW(outFileTrace) == 0 && GetLastError() != ERROR_FILE_NOT_FOUND) {
					MessageBox(L"Failed to Delete", outFileTrace, MB_OK | MB_ICONWARNING);
					break;
				}

				if (DeleteFileW(outFileParam) == 0 && GetLastError() != ERROR_FILE_NOT_FOUND) {
					MessageBox(L"Failed to Delete", outFileParam, MB_OK | MB_ICONWARNING);
					break;
				}

				if (DeleteFileW(outFileStats) == 0 && GetLastError() != ERROR_FILE_NOT_FOUND) {
					MessageBox(L"Failed to Delete", outFileStats, MB_OK | MB_ICONWARNING);
					break;
				}
			}
			//
			writeTraces(outFileTrace);
			//
			parameters.write(outFileParam);
			//parameters.setEnvironmentC(outFileParam);

			printf("Write Stats to %ls\n", outFileStats);
			writeStats(outFileStats);
			//
			if (parameters.SaveTraceStats())
				saveTraceStats(outFileTraceStats);

			printf("Write Image to %ls\n", outFileImg);
#ifndef MY_NO_pylon
			m_image.Save(ImageFileFormat_Bmp, ((CStringA)outFileImg).GetString());
			m_image.Release();
#endif   
			if (parameters.UseDb()) {
				printf("Push Trace of to Db %ls - %ls\n", expName, meaName);
				//saveTracesDb();
			}

			if (parameters.SaveVideo())
			{
				printf("Write Video to %ls\n", outFileVideo);
				saveAvi(outFileVideo);
				bKeepUncompressedVideo = false;
				if (parameters.CompressVideo()) {
					printf("Compress Video File\n");
					compressFile(outFileVideo, bKeepUncompressedVideo);
				}
			}
		}

		if (bAutoClose)
			OnClose();
		bRes = true;  break;
	}
	return bRes;
}

bool CMainFrame::compressFile(CString fNameIn, bool bKeepSource) {

	char mySHPARA[400];
	CString fNameOut = fNameIn + L".7z";

	int n2 = sprintf_s(mySHPARA, 299, "a -aoa %ls %ls", fNameOut.operator LPCWSTR(), fNameIn.operator LPCWSTR());
		
	SHELLEXECUTEINFOA ShExecInfo = { 0 };
	ShExecInfo.cbSize = sizeof(SHELLEXECUTEINFOA);
	ShExecInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
	ShExecInfo.hwnd = NULL;
	ShExecInfo.lpVerb = NULL;
	ShExecInfo.lpFile = "C:\\Labhub\\Import\\7za.exe";
	ShExecInfo.lpParameters = mySHPARA;
	ShExecInfo.lpDirectory = NULL;
	ShExecInfo.nShow = SW_HIDE;
	ShExecInfo.hInstApp = NULL;
	
	//MessageBox(L"Start Compression, might take minutes", L"SLOW - WARNING", MB_ICONWARNING);
	printf("Compress File %ls\n", fNameIn);

	if (ShellExecuteExA(&ShExecInfo) == TRUE) {
		WaitForSingleObject((HANDLE)ShExecInfo.hProcess, INFINITE);
	}
	else {
		myError(NULL, "ShellExecuteExA Error: ", GetLastError());
	}
	CloseHandle((HANDLE)ShExecInfo.hProcess);

	if (!bKeepSource && fileExists(fNameIn) && fileExists(fNameOut))
						DeleteFileW(fNameIn);
		

	return true;
}

bool CMainFrame::saveTiffs(CString rootname) {
#ifndef MY_NO_pylon
	CString str;
	int n = 1;
	if (imageStackNum > imageStackSize) {
		for (int i = imageStackIndx + 1; i < imageStackSize; i++) {
			str.Format(L"%s.%04d.bmp", rootName, n++);
			m_image.AttachGrabResultBuffer(m_imageStack[i]);
			m_image.Save(ImageFileFormat_Bmp, ((CStringA)str).GetString());
			m_image.Release();
			m_imageStack[i].Release();
		}
	}

	for (int i = 0; i <= imageStackIndx; i++) {
		str.Format(L"%s.%04d.bmp", rootName, n++);
		m_image.AttachGrabResultBuffer(m_imageStack[i]);
		m_image.Save(ImageFileFormat_Bmp, ((CStringA)str).GetString());
		m_image.Release();
		m_imageStack[i].Release();
	}
#else

#endif
	return true;
}

bool CMainFrame::saveAvi(CString outfile)
{
	// Create an AVI writer object.
	ULARGE_INTEGER FreeBytesCaller;
	ULARGE_INTEGER TotalBytes;
	ULARGE_INTEGER FreeBytes;

	GetDiskFreeSpaceExA( CStringA(filePath), &FreeBytesCaller, &TotalBytes, &FreeBytes);

#ifndef MY_NO_pylon
	CAviWriter aviWriter;
	// The AVI writer supports the output formats PixelType_Mono8,
	// PixelType_BGR8packed, and PixelType_BGRA8packed.
	EPixelType aviPixelType = PixelType_BGR8packed;

	// The frame rate used for playing the video (play back frame rate).
	const int cFramesPerSecond = (int)m_camera->AcquisitionFrameRate();
		
	
	// Get the required camera settings.
	CIntegerPtr width(m_camera->GetNodeMap().GetNode("Width"));
	CIntegerPtr height(m_camera->GetNodeMap().GetNode("Height"));
	CEnumerationPtr pixelFormat(m_camera->GetNodeMap().GetNode("PixelFormat"));
	if (pixelFormat.IsValid())
	{
		// If the camera produces Mono8 images use Mono8 for the AVI file.
		if (pixelFormat->ToString() == "Mono8")
		{
			aviPixelType = PixelType_Mono8;
		}
	}
	// Optionally set up compression options.
	SAviCompressionOptions* pCompressionOptions = NULL;
	// Uncomment the two code lines below to enable AVI compression.
	// A dialog will be shown for selecting the codec.
	//SAviCompressionOptions compressionOptions( "MSVC", true);
	//pCompressionOptions = &compressionOptions;

	// Open the AVI writer.
	aviWriter.Open(
		CStringA(outfile).operator LPCSTR(),
		cFramesPerSecond,
		aviPixelType,
		(uint32_t)width->GetValue(),
		(uint32_t)height->GetValue(),
		ImageOrientation_BottomUp, // Some compression codecs will not work with top down oriented images.
		pCompressionOptions);

	int n = 1;
	if (imageStackNum > imageStackSize) 
		for (int i = imageStackIndx + 1; i < imageStackSize; i++) 
			aviWriter.Add(m_imageStack[i]);			
	
	for (int i = 0; i <= imageStackIndx; i++) 
		aviWriter.Add(m_imageStack[i]);		
#else

#endif	
	return true;
}

void CMainFrame::drawPlots(){
	double delta = 0.0;
	double upper, lower;
	for (int i = 0; i < nWell; i++) {		
		delta = m_trace[i].slowTrace.range() * 0.3;
		upper = m_trace[i].slowTrace.MAX() + delta;
		lower = min(m_trace[i].slowTrace.MIN(), m_trace[i].cutOff) - delta;
		m_Plot[i].SetLYRange(lower, upper);
		if (bColorByTwitch)       //                    Value         Green   Yellow  Red
			m_Plot[i].m_plotBkColor = numToRGB(m_trace[i].getPeak(), 0.010,   0.008, 0.003);
		else if (bColorByBase)
			m_Plot[i].m_plotBkColor = numToRYGYR(m_trace[i].slowTraceAVG()/pixel_per_mm , 3.0, 3.3, 3.9, 4.1, 4.2);
		
		m_Plot[i].Invalidate();
		m_Plot[i].UpdateWindow();
	}
}

COLORREF CMainFrame::numToRYGYR(double V, double Rl, double Yl, double G, double Yh, double Rh) {
	
	long dMax = 255;
	long dMin = 80;

	long dRange = dMax - dMin;

	long mR = (long)dMax;
	long mG = (long)dMax;
	long mB = (long)dMax;
	//
	if (V < Rl) {
		// Pure Red;  (255,  0,  0)
		mR = (long)dMax;
		mG = (long)dMin;
		mB = (long)dMin;
	}
	else if ( V < Yl)
	{
		// Red->Yelow, increase Green
		// (255,  0,  0)  -> (255, 255, 0)
		mR = (long)dMax;
		mG = (long)(dMin + dRange * (V - Rl) / (Yl - Rl));
		mB = (long)dMin;
	}
	else if ( V < G) {
		//yellow -> green
		// (255, 255, 0)  -> (  0, 255,   0)
		mR = (long)(dMax - dRange * (V - Yl) / (G - Yl));
		mG = (long)dMax;
		mB = (long)dMin;
	}
	else if (V < Yh) {
		// green -> yellow
		// (  0, 255, 0)  -> ( 255, 255,   0)
		mR = (long)(dMin + dRange * (V - Yh) / (Yh - G));
		mG = (long)dMax;
		mB = (long)dMin;
	}
	else if (V < Rh) {
		// green -> yellow
		// (  255, 255, 0)  -> ( 255,   0,   0)
		mR = (long)dMax;
		mG = (long)(dMin - dRange * (V - Rh) / (Rh - Yh));
		mB = (long)dMin;
	}
	else {
		// Pure Red;  (255,  0,  0)
		mR = dMax;
		mG = dMin;
		mB = dMin;
	}
	return mR + (mG << 8) + (mB << 16);
}

COLORREF CMainFrame::numToRYG(double V, double R, double Y, double G) {
	double dV = V - G;
	double dY = Y - G;
	double dR = R - G;

	long dMax = 255;
	long dMin = 80;

	long mR = dMax;
	long mG = dMax;
	long mB = dMax;
	//
	if (V < R) {
	// Pure Red;
	mB = dMin;
	mG = dMin;
	} 
	else if (dV / dY < 1)
	{
		// white -> Yellow 0-255 white -> yellow
		//set BK color AAA
		//unsigned int j = m_Plot[i].m_stats[2].dValue * 10;
		mB = (long)(dMax - dMin * (dV / dY));
		//log(val) / log(1000) * 512;   
		// 0-255 white -> yellow,   256-512  yellow -> red
	}
	else if (dV / dR < 1) {
		//yellow -> red 256-512  
		//col = (col < 256) ? (((255 - col) << 16) + 0x0000ffff) : (((512 - col) << 8) + 0x000000ff);
		mG = (long)(dMax - dMin * dV / dY);
		mB = dMin;
	}
	else  {
		// alles gut
	}
	return mR + (mG << 8) + (mB << 16);
}

COLORREF CMainFrame::numToRGB(double V, double G, double Y, double R){
	double dV = V - G;
	double dY = Y-G;
	double dR = R - G;

	long dMax = 255;
	long dMin = 80; 
	
	long mR = dMax;
	long mG = dMax;
	long mB = dMax;	
	//
	if ( dV / dY < 0){
		// alles gut 		
	}
	else if (dV / dY < 1)
	{
		// white -> Yellow 0-255 white -> yellow
		//set BK color AAA
		//unsigned int j = m_Plot[i].m_stats[2].dValue * 10;
		mB = (long)(dMax - dMin * (dV / dY));
		//log(val) / log(1000) * 512;   
		// 0-255 white -> yellow,   256-512  yellow -> red
	} else if (dV / dR < 1){
		//yellow -> red 256-512  
		//col = (col < 256) ? (((255 - col) << 16) + 0x0000ffff) : (((512 - col) << 8) + 0x000000ff);
		mG = (long)(dMax - dMin * dV / dY);
		mB = dMin;
	}
	else {
		// Pure Red;
		mB = dMin;
		mG = dMin;
	}
	return mR + (mG << 8) + (mB << 16) ;
}

void __fastcall CMainFrame::myReCalImgMetrix(RECT &irr)
{
	PVnLeftX = 3; // 5
	const int nRightX = 3;  // 32
	PVnTopY = 3;	// 5
	const int nBottomY = 24; // 100
	//RECT irr; GetClientRect(&irr);//GetWindowRect(&irr);
	PVnXX = irr.right - irr.left - PVnLeftX - nRightX;
	PVnYY = irr.bottom - irr.top - PVnTopY - nBottomY;

//PVnWidth, PVnHeight;
//double PVdRX, PVdRY;
}
void CMainFrame::drawROIs(bool bBK)
{	//bBK = false
	if(bBK)		// this call is to erase the BK
	{
		CDC *pdc = GetDC();
		if (pdc == NULL)return;
		RECT irr;
		GetClientRect(&irr);
		pdc->FillSolidRect(3, 3, irr.right - irr.left-6, irr.bottom - irr.top-3-24, RGB(255,255,255)); // this will not immediately update the screen
		ReleaseDC(pdc);
		return;
	}
	if (nDrawROIs > 0)	// skip some user drawing
	{
		nDrawROIs--;
		return;
	}
#ifndef MY_NO_pylon
	uint8_t* pPixImg = (uint8_t *) m_image.GetBuffer();
	uint8_t* pPix;
	int lineWidth = 6;
	//
	for (int i = 0; i < nWell; i++) {
		for (int k = 0; k < lineWidth; k++) {
			for (int j = 0; j < roiCols; j++){
				// horizontal line above
				pPix = pPixImg + roiPixOffset[i] + roiPixList[j] + k * imageWidth - 1;
				*pPix = 255;
				pPix = pPix += roiCols;
				*pPix = 255;
				// horizontal line below
				pPix = pPixImg + roiPixOffset[i] + roiPixList[nRoiPix - roiCols + j] + (k - lineWidth + 1) * imageWidth - 1;
				*pPix = 255;
				pPix = pPix += roiCols;
				*pPix = 255;
			}
			for (int j = 0; j < roiRows; j++) {
				//vertical line width=3
				pPix = pPixImg + roiPixOffset[i] + roiCols + j * imageWidth - 1 + k;
				*pPix = 255;
			}
		}
	}
	//	
	//#ifdef _DEBUG h_RoiPreview->SetImage(m_image); 
	//#endif
	//Pylon::DisplayImage(1, ptrGrabResult);:(1, ptrGrabResult);
	//
	const bool bStrech = true;
	const bool bFill = true;
	const int nLeftX = 3; // 5
	const int nRightX = 3;  // 32
	const int nTopY = 3;	// 5
	const int nBottomY = 24; // 100
	CDC *pdc = GetDC();
	if (pdc == NULL)return;
	CDC pdcM;
	pdcM.CreateCompatibleDC(pdc);
	//
	bmpPreView.CopyImage(m_image);
	//
	HGDIOBJ hOld = pdcM.SelectObject(bmpPreView); // //HBITMAP hBMP = bmp;
	int nXX, nYY, nWidth, nHeight; nHeight = bmpPreView.GetHeight(); nWidth = bmpPreView.GetWidth();
	if (bStrech)
	{
		RECT irr;
		
		GetClientRect(&irr);
		nXX = irr.right - irr.left - nLeftX - nRightX;
		nYY = irr.bottom - irr.top - nTopY - nBottomY;
		if (!bFill)
		{
			double dRR = double(bmpPreView.GetHeight()) / double(bmpPreView.GetWidth());
			if (dRR*double(nXX) > double(nYY)){
				nXX = (int)(double(nYY) / dRR + 0.0005);
			}
			else
			{
				nYY = (int)(dRR * double(nXX) + 0.0005); //nYY = dRR*double(nXX) + 0.0005;
			}

		}
		pdc->StretchBlt(nLeftX, nTopY, nXX, nYY, &pdcM, 0, 0, nWidth, nHeight, SRCCOPY);
	}
	else
		pdc->BitBlt(nLeftX, nTopY, nWidth, nHeight, &pdcM, 0, 0, SRCCOPY);
	//
#else
	const unsigned char* pPixImg = (const unsigned char*)mFF2->pMEM;
	unsigned char* pPix;
	int lineWidth = 6;
	//
	for (int i = 0; i < nWell; i++) {
		for (int k = 0; k < lineWidth; k++) {
			for (int j = 0; j < roiCols; j++){
				// horizontal line above
				pPix = (unsigned char*)(pPixImg + roiPixOffset[i] + roiPixList[j] + k * imageWidth - 1);
				*pPix = 255;
				pPix = pPix += roiCols;
				*pPix = 255;
				// horizontal line below
				pPix = (unsigned char*)(pPixImg + roiPixOffset[i] + roiPixList[nRoiPix - roiCols + j] + (k - lineWidth + 1) * imageWidth - 1);
				*pPix = 255;
				pPix = pPix += roiCols;
				*pPix = 255;
			}
			for (int j = 0; j < roiRows; j++) {
				//vertical line width=3
				pPix = (unsigned char*)(pPixImg + roiPixOffset[i] + roiCols + j * imageWidth - 1 + k);
				*pPix = 255;
			}
		}
	}
	//	
	//#ifdef _DEBUG h_RoiPreview->SetImage(m_image); 
	//#endif
	//Pylon::DisplayImage(1, ptrGrabResult);:(1, ptrGrabResult);
	//
	const bool bStrech = true;
	const bool bFill = true;
	const int nLeftX = 3;
	const int nRightX = 3;
	const int nTopY = 3;
	const int nBottomY = 24;
	CDC *pdc = GetDC();
	if (pdc == NULL)return;
	CDC pdcM;
	pdcM.CreateCompatibleDC(pdc);
	//
	//bmpPreView.CopyImage(m_image);
	//
	HBITMAP hBMP = mFF2->myGetBMP();
	HGDIOBJ hOld = pdcM.SelectObject(hBMP); // //HBITMAP hBMP = bmp; hOld = pdcM.SelectObject(bmpPreView);

	//if ((mskEHM & MY_SHOW_OBJ_KASK)!=0) myDrawTag(pdcM.m_hDC, 0, 0, 1, 1, 1, 1);  // only BK

	int nXX, nYY, nWidth, nHeight; nHeight = mFF2->GetHeight(); nWidth = mFF2->GetWidth();
	if (bStrech)
	{
		RECT irr;	
		GetClientRect(&irr);//GetWindowRect(&irr);
		nXX = irr.right - irr.left - nLeftX - nRightX;
		nYY = irr.bottom - irr.top - nTopY - nBottomY;
		if (!bFill)
		{
			double dRR = double(nHeight) / double(nWidth); //double dRR = double(bmpPreView.GetHeight()) / double(bmpPreView.GetWidth());
			if (dRR*double(nXX) > double(nYY)){
				nXX = (int)(double(nYY) / dRR + 0.0005);
			}
			else
			{
				nYY = (int)(dRR * double(nXX) + 0.0005); //nYY = dRR*double(nXX) + 0.0005;
			}

		}
		pdc->StretchBlt(nLeftX, nTopY, nXX, nYY, &pdcM, 0, 0, nWidth, nHeight, SRCCOPY); //pdc->StretchBlt(nLeftX, nTopY, nXX, nYY, &pdcM, 0, 0, bmpPreView.GetWidth(), bmpPreView.GetHeight(), SRCCOPY);
	}
	else
		pdc->BitBlt(nLeftX, nTopY, nWidth, nHeight, &pdcM, 0, 0, SRCCOPY); //pdc->BitBlt(nLeftX, nTopY, bmpPreView.GetWidth(), bmpPreView.GetHeight(), &pdcM, 0, 0, SRCCOPY);

#endif

	//InvalidateRect(NULL, FALSE);
	pdcM.SelectObject(hOld); // recover/replace, this step is for deleting pdcM using
	pdcM.DeleteDC();
	if ((mskEHM & MY_SHOW_OBJ_KASK) != 0 || (mskEHM & MY_DSP_POLE) == MY_DSP_POLE)
	{
		if (bStrech)
			myDrawTag(pdc->m_hDC, nLeftX, nTopY, nXX, nYY, nWidth, nHeight);
		else
			myDrawTag(pdc->m_hDC, nLeftX, nTopY, nWidth, nHeight, nWidth, nHeight);
	}
	ReleaseDC(pdc);

#ifndef MY_NO_pylon

	//m_image.Release();    // maybe we also do not this line

#endif

}

int __fastcall myD2I(const double d)
{
	return (int)(d + 0.5);
}

void __fastcall myDrawFrame(HDC &hdc, HBRUSH &brush, const double dX, const double dY, const double exdW, const double exdH, const double dType, const double dCC, const double dCC2)
{
	RECT irr;
	//double dX = dRX * nX + nLeftX;
	//double dY = dRY * nY + nTopY;
	SetRect(&irr, myD2I(dX), myD2I(dY), myD2I(dX + exdW), myD2I(dY + exdH)); // Right angle triangle btw.	
	int cc = myD2I(abs(dCC) * 255) & 0xff;
	if (abs(dType - 1) < 1e-7 && dCC > 0.1 && dCC2 > 0.1) {
		cc = 0x0000ff00 | (0xff - cc);
	}
	else{
		cc = 0x0000ff | (cc << 16) | (cc << 8); // Green, Blue, Red
	}
	::SetDCBrushColor(hdc, cc);
	FrameRect(hdc, &irr, brush);
}

// No support for other values rather than 13
#define	MY_CYC_P13		13
#define	MY_2PI			6.283185307179586476925286766559
#define MY_COS_30		0.86602540378443864676372317075294

typedef struct _MY_dPOINT
{
	double  x;
	double  y;
}MY_dPOINT;

void __fastcall myCycle_12P(MY_dPOINT *sP13, const double dR = 7, const double dRX = 1, const double dRY = 1)
{
	double dRRX = dR * dRX; double dRRY = dR * dRY;
	sP13[0].x = 0;							sP13[0].y = dRRY;
	sP13[1].x = 0.5 * dRRX;					sP13[1].y = MY_COS_30 * dRRY;
	sP13[2].x = MY_COS_30 * dRRX;			sP13[2].y = 0.5 * dRRY;

	sP13[3].x = dRRX;						sP13[3].y = 0;
	sP13[4].x = sP13[2].x;					sP13[4].y = -sP13[2].y;
	sP13[5].x = sP13[1].x;					sP13[5].y = -sP13[1].y;

	sP13[6].x = 0;							sP13[6].y = -sP13[0].y;
	sP13[7].x = -sP13[1].x;					sP13[7].y = -sP13[1].y;
	sP13[8].x = -sP13[2].x;					sP13[8].y = -sP13[2].y;

	sP13[9].x = -sP13[3].x;					sP13[9].y = 0;
	sP13[10].x = -sP13[2].x;				sP13[10].y = sP13[2].y;
	sP13[11].x = -sP13[1].x;				sP13[11].y = sP13[1].y;

	sP13[12] = sP13[0];
}
void __fastcall myMoveToXY(const MY_dPOINT *sP, POINT *sP1, const int n, const double dX, const double dY)
{
	for (int i = 0; i < n; i++){
		sP1[i].x = myD2I(sP[i].x + dX); sP1[i].y = myD2I(sP[i].y + dY);
	}
}
void __fastcall myCycle_XY(HDC hdc, const double dX, const double dY, const MY_dPOINT *sP0)
{
	POINT sP1[MY_CYC_P13];
	myMoveToXY(sP0, sP1, MY_CYC_P13, dX, dY);
	Polyline(hdc, sP1, MY_CYC_P13);
}

void __fastcall CMainFrame::myDrawTag(HDC hdc, int nLeftX, int nTopY, int nXX, int nYY, int nW, int nH)
{
	double dRX = (double)nXX / nW;
	double dRY = (double)nYY / nH;
	// pole position
	if ((mskEHM & MY_DSP_POLE) == MY_DSP_POLE)
	{
		MY_dPOINT sP[MY_CYC_P13]; myCycle_12P(sP, 7, dRX, dRY);
		HPEN pPenOld = (HPEN)::SelectObject(hdc, ::GetStockObject(DC_PEN));
		unsigned long colorSave = ::SetDCPenColor(hdc, RGB(0, 0, 0));
		if ((mskEHM & MY_NEW_METHOD_DOING) != 0)
			::SetDCPenColor(hdc, 0x00ff00ff);
		else
			::SetDCPenColor(hdc, 0x0000ff);
		for (int i = 0; i < 48; i++)
		{
			myCycle_XY(hdc, dWellPoleXY[i][0] * dRX + nLeftX, dWellPoleXY[i][1] * dRY + nTopY, sP);
			myCycle_XY(hdc, dWellPoleXY[i][2] * dRX + nLeftX, dWellPoleXY[i][3] * dRY + nTopY, sP);
		}
		::SetDCPenColor(hdc, colorSave);
		::SelectObject(hdc, pPenOld);
	}
	// Method2 detect area
	if ((mskEHM & MY_SHOW_OBJ_KASK) != 0)
	{
		if (eFit->isINIT_OK())
		{
			HBRUSH brush = (HBRUSH)::GetStockObject(DC_BRUSH);
			unsigned long colorSave = ::SetDCBrushColor(hdc, RGB(0, 0, 0));

			double exdW = MY_CUT_IMAGE_X * dRX;
			double exdH = MY_CUT_IMAGE_Y * dRY;
			for (int i = 0; i < 48; i++)
			{
				myDrawFrame(hdc, brush, dRX * eFit->nXXYY[i][4] + nLeftX, dRY * (eFit->nXXYY[i][5] + 1) +  nTopY, exdW, exdH, eFit->preVal[i][0], eFit->preVal[i][1], eFit->preVal[i][2]);
				myDrawFrame(hdc, brush, dRX * eFit->nXXYY[i][6] + nLeftX, dRY * (eFit->nXXYY[i][7] + 1) + nTopY, exdW, exdH, eFit->preVal[i][0], eFit->preVal[i][2], eFit->preVal[i][1]);
			}

			::SetDCBrushColor(hdc, colorSave);
		}
	}
}

// in the h file in class CMainFrame, put function define, instead calcDistance using calcDistanceP for % top some pixels
//in the main file include math
#include <math.h>

inline double mySquare(const double d) {
	return d * d;
}

double CMainFrame::calcDistanceP()
{
#ifndef MY_NO_pylon
	const unsigned char* pPixImg = (unsigned char*)m_image.GetBuffer();	
#else
	const unsigned char* pPixImg = (const unsigned char*)mFF2->pMEM;
#endif
	
	bool bRawXY = ((mskEHM & MY_DSP_POLE_ENABLE) != 0);

	if ((mskEHM & MY_NEW_METHOD_MASK) != 0 && eFit->isINIT_OK())  // method 2, new method
	{
		double dOut[48];	
		int nIDC;
		if (bRawXY)
		{
			myOBJ48_Tag *pFrame;
			nIDC = eFit->myANA_48(pPixImg, dOut, false, &pFrame, MY_NO_DEL_ANA);
			eFit->myGetRaw(pFrame, (double *)dWellPoleXY);
		}
		else{
			nIDC = eFit->myANA_48(pPixImg, dOut);
		}
		double dT = m_time.get();
		for (int i = 0; i < nWell; i++) {	
			m_trace[i].put(dOut[i], dT);
		}

		mskEHM |= MY_NEW_METHOD_DOING;
	}
	else   // method 1, old one
	{
		int n = (int)((double)nRoiPix * pixUse + 0.5); // number of pixels from high intensity needed, default is 2%
		double dT = m_time.get();
		for (int i = 0; i < nWell; i++) {
			//int iRow = i / nColCount; //for (iRow = 0; iRow < nRowCount; iRow++) iWell0 = iRow*nColCount; iWell = iWell0 + iCol;
			//int iCol = i % nColCount; //for (int iCol = 0; iCol < nColCount; iCol++) 
			double dXL, dYL, dXR, dYR;
			unsigned char unTHL = myGetImgXY_iW(pPixImg, i, n, dXL, dYL);
			unsigned char unTHR = myGetImgXY_iW(pPixImg + roiCols, i, n, dXR, dYR);
			//
			pPixThreshL[i] = unTHL;
			pPixThreshR[i] = unTHR;
			//
			if (bRawXY){
				dWellPoleXY[i][0] = dXL + nWellXY[i][0]; dWellPoleXY[i][1] = dYL + nWellXY[i][1]; dWellPoleXY[i][2] = dXR + nWellXY[i][0] + roiCols; dWellPoleXY[i][3] = dYR + nWellXY[i][1];
			}
			double dist = (unTHL < pixThresh) ? 0.0 : sqrt(mySquare(dXR + roiCols - dXL) + mySquare(dYR - dYL));
			//printf("dist %.1f XYL=(%.1f, %.1f) XYR=(%.1f, %.1f).\n", dist, dXL, dYL, dXR, dYR); 
			m_trace[i].put(dist, dT);
			// following is to replace the pixel under th with 0, when display is trigged, we use
			const unsigned char* pPixImg0 = pPixImg + roiPixOffset[i];
			for (int j = 0; j < nRoiPix; j++) {
				//
				const unsigned char* pPix = pPixImg0 + roiPixList[j];
				if (*pPix < unTHL) {
					*((unsigned char*)pPix) = 0;
				}
				pPix += roiCols;
				if (*pPix < unTHR) {
					*((unsigned char*)pPix) = 0;
				}
			}
		}
		mskEHM &= ~MY_NEW_METHOD_DOING;
		//double dNow = (double)(clock()) / (double)CLOCKS_PER_SEC * 1000;
		//printf("Calc Distances took %G ms \n", dNow - dStart);
		//return tStop - dStart;
	}

	if (bRawXY)
	if ((mskEHM & MY_DSP_POLE_OK) == 0)
		mskEHM |= MY_DSP_POLE_OK;

	myNewMethodDSPCheck();  // Status "New" or "Old" method

	return 0;
}
void __fastcall CMainFrame::myNewMethodDSPCheck(void)
{
	// check display	
	CStatusBarCtrl &m_wndSBC = m_wndStatusBar.GetStatusBarCtrl();
	int nType;
	int nLength = m_wndSBC.GetTextLength(1, &nType);
	bool bSet = false;
	bool bNew = ((mskEHM & MY_NEW_METHOD_DOING) != 0);
	if (nLength >= 19)bSet = true;
	else{
		if (nLength == 0)bSet = true;
		else{
			wchar_t wStr[20];
			int nTextLength = m_wndSBC.GetText(wStr, 1, &nType);
			if (nTextLength > 3){
				if (bNew && wStr[3] == L'1' || !bNew && wStr[3] == L'2')
					bSet = true;
			}
			else bSet = true;
		}
	}
	if (bSet){
		m_wndSBC.SetText(bNew ? L"MED2" : L"MED1", 1, SBT_NOBORDERS);
	}

}
unsigned char __fastcall CMainFrame::myGetImgXY_iW(const unsigned char* pMEM, const int iW, const int n, double& dX, double& dY)
{
	int xBin[256] = { 0 };
	int yBin[256] = { 0 };
	int nBin[256] = { 0 };
	const unsigned char* pImgXY0 = pMEM + roiPixOffset[iW]; //roiPixOffsetY[iRow] + roiPixOffsetX[iCol];
	//int nLenX = (roiCols >> 1);
	for (int j = 0; j < roiRows; j++) {
		for (int i = 0; i < roiCols; i++) {
			unsigned char cOne = pImgXY0[i];
			nBin[cOne]++; xBin[cOne] += i; yBin[cOne] += j;
		}
		pImgXY0 += imageWidth;
	}
	double dS = 0; dX = dY = 0; 
	unsigned char ucTH = 0;
	int m = 0;
	for (int i = 255; i >= 0; i--) {
		int j;
		if ((j = nBin[i]) > 0) {
			dS += j * i; dX += xBin[i] * i; dY += yBin[i] * i; m += nBin[i];
			if (m >= n) { ucTH = i; break; };
		}
	}
	if (dS > 0) {
		dS = 1 / dS; dY *= dS; dX *= dS;
	}
	return ucTH;
}



double CMainFrame::simulateDistance()
{
	for (int i = 0; i < nWell; i++) 
		m_trace[i].put(3.9 * pix2mm - pow(sin(m_time.get() + i), 12) + (double)rand() / RAND_MAX, m_time.get());
	return 0.0;
}

long long int CMainFrame::calcImageSum()
{
	//
	//double dStart = (double)(clock()) / (double)CLOCKS_PER_SEC * 1000;
	long long int Sum=0;
	//
	
#ifndef MY_NO_pylon
	uint8_t* pPixImg = (uint8_t*) m_image.GetBuffer();
	int j = (int)(m_image.GetWidth() * m_image.GetHeight());
#else
	const unsigned char* pPixImg = (const unsigned char*)mFF2->pMEM;
	int j = (int)(mFF2->GetWidth() * mFF2->GetHeight());
#endif
	
	for (int i = 0; i < j; i++)
		Sum += pPixImg[i];
	//for (int i = 0; i < m_image.GetWidth() * m_image.GetHeight(); i++)
	//	Sum = Sum + pPixImg[i];
	
	return Sum;
}

double CMainFrame::calcDistance()
{
	//
	//double dStart = (double)(clock()) / (double)CLOCKS_PER_SEC * 1000;
	double dist=0.0;
	bool bOK;
	//
#ifndef MY_NO_pylon
	uint8_t* pPixImg = (uint8_t*)m_image.GetBuffer();
#else
	const unsigned char* pPixImg = (const unsigned char*)mFF2->pMEM;
#endif
	const unsigned char* pPix;
	//
	uint64_t nPixL, nPixR;
	double rowSumL, rowSumR, wRowSumL, wRowSumR;
	//
	for (int i = 0; i < nWell; i++) {
		rowSumL  = 0.0;
		rowSumR  = 0.0;
		wRowSumL = 0.0;
		wRowSumR = 0.0;
		//
		nPixL = 0;
		nPixR = 0;
		
		double minPix = 0.01; // At least 1% of the pixels should be above threshold
		//
		for (int j = 0; j < nRoiPix; j++) {
			//
			pPix = pPixImg + roiPixOffset[i] + roiPixList[j];
			int dd;
			if ((dd = ((int)*pPix - (int)pixThresh)) > 0) {				//if (*pPix > pixThresh){
				rowSumL  += dd; //rowSumL++;
				wRowSumL += (double) roiXpos[j] * dd;
				nPixL++;
			}
			else {
				*((unsigned char*)pPix) = 0;  //make pixel black on screen
			}
			//
			pPix += roiCols;
			//
			if ((dd = ((int)*pPix - (int)pixThresh)) > 0) {				//if (*pPix > pixThresh){
				rowSumR  += dd; //rowSumR++;
				wRowSumR += (double)roiXpos[j] * dd;
				nPixR++;
			}
			else {
				*((unsigned char*)pPix) = 0;//make pixel black on screen
			}
		}
		//
		bOK = (nPixL > nRoiPix * minPix && nPixR > nRoiPix * minPix);
		dist = (bOK) ? (double) wRowSumR / rowSumR + roiCols - (double) wRowSumL / rowSumL : 0.0;
		//
		m_trace[i].put(dist, timestamp);
	}
	
	return 0.0;
}

void CMainFrame::writeTraces(CString fName){
	CStringA safName = fName;
	const char* sfName = safName;
	//
	printf("Write Traces to %s, save last %.1f seconds\n", sfName, saveTime);
	//
	FILE* pFile = NULL;
	while (1) {
		if (fopen_s(&pFile, sfName, "w") != 0) //pFile = fopen( (CStringA) fName, "w");
		{
			MessageBox(L"Failed to open ", fName, MB_OK); break;
		}

		if (pFile == NULL) {
			MessageBox(L"Failed to open ", fName, MB_OK); break;
		}
		//
		if (m_time.NELE() != m_trace[0].rawTrace.NELE()) {
			MessageBox(L"ERROR, Size of Time and Trace not Equal", L"ERROR"); break;
		}
		//double firstTime = timestamp - measurementDuration;
		double firstTime = m_time.getFirst();
		double lastTime = m_time.getLast();
		double timeRange = m_time.getLast() - m_time.getFirst();

		double startTime = 0.0;
		//
		if (saveTime < 0) {  //means saveAll;
			startTime = m_time.getFirst();
		}
		else if (saveTime < timeRange) {
			startTime = m_time.getLast() - saveTime;
		}
		else {
			CString str;
			str.Format(L"ERROR, Can only Save %12.4f of %12.4f seconds", timeRange, saveTime);
			MessageBox(str, L"ERROR");
			startTime = m_time.getFirst();
		}

		//
		double time_ = 0.0;
		int idx;
		int curIdx = m_time.index();

		if (m_time.NELE() > m_time.size())
		{
			for (idx = curIdx + 1; idx < m_time.size(); idx++)
			{
				if (m_time.buf_[idx] >= startTime) {
					//
					fprintf(pFile, "%12.4f", m_time.buf_[idx] - startTime);
					//
					for (int i = 0; i < nWell; i++)
						fprintf(pFile, ", %8.2f", m_trace[i].rawTrace.buf_[idx]);
					//
					fprintf(pFile, "\n");
				}
			}
		}
		//	
		for (idx = 0; idx < curIdx; idx++)
		{
			//time_ = m_time.buf_[idx];
			if (m_time.buf_[idx] >= startTime) {
				//
				fprintf(pFile, "%12.4f", m_time.buf_[idx] - startTime);
				//
				for (int i = 0; i < nWell; i++)
					fprintf(pFile, ", %8.2f", m_trace[i].rawTrace.buf_[idx]);
				//
				fprintf(pFile, "\n");
			}
		}
		break;
	}
	if (pFile != NULL)
		fclose(pFile);
	//
	printf("Done writing Traces\n");
}

void CMainFrame::saveTracesDb() {
	//
	//unsigned idMea = Db.getMeasurementId(expName, meaName, true);


	//
	for (int i = 0; i < nWell; i++) {
		//fprintf(pFile, "%.0f, %8.3f, %8.3f, %8.3f, %8.3f, %.0f\n", (long)1.0 + i, m_trace[i].slowTrace.AVG(), m_trace[i].slowTrace.DEV(), m_trace[i].slowTrace.MIN(), m_trace[i].slowTrace.MAX(), (long)m_trace[i].getPeakN());
	}
	
}



void CMainFrame::saveTraceStats(CString fName)
{
	CStringA safName = fName;
	const char* sfName = safName;
	FILE* pFile = NULL;
	while (1) {
		if (fopen_s(&pFile, sfName, "w") != 0) //FILE* pFile = fopen_s(sfName, "w");
		{
			MessageBox(L"Failed to open ", fName, MB_OK); break;
		}
		if (pFile == NULL) MessageBox(L"Failed to open ", fName, MB_OK);
		else {
			//
			for (int i = 0; i < nWell; i++) {
				fprintf(pFile, " %.0f, ", (double)i + 1);
				fprintf(pFile, " %8.3f, %8.3f,", m_trace[i].slowTrace.AVG(), m_trace[i].slowTrace.DEV());
				fprintf(pFile, " %8.3f, %8.3f, %d\n", m_trace[i].slowTrace.MIN(), m_trace[i].slowTrace.MAX(), m_trace[i].getPeakN());
			}
		}
		break;
	}
	if (pFile != NULL)
		fclose(pFile);
}


void CMainFrame::writeStats(CString fName){
	//printf("writeStats to %s\n", (CStringA)  fName);
	Xml Xml(fName);
	//
	Xml.openElement(L"Statistics"); 
	//
	for (int i = 0; i < nWell; i++)
	{
		//
		Xml.openElement( sWell.GetAt(i) );
		//		
		Xml.print(L"nPeak", L"%.0f", (long) m_trace[i].getPeakN());
		// Peak specific data
		if (m_trace[i].getPeakN() > 0){
			Xml.print(L"meanPeakHeight",  L"%.3f", m_trace[i].getPeakAvg()); 
			Xml.print(L"stdevPeakHeight", L"%.3f", m_trace[i].getPeakDev());
			Xml.print(L"skewPeakHeight",  L"%.3f", m_trace[i].getPeakSkew());
			Xml.print(L"kurtPeakHeight",  L"%.3f", m_trace[i].getPeakKurt());
			Xml.print(L"minPeakHeight",   L"%.3f", m_trace[i].getPeakMin());
			Xml.print(L"maxPeakHeight",   L"%.3f", m_trace[i].getPeakMax());
			//
			Xml.print(L"meanRR",          L"%.3f", m_trace[i].getRRAvg());
			Xml.print(L"stdevRR",         L"%.3f", m_trace[i].getRRDev());
			Xml.print(L"minRR",           L"%.3f", m_trace[i].getRRMin());
			Xml.print(L"maxRR",           L"%.3f", m_trace[i].getRRMax());
			//
			Xml.print(L"SNR",             L"%.3f", m_trace[i].getSNR());
		}

		Xml.print(L"pixThreshL", L"%.0f", pPixThreshL[i], CString("Threshold ROI left Pole"));
		Xml.print(L"pixThreshR", L"%.0f", pPixThreshR[i], L"Threshold ROI right Pole");
		//write Mean Baseline as slowTrace.MAX
		Xml.print(L"baseline",   L"%.3f", m_trace[i].getBaseline());
		Xml.print(L"noise",      L"%.3f", sqrt(m_trace[i].getNoiseAvg()));

		
		Xml.print(L"traceN",          L"%.0f", m_trace[i].slowTrace.NVAL());
		Xml.print(L"traceAvg",        L"%.3f", m_trace[i].slowTrace.AVG());
		Xml.print(L"traceDev",        L"%.3f", m_trace[i].slowTrace.DEV());
		Xml.print(L"traceSkew",       L"%.3f", m_trace[i].slowTrace.skewness());
		Xml.print(L"traceKurt",       L"%.3f", m_trace[i].slowTrace.kurtosis());
		Xml.print(L"traceMin",        L"%.3f", m_trace[i].slowTrace.MIN());
		Xml.print(L"traceMax",        L"%.3f", m_trace[i].slowTrace.MAX());
		
		Xml.closeElement();
	}
	Xml.closeElement();
	Xml.close();
	printf("Done writing Stats\n");
}


void CMainFrame::findPeaks()
{
	//
	for (int i = 0; i < nWell; i++) {
		if (m_trace[i].findPeak()){
			//printf("findPeaks: %d Height %.3f RR %.3f Noise %.2f time %.1f curTime %.1f\n", 
			//	i, m_trace[i].peakAbs.get(), m_trace[i].peakRR.get(), m_trace[i].noise.mean()*100, m_trace[i].peakAbs.getTime(), timestamp);
			//
			m_Plot[i].AddPoint(1, *m_trace[i].peakAbs.getTimePtr(), *m_trace[i].peakAbs.getPtr());
			//
			m_Plot[i].m_stats[1].dValue = m_trace[i].getRR();
			m_Plot[i].m_stats[2].dValue = m_trace[i].getPeak()*100;
			m_Plot[i].m_stats[3].dValue = m_trace[i].getBaseline() / pix2mm;
			m_Plot[i].m_stats[4].dValue = m_trace[i].getSNR();
		}
	}
}

bool CMainFrame::InitStimulationBoard()
{
	bool bOK = false;
	while (1){
		long long int Sum_On, Sum_Off;
		CString outFileImg;
		CString sTmp;
		//
		//first check USB communication to board
		if (!stimulationBoard.initialize()) {
			sTmp.Format(L"Failed to initialize Stimulation Board on port %d", stimulationBoard.portNum);
			MessageBoxW(sTmp, L"Abort", MB_OK | MB_ICONWARNING);
			break;
		}
		//
		//bExternalTrigger
		int nStim = stimulationBoard.setStimulation(0);
		bExternalTrigger = false;

		stimulationBoard.write("q\0", 2); //bQuiet

		bool bThorough = false;

		if (bThorough) {
			if (bExternalTrigger)
			{
				stimulationBoard.write("T");
				stimulationBoard.write("F");
			}
			else {
				stimulationBoard.write("t");
				stimulationBoard.write("U");
			}
			//bool bUV   = stimulationBoard.setUV(1, 0);
			//
#ifndef MY_NO_pylon
			//m_camera->AcquisitionMode.SetValue(AcquisitionMode_SingleFrame);
			//m_camera->AcquisitionStart.Execute();
			m_camera->Open();
			// Grab some images for demonstration.
			m_camera->GrabOne(5000, m_ptrGrabResult);
			//m_camera->StartGrabbing(1);
			//m_camera->RetrieveResult(5000, m_ptrGrabResult, TimeoutHandling_Return);
			m_image.AttachGrabResultBuffer(m_ptrGrabResult);

			Sum_On = calcImageSum();
#else
			Sum_On = 1000;
#endif

			outFileImg = parameters.ExportPath() + L"\\uvOn.bmp";
			//m_image.Save(ImageFileFormat_Bmp, ((CStringA)outFileImg).GetString()); 
#ifndef MY_NO_pylon
			m_image.Release();
#endif
			//UV Off
			stimulationBoard.write("u");
			//stimulationBoard.setUV(0);
#ifndef MY_NO_pylon
			m_camera->GrabOne(5000, m_ptrGrabResult);
			m_image.AttachGrabResultBuffer(m_ptrGrabResult);
			Sum_Off = calcImageSum();
#else
			Sum_Off = 0;
#endif
			outFileImg = parameters.ExportPath() + L"\\uvOff.bmp";
			//m_image.Save(ImageFileFormat_Bmp, ((CStringA)outFileImg).GetString()); 

#ifndef MY_NO_pylon
			m_image.Release();
#endif
			double IO_ratio = (double)(Sum_On - Sum_Off) / Sum_On;
			//
			if (Sum_On < 1 || IO_ratio < 0.8)
			{
				wchar_t buffer[256];
				swprintf_s(buffer, 256, L"Sum Off %d On %d ratio %f\ncheck %s", Sum_Off, Sum_On, IO_ratio, parameters.ExportPath());
				MessageBoxW(buffer, L"Failed to test UV Light", MB_OK | MB_ICONWARNING);
				bOK = false;
			}

#ifndef MY_NO_pylon
			m_ptrGrabResult.Release();
			m_camera->Close();
#endif
		}
		//
		if (rrVal > 0)
			stimulationBoard.RR(rrVal);
		else
			stimulationBoard.setStimulation(0);

		bOK = true; break;
	}
	return bOK;
}

bool CMainFrame::InitDb()
{
#ifdef USE_DB
	Db.connect();	
#endif
	return true;
}

bool CMainFrame::InitCamera()
{
#ifndef MY_NO_pylon
	bool bReturn = false;
	while (1) {
		//m_wndStatusBar.SetPaneText(1, _T("Initialize Camera"), TRUE);
		// Before using any pylon methods, the pylon runtime must be initialized. 
		PylonInitialize();

		CTlFactory& tlFactory = CTlFactory::GetInstance();

		DeviceInfoList_t devices;

		//Checks whether or not the camera is plugged in the device.
		if (tlFactory.EnumerateDevices(devices) == 0)
		{
			int Mbox = MessageBox(L"Please plug in the camera and click 'Retry'.", L"Camera not detected.", MB_RETRYCANCEL | MB_ICONERROR);

			if (Mbox == IDRETRY)
			{
				//InitCamera(); // cannot call self here
				PylonTerminate();
				continue;
			}else if(Mbox == IDCANCEL){
				//exit(EXIT_FAILURE); // better not quit here try return false
				break;
			}

		}
		//Sets up the camera object
		Pylon::CDeviceInfo m_info;

		m_camera = new Pylon::CBaslerUsbInstantCamera(tlFactory.CreateFirstDevice(m_info));

		//SetGenICamLogConfig(getenv("TMP"));

		bCameraEvent = false;
		// Camera event processing must be activated first, the default is off.
		m_camera->GrabCameraEvents = true;

		//pEventHandler = new CSampleCameraEventHandler;

	//m_camera->RegisterCameraEventHandler(pEventHandler, "EventExposureEndData", eMyExposureEndEvent, RegistrationMode_ReplaceAll, Cleanup_Delete, CameraEventAvailability_Optional); //RegistrationMode_ReplaceAll,RegistrationMode_Append
	//
	m_camera->Open();
	// Get camera device information.
	printf("Camera Device Information\n=========================\n");
	printf("Vendor            : %s\n", m_camera->DeviceVendorName.GetValue());
	printf("Model             : %s\n", m_camera->DeviceModelName.GetValue());
	printf("Firmware version  : %f\n", m_camera->DeviceFirmwareVersion.GetValue());
	printf("Default settings\n==================\n");
	printf("Gain              : %.1f\n", m_camera->Gain.GetValue());
	printf("Exposure time     : %.1f\n", m_camera->ExposureTime.GetValue());
	printf("FPS      was      : %.1f\n", m_camera->AcquisitionFrameRate.GetValue());
	printf("MaxNumBuffer      : %d\n", m_camera->MaxNumBuffer.GetValue());
	printf("MaxNumGrabResults : %d\n", m_camera->MaxNumGrabResults.GetValue());
	printf("NumEmptyBuffers   : %d\n", m_camera->NumEmptyBuffers.GetValue());
	printf("GrabLoopPriority  : %d\n", m_camera->GrabLoopThreadPriority.GetValue());
	printf("IntGEPriority     : %d\n", m_camera->InternalGrabEngineThreadPriority.GetValue());
    printf("pixelFormat       : %d\n", m_camera->PixelFormat.GetValue());


	//printf("binningHorizontal : %d\n", m_camera->BinningHorizontal.GetValue());
	//printf("binningVertical   : %d\n", m_camera->BinningVertical.GetValue());

		//m_camera->BinningHorizontal.SetValue(parameters.GetBinningFactor());
		//m_camera->BinningVertical.SetValue(parameters.GetBinningFactor());
		//m_camera->BinningHorizontalMode.SetValue(BinningHorizontalMode_Average);
		//m_camera->BinningVerticalMode.SetValue(BinningVerticalMode_Average);
		//int maxNumBuf = 16;
		//maxNumBuf = 300;   //keep 10 secs
		//m_camera->MaxBufferSize(8834400);



	m_camera->MaxNumBuffer.SetValue(parameters.ImageBuffer());
	parameters.ImageBuffer(m_camera->MaxNumBuffer.GetValue());
	imageStackSize = parameters.ImageBuffer();

		//
		m_camera->AcquisitionFrameRateEnable.SetValue(true);
		m_camera->AcquisitionFrameRate.SetValue(parameters.FrameRate());
		parameters.FrameRate(m_camera->AcquisitionFrameRate.GetValue());
		//
		printf(" FPS      is      : %.1f\n", parameters.FrameRate());
		printf(" MaxNumBuffer     :  %d\n", m_camera->MaxNumBuffer.GetValue());

		//cout << "MaxNumBuffer  : " <<  m_camera->MaxNumBuffer.SetValue(c_countOfImagesToGrab);

		try
		{
			m_camera->EventSelector = EventSelector_ExposureEnd; //AAA
			// Enable it.
			m_camera->EventNotification = EventNotification_On;//EventNotification_GenICamEvent //AAA
			bCameraEvent = true;
		}
		catch (...)
		{
			bCameraEvent = false;
		}

		printf("Width    : %d\n", m_camera->Width.GetValue());
		printf("Height   : %d\n", m_camera->Height.GetValue());
		printf("OffsetX  : %d\n", m_camera->OffsetX.GetValue());
		printf("OffsetY  : %d\n", m_camera->OffsetY.GetValue());
		printf("Gain     : %.1f\n", m_camera->Gain.GetValue());



		m_camera->GainAuto.SetValue(GainAuto_Off, false);
		m_camera->Gain.SetValue(parameters.Gain());
		m_camera->ExposureTime.SetValue(parameters.ExposureTime());
		// Camera might be mono or color
		m_camera->PixelFormat.SetValue(PixelFormat_Mono8);



		//if we have a color camera take only blue channel;
		try
		{
			// Set the red balance ratio
			m_camera->BalanceRatioSelector.SetValue(BalanceRatioSelector_Red);
			m_camera->BalanceRatio.SetValue(0.0);
			m_camera->BalanceRatioSelector.SetValue(BalanceRatioSelector_Green);
			m_camera->BalanceRatio.SetValue(0.0);
			m_camera->BalanceRatioSelector.SetValue(BalanceRatioSelector_Blue);
			m_camera->BalanceRatio.SetValue(1.0);
		}
		catch (...)
		{

		}

		//check if there is a GPIO for Flash triggring
		if (parameters.UvFlash())
		{
			try
			{
				m_camera->ShutterMode.SetValue(ShutterMode_GlobalResetRelease);
				// if the line is GPIO (maybe our camera, only line 3 and 4 is programable)
				m_camera->LineSelector.SetValue(LineSelector_Line4);   // This value could be LineSelector_Line0 - 7,
				m_camera->LineMode.SetValue(LineMode_Output); // set this line as output
				m_camera->LineSource.SetValue(LineSource_ExposureActive);
				m_camera->LineInverter.SetValue(true); //// Enable the line inverter for the I/O line selected
				//
				bFlashTrigger = true;
			}
			catch (...)
			{
				printf("Uv Flash: Failed to set Camera to GlobalReleaseReset, revert to  Global shutter");
				bFlashTrigger = false;
				m_camera->ShutterMode.SetValue(ShutterMode_Rolling);
			}
		}
		else {
			bFlashTrigger = false;
			m_camera->ShutterMode.SetValue(ShutterMode_Rolling);
		}

		//
		int64_t	imgWidth = parameters.ImgWidth();
		int64_t imgHeight = parameters.ImgHeight();
		int64_t	xOffset = parameters.XOffset();
		int64_t yOffset = parameters.YOffset();
		//
		int64_t	maxWidth = m_camera->SensorWidth.GetValue();
		int64_t maxHeight = m_camera->SensorHeight.GetValue();
		/*
		* if no image Size is given in input maximize image
		*/
		m_camera->CenterX = false;
		m_camera->CenterY = false;
		m_camera->OffsetX = 0;
		m_camera->OffsetY = 0;
		//
		m_camera->ReverseX.SetValue(parameters.ReverseX());
		m_camera->ReverseY.SetValue(parameters.ReverseY());
		//
		m_camera->Width = (imgWidth > 0 && imgWidth < maxWidth - xOffset) ? imgWidth : maxWidth - xOffset;
		m_camera->Height = (imgHeight > 0 && imgHeight < maxHeight - yOffset) ? imgHeight : maxHeight - yOffset;
		//
		m_camera->OffsetX = (xOffset > 0) ? xOffset / 8 * 8 : 0;
		m_camera->OffsetY = (yOffset > 0) ? yOffset / 8 * 8 : 0;
		//
		imageWidth = m_camera->Width();
		imageHeight = m_camera->Height();
		//
		m_image.Create(PixelType_Mono8, (int)m_camera->Width(), (int)m_camera->Height());
		// 
		// 
		//
		printf("Width      : %d \n", m_camera->Width());
		printf("Height     : %d \n", m_camera->Height());
		printf("OffsetX    : %d\n", m_camera->OffsetX());
		printf("OffsetY    : %d\n", m_camera->OffsetY());
		printf("Exposure   : %.1f\n", m_camera->ExposureTime());
		printf("Gain       : %f \n", m_camera->Gain());
		//
		//
		//m_camera->AcquisitionMode.SetValue(AcquisitionMode_SingleFrame); 
		//m_camera->AcquisitionStart.Execute();
		m_camera->GrabOne(5000, m_ptrGrabResult);
		//m_camera->StartGrabbing(1);
		//start acquisition thread	
		//m_camera->RetrieveResult(5000, m_ptrGrabResult, TimeoutHandling_Return);
		m_image.AttachGrabResultBuffer(m_ptrGrabResult);
		timestamp = (double)m_ptrGrabResult->GetTimeStamp() / (double)cameraClock;
		cameraClock = parameters.CameraClock();
		//vTimer2 = vTimer2_TICK;	//
		// Determine the sensor readout time under the current settings
		double d = m_camera->SensorReadoutTime.GetValue();

		printf("SensorReadoutTime : %f \n", m_camera->SensorReadoutTime.GetValue());
		printf("ReverseX          : %d \n", m_camera->ReverseX.GetValue());
		printf("ReverseY          : %d \n", m_camera->ReverseY.GetValue());

		printf("MaxNumBuffer      : %d\n", m_camera->MaxNumBuffer.GetValue());
		printf("MaxNumGrabResults : %d\n", m_camera->MaxNumGrabResults.GetValue());
		//
		m_image.Release();
		// this is to release the previus one after all parameter is set, AAA
		m_ptrGrabResult.Release();
		m_camera->Close();
		bReturn = true; break;
	}
#else
	bool bReturn = true;
	bCameraEvent = false;

	parameters.ImageBuffer(1); // 4G
	parameters.FrameRate(50);
	imageWidth = mFF2->GetWidth();
	imageHeight = mFF2->GetHeight();

	timestamp = 0;
	cameraClock = 1;

#endif
	return bReturn;
}

bool CMainFrame::InitPlots()
{
	CRect Rect0;
	GetClientRect(Rect0);
	
	int winId0 = 12000;
	CRect plot_Rect, stats_Rect;

	int iWidth0 = 1; //space from left
	int iWidth1 = (int)((double)(Rect0.Width() - iWidth0) / ((double)nColCount + 1 / 13) / 13);    // space from the right
	int iWidth = (int)(((double)Rect0.Width() - iWidth0 - iWidth1) / nColCount);
	int iHigh0 = 1;  //space from Top
	int ntmSpace = 20; // status bar size
	int iHigh = (int)(((double)Rect0.Height()- ntmSpace - iHigh0) / nRowCount);
	int iLeft, iTop, iRight, iBottom;

	double minX = 0;
	double maxX = 10;

	double minY = 130;
	double maxY = 150;
	nWell = nRowCount*nColCount;
	
	m_Plot = new myPlot[nWell];

	for (int i = 0; i < nWell; i++)
		m_Plot[i].pParent = this;

	CStringA str;

	int iCount = 0;
	for (int iRow = 0; iRow < nRowCount; iRow++) // 8
		for (int iCol = 0; iCol < nColCount; iCol++) //6			
		{
			// set rectangle from left, top, right, and bottom
			iLeft   = iCol* (iWidth + 0) + iWidth0;
			iTop    = iRow* (iHigh + 0)  + iHigh0;
			iRight  = iLeft + iWidth;     // (iCol + 1)*(iWidth + 0) + iWidth0 - 0;
			iBottom = iTop + iHigh;       // (iRow + 1)*(iHigh + 0) + iHigh0 - 0;
			//
			plot_Rect.SetRect(iLeft, iTop, iRight, iBottom);
			//
			m_Plot[iCount].Create(WS_CHILD | WS_VISIBLE, plot_Rect, this, winId0);
			// Set Series
			//Series 0 = Raw Trace
			m_Plot[iCount].SetSerie(0, PS_SOLID, RGB(127, 127, 127), 0.0, 300.0, "Pe");
			//Series 1 = Peaks
			m_Plot[iCount].SetSerie(1, PS_SOLID, RGB(255,   0,   0), 0.0, 300.0, "Pe");
			// Series 2 = runThresh
			m_Plot[iCount].SetSerie(2, PS_SOLID, RGB(  0, 255, 255), 0.0, 300.0, "Pe");
			// Series 3 = lowPass
			m_Plot[iCount].SetSerie(3, PS_SOLID, RGB(  0,   0,   0), 0.0, 300.0, "Pe");
			// Series 4 = runMin
			m_Plot[iCount].SetSerie(4, PS_SOLID, RGB(  0,   0, 255), 0.0, 300.0, "Pe");
			// Series 5 = runMax
			m_Plot[iCount].SetSerie(5, PS_SOLID, RGB(  0,   0, 255), 0.0, 300.0, "Pe");

			m_Plot[iCount].m_bAutoScrollX = TRUE;
			m_Plot[iCount].m_bctlBorder = false;		// control border
			m_Plot[iCount].m_bplotBorder = false;		// plot border
			m_Plot[iCount].m_blegendBorder = false;	// legend border
			
			m_Plot[iCount].m_bPrimaryLegend = false;	// primary legend
			//m_Plot[iCount].SetLegend(0, 1, 0x00 , "TTT");	// primary legend

			m_Plot[iCount].m_bSecondaryLegend = false;	// secondary legend
			
			//str.Format("%c%d", iRow + 'A', iCol+1);
			m_Plot[iCount].m_TextScale = 5.0;
			m_Plot[iCount].m_bStats = true;	            // do print Stats
			//m_Plot[iCount].SetStats(0, (CStringA) sWell.GetAt(iCount+1), "",  0.0);	          // sWell label
			m_Plot[iCount].SetStats(1, "RR",  " = %.1f s",  1.0);	  // RR Distance in sec
			m_Plot[iCount].SetStats(2, "FOC", " = %.1f %%", 1.0);  // Contraction in %
			m_Plot[iCount].SetStats(3, "Base"," = %.1f mm", 3.9); // Base distance in mm
			m_Plot[iCount].SetStats(4, "SNR", " = %.1f dB", 0.0); // Noise in %

			m_Plot[iCount].m_bAxisLY = TRUE;			// left axis
			m_Plot[iCount].m_bAxisRY = false;			// right axis
			m_Plot[iCount].m_bAxisBX = TRUE;			// bottom axis		
			m_Plot[iCount].m_bDrawGrid = false;			// bottom axis		
			//m_Plot[iCount].m_timeaxis.m_maxtime = 60;               // at 50fps 3000frames=60sec
			//SetBXRange(CTime((double)0), CTime((double)100)); 
			m_Plot[iCount].SetBXRange(minX, maxX, TRUE);
			m_Plot[iCount].SetLYRange(minY, maxY);
			//
			//
			iCount++;
		}
	canSize = FALSE;
	return true;
}

bool CMainFrame::resetBuffer()
{
	m_time.reset();
	for (int i = 0; i < nWell; i++)
	{
		m_trace[i].reset();
		m_trace[i].initialize();
	}
	return true;
}

bool CMainFrame::InitBuffer()
{
	//double fps = m_camera->AcquisitionFrameRate.GetValue();
	double fps = parameters.FrameRate();
	
	m_trace = new trace[nWell];
	//printf("InitBuffer: FPS %.1f fastTrace %.1f slowTrace %.1f allTrace %.1f\n", fps, fastTrace*fps, slowTrace*fps, allTrace*fps);
	//
	m_time.resize((int)(allTrace * fps));
	//
	for (int i = 0; i < nWell; i++) {
		m_trace[i].resize((int)(fastTrace * fps), (int)(slowTrace * fps), (int)(allTrace * fps), lowPass, nPeak);  //Takes normalized pixSum
		m_trace[i].slowTrace.num = i;
		m_trace[i].noiseThresh = parameters.NoiseThresh();
		m_trace[i].peakThresh  = parameters.PeakThresh();
		pPixThreshL[i] = pixThresh;
		pPixThreshR[i] = pixThresh;

	}

	//
	return true;
}

//#define USING_ADD_DATA_UPDATE

//#ifdef USING_ADD_DATA_UPDATE
//int gl_Update = 0;
//int gl_UpdateSave = 20;
//#endif

extern CDemoApp theApp;

VOID CALLBACK myStartMeasurement(HWND hwnd, UINT message, unsigned long long idTimer, DWORD dwTime)
{
	theApp.pMainFrame->StartMeasurement();
}

VOID CALLBACK myStopMeasurement(HWND hwnd, UINT message, unsigned long long idTimer, DWORD dwTime)
{
	theApp.pMainFrame->StopMeasurement();
}

VOID CALLBACK myUpdateScreen(HWND hwnd, UINT message, unsigned long long idTimer, DWORD dwTime)
{
	theApp.pMainFrame->updateScreen();
}

void CMainFrame::updateScreen()
{
	currTime = (double)clock() / CLOCKS_PER_SEC;
	CString str;
	//
	if (bMeasuring && !bDrawROIs)
	{
		drawPlots();
		str.Format(L"FOC48: Measurement %d/%d for %8.1f sec of %8.1f sec. Frame %d fps %.1f bDrawRoi %d",
			iMeasurement, nMeasurement, currTime - measurementStartTime, measurementDuration, m_time.NELE(), m_time.NELE() / (currTime - measurementStartTime), bDrawROIs);
	} else if (bGrabbing && bDrawROIs){
		drawROIs();
	    str.Format(L"Next Measurement %d/%d of %8.1f starts in %8.1f sec", iMeasurement, nMeasurement, measurementDuration, measurementStartTime - currTime);
	} else {
		str.Format(L"FOC48: Next Measurement %d/%d of %8.1f starts in %8.1f sec", iMeasurement, nMeasurement, measurementDuration, measurementStartTime- currTime);
		
	}
	/*
	CString strPercentDone;	
	strPercentDone.Format(L"%.1f", (currTime - measurementStartTime)/saveTime);
	m_wndStatusBar.SetPaneText(m_wndStatusBar.CommandToIndex(ID_SEPARATOR), strPercentDone);
	m_wndStatusBar.UpdateWindow();
	*/
	SetWindowText(str);
}

void CMainFrame::importParameters()
{
	bSimulate           = parameters.Simulate();
	bAutoSave           = parameters.AutoSave();
	//
	bUvFlash            = parameters.UvFlash();
	bStimulate          = parameters.Stimulate();
	rrVal               = parameters.RR();

	bExternalTrigger    = parameters.ExternalTrigger();

	measurementDuration = parameters.Duration();
	measurementInterval = parameters.Interval() * 60 * 60;   //convert from h to sec
	nMeasurement        = parameters.Measurements();	
	pixThresh           = parameters.PixThresh();
	saveTime            = parameters.SaveTime();
	pixUse              = parameters.PixUse();
	bReverseX           = parameters.ReverseX();
	bReverseY           = parameters.ReverseY();
	suffix				= parameters.Suffix();
	pixel_per_mm        = parameters.Pixel_per_mm();
	//
	expName             = parameters.ExpName();

	stimulationBoard.setPort(parameters.ArduinoCOM());

	bColorByTwitch   = parameters.ColorByTwitch();
	bColorByBase     = parameters.ColorByBase();

	myPreSetMenu(parameters.get_mySet_mskEHM());

#ifdef USE_DB
	Db.host.host = parameters.DbHost();   //url or IP to Database
	Db.host.port = parameters.DbPort();   //database Port e.g. 3306
	Db.host.name = parameters.DbName();   //database Name e.g. contractionDb
	Db.host.user = parameters.DbUser();   //database Username with write priviledges tp forces table
	Db.host.pass = parameters.DbPass();   //database password, TDO: store secretely:)
#endif
}

int nFF = 0;
int nSS = 0;
// on return, nSSx is the current us
// when both input is 0, will init the out as 0
// otherwise, return the delta time in us
int __fastcall myTimePass_us(int &nSSx, const int nFFx)
{
	std::chrono::system_clock::time_point aNow = std::chrono::system_clock::now();
	long long aa = aNow.time_since_epoch().count();	// 0.1 us as a count
	int n = aa & 0x0fffffff;

	//SYSTEMTIME st;
	//GetSystemTime(&st); //GetLocalTime(&lt);
	//int n = st.wSecond;
	//n = n * 1000 + st.wMilliseconds;
	int dn;
	if (nSSx == 0 && nFFx == 0){
		dn = 0;
	}
	else{
		dn = n;
		if (nSSx > n){
			dn += 0x10000000;	//dn = n + 60 * 1000;
		}
		dn -= nSSx;
	}
	nSSx = n;
	return dn;
}

VOID CALLBACK myCheckFrame(HWND hwnd, UINT message, unsigned long long idTimer, DWORD dwTime)
{
	CMainFrame *pMain = dynamic_cast<CMainFrame *>(theApp.m_pMainWnd); if (pMain == NULL)return;	
	if (pMain->bGrabbing) {
		int dn = myTimePass_us(nSS, nFF);	// get time passed in us	
		int nFF_Count = 0;

#ifndef MY_NO_pylon		

		pMain->m_camera->RetrieveResult(5000, pMain->m_ptrGrabResult, TimeoutHandling_Return);
		if (pMain->m_ptrGrabResult->GrabSucceeded())
		{
			pMain->addFrameBuffer(pMain->m_ptrGrabResult);
			//theApp.pMainFrame->m_image.AttachGrabResultBuffer(theApp.pMainFrame->m_ptrGrabResult);
			pMain->analyzeFrame();
			nFF_Count++;
		}

#else

		bool bGet = mFF2->myGetBlock(nFF);  // get image, nFF is index, 0-based
		if (bGet){
			//pMain->drawROIs();			// if image is correctly get, display it
			pMain->addFrameBuffer(mFF2->pMEM);   // contain recalculate routing
			//theApp.pMainFrame->m_image.AttachGrabResultBuffer(theApp.pMainFrame->m_ptrGrabResult);
			pMain->analyzeFrame();   // iFrame will be added
			nFF_Count++;
		}

#endif
		// information
		wchar_t wStr[16];
		swprintf_s(wStr, 14, L"%d", dn / 10000); pMain->myShowInStatusBar(wStr, 3); nFF += nFF_Count;
		swprintf_s(wStr, 14, L"%d%c", nFF, (nFF_Count > 0) ? ' ' : '*');	pMain->myShowInStatusBar(wStr, 2);
	}
}

#ifndef MY_NO_pylon

bool CMainFrame::addFrameBuffer(Pylon::CGrabResultPtr& m_result_ptr)
{	
	m_image.AttachGrabResultBuffer(m_result_ptr);
#ifndef USE_MY_MEM_FILE
	
#else
	mySetMEMData((char *)m_image.GetBuffer());
#endif	//USE_MY_MEM_FILE

	myNewMethodPreCheck();

	imageStackIndx = imageStackNum % imageStackSize;
	imageStackNum++;
	int nNext = imageStackNum % imageStackSize;

	if (imageStackNum >= imageStackSize)
		m_imageStack[nNext].Release();
	
	m_imageStack[imageStackIndx] = m_result_ptr;

	return true;
}

#else

bool CMainFrame::addFrameBuffer(const unsigned char *pBuf)
{
#ifndef USE_MY_MEM_FILE

#else
	mySetMEMData((const char *)pBuf);
#endif	//USE_MY_MEM_FILE

	myNewMethodPreCheck();

	imageStackNum++;
	imageStackIndx = imageStackNum % imageStackSize; // infuture, user can delete

	//if (imageStackNum > imageStackSize)
	//	m_imageStack[imageStackIndx].Release();

	//m_imageStack[imageStackIndx] = m_result_ptr;
	
	return true;
}

#endif

void __fastcall CMainFrame::myNewMethodPreCheck(void)
{
	if ((mskEHM & MY_CAL_MASK) != 0)
	{
		myCal_1(); mskEHM &= ~MY_CAL_MASK;
	}
	else if ((mskEHM & MY_NEW_METHOD_MASK) != 0 && (!eFit->isINIT_OK()))   // user select the new methods, but we never started
	{
		myCal_1(); mskEHM &= ~MY_CAL_MASK;
		if (!eFit->isINIT_OK()){			// we try to start, but still not start, we toggle back
			myNewMethodToggle();
		}
	}
}
void CMainFrame::analyzeFrameSimulate() 
{
	iFrame++;
	timestamp = (double) clock(); //time in seconds
	//
	m_time.put(timestamp);
	//
	simulateDistance();
	//calcDistance();
	findPeaks();
	AddData();
	Sleep(5);
}

void CMainFrame::analyzeFrame()
{
	iFrame++;
	//m_image.AttachGrabResultBuffer(m_ptrGrabResult);
#ifndef MY_NO_pylon	
	timestamp = (double) m_ptrGrabResult->GetTimeStamp() / (double) cameraClock;
#else
	timestamp = (double)(nFF * 20)*0.001;  // turn to Second
#endif
	//
	m_time.put(timestamp);
	//
	calcDistanceP();
	//calcDistance();
	findPeaks();
	AddData();
	//
	// keep image in circ_buffer
	//

	// following code has beed in addFrameBuffer()
	//imageStackNum++;
	//imageStackIndx = (imageStackIndx + 1 == imageStackSize) ? 0 : imageStackIndx + 1;
	////	
	//m_imageStack[imageStackIndx]=m_ptrGrabResult;	
}

void CMainFrame::AddData()
{
	static BOOL pros = { FALSE };
	if (! pros){
		pros = TRUE;
		// fastTrace conains filtered Data, with 
		// m_Plot[i].AddPoint(0, timestamp, *m_trace[i].fastTrace.getPtr());			
		for (int i = 0; i < nWell; i++) {
			m_Plot[i].AddPoint(0, timestamp, *m_trace[i].rawTrace.getPtr()); 
			//m_Plot[i].AddPoint(2, timestamp, *m_trace[i].slowTrace.getPtr());
			m_Plot[i].AddPoint(2, timestamp, *m_trace[i].getCutOffPtr());
			m_Plot[i].AddPoint(3, *m_trace[i].lowPass.centerTimePtr(), *m_trace[i].lowPass.centerPtr());
			m_Plot[i].AddPoint(4, timestamp, *m_trace[i].slowTrace.maxPtr());
			m_Plot[i].AddPoint(5, timestamp, *m_trace[i].slowTrace.minPtr());
		}
		
		//printf("max %8.3f trace %8.3f min %8.3f peakCut %8.3f noiseCut %8.3f cutOff %8.3f\n",
		//	m_trace[10].slowTrace.MAX(), m_trace[10].rawTrace.getLast(), m_trace[10].slowTrace.MIN(),
		//	m_trace[10].peakCut, m_trace[10].noiseCut,  *m_trace[10].getCutOffPtr() );
		//
		pros = FALSE;
	}
}

void CMainFrame::UpdateView()
{
	printf("UpdateView() Frame: %d\n", iFrame);
	//
	static BOOL pros = { FALSE };
	if (!pros){
		pros = TRUE;
		//double dStart = (double)(clock()) / (double)CLOCKS_PER_SEC * 1000;
		//
		for (int i = 0; i < nWell; i++)
		{
			m_Plot[i].Invalidate();
			m_Plot[i].UpdateWindow();
		}
		//double dNow = (double)(clock()) / (double)CLOCKS_PER_SEC * 1000;
		pros = FALSE;
		//printf("Update %d Plots took %G ms\n", nWell, dNow - dStart);
	}
}

//void CMainFrame::OnUpdatePane(CCmdUI* pCmdUI)
//{
//	pCmdUI->Enable(); // default is TRUE
//}

/////////////////////////////////////////////////////////////////////////////
// CDemoView drawing

//void CMainFrame::OnDraw(CDC* pDC)		// this is called by internal OnPaint
//{
//	//CDemoDoc* pDoc = GetDocument();  // AAA seems no use
//
//	//CFrameWnd::OnDraw
//}

//BOOL CMainFrame::OnEraseBkgnd(CDC* pDC){
//	return TRUE;
//}

//void CMainFrame::OnPaint(){
//	//CFrameWnd::OnPaint();
//CPaintDC dc(this);
//OnPrepareDC(&dc);
//OnDraw(&dc);
//}

#define SC_MAXIMIZE2		0xF032	// when double click on the title bar
#define SC_RESTORE2			0xF122
#define SC_SZLEFT2			0xF001	//Resize from left
#define SC_SZRIGHT2			0xF002 //(SC_SZRIGHT, Resize from right)
#define SC_SZTOP2			0xF003 //(SC_SZTOP, Resize from top)
#define SC_SZTOPLEFT2		0xF004 //(SC_SZTOPLEFT, Lock the bottom right corner of the form, the top left corner move for resize)
#define SC_SZTOPRIGHT2		0xF005 //(SC_SZTOPRIGHT, Same from bottom left corner)
#define SC_SZBOTTOM2		0xF006 //(SC_SZBOTTOM, Lock top right and left border, resize bottom)
#define SC_SZBOTTOMLEFT2	0xF007 //(SC_SZBOTTOMLEFT, Lock top and right border, resize other border)
#define SC_SZBOTTOMRIGHT2	0xF008 //(SC_SZBOTTOMRIGHT, Lock left and top border and resize other)
#define SC_SZDRAG			0xF009 //(SC_SIZE | 0x9, Drag from anywhere)

void CMainFrame::OnSysCommand(UINT nID, LPARAM lParam)
{
	//	
	if (nID <= SC_SZBOTTOMRIGHT2 && nID >= SC_SIZE){
		CRect Rect0;
		GetClientRect(Rect0);
		CMainFrame::OnSize(7, Rect0.Width(), Rect0.Height());
		//CFrameWnd::OnSysCommand(nID, lParam);    // no need this line
	}
	else{
		switch (nID)
		{
		case SC_MAXIMIZE:;
		case SC_MAXIMIZE2:;
			while (1){
				CRect Rect0;
				GetClientRect(Rect0);
				CMainFrame::OnSize(SIZE_MAXIMIZED, Rect0.Width(), Rect0.Height());
				break;
			}
			break;
		case SC_RESTORE:;
		case SC_RESTORE2:;
			while (1){
				CRect Rect0;
				GetClientRect(Rect0);
				CMainFrame::OnSize(SIZE_RESTORED, Rect0.Width(), Rect0.Height());
				break;
			}
			break;
		default:
			CFrameWnd::OnSysCommand(nID, lParam);
#ifdef _DEBUG
			if (nID != SC_CLOSE){
				wchar_t wStr[80];
				swprintf_s(wStr, 80, L"OnSysCommand: nID=0x%08x, lParam=0x%08x", nID, lParam); myShowInStatusBar(wStr);
			}
#endif
		}
	}
}

void CMainFrame::OnSize(UINT nType, int cx, int cy)
{
	//if (nType!=0&&cx!=0&cy!=0)
	CFrameWnd::OnSize(nType, cx, cy);
	if (canSize){	
		canSize = false;
		double topSpace = 1; //space from Top
		double leftSpace = 1; //space from left
		double rightSpace;
		double dWidth;
		double dHeight;
		while (1){
			CRect Rect0;
			GetClientRect(Rect0);
			double btmSpace = 20; // status bar size		
			rightSpace = (double)(Rect0.Width() - leftSpace) / ((double)nColCount + 1 / 13) / 13;    // space from the right
			dWidth = ((double)(Rect0.Width()) - leftSpace - rightSpace) / (double)nColCount;
			dHeight = ((double)(Rect0.Height()) - topSpace - btmSpace) / (double)nRowCount;
			break;
		}
		double dLeft, dTop, dRight, dBtm;
		CRect Rect;
		int iCount = 0;
		for (int iRow = 0; iRow < nRowCount; iRow++) // 8
			for (int iCol = 0; iCol < nColCount; iCol++) //6			
			{
				dLeft  = (double) iCol * dWidth + leftSpace;
				dTop   =  (double) iRow * dHeight + topSpace;
				dRight = dLeft + dWidth;
				dBtm   = dTop + dHeight;

				Rect.SetRect((int)(dLeft + 0.5e-7), (int)(dTop + 0.5e-7), (int)(dRight + 0.5e-7), (int)(dBtm + 0.5e-7));
				//Rect.SetRect(dLeft, dTop, dRight, dBtm);
				m_Plot[iCount].MoveWindow(Rect);
				//m_Plot[iCount].myDraw();
				iCount++;
			}
		canSize = true;
		//this->UpdateWindow();
	}
}

/////////////////////////////////////////////////////////////////////////////
// CMainFrame diagnostics

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CFrameWnd::Dump(dc);
}

#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CMainFrame message handlers
