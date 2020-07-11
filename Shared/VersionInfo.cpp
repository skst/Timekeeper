//---------------------------------------------------------------------------
// (c) 2003-2010 12noon, Stefan K. S. Tucker
// (c) 1994-2002 Perpetual Motion Software, Stefan K. S. Tucker
//---------------------------------------------------------------------------

#include "stdafx.h"
#include <winver.h>
#include <strsafe.h>
#pragma comment(lib, "version.lib")

#include "skstutil.h"
#include "misc.h"
#include "VersionInfo.h"

using skst::VersionInfo;

VersionInfo::VersionInfo(const HINSTANCE hInst)
{
   //-----------------
   // Get path to app
   //-----------------
   TCHAR s[_MAX_PATH];
	::GetModuleFileName(hInst, s, _MAX_PATH);

   // determine size of app's version information
   DWORD dwZero;
   const DWORD cbVerSize = ::GetFileVersionInfoSize(s, &dwZero);
   if (cbVerSize == 0)
   {
      _pVerInfo = nullptr;
      ::OutputDebugString(skst::MyFormatV(_T("Unable to retrieve size of version information from %s.\n"), s).c_str());
      return;
   }

   // retrieve the app's version information
   _pVerInfo = new BYTE [cbVerSize];
   assert(_pVerInfo != nullptr);
   if (!::GetFileVersionInfo(s, dwZero, cbVerSize, _pVerInfo))
   {
      ::OutputDebugString(_T("Unable to retrieve version information.\n"));
      return;
   }

   // get fixed file version info
   UINT uiInfoSize;                 // size of requested info
   if (!::VerQueryValue(_pVerInfo, _T("\\"),
                        (VOID **) &_pFileInfo, &uiInfoSize))
   {
      ::OutputDebugString(_T("Unable to retrieve fixed file information.\n"));
   }

   // get the language type
   if (!::VerQueryValue(_pVerInfo, _T("\\VarFileInfo\\Translation"),
                        (VOID **) &_ptLangInfo, &uiInfoSize))
   {
      ::OutputDebugString(_T("Unable to retrieve language information.\n"));
   }
}


VersionInfo::~VersionInfo()
{
   if (_pVerInfo != nullptr)
   {
      delete [] _pVerInfo;
      _pVerInfo = nullptr;
   }
}


//---------------------------------------------------------------------------
// This routine returns a pointer to the value of the passed key.
// It simply sets the second parameter to the address of a LPSTR;
// because the function only sets the LPSTR to point into the version
// info block, there is no need to allocate storage for a string.
// The string pointer is set to NULL if there is no version information
// block or there was an error in retrieving it (see ctor).
//---------------------------------------------------------------------------
void
VersionInfo::GetInfo(PCTSTR szKey,
                     PCTSTR *pszValue)
const
{
   if (_pVerInfo == nullptr)     // if no ver info block (or error)
   {
      *pszValue = nullptr;
      return;
   }

   // format language information
   TCHAR s[200];
	// http://msdn.microsoft.com/en-us/library/ms647464(VS.85).aspx
	StringCchPrintf(s, _countof(s), _T("\\StringFileInfo\\%04x%04x\\%s"),
							_ptLangInfo->wLang, _ptLangInfo->wCharSet, szKey);

   UINT uiInfoSize;           // size of requested info
   ::VerQueryValue(_pVerInfo, s, (VOID **) pszValue, &uiInfoSize);
}
