//*******************************************************************************************************/
//* FileName        : myPlot.h
//*
//* Description	    : Real Time Plot for MFC
//*
//* Contents:		: axis		y (x) axis info.
//*					  timeaxis	time axis info
//*					  legend	legend info.
//*					  serie		data serie info & array
//*					  myPlot	The plot itself.
//*
//* Author          : Jan Vidar Berger
//*******************************************************************************************************/
#if !defined(AFX_DQPLOT_H__0D536D37_5CF1_11D1_AED1_0060973A08A4__INCLUDED_)
#define AFX_DQPLOT_H__0D536D37_5CF1_11D1_AED1_0060973A08A4__INCLUDED_

#pragma once


// myPlot.h : header file
//
#define MAXLEGENDS	1
#define MAXSTATS	5
#define MAXSERIES	6



// From Resurce.h

#ifdef APSTUDIO_INVOKED
#ifndef APSTUDIO_READONLY_SYMBOLS

#define _APS_NEXT_RESOURCE_VALUE        129
#define _APS_NEXT_COMMAND_VALUE         32771
#define _APS_NEXT_CONTROL_VALUE         1000
#define _APS_NEXT_SYMED_VALUE           101
#endif
#endif






//*******************************************************************************************************/
//* simple data value struct. used in dynamic array
//*******************************************************************************************************/
typedef struct _value{
	double	dValue;
	double	ValueTime;
}value;

//*******************************************************************************************************/
//* non-time axis. used for left and right y axis. might be used for x as well.
//*******************************************************************************************************/
class  axis
{
public:
	CString szTitle;
	double	minrange;
	double	maxrange;

	double	m_dValuePrPixel;

	axis()
	{
		szTitle = "Title";
		minrange = 100.0;
		maxrange = 200.0;

		m_dValuePrPixel = 1;
	};
};

//*******************************************************************************************************/
//* time axis
//*******************************************************************************************************/
class  timeaxis
{
public:
	CString		m_szTitle;		// time axis title;
	double		m_mintime;		// min time
	double		m_maxtime;		// max time
	int			m_iTimeMode;	// axis grid and legend interval index

	double		m_dSecondsPrPixel;

	timeaxis()
	{
		m_szTitle = "Time";
		m_mintime = 0;
		m_maxtime = 60;
		m_iTimeMode = 0;
		m_dSecondsPrPixel = 1;
	}
};

//*******************************************************************************************************/
//* legend
//*******************************************************************************************************/
class  legend
{
public:
	BOOL		m_bIAmInUse;
	COLORREF	m_color;		// legend color code
	int			m_istyle;
	CString		m_szTitle;		// legend title

	legend(){
		m_bIAmInUse = FALSE;
		m_color = 0;
		m_istyle = PS_SOLID;
		m_szTitle = "";
	}
};

class stats
{
public:
	BOOL		m_bIAmInUse;
	CStringA	m_szTitle;		// stats title
	double      dValue;
	CStringA    m_format;         // format of Value

	stats(){
		m_bIAmInUse = FALSE;
		m_szTitle = "";		
		dValue    = 0.0;
		m_format  = "%.1f";
	}

	CString getString(){
		CStringA str;
		str.Format(m_format, dValue);
		str = m_szTitle + str;
		return CString(str);
	}
};


//*******************************************************************************************************/
//* data serie
//*******************************************************************************************************/
class  serie
{
public:
	BOOL		m_bIAmInUse;
	COLORREF	m_color;		// serie line color
	int			m_iLineStyle;	// line style
	BOOL		m_bRightAxisAlign; // align to right axis
	value	*	m_pvalues;		// value array
	long		m_lNoValues;	// no values (used for array size)
	long		m_lbegin;		// list begin
	long		m_lend;			// list end
	//	CPen		m_pen;			// pre-calculated pen (for speed)

	serie();
	~serie();

	void AddPoint(double &valuetime, double &y);
	void Reset();
};

class CMainFrame;

