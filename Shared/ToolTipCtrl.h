//---------------------------------------------------------------------------
// (c) 2006 12noon, Stefan K. S. Tucker
//---------------------------------------------------------------------------

#pragma once

#include <commctrl.h>

//TODO: save passed LPCTSTR text in a vector of strings and point TOOLINFO to *that*.
//TODO: allow passing inst/id for text

/*
   There are a number of different ways to use tooltips:

   Static text
      The text is hardcoded for the window (or area).
   Dynamic text
      The text changes every time the tip is displayed.

      If the text is dynamic, the notification messages can be processed
      by the same window (the control) or by a different window (unless
      specifying a rectangular area--see below).

   Whole window
      A tooltip is assigned to a window's client area.
   Rectangular areas
      A tooltip is assigned to a rectangular area of a window's client area.

      When specifying a window's rectangular area for the tool, the
      notification messages must be processed by that window. This is because
      the "id" field is used to indicate a separate window or a tool id and
      a tool id is required when specifying a rectangular area.

   Fixed tooltip
      The tooltip is displayed when the cursor enters the window (or area)
      and doesn't move or change until the cursor leaves.
   Tracking tooltip
      The tooltip is displayed when the cursor enters the window (or area)
      and tooltip window moves and the text changes as the cursor moves
      around within the window (or area).


   N.B. MFC's "support" for tooltips requires the controls' parent window
        to process the notification messages. Attempting to get around that
        by overriding OnHitText() sort of works, but only for one control.
        Tooltips are disabled for other windows. Weird.
*/

/*
   These calls indicate either a FIXED tooltip--it doesn't move--or a TRACKING tooltip--it moves with the mouse.
   For the fixed version, call the version without "Track" at the end of the function name.
   For the tracking version, call the version with "Track" at the end of the function name.

      AddToolTextStatic[Track]
         The tooltip's text doesn't change
         The tooltip is displayed over the window with the passed HWND
         OR, the tooltip is displayed over an area, so pass the area's unique id and a RECT

      AddToolTextDynamic[Track]
         The tooltip's text changes (via the notification function of the sink's HWND, or the control's HWND if no sink is specified)
         The tooltip is displayed over the window with the passed HWND

      AddToolTextDynamic[Track]
         The tooltip's text changes (via the notification function of the control's HWND)
         The tooltip is displayed over an area, so pass the area's unique id and a RECT
*/

/*
   One window can receive the WM_NOTIFY messages while the cursor is over
   another window. The control is the window that displays a tooltip when
   the mouse hovers over it (and causes notification messages to be sent).
   The two windows can be the same, in which case the control window
   processes its own notification messages.

   The drawback is that the tooltip doesn't move and change based on the
   mouse's position inside the window. To do that, we have to use TTF_TRACK
   and process the WM_MOUSEMOVE ourselves, rather than use TTF_SUBCLASS.
*/

/*
   Example of handling notification messages:

   [window sink class derived from CWnd]

   :: sink.h ::
   afx_msg BOOL OnToolTipNotify(UINT id, NMHDR *pNMHDR, LRESULT *pResult);

   :: sink.cpp ::
   ON_NOTIFY_EX(TTN_GETDISPINFO, 0, OnToolTipNotify)
//   ON_NOTIFY_EX_RANGE(TTN_GETDISPINFO, 0, 0xFFFF, OnToolTipNotify)

   BOOL WndSink::OnToolTipNotify(UINT id, NMHDR *pNMHDR, LRESULT *pResult)
   {
      ASSERT(id == 0);
      NMTTDISPINFO *pttDispInfo = reinterpret_cast<NMTTDISPINFO *>(pNMHDR);
      *pResult = 0;

      // pttDispInfo->hdr.hwndFrom is the associated tool wndw

      POINT pt;
      ::GetCursorPos(&pt);
      ::ScreenToClient(GetSafeHwnd(), &pt);
      _snprintf(pttDispInfo->szText, sizeof pttDispInfo->szText, "WindowSink::OnToolTipNotify(%#x) (%d,%d)", pttDispInfo->hdr.idFrom, pt.x, pt.y);
      TRACE(_T("%s\n"), pttDispInfo->szText);

      return TRUE;
   }
*/

