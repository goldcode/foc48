//*******************************************************************************************************/
//* FileName:		myPlot.cpp
//*
//* Contents:		Implementation of myPlot, axis, legend, serie and timeaxis
//*
//* NOTE 1:			Only a minimum of parameter validation is implemented to save time since this plot is 
//*					time critical.
//*
//* NOTE 2:			All functionality is not fully implemented.
//*
//* Author:			Jan Vidar Berger
//*******************************************************************************************************/
//* 12.feb.98	Jan Vidar Berger	Implemented flicker free drawing. Thanks to John Kim for providing 
//*									the MemDC and to Keith Rule, the author of CMemDC.
//*******************************************************************************************************/

//#include "stdafx.h"

#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions

#include "myPlot.h"
//#include "MemDC.h"
#include "myMemDC.h"
//#include "malloc.h"

#include "MainFrm.h"

static long	m_lMaxDataPrSerie=500;	// max allowed data pr. serie.
static long m_lMaxDataTotal=500;	// max allowed data total.

//*******************************************************************************************************/
//* time axis threshold. contains grid and label intervals to be used within specified
//* seconds pr. pixels thresholds. The array is terminated by a 'bIAmInUse'=FALSE.
//*******************************************************************************************************/
struct{
	BOOL	bIAmInUse;					// indicate valid entry, last=FALSE
	long	lgridinterval;				// grid line interval in seconds
	long	llabelinterval;				// time label interval in seconds
	long	lmodethreshold;				// mode threshold in seconds pr. pixel
}gridsetting[] = {
	TRUE, 1, 4, 0,						// 0: pr. second

	FALSE, 1, 1, 0,						// last entry in table
};

//*******************************************************************************************************/
//* Function:		serie::serie
//*******************************************************************************************************/
serie::serie()
{
	m_bIAmInUse = FALSE;
	m_color = RGB(0, 0, 0);
	m_iLineStyle = PS_SOLID;
	m_bRightAxisAlign = FALSE;
	m_lNoValues = 0;
	m_lbegin = 0;
	m_lend = 0;
	//m_pvalues = NULL;

	m_pvalues = new value[m_lMaxDataPrSerie];


}

//*******************************************************************************************************/
//* Function:		serie::~serie
//*******************************************************************************************************/
serie::~serie()
{
	if (m_pvalues != NULL)
		free(m_pvalues);
}

//*******************************************************************************************************/
//* Function:		serie::AddPoint
//*
//* Description:	AddPoint add new data to the end of a data serie. It will simply append the data,
//*					update the list index and get out. 
//*
//*					This function will also call realloc or malloc to re-size or create the plot array as
//*					needed.
//*
//*					The nice thing about circular lists is that they are multi thread enabled as is. You
//*					must however implement a syncronization mechanism if more than one thread is supposed
//*					to enter data into the plot.
//*
//* Parameters:		valuetime		Time (x value).
//*					y				y value
//*
//* Return Value:	-none-
//*
//* Author:			Jan Vidar Berger
//*******************************************************************************************************/
void serie::AddPoint(double &valuetime, double &y)
{
	m_pvalues[m_lend].ValueTime = valuetime;
	m_pvalues[m_lend].dValue = y;
	m_lNoValues++;
	m_lend++;
	//
	if (m_lend  >= m_lMaxDataPrSerie) m_lend = 0;	
	//
	if (m_lbegin == m_lend){
		m_lbegin++;
		if (m_lbegin >= m_lMaxDataPrSerie)
			m_lbegin = 0;
	}
}

//*******************************************************************************************************/
//* Function:		serie::Reset
//*
//* Description:	Reset the serie. Remove data and reset indexes and pointers.
//*
//* Parameters:		-none-
//*
//* Return Value:	-none-
//*
//* Author:			Jan Vidar Berger
//*******************************************************************************************************/
void serie::Reset()
{
	m_lNoValues = 0;

	if (m_pvalues != NULL)
		free(m_pvalues);

	m_pvalues = NULL;

	m_lbegin = 0;
	m_lend = 0;
}

//*******************************************************************************************************/
//*******************************************************************************************************/
ULONG myPlot::GetGestureStatus(CPoint /*ptTouch*/) { return 0; }

