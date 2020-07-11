////////////////////////////////////////////////////////////////
// Copyright 1998 Paul DiLascia
// If this program works, it was written by Paul DiLascia.
// If not, I don't know who wrote it.
//
#include "StdAfx.h"
#include "COMtoys/IniFile.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

void CIniFile::Use(CWinApp *pApp, const INI_LOCATION where, LPCTSTR szProfileName)
{
	TCHAR buf[_MAX_PATH];
   // For some reason, this gets the 8.3 path, even though MS's doc says that only happens on 9x/Me.
   // Thanks, Microsoft.
	VERIFY(::GetModuleFileName(pApp->m_hInstance, buf, sizeof buf));
	LPTSTR ext = _tcsrchr(buf, _T('.'));   // points to ".exe"
	ASSERT(ext);
   _tcscpy_s(ext, ::_tcslen(ext) + 1, _T(".INI"));	// now .INI

	LPTSTR name = _tcsrchr(buf, _T('\\'));	// points to "foo.ini"
	VERIFY(name++);
	if (szProfileName != NULL)
      _tcscpy_s(name, ::_tcslen(name) + 1, szProfileName);

	szProfileName = (where == LocalDir) ? buf : name;

	// set the name
	free((void*)pApp->m_pszProfileName);
	pApp->m_pszProfileName = _tcsdup(szProfileName);
	free((void*)pApp->m_pszRegistryKey);
	pApp->m_pszRegistryKey = NULL;
}
