////////////////////////////////////////////////////////////////
// Copyright 1998 Paul DiLascia
// If this program works, it was written by Paul DiLascia.
// If not, I don't know who wrote it.
// ---
// CBandObj-- generic Windows band object (IE Info/comm band and desktop band).
// Provides default implementation for IDeskBand. You must also include the
// band object registration script, BandObj.rgs, in your RC file.
//
//
#include "StdAfx.h"
#include <uxtheme.h>
#pragma comment(lib, "uxtheme.lib")
#include <atliface.h>                           //skst
#include "COMtoys/BandObj.h"                    //skst

#include "Shared/GetProcAddress.h"              //skst

#define BANDOBJCLASS _T("DeskBandClass")        //skst

// To turn on debugging, set this TRUE
BOOL CBandObj::bTRACE = FALSE;

#ifdef _DEBUG
#define BOTRACEFN				\
	CTraceFn __fooble;		\
	if (CBandObj::bTRACE)	\
		TRACE						
#define BOTRACE				\
	if (CBandObj::bTRACE)	\
		TRACE
#else
#define BOTRACEFN
#define BOTRACE
#endif

DEBUG_BEGIN_INTERFACE_NAMES()
	DEBUG_INTERFACE_NAME(IDeskBand)
	DEBUG_INTERFACE_NAME(IDeskBand2)
DEBUG_END_INTERFACE_NAMES();

IMPLEMENT_DYNAMIC(CBandObjDll, CTMfcModule);

CBandObjDll::CBandObjDll()
{
	CBandObj::bTRACE = TRUE; // TRACEing on for bandobj
	ComToys::bTRACE = TRUE;  // TRACEing on for COM toys
}

CBandObjDll::~CBandObjDll()
{
}

//////////////////
// Call from your InitInstance to add a new band object class.
//
BOOL CBandObjDll::AddBandClass(REFCLSID clsid, CRuntimeClass* pClass,
	const CATID& catid, UINT nResID)
{
	BOTRACEFN(_T("CBandObjDll::AddBandClass\n"));
	return OnCreateFactory(clsid, pClass, catid, nResID)!=NULL;
}

//////////////////
// Override if you ever want to create a different kind of factory (rare)
//
CBandObjFactory* CBandObjDll::OnCreateFactory(REFCLSID clsid,
	CRuntimeClass* pClass, const CATID& catid, UINT nResID)
{
	return new CBandObjFactory(clsid, pClass, catid, nResID);
}

////////////////////////////////////////////////////////////////
// CBandObjFactory
//
IMPLEMENT_DYNAMIC(CBandObjFactory, COleObjectFactory)

CBandObjFactory::CBandObjFactory(REFCLSID clsid, CRuntimeClass* pClass,
	const CATID& catid, UINT nIDRes)
	:  CTFactory(clsid, nIDRes),
		COleObjectFactory(clsid, pClass, FALSE, NULL)
{
	BOTRACEFN(_T("CBandObjFactory::CBandObjFactory\n"));
	ASSERT(pClass && pClass->IsDerivedFrom(RUNTIME_CLASS(CBandObj)));
	m_catid = catid;
}

CBandObjFactory::~CBandObjFactory()
{
	BOTRACEFN(_T("CBandObjFactory::~CBandObjFactory\n"));
}

////////////////
// Band objects are not insertable, so bypass MFC's standard
// registration. There's no ProgID
//
BOOL CBandObjFactory::OnRegister(BOOL bRegister)
{
	BOTRACEFN(_T("CBandObjFactory(%p)::OnRegister(%d)\n"), this, bRegister);

	if (!CTFactory::OnRegister(bRegister))
		return FALSE;

	// Get ICatRegister
	CTCatRegister iCat;
	HRESULT hr = bRegister ?
		iCat->RegisterClassImplCategories(CTFactory::GetClassID(),1,&m_catid) :
		iCat->UnRegisterClassImplCategories(CTFactory::GetClassID(),1,&m_catid);
	return SUCCEEDED(hr) || !bRegister;	// if can't unregister, ignore
}

