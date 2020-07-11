//---------------------------------------------------------------------------
// (c) 2003-2014 12noon, Stefan K. S. Tucker
// (c) 1994-2002 Perpetual Motion Software, Stefan K. S. Tucker
//---------------------------------------------------------------------------

#pragma once

#include "skstutil.h"

namespace skst
{

class VersionInfo
{
protected:
   BYTE *_pVerInfo;              // ptr to ver info (allocated)
                                 // NULL if no ver info block (or error)
   struct TLangCharSet
   {
      WORD wLang, wCharSet;
   } *_ptLangInfo;               // ptr to language info block

   VS_FIXEDFILEINFO *_pFileInfo; // file's version info

public:
   VersionInfo(const HINSTANCE hInst);
   virtual ~VersionInfo();

   void GetFileVersion(DWORD& dwMS, DWORD& dwLS) const
   {
      dwMS = _pFileInfo->dwFileVersionMS;
      dwLS = _pFileInfo->dwFileVersionLS;
   }

   std::wstring GetProductName() const { return GetInfo(_T("ProductName")); }
   std::wstring GetProductVersion() const { return GetInfo(_T("ProductVersion")); }
   std::wstring GetFileVersion() const { return GetInfo(_T("FileVersion")); }
   std::wstring GetCompanyName() const { return GetInfo(_T("CompanyName")); }
   std::wstring GetCopyright() const { return GetInfo(_T("LegalCopyright")); }

	std::wstring GetInfo(LPCTSTR szKey) const
   {
      LPCTSTR szValue;
      GetInfo(szKey, &szValue);
		return (szValue == nullptr) ? std::wstring() : szValue;
   }

   //-----------------------------------------------------------------
   // The second parameter is simply the address of a string; because
   // the function merely sets the ptr to point into the version
   // info block, there is no need to allocate storage for a string.
   //-----------------------------------------------------------------
   void  GetInfo(LPCTSTR szKey, LPCTSTR *pszValue) const;
};

}
