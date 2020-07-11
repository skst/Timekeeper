//---------------------------------------------------------------------------
// (c) 2006-2020 12noon, Stefan K. S. Tucker
//---------------------------------------------------------------------------

#include "stdafx.h"

#include "skstutil.h"
#include "Graphics_MyWIn.h"
#include "MyLog.h"

#include "ToolTipCtrl.h"
using skst::MyToolTipCtrl;


// Uncomment this line to enable tooltip debugging
//#define _DEBUG_TOOLTIPS

//CWnd BEGIN_MESSAGE_MAP(MyToolTipCtrl, CWnd)
//	//{{AFX_MSG_MAP(MyToolTipCtrl)
//		// NOTE - the ClassWizard will add and remove mapping macros here.
//	//}}AFX_MSG_MAP
//CWnd END_MESSAGE_MAP()

RECT MyToolTipCtrl::rectNull = { 0, 0, 0, 0 };

HHOOK MyToolTipCtrl::_hMsgHook = nullptr;
MyToolTipCtrl::Instances MyToolTipCtrl::_instances;


MyToolTipCtrl::MyToolTipCtrl(const bool bOnlyShowTipsIfActive)
   : _bOnlyShowTipsIfActive(bOnlyShowTipsIfActive)
{
	INITCOMMONCONTROLSEX icx;
	icx.dwSize = sizeof icx;
	icx.dwICC = ICC_BAR_CLASSES;
   ::InitCommonControlsEx(&icx);

   m_hWnd = nullptr;
   _idTimer = 0;

   _instances.push_back(this);

   skst::MyLog::GetLog().LogV(_T("MyToolTipCtrl: %#x is the %d-th instance"), this, _instances.size());
}

MyToolTipCtrl::~MyToolTipCtrl()
{
   Instances::iterator it = std::find(_instances.begin(), _instances.end(), this);
   assert(it != _instances.end());
   _instances.erase(it);

   // if there are no more instances, unhook
   if (_instances.size() == 0)
   {
      if (_hMsgHook != nullptr)
      {
         ::UnhookWindowsHookEx(_hMsgHook);
         _hMsgHook = nullptr;
      }
   }

   if (m_hWnd != nullptr)
   {
      ::DestroyWindow(m_hWnd);
      m_hWnd = nullptr;
   }
}


/*
   create the tooltip control. return true if successful.
*/
bool MyToolTipCtrl::Create(const bool bBalloon)
{
   if (m_hWnd != nullptr)
      return true;

   /*
      No parent window needs to be specified because tooltips don't seem to need one.
      Plus: This assert is not always true. Apparently, a tooltip control's parent
      cannot be a dialog child control. If the handle to a dialog child control
      is passed, the parent is actually the parent of the passed control. Why?
   */
   //ASSERT(hWndParent == ::GetParent(m_hWnd));

//CWnd   if (!CreateEx(
   m_hWnd = ::CreateWindowEx(//CWnd
                           WS_EX_TOOLWINDOW | WS_EX_TOPMOST, TOOLTIPS_CLASS, 
                           nullptr, WS_POPUP | WS_BORDER | TTS_ALWAYSTIP | (bBalloon ? TTS_BALLOON : 0),
                           0, 0, 0, 0, 
                           nullptr, nullptr,
                           ::GetModuleHandle(nullptr), nullptr);//CWnd
   assert(m_hWnd != nullptr);
   return (m_hWnd != nullptr);
}


/////////////////////////////////////////////////////////////////////////////
//
// delete tools
//
void MyToolTipCtrl::DeleteTools()
{
   while (GetToolCount() > 0)
   {
      TOOLINFO ti;
      ti.cbSize = sizeof ti;
      ti.lpszText = nullptr;
      ::SendMessage(m_hWnd, TTM_ENUMTOOLS, 0, (LPARAM) &ti);

      DeleteTool(ti.hwnd, ti.uId);
   }
}

