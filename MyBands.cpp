//---------------------------------------------------------------------------
// (c) 2006-2020 12noon, Stefan K. S. Tucker
//---------------------------------------------------------------------------

/*
	KNOWN ISSUE

	Right-click on Command Console shortcut in Quick Launch bar gives
	this error when using the Debug build. Ignore seemed ok.
	It's the result of GetMenu() returning NULL and wincore.cpp
	using ENSURE() to assert that it's not NULL.

	---------------------------
	Microsoft Visual C++ Debug Library
	---------------------------
	Debug Assertion Failed!

	Program: C:\WINDOWS\Explorer.EXE
	File: f:\sp\vctools\vc7libs\ship\atlmfc\src\mfc\wincore.cpp
	Line: 1325

	For information on how your program can cause an assertion failure, see the Visual C++ documentation on asserts.

	(Press Retry to debug the application)
	---------------------------
	Abort   Retry   Ignore   
	---------------------------
*/

#include "StdAfx.h"
#include <afxdtctl.h>   // CMonthCalCtrl
#include <shlguid.h>    // CATID_DeskBand
#include <initguid.h>   // so my GUIDs are created

#include "COMtoys/ComToys.h"
#include "COMtoys/Debug.h"
#include "COMtoys/IniFile.h"
#include "COMtoys/COMtoysLink.h"
//#include "TraceWin40/TraceWin.h"

#include "Shared/Graphics_MyMFC.h"

#include "Shared/misc.h"

#include "MyBands.h"
#include "resource.h"

BEGIN_MESSAGE_MAP(CMyBandsDll, CBandObjDll)
END_MESSAGE_MAP()

CMyBandsDll theApp;

// {93406D99-22C6-4676-8FFA-04D1CB16F47C}
DEFINE_GUID(CLSID_MYDESKBAND,
            0x93406d99, 0x22c6, 0x4676, 0x8f, 0xfa, 0x4, 0xd1, 0xcb, 0x16, 0xf4, 0x7c);

//////////////////
// initialize: add each band class
//
BOOL CMyBandsDll::InitInstance()
{
   ComToys::bTRACE = FALSE;
   CBandObj::bTRACE = FALSE;

	TRACEFN(_T(__FUNCTION__) _T("\n"));

	SetRegistryKey(_T("12noon"));
   TRACE("App name: %s\n", ::AfxGetAppName());

   /*
      do we turn debugging on?
   */
   // (Can't add an empty string to the string table. Nice editor, Microsoft.)
   if (::AfxGetApp()->GetProfileInt(_T(""), CString(MAKEINTRESOURCE(IDS_INI_DEBUG)), 0))
   {
      ComToys::bTRACE = TRUE;
      CBandObj::bTRACE = TRUE;
   }

	VERIFY(AddBandClass(CLSID_MYDESKBAND, RUNTIME_CLASS(CMyDeskBand), CATID_DeskBand, IDR_DESKBAND));

   /*
      If we haven't shown the help dialog yet, do so
   */
   if (::AfxGetApp()->GetProfileInt(_T(""), CString(MAKEINTRESOURCE(IDS_INI_SHOW_HELP)), 1))
   {
      /*
         When InstallShield is used to install this band, the help dialog
         is displayed UNDER the InstallShield wizard. Sigh.
         So, we have to force the dialog to the foreground.
      */
		MyMFC::CForegroundDlg(IDD_BAND_ENABLE).DoModal();

      // indicate that we've shown the help dialog already
      ::AfxGetApp()->WriteProfileInt(_T(""), CString(MAKEINTRESOURCE(IDS_INI_SHOW_HELP)), 0);
   }

	/*
		COleDateTime::Format() uses _tcsftime() to set the date/time to the specified format.
		_tcsftime() uses the C locale, which, by default, is "C"--the CRT default, which is
		English (and incorrect). That means, we have to specify that the locale (for date/time)
		is the operating system's.

		http://www.codeproject.com/cpp/introtolocalization.asp?df=100&forumid=661&exp=0&select=1191931
		http://www.developermania.com/newsgroups/item/154761/Re_COleDateTimeFormat_and_language.aspx
	*/
	TRACE("Locale before: %s\n", ::setlocale(LC_TIME, NULL));
	::setlocale(LC_TIME, "");	// use operating system's locale for date/time
	TRACE("Locale after: %s\n", ::setlocale(LC_TIME, NULL));

	return CBandObjDll::InitInstance();
}


//////////////////////////////////////////////////////////////////////////////
// CMyDeskBand

