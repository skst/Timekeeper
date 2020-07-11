//---------------------------------------------------------------------------
// (c) 2006 Stefan K. S. Tucker, 12noon
//---------------------------------------------------------------------------

#pragma once

namespace MyMFC
{

/*
   N.B. If two StaticColor controls overlap,
   the lower one (in the Z-order) receives all mouse events.
   Not sure why. See "Static Controls." It even receives
   WM_NCHITTEST so we can't change it--apparently.
*/

class StaticColor : public CStatic
{
public:
   enum EColor
   {
      COLOR_DEFAULT,       // the color is the corresponding Windows system color
      COLOR_SET,           // the color is specified
      COLOR_TRANSPARENT,   // the "color" is transparent
   };

protected:
   bool _bTextColorSet;
   COLORREF _rgbText;

   EColor _eBgColor;
   CBrush _brushBG;

public:
   StaticColor()
   {
      _bTextColorSet = false;
      _eBgColor = COLOR_DEFAULT;
   }
   virtual ~StaticColor() {}

   void SetWindowText(LPCTSTR sz);
   void SetFont(CFont *pFont, BOOL bRedraw = TRUE);

   static COLORREF GetTextColorDefault();
   void SetTextColorDefault();
   void SetTextColor(const BYTE r, const BYTE g, const BYTE b) { SetTextColor(RGB(r,g,b)); }
   void SetTextColor(const COLORREF rgb);

   void SetBgColorDefault();
   void SetBgColorTransparent();
   void SetBgColor(const BYTE r, const BYTE g, const BYTE b) { SetBgColor(RGB(r,g,b)); }
   void SetBgColor(const int ixSysColor);
   void SetBgColor(const COLORREF rgb);

protected:
   void DrawParentBackground();

	afx_msg HBRUSH CtlColor(CDC* pDC, UINT nCtlColor);

	DECLARE_MESSAGE_MAP()
};

}
