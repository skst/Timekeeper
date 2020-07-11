////////////////////////////////////////////////////////////////
// ComToys(TM) Copyright 1999 Paul DiLascia
// If this code works, it was written by Paul DiLascia.
// If not, I don't know who wrote it.
//
#ifndef COMTOYS_H
#define COMTOYS_H

#include <wininet.h>//VS.NET
/*
   VCInstallDir = C:\Program Files\Microsoft Visual Studio 8\VC\
   We need the comdef.h in the MS Platform SDK at
      C:\Program Files\Microsoft Platform SDK\Include\ComDef.h
*/
//#include </Program Files/Microsoft Platform SDK/Include/comdef.h>
#include <comdef.h>			 // basic COM defs (_bstr_t etc., includes comutil.h)
#include <shlobj.h>			 // shell objects
#include <afxdisp.h>			 // MFC OLE stuff
#include <afxctl.h>			 // control stuff
#include <atlbase.h>			 // ATL stuff
#include <afxmt.h>			 // Multithreading

#include <map>

#ifndef __cplusplus
#error ComToys requires C++!
#endif

#if !defined(BUILD_COMTOYS_DLL) && !defined(USE_COMTOYS_DLL)
// force-link precompiled hdr stuff
//skst: don't need if no pch: #pragma comment(linker, "/include:__ctForceLinkSTDAFX")
#endif

//////////////////
// Stuff to make DLL extension classes work
//
#if defined(BUILD_COMTOYS_DLL)
#pragma message("Building COMtoys as DLL")   //skst
#	define DLLFUNC  __declspec ( dllexport )
#	define DLLDATA  __declspec ( dllexport )
#	define DLLCLASS __declspec ( dllexport )
#elif defined(USE_COMTOYS_DLL)
#pragma message("Using COMtoys DLL")         //skst
#	define DLLFUNC  __declspec ( dllimport )
#	define DLLDATA  __declspec ( dllimport )
#	define DLLCLASS __declspec ( dllimport )
#else
#	define DLLFUNC
#	define DLLDATA
#	define DLLCLASS
#endif

#include "debug.h"

// Acts like namespace
class DLLCLASS ComToys {
public:
	static BOOL bTRACE; // to turn on tracing
};

// Following stuff is general, move into PixieLb
inline CString PLGetResourceSubstring(UINT nResID, UINT n)
{
	CString sRes, s;
	if (sRes.LoadString(nResID))
		AfxExtractSubString(s, sRes, n);
	return s;
}

extern DLLFUNC HRESULT CTOleAlloc(ULONG cb, LPVOID* ppv);
extern DLLFUNC		LONG CTDelRegTree(HKEY hParentKey,const CString& strKeyName);

// debugging macros for indented tracing--see debug.h
#ifdef _DEBUG
#define CTTRACEFN															\
	CTraceFn __fooble;													\
	if (ComToys::bTRACE)													\
		TRACE																	\

#define CTTRACE															\
	if (ComToys::bTRACE)													\
		TRACE

#else // not _DEBUG
#define CTTRACEFN
#define CTTRACE

#endif

// Following is used to check for NULL args
#define CTCHECKARG(ppv)													\
	if (ppv==NULL)															\
		return E_INVALIDARG;												\

////////////////
// Standard entry macros to set up MFC state, w/TRACE and non-TRACE versions.
//
// Extern fn like DllGetClassObject: use AfxGetStaticModuleState()
#define MFCENTRY		AFX_MANAGE_STATE(AfxGetStaticModuleState());
#define MFCENTRYTR	AFX_MANAGE_STATE(AfxGetStaticModuleState()); CTTRACEFN

// CCmdTarget: use m_pModuleState
#define CMDTARGENTRY		AFX_MANAGE_STATE(m_pModuleState)
#define CMDTARGENTRYTR	AFX_MANAGE_STATE(m_pModuleState); CTTRACEFN

//////////////////
// This simple class is used to lock/unlock a critical section. To use it:
//    CTLockData(mySyncObject);
// Constructor calls Lock; destructor calls Unlock, so you don't have to.
//
class DLLCLASS CTLockData {
private:
	CSyncObject& m_syncObj;
public:
	CTLockData(CSyncObject& so) : m_syncObj(so) { VERIFY(m_syncObj.Lock()); }
	~CTLockData() { m_syncObj.Unlock(); }
};

/////////////////
// Handy class to get the name of a module
//
class DLLCLASS CTModuleName {
protected:
	TCHAR buf[_MAX_PATH];
public:
	CTModuleName(HINSTANCE hinst) {
		::GetModuleFileName(hinst, buf, _MAX_PATH);
	}
	operator LPCTSTR () { return buf; }
};