myPlot::myPlot(CMainFrame *p)
{
	pParent = p;
	m_ctlBkColor = RGB(255, 255, 255);  // board color  //AAA
	m_plotBkColor = RGB(255, 255, 255); // backround color  //AAA
	m_plotBdColor = RGB(0, 0, 0);     // board color  //AAA
	m_legendBkColor = RGB(255, 255, 255);

	m_gridColor = RGB(127, 127, 127);
	m_bctlBorder = TRUE;
	m_bplotBorder = TRUE;
	m_blegendBorder = TRUE;
	m_bPrimaryLegend = FALSE;
	m_bSecondaryLegend = FALSE;
	m_bAxisLY = TRUE;
	m_bAxisRY = TRUE;
	m_bAxisBX = TRUE;
	m_bAutoScrollX = TRUE;
	m_bSimMode = FALSE;

	m_dNoData = 0.0;

	m_dzoom = 1.0;

	lArraySize = 1000;			// only points with differebt x,y will be put into the array

	pLineArray = new CPoint[lArraySize];
	//SetBXRange(CTime::GetCurrentTime()-CTimeSpan(60),CTime::GetCurrentTime());
	double min_t = 0.0;
	double max_t = 10.0;
	SetBXRange( min_t, max_t );

	m_logFont.lfHeight = -13;
	m_logFont.lfWidth = 0;
	m_logFont.lfEscapement = 0;
	m_logFont.lfOrientation = 0;
	m_logFont.lfWeight = 400;
	m_logFont.lfItalic = FALSE;
	m_logFont.lfUnderline = FALSE;
	m_logFont.lfStrikeOut = FALSE;
	m_logFont.lfCharSet = ANSI_CHARSET;
	m_logFont.lfOutPrecision = OUT_DEFAULT_PRECIS;
	m_logFont.lfClipPrecision = CLIP_DEFAULT_PRECIS;
	m_logFont.lfQuality = PROOF_QUALITY;
	m_logFont.lfPitchAndFamily = DEFAULT_PITCH;
	//
	//wcscpy(m_logFont.lfFaceName, L"Ariel");

	m_zoomFont.lfHeight = -13;
	m_zoomFont.lfWidth = 0;
	m_zoomFont.lfEscapement = 0;
	m_zoomFont.lfOrientation = 0;
	m_zoomFont.lfWeight = 400;
	m_zoomFont.lfItalic = FALSE;
	m_zoomFont.lfUnderline = FALSE;
	m_zoomFont.lfStrikeOut = FALSE;
	m_zoomFont.lfCharSet = ANSI_CHARSET;
	m_zoomFont.lfOutPrecision = OUT_DEFAULT_PRECIS;
	m_zoomFont.lfClipPrecision = CLIP_DEFAULT_PRECIS;
	m_zoomFont.lfQuality = PROOF_QUALITY;
	m_zoomFont.lfPitchAndFamily = DEFAULT_PITCH;
	//wcscpy(m_zoomFont.lfFaceName, L"Ariel");

	m_font.CreateFontIndirect(&m_zoomFont);
	

}

//*******************************************************************************************************/
//*******************************************************************************************************/
myPlot::~myPlot()
{
	delete[] pLineArray;
}


//*******************************************************************************************************/
//*******************************************************************************************************/
BEGIN_MESSAGE_MAP(myPlot, CWnd)
	//{{AFX_MSG_MAP(myPlot)
	ON_WM_PAINT()
	ON_WM_LBUTTONDOWN()
	ON_WM_RBUTTONDOWN()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

//*******************************************************************************************************/
//*******************************************************************************************************/
BOOL myPlot::Create(DWORD dwstyle, CRect &rect, CWnd *pParent, UINT id)
{
	HWND Handle = NULL;
	DWORD style = dwstyle & (~WS_BORDER);
	if (dwstyle & WS_BORDER)
		m_bctlBorder = TRUE;
	else
		m_bctlBorder = FALSE;
	//
	if (!CWnd::Create(NULL, NULL, style, rect, pParent, id, NULL))
		return FALSE;
	//
	m_ctlRect = rect;
	pParent->ClientToScreen(m_ctlRect);
	ScreenToClient(m_ctlRect);
	//
	ComputeRects(TRUE);
	return TRUE;
}