//*******************************************************************************************************/
//* Class           : myPlot
//*
//* Base Class      : public CWnd
//*
//* Description     : Plot Component.
//*
//*					  This is a standard plot and can be used for any application.
//*
//*						1. A special 'autoscroll'mode exist for real time plots.
//*						2. Only a minimum of features are implemented.
//*						3. Series and legends are separated and must be set up individually.
//*						4. A set of defines (see top of file) are used to set the max array sizes.
//*						5. Only time are supported as x-axis.
//*						6. A large range of pre-calculated values are used for maximum speed.
//*
//* Author          : Jan Vidar Berger
//*******************************************************************************************************/
class  myPlot : public CWnd
{
	// Construction
public:
	myPlot(CMainFrame *p=NULL);
	virtual ~myPlot();

	// Attributes
public:

	CMainFrame *pParent;

	CRect		m_ctlRect;			// control rect
	CRect		m_clientRect;		// ctlRect - borderspace
	CRect		m_plotRect;			// clientRect - margins

	CRect		m_legendRect;		// any rect within clientRect
	CRect		m_statsRect;		// any rect within clientRect

	CRect		m_axisLYRect;		// Left axisi rect
	CRect		m_axisRYRect;		// right y axis
	CRect		m_axisBXRect;		// bottom x axis

	int			m_iMleft;			// left margin
	int			m_iMright;			// right margin
	int			m_iMtop;			// top margin
	int			m_iMbottom;			// bottom margin

	COLORREF	m_ctlBkColor;		// control background color
	COLORREF	m_plotBkColor;		// plot area bacground color //AAA
	COLORREF	m_plotBdColor;		// plot area boarder color //AAA
	COLORREF	m_legendBkColor;	// legend background color
	COLORREF	m_gridColor;		// grid line color

	

	BOOL		m_bctlBorder;		// control border
	BOOL		m_bplotBorder;		// plot border
	BOOL		m_blegendBorder;	// legend border
	BOOL		m_bPrimaryLegend;	// primary legend
	BOOL		m_bSecondaryLegend;	// secondary legend
	BOOL		m_bStats;			// show stats
	BOOL		m_bAxisLY;			// left axis
	BOOL		m_bAxisRY;			// right axis
	BOOL		m_bAxisBX;			// bottom axis
	BOOL		m_bAutoScrollX;		// automatic x range scrolling
	BOOL		m_bSimMode;			// simulate data input
	BOOL		m_bDrawGrid;		// simulate data input

	double		m_dNoData;			// No Data Value (used for gaps)

	legend		m_primarylegends[MAXLEGENDS];
	legend		m_secondarylegends[MAXLEGENDS];
	stats		m_stats[MAXSTATS];

	serie		m_series[MAXSERIES];

	axis		m_leftaxis;			// left axis
	axis		m_rightaxis;		// right axis
	timeaxis	m_timeaxis;			// bottom axis

	CPoint		*pLineArray;		// pre-calculated when new data are entered into the system
	long		lArraySize;			// current size of pLineArray

	CFont		m_font;
	LOGFONT		m_logFont;
	LOGFONT		m_zoomFont;
	double      m_zoomFactor = 0.2;
	double		m_dzoom;
	int			m_TextHeight;
	double      m_TextScale = 4.0;

	// Operations
public:
	BOOL		Create(DWORD dwstyle, CRect &rect, CWnd *pParent, UINT id);
	void		MoveWindow(CRect &Rect);

	virtual void myDraw() { OnPaint(); };   // TM Added to fix Maximize w/o draw problem
	//
	virtual void Draw(CDC * dc);			// Draw the entire plot
	virtual void DrawBasic(CDC * dc);		// Draw plot basics
	virtual void DrawPlot(CDC * dc);		// Draw the plot series
	virtual void DrawSerie(CDC *dc, int serie);
	//virtual void DrawSerie_old(CDC *dc, int serie);
	virtual void DrawLegendShadow(CDC * dc);// Draw legend shadows
	virtual void DrawLegend(CDC * dc);		// Draw legends
	virtual void DrawStats(CDC * dc);		// Draw legends

