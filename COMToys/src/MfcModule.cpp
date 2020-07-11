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

IMPLEMENT_DYNAMIC(CTMfcModule, COleControlModule);

CTMfcModule::CTMfcModule()
{
}

CTMfcModule::~CTMfcModule()
{
}

BOOL CTMfcModule::InitInstance()
{
	CTTRACEFN(_T(__FUNCTION__) _T("\n"));

   //skst
   /*
      CTRegistrar uses IRegistrar--a completely undocumented ATL interface.
      What's also undocumented is that it requires atl.dll, which is not
      present on Windows 98 with IE 5.0. It is present after we install
      IE 6.0. Who knows about IE 5.5?
      Thanks, Microsoft.
   */
CTTRACE(_T("\tbefore CTRegistrar ctor\n"));
   CTRegistrar r;
CTTRACE(_T("\tafter CTRegistrar ctor\n"));
   if (!r)
   {
      ::AfxMessageBox(_T("This requires Microsoft Internet Explorer 6.0."));
      return FALSE;
   }
   //skst

	if (!CTModule::InitInstance(CWinApp::m_hInstance))
		return FALSE;
	return COleControlModule::InitInstance();
}

int CTMfcModule::ExitInstance()
{
	CTTRACEFN(_T("CTMfcModule::ExitInstance\n"));
	CTModule::ExitInstance();
	return COleControlModule::ExitInstance();
}

/////////////////////////////////////////////////////////////////////////////
// Implementation special DLL entry points.
// Call MFC to do the work.

HRESULT CTMfcModule::OnGetClassObject(REFCLSID clsid, REFIID iid, LPVOID* ppv)
{
	CTTRACEFN(_T("CTMfcModule::OnGetClassObject\n"));
	HRESULT hr = AfxDllGetClassObject(clsid, iid, ppv);
	if (*ppv==NULL) {
		TRACE(_T("***CTMfcModule::OnGetClassObject failed\n"));
		TRACE(_T("***Did you create factories before calling CTMfcModule::InitInstance?\n"));
	}
	return hr;
}

HRESULT CTMfcModule::OnCanUnloadNow(void)
{
	CTTRACEFN(_T("CTMfcModule::OnCanUnloadNow\n"));
	return AfxDllCanUnloadNow();
}

HRESULT CTMfcModule::OnRegisterServer(BOOL bRegister)
{
	CTTRACEFN(_T("CTMfcModule::OnRegisterServer\n"));
	HRESULT hr =CTModule::OnRegisterServer(bRegister);
	if (hr!=S_OK)
		return hr;
	return COleObjectFactory::UpdateRegistryAll(bRegister)
		? S_OK : SELFREG_E_CLASS;
}

HRESULT CTMfcModule::OnInstall(BOOL bInstall, LPCWSTR pszCmdLine)
{
	CTTRACEFN(_T("CTMfcModule::OnInstall\n"));
	return CTModule::OnInstall(bInstall, pszCmdLine);
}
