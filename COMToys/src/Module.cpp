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

CTModule* CTModule::g_pModule = NULL;

CTModule::CTModule()
{
	ASSERT(g_pModule==NULL); // only one allowed!
	g_pModule = this;
	m_dwLock = 0;
}

CTModule::~CTModule()
{
}

LONG CTModule::Lock()
{
	return InterlockedIncrement(&m_dwLock);
}

LONG CTModule::Unlock()
{
	return InterlockedDecrement(&m_dwLock);
}

/////////////////
// Currently, InitInstance and ExitInstance are *NOT* automatically
// called in DLL_PROCESS_ATTACH/DETACH, so you have to call them manually--
// but only if you're not using MFC. CTMfcModule::InitInstance automatically
// calls CTModule::InitInstance.
// 
BOOL CTModule::InitInstance(HINSTANCE hInstance)
{
	CTTRACEFN(_T(__FUNCTION__) _T("\n"));
	m_hInstance = hInstance;
	return TRUE;
}

int CTModule::ExitInstance()
{
	CTFactory::DeleteAll(); // delete all factories
	return 0;
}

/////////////////////////////////////////////////////////////////////////////
// Default implementation for special CTModule entry points
// For now, CTModule isn't implemented. This class exists only to have
// a non-MFC module class some day, and provide a base class for CTMfcModule.
//
HRESULT CTModule::OnGetClassObject(REFCLSID clsid, REFIID iid, LPVOID* ppv)
{
	CTTRACEFN(_T("CTModule::OnGetClassObject\n"));
	return CLASS_E_CLASSNOTAVAILABLE; // for now
}

HRESULT CTModule::OnCanUnloadNow(void)
{
	CTTRACEFN(_T("CTModule::OnCanUnloadNow\n"));
	return S_OK;
}

HRESULT CTModule::OnRegisterServer(BOOL bRegister)
{
	CTTRACEFN(_T("CTModule::OnRegisterServer\n"));
	return CTFactory::OnRegisterAll(bRegister) ? S_OK : SELFREG_E_CLASS;
}

HRESULT CTModule::OnInstall(BOOL bInstall, LPCWSTR pszCmdLine)
{
	CTTRACEFN(_T("CTModule::OnInstall\n"));
	return S_OK;
}