BOOL CBandObjFactory::UpdateRegistry(BOOL bRegister)
{
	return TRUE; // bypass MFC stuff, not using it
}

////////////////////////////////////////////////////////////////
// CBandObj

CBandObj::CBandObj(REFCLSID clsid) :
	CTOleWindow(m_hWnd),
	CTDockingWindow(m_hWnd),
	CTPersist(clsid),
   CTPersistStream(m_bModified),
	CTInputObject(m_hWnd, m_hAccel),
	CTMfcContextMenu(this, m_menu)
{
	BOTRACEFN(_T("CBandObj(%p)::CBandObj\n"),this);
	AfxOleLockApp(); // don't unload DLL while I'm alive

   m_dwBandID = m_dwViewMode = 0;

	// set up default DESKBANDINFO
   for (int i = 0; i < sizeof m_dbiDefault / sizeof DESKBANDINFO; ++i)  //skst: init all of them
   {
	   m_dbiDefault[i].ptMinSize = CPointL(-1,-1);
	   m_dbiDefault[i].ptMaxSize = CPointL(-1,-1);
	   m_dbiDefault[i].ptActual  = CPointL(0,0);
	   m_dbiDefault[i].ptIntegral= CPointL(1,1);
	   m_dbiDefault[i].dwModeFlags = DBIMF_NORMAL;
   }

	// stuff to initialize from factory
	CTFactory* pFact = CTFactory::GetFactory(clsid);
	m_nIDRes = pFact->GetResourceID();
	m_strTitle = pFact->GetTitle();
	m_bModified = FALSE;
	m_bCompositionEnabled = TRUE;
}

CBandObj::~CBandObj()
{
	BOTRACEFN(_T("CBandObj(%p)::~CBandObj\n"), this);
	AfxOleUnlockApp();	// OK by me to unload DLL
}

//////////////////
// called when ref count goes to zero: delete myself
//
void CBandObj::OnFinalRelease()
{
	BOTRACEFN(_T("CBandObj(%p)::OnFinalRelease\n"), this);
	CCmdTarget::OnFinalRelease(); // will delete this
}

/////////////////
// These macros implement all the interfaces using ComToys classes!
//
IMPLEMENT_IUnknownCT			(CBandObj);
IMPLEMENT_IOleWindow			(CBandObj, CTOleWindow);
IMPLEMENT_IDockingWindow	(CBandObj, CTDockingWindow);
IMPLEMENT_IPersist			(CBandObj, CTPersist);
IMPLEMENT_IContextMenu		(CBandObj, CTMfcContextMenu);
//skst: before implementing part of IPersistStream in this class: IMPLEMENT_IPersistStream	(CBandObj, CTPersistStream);
IMPLEMENT_IInputObject		(CBandObj, CTInputObject);


/*skst:

   Implement IPersistStream

   Windows calls IPersistStream::Load() when the band is first started,
   before it calls IDeskBand::GetBandInfo().
   It also calls Load() when switching docking state (between horizontal,
   vertical, and floating).

   It calls IPersistStream::Save() "on occasion".

   Unfortunately, the information in the stream is NOT saved between sessions.

   This makes it a good mechanism for transferring data between docking states
   because the band is RECREATED when switching states. Therefore, any changes
   the user makes to the defaults are not automatically carried over from one
   docking state to the other.

   Since any such data is undoubtedly specific to the implementation of each
   desk band, these functions call virtual versions of themselves in order
   to load/save such data. The derived class needs only to implement those
   virtual functions.
*/
IMPLEMENT_IPersistStream_IsDirty(CBandObj, CTPersistStream);

