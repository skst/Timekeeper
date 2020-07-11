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

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
	MFCENTRY;
	CTCHECKARG(ppv);
	CTTRACEFN(_T("DllGetClassObject(%s, %s)\n"),_TR(rclsid),_TR(riid));
	SCODE sc = CTModule::GetModule()->OnGetClassObject(rclsid, riid, ppv);
	CTTRACE(_T(" DllGetClassObject returns %s\n"), _TR(sc));
	return sc;
}

STDAPI DllCanUnloadNow(void)
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

STDAPI DllUnregisterServer(void)
{
	MFCENTRYTR("DllUnregisterServer\n");
	return CTModule::GetModule()->OnRegisterServer(FALSE);
}

STDAPI DllInstall(BOOL bInstall, LPCWSTR pszCmdLine)
{
	MFCENTRYTR("DllInstall\n");
	return CTModule::GetModule()->OnInstall(bInstall, pszCmdLine);
}

}