const int cxDefaultClock = 50;   // minimum width of clock
const int cyDefaultClock = 22;   // minimum height of clock (height of default deskband)

const int iMarginButton = 4 * 2; // need +4 pixels on every side around the button's icon (black line, white line, DMZ, focus)
const int iMarginStatic = 2 * 2; // want +2 pixels on every side around a static control

// control ids for child windows
const int IDC_CLOCK = WM_USER + 1;
const int IDC_ABOUT = WM_USER + 2;
const int IDC_CALENDAR = WM_USER + 3;


IMPLEMENT_DYNCREATE(CMyDeskBand, CBandObj)
BEGIN_MESSAGE_MAP(CMyDeskBand, CBandObj)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_SETFOCUS()
	ON_WM_TIMER()
   ON_NOTIFY_EX(TTN_GETDISPINFO, 0, OnToolTipNotify)

	ON_COMMAND(ID_PRODUCTS, &CMyDeskBand::OnProducts)
	ON_COMMAND(ID_MYHELP, &CMyDeskBand::OnHelp)
	ON_COMMAND(ID_ABOUT, &CMyDeskBand::OnAbout)
	ON_STN_CLICKED(IDC_ABOUT, &CMyDeskBand::OnAbout)

   ON_STN_CLICKED(IDC_CLOCK, &CMyDeskBand::OnCalendar)
   ON_COMMAND(ID_CLOCK_OPTIONS, &CMyDeskBand::OnClockOptions)
//ON_STN_DBLCLK(IDC_CLOCK, &CMyDeskBand::OnClockOptions)

   ON_UPDATE_COMMAND_UI(ID_CAL_SHOW_TODAY, &CMyDeskBand::OnUpdateCalendarShowToday)
   ON_COMMAND(ID_CAL_SHOW_TODAY, &CMyDeskBand::OnCalendarShowToday)
   ON_UPDATE_COMMAND_UI(ID_CAL_SHOWWEEKNUMS, &CMyDeskBand::OnUpdateCalShowWeekNums)
   ON_COMMAND(ID_CAL_SHOWWEEKNUMS, &CMyDeskBand::OnCalShowWeekNums)
   ON_UPDATE_COMMAND_UI(ID_CAL_SAVELASTPOS, &CMyDeskBand::OnUpdateCalSaveLastPos)
   ON_COMMAND(ID_CAL_SAVELASTPOS, &CMyDeskBand::OnCalSaveLastPos)
   ON_UPDATE_COMMAND_UI(ID_CAL_SAVELASTSIZE, &CMyDeskBand::OnUpdateCalSaveLastSize)
   ON_COMMAND(ID_CAL_SAVELASTSIZE, &CMyDeskBand::OnCalSaveLastSize)
   ON_COMMAND(ID_CAL_SELECTFONT, &CMyDeskBand::OnCalSelectFont)
	ON_COMMAND(ID_CLOCK_ADJUSTDATETIME, &CMyDeskBand::OnAdjustDateTime)
	ON_WM_SETTINGCHANGE()
END_MESSAGE_MAP()