void MyToolTipCtrl::DeleteTool(const HWND hWndSink, const UINT_PTR id)
{
	TOOLINFO ti;
	ti.cbSize = sizeof ti;
	ti.hwnd = hWndSink;
   ti.uFlags = 0;
   if (id == 0)
   {
      ti.uFlags |= TTF_IDISHWND;
	   ti.uId = (UINT_PTR) hWndSink;
   }
   else
	   ti.uId = id;
   ::SendMessage(m_hWnd, TTM_DELTOOL, 0, (LPARAM) &ti);
}


/////////////////////////////////////////////////////////////////////////////
//
// fixed tooltips
//
void MyToolTipCtrl::AddToolTextStatic(const HWND hWndControl, LPTSTR szTooltip, const UINT id, const RECT& rect)
{
   assert(hWndControl != nullptr);
   assert(szTooltip != nullptr);
   AddTool(false, hWndControl, nullptr, id, rect, szTooltip);
}

void MyToolTipCtrl::AddToolTextDynamic(const HWND hWndControl, HWND hWndSink)
{
   assert(hWndControl != nullptr);
   AddTool(false, hWndControl, hWndSink);
}

void MyToolTipCtrl::AddToolTextDynamic(const HWND hWndControl, const UINT id, const RECT& rect)
{
   assert(hWndControl != nullptr);
   AddTool(false, hWndControl, nullptr, id, rect);
}

/////////////////////////////////////////////////////////////////////////////
//
// tracking tooltips
//
void MyToolTipCtrl::AddToolTextStaticTrack(const HWND hWndControl, LPTSTR szTooltip, const UINT id, const RECT& rect)
{
   assert(hWndControl != nullptr);
   assert(szTooltip != nullptr);
   AddTool(true, hWndControl, nullptr, id, rect, szTooltip);
}

void MyToolTipCtrl::AddToolTextDynamicTrack(const HWND hWndControl, HWND hWndSink)
{
   assert(hWndControl != nullptr);
   AddTool(true, hWndControl, hWndSink);
}

void MyToolTipCtrl::AddToolTextDynamicTrack(const HWND hWndControl, const UINT id, const RECT& rect)
{
   assert(hWndControl != nullptr);
   AddTool(true, hWndControl, nullptr, id, rect);
}

void MyToolTipCtrl::AddTool(const bool bTrack, const HWND hWndControl, HWND hWndSink, const UINT id, const RECT& rect, LPTSTR szToolTip)
{
   assert(m_hWnd != nullptr);       // have to call Create() first

   assert(hWndControl != nullptr);
   assert((szToolTip != nullptr) || (::GetWindowLong(hWndControl, GWL_STYLE) & (SS_NOTIFY | BS_NOTIFY))); // the ctrl needs to notify if there's no tip text
   if (hWndSink == nullptr)
      hWndSink = hWndControl;

	TOOLINFO ti;
	ti.cbSize = sizeof ti;
	ti.hwnd = hWndSink;
   if (bTrack)
      ti.uFlags = TTF_TRACK | TTF_ABSOLUTE;//track (not necessary though--weird)
   else
      ti.uFlags = TTF_SUBCLASS;
   if (id == 0)
   {
      ti.uFlags |= TTF_IDISHWND;
	   ti.uId = (UINT_PTR) hWndControl;
   }
   else
   {
	   ti.uId = id;
      ti.rect = rect;
   }
	ti.lpszText = szToolTip;

   ::SendMessage(m_hWnd, TTM_ADDTOOL, 0, (LPARAM) &ti);

   /*
      Install a hook to monitor all mouse messages and pass them on to the
      tooltip control. This way, we know if the mouse is moving within the
      control and we can change the tooltip dynamically.
   */
   if (bTrack && (_hMsgHook == nullptr))
   {
      skst::MyLog::GetLog().LogV(_T("Setting Mouse hook...\n"));
      _hMsgHook = ::SetWindowsHookEx(WH_MOUSE, MyToolTipCtrl::MouseProcTrack, nullptr, ::GetCurrentThreadId());
   }
}