// friendly version doesn't require ptr to OLESTR 
inline LPOLESTR StringFromCLSID(REFCLSID clsid)
{
	LPOLESTR lp=NULL;
	::StringFromCLSID(clsid, &lp);
	return lp;
}

// declare a typedef; eg SPIPersist
#define DECLARE_SMARTPTR(ifacename) \
typedef CComQIPtr<ifacename> SP##ifacename;

// Special case for IUnknown: don't use CComQIPtr
typedef CComPtr<IUnknown> SPIUnknown;

////////////////
// Handy registrar class. Just instantiate and go.
//
class CTRegistrar : public CComPtr<IRegistrar> {
public:
	CTRegistrar() {
	   CTTRACEFN(_T(__FUNCTION__) _T("\n"));
		CoCreateInstance(CLSID_Registrar, NULL, CLSCTX_INPROC);
		ASSERT(p);
	}
};

////////////////
// Handy Category Register class. Instantiate and go.
//
class CTCatRegister : public CComPtr<ICatRegister> {
public:
	CTCatRegister() {
		CoCreateInstance(CLSID_StdComponentCategoriesMgr, NULL, CLSCTX_INPROC);
		ASSERT(p);
	}
};

//////////////////
// This class is for future expansion--non-MFC module
//
class DLLCLASS CTModule {
protected:
	static CTModule* g_pModule; // the one-and-only
	HINSTANCE m_hInstance;		 // module handle
	LONG		 m_dwLock;			 // module lock (object count)

public:
	CTModule();
	virtual ~CTModule();

	virtual LONG Lock();
	virtual LONG Unlock();

	// These correspond to the 4 special entry points.
	// Register/unregister are combined into one method w/BOOL arg.
	virtual HRESULT OnGetClassObject(REFCLSID, REFIID,LPVOID*);
	virtual HRESULT OnCanUnloadNow();
	virtual HRESULT OnRegisterServer(BOOL bRegister);
	virtual HRESULT OnInstall(BOOL bInstall, LPCWSTR pszCmdLine);

	virtual BOOL InitInstance(HINSTANCE hInstance);
	virtual int  ExitInstance();

	HINSTANCE GetInstanceHandle() { return m_hInstance; }
	static CTModule* GetModule()	{ return g_pModule; }
};

inline LONG CTLockApp()   { return CTModule::GetModule()->Lock(); }
inline LONG CTUnlockApp() { return CTModule::GetModule()->Unlock(); }

// instantiate in your object to lock/unlock the DLL.
class DLLCLASS CTModuleLock {
public:
	CTModuleLock()  { CTLockApp(); }
	~CTModuleLock() { CTUnlockApp(); }
};

//////////////// IUnknown ////////////////
#define DECLARE_IUnknown()												\
	STDMETHOD_(ULONG, AddRef)();										\
	STDMETHOD_(ULONG, Release)();										\
	STDMETHOD(QueryInterface)(REFIID iid, LPVOID* ppvObj);

