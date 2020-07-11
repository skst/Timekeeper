//---------------------------------------------------------------------------
// (c) 2002-2010 12noon, Stefan K. S. Tucker
// (c) 1996-2001 Perpetual Motion Software, Stefan K. S. Tucker
//---------------------------------------------------------------------------

#include "stdafx.h"
#include <regstr.h>

#include "skstutil.h"
#include "mylog.h"
#include "Registry.h"

using skst::Registry;

/*
	Note that MFC's SetRegistryKey() and GetRegistryKey() automatically
	handle the HKCU path. We can't use them in MyWin, but we might
	want to do something like that.
*/
#if defined(_DEBUG)
	LPCTSTR const REGISTRY_SUBKEY = _T("Software\\12noon DEBUG\\");
#else
	LPCTSTR const REGISTRY_SUBKEY = _T("Software\\12noon\\");
#endif

void
Registry::CommonCtor()
{
   _hKey = nullptr;
   _dwKeyIndex = 0;
}

//--------------------------------------------------------------------
// This creates the appropriate entries under HKEY_CURRENT_USER for a
// new sound event.
//    HKEY_CURRENT_USER/
//       AppEvents/
//          EventLabels/
//             <event name> = <default: readable event name>
//             FireworksLow = "Low explosion"
//          Schemes/
//             Apps/
//                <app name>/ = <default: readable app name>
//                Fireworks = "Fireworks Screen Saver"
//                   <event name>/ = <default: readable event name>
//                      .current/ = <default: sound file>
//                   FireworksLow = "Low explosion"
//                      .current/ = "c:\resource\fish.wav"
//
// The key that is opened is the one that contains (or will contain)
// the user's choice for the event.
//--------------------------------------------------------------------
Registry::Registry(LPCTSTR szAppKeyName, LPCTSTR szAppName,
                   LPCTSTR szEventKeyName, LPCTSTR szEventName)
{
   CommonCtor();

   // "AppEvents\\EventLabels"
   tstring strSubKey(REGSTR_PATH_EVENTLABELS _T("\\"));
   strSubKey += szEventKeyName;
   CreateKey(HKEY_CURRENT_USER, strSubKey.c_str(), szEventName);
   CloseKey();

   // "AppEvents\\Schemes\\Apps"
   strSubKey = REGSTR_PATH_APPS _T("\\");
   strSubKey += szAppKeyName;
   CreateKey(HKEY_CURRENT_USER, strSubKey.c_str(), szAppName);
   CloseKey();

   strSubKey += _T("\\");
   strSubKey += szEventKeyName;
   CreateKey(HKEY_CURRENT_USER, strSubKey.c_str(), szEventName);
   CloseKey();

   strSubKey += _T("\\.current");
   CreateKey(HKEY_CURRENT_USER, strSubKey.c_str(), _T(""));
}


//---------------------------------------------------------------------------
/*
   This ctor is for settings that are specific to the application
   created by this company.  It does not create the key if it does not exist.
   The caller can use Exists() to determine if it does not exist.
   It opens the key for READ access only.
*/
//---------------------------------------------------------------------------
Registry::Registry(EType eType, LPCTSTR szSubKeyTrailer)
{
   // the subkey can be empty (e.g., company-wide settings)

   CommonCtor();

   HKEY hMainKey = GetKeyType(eType);
   if (hMainKey == nullptr)
      return;

   tstring strSubKey(REGISTRY_SUBKEY);
   strSubKey += szSubKeyTrailer;
   OpenKey(hMainKey, strSubKey.c_str());

   // if we can't open the key and we're supposed to try HKCU next, do it
   if (!Exists() && (eType == ETYPE_MACHINE_THEN_USER))
      OpenKey(HKEY_CURRENT_USER, strSubKey.c_str());
}

//---------------------------------------------------------------------------
/*
   This ctor is for any setting.
   It does not create the key if it does not exist.
   The caller can use Exists() to determine if it does not exist.
   It opens the key for READ access only.
*/
//---------------------------------------------------------------------------
Registry::Registry(HKEY hRoot, LPCTSTR szKeyPath)
{
   CommonCtor();

   OpenKey(hRoot, szKeyPath);
}

void
Registry::OpenKey(HKEY hMainKey, LPCTSTR szSubKey)
{
   //----------------------------
   // open existing key (if any)
   //----------------------------
   if (::RegOpenKeyEx(hMainKey, szSubKey, 0, KEY_READ, &_hKey) != ERROR_SUCCESS)
   {
      MyLog::GetLog().LogV(_T("ERROR: unable to open key '%s\\%s' for READ access."), (hMainKey == HKEY_LOCAL_MACHINE) ? _T("HKLM") : (hMainKey == HKEY_CURRENT_USER) ? _T("HKCU") : _T("other"), szSubKey);
      _hKey = nullptr;  // error or key doesn't exist
   }
}