CMyDeskBand::CMyDeskBand()
   : CBandObj(CLSID_MYDESKBAND)
   , _tips(false /*only show if active*/)
   , _dlgConfiguration(this)
{
   TRACE(__FUNCTION__ "\n");

   /*
      enable month control class
   */
   INITCOMMONCONTROLSEX icc;
   icc.dwSize = sizeof icc;
   icc.dwICC = ICC_DATE_CLASSES;
   ::InitCommonControlsEx(&icc);

   _idTimerUpdateDisplay = 0;

   /*
      Load options
      (The configuration dlg class loads its own in the ctor.)
   */

   /*
      determine sizes of various band components
   */

#if defined(_LOGO)
//#if defined(_LOGO_ICON)
if (!_bThemesSupported)
{
   _sizeAbout.cx = ::GetSystemMetrics(SM_CXSMICON);
   _sizeAbout.cy = ::GetSystemMetrics(SM_CYSMICON);
}
//#else
else
{
   HBITMAP hBmp = (HBITMAP) ::LoadImage(::AfxGetInstanceHandle(), MAKEINTRESOURCE(IDB_ABOUT),
                                       IMAGE_BITMAP, 0, 0, LR_DEFAULTSIZE | LR_SHARED);
   BITMAP bm;
   ::GetObject(hBmp, sizeof bm, &bm);
   _sizeAbout.cx = bm.bmWidth;
   _sizeAbout.cy = bm.bmHeight;
//#endif
}
#endif

   /*
      load the previous band size from the Registry
   */
   VERIFY(_fontClock.CreateFontIndirect(&_dlgConfiguration.GetFont()));
//TEST:Is the band's width STILL set correctly on startup?   UpdateClockText();
//For now, we're going to auto-size the control.
//   _sizeClockText.cx = ::AfxGetApp()->GetProfileInt(CString(MAKEINTRESOURCE(IDS_INI_SECTION_CLOCK)), CString(MAKEINTRESOURCE(IDS_INI_IDEAL_CX)), _sizeClockText.cx);
//   _sizeClockText.cy = ::AfxGetApp()->GetProfileInt(CString(MAKEINTRESOURCE(IDS_INI_SECTION_CLOCK)), CString(MAKEINTRESOURCE(IDS_INI_IDEAL_CY)), _sizeClockText.cy);

//#if defined(_DEBUG)
//   /*
//      How can we calculate the client area of the taskbar?
//      This doesn't do it.
//         [1568] SM_CXSCREEN x SM_CYSCREEN = 1280x1024
//         [1568] SPI_GETWORKAREA = 1280x996
//         [1568] SM_CXEDGE x SM_CYEDGE = 2x2
//         [1568] SPI_GETBORDER = 1
//         [1568] border width = 1
//         [1568]  calc = 26
//   */
//   TRACE("SM_CXSCREEN x SM_CYSCREEN = %dx%d\n", ::GetSystemMetrics(SM_CXSCREEN), ::GetSystemMetrics(SM_CYSCREEN));
//   CRect rr;
//   ::SystemParametersInfo(SPI_GETWORKAREA, 0, rr, 0);
//   TRACE("SPI_GETWORKAREA = %dx%d\n", rr.Width(), rr.Height());
//   TRACE("SM_CXEDGE x SM_CYEDGE = %dx%d\n", ::GetSystemMetrics(SM_CXEDGE), ::GetSystemMetrics(SM_CYEDGE));
//   int border;
//   ::SystemParametersInfo(SPI_GETBORDER, 0, &border, 0);
//   TRACE("SPI_GETBORDER = %d\n", border);
//   NONCLIENTMETRICS ncm;
//   ncm.cbSize = sizeof ncm;
//   ::SystemParametersInfo(SPI_GETNONCLIENTMETRICS, 0, &ncm, 0);
//   TRACE("border width = %d\n", ncm.iBorderWidth);
//   TRACE("\tcalc = %d\n", ::GetSystemMetrics(SM_CYSCREEN) - rr.Height() - ::GetSystemMetrics(SM_CYEDGE));
//#endif

//TEST:Is the band's width STILL set correctly on startup?   SetBandSizes();
}


