// MainFrm.h : interface of the CMainFrame class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_MAINFRM_H__BB278709_A140_11D1_BEB4_006008918F1C__INCLUDED_)
#define AFX_MAINFRM_H__BB278709_A140_11D1_BEB4_006008918F1C__INCLUDED_

//#if _MSC_VER >= 1000
//#pragma once
//#endif // _MSC_VER >= 1000


#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions
#include <afxdialogex.h>

class trace;
class myPlot;
#include "circular_buffer.h"
#include "Parameters.h"
#include "USBstimulationBoard.h"

// Following line is used for computer that has no pylon installed, to use a memory recorded file.

// #define MY_NO_pylon

// Following line is for memory recorded files.
//#define USE_MY_MEM_FILE	1

#include "contractionDB.h"

#ifndef MY_NO_pylon

#include <pylon\PylonIncludes.h>
//#include <pylon\AviCompressionOptions.h>
#include <pylon\PylonGUI.h>
//#include <pylon\usb\BaslerUsbCamera.h>
#include <pylon\usb\BaslerUsbInstantCamera.h>

using namespace Pylon;
using namespace Basler_UsbCameraParams;
using namespace Basler_UsbStreamParams;

#else

#endif // MY_NO_pylon

// ---------------------------------------------------------------------------
//
// Without any setting, myDBG_print or printf will use Debug_output_window
//
// if using an edit control e.g. IDC_EDIT0, set as:
//
// hDBG = reinterpret_cast<unsigned long long>(GetDlgItem(IDC_EDIT0)->m_hWnd);
//
// after that, all printf or myDBG_print will be displaied in that edit control.
//
// ---------------------------------------------------------------------------

VOID CALLBACK myCheckFrame		(HWND hwnd, UINT message, unsigned long long idTimer, DWORD dwTime);
VOID CALLBACK myStartMeasurement(HWND hwnd, UINT message, unsigned long long idTimer, DWORD dwTime);
VOID CALLBACK myStopMeasurement	(HWND hwnd, UINT message, unsigned long long idTimer, DWORD dwTime);
VOID CALLBACK myUpdateScreen	(HWND hwnd, UINT message, unsigned long long idTimer, DWORD dwTime);

#define	MY_SHOW_OBJ_KASK	0x00000001
#define	MY_CAL_MASK			0x00000010		// re-calculate
#define	MY_NEW_METHOD_MASK	0x00000100
#define	MY_NEW_METHOD_DOING	0x00000200		// if it is really using
#define	MY_DSP_POLE_ENABLE	0x00001000		// when ENABLE is toggled, set OK to off
#define MY_DSP_POLE_OK		0x00002000
#define	MY_DSP_POLE			(MY_DSP_POLE_ENABLE|MY_DSP_POLE_OK)
#define	MY_MOUSE_TRAP_EX	0x00010000

//extern CDemoApp theApp;


VOID CALLBACK myCheckFrame(HWND hwnd, UINT message, UINT idTimer, DWORD dwTime);
VOID CALLBACK myStartMeasurement(HWND hwnd, UINT message, UINT idTimer, DWORD dwTime);
VOID CALLBACK myStopMeasurement(HWND hwnd, UINT message, UINT idTimer, DWORD dwTime);
VOID CALLBACK myUpdateScreen(HWND hwnd, UINT message, UINT idTimer, DWORD dwTime);


class CMainFrame : public CFrameWnd
{
public:
	CMainFrame(CWinApp* p);
protected:
	DECLARE_DYNAMIC(CMainFrame)

public:
	~CMainFrame();

	CWinApp* pParent;
	virtual bool InitCamera(); 
	virtual bool InitStream();
	virtual bool InitPlots();
	virtual bool InitROIs();
	virtual bool InitBuffer();
	virtual bool InitStimulationBoard();
	virtual bool InitDb();
	void OnClose();
	
