//*******************************************************************************************************/
//* FileName:		MemDC.h
//*
//* Contents:		Definition and implementation for myCMemDC
//*
//* Copyright		You may freely use or modify this code provided this Copyright is included in all 
//* Notice:			derived versions.
//*
//* Author:			Keith Rule
//*
//* Email:			keithr@europa.com
//*
//* Copyright 1996-1997, Keith Rule
//*******************************************************************************************************/
//* 10/3/97		Keith Rule			Fixed scrolling bug.
//* 10/3/97		Keith Rule			Added print support.
//* 12.feb.98	Jan Vidar Berger	Ported myCMemDC into myPlot and added some comments.
//*******************************************************************************************************/
#ifndef _MEMDC_H_
#define _MEMDC_H_


#include "afxwin.h"

//*******************************************************************************************************/
//* Class:			myCMemDC - memory DC
//*
//* Base Class:		public CDC
//*
//* Description:	This class implements a memory Device Context that enables flicker free drawing.
//*
//* Usage:			Implemen myCMemDC in your CMyWnd::OnPaint as following:
//*							{
//*								CPaintDC	dc(pWnd);
//*								myCMemDC		mDC(dc);	// call myCMemDC::myCMemDC(CDC *dc)
//*
//*								Draw(mDC);				// draw on myCMemDC rather than CDC directly.
//*							}							// call myCMemDC::~myCMemDC()
//*
//*					Finally, add and modify WM_ERASEBKGND message in your project as following.
//*
//*					BOOL CMyView::OnEraseBkgnd(CDC* pDC) 
//*					{
//*						return FALSE;
//*					}
//*
//* How it works:	1.	The constructor will construct compatibledc and do the work required to enable 
//*						drawing on a memory dc.
//*					2.	You can draw on the myCMemDC as if it is a CDC.
//*					3.	The destructor copies the memory dc into the real DC.
//*******************************************************************************************************/


class myCMemDC : public CDC {

private:
	CBitmap m_bitmap;								// Offscreen bitmap
	CBitmap* m_oldBitmap;							// bitmap originally found in myCMemDC
	CDC* m_pDC;										// Saves CDC passed in constructor
	CRect m_rect;									// Rectangle of drawing area.
	BOOL m_bMemDC;									// TRUE if CDC really is a Memory DC.
public:
	myCMemDC(CDC* pDC) : CDC(), m_oldBitmap(NULL), m_pDC(pDC)
	{
		//ASSERT(m_pDC != NULL);					// If you asserted here, you passed in a NULL CDC, this is for debug, now we used in real case.
		if (m_pDC == NULL) {
			m_bMemDC = false; return;
		}

		m_bMemDC = !pDC->IsPrinting();

		if (m_bMemDC) {
			// Create a Memory DC
			CreateCompatibleDC(pDC);
			pDC->GetClipBox(&m_rect);
			m_bitmap.CreateCompatibleBitmap(pDC, m_rect.Width(), m_rect.Height());
			m_oldBitmap = SelectObject(&m_bitmap);
			SetWindowOrg(m_rect.left, m_rect.top);
		}
		else {
			// Make a copy of the relevent parts of the current DC for printing
			m_bPrinting = pDC->m_bPrinting;
			m_hDC = pDC->m_hDC;
			m_hAttribDC = pDC->m_hAttribDC;
		}
	}

	~myCMemDC()
	{
		if (m_bMemDC) {

			// Copy the offscreen bitmap onto the screen.
			m_pDC->BitBlt(m_rect.left, m_rect.top, m_rect.Width(), m_rect.Height(),
				this, m_rect.left, m_rect.top, SRCCOPY);
			//Swap back the original bitmap.			
			SelectObject(m_oldBitmap);
		}
		else {
			// All we need to do is replace the DC with an illegal value,
			// this keeps us from accidently deleting the handles associated with
			// the CDC that was passed to the constructor.
			m_hDC = m_hAttribDC = NULL;
		}
	}

	// Allow usage as a pointer
	myCMemDC* operator->() { return this; }

	// Allow usage as a pointer
	operator myCMemDC* () { return this; }
};

#endif
