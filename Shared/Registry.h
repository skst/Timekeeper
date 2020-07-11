//---------------------------------------------------------------------------
// (c) 2003-2009 12noon, Stefan K. S. Tucker
// (c) 2001-2002 Perpetual Motion Software, Stefan K. S. Tucker
//---------------------------------------------------------------------------

#pragma once

#include <string>
#include <vector>

#include "skstutil.h"

namespace skst
{

class Registry
{
   protected:
      HKEY  _hKey;
      DWORD _dwKeyIndex;   // enum index for keys

   public:
      enum EType
      {
         ETYPE_USER,                         // use HKCU hive
         ETYPE_MACHINE,                      // use HKLM hive
         ETYPE_MACHINE_THEN_USER,            // try HKLM hive, if fail, try HKCU
      };

      // ctor for sound events
      Registry(LPCTSTR szAppKeyName,      // key name in registry
               LPCTSTR szAppName,         // readable name in Control Panel
               LPCTSTR szEventKeyName,    // key name in registry
               LPCTSTR szEventName);      // readable name in Control Panel

      // ctor for company settings - open key only if it exists
      Registry(EType eType,
               LPCTSTR szSubKeyTrailer);  // e.g., "<appname>\\Options"

      // ctor for company settings - open key and create if necessary
      Registry(EType eType,
               LPCTSTR szSubKeyTrailer,   // e.g., "<appname>\\Options"
               LPCTSTR szDefaultValue);   // can be empty

      // ctor for any path - open key only if it exists
      Registry(HKEY hRoot, LPCTSTR szKeyPath);

      // ctor for any path - open key and create if necessary
      Registry(HKEY hRoot, LPCTSTR szKeyPath, LPCTSTR szDefaultValue);

      virtual ~Registry();

      bool     Exists() const { return (_hKey != nullptr); }

      static void DeleteSoundKey(LPCTSTR szAppKeyName,
                                 LPCTSTR szEventKeyName);
      void     DeleteKey();

      void     DeleteValue(LPCTSTR szValueName);

      void        SetString(LPCTSTR szValueName, LPCTSTR szValue) const;
      tstring		GetString(LPCTSTR szValueName, LPCTSTR szDefaultValue) const;

      void     SetDWORD(LPCTSTR szValueName, DWORD dwValue) const;
      DWORD    GetDWORD(LPCTSTR szValueName, DWORD dwDefaultValue) const;

      void     SetBoolean(LPCTSTR szValueName, bool bValue) const
                  { SetDWORD(szValueName, DWORD(bValue ? 1 : 0)); }
      bool     GetBoolean(LPCTSTR szValueName, bool bDefaultValue) const
                  { return GetDWORD(szValueName, DWORD(bDefaultValue ? 1 : 0)) ? true : false; }

      void     SetBOOL(LPCTSTR szValueName, BOOL bValue) const
                  { SetDWORD(szValueName, DWORD(bValue)); }
      BOOL     GetBOOL(LPCTSTR szValueName, BOOL bDefaultValue) const
                  { return GetDWORD(szValueName, DWORD(bDefaultValue)); }

      void     SetBinary(LPCTSTR szValueName, const std::vector<BYTE>& vData) const;
      void     GetBinary(LPCTSTR szValueName, std::vector<BYTE>& vData) const;

      void     EnumKeys(tstring& strName);

   protected:
      void  CommonCtor();
      HKEY  GetKeyType(EType eType) const;
      void  OpenKey(HKEY hMainKey, LPCTSTR szSubKey);
      void  CreateKey(HKEY hMainKey, LPCTSTR szSubKey, LPCTSTR szDefaultValue);
      void  CloseKey();
};

}