	int nSSB_0[8];
	int mskEHM = 0;
	int nWellXY[48][2]; // XL,YL
	double dWellPoleXY[48][4]; // XL,YL,XR,YR
	int PVnLeftX, PVnTopY, PVnXX, PVnYY, PVnWidth, PVnHeight;
	double PVdRX, PVdRY;
	//int nUseMethod = 0;
	virtual void __fastcall myShowInStatusBar(const wchar_t *pStr, const int nID = 0);
	virtual bool __fastcall myDllTestLoad(void);
	virtual bool __fastcall mySetMEMFile(void);
	virtual bool __fastcall mySetMEMData(const char *pBuf);
	virtual void __fastcall mySetMEMUser(void);
	virtual void __fastcall myCal_nSSB(int *nSSB);
	virtual void __fastcall myCal_0(void);
	virtual void __fastcall myCal_1(void);
	virtual int __fastcall myToggle_Ex(int &mskVal, const unsigned int nID, const int nMSK);
	virtual int __fastcall myViewTagToggle(void);
	virtual int __fastcall myNewMethodToggle(void);
	virtual int __fastcall myDspPoleToggle(void);
	virtual int __fastcall myMouseTrapToggle(void);
	virtual void __fastcall myDrawTag(HDC hdc, int nLeftX, int nTopY, int nXX, int nYY, int nW, int nH);
	virtual void __fastcall myNewMethodPreCheck(void);
	virtual void __fastcall myNewMethodDSPCheck(void);
	virtual void __fastcall myPreSetMenu(const int mskEHM_in);
	virtual void __fastcall myReCalImgMetrix(RECT &irr);

	int	nFrameAddIndex;
	virtual bool resetBuffer();

	virtual ULONG GetGestureStatus(CPoint ptTouch);

	//void getVideoData();
	void AddData();
	void findPeaks();
	void writeTraces(CString fName);
	void writeStats(CString fName);
	void saveTraceStats(CString fName);
	void UpdateView();

	void StimulationBoard_WR(const char* cIn);

	void OnSysCommand(UINT nID, LPARAM lParam);
	//void OnMaximize();
	//
	COLORREF numToRGB(double val, double green, double yellow, double red);

	USBstimulationBoard stimulationBoard;

	BOOL bSimulate = false;
	//Define Timers
	/*Current Time in Seconds*/
	double currTime = (double)clock() / CLOCKS_PER_SEC;  //Current Time in Seconds
	double measurementStart = currTime;
	double measurementStop = currTime + 96000;  // Initially measure 24h
	double measurementInterval = 3600.0;         // in seconds
	double measurementDuration = 130.0;         // in Seconds
	double saveTime            = 120.0;         // Savetime in Seconds	

	int checkFrameInterval = 3;     //Data   Analysis in ms
	static const int updateScreenInterval = 500;   //Update Screen in  ms

	int  iMeasurement  = 1;
	int  nMeasurement  = 1;
	bool bMeasuring    = false;
	bool bGrabbing     = false;
	//
	bool bFlashTrigger = false;
	bool bExternalTrigger = false;
	bool bToDb = false;
	bool bStimulate = false;
	int  rrVal    = 0;	// rr is too simple that will conflict with others

	static const int checkFrameTimer = 701;
	static const int updateScreenTimer = 702;
	static const int measurementStartTimer = 703;
	static const int measurementStopTimer = 704;
	static const int ID_WAITDLG = 705;


	//class CProgressCtrl pWaitBar;
	CDialog* pWaitDlg = NULL;

	void StartMeasurement();
	void StopMeasurement();
	
	void StartCamera();
	void StopCamera();
	
	void updateScreen();
	void analyzeFrame();
	void analyzeFrameSimulate();
	
	void importParameters();

	int* wRowSumR;
	int* RowSumR;
	int* wRowSumL;
	int* RowSumL;

	int xGrid, yGrid;

	//void cutROIs();
	int nDrawROIs = 0;
	void drawROIs(bool bBK = false);
	void drawPlots();
	double simulateDistance();
	double calcDistance();
	//double calcDistLine();
	long long int calcImageSum();


	double calcDistanceP();
	unsigned char __fastcall myGetImgXY_iW(const unsigned char* pMEM, const int iW, const int n, double& dX, double& dY);

	//BOOL    SetStimulationBoardStatus(CString sStatus);
	//CString GetStimulationBoardStatus();
	//int     FindStimulationBoard();

#ifndef MY_NO_pylon
	CPylonImageWindow* h_RoiPreview;
#endif
	//
	void mySetTimer();

	//afx_msg void OnTimer(UINT_PTR nIDEvent);