void CMyDeskBand::SetBandSizes()
{
   TRACE(__FUNCTION__ "\n");

   /*
      init min and actual sizes
         minimum is space for the button and same for the graph
         ideally space for one button and 3x that for the graph

      Unfortunately, the minimim size is what's actually used for
      the bar's initial size. Duh.
   */
   /*
      init horizontal deskband info
   */
   m_dbiDefault[DBIF_VIEWMODE_NORMAL].ptMinSize = CPointL(cxDefaultClock, cyDefaultClock);
   m_dbiDefault[DBIF_VIEWMODE_NORMAL].ptActual  = CPointL(_sizeClockText.cx, _sizeClockText.cy);
#if defined(_LOGO)
   m_dbiDefault[DBIF_VIEWMODE_NORMAL].ptMinSize.x += _sizeAbout.cx + iMarginStatic;
   m_dbiDefault[DBIF_VIEWMODE_NORMAL].ptActual.x  += _sizeAbout.cx + iMarginStatic;
#endif

   /*
      init vertical deskband info
   */
   m_dbiDefault[DBIF_VIEWMODE_VERTICAL].ptMinSize = CPointL(cxDefaultClock, cyDefaultClock);
   m_dbiDefault[DBIF_VIEWMODE_VERTICAL].ptActual  = CPointL(_sizeClockText.cx, _sizeClockText.cy);
#if defined(_LOGO)
   m_dbiDefault[DBIF_VIEWMODE_VERTICAL].ptMinSize.y += _sizeAbout.cy + iMarginStatic;
   m_dbiDefault[DBIF_VIEWMODE_VERTICAL].ptActual.y  += _sizeAbout.cy + iMarginStatic;
#endif

   /*
      init floating deskband info
         Note that Windows tries to automatically swap x and y when the band is taller than it is wide.
         However, Windows fails miserably at this, so we have to choose minimums that work best in either direction.
   */
   m_dbiDefault[DBIF_VIEWMODE_FLOATING].ptMinSize = CPointL(cxDefaultClock, cyDefaultClock);
   m_dbiDefault[DBIF_VIEWMODE_FLOATING].ptActual  = CPointL(_sizeClockText.cx, _sizeClockText.cy);
#if defined(_LOGO)
   m_dbiDefault[DBIF_VIEWMODE_FLOATING].ptMinSize.x += _sizeAbout.cx + iMarginStatic;
   m_dbiDefault[DBIF_VIEWMODE_FLOATING].ptActual.x  += _sizeAbout.cx + iMarginStatic;
#endif

   // init the rest of the structure(s)
   m_dbiDefault[DBIF_VIEWMODE_NORMAL].dwModeFlags =
      m_dbiDefault[DBIF_VIEWMODE_VERTICAL].dwModeFlags =
      m_dbiDefault[DBIF_VIEWMODE_FLOATING].dwModeFlags = DBIMF_NORMAL | DBIMF_VARIABLEHEIGHT;

   m_dbiDefault[DBIF_VIEWMODE_NORMAL].ptIntegral =
      m_dbiDefault[DBIF_VIEWMODE_VERTICAL].ptIntegral =
      m_dbiDefault[DBIF_VIEWMODE_FLOATING].ptIntegral = CPointL(0, 1); // x is ignored; y is vertical sizing step value

//force it to be wide enough (TODO: clean up)
m_dbiDefault[DBIF_VIEWMODE_NORMAL].ptMinSize = m_dbiDefault[DBIF_VIEWMODE_NORMAL].ptActual; 
m_dbiDefault[DBIF_VIEWMODE_VERTICAL].ptMinSize = m_dbiDefault[DBIF_VIEWMODE_VERTICAL].ptActual; 
m_dbiDefault[DBIF_VIEWMODE_FLOATING].ptMinSize = m_dbiDefault[DBIF_VIEWMODE_FLOATING].ptActual; 

   TRACE(_T("\tH: Min size = %dx%d\n"),    m_dbiDefault[DBIF_VIEWMODE_NORMAL].ptMinSize.x,   m_dbiDefault[DBIF_VIEWMODE_NORMAL].ptMinSize.y);
   TRACE(_T("\tH: Actual size = %dx%d\n"), m_dbiDefault[DBIF_VIEWMODE_NORMAL].ptActual.x,    m_dbiDefault[DBIF_VIEWMODE_NORMAL].ptActual.y);
//   TRACE(_T("\tV: Min size = %dx%d\n"),    m_dbiDefault[DBIF_VIEWMODE_VERTICAL].ptMinSize.x, m_dbiDefault[DBIF_VIEWMODE_VERTICAL].ptMinSize.y);
//   TRACE(_T("\tV: Actual size = %dx%d\n"), m_dbiDefault[DBIF_VIEWMODE_VERTICAL].ptActual.x,  m_dbiDefault[DBIF_VIEWMODE_VERTICAL].ptActual.y);
//   TRACE(_T("\tF: Min size = %dx%d\n"),    m_dbiDefault[DBIF_VIEWMODE_FLOATING].ptMinSize.x, m_dbiDefault[DBIF_VIEWMODE_FLOATING].ptMinSize.y);
//   TRACE(_T("\tF: Actual size = %dx%d\n"), m_dbiDefault[DBIF_VIEWMODE_FLOATING].ptActual.x,  m_dbiDefault[DBIF_VIEWMODE_FLOATING].ptActual.y);
}


//////////////////
// remove border from window (see CBandObj::OnCreateWindow())
BOOL CMyDeskBand::PreCreateWindow(CREATESTRUCT& cs)
{
   // remove border
   cs.style &= ~WS_BORDER;
   return CBandObj::PreCreateWindow(cs);
}