STDMETHODIMP CBandObj::GetSizeMax(ULARGE_INTEGER *pcbSize)
{
	CMDTARGENTRY;
	BOTRACEFN(_T("CBandObj(%p)::IPersistStream::GetSizeMax %p, pcbSize = %p\n"), this, pcbSize);

   if (pcbSize == NULL)
      return E_POINTER;

   IPersistStreamGetSizeMax(pcbSize);

	return S_OK;
}

STDMETHODIMP CBandObj::Save(IStream *pStream, BOOL bClearDirty)
{
	CMDTARGENTRY;
	BOTRACEFN(_T("CBandObj(%p)::IPersistStream::Save %p, bClearDirty = %x\n"), this, pStream, bClearDirty);

   if (pStream == NULL)
      return E_POINTER;

   const HRESULT hr = IPersistStreamSave(pStream);
   if (FAILED(hr))
      return hr;

	if (bClearDirty)
		m_bModified = FALSE;

	return S_OK;
}

STDMETHODIMP CBandObj::Load(IStream *pStream)
{
	CMDTARGENTRY;
	BOTRACEFN(_T("CBandObj(%p)::IPersistStream::Load %p\n"), this, pStream);

   if (pStream == NULL)
      return E_POINTER;

   return IPersistStreamLoad(pStream);
}

/*
*/
//////////////////////////////////////////////////////////////////////////////


//////////////////
// This CCmdTarget override is used to bypass MFC's interface
// maps in CCmdTarget::InternalQueryInterface.
//
LPUNKNOWN CBandObj::GetInterfaceHook(const void* piid)
{
	return GetInterface(*((IID*)piid));
}