//---------------------------------------------------------------------------
/*
   This ctor is for settings that are specific to the application
   created by this company.  It creates the key if it does not exist.
*/
//---------------------------------------------------------------------------
Registry::Registry(EType eType, LPCTSTR szSubKeyTrailer, LPCTSTR szDefaultValue)
{
   CommonCtor();

   HKEY hMainKey = GetKeyType(eType);
   if (hMainKey == nullptr)
      return;

   tstring strSubKey(REGISTRY_SUBKEY);
   strSubKey += szSubKeyTrailer;
   CreateKey(hMainKey, strSubKey.c_str(), szDefaultValue);

   // if we can't open the key and we're supposed to try HKCU next, do it
   if (!Exists() && (eType == ETYPE_MACHINE_THEN_USER))
      CreateKey(HKEY_CURRENT_USER, strSubKey.c_str(), szDefaultValue);
}


//---------------------------------------------------------------------------
/*
   This ctor is for any setting.
   It creates the key if it does not exist.
*/
//---------------------------------------------------------------------------
Registry::Registry(HKEY hRoot, LPCTSTR szKeyPath, LPCTSTR szDefaultValue)
{
   CommonCtor();

   CreateKey(hRoot, szKeyPath, szDefaultValue);
}


void
Registry::CreateKey(HKEY hMainKey, LPCTSTR szSubKey, LPCTSTR szDefaultValue)
{
   assert(_hKey == nullptr);

   //------------
   // create key
   //------------
   DWORD dwDisposition;
// TODO: what's the class param (should we set it?)
   TCHAR *szClass = _T("");
   if (::RegCreateKeyEx(hMainKey, szSubKey, 0,
                        szClass, REG_OPTION_NON_VOLATILE,
                        KEY_ALL_ACCESS, nullptr,
                        &_hKey, &dwDisposition) != ERROR_SUCCESS)
   {
      MyLog::GetLog().LogV(_T("ERROR: cannot create key '%s\\%s'."), (hMainKey == HKEY_LOCAL_MACHINE) ? _T("HKLM") : (hMainKey == HKEY_CURRENT_USER) ? _T("HKCU") : _T("other"), szSubKey);
      _hKey = nullptr;
      return;
   }


   //------------------------------------------------------------
   // if it's new and there's a default value, set default value
   //------------------------------------------------------------
   if ((dwDisposition == REG_CREATED_NEW_KEY) && (szDefaultValue[0] != _T('\0')))
   {
      ::RegSetValueEx(_hKey, nullptr, 0, REG_SZ,
                      (const BYTE *) szDefaultValue,
                      ((DWORD)_tcslen(szDefaultValue) + 1) * sizeof TCHAR);
   }
}


//------------------------------------------------------------------
// This routine determines the root of key with the specified type.
//------------------------------------------------------------------
HKEY
Registry::GetKeyType(EType eType)
const
{
   switch (eType)
   {
      case ETYPE_MACHINE_THEN_USER:
         return HKEY_LOCAL_MACHINE;

      case ETYPE_MACHINE:
         return HKEY_LOCAL_MACHINE;

      case ETYPE_USER:
         return HKEY_CURRENT_USER;

      default:
         MyLog::GetLog().LogV(_T("ERROR: unknown hkey type %d."), eType);
         assert(FALSE); // invalid key type
         return nullptr;
   }
}


Registry::~Registry()
{
   CloseKey();
}


void
Registry::CloseKey()
{
   if (_hKey != nullptr)
   {
      ::RegCloseKey(_hKey);
      _hKey = nullptr;
   }
}


void
Registry::DeleteSoundKey(LPCTSTR szAppKeyName, LPCTSTR szEventKeyName)
{
   // "AppEvents\\EventLabels\\<event key name>"
   tstring strSubKey(REGSTR_PATH_EVENTLABELS _T("\\"));
   strSubKey += szEventKeyName;
   Registry key1(HKEY_CURRENT_USER, strSubKey.c_str());
   key1.DeleteKey();

   // "AppEvents\\Schemes\\Apps\\<app key name>"
   // "AppEvents\\Schemes\\Apps\\<app key name>\\<event key name>"
   // "AppEvents\\Schemes\\Apps\\<app key name>\\<event key name>\\<schemes>"
   tstring strBase(REGSTR_PATH_APPS _T("\\"));
   strBase += szAppKeyName;
   Registry key2(HKEY_CURRENT_USER, strBase.c_str());
   key2.DeleteKey();
}


void
Registry::DeleteKey()
{
   if (!Exists())
      return;

   tstring strKey;
   EnumKeys(strKey);
   while (!strKey.empty())
   {
      Registry reg(_hKey, strKey.c_str());
      reg.DeleteKey();

      EnumKeys(strKey);
   }

   ::RegDeleteKey(_hKey, strKey.c_str());
}


void Registry::DeleteValue(LPCTSTR szValueName)
{
   const LONG l = ::RegDeleteValue(_hKey, szValueName);
   if (l == ERROR_ACCESS_DENIED)
      skst::MyLog::GetLog().LogV(_T(__FUNCTION__) _T(": Unable to delete key '%s'"), szValueName);
   assert(l == ERROR_SUCCESS);
}


