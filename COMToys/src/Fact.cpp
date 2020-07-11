////////////////////////////////////////////////////////////////
// ComToys(TM) Copyright 1999 Paul DiLascia
// If this code works, it was written by Paul DiLascia.
// If not, I don't know who wrote it.
//
#include "StdAfx.h"
#include "COMtoys/ComToys.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

DEBUG_BEGIN_INTERFACE_NAMES()
	DEBUG_INTERFACE_NAME(IClassFactory)
	DEBUG_INTERFACE_NAME(IClassFactory2)
DEBUG_END_INTERFACE_NAMES();

CPtrList	CTFactory::g_lsFactories;		// master list
CCriticalSection CTFactory::g_mydata;	// lock to protect it

CTFactory::CTFactory(REFCLSID clsid, UINT nResID, UINT tm) : m_clsid(clsid)
{
	CTTRACEFN(_T("CTFactory(%p)::CTFactory (%s)\n"), this, DbgName(clsid));
	ASSERT(TMNONE<=tm && tm<=TMBOTH);
	m_nThreadingModel = tm;
	m_nIDRes = nResID;
	m_bAutoDel = TRUE;

	// add to list of factories, but lock global data first
	CTLockData lock(g_mydata);
	g_lsFactories.AddHead(this);
}

//////////////////
// Destroy: remove myself from global list
//
CTFactory::~CTFactory()
{
	CTTRACEFN(_T("CTFactory(%p)::~CTFactory\n"), this);
	CTLockData lock(g_mydata);
	POSITION pos = g_lsFactories.Find(this);
	ASSERT(pos);
	g_lsFactories.RemoveAt(pos);
}

//////////////////
// Delete all factories in global list
//
void CTFactory::DeleteAll()
{
	CTLockData lock(g_mydata);
	while (!g_lsFactories.IsEmpty()) {
		CTFactory* pf = (CTFactory*)g_lsFactories.GetHead();
		ASSERT(pf);
		if (pf->m_bAutoDel)
			delete pf;
	}
	g_lsFactories.RemoveAll();
}

//////////////////
// Get factory from class ID
//
CTFactory* CTFactory::GetFactory(REFCLSID clsid)
{
	CTLockData lock(g_mydata);
	POSITION pos = g_lsFactories.GetHeadPosition();
	while (pos) {
		CTFactory* pf = (CTFactory*)g_lsFactories.GetNext(pos);
		if (pf->GetClassID()==clsid)
			return pf;
	}
	return NULL;
}

//////////////////
// This is like MFC: registter all the factories
//
BOOL CTFactory::OnRegisterAll(BOOL bRegister)
{
	CTLockData lock(g_mydata);
	POSITION pos = g_lsFactories.GetHeadPosition();
	while (pos) {
		CTFactory* pf = (CTFactory*)g_lsFactories.GetNext(pos);
		if (!pf->OnRegister(bRegister))
			return FALSE;
	}
	return TRUE;
}

//////////////////
// Standard registration fn looks for resource registry script
// with same resource ID as the factory
//
BOOL CTFactory::OnRegister(BOOL bRegister)
{
	CTTRACEFN(_T("CTFactory(%p)::OnRegister(%d)\n"), this, bRegister);

	static const LPOLESTR RT_REGISTRY = OLESTR("REGISTRY");

	UINT nID = GetResourceID();
	if (nID==0)
		return TRUE;

	HINSTANCE hInst = CTModule::GetModule()->GetInstanceHandle();
	if (!::FindResource(hInst, MAKEINTRESOURCE(nID), CString(RT_REGISTRY)))
		return FALSE;

	// initialize registry variables
	CTRegistrar iReg;
	OnInitRegistryVariables(iReg);

	// register/unregister script
	USES_CONVERSION;
	CTModuleName modname(hInst);
	LPCOLESTR lposModuleName = T2COLE(modname);
	HRESULT hr = bRegister ?
		iReg->ResourceRegister  (lposModuleName, nID, RT_REGISTRY) :
		iReg->ResourceUnregister(lposModuleName, nID, RT_REGISTRY);
	if (FAILED(hr)) {
		TRACE(_T("*** CTMfcFactory: un/register failed: %s\n"), _TR(hr));
		if (bRegister)
			return FALSE;
	}

	if (bRegister==FALSE) {
		// IRegistrar doesn't always delete top-level key right, so delete it
		CString sClsid;
		sClsid = StringFromCLSID(CTFactory::GetClassID());
		if (!sClsid.IsEmpty()) { // for extra-safety! don't delete CLSID !!
			CString sKey;
			sKey.Format(_T("CLSID\\%s"), (LPCTSTR)sClsid);
			CTDelRegTree(HKEY_CLASSES_ROOT, sKey);
		}
	}
	return TRUE;
}