	virtual void DrawGrid(CDC* dc);		// Draw grids
	virtual void DrawYAxisGrid(CDC * dc);
	virtual void DrawXAxisGrid(CDC * dc);

	virtual void ComputeRects(BOOL bInitialize);

	virtual BOOL AddPoint(int serie, double &valuetime, double &y);
	virtual void SetBXRange(double &fromtime, double &totime, BOOL bMove = TRUE);
	virtual void SetLYRange(double &minrange, double &maxrange);
	virtual void SetRYRange(double &minrange, double &maxrange);
	virtual void Reset();

	virtual void SetSerie(int s, int style, COLORREF color, double minrange, double maxrange, const char *szTitle, BOOL Rightalign = FALSE);
	virtual void SetLegend(int l, int style, COLORREF color, const char *text);
	virtual void SetStats(int l, CStringA text, CStringA fmt, double dVal);
	virtual void SetBXTitle(const char *title);
	virtual void SetLYTitle(const char *title);
	virtual void SetRYTitle(const char *title);
public:
	
	// Generated message map functions
protected:
	//{{AFX_MSG(myPlot)
	double topSpace = 5;
	double btmSpace = 35;
	//
	virtual ULONG GetGestureStatus(CPoint ptTouch); 
	afx_msg void OnPaint();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);


	//}}AFX_MSG
	BOOL myPlot::OnEraseBkgnd(CDC* pDC);
	DECLARE_MESSAGE_MAP()
};

//---------------------------------------------------------------------------
// e.g. following call will display text on x:17, y:0 with default setting
//
// #include "myTRANS.h"
// myDrawTextAlpha(GetDC(ed1->Handle),17,0, "TEST: 200.12");
//
//---------------------------------------------------------------------------


typedef union _MY_BGRA {
	unsigned long ulBGR;
	unsigned int  unBGR;
	COLORREF rBGR;
	struct{
		unsigned char b;
		unsigned char g;
		unsigned char r;
		unsigned char a;
	}RGB;
} MY_BGRA, *pMY_BGRA;


typedef union _MY_RGBA{
	unsigned long ulRGB;
	unsigned int  unRGB;
	COLORREF rRGB;
	struct{
		unsigned char r;
		unsigned char g;
		unsigned char b;
		unsigned char a;
	}RGB;
} MY_RGBA, *pMY_RGBA;

void __fastcall myDrawTextAlpha(HDC inDC, const int X, const int Y, const char *pStr, int nFontHigh = 27,
	const char *pFontName = "Arial\0", const MY_RGBA cTXT = { 0x9f1ffc3d }, const MY_RGBA cBK = { 0x3f604f2f },
	const MY_RGBA cBKA = { 0xff0b001e }, const int nAlpha = 0xff, const int nMarginL = 1, const int nMarginR = 1);

void __fastcall drawTextAlpha(HDC inDC, const char *pStr, CRect rect, int nMarginT = 2,
	const char *pFontName = "Arial\0", const MY_RGBA cTXT = { 0x9f1ffc3d }, const MY_RGBA cBK = { 0x3f604f2f },
	const MY_RGBA cBKA = { 0xff0b001e }, const int nAlpha = 0xff, const int nMarginL = 1, const int nMarginR = 1);

//---------------------------------------------------------------------------
// Following is for class myTxt2BMP and myDrawTextAlpha function
//
// e.g. following call will display text on x:17, y:0 with default setting
//  
// myDrawTextAlpha(GetDC(ed1->Handle),17,0, "TEST: 200.12");
// 
//---------------------------------------------------------------------------

class myTxt2BMP
{
public:
	myTxt2BMP(const char *pStr, const char *pFontName = "Arial\0", const int nFontHigh = 27,
		MY_RGBA cTXT = { 0xffffff }, MY_RGBA cBK = { 0x001f1f1f }, MY_RGBA cBKA = { 0x000000 }, int nMarginL = 1, int nMarginR = 1);
	~myTxt2BMP();
	HBITMAP hBMP;
	BITMAPINFOHEADER iBMP;
	int nTextLength;
};



#endif 
