//---------------------------------------------------------------------------
// (c) 2006 Stefan K. S. Tucker, 12noon
//---------------------------------------------------------------------------

#pragma once

#include "stdafx.h"

#include "MyWin/Graphics.h"

/*
   Without the Alternate method, calling SetFont() draws the text (in the
   modified font) OVER the "old" text--without erasing the background.
   The Alternate method erases the bg, but at the cost of some flicker.
   And, apparently, by screwing up the existing background and even(!!)
   the border of Edit controls?! (Not fixed by doing Invalidate(FALSE).)

   These changes are the Alternate method:

   StaticColor::SetWindowText
   {
      ...
   
      Invalidate();                 // w/o this, new text overlaps old text
      CStatic::SetWindowText(sz);
   }


   /
      We have to do this for those cases where something occurs
      that requires the background to be erased, but doesn't change
      the text (via SetWindowText()) such as WM_SETFONT.
   /

   BOOL StaticColor::OnEraseBkgnd(CDC* pDC)
   {
      if (_eBgColor == COLOR_TRANSPARENT)
      {
         DrawParentBackground();
         GetParent()->UpdateWindow();  // we have to do this after DrawParentBackground() or there's an infinite loop of WM_ERASEBKGNDs
                                       // BUT it causes a small flicker
         return TRUE;
      }

      return CStatic::OnEraseBkgnd(pDC);
   }
*/

#include "StaticColor.h"

BEGIN_MESSAGE_MAP(MyMFC::StaticColor, CStatic)
	ON_WM_CTLCOLOR_REFLECT()
END_MESSAGE_MAP()


COLORREF MyMFC::StaticColor::GetTextColorDefault()
{
   DWORD dwColor = ::GetSysColor(COLOR_BTNTEXT);
   return RGB(MyGetRValue(dwColor), MyGetGValue(dwColor), MyGetBValue(dwColor));
}


void MyMFC::StaticColor::SetTextColorDefault()
{
   _bTextColorSet = false;
   Invalidate(FALSE);
}

void MyMFC::StaticColor::SetTextColor(const COLORREF rgb)
{
   _bTextColorSet = true;
   _rgbText = rgb;
   Invalidate(FALSE);
}


void MyMFC::StaticColor::SetBgColorDefault()
{
   _brushBG.DeleteObject();

   _eBgColor = COLOR_DEFAULT;

   Invalidate(FALSE);
}

/*
   Of course, setting the bg to transparent means
   that we need to draw the background ourselves.
   (If the background is not redrawn, the
   text simply overwrites any previous text.)
*/
void MyMFC::StaticColor::SetBgColorTransparent()
{
   _brushBG.DeleteObject();

   _eBgColor = COLOR_TRANSPARENT;
   _brushBG.CreateStockObject(NULL_BRUSH);

   DrawParentBackground();
}

void MyMFC::StaticColor::SetBgColor(const int ixSysColor)
{
   SetBgColor(::GetSysColor(ixSysColor));
}

void MyMFC::StaticColor::SetBgColor(const COLORREF rgb)
{
   _brushBG.DeleteObject();

   _eBgColor = COLOR_SET;
   _brushBG.CreateSolidBrush(rgb);

   Invalidate(FALSE);
}


/*
   In order to set the text in a transparent control, we have to
   invalidate the part of the parent that contains this control
   (to make the parent draw its background again).

   We have to do this because our WM_CTLCONTROL handler returns
   a null (transparent) brush, which causes Windows to not
   paint the background.

   N.B.: This makes transparent backgrouccnds inappropriate for
         controls that frequently update their text.
*/
void MyMFC::StaticColor::SetWindowText(LPCTSTR sz)
{
   // if the bg is NOT transparent, just set the text normally
   if (_eBgColor != COLOR_TRANSPARENT)
   {
      CStatic::SetWindowText(sz);
      return;
   }

   /*
      We turn redraw off and then on so that the new text is
      not momentarily displayed as overwriting the old text.
   */
   SetRedraw(FALSE);
   CStatic::SetWindowText(sz);
   SetRedraw(TRUE);
   DrawParentBackground();
}


/*
   Have to "override" (it's not virtual) this function so that we can erase
   the background if it's transparent. We've tried working with
   WM_ERASEBKGND, but it flickers too much.
*/
void MyMFC::StaticColor::SetFont(CFont *pFont, BOOL bRedraw)
{
   CStatic::SetFont(pFont, bRedraw);
   DrawParentBackground();
}


/*
   We need to do this when we switch to a transparent background.
   That's because a transparent background draws *nothing* so we
   have to tell the parent to draw *its* background (which is
   really *our* background). Got it?
*/
void MyMFC::StaticColor::DrawParentBackground()
{
   /*
      Convert this window's client coords to the coord system
      of the parent and tell the parent to repaint that area.
   */
   CRect r;
   GetClientRect(r);
   ASSERT(GetParent() != nullptr);
   MapWindowPoints(GetParent(), r);
   GetParent()->InvalidateRect(r, TRUE);  // needs to be TRUE to trigger WM_ERASEBKGND, which might be where the parent draws its bg
}



/////////////////////////////////////////////////////////////////////////////
// StaticColor message handlers

HBRUSH MyMFC::StaticColor::CtlColor(CDC* pDC, UINT nCtlColor)
{
   if (nCtlColor != CTLCOLOR_STATIC)
      return HBRUSH(FALSE);            // parent handles this msg

   /*
      N.B.: If the bg color IS default, we STILL have to return SOMETHING
            so that we handle the msg (else text color is ignored).
   */
   HBRUSH hBrush;
   if (_eBgColor == COLOR_DEFAULT)
      hBrush = (HBRUSH) Default();  // can't call CStatic::OnCtlColor (recursive)
   else
      hBrush = _brushBG;

   pDC->SetBkMode(TRANSPARENT);

   if (_bTextColorSet)
      pDC->SetTextColor(_rgbText);

   return hBrush;
}