//*******************************************************************************************************/
//* Function        : myPlot::ComputeRects
//*
//* Description	    : Compute rects used for internal possitioning of different objects. This function is 
//*					  called when the plot is created or sized.
//*
//* Return type	    : void 
//*
//* Parameter(s)    : bInitialization		indicate wherever parameters that can be changed abu the user
//*											also should be computed.
//*
//* Author          : Jan Vidar Berger
//*******************************************************************************************************/
void myPlot::ComputeRects(BOOL bInitialization)
{
	CClientDC dc(this);
	int w = 0;
	int n = 0;
	//
	CSize z = dc.GetTextExtent(CString("A"));
	//	
	m_dzoom = ((double)m_ctlRect.Height() / (double)z.cy) * m_zoomFactor;
	//
	m_zoomFont.lfWidth = (int)(m_logFont.lfWidth * m_dzoom);
	m_zoomFont.lfHeight = (int)(m_logFont.lfHeight * m_dzoom);
	//
	m_font.Detach();
	m_font.CreateFontIndirect(&m_zoomFont);
	//
	CFont *oFont = dc.SelectObject(&m_font);
	//	SetFont(&m_font);
	z = dc.GetTextExtent(CString("A"));	
	//
	m_TextHeight = z.cy;
	//
	if (m_bctlBorder){
		m_clientRect.left = m_ctlRect.left + 2;
		m_clientRect.right = m_ctlRect.right - 2;
		m_clientRect.top = m_ctlRect.top + 2;
		m_clientRect.bottom = m_ctlRect.bottom - 2;
	}
	else{
		m_clientRect = m_ctlRect;
	}
	//
	if (bInitialization)
	{
		m_iMtop = m_iMbottom = m_clientRect.Height() / 20;
		m_iMright = 1; m_iMleft = m_clientRect.Width() / 13;
	}
	// compute plot rect.
	m_plotRect.left   = m_clientRect.left   + m_iMleft;
	m_plotRect.right  = m_clientRect.right  - m_iMright;
	m_plotRect.top    = m_clientRect.top    + m_iMtop;
	m_plotRect.bottom = m_clientRect.bottom - m_iMbottom;
	// compute default legend possition
	//CStringA str;
	//
	if (bInitialization)
	{
		m_legendRect.top  = m_plotRect.top - (m_iMtop / 2); 
		m_legendRect.left = m_plotRect.left + (m_iMleft / 5);
		
		//m_legendRect.right  = m_plotRect.left + (m_plotRect.Width() / 5);		
		//m_legendRect.bottom = m_plotRect.top + (m_iMtop);

		w = 0;
		n = 0;
		for (int x = 0; x< MAXLEGENDS; x++){
			if (m_primarylegends[x].m_bIAmInUse){
				n++;
				z = dc.GetTextExtent(CString(m_primarylegends[x].m_szTitle));
				if (z.cx > w)
					w = z.cx;
				//				m_TextHeight = z.cy;
			}
		}

		m_legendRect.right = m_legendRect.left + 40 + w;
        m_legendRect.bottom = m_legendRect.top + 10 + (m_TextHeight*n);
		/*
		Now calculate the Rect to place Stats in 
		*/			 
		//
		w = 0;
		n = 0;
		//
		for (int x = 0; x< MAXSTATS; x++){
			if (m_stats[x].m_bIAmInUse){
				n++;
				z=dc.GetTextExtent(m_stats[x].getString());
				if (z.cx > w)
					w = z.cx;
				//				m_TextHeight = z.cy;
			}
		}

		m_statsRect.top    = m_plotRect.top + 10; 
		m_statsRect.left   = m_plotRect.right - w; 
		m_statsRect.right  = m_plotRect.right;
		m_statsRect.bottom = m_statsRect.top  + (m_TextHeight+1)*n;
	}

	// compute left axis area
	m_axisLYRect.left = m_clientRect.left + (m_iMleft / 5);
	m_axisLYRect.right = m_plotRect.left;
	m_axisLYRect.top = m_plotRect.top;
	m_axisLYRect.bottom = m_plotRect.bottom;

	// compute right axis area
	m_axisRYRect.left = m_plotRect.left;
	m_axisRYRect.right = m_clientRect.right - (m_iMright / 5);
	m_axisRYRect.top = m_plotRect.top;
	m_axisRYRect.bottom = m_plotRect.bottom;

	// compute bottom axis area
	m_axisBXRect.left = m_plotRect.left;
	m_axisBXRect.right = m_plotRect.right;
	m_axisBXRect.top = m_plotRect.bottom;
	m_axisBXRect.bottom = m_clientRect.bottom - (m_iMbottom / 5);

	//	if(bInitialization)
	//	{
	m_timeaxis.m_dSecondsPrPixel = (m_timeaxis.m_maxtime - m_timeaxis.m_mintime) / m_plotRect.Width();
	m_leftaxis.m_dValuePrPixel   = (m_leftaxis.maxrange - m_leftaxis.minrange)   / m_plotRect.Height();
	m_rightaxis.m_dValuePrPixel  = (m_rightaxis.maxrange - m_rightaxis.minrange) / m_plotRect.Height();
	//	}
	dc.SelectObject(oFont);
}

//*******************************************************************************************************/
//* Function:		myPlot::OnPaint
//*
//* Description:	This function will create a memory image, call Draw to draw the plot on it, and when
//*					copy the image into memory.
//*
//*					This is fast and provides flicker free plot update.
//*
//* Author:			Jan Vidar Berger
//*******************************************************************************************************/
void myPlot::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	CRect cReg;
	myCMemDC  pdc(&dc);  // non flickering painting
	Draw(&pdc);
	// Do not call CWnd::OnPaint() for painting messages
}

BOOL myPlot::OnEraseBkgnd(CDC* pDC)
{
	return TRUE;
}

//*******************************************************************************************************/
//*******************************************************************************************************/
void myPlot::Draw(CDC * dc)
{
	CFont *oFont = dc->SelectObject(&m_font);
	DrawBasic(dc);
	if (m_bDrawGrid)
		DrawGrid(dc);
	DrawPlot(dc);

	DrawLegend(dc);
	if (m_bStats)  DrawStats(dc);
	dc->SelectObject(oFont);
}

//*******************************************************************************************************/
//*******************************************************************************************************/
void myPlot::DrawBasic(CDC * dc)
{
	CBrush brushctlBkColor(m_ctlBkColor);
	dc->FillRect(m_ctlRect, &brushctlBkColor);
	if (m_bctlBorder)
	{
		dc->DrawEdge(m_ctlRect, BDR_SUNKENINNER | BDR_SUNKENOUTER, BF_RECT);
	}
	
	CPen pen(PS_SOLID, 1, m_plotBdColor); //AAA
	CPen *oPen = dc->SelectObject(&pen); //AAA
	CBrush brush(m_plotBkColor); //AAA
	CBrush *oBrush = dc->SelectObject(&brush); //AAA

	dc->Rectangle(m_plotRect);

	dc->SelectObject(oBrush); //AAA
	dc->SelectObject(oPen); //AAA

	DrawLegendShadow(dc);

}