// I'm being created: create controls
int CMyDeskBand::OnCreate(CREATESTRUCT *pcs)
{
   TRACE(__FUNCTION__ "\n");
	if (CBandObj::OnCreate(pcs)==-1)
		return -1;

   BuildContextMenu();

   /*
      create tooltip control to manage tooltips for child windows
   */
   _tips.Create();

   // size the controls for real later, in OnSize()

   /*
      About button
   */
#if defined(_LOGO)
//#if defined(_LOGO_ICON)
if (!_bThemesSupported)
{
   _ctlAbout.Create(NULL, WS_CHILD | WS_VISIBLE | SS_NOTIFY | SS_CENTERIMAGE | SS_ICON,
                     CRect(0,0,0,0), this, IDC_ABOUT);
   _ctlAbout.SetIcon((HICON) ::LoadImage(::AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_ABOUT), IMAGE_ICON,
                                          ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON),
                                          LR_SHARED));
}
//#else
else
{
   _ctlAbout.Create(NULL, WS_CHILD | WS_VISIBLE | SS_NOTIFY | SS_REALSIZEIMAGE | SS_CENTERIMAGE | SS_BITMAP,
                     CRect(0,0,0,0), this, IDC_ABOUT);
   /*
      From the VS.2005 doc:
      "Retrieves the color value of the first pixel in the image and
      replaces the corresponding entry in the color table with the default
      window color (COLOR_WINDOW). All pixels in the image that use that
      entry become the default window color. This value applies only to
      images that have corresponding color tables."

      So we can't use LR_LOADTRANSPARENT because it'll look bad on XP.

      And we don't use LR_LOADMAP3DCOLORS because it looks bad on XP.
   */
   _ctlAbout.SetBitmap((HBITMAP) ::LoadImage(::AfxGetInstanceHandle(), MAKEINTRESOURCE(IDB_ABOUT),
                                             IMAGE_BITMAP, 0, 0, LR_SHARED));
//#endif
}
   _tips.AddToolTextStaticTrack(_ctlAbout, _T("About Timekeeper"));
#endif

   /*
      clock display
   */
   // SS_LEFTNOWORDWRAP: it asserts
   // SS_SIMPLE: bg is white and no ellipsis
   // SS_CENTER: centers horizontally, but not vertically; also wraps
   if (!_ctlClock.Create(_T(""), WS_CHILD | WS_VISIBLE | SS_NOTIFY | SS_NOPREFIX | SS_CENTERIMAGE | SS_ENDELLIPSIS, CRect(0,0,0,0), this, IDC_CLOCK))
   {
      TRACE("Create() failed\n");
      return -1;
   }
   UpdateClockText();   // kind of redundant (because the ctor does this); we do it to "prime" the control's text
   FormatClock();

//TODO: if seconds aren't displaying, should we set a 1-minute timer (and then set it again--to keep it accurate, like Alarm++)
   _idTimerUpdateDisplay = SetTimer(rand(), 1000, NULL);    // refresh every second

	// set up dynamic tooltip
   _tips.AddToolTextDynamicTrack(_ctlClock, GetSafeHwnd());

// We need to set these here (in addition to or instead of CMyDeskBand() so that it's not too wide)
SetBandSizes();
NotifyBandInfoChanged();

	return 0;
}


void CMyDeskBand::OnDestroy()
{
	TRACE(_T(__FUNCTION__) _T("\n"));

	if (_idTimerUpdateDisplay != 0)
	{
		KillTimer(_idTimerUpdateDisplay);
		_idTimerUpdateDisplay = 0;
	}

   /*
      If the band is closed, the calendar stays up. seems we have to do this.
      Why isn't ~CMyDeskBand (which would call DestroyWindow() called??
   */
   _cal.DestroyWindow();
}


