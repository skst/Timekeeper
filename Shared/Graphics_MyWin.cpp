//---------------------------------------------------------------------------
// (c) 2006 12noon, Stefan K. S. Tucker
//---------------------------------------------------------------------------

#include "stdafx.h"

#include "Graphics_MyWin.h"

//--------------------------------------------------------------------------------------------


/*
   Ensure the passed rectangle is completely on-screen.
   The rectangle's dimensions aren't changed.
*/
void MyWin::MoveRectangleInsideScreen(RECT& r)
{
   RECT rBig;
   rBig.left = 0;
   rBig.right = ::GetSystemMetrics(SM_CXSCREEN);
   rBig.top = 0;
   rBig.bottom = ::GetSystemMetrics(SM_CYSCREEN);
   MyWin::MoveRectangleInsideRectangle(rBig, r);
}

/*
   Ensure the passed rectangle is completely inside the workarea (not the taskbar).
   The rectangle's dimensions aren't changed.
*/
void MyWin::MoveRectangleInsideWorkarea(RECT& r)
{
   RECT rcWorkArea;
   ::SystemParametersInfo(SPI_GETWORKAREA, 0, &rcWorkArea, 0);
   MyWin::MoveRectangleInsideRectangle(rcWorkArea, r);
}

/*
   Ensure the passed rectangle is completely inside another rectangle.
   The rectangle's dimensions aren't changed.
*/
void MyWin::MoveRectangleInsideRectangle(const RECT& rBig, RECT& rSmall)
{
   if (::IsRectEmpty(&rSmall))
      return;

   // ensure width of rect is fully on-screen
   const int w = rSmall.right - rSmall.left;
   const int cxBig = rBig.right - rBig.left;
   if (rSmall.left + w >= cxBig)
      ::OffsetRect(&rSmall, (cxBig - w) - rSmall.left, 0);

   // ensure height of rect is fully on-screen
   const int h = rSmall.bottom - rSmall.top;
   const int cyBig = rBig.bottom - rBig.top;
   if (rSmall.top + h >= cyBig)
      ::OffsetRect(&rSmall, 0, (cyBig - h) - rSmall.top);

   // more important that the upper-left corner be in the rect, so we do this last
   if (rSmall.left < 0)
      ::OffsetRect(&rSmall, -rSmall.left, 0);
   if (rSmall.top < 0)
      ::OffsetRect(&rSmall, 0, -rSmall.top);
}