//*******************************************************************************************************/
//*******************************************************************************************************/
void myPlot::DrawPlot(CDC * dc)
{
	for (int s = 0; s<MAXSERIES; s++)
	{
		if (m_series[s].m_bIAmInUse)
		{
			DrawSerie(dc, s);
		}
	}
}

//*******************************************************************************************************/
//*******************************************************************************************************/

void myPlot::DrawSerie(CDC *dc, int s)
{
	CPoint p;
	int ly=0;
	// lets get some serie parameters now and save the time of indexing during the loop
	long y = m_series[s].m_lbegin;
	long m = m_series[s].m_lend;
	long a = m_series[s].m_lNoValues;
	
	CPen pen(m_series[s].m_iLineStyle, 1, m_series[s].m_color);
	CPen *old = dc->SelectObject(&pen);
	
	int l_total = (a >= m_lMaxDataPrSerie) ? m_lMaxDataPrSerie : a;

	if (a > m_lMaxDataPrSerie){
		for (int y = m; y < m_lMaxDataPrSerie; y++){
			p.x = (int)(m_plotRect.left + ((m_series[s].m_pvalues[y].ValueTime - m_timeaxis.m_mintime) / m_timeaxis.m_dSecondsPrPixel));
			p.y = (int)(m_plotRect.bottom - ((m_series[s].m_pvalues[y].dValue - m_leftaxis.minrange) / m_leftaxis.m_dValuePrPixel));
			if ((ly == 0 || p.x != pLineArray[ly].x || p.y != pLineArray[ly].y)
				&& (p.x >= m_plotRect.left && p.x <= m_plotRect.right))
			{
				pLineArray[ly].x = p.x;
				pLineArray[ly].y = p.y;
				ly++;
			}
		}
	}

	for (int y = 0; y < m; y++){
		p.x = (int)(m_plotRect.left + ((m_series[s].m_pvalues[y].ValueTime - m_timeaxis.m_mintime) / m_timeaxis.m_dSecondsPrPixel));
		p.y = (int)(m_plotRect.bottom - ((m_series[s].m_pvalues[y].dValue - m_leftaxis.minrange) / m_leftaxis.m_dValuePrPixel));
		if ((ly == 0 || p.x != pLineArray[ly].x || p.y != pLineArray[ly].y)
			&& (p.x >= m_plotRect.left && p.x <= m_plotRect.right))
		{
			pLineArray[ly].x = p.x;
			pLineArray[ly].y = p.y;
			ly++;
		}
	}

	if (ly > 0){
		dc->Polyline(pLineArray, ly);
	}
	dc->SelectObject(old);


}


//*******************************************************************************************************/
//*
//*******************************************************************************************************/
void myPlot::DrawGrid(CDC * dc)
{
	DrawXAxisGrid(dc);
	DrawYAxisGrid(dc);
}

//*******************************************************************************************************/
//*******************************************************************************************************/
void myPlot::DrawYAxisGrid(CDC * dc)
{
	double yGrid = m_leftaxis.minrange;
	double delta = 25.0 + (long)(((m_leftaxis.m_dValuePrPixel))) * 25;
	if ((long)delta % 50 != 0 && delta > 20.0)
		delta += 25;
	double d10 = delta / 5.0;
	// todo: delta switch
	long diff = ((long)yGrid) % ((long)delta);
	yGrid = yGrid - diff;
	CPen *old, pen(PS_SOLID, 1, m_gridColor);
	CPen stick(PS_SOLID, 0, RGB(0, 0, 0));
	CPen mline(PS_SOLID, 0, RGB(192, 192, 192));

	for (long sy = (long)((long)(m_leftaxis.minrange) - diff); sy < m_leftaxis.maxrange; sy += (long)d10)
	{
		int off = 3;
		if ((long)sy % (long)delta == 0){
			off = 5;
		}
		if (sy > m_leftaxis.minrange){
			int y = (int)(m_plotRect.bottom - ((sy - m_leftaxis.minrange) / m_leftaxis.m_dValuePrPixel));
			old = dc->SelectObject(&stick);
			dc->MoveTo(CPoint(m_plotRect.left, y));
			dc->LineTo(CPoint(m_plotRect.left - off, y));
			dc->MoveTo(CPoint(m_plotRect.right, y));
			dc->LineTo(CPoint(m_plotRect.right + off, y));
			dc->SelectObject(old);

			old = dc->SelectObject(&mline);
			dc->MoveTo(CPoint(m_plotRect.left + 1, y));
			dc->LineTo(CPoint(m_plotRect.right - 1, y));
			dc->SelectObject(old);
		}
	}

	old = dc->SelectObject(&pen);
	while (yGrid <= m_leftaxis.maxrange)
	{
		double yy = m_plotRect.bottom - ((yGrid - m_leftaxis.minrange) / m_leftaxis.m_dValuePrPixel);
		int y = (int)yy;
		if (yGrid > m_leftaxis.minrange && yGrid<m_leftaxis.maxrange){
			dc->MoveTo(CPoint(m_plotRect.left + 1, y));
			dc->LineTo(CPoint(m_plotRect.right - 1, y));
		}

		wchar_t b[100];
		int n = swprintf_s(b, 99, L"%.0f", yGrid);
		dc->DrawText(b, n, CRect(m_clientRect.left, y - m_TextHeight / 2, m_plotRect.left - 5, y + m_TextHeight / 2), DT_RIGHT | DT_BOTTOM);

		double yr = (m_plotRect.bottom - yy) * m_rightaxis.m_dValuePrPixel + m_rightaxis.minrange;
		n = swprintf_s(b, 99, L"%.0f", yr);
		dc->DrawText(b, n, CRect(m_plotRect.right + 5, y - m_TextHeight / 2, m_clientRect.right, y + m_TextHeight / 2), DT_LEFT | DT_BOTTOM);

		yGrid += delta;
	}
	dc->SelectObject(old);
}