//////////////////
// This CTComObj override tells CTComObj what interface to use.
//
LPUNKNOWN CBandObj::GetInterface(REFIID iid)
{
	if (iid==IID_IUnknown) 
		return (IDeskBand2*)this;

#define IF_INTERFACE(iid, iface)										\
	if (iid==IID_##iface)												\
		return (iface*)this;												\

	IF_INTERFACE(iid, IOleWindow);
	IF_INTERFACE(iid, IDockingWindow);
	IF_INTERFACE(iid, IObjectWithSite);
	IF_INTERFACE(iid, IInputObject);
	IF_INTERFACE(iid, IPersist);
	IF_INTERFACE(iid, IPersistStream);
	IF_INTERFACE(iid, IContextMenu);
	IF_INTERFACE(iid, IDeskBand);
	IF_INTERFACE(iid, IDeskBand2);

	return NULL;
}

// **************** IDeskBand ****************

/////////////////
// Windows wants band info: stuff with m_dbiDefault
// or resource information
//
STDMETHODIMP CBandObj::GetBandInfo(DWORD dwBandID, DWORD dwViewMode, DESKBANDINFO* pdbi)
{
	CMDTARGENTRY;
	BOTRACEFN(_T("CBandObj(%p)::IDeskBand::GetBandInfo id=%d mode=%0x\n"), this, dwBandID, dwViewMode);

   m_dwBandID = dwBandID;					 // save in case you're interested
   m_dwViewMode = dwViewMode;				 // ditto

	if (!pdbi) 
		return E_INVALIDARG;

	const DWORD mask = pdbi->dwMask;
/*skst: Windows TRIES to use the same dbi for horizontal, vertical, and (I guess)
        floating bands, but that's stupid because we don't necessarily want the
        minimum width of a vertical band to be the same as the minimum height
        of a horizontal band. So, we maintain THREE separate dbi structures.
	if (mask & DBIM_MINSIZE)
		pdbi->ptMinSize = m_dbiDefault.ptMinSize;

	if (mask & DBIM_MAXSIZE)
		pdbi->ptMaxSize = m_dbiDefault.ptMaxSize;

	if (mask & DBIM_INTEGRAL)
		pdbi->ptIntegral = m_dbiDefault.ptIntegral;

	if (mask & DBIM_ACTUAL)  // desired size
		pdbi->ptActual = m_dbiDefault.ptActual;
*/
   ASSERT(DBIF_VIEWMODE_NORMAL == 0);
   ASSERT(DBIF_VIEWMODE_VERTICAL == 1);
   ASSERT(DBIF_VIEWMODE_FLOATING == 2);

	if (mask & DBIM_MINSIZE)
		pdbi->ptMinSize = m_dbiDefault[m_dwViewMode].ptMinSize;

	if (mask & DBIM_MAXSIZE)
		pdbi->ptMaxSize = m_dbiDefault[m_dwViewMode].ptMaxSize;

	if (mask & DBIM_INTEGRAL)
		pdbi->ptIntegral = m_dbiDefault[m_dwViewMode].ptIntegral;

	if (mask & DBIM_ACTUAL)  // desired size
		pdbi->ptActual = m_dbiDefault[m_dwViewMode].ptActual;

	if (mask & DBIM_TITLE) {
		USES_CONVERSION;
		lstrcpyW(pdbi->wszTitle, CT2OLE(GetTitle()));
	}

	if (mask & DBIM_MODEFLAGS)
		pdbi->dwModeFlags = m_dbiDefault[m_dwViewMode].dwModeFlags;

	if (mask & DBIM_BKCOLOR)
		pdbi->dwMask &= ~DBIM_BKCOLOR;	 // clear to use default color

   TRACE(_T("GetBandInfo: Min=%dx%d, Max=%dx%d, Actual=%dx%d\n"), pdbi->ptMinSize.x, pdbi->ptMinSize.y, pdbi->ptMaxSize.x, pdbi->ptMaxSize.y, pdbi->ptActual.x, pdbi->ptActual.y); //skst

	return S_OK;
}


// **************** IDeskBand2 ****************

STDMETHODIMP CBandObj::CanRenderComposited(BOOL *pfCanRenderComposited)
{
	if (pfCanRenderComposited != nullptr)
	{
		*pfCanRenderComposited = TRUE;
	}
	return S_OK;
}

STDMETHODIMP CBandObj::SetCompositionState(BOOL fCompositionEnabled)
{
	m_bCompositionEnabled = fCompositionEnabled;
	return S_OK;
}

STDMETHODIMP CBandObj::GetCompositionState(BOOL *pfCompositionEnabled)
{
	if (pfCompositionEnabled != nullptr)
	{
		*pfCompositionEnabled = m_bCompositionEnabled;
	}
	return S_OK;
}


// **************** IObjectWithSite ****************

//////////////////
// SetSite set the site and create window
//
STDMETHODIMP CBandObj::SetSite(IUnknown* pSite)
{
	CMDTARGENTRY;
	BOTRACEFN(_T("CBandObj(%p)::IObjectWithSite::SetSite %p\n"), this, pSite);

	// Standard implementation
	HRESULT hr = CTInputObjectSite::SetSite(pSite);
	if (!SUCCEEDED(hr))
		return hr;

	if (pSite!=NULL) {
		// Get parent window, which is site's window
		CComQIPtr<IOleWindow> spOleWin = m_spSite;
		if (!spOleWin)
			return E_FAIL;

		HWND hwndParent = NULL;
		spOleWin->GetWindow(&hwndParent);
		if (!hwndParent)
			return E_FAIL;

		CWnd* pParent = CWnd::FromHandle(hwndParent);
		ASSERT_VALID(pParent);
		CRect rc;
		pParent->GetClientRect(&rc);
      TRACE(_T("\tClient: (%d,%d) %dx%d\n"), rc.left, rc.top, rc.Width(), rc.Height());   //skst

		// Call virtual Create fn to create the window
		if (!OnCreateWindow(pParent, rc))
			return E_FAIL;
	}
	return S_OK;
}

// use standard implementation for GetSite
IMPLEMENT_IObjectWithSite_GetSite(CBandObj, CTInputObjectSite);

////////////////////////////////////////////////////////////////
// CBandObj MFC fns
//
IMPLEMENT_DYNAMIC(CBandObj, CWnd)
BEGIN_MESSAGE_MAP(CBandObj, CWnd)
	ON_WM_CREATE()
   ON_WM_SETFOCUS()
   ON_WM_KILLFOCUS()
	ON_WM_MOUSEACTIVATE()
   ON_WM_SIZE()                                       //skst
   ON_WM_ERASEBKGND()                                 //skst
	ON_WM_MEASUREITEM()
END_MESSAGE_MAP()

//////////////////
// Create the window. You can override to do your own thing
//
BOOL CBandObj::OnCreateWindow(CWnd* pParent, const CRect& rc)
{
	static BOOL bRegistered = FALSE;
	static CCriticalSection cs; // protect static global
	CTLockData lock(cs);

	// register window class if not already
	if (!bRegistered) {
		WNDCLASS wc;
		memset(&wc, 0, sizeof(wc));
		wc.style = CS_HREDRAW | CS_VREDRAW | CS_GLOBALCLASS;
		wc.lpfnWndProc = (WNDPROC)::DefWindowProc; // will get hooked by MFC
		wc.hInstance = AfxGetInstanceHandle();
      wc.hCursor = LoadCursor(NULL, IDC_ARROW);
      wc.lpszMenuName = NULL;
      wc.lpszClassName = BANDOBJCLASS;

//      wc.hbrBackground = ::CreateSolidBrush(::GetSysColor(COLOR_3DFACE));   //skst (was COLOR_WINDOW)
      wc.hbrBackground = HBRUSH(COLOR_3DFACE + 1); //skst (was COLOR_WINDOW+1)

      if (!AfxRegisterClass(&wc)) {
			TRACE(_T("*** CBandObj::GetWindow - AfxRegisterClass failed!\n"));
			return FALSE;
		}
		bRegistered = TRUE;
	}

	// create the window
	if (!CWnd::Create(BANDOBJCLASS, GetTitle(),
		WS_CHILD|WS_CLIPSIBLINGS|WS_BORDER,
		rc, pParent, m_dwBandID, NULL)) {

		TRACE(_T("*** CBandObj::GetWindow failed to create window!\n"));
		return FALSE;
	}
	return TRUE;
}

//////////////////
// Window is being created: load context menu and accels, if any
//
int CBandObj::OnCreate(LPCREATESTRUCT lpcs)
{
	BOTRACEFN(_T("CBandObj(%p)::OnCreate\n"), this);

	// Load context menu if any. context menu is 1st submenu in resource menu.
/*skst: The original code caused a memory leak because the main menu bar is never destroyed!
	CMenu menu;
	if (menu.LoadMenu(GetResourceID())) {
		HMENU hSubMenu = ::GetSubMenu(menu, 0);
		if (hSubMenu) {
			m_menu.Attach(hSubMenu);
			menu.Detach(); // otherwise destructor will destroy my menu!
		}
	}
*/
   m_menuBar.DestroyMenu();
	if (m_menuBar.LoadMenu(GetResourceID()))
   {
      CMenu *pSubMenu = m_menuBar.GetSubMenu(0);
		if (pSubMenu != NULL)
      {
         m_menu.DestroyMenu();
			m_menu.Attach(pSubMenu->Detach());
		}
	}

	// Load accelerators if any
	m_hAccel = ::LoadAccelerators(AfxGetInstanceHandle(), MAKEINTRESOURCE(GetResourceID()));

	// If there's no main window, install myself as main window,
	// so PreTranslateMessage etc will work. 
	//
	CWinApp*	pApp = AfxGetApp();
	ASSERT_VALID(pApp);
	if (pApp->m_pMainWnd != NULL)
   {
      /*skst:
         This happens when the bar changes from docked to floating or vice-versa.
         So, we ALWAYS have to set the main wnd to 'this'.
      */
		TRACE(_T("*** CBandObj: WARNING: Main window already exists ***\n"));
      TRACE(_T("\tcur = %#x, new = %#x\n"), pApp->m_pMainWnd, this);
	}

   pApp->m_pMainWnd = this;   //skst (see above)

	return CWnd::OnCreate(lpcs);
}

//////////////////
// This is required to route commands to the app object, which is normally
// done in MFC by CFrameWnd
//
BOOL CBandObj::OnCmdMsg(UINT nID, int nCode, void* pExtra,
	AFX_CMDHANDLERINFO* pHandlerInfo)
{
	// pump through myself first
	if (CWnd::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo))
		return TRUE;

	// pump through app
	CWinApp* pApp = AfxGetApp();
	if (pApp != NULL && pApp->OnCmdMsg(nID, nCode, pExtra, pHandlerInfo))
		return TRUE;

	return FALSE;
}

