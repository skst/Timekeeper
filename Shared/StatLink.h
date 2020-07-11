//---------------------------------------------------------------------------
// Copyright (c) 1997-2001, All Rights Reserved
// Perpetual Motion Software, Stefan K. S. Tucker.
//---------------------------------------------------------------------------

//-------------------------------------------------------------------------
// This is modified from Microsoft Systems Journal, December 1997 v12 n12,
// Q&A C++, by Paul DiLascia, pp. 104-108.
//-------------------------------------------------------------------------
// The article also added features such as a member string to set as the
// link -- if the static control's text itself wasn't the URL.
// It allows icons to be links.
// It saved a bool as a 'visited' flag, and changed the text color
// accordingly.
//-----------------------------------------------------------------------

#pragma once

/////////////////////////////////////////////////////////////////////////////
// CStaticHyperlink window

class CStaticHyperlink : public CStatic
{
protected:
   HCURSOR _hCursor;
   CString _strURL;     // optional: if empty, uses text
   int _idError1;

public:
   CStaticHyperlink(const int idCursor = 0);
   virtual ~CStaticHyperlink() {}

   void SetURL(const int id) { _strURL.LoadString(id); }
   void SetErrorID(const int id) { _idError1 = id; }

public:
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CStaticHyperlink)
	//}}AFX_VIRTUAL

protected:
	// Generated message map functions
	//{{AFX_MSG(CStaticHyperlink)
	afx_msg HBRUSH CtlColor(CDC* pDC, UINT nCtlColor);
   afx_msg void OnClicked();
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