// Yes, this class can be derived from CWnd. See the //CWnd comments.

namespace skst
{

class MyToolTipCtrl //CWnd : public CWnd
{
public:
   typedef std::vector<MyToolTipCtrl*> Instances;

protected:
   HWND m_hWnd;//CWnd
   const bool _bOnlyShowTipsIfActive;  // if set, we only display tooltips if the app is active

   UINT_PTR _idTimer;                  // remember if a wndw is being tracked for WM_MOUSELEAVE
   static HWND _hWndTrack;             // there's only one tip at a time, so this can be static

   static HHOOK _hMsgHook;
   static Instances _instances;

   static RECT rectNull;

public:
	MyToolTipCtrl(const bool bOnlyShowTipsIfActive = true);
   virtual ~MyToolTipCtrl();

   MyToolTipCtrl& operator=(const MyToolTipCtrl& a)
   {
      if (this == &a)
         return *this;

      m_hWnd = a.m_hWnd;
// these are const members and we never use the assignment operator anyway
//const     _bOnlyShowTipsIfActive = a._bOnlyShowTipsIfActive;
      _idTimer = a._idTimer;

      // the other members are static

      return *this;
   }

	bool Create(const bool bBalloon = false);

   HWND GetSafeHwnd() { return m_hWnd; }//CWnd

   // ** FIXED
   // * static (window or rect)
   void AddToolTextStatic(const HWND hWndControl,        LPTSTR szTooltip,       const UINT id = 0, const RECT& rect = rectNull);
   // * dynamic (window)
   void AddToolTextDynamic(const HWND hWndControl,       HWND hWndSink = nullptr);
   // * dynamic (rect)
   void AddToolTextDynamic(const HWND hWndControl,       const UINT id, const RECT& rect);

   // ** TRACKING
   // * static (window vs rect)
   void AddToolTextStaticTrack(const HWND hWndControl,   LPTSTR szTooltip,       const UINT id = 0, const RECT& rect = rectNull);
   // * dynamic (window)
   void AddToolTextDynamicTrack(const HWND hWndControl,  HWND hWndSink = nullptr);
   // * dynamic (rect)
   void AddToolTextDynamicTrack(const HWND hWndControl,  const UINT id, const RECT& rect);

   void DeleteTools();
   void DeleteTool(const HWND hWndSink, const UINT_PTR id = 0);

	void SetFont(const LOGFONT *lplf);
	void SetTipTextColor(COLORREF clr);
	void SetBkTipColor(COLORREF clr);
//TODO: what are the units of width??! Geez, Microsoft doc is awful.
   void SetMaxTipWidth(const int iWidth);

protected:
   void AddTool(const bool bTrack, const HWND hWndControl, HWND hWndSink, const UINT id = 0, const RECT& rect = rectNull, LPTSTR szToolTip = LPSTR_TEXTCALLBACK);

   int GetToolCount() const { return (int) ::SendMessage(m_hWnd, TTM_GETTOOLCOUNT, 0, 0); }

   bool HitTooltipControl(HWND hWnd, const POINT& pt);
   bool HitTool(const TOOLINFO& ti, const HWND hWnd, const POINT& pt);
   void ActivateTooltip(const TOOLINFO& ti, bool bActivate);

   static LRESULT CALLBACK MouseProcTrack(int nCode, WPARAM wParam, LPARAM lParam);
   static VOID CALLBACK TimerProcTrack(HWND /*hwnd*/, UINT /*uMsg*/, UINT_PTR /*idEvent*/, DWORD /*dwTime*/);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(MyToolTipCtrl)
	//}}AFX_VIRTUAL

	// Generated message map functions
//	//{{AFX_MSG(MyToolTipCtrl)
//		// NOTE - the ClassWizard will add and remove member functions here.
//	//}}AFX_MSG
//CWnd   DECLARE_MESSAGE_MAP()
};

}