//*******************************************************************************************************/
int __fastcall myD2I(const double d);
//*******************************************************************************************************/
void myPlot::DrawXAxisGrid(CDC * dc)				// this function will not be used in this app
{
	long mmm = myD2I(m_timeaxis.m_mintime);
	long mmx = myD2I(m_timeaxis.m_maxtime);
	long yGrid = mmm;
	long delta = (long)(10.0 + (long)(((m_timeaxis.m_dSecondsPrPixel))) * 10);
	long d10 = (long)(delta / 10.0);
	// todo: delta switch
	long diff = ((long)yGrid) % ((long)delta);
	yGrid = yGrid - diff;
	CPen *old, pen(PS_SOLID, 1, m_gridColor);
	CPen stick(PS_SOLID, 0, RGB(0, 0, 0));
	CPen mline(PS_SOLID, 0, RGB(192, 192, 192));

	//for (long sx = m_timeaxis.m_mintime - diff; sx < m_timeaxis.m_maxtime; sx += d10)
	for (long sx = yGrid; sx < mmx; sx += d10)
	{
		int off = 3;
		if ((long)sx % (long)delta == 0){
			off = 5;
		}
		// (sx > m_timeaxis.m_mintime)
		if (sx > mmm){
			int x = (int)(m_plotRect.left + ((sx - mmm) / m_timeaxis.m_dSecondsPrPixel));
			//      (int)(m_plotRect.left + ((sx - m_timeaxis.m_mintime) / m_timeaxis.m_dSecondsPrPixel));
			//			int x = m_plotRect.right - ((sx-m_timeaxis.m_mintime.GetTime())/m_timeaxis.m_lSecondsPrPixel);
			old = dc->SelectObject(&stick);
			dc->MoveTo(CPoint(x, m_plotRect.bottom));
			dc->LineTo(CPoint(x + off, m_plotRect.bottom));
			dc->SelectObject(old);

			old = dc->SelectObject(&mline);
			dc->MoveTo(CPoint(x, m_plotRect.bottom - 1));
			dc->LineTo(CPoint(x, m_plotRect.top + 1));
			dc->SelectObject(old);
		}
	}

	old = dc->SelectObject(&pen);
	// (yGrid <= m_timeaxis.m_maxtime)
	while (yGrid <= mmx)
	{
		int x = (int)(m_plotRect.left + ((yGrid - mmm) / m_timeaxis.m_dSecondsPrPixel));
		// (int)(m_plotRect.left + ((yGrid - m_timeaxis.m_mintime) / m_timeaxis.m_dSecondsPrPixel));
		if (yGrid > m_timeaxis.m_mintime && yGrid<m_timeaxis.m_maxtime){
			dc->MoveTo(CPoint(x, m_plotRect.bottom - 1));
			dc->LineTo(CPoint(x, m_plotRect.top + 1));
		}

		//		char b[100];
		//		sprintf(b, "%.0f", yGrid);
		//		dc->DrawText(b, CRect(m_clientRect.left, y-m_TextHeight/2,m_plotRect.left-5,y+m_TextHeight/2), DT_RIGHT|DT_BOTTOM);

		yGrid += delta;
	}
	dc->SelectObject(old);
}

//*******************************************************************************************************/
//*******************************************************************************************************/
void myPlot::DrawLegendShadow(CDC * dc)
{
	if (m_blegendBorder){
		CPen pen(PS_SOLID, 1, RGB(127, 127, 127));
		CPen *oPen = dc->SelectObject(&pen);
		CBrush *oBrush, brush(RGB(127, 127, 127));
		oBrush = dc->SelectObject(&brush);
		dc->Rectangle(CRect(m_legendRect.left + 5, m_legendRect.top + 5, m_legendRect.right + 5, m_legendRect.bottom + 5));
		dc->SelectObject(oBrush);
		dc->SelectObject(oPen);
	}
}