//////////////////
// Focus handling stuff
//
void CBandObj::OnSetFocus(CWnd* pOldWnd)
{
	BOTRACEFN(_T("CBandObj(%p)::OnSetFocus\n"), this);
	OnFocusChange(TRUE);
}

void CBandObj::OnKillFocus(CWnd* pNewWnd)
{
	BOTRACEFN(_T("CBandObj(%p)::OnKillFocus\n"), this);
	OnFocusChange(FALSE);
}

//////////////////
// focus changed: tell site
//
void CBandObj::OnFocusChange(BOOL bFocus)
{
	BOTRACEFN(_T("CBandObj(%p)::OnFocusChange\n"), this);
	if (m_spSite) {
		m_spSite->OnFocusChangeIS((IDockingWindow*)this, bFocus);
	}
}

int CBandObj::OnMouseActivate(CWnd* pDesktopWnd, UINT nHitTest, UINT msg)
{
	SetFocus();
	return MA_ACTIVATE;
}


#include "COMtoys/DllEntry.cpp"  //skst

//////////////////////////////////////////////////////////////////////////////
//skst
/*
   On 2000 (and probably others) OnWindowPosChanged() and OnSize() aren't
   called if the height is below 90 pixels, despite the corresponding
   messages being sent to the window!

   And even if it sends them (for example, if we're sizing the window
   horizontally), it reports the window height INCORRECTLY, saying it's
   90 pixels when in fact, it's less.

   For example,
      Drag deskband off bar so it's floating.
      Resize it vertically, shrinking it.
      When the height reaches 90, Windows stops sending WM_SIZE and
      WM_WINDOWPOSCHANGED messages to the window (confirmed by Spy++).
      It still sends WM_WINDOWPOSCHANGING, but the height is always 90.
      Because, apparently, Windows "forgets" to resize the enclosed
      child deskband window once it reaches that height.

   Docked:
      Shell_TrayWnd
         ReBarWindow32
            DeskBandClass
               Button
               Graph
            other windows
         other windows

   Floating:
      BaseBar
         ReBarWindow32
            DeskBandClass
               Button
               Graph

   So, if it's floating, the deskband window's parent has the correct size
   (and its client area is equal to its window area). Therefore, if we're
   floating and the parent's client size doesn't match the deskband window's,
   we resize the deskband window to the parent's client area.
*/
void CBandObj::OnSize(UINT nType, int cx, int cy)
{
//   TRACE(_T("CBandObj::OnSize(%d,%d)\n"),cx,cy);
	CWnd::OnSize(nType, cx, cy);

   // kludge around Windows bug (see header comment)
   if (m_dwViewMode == DBIF_VIEWMODE_FLOATING)
   {
      // resize this window to the size of the parent's client area
      CRect r;
      GetParent()->GetClientRect(r);
      if ((r.Width() != cx) || (r.Height() != cy))
      {
         TRACE(_T("\tThe deskband (%dx%d) does NOT fill the parent's client area (%dx%d). Resizing it...\n"), cx, cy, r.Width(), r.Height());
         SetWindowPos(NULL, r.left, r.top, r.Width(), r.Height(), SWP_NOOWNERZORDER | SWP_NOZORDER);
         cx = r.Width();
         cy = r.Height();
         // stop because SetWindowPos() forces another WM_SIZE message
         // (which will have the correct width & height)
         return;
      }
   }

   DoSize(nType, cx, cy);
}


