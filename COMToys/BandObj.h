////////////////////////////////////////////////////////////////
// Copyright 1998 Paul DiLascia
// If this program works, it was written by Paul DiLascia.
// If not, I don't know who wrote it.
// 
#pragma once

#include "ComToys.h"

DECLARE_SMARTPTR(IInputObjectSite)

//////////////////
// Handly to initialize a POINTL
struct CPointL : public POINTL {
	CPointL(long xx, long yy) { x=xx; y=yy; }
};



/*skst
	Deskbands are not supported as of Windows 7.

	REF:
	http://technet.microsoft.com/en-us/evalcenter/bb762064.aspx

	"Taskbar Extensions"
	http://msdn.microsoft.com/en-us/library/windows/desktop/dd378460.aspx#deskbands
	"In versions of Windows prior to Windows 7, something similar to thumbnail toolbar
	functionality could be achieved through a deskband—a toolbar hosted in the taskbar.
	For instance, Windows Media Player could minimize to the taskbar as a set of
	transport controls rather than a standard button. In Windows 7, deskbands can still
	be implemented and thumbnail toolbars are not intended to replace them all. Not all
	applications will lend themselves to a thumbnail toolbar, and another solution such
	as a deskband or a task in a destination list might be the right answer for your
	application; you must decide which solution works best for your application as part
	of your development cycle. However, be aware that deskbands must support Windows Aero
	with translucency ("glass") enabled and the IDeskBand2 interface."

	IDeskBand2
	http://msdn.microsoft.com/en-us/library/windows/desktop/bb762064.aspx

	IDeskBand
	http://msdn.microsoft.com/en-us/library/windows/desktop/bb762067.aspx
*/


//////////////////
// class factory used to create band objects
//
class CBandObjFactory : public COleObjectFactory, public CTFactory
{
protected:
	CATID m_catid;		// band object categroy ID

public:
	CBandObjFactory(REFCLSID clsid, CRuntimeClass* pClass, const CATID& catid, UINT nResID);
	~CBandObjFactory();

	const CATID& GetCatID() { return m_catid; }

	// overrides
	virtual BOOL OnRegister(BOOL bRegister);
	virtual BOOL UpdateRegistry(BOOL bRegister);
	DECLARE_DYNAMIC(CBandObjFactory)
};

//////////////////
// This is here just in case there's ever a need to do
// band-obj stuff derive your DLL from this
//
class CBandObjDll : public CTMfcModule
{
protected:
	CBandObjDll();
	virtual ~CBandObjDll();

public:
	virtual BOOL AddBandClass(REFCLSID clsid, CRuntimeClass* pClass, const CATID& catid, UINT nResID);

	// so you can override (rare)
	virtual CBandObjFactory* OnCreateFactory(REFCLSID clsid, CRuntimeClass* pClass, const CATID& catid, UINT nResID);

	DECLARE_DYNAMIC(CBandObjDll);
};

// to reduce typing and angle brackets :)
typedef CTObjectWithSite<IInputObjectSite> CTInputObjectSite;

//////////////////
// The band object! Common implementation for info, comm and desk bands.
// This is a CWnd, and it also inherits all the band object COM interfaces.
//
class CBandObj : public CWnd,

	// interfaces
// public IUnknown,			// inherited
//	public IOleWindow,		// inherited
//	public IDockingWindow,	// inherited
//	public IDeskBand,			// inherited
	public IDeskBand2,
	public IObjectWithSite,
	public IInputObject,
//	public IPersist,			// inherited
	public IPersistStream,
	public IContextMenu,

	// implementations 
	public CTOleWindow,
	public CTDockingWindow,
	public CTPersist,
   public CTPersistStream,
	public CTInputObject,
	public CTInputObjectSite,
	public CTMfcContextMenu
{
protected:
	DESKBANDINFO m_dbiDefault[3];	// default band info          //skst: horiz, vert, and floating
	CString	m_strTitle;				// window title
	UINT		m_nIDRes;				// resource ID
	DWORD		m_dwBandID;				// band ID (from Windows)
	DWORD		m_dwViewMode;			// view mode (from Windows)
   CMenu    m_menuBar;                                         //skst: need to destroy m_menu's "parent"
	CMenu		m_menu;					// context menu
	HACCEL	m_hAccel;				// accelerators
	BOOL		m_bModified;			// always FALSE;
	BOOL		m_bCompositionEnabled;	// IDeskBand2

public:
	static BOOL bTRACE; // controls tracing

	CBandObj(REFCLSID clsid);
	virtual ~CBandObj();

	UINT		GetResourceID()	{ return m_nIDRes; }
	CString	GetTitle()			{ return m_strTitle; }

	// overrides
	virtual LPUNKNOWN GetInterface(REFIID iid);
	virtual LPUNKNOWN GetInterfaceHook(const void* iid);
	virtual void		OnFinalRelease();

	// MFC overrides
	virtual BOOL OnCmdMsg(UINT, int, void*, AFX_CMDHANDLERINFO*);

	// new fns
	virtual void OnFocusChange(BOOL bFocus);
	virtual BOOL OnCreateWindow(CWnd* pParent, const CRect& rc);

   // functions for saving (aka transferring) data between docking states
   // (The parameters are already validated by the time these are called.)
   virtual void    IPersistStreamGetSizeMax(ULARGE_INTEGER * /*pcbSize*/) {}  //skst
   virtual HRESULT IPersistStreamSave(IStream * /*pStream*/) { return S_OK; } //skst
   virtual HRESULT IPersistStreamLoad(IStream * /*pStream*/) { return S_OK; } //skst

   void NotifyBandInfoChanged();                                              //skst
   virtual void    DoSize(UINT /*nType*/, int /*cx*/, int /*cy*/) {}          //skst

   // interfaces implemented
	DECLARE_IUnknown();
	DECLARE_IOleWindow();
	DECLARE_IDockingWindow();
	DECLARE_IInputObject();
	DECLARE_IPersist();
	DECLARE_IPersistStream();
	DECLARE_IContextMenu();
	DECLARE_IObjectWithSite();
   STDMETHOD (GetBandInfo) (DWORD, DWORD, DESKBANDINFO*);	// IDeskBand
	STDMETHOD (CanRenderComposited) (BOOL*);						// IDeskBand2
	STDMETHOD (SetCompositionState) (BOOL);						// IDeskBand2
	STDMETHOD (GetCompositionState) (BOOL*);						// IDeskBand2

protected:
	afx_msg int  OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg int  OnMouseActivate(CWnd* pDesktopWnd, UINT nHitTest, UINT message);
   afx_msg void OnSize(UINT nType, int cx, int cy);											//skst
   afx_msg BOOL OnEraseBkgnd(CDC* pDC);															//skst
	afx_msg void OnMeasureItem(int nIDCtl, LPMEASUREITEMSTRUCT lpMeasureItemStruct);	//skst
   DECLARE_MESSAGE_MAP();
	DECLARE_DYNAMIC(CBandObj)
};
