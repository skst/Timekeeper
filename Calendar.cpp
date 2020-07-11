#include "stdafx.h"

#include "Shared/Graphics_MyWin.h"
#include "Shared/Utility.h"

#include "Calendar.h"

const LPCTSTR REG_SECTION_NAME = _T("Calendar");

const LPCTSTR REG_ENTRY_SHOWTODAY = _T("ShowToday");
const LPCTSTR REG_ENTRY_WEEKNUMS = _T("WeekNumbers");
const LPCTSTR REG_ENTRY_FONT = _T("Font");

const LPCTSTR REG_ENTRY_SAVE_POSITION = _T("SavePosition");
const LPCTSTR REG_ENTRY_POSX = _T("PositionX");
const LPCTSTR REG_ENTRY_POSY = _T("PositionY");

const LPCTSTR REG_ENTRY_SAVE_SIZE = _T("SaveSize");
const LPCTSTR REG_ENTRY_NX = _T("NumMonthsX");
const LPCTSTR REG_ENTRY_NY = _T("NumMonthsY");

IMPLEMENT_DYNAMIC(CalendarWindow, CMonthCalCtrl)

CalendarWindow::CalendarWindow()
{
   _sizeMinClient.cx = 0;
   _sizeMinClient.cy = 0;

   /*
      get calendar options
   */
   _bShowToday = !!::AfxGetApp()->GetProfileInt(REG_SECTION_NAME, REG_ENTRY_SHOWTODAY, true);
   _bShowWeekNums = !!::AfxGetApp()->GetProfileInt(REG_SECTION_NAME, REG_ENTRY_WEEKNUMS, false);

   _bSavePosition = !!::AfxGetApp()->GetProfileInt(REG_SECTION_NAME, REG_ENTRY_SAVE_POSITION, false);
   _xPos = ::AfxGetApp()->GetProfileInt(REG_SECTION_NAME, REG_ENTRY_POSX, -1);
   _yPos = ::AfxGetApp()->GetProfileInt(REG_SECTION_NAME, REG_ENTRY_POSY, -1);

   _bSaveSize = !!::AfxGetApp()->GetProfileInt(REG_SECTION_NAME, REG_ENTRY_SAVE_SIZE, true);
   _nNumMonthsX = ::AfxGetApp()->GetProfileInt(REG_SECTION_NAME, REG_ENTRY_NX, 2);
   _nNumMonthsY = ::AfxGetApp()->GetProfileInt(REG_SECTION_NAME, REG_ENTRY_NY, 1);

   /*
      set up calendar font
   */
   // if there isn't an entry, init to icon title font
   BYTE *pData = NULL;
   UINT uiNumBytes = 0;
   if (!::AfxGetApp()->GetProfileBinary(REG_SECTION_NAME, REG_ENTRY_FONT, &pData, &uiNumBytes))
   {
      ASSERT(pData == NULL);
      ::SystemParametersInfo(SPI_GETICONTITLELOGFONT, sizeof _lfFont, &_lfFont, 0);
   }
   else
   {
      ASSERT(uiNumBytes > 0);
      ASSERT(pData != NULL);
      ::memcpy(&_lfFont, pData, uiNumBytes);
      delete [] pData;
   }

   // create font
   VERIFY(_font.CreateFontIndirect(&_lfFont));
}


/*
   This version of Create() uses the information loaded from the Registry.
*/
bool CalendarWindow::Create()
{
   return Create(_xPos, _yPos, _nNumMonthsX, _nNumMonthsY, _bShowToday, _bShowWeekNums);
}