	myPlot* m_Plot;
	trace* m_trace;
	circular_buffer m_time;
	double timestamp;
	double measurementStartTime = 0.0;
	double fps = 30;
	double fastTrace = 0.33;  //defines min Peak distance in sec = 3Hz
	double slowTrace = 10;    //defines time to adapt PeakCut (~30sec)
	double allTrace = 3600;   // defines complete trace stored in memory and fileSave intervall
	//	
	//
	int lowPass = 3;     //average over frames           
	int nPeak = 11;     //numerb of peaks for averaging
	//
	uint64_t cameraClock;

	int vTimer2;
	// Attributes

	int nColCount = 6;
	int nRowCount = 8;
	int nWell;
	int iFrame = 0;

	//int iLast;
	//double tLast;
	//double tLastStamp;

	double ratioRows = 0.3;
	double ratioCols = 0.5;

	BOOL canSize = FALSE;
	BOOL bDrawROIs = FALSE;

	//static const int ChartUpdateTimer = 2;
	//static const int ChartUpdateInterval = 500;

	int  pixThresh = 127;
	int pPixThreshL[48];
	int pPixThreshR[48];
	double pixUse = 0.03;    // ratio of brightest pixels to be used, default = 3%
	bool bReverseX = false;
	bool bReverseY = false;


	int roiRows;
	int roiCols;
	int nRoiPix;
	int * roiPixOffset;   //array of 96
	int * roiPixOffsetX;   //array of 96
	int * roiPixOffsetY;   //array of 96
	int * roiXpos;        //array of roiPix
	int * roiPixList;

	CStringArray sWell;
	//
	//Measurement ID Suffix
	CString suffix  = "";

	Parameters parameters;
	int       imageStackSize = 128;
	int       imageStackIndx = 0;
	long long imageStackNum = 0;

#ifndef MY_NO_pylon
	Pylon::CBaslerUsbInstantCamera *m_camera;
	//aaa m_ptrGrabResult;
	Pylon::CGrabResultPtr m_ptrGrabResult;

	Pylon::CGrabResultPtr m_imageStack[5000];
	Pylon::CDeviceInfo m_info;
	//
	Pylon::CPylonImage m_image;
	//
	Pylon::CPylonBitmapImage bmpPreView;
	//

	bool addFrameBuffer(Pylon::CGrabResultPtr &m_result_ptr);
#else
	bool addFrameBuffer(const unsigned char *pBuf);
#endif
	
#ifdef USE_DB
	contractionDB Db;
#endif

	double xGrid_mm =18.0;
	double pix2mm   =0.0;

	//full-size image
	// Overrides
	int64_t imageWidth = 0;
	int64_t imageHeight = 0;	

	//unsigned char pixThresh = 127;
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMainFrame)
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//}}AFX_VIRTUAL

	bool bAutoSave  = false;
	bool bAutoClose = false;
	bool bUvFlash   = false;
	//bool bStimulate = false;
	bool bColorByTwitch;
	bool bColorByBase;
	double pixel_per_mm;
	
	COLORREF numToRYGYR(double V, double Rl, double Yl, double G, double Yh, double Rh);
	COLORREF numToRYG(double V, double R, double Y, double G);

	void saveTracesDb();

	//int nWell = 0;

	CString meaName;
	CString expName;
	CString filePath, rootName;

	bool bFilePathSet = false;

	bool SetSavePath(); 
	bool Save(bool bInteractive);
	bool saveAvi(CString fName);
	bool saveTiffs(CString fName);
	
	bool bKeepUncompressedVideo = true;
	bool compressFile(CString fName, bool bKeepSource);

	void OnSize(UINT nType, int cx, int cy);
	//void OnDraw(CDC* pDC);
	//void OnUpdatePane(CCmdUI* pCmdUI);
	//void OnPaint(void);
	//BOOL OnEraseBkgnd(CDC* pDC);

	//BOOL ReadXML(CString inParFile);

	//afx_msg void OnMaximizeWindow();
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);	
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point){Save(true);}
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point)  {Save(true);}
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:  // control bar embedded members
	CStatusBar      m_wndStatusBar;
	CToolBar        m_wndToolBar;
	// Generated message map functions
	//{{AFX_MSG(CMainFrame)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	DECLARE_MESSAGE_MAP()
};

#endif // !defined(AFX_MAINFRM_H__BB278709_A140_11D1_BEB4_006008918F1C__INCLUDED_)