//*******************************************************************************************************/
//*******************************************************************************************************/
void myPlot::DrawLegend(CDC * dc)
{
	if (m_blegendBorder){
		CPen pen(PS_SOLID, 1, RGB(0, 0, 0));
		CPen *oPen = dc->SelectObject(&pen);
		CBrush *oBrush, brush(m_legendBkColor);
		oBrush = dc->SelectObject(&brush);
		dc->Rectangle(m_legendRect);
		dc->SelectObject(oBrush);
		dc->SelectObject(oPen);
	}
	int y = m_legendRect.top + 5;
	int dx = m_legendRect.left + (2 * m_TextHeight);

	int mh = m_TextHeight / 2;
	for (int x = 0; x< MAXLEGENDS; x++){
		if (m_primarylegends[x].m_bIAmInUse){
			CRect lRect(dx + 5, y, m_legendRect.right - 5, y + m_TextHeight);
			CPen pen(m_primarylegends[x].m_istyle, 1, m_primarylegends[x].m_color);
			CPen *oPen = dc->SelectObject(&pen);
			dc->MoveTo(CPoint(m_legendRect.left + 5, y + mh));
			dc->LineTo(CPoint(dx, y + mh));
			dc->SelectObject(oPen);

			dc->DrawText(m_primarylegends[x].m_szTitle, lRect, DT_LEFT);
			y += m_TextHeight + 1;
		}
	}
}

//*******************************************************************************************************/
//*******************************************************************************************************/
void myPlot::DrawStats(CDC * dc)
{
	COLORREF oldBKColor = dc->SetBkColor(m_legendBkColor); //AAA, keep old
	int oldBKMode = dc->SetBkMode(OPAQUE); //TRANSPARENT

	int y  = m_statsRect.top ;
	int dx = m_statsRect.left;
	int mh = m_TextHeight ;
	
	for (int x = 0; x< MAXSTATS; x++){
		if (m_stats[x].m_bIAmInUse){
			CRect lRect(dx, y, m_statsRect.right, y + m_TextHeight);			
			//str.Format(m_stats[x].m_format, m_stats[x].dValue);
			
			//myDrawTextAlpha(dc->m_hDC, lRect.left, lRect.top, (CStringA) m_stats[x].getString(), lRect.Height-4);
			//dc->DrawText( m_stats[x].getString() , lRect, DT_LEFT);
			drawTextAlpha(dc->m_hDC, (CStringA) m_stats[x].getString(), lRect, 2 ); 
			y += m_TextHeight + 1;
		}
	}
	dc->SetBkMode(oldBKMode); //AAA
	dc->SetBkColor(oldBKColor); //AAA
}


//*******************************************************************************************************/
//* Function:		myPlot::AddPoint
//*******************************************************************************************************/


BOOL myPlot::AddPoint(int serie, double &valuetime, double &value)
{
	m_series[serie].AddPoint(valuetime, value);

	if (m_bAutoScrollX && valuetime > m_timeaxis.m_maxtime){
		double span    = m_timeaxis.m_maxtime - m_timeaxis.m_mintime;
		double mintime = valuetime - span;

		SetBXRange(mintime, valuetime);
	}
    return TRUE;	
}

//*******************************************************************************************************/
//*******************************************************************************************************/
void myPlot::SetBXRange(double &fromtime, double &totime, BOOL bMove)
{
	m_timeaxis.m_mintime = fromtime;
	m_timeaxis.m_maxtime = totime;
	if (!bMove){
		m_timeaxis.m_dSecondsPrPixel = ((double)(m_timeaxis.m_maxtime - m_timeaxis.m_mintime + 1)) / (double)m_plotRect.Width();
	}
}

//*******************************************************************************************************/
//*******************************************************************************************************/
void myPlot::SetLYRange(double &minrange, double &maxrange)
{
	m_leftaxis.minrange = minrange;
	m_leftaxis.maxrange = maxrange;
	m_leftaxis.m_dValuePrPixel = ((double)( maxrange - minrange) / (double)m_plotRect.Height());
}

//*******************************************************************************************************/
//*******************************************************************************************************/
void myPlot::SetRYRange(double &minrange, double &maxrange)
{
	m_rightaxis.minrange = minrange;
	m_rightaxis.maxrange = maxrange;
	m_rightaxis.m_dValuePrPixel = ((double)(maxrange - minrange) / (double)m_plotRect.Height());
}

//*******************************************************************************************************/
//*******************************************************************************************************/
void myPlot::SetBXTitle(const char *title)
{
}

//*******************************************************************************************************/
//*******************************************************************************************************/
void myPlot::SetLYTitle(const char *title)
{
}

//*******************************************************************************************************/
//*******************************************************************************************************/
void myPlot::SetRYTitle(const char *title)
{
}

//*******************************************************************************************************/
//*******************************************************************************************************/
void myPlot::Reset()
{
}

//*******************************************************************************************************/
//*******************************************************************************************************/
void myPlot::SetSerie(int s, int style, COLORREF color, double minrange, double maxrange, const char *szTitle, BOOL Rightalign)
{
	m_series[s].m_bIAmInUse = TRUE;
	m_series[s].m_color = color;
	m_series[s].m_iLineStyle = style;
	m_series[s].m_bRightAxisAlign = Rightalign;
}

//*******************************************************************************************************/
//*******************************************************************************************************/
void myPlot::MoveWindow(CRect & Rect)
{
	m_ctlRect = Rect;
	GetParent()->ClientToScreen(m_ctlRect);
	ScreenToClient(m_ctlRect);
	ComputeRects(TRUE);
	CWnd::MoveWindow(Rect);
}