bool CalendarWindow::Create(const int xPos, const int yPos, const int nNumMonthsX, const int nNumMonthsY, const bool bShowToday, const bool bShowWeekNums)
{
   if (GetSafeHwnd() != NULL)
      return true;

   /*
      Save the passed values as if they were our own.
   */
   _xPos = xPos;
   _yPos = yPos;
   _nNumMonthsX = nNumMonthsX;
   _nNumMonthsY = nNumMonthsY;
   _bShowToday = bShowToday;
   _bShowWeekNums = bShowWeekNums;

   CPoint pt(_xPos, _yPos);
   if (!_bSavePosition || (_xPos == -1) || (_yPos == -1))
      ::GetCursorPos(&pt);

   //Client edge: raised border which implies the window can be resized
   if (CreateEx(WS_EX_PALETTEWINDOW | WS_EX_CLIENTEDGE, MONTHCAL_CLASS, _T("Calendar"),
                  WS_OVERLAPPED | WS_POPUP | WS_BORDER | WS_THICKFRAME | WS_CAPTION | WS_SYSMENU | MCS_MULTISELECT,
                  pt.x, pt.y, 0, 0, GetSafeHwnd(), 0) == 0)
   {
      return false;
   }

	if (!bShowToday)
      TodayHide();
   if (bShowWeekNums)
      WeekNumbersShow();

   /*
      MCSC_BACKGROUND  Set the background color displayed between months. 
      MCSC_MONTHBK  Set the background color displayed within the month. 
      MCSC_TEXT  Set the color used to display text within a month. 
      MCSC_TITLEBK  Set the background color displayed in the calendar's title. 
      MCSC_TITLETEXT  Set the color used to display text within the calendar's title. 
      MCSC_TRAILINGTEXT  Set the color used to display header day and trailing day text.
                        Header and trailing days are the days from the previous and following months that appear on the current month calendar. 
   */
   //SetColor(MCSC_BACKGROUND, RGB(0x80, 0x80, 0x80));

   SetMonthDelta(1);

   SetFont(&_font, TRUE);

   // size calendar for given # months
   Resize();
   return true;
}


/*
   Find out how big the window must be for a certain number of months.
   The returned size is that of the WINDOW (not the CLIENT area).

   In theory, we could subtract the height of "Today:" if it's not
   displayed, but that seems to cause no end of problems.

   N.B.: The control's maximum size is 4x3 or 3x4 -- one year.
*/
CSize CalendarWindow::GetRequiredDimensions(const int nNumMonthsX, const int nNumMonthsY)
{
   CRect rc(CPoint(0,0), _sizeMinClient);

   const LONG nWidthPixelsToday = (LONG)SendMessage(MCM_GETMAXTODAYWIDTH);
   if (nWidthPixelsToday > rc.Width())
      rc.right = nWidthPixelsToday;

   // limit control to 4x4 because it can only show 4x3 or 3x4 months
   rc.right = rc.Width() * min(4, nNumMonthsX);
   rc.bottom = rc.Height() * min(4, nNumMonthsY);

   // convert from client dimensions to window dimensions
   ::AdjustWindowRectEx(rc, GetStyle(), FALSE /*menu*/, GetExStyle());

//   TRACE("Calendar dimensions: %dx%d\n", rc.Width(), rc.Height());
   return rc.Size();
}


/*
   Resize the calendar to contain the # of months specified in the
   member variables. Try to maintain the same position, but if
   necessary, move to keep in the work area.
*/
void CalendarWindow::Resize()
{
   if (GetSafeHwnd() == NULL)
      return;

   // get client area of 1x1 calendar (used for sizing)
   CalculateMinimumClientArea();

   /*
      get current window position and factor in required
      window dimensions for this # of months
   */
   CRect rWndw;
   GetWindowRect(rWndw);
   rWndw.BottomRight() = rWndw.TopLeft() + GetRequiredDimensions(_nNumMonthsX, _nNumMonthsY);

   /*
      ensure the calendar is fully on-screen
   */
   MyWin::MoveRectangleInsideWorkarea(rWndw);

   SetWindowPos(NULL, rWndw.left, rWndw.top, rWndw.Width(), rWndw.Height(), SWP_NOZORDER | SWP_NOREPOSITION | SWP_SHOWWINDOW);

   /*
      update the # of days the user can select
   */
   SetMaxSelCount(7 * 5 * _nNumMonthsX * _nNumMonthsY);  // can select up to the entire display
}