void
Registry::SetString(LPCTSTR szValueName, LPCTSTR szValue)
const
{
   assert(_hKey != nullptr);

   if (_hKey == nullptr)
      return;

   if (::RegSetValueEx(_hKey, szValueName, 0, REG_SZ,
                       (const BYTE *) szValue,
                       ((DWORD)_tcslen(szValue) + 1) * sizeof TCHAR) != ERROR_SUCCESS)
   {
      MyLog::GetLog().LogV(_T("ERROR: cannot set registry value name %s to value %s."), szValueName, szValue);
   }
}


tstring Registry::GetString(LPCTSTR szValueName, LPCTSTR szDefaultValue) const
{
   assert(_hKey != nullptr);

   if (_hKey == nullptr)
      return szDefaultValue;

   TCHAR szValue[MAX_PATH];
   DWORD dwType, dwSize = sizeof szValue;

   if (::RegQueryValueEx(_hKey, szValueName, nullptr, &dwType,
                         (BYTE *) szValue, &dwSize) == ERROR_SUCCESS)
   {
      assert(dwType == REG_SZ);
      return szValue;
   }

   // TODO: we could handle ERROR_MORE_DATA, but we don't bother

   return szDefaultValue;
}


void
Registry::SetDWORD(LPCTSTR szValueName, DWORD dwValue)
const
{
   assert(_hKey != nullptr);

   if (_hKey == nullptr)
      return;

   if (::RegSetValueEx(_hKey, szValueName, 0, REG_DWORD,
                       (BYTE *) &dwValue, sizeof dwValue) != ERROR_SUCCESS)
   {
      MyLog::GetLog().LogV(_T("ERROR: cannot set registry value name %s to value %ld."), szValueName, dwValue);
   }
}

DWORD
Registry::GetDWORD(LPCTSTR szValueName, DWORD dwDefaultValue)
const
{
   assert(_hKey != nullptr);

   if (_hKey == nullptr)
      return dwDefaultValue;

   DWORD dwValue;
   DWORD dwType, dwSize = sizeof dwValue;

   if (::RegQueryValueEx(_hKey, szValueName, 0, &dwType,
                         (BYTE *) &dwValue, &dwSize) == ERROR_SUCCESS)
   {
      assert(dwType == REG_DWORD);
      return dwValue;
   }

   return dwDefaultValue;
}


void
Registry::SetBinary(LPCTSTR szValueName, const std::vector<BYTE>& vData)
const
{
   assert(_hKey != nullptr);

   if (_hKey == nullptr)
      return;

   if (::RegSetValueEx(_hKey, szValueName, 0, REG_BINARY,
                       &vData[0], (DWORD)vData.size()) != ERROR_SUCCESS)
   {
      MyLog::GetLog().LogV(_T("ERROR: cannot set registry value name %s to binary value."), szValueName);
   }
}


void
Registry::GetBinary(LPCTSTR szValueName, std::vector<BYTE>& vData)
const
{
   assert(_hKey != nullptr);

   vData.clear();

   if (_hKey == nullptr)
      return;

   DWORD dwType, dwSize;

   // get size of binary data (if any)
   if (::RegQueryValueEx(_hKey, szValueName, 0, &dwType, nullptr, &dwSize) != ERROR_SUCCESS)
      return;

   assert(dwType == REG_BINARY);
   assert(dwSize > 0);

   // get actual binary data
   vData.resize(dwSize);   // for performance, pre-allocate data
   if (::RegQueryValueEx(_hKey, szValueName, 0, &dwType,
                         &(vData[0]), &dwSize) != ERROR_SUCCESS)
   {
      MyLog::GetLog().Log(_T("Error reading byte array from Registry."));
      vData.clear();
      return;
   }

   assert(dwType == REG_BINARY);
   assert(dwSize > 0);
}


//---------------------------------------------------------------------------
/*
   This routine returns the name of the next key.
   It returns an empty string if there are no more keys to retrieve.
*/
//---------------------------------------------------------------------------
void Registry::EnumKeys(tstring& strName)
{
   assert(_hKey != nullptr);

   strName.erase();

   if (_hKey == nullptr)
      return;

   DWORD dwMaxSubKeyLen;
   ::RegQueryInfoKey(_hKey, nullptr, nullptr, nullptr, nullptr, &dwMaxSubKeyLen, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr);

   ++dwMaxSubKeyLen;    // include terminating null
   std::auto_ptr<TCHAR> szName(new TCHAR[dwMaxSubKeyLen]);
   const LONG l = ::RegEnumKeyEx(_hKey, _dwKeyIndex,
                                 szName.get(),						// key name
                                 &dwMaxSubKeyLen,					// buf size
                                 nullptr,
                                 nullptr, nullptr, nullptr);	// don't get class
   strName = szName.get();

   if (l != ERROR_SUCCESS)
   {
      if (l != ERROR_NO_MORE_ITEMS)
         MyLog::GetLog().Log(_T("Error enumerating keys."));

      _dwKeyIndex = 0;
      strName.erase();
      return;
   }

   ++_dwKeyIndex;
}

