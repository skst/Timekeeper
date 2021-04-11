////////////////////////////////////////////////////////////////
// ComToys(TM) Copyright 1999 Paul DiLascia
// If this code works, it was written by Paul DiLascia.
// If not, I don't know who wrote it.
//

/////////////////////////////////////////////////////////////////////////////
// Special entry points required for inproc servers. Implementation passes
// control to equivalent DLL object methods, so you can override behavior.
// You should include this file in your main module .cpp file (the one that
// implements your CWinApp-derived class.)

extern "C"
{

_Check_return_
STDAPI DllGetClassObject(_In_ REFCLSID rclsid, _In_ REFIID riid, _Outptr_ LPVOID FAR* ppv)
{
	MFCENTRY;
	CTCHECKARG(ppv);
	CTTRACEFN(_T("DllGetClassObject(%s, %s)\n"),_TR(rclsid),_TR(riid));
	SCODE sc = CTModule::GetModule()->OnGetClassObject(rclsid, riid, ppv);
	CTTRACE(_T(" DllGetClassObject returns %s\n"), _TR(sc));
	return sc;
}

__control_entrypoint(DllExport)
STDAPI DllCanUnloadNow()
{
	MFCENTRYTR("DllCanUnloadNow\n");
	HRESULT hr = CTModule::GetModule()->OnCanUnloadNow();
	CTTRACE(">DllCanUnloadNow returns %s\n",_TR(hr));
	return hr;
}

STDAPI DllRegisterServer()
{
	MFCENTRYTR("DllRegisterServer\n");
	return CTModule::GetModule()->OnRegisterServer(TRUE);
}

STDAPI DllUnregisterServer()
{
	MFCENTRYTR("DllUnregisterServer\n");
	return CTModule::GetModule()->OnRegisterServer(FALSE);
}

STDAPI DllInstall(BOOL bInstall, _In_opt_ LPCWSTR pszCmdLine)
{
	MFCENTRYTR("DllInstall\n");
	return CTModule::GetModule()->OnInstall(bInstall, pszCmdLine);
}

}