//////////////////
// Window was resized: adjust controls
//
/*
   Horizontal
   +---------------+-------------------------------+
   |               |                               |
   |               |                               |
   |   About btn   |             clock             |
   |               |                               |
   |               |                               |
   +---------------+-------------------------------+

       bmp width              remainder


   Vertical
   +-----------+
   |           |
   |           |
   | About btn |  bmp height
   |           |
   |           |
   +-----------+
   |           |
   |           |
   |           |
   |           |
   |   clock   |  remainder
   |           |
   |           |
   |           |
   |           |
   +-----------+
*/
void CMyDeskBand::DoSize(UINT /*nType*/, int cx, int cy)
{
   UNUSED(cx);
   UNUSED(cy);
	TRACE(_T(__FUNCTION__) _T("(%d,%d)\n"), cx, cy);

   CRect rClock;
   GetClientRect(rClock);
   ASSERT((rClock.Width() == cx) && (rClock.Height() == cy));

#if defined(_LOGO)
   // enlarge rect by margin constant (to fit icon and edges)
   CRect rAbout;

   /*
      horizontal
   */
   if (m_dwViewMode != DBIF_VIEWMODE_VERTICAL)
   {
      // About btn
      rAbout.left = rClock.left + (iMarginStatic / 2);
      rAbout.right = rAbout.left + _sizeAbout.cx;
      rAbout.top = rClock.CenterPoint().y - (_sizeAbout.cy / 2);
      rAbout.bottom = rAbout.top + _sizeAbout.cy;
//      TRACE(_T("\tH: rClock.Height = %d, rClock.top = %d, rAbout.top = %d\n"), rClock.Height(), rClock.top, rAbout.top);

      rClock.left = rAbout.right + (iMarginStatic / 2);
   }
   /*
      vertical
   */
   else
   {
      // About btn
      rAbout.top = rClock.top + (iMarginStatic / 2);
      rAbout.bottom = rAbout.top + _sizeAbout.cy;
      rAbout.left = rClock.CenterPoint().x - (_sizeAbout.cx / 2);
      rAbout.right = rAbout.left + _sizeAbout.cx;
//      TRACE(_T("\tV: rClock.Width = %d, rClock.left = %d, rAbout.left = %d\n"), rClock.Width(), rClock.left, rAbout.left);

      rClock.top = rAbout.bottom + (iMarginStatic / 2);
   }

//   TRACE("\tAbout: (%d,%d) %dx%d\n", rAbout.left, rAbout.top, rAbout.Width(), rAbout.Height());
   _ctlAbout.SetWindowPos(NULL, rAbout.left, rAbout.top, rAbout.Width(), rAbout.Height(), SWP_NOZORDER);
#endif

//   TRACE("\tClock: (%d,%d) %dx%d\n", rClock.left, rClock.top, rClock.Width(), rClock.Height());
   _ctlClock.SetWindowPos(NULL, rClock.left, rClock.top, rClock.Width(), rClock.Height(), SWP_NOZORDER);
}


//////////////////
// I got focus: pass to child control
//
void CMyDeskBand::OnSetFocus(CWnd* pOldWnd)
{
	__super::OnSetFocus(pOldWnd);

   // if we don't do this, the button can't be clicked when the deskband is "floating"
#if defined(_LOGO)
   _ctlAbout.SetFocus();
#endif
}

//////////////////
// route commands through ??? control
//
BOOL CMyDeskBand::OnCmdMsg(UINT a, int b, void* c, AFX_CMDHANDLERINFO* d)
{
#if defined(_LOGO)
   if (_ctlAbout.OnCmdMsg(a, b, c, d))
      return TRUE;
#endif
   if (_ctlClock.OnCmdMsg(a, b, c, d))
      return TRUE;

	return CBandObj::OnCmdMsg(a, b, c, d);
}


/////////////////////////////////////////////////////////////////////////////
// preserve state information when changing docking states

void CMyDeskBand::IPersistStreamGetSizeMax(ULARGE_INTEGER *pcbSize)
{
   pcbSize->LowPart = sizeof DWORD;
   pcbSize->HighPart = 0;
}

/*
   have to set m_bModified whenever any saved data changes
*/
HRESULT CMyDeskBand::IPersistStreamSave(IStream *pStream)
{
//Note: This is how to transfer data between invocations of the band (e.g., LOGFONT and all ConfigureDlg settings)
   // sample data
   const DWORD dw = 47;

   DWORD dwActual;
   const HRESULT hr = pStream->Write(&dw, sizeof dw, &dwActual);
   if (FAILED(hr) || (dwActual != sizeof dw))
      return STG_E_CANTSAVE;

//   TRACE(__FUNCTION__ ": Sample data = %#lx\n", dw);

   return S_OK;
}

HRESULT CMyDeskBand::IPersistStreamLoad(IStream *pStream)
{
//Note: This is how to transfer data between invocations of the band (e.g., LOGFONT and all ConfigureDlg settings)
   DWORD dw;
   DWORD dwActual;
   const HRESULT hr = pStream->Read(&dw, sizeof dw, &dwActual);
   if (FAILED(hr) || (dwActual != sizeof dw))
      return E_FAIL;

   ASSERT(dw == 47);
//   TRACE(__FUNCTION__ ": Sample data = %#lx\n", dw);

	return S_OK;
}


/////////////////////////////////////////////////////////////////////////////
// custom handlers

void CMyDeskBand::OnProducts()
{
   PMS::OpenURL(CString(MAKEINTRESOURCE(PMS_IDS_WEB_PMS)));
}

void CMyDeskBand::OnAdjustDateTime()
{
	/*
		These methods all work on XP SP2 Home:
			timedate.cpl
			control.exe timedate.cpl
			control.exe date/time
	*/
   ::ShellExecute(GetSafeHwnd(), _T("open"), _T("control.exe"), _T("date/time"), _T(""), SW_SHOWNORMAL);
}

