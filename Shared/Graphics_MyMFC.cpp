//---------------------------------------------------------------------------
// (c) 2006 12noon, Stefan K. S. Tucker
//---------------------------------------------------------------------------

#include "stdafx.h"
#include <windowsx.h>

#include "graphics.h"

/*
   Calculate the size (in pixels) of a passed string for a passed font.
*/
CSize MyMFC::CalculateTextSize(LPCTSTR szText, CFont& font)
{
   return CalculateTextSize(szText, (HFONT) font);
}

CSize MyMFC::CalculateTextSize(LPCTSTR szText, HFONT hFont)
{
   // create a DC for the font
   CDC dc;
   dc.CreateCompatibleDC(nullptr);
   // use the passed font
   HFONT hFontOld = SelectFont(dc.GetSafeHdc(), hFont);

   // calc the size
//   const CSize sz(dc.GetOutputTextExtent(szText));
   CRect r;
   dc.DrawText(szText, &r, DT_CALCRECT);
   const CSize sz(r.Width(), r.Height());

   SelectFont(dc.GetSafeHdc(), hFontOld);

//   TRACE(_T("\tText extent of \"%s\": %dx%d\n"), szText, sz.cx, sz.cy);
   return sz;
}