/*
   Calculate the client size of a 1x1 calendar.
   We need this for determining the size of a calendar,
   based on the # of months.
*/
void CalendarWindow::CalculateMinimumClientArea()
{
   /*
      BUG:

      If we change the font to something smaller, this function does NOT
      return a smaller size for the calendar. It returns a larger size
      if we increase the font size, but it does not return a smaller size.

      Even if we destroy the window and re-create it, it continues to return
      the same (larger) size. We have to close the band and re-open it to
      see the correct size. WHY?!
   */

   /*
      If we just call GetMinReqRect(), it returns the WINDOW size, not the client.
      Since there's no way to convert window to client (like we can convert
      client to window with AdjustWindowRect), we have to resize the window
      to 1x1 and then resize it back, and cope with the unavoidable
      WM_WINDOWPOSCHANGED messages. Ugh.

      If we just call SizeMinReq(FALSE), it resizes the window to 1x1 and
      causes WM_WINDOWPOSCHANGED to be sent and that changes _nNumMonthsX/Y.
   */
   // save old dimensions
   CRect rw;
   GetWindowRect(rw);
   // resize to 1x1
   SizeMinReq(FALSE);
   // get client dimensions
   CRect rc;
   GetClientRect(rc);      // min size = 205x142
   // resize to original dimensions
   SetWindowPos(NULL, 0, 0, rw.Width(), rw.Height(), SWP_NOZORDER | SWP_NOREPOSITION | SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOSENDCHANGING);

TRACE(__FUNCTION__ ": min size = %dx%d\n", rc.Width(), rc.Height());
   _sizeMinClient = rc.Size();
}


void CalendarWindow::_TodayShow(const bool b)
{
   _bShowToday = b;
   ::AfxGetApp()->WriteProfileInt(REG_SECTION_NAME, REG_ENTRY_SHOWTODAY, _bShowToday);

   if (GetSafeHwnd() != NULL)
   {
      if (_bShowToday)
         ModifyStyle(MCS_NOTODAY, 0);
      else
         ModifyStyle(0, MCS_NOTODAY);
   }
}


void CalendarWindow::_WeekNumbersShow(const bool b)
{
   _bShowWeekNums = b;
   ::AfxGetApp()->WriteProfileInt(REG_SECTION_NAME, REG_ENTRY_WEEKNUMS, _bShowWeekNums);

   if (GetSafeHwnd() != NULL)
   {
		/*
			From MSDN: Week 1 is the first week with 4 or more days.
		*/
      if (_bShowWeekNums)
         ModifyStyle(0, MCS_WEEKNUMBERS);
      else
         ModifyStyle(MCS_WEEKNUMBERS, 0);
   }

   // the calendar's size changes slightly, so recalc
   Resize();
}


void CalendarWindow::_SavePosition(const bool b)
{
   _bSavePosition = b;
   ::AfxGetApp()->WriteProfileInt(REG_SECTION_NAME, REG_ENTRY_SAVE_POSITION, _bSavePosition);
}


void CalendarWindow::_SaveSize(const bool b)
{
   _bSaveSize = b;
   ::AfxGetApp()->WriteProfileInt(REG_SECTION_NAME, REG_ENTRY_SAVE_SIZE, _bSaveSize);
}


void CalendarWindow::ChooseFont()
{
   // if we passed _lfFont, the dlg would modify it directly
   LOGFONT lf = _lfFont;
   CFontDialog dlg(&lf, CF_SCREENFONTS | CF_INITTOLOGFONTSTRUCT /*| CF_APPLY*/, NULL, this);
   if (dlg.DoModal() != IDOK)
      return;

   dlg.GetCurrentFont(&_lfFont);

   /*
      replace current font with the new one
   */
   HGDIOBJ hFontOld = _font.Detach();
   ASSERT(hFontOld != NULL);

   // create and set font
   VERIFY(_font.CreateFontIndirect(&_lfFont));
   if (GetSafeHwnd() != NULL)
      SetFont(&_font, TRUE);

   DeleteObject(hFontOld);

   ::AfxGetApp()->WriteProfileBinary(REG_SECTION_NAME, REG_ENTRY_FONT, (BYTE*) &_lfFont, sizeof _lfFont);

   /*
      This affects the calendar dimensions so we have to
      re-calculate and resize.
   */
   Resize();
}


BEGIN_MESSAGE_MAP(CalendarWindow, CMonthCalCtrl)
   ON_WM_DESTROY()
   ON_WM_GETMINMAXINFO()
   ON_WM_SIZING()
   ON_WM_KEYDOWN()
   ON_WM_WINDOWPOSCHANGED()
END_MESSAGE_MAP()


// CalendarWindow message handlers

void CalendarWindow::OnGetMinMaxInfo(MINMAXINFO* lpMMI)
{
   __super::OnGetMinMaxInfo(lpMMI);
//   TRACE(__FUNCTION__ ": mintrack=%dx%d, maxtrack=%dx%d\n", lpMMI->ptMinTrackSize.x, lpMMI->ptMinTrackSize.y, lpMMI->ptMaxTrackSize.x, lpMMI->ptMaxTrackSize.y);

   if (GetSafeHwnd() == NULL)
      return;

   const CSize sz(GetRequiredDimensions(1, 1));
   lpMMI->ptMinTrackSize.x = sz.cx;
   lpMMI->ptMinTrackSize.y = sz.cy;
}


