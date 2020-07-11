//---------------------------------------------------------------------------
// Copyright (c) 2002-2009 12noon, Stefan K. S. Tucker.
// Copyright (c) 1997-2002 Perpetual Motion Software, Stefan K. S. Tucker.
//---------------------------------------------------------------------------

#include "stdafx.h"
#include <afxsettingsstore.h>
#include <afxregpath.h>

#include "pmslib.h"
#include "pmsres.h"
#include "StatLink.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CStaticHyperlink

CStaticHyperlink::CStaticHyperlink(const int idCursor)
{
   _idError1 = 0;

// only defined for >=Win2000
#if !defined(IDC_HAND)
#define IDC_HAND            MAKEINTRESOURCE(32649)
#endif
   /*
      if the cursor id is 0, we use a system cursor
   */
   HINSTANCE hRes = nullptr;
   LPTSTR pCursor = MAKEINTRESOURCE(idCursor);
   if (idCursor == 0)
      pCursor = IDC_HAND;
   else
      hRes = ::AfxGetResourceHandle();

   _hCursor = (HCURSOR) ::LoadImage(hRes, pCursor, IMAGE_CURSOR, 0, 0, LR_DEFAULTSIZE | LR_SHARED);
}


BEGIN_MESSAGE_MAP(CStaticHyperlink, CStatic)
	//{{AFX_MSG_MAP(CStaticHyperlink)
	ON_WM_CTLCOLOR_REFLECT()
   ON_CONTROL_REFLECT(STN_CLICKED, OnClicked)
	ON_WM_SETCURSOR()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CStaticHyperlink message handlers

HBRUSH CStaticHyperlink::CtlColor(CDC* pDC, UINT nCtlColor) 
{
   UNUSED(nCtlColor);
   ASSERT(nCtlColor == CTLCOLOR_STATIC);

   //------------------------------------------------------------------
   // make sure the Notify style is set -- otherwise, we won't get any
   // mouse clicks (the default for static controls is HitTransparent)
   //------------------------------------------------------------------
   DWORD dwStyle = GetStyle();
   if (!(dwStyle & SS_NOTIFY))
   {
      ::SetWindowLong(GetSafeHwnd(), GWL_STYLE, dwStyle | SS_NOTIFY);
   }

   //-----------------------------------------------------------------
   // if there's no window text, don't bother changing font and color
   //-----------------------------------------------------------------
   CString cText;
   GetWindowText(cText);
   if (cText.IsEmpty())
      return nullptr;

   //--------------------------------------
	// Change any attributes of the DC here
   //--------------------------------------

   //--------------------------
   // change font to underline
   //--------------------------
   CFont font;
   LOGFONT lf;
   GetFont()->GetObject(sizeof(lf), &lf);
   lf.lfUnderline = TRUE;
   font.CreateFontIndirect(&lf);
   pDC->SelectObject(&font);

   //------------------------------------------------------------
   // try to get link color settings from IE
   //
   // HKCU \ Software \ Microsoft \ Internet Explorer \ Settings
   //    AnchorColor = red,green,blue
   //    AnchorColorVisited = red,green,blue
   //
   // (we don't know the paths for other browsers)
   //------------------------------------------------------------
	COLORREF crAnchor = RGB(0, 0, 0xff);   // default
	CSettingsStore store(FALSE /*admin*/, TRUE /*read-only*/);
	if (store.Open(_T("Software\\Microsoft\\Internet Explorer\\Settings")))
	{
		CString s;
		if (store.Read(_T("Anchor Color"), s) && !s.IsEmpty())
		{
			UINT r, g, b;
			if (_stscanf_s((LPCTSTR) s, _T("%u,%u,%u"), &r, &g, &b) == 3)
				crAnchor = RGB(r, g, b);
		}
	}
   pDC->SetTextColor(crAnchor);
   pDC->SetBkMode(TRANSPARENT);

   //----------------------------------------------------------------------
	// return a non-NULL brush if the parent's handler should not be called
   //----------------------------------------------------------------------
   // return transparent brush to preserve parent bg color
	return (HBRUSH) ::GetStockObject(HOLLOW_BRUSH);
}

void CStaticHyperlink::OnClicked()
{
   // if there is no stored URL, use the control's text
   CString strURL;
   if (_strURL.IsEmpty())
      GetWindowText(strURL);
   else
      strURL = _strURL;

   if (!PMS::OpenURL(strURL))
   {
      if (_idError1 > 0)
      {
         CString s;
         ::AfxFormatString1(s, _idError1, strURL);
         ::AfxMessageBox(s, MB_OK | MB_ICONEXCLAMATION, _idError1);
      }
      else
         ::MessageBeep(UINT(-1));
   }
}


BOOL CStaticHyperlink::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message) 
{
   if (_hCursor == nullptr)
	   return CStatic::OnSetCursor(pWnd, nHitTest, message);

   ::SetCursor(_hCursor);
   return TRUE;
}