void CMyDeskBand::OnAbout()
{
   PMS::PMSAbout(::AfxGetInstanceHandle(), NULL, MAKEINTRESOURCE(IDR_MAINFRAME), _T(" "), FALSE);
}


void CMyDeskBand::OnHelp() 
{
   const CString strPath(skst::GetLocalPath(::AfxGetResourceHandle(), CString(MAKEINTRESOURCE(IDS_HELP_FILE))).c_str());
	TRACE(_T("Help file: {%s}\n"), (LPCTSTR) strPath);
   ::ShellExecute(GetSafeHwnd(), _T("open"), strPath, _T(""), _T(""), SW_SHOWNORMAL);
}


void CMyDeskBand::BuildContextMenu()
{
   /*
      restore original context menu
   */
   // from CBandObj::OnCreate()
/*
	CMenu menu;
	if (menu.LoadMenu(GetResourceID()))
   {
      CMenu *pSubMenu = menu.GetSubMenu(0);
		if (pSubMenu != NULL)
      {
         m_menu.DestroyMenu();

			m_menu.Attach(pSubMenu->Detach());
         menu.Detach();
		}
	}
*/
   // reload menu bar to get fresh "clean" submenu
   m_menu.DestroyMenu();      // destroy submenu first
   m_menuBar.DestroyMenu();   // THEN destroy menu bar
   if (!m_menuBar.LoadMenu(GetResourceID()))
   {
      TRACE(__FUNCTION__ ": error in LoadMenu(%u)\n", GetResourceID());
      return;
   }

   // it seems that using m_menuBar.GetSubMenu(0)->m_hMenu causes a memory leak
   if (!m_menu.Attach(::GetSubMenu(m_menuBar, 0)))
      TRACE(__FUNCTION__ ": unable to attach submenu.\n");
#if defined(_DEBUG)
   if (m_menu.m_hMenu != NULL)
   {
      if (!::IsMenu(m_menu.m_hMenu))
         TRACE(__FUNCTION__ ": m_menu %#x is INVALID.\n", m_menu.m_hMenu);
   }
#endif
}


/*
   Called if some system parameters are changed.
   For example, date/time format, font, colors, etc.

   The help for WM_SETTINGCHANGE says:
      In general, when you receive this message, you should check and reload
      any system parameter settings that are used by your application.
*/
void CMyDeskBand::OnSettingChange(UINT uFlags, LPCTSTR lpszSection)
{
	__super::OnSettingChange(uFlags, lpszSection);

// BUG: It seems that we need to force a reload of date/time format in Regional Settings. How?
	OnTimer(_idTimerUpdateDisplay);
}


void CMyDeskBand::OnTimer(UINT_PTR idEvent)
{
	if (idEvent == _idTimerUpdateDisplay)
	{
		UpdateClockText();

		/*
			if the calendar is displayed, set the caption to the locale's long date/time
		*/
		if (_cal.GetSafeHwnd() != NULL)
			_cal.SetWindowText(COleDateTime::GetCurrentTime().Format(_T("%#c")));

		/*
			Compute the size of the text. If it's larger than the available space, resize the band.
		*/
		CRect rClock;
		_ctlClock.GetClientRect(&rClock);
//TRACE("Compare: need(%d) to have(%d)\n", _sizeClockText.cx, rClock.Width());
	   if ((_sizeClockText.cx > rClock.Width()) || (_sizeClockText.cy > rClock.Height()))
		{
			TRACE("New larger size: %dx%d\n", _sizeClockText.cx, _sizeClockText.cy);
//For now, we're going to auto-size the control.
//      ::AfxGetApp()->WriteProfileInt(CString(MAKEINTRESOURCE(IDS_INI_SECTION_CLOCK)), CString(MAKEINTRESOURCE(IDS_INI_IDEAL_CX)), _sizeClockText.cx);
//      ::AfxGetApp()->WriteProfileInt(CString(MAKEINTRESOURCE(IDS_INI_SECTION_CLOCK)), CString(MAKEINTRESOURCE(IDS_INI_IDEAL_CY)), _sizeClockText.cy);

	      SetBandSizes();
		   NotifyBandInfoChanged();
		}
   }
}

void CMyDeskBand::UpdateClockText()
{
   /*
      format and set the clock's new text
   */
   const CString s(_dlgConfiguration.UpdateControlText(_ctlClock));

   /*
      calculate the new text's size
   */
   _sizeClockText = MyMFC::CalculateTextSize(s, _fontClock);
   // add a 1-pixel margin all around
   _sizeClockText.cx += 2;
   _sizeClockText.cy += 2;
}