#define IMPLEMENT_IUnknown_AddRef(baseclas, implclas)			\
STDMETHODIMP_(ULONG) baseclas::AddRef()							\
{																				\
	CMDTARGENTRYTR(_T(#baseclas) _T("(%p)::AddRef\n"),this);	\
	return implclas::AddRef();											\
}																				\

#define IMPLEMENT_IUnknown_Release(baseclas, implclas)		\
STDMETHODIMP_(ULONG) baseclas::Release()							\
{																				\
	CMDTARGENTRYTR(_T(#baseclas) _T("(%p)::Release\n"), this); \
	return implclas::Release();										\
}																				\

#define IMPLEMENT_IUnknown_QueryInterface(baseclas, implclas) \
STDMETHODIMP baseclas::QueryInterface(REFIID iid, LPVOID* ppvRet) \
{																				\
	CTCHECKARG(ppvRet);													\
	CMDTARGENTRYTR(_T(#baseclas)										\
		_T("(%p)::QueryInterface(%s)\n"), this, _TR(iid));		\
	HRESULT hr = implclas::QueryInterface(iid, ppvRet);		\
	CTTRACE(_T(">") _T(#baseclas)										\
		_T("::QueryInterface returns %s, *ppv=%p\n"),			\
		_TR(hr), *ppvRet);												\
   return hr;																\
}																				\

#define IMPLEMENT_IUnknown(baseclas, implclas)					\
IMPLEMENT_IUnknown_AddRef(baseclas, implclas)					\
IMPLEMENT_IUnknown_Release(baseclas, implclas)					\
IMPLEMENT_IUnknown_QueryInterface(baseclas, implclas)			\

////////////////
// Implement IUnkown for a CCmdTarget-derived class. Delegates to CCmdTarget
//
#define IMPLEMENT_IUnknownCT_AddRef(baseclas)					\
STDMETHODIMP_(ULONG) baseclas::AddRef()							\
{																				\
	CMDTARGENTRYTR(_T(#baseclas) _T("(%p)::AddRef "),this);	\
	DWORD dwRef = ExternalAddRef();									\
	CTTRACE(_T("> returns count=%d\n"),dwRef);					\
	return dwRef;															\
}

#define IMPLEMENT_IUnknownCT_Release(baseclas)					\
STDMETHODIMP_(ULONG) baseclas::Release()							\
{																				\
	CMDTARGENTRYTR(_T(#baseclas) _T("(%p)::Release "),this);	\
	DWORD dwRef = ExternalRelease();									\
	CTTRACE(_T("> returns count=%d\n"),dwRef);					\
	return dwRef;															\
}

#define IMPLEMENT_IUnknownCT_QueryInterface(baseclas)			\
STDMETHODIMP baseclas::QueryInterface(REFIID iid, LPVOID* ppvRet) \
{																				\
	CTCHECKARG(ppvRet);													\
	CMDTARGENTRYTR(_T(#baseclas)										\
		_T("(%p)::QueryInterface(%s) "), this, _TR(iid));		\
	HRESULT hr = ExternalQueryInterface(&iid, ppvRet);			\
	CTTRACE(_T("> returns %s, *ppv=%p, count=%d\n"),			\
		_TR(hr), *ppvRet, CCmdTarget::m_dwRef);					\
   return hr;																\
}

#define IMPLEMENT_IUnknownCT(baseclas)								\
IMPLEMENT_IUnknownCT_AddRef(baseclas)								\
IMPLEMENT_IUnknownCT_Release(baseclas)								\
IMPLEMENT_IUnknownCT_QueryInterface(baseclas)

//////////////////
// Root COM Object. Implements IUnknown, assuming STA. Not implemented yet.
//
class DLLCLASS CTComObj {
protected:
	CTModuleLock	m_lockApp;	// automatic module locking
	DWORD				m_dwRef;		// reference count
public:
	CTComObj();
	virtual ~CTComObj();
	virtual void OnFinalRelease();
	// you must implement to return interface pointers for QI
	virtual LPUNKNOWN GetInterface(REFIID iid);
	DECLARE_IUnknown();
};

#define DECLARE_IClassFactory()										\
	STDMETHOD(CreateInstance)(LPUNKNOWN, REFIID, LPVOID*);	\
	STDMETHOD(LockServer)(BOOL);										\

#define DECLARE_IClassFactory2()										\
	STDMETHOD(GetLicInfo)(LPLICINFO);								\
	STDMETHOD(RequestLicKey)(DWORD, BSTR*);						\
	STDMETHOD(CreateInstanceLic)(LPUNKNOWN, LPUNKNOWN, REFIID, BSTR, LPVOID*); \

//////////////////
// Basic class factory. Not implemented yet.
//
class DLLCLASS CTFactory {
protected:
	static CPtrList g_lsFactories;		 // list of all factories
	static CCriticalSection g_mydata;	 // lock to protect it
	REFCLSID m_clsid;					 // class ID
	UINT		m_nIDRes;				 // resource ID for stuff
	int		m_nThreadingModel;	 // threading model (see below)

public:
	BOOL		m_bAutoDel;				 // delete in ExitInstance?

	CTFactory(REFCLSID clsid, UINT nResID, UINT tm=TMAPARTMENT);
	virtual ~CTFactory();

	// not implmented yet
	DECLARE_IClassFactory();
	DECLARE_IClassFactory2();

	// threading model
	enum { TMNONE=0, TMAPARTMENT, TMFREE, TMBOTH };

	static BOOL OnRegisterAll(BOOL bRegister);
	static CTFactory* GetFactory(REFCLSID clsid);
	static void DeleteAll();

	// useful methods
	const CLSID& GetClassID()	{ return m_clsid; }
	UINT GetResourceID()			{ return m_nIDRes; }
	UINT GetThreadingModel()	{ return m_nThreadingModel; }

	// substrings must be in the resource string n_IDRes, separated by newline
	CString GetResourceSubstring(UINT n) {
		return PLGetResourceSubstring(GetResourceID(), n);
	}
	CString GetTitle()			{ return GetResourceSubstring(0); }
	CString GetClassName()		{ return GetResourceSubstring(1); }
	CString GetProgID()			{ return GetResourceSubstring(2); }

	// MFC-like virt fns to override
	virtual BOOL OnRegister(BOOL bRegister);
	virtual BOOL OnInitRegistryVariables(IRegistrar* pReg);
};

//////////////// IPersist ////////////////

#define DECLARE_IPersist()												\
	STDMETHOD(GetClassID)(LPCLSID pClassID);						\

#define IMPLEMENT_IPersist(baseclas, implclas)					\
STDMETHODIMP baseclas::GetClassID(LPCLSID pClassID)			\
{																				\
	CMDTARGENTRYTR(_T(#baseclas)										\
		_T("(%p)::IPersist::GetClassID\n"), this);				\
	return implclas::GetClassID(pClassID);							\
}																				\

////////////////
// CTPersist--all it needs is a Class ID
//
class DLLCLASS CTPersist {
public:
	const CLSID& m_clsid;
	CTPersist(const CLSID& clsid);
	virtual ~CTPersist();
	DECLARE_IPersist();
};

//////////////// IPersistStream ////////////////

#define DECLARE_IPersistStream()										\
   STDMETHOD(IsDirty)(void);											\
   STDMETHOD(Load)(LPSTREAM);											\
   STDMETHOD(Save)(LPSTREAM, BOOL);									\
   STDMETHOD(GetSizeMax)(ULARGE_INTEGER*);						\

#define IMPLEMENT_IPersistStream_IsDirty(baseclas, implclas) \
STDMETHODIMP baseclas::IsDirty()										\
{																				\
	CMDTARGENTRYTR(_T(#baseclas)										\
		_T("(%p)::IPersistStream::IsDirty\n"), this);			\
	return implclas::IsDirty();										\
}

#define IMPLEMENT_IPersistStream_Load(baseclas, implclas)	\
STDMETHODIMP baseclas::Load(LPSTREAM lpStream)					\
{																				\
	CMDTARGENTRYTR(_T(#baseclas)										\
		_T("(%p)::IPersistStream::Load\n"), this);				\
	return implclas::Load(lpStream);									\
}																				\

#define IMPLEMENT_IPersistStream_Save(baseclas, implclas)	\
STDMETHODIMP baseclas::Save(LPSTREAM lpStream, BOOL bClearDirty) \
{																				\
	CMDTARGENTRYTR(_T(#baseclas)										\
		_T("(%p)::IPersistStream::Save\n"), this);				\
	return implclas::Save(lpStream, bClearDirty);				\
}																				\

#define IMPLEMENT_IPersistStream_GetSizeMax(baseclas, implclas) \
STDMETHODIMP baseclas::GetSizeMax(ULARGE_INTEGER* pcbSize)	\
{																				\
	CMDTARGENTRYTR(_T(#baseclas)										\
		_T("(%p)::IPersistStream::GetSizeMax\n"), this);		\
	return implclas::GetSizeMax(pcbSize);							\
}																				\

#define IMPLEMENT_IPersistStream(baseclas, implclas)			\
IMPLEMENT_IPersistStream_IsDirty(baseclas, implclas)			\
IMPLEMENT_IPersistStream_Load(baseclas, implclas)				\
IMPLEMENT_IPersistStream_Save(baseclas, implclas)				\
IMPLEMENT_IPersistStream_GetSizeMax(baseclas, implclas)		\

/////////////////
// CTPersistStream
// 
class DLLCLASS CTPersistStream {
protected:
	BOOL& m_bModified;
public:
	CTPersistStream(BOOL& bModified);
	virtual ~CTPersistStream();
	DECLARE_IPersistStream();
};

//////////////// IPersistStreamInit ////////////////

#define DECLARE_IPersistStreamInit()								\
DECLARE_IPersistStream();												\
STDMETHOD(InitNew)(void);												\

#define IMPLEMENT_IPersistStreamInit_IsDirty(baseclas, implclas) \
STDMETHODIMP baseclas::IsDirty()										\
{																				\
	CMDTARGENTRYTR(_T(#baseclas)										\
		_T("(%p)::IPersistStreamInit::IsDirty\n"), this);		\
	return implclas::IsDirty();										\
}

#define IMPLEMENT_IPersistStreamInit_Load(baseclas, implclas) \
STDMETHODIMP baseclas::Load(LPSTREAM lpStream)					\
{																				\
	CMDTARGENTRYTR(_T(#baseclas)										\
		_T("(%p)::IPersistStreamInit::Load\n"), this);			\
	return implclas::Load(lpStream);									\
}																				\

#define IMPLEMENT_IPersistStreamInit_Save(baseclas, implclas) \
STDMETHODIMP baseclas::Save(LPSTREAM lpStream, BOOL bClearDirty) \
{																				\
	CMDTARGENTRYTR(_T(#baseclas)										\
		_T("(%p)::IPersistStreamInit::Save\n"), this);			\
	return implclas::Save(lpStream, bClearDirty);				\
}																				\

#define IMPLEMENT_IPersistStreamInit_GetSizeMax(baseclas, implclas) \
STDMETHODIMP baseclas::GetSizeMax(ULARGE_INTEGER* pcbSize)	\
{																				\
	CMDTARGENTRYTR(_T(#baseclas)										\
		_T("(%p)::IPersistStreamInit::GetSizeMax\n"), this);	\
	return implclas::GetSizeMax(pcbSize);							\
}																				\

#define IMPLEMENT_IPersistStreamInit_InitNew(baseclas, implclas) \
STDMETHODIMP baseclas::InitNew()										\
{																				\
	CMDTARGENTRYTR(_T(#baseclas)										\
		_T("(%p)::IPersistStreamInit::InitNew\n"), this);		\
	return implclas::InitNew();	/*skst*/							\
}

#define IMPLEMENT_IPersistStreamInit(baseclas, implclas)		\
IMPLEMENT_IPersistStreamInit_IsDirty(baseclas, implclas)		\
IMPLEMENT_IPersistStreamInit_Load(baseclas, implclas)			\
IMPLEMENT_IPersistStreamInit_Save(baseclas, implclas)			\
IMPLEMENT_IPersistStreamInit_GetSizeMax(baseclas, implclas)	\
IMPLEMENT_IPersistStreamInit_InitNew(baseclas, implclas)		\

/////////////////
// CTPersistStreamInit
// Since this is mutually exclusive with IPersistStream, OK
// to derive from CTPersistStream
// 
class DLLCLASS CTPersistStreamInit : public CTPersistStream {
public:
	CTPersistStreamInit(BOOL& bModified);
	virtual ~CTPersistStreamInit();
	STDMETHOD(InitNew)(void);
};

/////////////////// IPersistFile /////////////////

#define DECLARE_IPersistFile()										\
   STDMETHOD(IsDirty)(void);											\
   STDMETHOD(Load)(LPCOLESTR pszFileName, DWORD dwMode);		\
   STDMETHOD(Save)(LPCOLESTR pszFileName, BOOL fRemember);	\
	STDMETHOD(SaveCompleted)(LPCOLESTR pszFileName);			\
	STDMETHOD(GetCurFile)(LPOLESTR __RPC_FAR *ppszFileName);	\
        
#define IMPLEMENT_IPersistFile_IsDirty(baseclas, implclas)	\
STDMETHODIMP baseclas::IsDirty()										\
{																				\
	CMDTARGENTRY(_T(#baseclas)											\
		_T("(%p)::IPersistFile::IsDirty\n"), this);				\
	return implclas::IsDirty();										\
}																				\

#define IMPLEMENT_IPersistFile_Load(baseclas, implclas)		\
STDMETHODIMP baseclas::Load(LPCOLESTR pszFileName, DWORD dwMode) \
{																				\
	CMDTARGENTRY(_T(#baseclas)											\
		_T("(%p)::IPersistFile::Load(%s)\n"),						\
		this, (LPCTSTR)CString(pszFileName));						\
	return implclas::Load(pszFileName, dwMode);					\
}																				\

#define IMPLEMENT_IPersistFile_Save(baseclas, implclas)		\
STDMETHODIMP baseclas::Save(LPCOLESTR pszFileName, BOOL fRemember) \
{																				\
	CMDTARGENTRY(_T(#baseclas)											\
		_T("(%p)::IPersistFile::Save(%s)\n"),						\
		this, (LPCTSTR)CString(pszFileName));						\
	return implclas::Save(pszFileName, fRemember);				\
}																				\

#define IMPLEMENT_IPersistFile_SaveCompleted(baseclas, implclas) \
STDMETHODIMP baseclas::SaveCompleted(LPCOLESTR pszFileName)	\
{																				\
	CMDTARGENTRY(_T(#baseclas)											\
		_T("(%p)::IPersistFile::SaveCompleted(%s)\n"),			\
		this, (LPCTSTR)CString(pszFileName));						\
	return implclas::SaveCompleted(pszFileName);					\
}																				\

#define IMPLEMENT_IPersistFile_GetCurFile(baseclas, implclas) \
STDMETHODIMP baseclas::GetCurFile(LPOLESTR __RPC_FAR *ppszFileName) \
{																				\
	CMDTARGENTRY(_T(#baseclas)											\
		_T("(%p)::IPersistFile::GetCurFile\n"), this);			\
	return implclas::GetCurFile(ppszFileName);					\
}

#define IMPLEMENT_IPersistFile(baseclas, implclas)				\
IMPLEMENT_IPersistFile_IsDirty(baseclas, implclas)				\
IMPLEMENT_IPersistFile_Load(baseclas, implclas)					\
IMPLEMENT_IPersistFile_Save(baseclas, implclas)					\
IMPLEMENT_IPersistFile_SaveCompleted(baseclas, implclas)		\
IMPLEMENT_IPersistFile_GetCurFile(baseclas, implclas)			\

/////////////////
// CTPersistFile
// 
class DLLCLASS CTPersistFile {
public:
	BOOL& m_bModified;
	CTPersistFile(BOOL& bModified);
	virtual ~CTPersistFile();
	DECLARE_IPersistFile();
	virtual LPCTSTR OnGetPathName();
};

/////////////////// IOleWindow ////////////////

#define DECLARE_IOleWindow()											\
   STDMETHOD (GetWindow) (HWND*);									\
   STDMETHOD (ContextSensitiveHelp) (BOOL);						\

#define IMPLEMENT_IOleWindow_GetWindow(baseclas, implclas)	\
STDMETHODIMP baseclas::GetWindow(HWND* pHwnd)					\
{																				\
	CMDTARGENTRYTR(_T(#baseclas)										\
		_T("(%p)::IOleWindow::GetWindow\n"), this);				\
	return implclas::GetWindow(pHwnd);								\
}																				\

#define IMPLEMENT_IOleWindow_ContextSensitiveHelp(baseclas, implclas) \
STDMETHODIMP baseclas::ContextSensitiveHelp(BOOL fEnterMode) \
{																				\
	CMDTARGENTRYTR(_T(#baseclas)										\
		_T("(%p)::IOleWindow::ContextSensitiveHelp\n"), this); \
	return implclas::ContextSensitiveHelp(fEnterMode);			\
}																				\

#define IMPLEMENT_IOleWindow(baseclas, implclas)				\
IMPLEMENT_IOleWindow_GetWindow(baseclas, implclas)				\
IMPLEMENT_IOleWindow_ContextSensitiveHelp(baseclas, implclas) \

// Implementation class just stores an HWND
class DLLCLASS CTOleWindow {
public:
	const HWND& m_hWndOleWindow;
	CTOleWindow(const HWND& hWnd) : m_hWndOleWindow(hWnd)	{ }
	virtual ~CTOleWindow() { }
	DECLARE_IOleWindow();
};

//////////////// IDockingWindow ////////////////
//
#define DECLARE_IDockingWindow()										\
   STDMETHOD(ShowDW) (BOOL fShow);									\
   STDMETHOD(CloseDW) (DWORD dwReserved);							\
   STDMETHOD(ResizeBorderDW) (LPCRECT prcBorder,				\
		IUnknown* punkDWSite, BOOL fReserved);						\

#define IMPLEMENT_IDockingWindow_ShowDW(baseclas,implclas)	\
STDMETHODIMP baseclas::ShowDW(BOOL fShow)							\
{																				\
	CMDTARGENTRYTR(_T(#baseclas)										\
		_T("(%p)::IDockingWindow::ShowDW(%d)\n"),this,fShow);	\
	return implclas::ShowDW(fShow);									\
}																				\

#define IMPLEMENT_IDockingWindow_CloseDW(baseclas,implclas)	\
STDMETHODIMP baseclas::CloseDW(DWORD dwReserved)				\
{																				\
	CMDTARGENTRYTR(_T(#baseclas)										\
		_T("(%p)::IDockingWindow::CloseDW\n"), this);			\
	return implclas::CloseDW(dwReserved);							\
}																				\

#define IMPLEMENT_IDockingWindow_ResizeBorderDW(baseclas,implclas) \
STDMETHODIMP baseclas::ResizeBorderDW(LPCRECT prcBorder,		\
	IUnknown* punkDWSite, BOOL fReserved)							\
{																				\
	CMDTARGENTRYTR(_T(#baseclas)										\
		_T("(%p)::IDockingWindow::ResizeBorderDW\n"), this);	\
	return implclas::ResizeBorderDW(prcBorder, punkDWSite, fReserved); \
}																				\

#define IMPLEMENT_IDockingWindow(baseclas,implclas)			\
IMPLEMENT_IDockingWindow_ShowDW(baseclas,implclas)	\
IMPLEMENT_IDockingWindow_CloseDW(baseclas,implclas)	\
IMPLEMENT_IDockingWindow_ResizeBorderDW(baseclas,implclas)	\

//////////////////
// CTDockingWindow manages a window
//
class DLLCLASS CTDockingWindow {
public:
	const HWND& m_hWndDockingWindow;
	CTDockingWindow(const HWND& hWnd) : m_hWndDockingWindow(hWnd) { }
	virtual ~CTDockingWindow() { }
	DECLARE_IDockingWindow();
};

////////////////// IInputObject ////////////////

#define DECLARE_IInputObject()										\
   STDMETHOD (UIActivateIO) (BOOL, LPMSG);						\
   STDMETHOD (HasFocusIO) (void);									\
   STDMETHOD (TranslateAcceleratorIO) (LPMSG);					\

#define IMPLEMENT_IInputObject_UIActivateIO(baseclas,implclas) \
STDMETHODIMP baseclas::UIActivateIO(BOOL fActivate, LPMSG pMsg) \
{																				\
	CMDTARGENTRYTR(_T(#baseclas)										\
		_T("(%p)::IInputObject::UIActivateIO(%d)\n"),			\
		this, fActivate);													\
	return implclas::UIActivateIO(fActivate, pMsg);				\
}																				\

#define IMPLEMENT_IInputObject_HasFocusIO(baseclas,implclas) \
STDMETHODIMP baseclas::HasFocusIO()									\
{																				\
	CMDTARGENTRYTR(_T(#baseclas)										\
		_T("(%p)::IInputObject::HasFocusIO\n"), this);			\
	return implclas::HasFocusIO();									\
}																				\

#define IMPLEMENT_IInputObject_TranslateAcceleratorIO(baseclas,implclas) \
STDMETHODIMP baseclas::TranslateAcceleratorIO(LPMSG pMsg)	\
{																				\
	CMDTARGENTRYTR(_T(#baseclas)										\
		_T("(%p)::IInputObject::TranslateAcceleratorIO\n"),	\
		this);																\
	return implclas::TranslateAcceleratorIO(pMsg);				\
}

#define IMPLEMENT_IInputObject(baseclas,implclas)				\
IMPLEMENT_IInputObject_UIActivateIO(baseclas,implclas)		\
IMPLEMENT_IInputObject_HasFocusIO(baseclas,implclas)			\
IMPLEMENT_IInputObject_TranslateAcceleratorIO(baseclas,implclas) \

// input object: manages window and accelerator
class DLLCLASS CTInputObject {
public:
	HACCEL&		m_hAccel;			// accelerators to use
	const HWND&	m_hWndInputObj;	// ..and window
	CTInputObject(const HWND& hWnd, HACCEL& hAccel) :
		m_hWndInputObj(hWnd), m_hAccel(hAccel) { }
	virtual ~CTInputObject() { }
	DECLARE_IInputObject();
};

#define DECLARE_IContextMenu()										\
   STDMETHOD (QueryContextMenu)(HMENU, UINT, UINT, UINT, UINT); \
   STDMETHOD (InvokeCommand)(LPCMINVOKECOMMANDINFO);			\
   STDMETHOD (GetCommandString)(UINT_PTR, UINT, UINT*, LPSTR, UINT); \

#define IMPLEMENT_IContextMenu_QueryContextMenu(baseclas,implclas) \
STDMETHODIMP baseclas::QueryContextMenu(HMENU p1,				\
	UINT p2, UINT p3, UINT p4, UINT p5)								\
{																				\
	CMDTARGENTRYTR(_T(#baseclas)										\
		_T("(%p)::IContextMenu::QueryContextMenu\n"), this);	\
	return implclas::QueryContextMenu(p1, p2, p3, p4, p5);	\
}																				\

#define IMPLEMENT_IContextMenu_InvokeCommand(baseclas,implclas) \
STDMETHODIMP baseclas::InvokeCommand(LPCMINVOKECOMMANDINFO p) \
{																				\
	CMDTARGENTRYTR(_T(#baseclas)										\
		_T("(%p)::IContextMenu::InvokeCommand(%d)\n"),			\
		this, LOWORD(p->lpVerb));										\
	return implclas::InvokeCommand(p);								\
}																				\

#define IMPLEMENT_IContextMenu_GetCommandString(baseclas,implclas) \
STDMETHODIMP baseclas::GetCommandString(UINT_PTR nID,			\
	UINT p2,UINT* p3,LPSTR p4,UINT p5)								\
{																				\
	CMDTARGENTRYTR(_T(#baseclas)										\
		_T("(%p)::IContextMenu::GetCommandString(%u)\n"),		\
		this, (unsigned int) nID);										\
	return implclas::GetCommandString(nID,p2,p3,p4,p5);		\
}																				\

#define IMPLEMENT_IContextMenu(baseclas,implclas)				\
IMPLEMENT_IContextMenu_QueryContextMenu(baseclas,implclas)	\
IMPLEMENT_IContextMenu_InvokeCommand(baseclas,implclas)		\
IMPLEMENT_IContextMenu_GetCommandString(baseclas,implclas)	\

////////////////
// IContextMenu 
//
class DLLCLASS CTMfcContextMenu {
public:
	CMINVOKECOMMANDINFO m_cmi;		// filled after InvokeCommand
	CCmdTarget* m_pCmdTarget;		// MFC command target
	CMenu&		m_ctxMenu;			// context menu
	BOOL m_bMenuSeperator;			// add seperator as last item?
	BOOL m_bAutoMenuEnable;			// just like MFC frame wnd

	std::map<UINT,UINT> _mapMenuIDs;	// map our menu IDs to theirs -skst

	CTMfcContextMenu(CCmdTarget* pTarg, CMenu& menu);

	// helpers
	void InitMenuItem(CCmdTarget* pTarg, CMenu& menu, UINT nIndex);

	// you can override; default looks for resource string
	virtual CString OnGetMenuString(UINT nID);

	DECLARE_IContextMenu();
};

#define DECLARE_IObjectWithSite()									\
   STDMETHOD (SetSite) (IUnknown* pUnk);							\
   STDMETHOD (GetSite) (REFIID iid, void** ppv);				\

#define IMPLEMENT_IObjectWithSite_SetSite(baseclas,implclas) \
STDMETHODIMP baseclas::SetSite(IUnknown* pSite)					\
{																				\
	CMDTARGENTRYTR(_T(#baseclas)										\
		_T("(%p)::IObjectWithSite::SetSite(%p)\n"),				\
		this, pSite);														\
	return implclas::SetSite(pSite);									\
}																				\

#define IMPLEMENT_IObjectWithSite_GetSite(baseclas,implclas) \
STDMETHODIMP baseclas::GetSite(REFIID iid, LPVOID* ppv)		\
{																				\
	CMDTARGENTRYTR(_T(#baseclas)										\
		_T("(%p)::IObjectWithSite::GetSite\n"), this);			\
	return implclas::GetSite(iid, ppv);								\
}																				\

#define IMPLEMENT_IObjectWithSite(baseclas,implclas)			\
IMPLEMENT_IObjectWithSite_SetSite(baseclas,implclas)			\
IMPLEMENT_IObjectWithSite_GetSite(baseclas,implclas)			\

//////////////////
// Implement IObjectWithSite for class and interface ID
// eg., CTObjectWithSite<IInputObjectSite, &IID_IInputObjectSite>
//
template <class T, const IID* piid = &__uuidof(T)>
class CTObjectWithSite {
protected:
	CComQIPtr<T, piid> m_spSite;
public:
	CTObjectWithSite() { }
	virtual ~CTObjectWithSite() { }

	const CComQIPtr<T, piid>& GetSite() { return m_spSite; }
	
   STDMETHOD (SetSite) (IUnknown* pUnk) {
		m_spSite = pUnk;
		return (pUnk==NULL || m_spSite) ? S_OK : E_FAIL;
	}
   STDMETHOD (GetSite) (REFIID iid, void** ppv) {
		CTCHECKARG(ppv);
		*ppv = NULL;
		return m_spSite ? m_spSite->QueryInterface(iid, ppv) : E_FAIL;
	}
};

///////////////////
// MFC-style DLL (module) object
//
class DLLCLASS CTMfcModule : public COleControlModule, public CTModule {
public:
	DECLARE_DYNAMIC(CTMfcModule);
	CTMfcModule();
	virtual ~CTMfcModule();

	virtual BOOL InitInstance();
	virtual int  ExitInstance();

	// These correspond to the 4 special entry points.
	// Register/unregister are combined into one method w/BOOL arg.
	virtual HRESULT OnGetClassObject(REFCLSID, REFIID,LPVOID*);
	virtual HRESULT OnCanUnloadNow();
	virtual HRESULT OnRegisterServer(BOOL bRegister);
	virtual HRESULT OnInstall(BOOL bInstall, LPCWSTR pszCmdLine);
};

#endif // COMTOYS_H