//*******************************************************************************************************/
//*******************************************************************************************************/
void myPlot::SetLegend(int l, int style, COLORREF color, const char *text)
{
	m_primarylegends[l].m_bIAmInUse = TRUE;
	m_primarylegends[l].m_color = color;
	m_primarylegends[l].m_istyle = style;
	m_primarylegends[l].m_szTitle = text;
	CClientDC dc(this);
	CFont *oFont = dc.SelectObject(&m_font);
	// change the parameters	
	int w = 0;
	int n = 0;
	for (int x = 0; x< MAXLEGENDS; x++){
		if (m_primarylegends[x].m_bIAmInUse){
			n++;
			CSize z = dc.GetTextExtent(CString(m_primarylegends[x].m_szTitle));
			if (z.cx > w)
				w = z.cx;
			//			m_TextHeight = z.cy;
		}
	}
	m_legendRect.right = m_legendRect.left + 10 + (2 * m_TextHeight) + w;
	m_legendRect.bottom = m_legendRect.top + 10 + (m_TextHeight*n);
	dc.SelectObject(oFont);
}

//*******************************************************************************************************/
//*******************************************************************************************************/
void myPlot::SetStats(int l, CStringA text, CStringA fmt, double dVal)
{
	m_stats[l].m_bIAmInUse = TRUE;
	m_stats[l].m_format    = fmt;
	m_stats[l].m_szTitle   =  text;
	m_stats[l].dValue      = dVal;
	CClientDC dc(this);
	CFont *oFont = dc.SelectObject(&m_font);
	int w = 0;
	int n = 0;
	for (int x = 0; x< MAXSTATS; x++){
		if (m_stats[x].m_bIAmInUse){
			n++;
			CSize z = dc.GetTextExtent(m_stats[x].getString());
			if (z.cx > w)
				w = z.cx;
			//			m_TextHeight = z.cy;
		}
	}
	//
	//m_statsRect.right  = m_statsRect.left + 10 + (2 * m_TextHeight) + w;
	m_statsRect.left = m_statsRect.right -  w;
	m_statsRect.bottom = m_statsRect.top + (m_TextHeight+1)*n;

	dc.SelectObject(oFont);
}


void myPlot::OnLButtonDown(UINT nFlags, CPoint point){
	if (pParent!=NULL)
		pParent->OnLButtonDown(-2, point);
}

void myPlot::OnRButtonDown(UINT nFlags, CPoint point){
	if (pParent != NULL)
		pParent->OnRButtonDown(-2, point);
		//pParent->OnRButtonDown(nFlags, point);
}

void myPlot::OnLButtonDblClk(UINT nFlags, CPoint point){
	if (pParent != NULL)
		pParent->OnLButtonDblClk(nFlags, point);
}


myTxt2BMP::myTxt2BMP(const char *pStr, const char *pFontName, const int nFontHigh,
	MY_RGBA cTXT, MY_RGBA cBK, MY_RGBA cBKA, int nMarginL, int nMarginR)
	//myTxt2BMP(const char *inText, HFONT inFont, COLORREF inColour, COLORREF cBK=0x00000000, COLORREF cText = 0x00ffffff)
{
	hBMP = NULL;
	nTextLength = (int)strlen(pStr);   // 4GB limit
	if (nTextLength <= 0) return;

	HFONT hFont = CreateFontA(nFontHigh, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, pFontName); //"Arial\0"

	HDC hDC = CreateCompatibleDC(NULL); // Create Compatible DC & select font
	HFONT hFontOld = (HFONT)SelectObject(hDC, hFont);

	//HBITMAP hMyDIB = NULL;

	// Get text area
	RECT reText = { 0 };
	DrawTextA(hDC, pStr, nTextLength, &reText, DT_CALCRECT);
	if ((reText.right > reText.left) && (reText.bottom > reText.top))
	{
		memset(&iBMP, 0x0, sizeof(iBMP)); //BITMAPINFOHEADER
		// init DIB BMP
		iBMP.biSize = sizeof(iBMP);
		iBMP.biWidth = reText.right - reText.left + 1 + nMarginL + nMarginR;
		iBMP.biHeight = reText.bottom - reText.top + 1;
		iBMP.biPlanes = 1;
		iBMP.biBitCount = 32;
		iBMP.biCompression = BI_RGB;

		void *pBitBuf = NULL;
		// Create and select DIB into DC
		hBMP = CreateDIBSection(hDC, (LPBITMAPINFO)&iBMP, 0, (LPVOID*)&pBitBuf, NULL, 0);
		HBITMAP hBMPOld = (HBITMAP)SelectObject(hDC, hBMP);
		if (hBMPOld != NULL)
		{
			// Set up DC properties
			SetTextColor(hDC, (cTXT.rRGB & 0xffffff));
			SetBkColor(hDC, (cBK.rRGB & 0xffffff));
			SetBkMode(hDC, OPAQUE);

			// Draw text to buffer with marginL shift
			reText.left += nMarginL; reText.right += nMarginL;
			DrawTextA(hDC, pStr, nTextLength, &reText, DT_NOCLIP); //DT_CENTER

			MY_BGRA *pData = (MY_BGRA *)pBitBuf;
			int n = iBMP.biHeight * iBMP.biWidth;
			for (int i = 0; i < n; i++) {
				MY_BGRA &rr = pData[i];
				if (rr.RGB.r == cTXT.RGB.r && rr.RGB.g == cTXT.RGB.g && rr.RGB.b == cTXT.RGB.b){
					rr.RGB.a = cTXT.RGB.a;

				}
				else if (rr.RGB.r == cBK.RGB.r && rr.RGB.g == cBK.RGB.g && rr.RGB.b == cBK.RGB.b){
					rr.RGB.a = cBK.RGB.a;
				}
				else{
					rr.RGB.a = cBKA.RGB.a;
				}
			}

			// return old BMP object
			SelectObject(hDC, hBMPOld);
		}
	}

	// return old font and clear hDC
	SelectObject(hDC, hFontOld);
	DeleteDC(hDC);
	DeleteObject(hFont);
}