void CalendarWindow::OnSizing(UINT fwSide, LPRECT pRect)
{
   __super::OnSizing(fwSide, pRect);

   CRect rDrag(pRect);
//   TRACE(__FUNCTION__ ": r=(%d,%d) %dx%d\n", pRect->left, pRect->top, rDrag.Width(), rDrag.Height());

   /*
      We want drag rect to be a multiple of the dimensions of a 1x1 calendar.
      First, we figure out the nearest multiple of months across, then down.
      Then we figure out which side to adjust to achieve that.

      Doubly hard because we need to account for the non-client areas as well.
      Do we just loop through all possible dimensions and find the closest one??
      No, this method is close enough.
   */
   const int cx = MyWin::RoundToNearestMultiple(rDrag.Width(), _sizeMinClient.cx);
   const int cy = MyWin::RoundToNearestMultiple(rDrag.Height(), _sizeMinClient.cy);
   const CSize sz(GetRequiredDimensions(cx / _sizeMinClient.cx, cy / _sizeMinClient.cy));

   if ((fwSide == WMSZ_TOPLEFT) || (fwSide == WMSZ_LEFT) || (fwSide == WMSZ_BOTTOMLEFT))
      pRect->left = pRect->right - sz.cx;
   else if ((fwSide == WMSZ_TOPRIGHT) || (fwSide == WMSZ_RIGHT) || (fwSide == WMSZ_BOTTOMRIGHT))
      pRect->right = pRect->left + sz.cx;

   if ((fwSide == WMSZ_TOPLEFT) || (fwSide == WMSZ_TOP) || (fwSide == WMSZ_TOPRIGHT))
      pRect->top = pRect->bottom - sz.cy;
   else if ((fwSide == WMSZ_BOTTOMLEFT) || (fwSide == WMSZ_BOTTOM) || (fwSide == WMSZ_BOTTOMRIGHT))
      pRect->bottom = pRect->top + sz.cy;
}


void CalendarWindow::OnDestroy()
{
   __super::OnDestroy();
   TRACE(__FUNCTION__ "\n");

   CRect r;
   GetWindowRect(r);

   if (_bSavePosition)
   {
      _xPos = r.left;
      _yPos = r.top;

      ::AfxGetApp()->WriteProfileInt(REG_SECTION_NAME, REG_ENTRY_POSX, _xPos);
      ::AfxGetApp()->WriteProfileInt(REG_SECTION_NAME, REG_ENTRY_POSY, _yPos);
   }

   if (_bSaveSize)
   {
      _nNumMonthsX = r.Width() / _sizeMinClient.cx;
      _nNumMonthsY = r.Height() / _sizeMinClient.cy;
      ::AfxGetApp()->WriteProfileInt(REG_SECTION_NAME, REG_ENTRY_NX, _nNumMonthsX);
      ::AfxGetApp()->WriteProfileInt(REG_SECTION_NAME, REG_ENTRY_NY, _nNumMonthsY);
   }
}

void CalendarWindow::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
   /*
      If the user hits ESC, close the calendar.
   */
   if (nChar == VK_ESCAPE)
      DestroyWindow();
   else
      __super::OnKeyDown(nChar, nRepCnt, nFlags);
}


void CalendarWindow::OnWindowPosChanged(WINDOWPOS *pWndPos)
{
   TRACE(__FUNCTION__ "\n");
   __super::OnWindowPosChanged(pWndPos);

   if (pWndPos->flags & SWP_NOSIZE)
      return;

   if ((_sizeMinClient.cx == 0) || (_sizeMinClient.cy == 0))
      return;

   /*
      calculate how many months are displayed
   */
   // pWndPos size is the same as returned by GetWindowRect()
   _nNumMonthsX = pWndPos->cx / _sizeMinClient.cx;
   _nNumMonthsY = pWndPos->cy / _sizeMinClient.cy;

   /*
      update the # of days the user can select
   */
   SetMaxSelCount(7 * 5 * _nNumMonthsX * _nNumMonthsY);  // can select up to the entire display
}