/////////////////////////////////////////////////////////////////////////////
//
// tracking support
//
LRESULT CALLBACK MyToolTipCtrl::MouseProcTrack(int nCode, WPARAM wParam, LPARAM lParam)
{
#if defined(_DEBUG_TOOLTIPS)
   skst::LogInOut lio("MyToolTipCtrl::MouseProcTrack");
#endif

   if (nCode >= 0)
   {
//      if ((wParam >= WM_MOUSEFIRST) && (wParam <= WM_MOUSELAST))
      if (wParam == WM_MOUSEMOVE)
      {
         MOUSEHOOKSTRUCT *pHook = reinterpret_cast<MOUSEHOOKSTRUCT *>(lParam);

#if defined(_DEBUG_TOOLTIPS)
         skst::MyLog::GetLog().LogV("MyToolTipCtrl::MouseProcTrack(%#x) (%d,%d) parent = %#x\n", pHook->hwnd, pHook->pt.x, pHook->pt.y, ::GetParent(pHook->hwnd));
#endif

         /*
            We need to check all instances of this class (all tool windows)
            to see if any of their tools have been hit. If so, we send the
            message to that tool window.
         */
         for (Instances::const_iterator it = _instances.begin(); it != _instances.end(); ++it)
         {
#if defined(_DEBUG_TOOLTIPS)
            skst::MyLog::GetLog().LogV("MyToolTipCtrl::MouseProcTrack: hit-test tooltip ctrl %#x...\n", *it);
#endif

            // is the cursor over a tool?
            (*it)->HitTooltipControl(pHook->hwnd, pHook->pt);
            /*
               We don't want to stop hit-testing more tooltip controls even if
               we find one that's active because if we stop, we won't DEACTIVATE
               the ones in the list that are after the one we're ACTIVATING.
            */
         }
      }
   }

   return ::CallNextHookEx(_hMsgHook, nCode, wParam, lParam);
}


VOID CALLBACK MyToolTipCtrl::TimerProcTrack(HWND /*hwnd*/, UINT /*uMsg*/, UINT_PTR /*idEvent*/, DWORD /*dwTime*/)
{
   POINT pt;
   ::GetCursorPos(&pt);
   const HWND hUnder = ::WindowFromPoint(pt);
   if (_hWndTrack != hUnder)
   {
#if defined(_DEBUG_TOOLTIPS)
      skst::MyLog::GetLog().LogV("TimerProcTrack (%#x): pt(%dx%d) NOT in wndw\n", _hWndTrack, pt.x, pt.y);
#endif
      /*
         Basically, this is telling each tooltip control that the mouse is over this other window now.
      */
      for (Instances::const_iterator it = _instances.begin(); it != _instances.end(); ++it)
      {
         // is the cursor over a tool?
         (*it)->HitTooltipControl(hUnder, pt);
      }
   }
#if defined(_DEBUG_TOOLTIPS)
   else
      skst::MyLog::GetLog().LogV("TimerProcTrack (%#x): pt(%dx%d) in wndw\n", _hWndTrack, pt.x, pt.y);
#endif
}