myTxt2BMP::~myTxt2BMP()
{
	if (hBMP != NULL)
		DeleteObject(hBMP);
}


//void __fastcall myDrawTextAlpha(HDC inDC, const int X, const int Y, const char *pStr, int nFontHigh=27,
//	const char *pFontName = "Arial\0", const MY_RGBA cTXT={0x9f1ffc3d}, const MY_RGBA cBK={0x3f604f2f},
//    const MY_RGBA cBKA={0xff0b001e}, const int nAlpha=0xff, const int nMarginL=1, const int nMarginR=1);

void __fastcall myDrawTextAlpha(HDC inDC, const int X, const int Y, const char *pStr, int nFontHigh, const char *pFontName,
	const MY_RGBA cTXT, const MY_RGBA cBK, const MY_RGBA cBKA, const int nAlpha, const int nMarginL, const int nMarginR)
{

	myTxt2BMP tt(pStr, pFontName, nFontHigh, cTXT, cBK, cBKA);
	if (tt.hBMP == NULL) return;

	// Create DC, select bmptxt onto it
	HDC hDC = CreateCompatibleDC(inDC);
	HBITMAP hBMPOld = (HBITMAP)SelectObject(hDC, tt.hBMP);
	if (hBMPOld != NULL)
	{

		BITMAP iBMP;
		GetObject(tt.hBMP, sizeof(iBMP), &iBMP);
		// iBMP.bmHeight;  tt.iBMP.biHeight;

		BLENDFUNCTION ssAlpha;
		ssAlpha.BlendOp = AC_SRC_OVER;
		ssAlpha.BlendFlags = 0;
		ssAlpha.SourceConstantAlpha = nAlpha;
		ssAlpha.AlphaFormat = AC_SRC_ALPHA;
		//AlphaBlend(inDC, X, Y, iBMP.bmWidth, iBMP.bmHeight, hDC, 0, 0, iBMP.bmWidth, iBMP.bmHeight, ssAlpha);
		AlphaBlend(inDC, X, Y, tt.iBMP.biWidth, tt.iBMP.biHeight, hDC, 0, 0,
			tt.iBMP.biWidth, tt.iBMP.biHeight, ssAlpha);

		// Clean up
		SelectObject(hDC, hBMPOld);

	}
	DeleteDC(hDC);
}


void __fastcall drawTextAlpha(HDC inDC, const char *pStr, CRect Rect, int nMarginT, const char *pFontName,
	const MY_RGBA cTXT, const MY_RGBA cBK, const MY_RGBA cBKA, const int nAlpha, const int nMarginL, const int nMarginR)
{

	int nFontHigh = Rect.Height() - nMarginT;
		
	myTxt2BMP tt(pStr, pFontName, nFontHigh, cTXT, cBK, cBKA);
	if (tt.hBMP == NULL) return;

	// Create DC, select bmptxt onto it
	HDC hDC = CreateCompatibleDC(inDC);
	HBITMAP hBMPOld = (HBITMAP)SelectObject(hDC, tt.hBMP);
	if (hBMPOld != NULL)
	{

		BITMAP iBMP;
		GetObject(tt.hBMP, sizeof(iBMP), &iBMP);
		// iBMP.bmHeight;  tt.iBMP.biHeight;



		BLENDFUNCTION ssAlpha;
		ssAlpha.BlendOp = AC_SRC_OVER;
		ssAlpha.BlendFlags = 0;
		ssAlpha.SourceConstantAlpha = nAlpha;
		ssAlpha.AlphaFormat = AC_SRC_ALPHA;
		//AlphaBlend(inDC, X, Y, iBMP.bmWidth, iBMP.bmHeight, hDC, 0, 0, iBMP.bmWidth, iBMP.bmHeight, ssAlpha);
		AlphaBlend(inDC, Rect.left, Rect.top, tt.iBMP.biWidth, tt.iBMP.biHeight, hDC, 0, 0,
			tt.iBMP.biWidth, tt.iBMP.biHeight, ssAlpha);

		// Clean up
		SelectObject(hDC, hBMPOld);

	}
	DeleteDC(hDC);
}


//---------------------------------------------------------------------------
// End of class myTxt2BMP
//---------------------------------------------------------------------------