BOOL CMyDeskBand::OnToolTipNotify(UINT id, NMHDR *pNMHDR, LRESULT *pResult)
{
   UNUSED(id);
   ASSERT(id == 0);
   NMTTDISPINFO *pttDispInfo = reinterpret_cast<NMTTDISPINFO *>(pNMHDR);
   *pResult = 0;

   // pttDispInfo->hdr.hwndFrom is the associated tool wndw

   /*
      tooltip is long date/time format
   */
   const CString str(COleDateTime::GetCurrentTime().Format(_T("%#c")));
   ::_tcscpy_s(pttDispInfo->szText, _countof(pttDispInfo->szText), str);

   return TRUE;
}


void CMyDeskBand::OnClockOptions()
{
	if (_dlgConfiguration.DoModal() != IDOK)
      return;

   // ensure the font's set
   FormatClock();
   // THEN get the new text size
   UpdateClockText();
	TRACE("New text size: %dx%d\n", _sizeClockText.cx, _sizeClockText.cy);

//TODO: if the timer is longer than 1-second, we probably want to call it explicitly here. or make the format/set combo a function and call THAT.

   // THEN force the band to resize
   SetBandSizes();
   NotifyBandInfoChanged();
}

void CMyDeskBand::FormatClock()
{
   /*
      set the clock's style
   */
   _ctlClock.ModifyStyle(SS_LEFT | SS_CENTER | SS_RIGHT, _dlgConfiguration.GetAlignmentStyle());

   /*
      replace current clock font with the new one
   */
   /*
      Can't remember the CFont* because that's a direct ptr
      to the handle map where the control's CFont is stored.
      So when we later use _fontClock.CreateFontIndirect(),
      it changes the HFONT the control's GetFont() returns.
      Short story: We save the HFONT for fool-proof future
      deleting.
   */
   HGDIOBJ hFontOld = _fontClock.Detach();
   ASSERT(hFontOld != NULL);

   VERIFY(_fontClock.CreateFontIndirect(&_dlgConfiguration.GetFont()));

   _ctlClock.SetFont(&_fontClock);

   ::DeleteObject(hFontOld);

   /*
      set the clock's colors
   */
   if (_dlgConfiguration.IsClockColorTextDefault())
      _ctlClock.SetTextColorDefault();
   else
      _ctlClock.SetTextColor(_dlgConfiguration.GetClockColorText());

   if (_dlgConfiguration.IsClockColorBgTransparent())
      _ctlClock.SetBgColorTransparent();
   else if (_dlgConfiguration.IsClockColorBgDefault())
      _ctlClock.SetBgColorDefault();
   else
      _ctlClock.SetBgColor(_dlgConfiguration.GetClockColorBG());
}


void CMyDeskBand::OnUpdateCalendarShowToday(CCmdUI *pCmdUI)
{
   pCmdUI->SetCheck(_cal.ShowToday());
}

void CMyDeskBand::OnCalendarShowToday()
{
   if (_cal.ShowToday())
      _cal.TodayHide();
   else
      _cal.TodayShow();
}


void CMyDeskBand::OnUpdateCalShowWeekNums(CCmdUI *pCmdUI)
{
   pCmdUI->SetCheck(_cal.ShowWeekNumbers());
}

void CMyDeskBand::OnCalShowWeekNums()
{
   if (_cal.ShowWeekNumbers())
      _cal.WeekNumbersHide();
   else
      _cal.WeekNumbersShow();
}


void CMyDeskBand::OnUpdateCalSaveLastPos(CCmdUI *pCmdUI)
{
   pCmdUI->SetCheck(_cal.SavePosition());
}

void CMyDeskBand::OnCalSaveLastPos()
{
   if (_cal.SavePosition())
      _cal.SavePositionDisable();
   else
      _cal.SavePositionEnable();
}


void CMyDeskBand::OnUpdateCalSaveLastSize(CCmdUI *pCmdUI)
{
   pCmdUI->SetCheck(_cal.SaveSize());
}

void CMyDeskBand::OnCalSaveLastSize()
{
   if (_cal.SaveSize())
      _cal.SaveSizeDisable();
   else
      _cal.SaveSizeEnable();
}


void CMyDeskBand::OnCalendar()
{
   if (_cal.GetSafeHwnd() != NULL)
   {
      _cal.SetForegroundWindow();
      return;
   }

   if (!_cal.Create())
      ::AfxMessageBox(_T("Unable to display calendar."));
}

void CMyDeskBand::OnCalSelectFont()
{
   _cal.ChooseFont();
}