bool MyToolTipCtrl::HitTooltipControl(HWND hWnd, const POINT& pt)
{
   if (m_hWnd == nullptr)
      return false;

#if defined(_DEBUG_TOOLTIPS)
   skst::MyLog::GetLog().LogV("\tTooltip control has %d tools\n", GetToolCount());
#endif

   TOOLINFO ti;
   ti.cbSize = sizeof ti;
   for (int i = 0; i < GetToolCount(); ++i)
   {
      ti.lpszText = nullptr;
      ::SendMessage(m_hWnd, TTM_ENUMTOOLS, i, (LPARAM) &ti);

      /*
         If tracking is NOT enabled, skip this (because that tool isn't using
         the hook).

         This means that if we want a tooltip (any kind of tooltip) to automatically
         track along with the mouse, all we have to do is set the TTF_TRACK flag.

         Or, we can decide to track all tooltips unilaterally by not doing this test.
         (Thus, EVERY tooltip managed by this class would track the cursor).

         Of course, this is ONLY true if there is some tracking tooltip created
         somewhere. If the hook is never set, this won't happen.
      */
      if (!(ti.uFlags & TTF_TRACK))
         continue;

      /*
         Is the mouse cursor over this tool's window (or area)?
      */
      if (HitTool(ti, hWnd, pt))
      {
         ActivateTooltip(ti, true);
         /*
            If we find a hit, we stop looking because we've already activated the
            active tool for this tooltip control and, therefore, deactivated the
            previously active tool.
         */
         return true;
      }
   }

   /*
      If no tools in this tooltip control get hit AND the tooltip control
      has an active tool, we deactivate it (because this tool/control is
      no longer hit).
   */
   TOOLINFO tiCur;
   tiCur.cbSize = sizeof tiCur;
   tiCur.lpszText = nullptr;
   if (::SendMessage(m_hWnd, TTM_GETCURRENTTOOL, 0, (LPARAM) &tiCur))
      ActivateTooltip(tiCur, false);

   return false;
}

bool MyToolTipCtrl::HitTool(const TOOLINFO& ti, const HWND hWnd, const POINT& pt)
{
   bool bHit = false;

   // if the tool covers the whole window...
   if (ti.uFlags & TTF_IDISHWND)
   {
#if defined(_DEBUG_TOOLTIPS)
      skst::MyLog::GetLog().LogV("\tWindow: %#x\n", ti.uId);
#endif
      // ...and it's the same wndw the mouse msg is destined for, it's a hit
      if ((HWND) ti.uId == hWnd)
         bHit = true;
   }
   else
   {
#if defined(_DEBUG_TOOLTIPS)
      skst::MyLog::GetLog().LogV("\tRect: window = %#x, id: %#x\n", ti.hwnd, ti.uId);
#endif

      /*
         Unfortunately, Microsoft's doc on TTM_HITTEST is quite opaque.
         We have no idea what coordinate system the point should be in.
         We have no idea whether it should be a tool window handle or a
         window handle in the structure, nor, if it should be a window
         handle, which one it should be.

         The [typically] opaque doc says:

            "Handle to the tool or window with the specified tool."

            So, which is it? A tool handle or a window handle? And
            if a window handle, how exactly does one "specify" the tool?

            "Client coordinates of the point to test."

            Client coordinates of what window? The tool's, the window's
            (which one?), which window, the screen?

         So, as is becoming a theme with tooltips, I take matters into
         my own hands and do my own hit test, which works.

         Thanks, Microsoft.
      */
      /*
      TTHITTESTINFO hti;
      hti.hwnd = ti.hwnd;
      hti.pt = pt;
      ::ScreenToClient(hti.hwnd, &hti.pt);
      hti.ti.cbSize = sizeof hti.ti;
      hti.ti.lpszText = NULL;
      const LRESULT rc = ::SendMessage(m_hWnd, TTM_HITTEST, 0, (LPARAM) &hti);
      if (rc)
      {
         b = true;
         break;
      }
      */
      // is this point over the tool's window and within the tool's rect?
      POINT ptClient(pt);
      ::ScreenToClient(ti.hwnd, &ptClient);
      const HWND hUnder = ::WindowFromPoint(pt);
      if ((ti.hwnd == hUnder) && ::PtInRect(&ti.rect, ptClient))
         bHit = true;
   }

   return bHit;
}