/*
   This function notifies the site container that the band info has changed.
   This causes the container to get the band info and, if appropriate, resize
   the band.


   See doc for:

   Creating Custom Explorer Bars, Tool Bands, and Desk Bands

   "Band objects can send commands to their container through its
   IOleCommandTarget interface. To obtain the interface pointer, call the
   container's IInputObjectSite::QueryInterface method and ask for
   IID_IOleCommandTarget. You then send commands to the container with
   IOleCommandTarget::Exec. The command group is CGID_DeskBand. When a
   band object's IDeskBand::GetBandInfo method is called, the container
   uses the dwBandID parameter to assign the band object an identifier
   that is used for three of the commands."

   DBID Command Flags Constants
   IOleCommandTarget::Exec
*/
void CBandObj::NotifyBandInfoChanged()
{
   //IOleCommandTarget *pCmdTarget;
////   HRESULT hr = CTInputObjectSite::GetSite()->QueryInterface(IID_IOleCommandTarget, (void**)&pCmdTarget);  // this is the same as the line below
   //HRESULT hr = m_spSite->QueryInterface(IID_IOleCommandTarget, (void**)&pCmdTarget);
   //if (!SUCCEEDED(hr))
   //{
   //   TRACE("Error: Unable to get IOleCommandTarget.\n");
   //   return;
   //}
   //if (pCmdTarget == NULL)
   //{
   //   TRACE("Warning: IOleCommandTarget is NULL.\n");
   //   return;
   //}

   // this is a more succinct way to get the interface ptr than the above method
   // (see atlcomcli.h)
   CComQIPtr<IOleCommandTarget> spCmd(m_spSite);   // query for IOleCommandTarget from our IInputObjectSite
   if (!spCmd)
   {
      TRACE("Error: Unable to get IOleCommandTarget.\n");
      return;
   }

   VARIANT pvaIn{};
   pvaIn.vt = VT_I4;
   pvaIn.lVal = m_dwBandID;
   const HRESULT hr = spCmd->Exec(&CGID_DeskBand, DBID_BANDINFOCHANGED, OLECMDEXECOPT_DONTPROMPTUSER, &pvaIn, /*VARIANT *pvaOut*/ NULL);
   if (!SUCCEEDED(hr))
      TRACE(_T("IOleCommandTarget failed\n"));
}
//////////////////////////////////////////////////////////////////////////////


BOOL CBandObj::OnEraseBkgnd(CDC* pDC)
{
	return CWnd::OnEraseBkgnd(pDC);
}


/*
	We do this to avoid MFC asserting in some situations.

	From [wincore.cpp]CWnd::OnMeasureItem(int nIDCtl, LPMEASUREITEMSTRUCT lpMeasureItemStruct)
*/
void CBandObj::OnMeasureItem(int nIDCtl, LPMEASUREITEMSTRUCT lpMeasureItemStruct)
{
	if (lpMeasureItemStruct->CtlType == ODT_MENU)
	{
		ASSERT(lpMeasureItemStruct->CtlID == 0);
		CMenu* pMenu=NULL;

		_AFX_THREAD_STATE* pThreadState = _afxThreadState.GetData();
		if (pThreadState->m_hTrackingWindow == m_hWnd)
		{
			// start from popup
			pMenu = CMenu::FromHandle(pThreadState->m_hTrackingMenu);
		}
		else
		{
			// start from menubar
			pMenu = GetMenu();
		}

//This asserts in Debug mode, so we just return.
//		ENSURE(pMenu);
		if (pMenu == NULL)
			return;
	}

	Default();
}