//////////////////
// Initialize standard built-in variables
//
// %CLSID%		= class ID (GUID) (COleObjectFactory::m_clsid)
// %MODULE%	   = full pathname of DLL
// %Title%     = title (resource substring 0)
// %ClassName% = human-readable COM class name (resource substring 1)
// %ProgID%    = ProgID (resource substring 2)
// %ThreadingModel% = "Apartment", "Free", or "Both" (m_nThreadingModel)
//
// You can override to add your own in addition
//
BOOL CTFactory::OnInitRegistryVariables(IRegistrar* pReg)
{
	USES_CONVERSION;

	HINSTANCE hInst = CTModule::GetModule()->GetInstanceHandle();
	VERIFY(pReg->AddReplacement(OLESTR("CLSID"),
		StringFromCLSID(CTFactory::GetClassID()))==S_OK);

	VERIFY(pReg->AddReplacement(OLESTR("MODULE"),
		T2COLE(CTModuleName(hInst)))==S_OK);

	VERIFY(pReg->AddReplacement(OLESTR("Title"),
		T2COLE(GetTitle()))==S_OK);

	VERIFY(pReg->AddReplacement(OLESTR("ClassName"),
		T2COLE(GetClassName()))==S_OK);

	VERIFY(pReg->AddReplacement(OLESTR("ProgID"),
		T2COLE(GetClassName()))==S_OK);

	// threading model from m_nThreadingModel enum
	static LPOLESTR tmstrs[4] = {
		OLESTR(""), OLESTR("Apartment"), OLESTR("Free"), OLESTR("Both") };
	VERIFY(pReg->AddReplacement(OLESTR("ThreadingModel"),
		tmstrs[m_nThreadingModel])==S_OK);

	return TRUE;
}

//////////////// following copied from MFC appui2.cpp ////////////////
// Under Win32, a reg key may not be deleted unless it is empty.
// Thus, to delete a tree,  one must recursively enumerate and
// delete all of the sub-keys.
//
static LONG AFXAPI DelRegTreeHelper(HKEY hParentKey, const CString& strKeyName)
{
	TCHAR   szSubKeyName[256];
	HKEY    hCurrentKey;
	DWORD   dwResult;

	if (strKeyName.IsEmpty())
		return ERROR_BADKEY; // safety

	if ((dwResult = RegOpenKey(hParentKey, strKeyName, &hCurrentKey)) ==
		ERROR_SUCCESS) {
		// Remove all subkeys of the key to delete
		while ((dwResult = RegEnumKey(hCurrentKey, 0, szSubKeyName, 255)) ==
				ERROR_SUCCESS) {
			if ((dwResult = AfxDelRegTreeHelper(hCurrentKey, szSubKeyName)) != ERROR_SUCCESS)
				break;
		}
		// If all went well, we should now be able to delete the requested key
		if ((dwResult == ERROR_NO_MORE_ITEMS) || (dwResult == ERROR_BADKEY)) {
			dwResult = RegDeleteKey(hParentKey, strKeyName);
		}
	}
	RegCloseKey(hCurrentKey);
	return dwResult;
}

DLLFUNC LONG CTDelRegTree(HKEY hParentKey, const CString& strKeyName)
{
	return DelRegTreeHelper(hParentKey, strKeyName);
}



/////////////////////////////////////////////////////////////////////////////
// Default implementation for special CTFactory entry points
// For now, CTFactory isn't implemented. This class exists only to have
// a non-MFC IClassFactory some day.
//
STDMETHODIMP CTFactory::CreateInstance(LPUNKNOWN, REFIID, LPVOID*)
{
	return E_NOTIMPL;
}

STDMETHODIMP CTFactory::LockServer(BOOL)
{
	return E_NOTIMPL;
}

STDMETHODIMP CTFactory::GetLicInfo(LPLICINFO)
{
	return E_NOTIMPL;
}

STDMETHODIMP CTFactory::RequestLicKey(DWORD, BSTR*)
{
	return E_NOTIMPL;
}

STDMETHODIMP CTFactory::CreateInstanceLic(LPUNKNOWN,
	LPUNKNOWN, REFIID, BSTR, LPVOID*)
{
	return E_NOTIMPL;
}