void MyToolTipCtrl::ActivateTooltip(const TOOLINFO& ti, bool bActivate)
{
   /*
      If the active window doesn't belong to this thread, disable the tooltip.

      If we don't do this and the user ALT+TABs away from the app while a
      tooltip is active, the tooltip stays visible--on top of the
      newly-activated app.

      ACTUALLY, that still happens. What this does is to prevent tooltips
      from being activated if the app using this class isn't active.
      So, if another app is active and the user moves the mouse cursor
      over a tool, the tooltip won't be activated.

      Of course, the user can mouse over the "phantom" tooltip and it'll
      deactivate because this app is no longer active.
   */
   if (_bOnlyShowTipsIfActive && (::GetActiveWindow() == nullptr))
      bActivate = false;

   /*
      If one of the tools of this tooltip control is being activated, we check
      to see what the current tool is (if any). If there is one and it's
      different from the one that just got hit, we deactivate it.
   */
   if (bActivate)
   {
      /*------------------------------------------------------------------------
         a) If a tip is for an entire window, we need to deactivate the
            tip if the window changes because otherwise the "old" tool
            won't be hidden.

         b) If a tip has an id (because it uses a rect, rather than the
            entire wndw), we need to deactivate the tip if the rect id
            changes, so that the "new" id is passed to the notify function.
            If we don't, the "switch" from one tool to another tool (in the
            same window) isn't recognized and the new id isn't passed to
            the notify function. So it would look as if the same tool were
            active, even though a different tool is.

         This is easy to correct. We get the current tool and
         compare its id to the id of the tool which matched.
         If they're different, we've switched to a new tool so we
         deactivate the old one.
      */
      TOOLINFO tiCur;
      tiCur.cbSize = sizeof tiCur;
      tiCur.lpszText = nullptr;
      if (::SendMessage(m_hWnd, TTM_GETCURRENTTOOL, 0, (LPARAM) &tiCur))
      {
#if defined(_DEBUG_TOOLTIPS)
         skst::MyLog::GetLog().LogV("\t\tCurrent tool: cur id %#x, match id %#x (if != deactivate)\n", tiCur.uId, ti.uId);
#endif
         if (ti.uId != tiCur.uId)
            ::SendMessage(m_hWnd, TTM_TRACKACTIVATE, FALSE, (LPARAM) &ti);
      }

      /*------------------------------------------------------------------------
         If we activate a tooltip without positioning it, it's drawn at (0,0) on
         the screen. Ugly. So, we position it before we activate it.
      */
/*
	BUG:
	If the band is on a secondary monitor (not primary),
	the tooltip is displayed on the primary monitor.
*/
      POINT pt;
      ::GetCursorPos(&pt);

		/*
			If the tooltip is under the mouse cursor, it flickers.
			So, we position the tip above the mouse cursor unless
			that would move it off-screen. In that case, we leave
			it alone (which puts it below the mouse).
		*/
      // offset tip from cursor a little
//      static int cx = ::GetSystemMetrics(SM_CXCURSOR) / 2;
      static int cy = ::GetSystemMetrics(SM_CYCURSOR) / 2;
		++pt.x;
		if (pt.y >= cy + 1)
			pt.y -= cy + 1;
		else
			++pt.y;

      /*
         ensure tip is on-screen
      */
      RECT r;
      ::GetWindowRect(m_hWnd, &r);
		::OffsetRect(&r, pt.x - r.left, pt.y - r.top);  // move rect so bottom-left point is mouse cursor
		MyWin::MoveRectangleInsideScreen(r);
      pt.x = r.left;
      pt.y = r.top;

      ::SendMessage(m_hWnd, TTM_TRACKPOSITION, 0, MAKELPARAM(pt.x, pt.y));
   }

#if defined(_DEBUG_TOOLTIPS)
   skst::MyLog::GetLog().LogV("\t%sctivate tooltip: window = %#x, id = %#x\n", bActivate ? _T("A") : _T("Dea"), m_hWnd, ti.uId);
#endif

//if (bActivate == !!ti.lParam)
//{
//#if defined(_DEBUG_TOOLTIPS)
//   skst::MyLog::GetLog().LogV("\t\tTool's state doesn't need changing\n");
//#endif
//   return;
//}
   ::SendMessage(m_hWnd, TTM_TRACKACTIVATE, bActivate ? TRUE : FALSE, (LPARAM) &ti);
//const_cast<TOOLINFO&>(ti).lParam = bActivate;
//::SendMessage(m_hWnd, TTM_SETTOOLINFO, 0, (LPARAM) &ti);

   if (bActivate && (_idTimer == 0))
   {
      // We'd love to use TrackMouseEvent() but the WM_MOUSELEAVE msg doesn't get intercepted by the mouse hook
//      skst::MyLog::GetLog().LogV("SetTimer (%#x)\n", ti.hwnd);
      _idTimer = ::SetTimer(nullptr, 0, 200, TimerProcTrack);
      _hWndTrack = ti.hwnd;
   }
   else if (!bActivate && (_idTimer != 0))
   {
//      skst::MyLog::GetLog().LogV("KillTimer\n");
      ::KillTimer(nullptr, _idTimer);
      _idTimer = 0;
      _hWndTrack = nullptr;
   }
}
HWND MyToolTipCtrl::_hWndTrack = nullptr;

/////////////////////////////////////////////////////////////////////////////
// other style of relaying messages (doesn't support tracking)
#if 0
LRESULT CALLBACK MyToolTipCtrl::MouseProc(int nCode, WPARAM wParam, LPARAM lParam)
{
   if (nCode >= 0)
   {
	   if ((wParam >= WM_MOUSEFIRST) && (wParam <= WM_MOUSELAST))
	   {
         MOUSEHOOKSTRUCT *pHook = reinterpret_cast<MOUSEHOOKSTRUCT *>(lParam);

         /*
            Theoretically, this could be limited to only relay msgs that are sent
            to a window passed to Create() (or to AddTool*()).
            However, this is more generic--messages sent to any window are relayed.
            It goes back to the issue described in Create(), where a dialog's child
            control cannot be the parent.
            If we REALLY wanted to do that, we could save the parent wndw handle
            and use it instead of GetParent(). But so far, there doesn't seem to
            be a problem with relaying messages for EVERY child control.
         */
         skst::MyLog::GetLog().LogV("MyToolTipCtrl::MouseProc(%#x) (%d,%d) parent = %#x\n", pHook->hwnd, pHook->pt.x, pHook->pt.y, ::GetParent(pHook->hwnd));
         for (it = _instances.begin(); it != _instances.end(); ++it)
         {
            TOOLINFO ti;
            ti.cbSize = sizeof ti;
            bool b = false;
            for (int i = 0; i < GetToolCount(); ++i)
            {
               ti.lpszText = NULL;
               ::SendMessage((*it)->m_hWnd, TTM_ENUMTOOLS, i, (LPARAM) &ti);

               // if the msg is destined for this tool's window, relay the msg to the tooltip ctrl
               if (pHook->hwnd == ti.hwnd)
		         {
                  // build MSG struct
                  MSG msg;
                  msg.hwnd = pHook->hwnd;
                  msg.message = wParam;
                  msg.wParam = 0;
                  msg.lParam = pHook->dwExtraInfo;
                  msg.time = ::GetMessageTime();
                  msg.pt = pHook->pt;
                  ::SendMessage(_this->m_hWnd, TTM_RELAYEVENT, 0, (LPARAM) &msg);
                  break;
		         }
            }
         }
	   }
	}

   return ::CallNextHookEx(_hMsgHook, nCode, wParam, lParam);
}
#endif
//
/////////////////////////////////////////////////////////////////////////////


void MyToolTipCtrl::SetFont(CONST LOGFONT *lplf)
{
   const HFONT hfont = ::CreateFontIndirect(lplf);
   ::SendMessage(m_hWnd, WM_SETFONT, (WPARAM) hfont, 0L);
}

void MyToolTipCtrl::SetTipTextColor(COLORREF clr)
{
	::SendMessage(m_hWnd, TTM_SETTIPTEXTCOLOR, (WPARAM) clr, 0);
}

void MyToolTipCtrl::SetBkTipColor(COLORREF clr)
{
	::SendMessage(m_hWnd, TTM_SETTIPBKCOLOR, (WPARAM) clr, 0);
}

void MyToolTipCtrl::SetMaxTipWidth(const int iWidth)
{
	::SendMessage(m_hWnd, TTM_SETMAXTIPWIDTH, 0, (LPARAM) iWidth);
}
