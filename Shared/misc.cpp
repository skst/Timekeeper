//---------------------------------------------------------------------------
// (c) 2002-2015 12noon, Stefan K. S. Tucker
// (c) 2001-2002 Perpetual Motion Software, Stefan K. S. Tucker
//---------------------------------------------------------------------------

#include "stdafx.h"
#include <uxtheme.h>
#pragma comment(lib, "uxtheme.lib")
#include <strsafe.h>

#include "skstutil.h"
#include "internal.h"
#include "mylog.h"
#include "misc.h"

using skst::MyLog;


/*
	Load the small version of the icon with the passed id.
*/
HICON skst::LoadIconSmall(const HINSTANCE hInst, const UINT id)
{
	/*
      If a visual style is enabled, the icon doesn't look right if it
      maps the colors, so we only do it if there's no visual style.
   */
   UINT uiFlags = LR_SHARED;
   if (!::IsThemeActive())
      uiFlags |= LR_LOADMAP3DCOLORS;

	return (HICON) ::LoadImage(hInst,
										MAKEINTRESOURCE(id),
										IMAGE_ICON,
										::GetSystemMetrics(SM_CXSMICON),
										::GetSystemMetrics(SM_CYSMICON),
										uiFlags);
}


/*
	Note: Use AfxGetFileTitle() instead of writing GetFileName().
*/

/*
   boring code to get pieces of module path
*/
// return "c:\...\directory\alarm.exe"
tstring skst::GetAppPath(const HINSTANCE hInst)
{
   TCHAR sz[_MAX_PATH];
   ::GetModuleFileName(hInst, sz, _countof(sz));
   return sz;
}

// return "c:\...\directory\"
tstring skst::GetAppPathOnly(const HINSTANCE hInst)
{
   tstring s(skst::GetAppPath(hInst));
   const size_t iPos = s.find_last_of(_T('\\'));
   assert(iPos != -1);
   return s.substr(0, iPos + 1);
}

// return "alarm.exe"
tstring skst::GetAppOnly(const HINSTANCE hInst)
{
   tstring s(skst::GetAppPath(hInst));
   const size_t iPos = s.find_last_of(_T('\\'));
   assert(iPos != -1);
   return s.substr(iPos + 1);
}

// same as: return GetAppPathOnly() + ::AfxGetAppName() + _T(".chm");
tstring skst::GetFileNameWithNewExtension(const HINSTANCE hInst, LPCTSTR szExt)
{
   if (szExt == nullptr)
      return _T("");

   tstring strModule(skst::GetAppPath(hInst));

   // use the exe path and name to load the options
   TCHAR szDrive[_MAX_DRIVE];
   TCHAR szDir[_MAX_DIR];
   TCHAR szFilename[_MAX_FNAME];
   _tsplitpath_s(strModule.c_str(), szDrive, _countof(szDrive), szDir, _countof(szDir), szFilename, _countof(szFilename), nullptr, 0);

   TCHAR szPath[_MAX_PATH];
   _tmakepath_s(szPath, _countof(szPath), szDrive, szDir, szFilename, szExt);

   return szPath;
}


/*
   Get path to this file in the same directory as the current module.
*/
tstring skst::GetLocalPath(const HINSTANCE hInst, LPCTSTR szFilename)
{
   TCHAR szPath[_MAX_PATH];
   ::GetModuleFileName(hInst, szPath, _countof(szPath));

   TCHAR szDrive[_MAX_DRIVE];
   TCHAR szDir[_MAX_DIR];
//   TCHAR szFilename[_MAX_FNAME];
//   TCHAR szExt[_MAX_EXT];
//   ::_splitpath(szPath, szDrive, szDir, szFilename, szExt);
   _tsplitpath_s(szPath, szDrive, _countof(szDrive), szDir, _countof(szDir), nullptr, 0, nullptr, 0);

   _tmakepath_s(szPath, _countof(szPath), szDrive, szDir, szFilename, nullptr);

   return szPath;
}


/*
   This routine returns a string to represent a system error.
*/
tstring skst::GetLastErrorString()
{
   HLOCAL hBuf;
   ::FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER, nullptr,
                  ::GetLastError(), 0, (LPTSTR) &hBuf, 1, nullptr);
   LPTSTR szBuf = (LPTSTR) ::LocalLock(hBuf);
   tstring strBuf(szBuf);
   ::LocalUnlock(hBuf);
   ::LocalFree(hBuf);
   return strBuf;
}

/*
   This routine returns a string to represent a common dialog extended error.
*/
tstring skst::GetCommDlgExtendedErrorString()
{
   switch (::CommDlgExtendedError())
   {
      case 0:                       return _T("No error");
      case CDERR_DIALOGFAILURE:     return _T("CDERR_DIALOGFAILURE");
      case CDERR_FINDRESFAILURE:    return _T("CDERR_FINDRESFAILURE");
      case CDERR_INITIALIZATION:    return _T("CDERR_INITIALIZATION");
      case CDERR_LOADRESFAILURE:    return _T("CDERR_LOADRESFAILURE");
      case CDERR_LOADSTRFAILURE:    return _T("CDERR_LOADSTRFAILURE");
      case CDERR_LOCKRESFAILURE:    return _T("CDERR_LOCKRESFAILURE");
      case CDERR_MEMALLOCFAILURE:   return _T("CDERR_MEMALLOCFAILURE");
      case CDERR_MEMLOCKFAILURE:    return _T("CDERR_MEMLOCKFAILURE");
      case CDERR_NOHINSTANCE:       return _T("CDERR_NOHINSTANCE");
      case CDERR_NOHOOK:            return _T("CDERR_NOHOOK");
      case CDERR_NOTEMPLATE:        return _T("CDERR_NOTEMPLATE");
      case CDERR_REGISTERMSGFAIL:   return _T("CDERR_REGISTERMSGFAIL");
      case CDERR_STRUCTSIZE:        return _T("CDERR_STRUCTSIZE");
      case FNERR_BUFFERTOOSMALL:    return _T("FNERR_BUFFERTOOSMALL");
      case FNERR_SUBCLASSFAILURE:   return _T("FNERR_SUBCLASSFAILURE");
      default:                      return _T("UNKNOWN");
   }
}


//---------------------------------------------------------------------------
/*
   This routine returns the specified locale information.
   See the "Locale Constants (LCTYPE)" section in the help.
*/
//---------------------------------------------------------------------------
tstring skst::GetLocaleInfoString(LCTYPE lcType)
{
   // get length of string
   const int iLen = ::GetLocaleInfo(LOCALE_USER_DEFAULT, lcType, nullptr, 0);

   // get string
   std::auto_ptr<TCHAR> sz(new TCHAR[iLen + 1]);
   ::GetLocaleInfo(LOCALE_USER_DEFAULT, lcType, sz.get(), iLen);

   return sz.get();
}


//---------------------------------------------------------------------------
/*
   This routine returns the path of the n-th file in the passed drop
   information block.  The caller must call DragFinish().
*/
//---------------------------------------------------------------------------
tstring skst::GetDroppedFilePath(HDROP h, UINT ix)
{
   UINT nNumFiles = ::DragQueryFile(h, UINT(-1), nullptr, 0);
   if (ix >= nNumFiles)
      return _T("");

	TCHAR szFileName[_MAX_PATH];
	::DragQueryFile(h, ix /*file # to get*/, szFileName, _countof(szFileName));

   return szFileName;
}


/*
   This function is shorthand for calling SHGetFolderPath().
	We used to have to call GetProcAddress().
*/
tstring skst::MyGetFolderPath(const int iType)
{
// TODO: Use SHGetKnownFolderPath() in Vista and later.

	TCHAR szStartup[_MAX_PATH];
	::SHGetFolderPath(nullptr, iType, nullptr, SHGFP_TYPE_CURRENT, szStartup);

   return szStartup;
}


tstring skst::IntToString(const int i)
{
   TCHAR sz[20];
   _stprintf_s(sz, _countof(sz), _T("%d"), i);
   return sz;
}


/*
   Convert ASCII string to BSTR

   caller must free returned BSTR
*/
BSTR skst::ASCIItoBSTR(LPCSTR str)
{
   // returns # chars needed in buffer, including null
   const int nChars = ::MultiByteToWideChar(CP_ACP, 0, str, -1, nullptr, 0);

   std::auto_ptr<WCHAR> wstr(new WCHAR[nChars]);
   ::MultiByteToWideChar(CP_ACP, 0, str, -1, wstr.get(), nChars);

   return ::SysAllocString(wstr.get());
}


std::string skst::BSTRtoASCII(const BSTR bstr)
{
   // returns # chars needed in buffer, including null
   const int nChars = ::WideCharToMultiByte(CP_ACP, 0, bstr, -1, nullptr, 0, nullptr, nullptr);

	std::auto_ptr<char> str(new char[nChars]);
   ::WideCharToMultiByte(CP_ACP, 0, bstr, -1, str.get(), nChars, nullptr, nullptr);

	return str.get();
}


/*
   Copy text from one place to another. Normally boring,
   but this adds an ellipsis to the end of text if it's too long.
*/
void skst::AddEllipsis(LPTSTR szDest, LPCTSTR szSrc, const int nchSize)
{
   LPCTSTR szEllipsis = _T("...");

// This works too, but it takes two steps.
//   _tcsncpy(szDest, szSrc, iSize);
//   szDest[iSize] = _T('\0');
// This is insecure.
//   lstrcpyn(szDest, szSrc, nchSize);
	StringCchCopyN(szDest, nchSize, szSrc, nchSize);

	// if the source string is not longer than the maximum length
	size_t nchLenthActual;
	if (FAILED(StringCchLength(szSrc, nchSize, &nchLenthActual)))	// if len is greater than max, append ellipsis
      StringCchCopy(szDest + nchSize - 1 - lstrlen(szEllipsis), lstrlen(szEllipsis), szEllipsis);
}


/*
   This routine is identical to _vsprintf() except it returns a string.
*/
tstring skst::MyFormatV(LPCTSTR szFormat, ...)
{
	va_list argList;
	va_start(argList, szFormat);

   const tstring str(::_MyFormatArgList(szFormat, argList).c_str());

   va_end(argList);

   return str;
}


/*
   for internal use only :-)
*/
tstring _MyFormatArgList(LPCTSTR szFormat, const va_list& argList)
{
   TCHAR szFormatted[1000];
   assert(_tcslen(szFormat) < _countof(szFormatted));
   _vstprintf_s(szFormatted, _countof(szFormatted), szFormat, argList);
   return szFormatted;
}


/*
   This function is based on the January 1996 Win32 MSJ article
   by Jeffrey Richter.

   It creates and spawns a batch file which keeps trying to delete
   the executable that spawned it. (It can't delete it until the
   exe stops running.) When the exe is gone, it deletes the
   subdirectory and itself (the batch file).
*/
void skst::DeleteSelf()
{
   /*
      write batch file to delete self
   */

   // get tmp batch file name
   TCHAR szTmpDir[_MAX_PATH];
   ::GetTempPath(_countof(szTmpDir), szTmpDir);
   TCHAR szUnique[_MAX_PATH];
   ::GetTempFileName(szTmpDir, _T("del"), 0, szUnique);

   // generate path with extension of ".tmp" instead of ".bat"
   TCHAR szBatchPath[_MAX_PATH];
   _tcscpy_s(szBatchPath, _countof(szBatchPath), szUnique);
   _tcscpy_s(szBatchPath + _tcslen(szBatchPath) - 3, _countof(szBatchPath) - (_tcslen(szBatchPath) - 3), _T("bat"));

   // since GetTempFileName() created the unique file, we rename it
   ::MoveFile(szUnique, szBatchPath);

   // open batch file
   MyLog::GetLog().LogV(_T("Creating batch file %s."), szBatchPath);
   HANDLE hFile = ::CreateFile(szBatchPath, GENERIC_WRITE, 0, nullptr /*sec*/, CREATE_ALWAYS,
                               FILE_ATTRIBUTE_TEMPORARY | FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, nullptr);
   if (hFile == INVALID_HANDLE_VALUE)
   {
      MyLog::GetLog().LogV(_T("Error creating batch file %s."), szBatchPath);
      return;
   }

   // build the batch file
   TCHAR szBatch[1000];
   _stprintf_s(szBatch, _countof(szBatch),
               _T(":Repeat\r\n")
               _T("del \"%s\"\r\n")
               _T("if exist \"%s\" goto :Repeat\r\n")
               _T("rd \"%s\"\r\n")
               _T("del \"%s\"\r\n"), 
               skst::GetAppPath(nullptr).c_str(),
               skst::GetAppPath(nullptr).c_str(),
               skst::GetAppPathOnly(nullptr).c_str(),
               szBatchPath);

   DWORD dwBytesWritten;
   ::WriteFile(hFile, szBatch, (DWORD)_tcslen(szBatch) * sizeof TCHAR, &dwBytesWritten, nullptr);
   ::CloseHandle(hFile);

   /*
      spawn batch file at low priority and set this app to high priority
   */
   STARTUPINFO si;
   ::ZeroMemory(&si, sizeof(si));
   si.cb = sizeof si;

   // hide console window
   si.dwFlags = STARTF_USESHOWWINDOW;
   si.wShowWindow = SW_HIDE;

   // spawn the batch file with low-priority and suspended.
   PROCESS_INFORMATION pi;
   if (!::CreateProcess(nullptr, szBatchPath, nullptr, nullptr, FALSE,
                        CREATE_NEW_CONSOLE | CREATE_NO_WINDOW |
                        CREATE_SUSPENDED | IDLE_PRIORITY_CLASS, nullptr, 
                        _T("\\"), &si, &pi))
   {
      MyLog::GetLog().Log(_T("Error creating batch process\n"));
      return;
   }

   // lower batch file's priority even more
   SetThreadPriority(pi.hThread, THREAD_PRIORITY_IDLE);

   // raise our priority so that we terminate as quickly as possible
   ::SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_ABOVE_NORMAL);
   ::SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS /*REQ: 2000 ABOVE_NORMAL_PRIORITY_CLASS*/);

   ::ResumeThread(pi.hThread);   // start batch file

   ::CloseHandle(pi.hThread);
   ::CloseHandle(pi.hProcess);

   // now we exit as fast as possible so we can be deleted
}


/*
   dwFreq      frequency in hertz (20Hz and up)
   dwDuration  milliseconds (50ms and up)
*/
void skst::BeepSpeaker(DWORD dwFreq, DWORD dwDuration)
{
   /*
      check frequency and duration limits
   */
   if (dwFreq == 0)
      return;

   if (dwFreq < 20)
      dwFreq = 20;

   if (dwDuration < 50)
      dwDuration = 50;

   ::Beep(dwFreq, dwDuration);
}


/*
   From MSDN Mag (Microsoft Systems Journal)
   December 2001, C++ Q&A, p 147
   Paul DiLascia
   Also MSKB Q140723
*/
/*
   But what about passwords??

   Also, when we use this in Alarm++'s event dialog's OnExecuteEventActivities(),
   the event dialog never appears (as if it were closed) and the screen-saver
   doesn't close. Hmm...
*/
BOOL CALLBACK CloseScreenSaverEnumFct(HWND hWnd, LPARAM)
{
   if (!::IsWindowVisible(hWnd))
      return TRUE;   // continue

   // if a screen-saver is running, only its window is visible in this desktop
   ::PostMessage(hWnd, WM_CLOSE, 0, 0);

   return FALSE;
}

void skst::CloseScreenSaver()
{
   // if screen-saver isn't active, return
   BOOL bRunning;
   ::SystemParametersInfo(SPI_GETSCREENSAVERRUNNING, 0, &bRunning, 0);
   if (!bRunning)
      return;

   HDESK hDesk = ::OpenDesktop(_T("Screen-saver"), 0, FALSE /*inherit?*/, DESKTOP_READOBJECTS | DESKTOP_WRITEOBJECTS);
   if (hDesk == nullptr)
      return;

   ::EnumDesktopWindows(hDesk, CloseScreenSaverEnumFct, 0);
   ::CloseDesktop(hDesk);
}

//---------------------------------------------------------------------------

//-----------------------------------------------------------------
/*
   This routine returns true if at least one boolean value is set
   but not all of them are.
*/
//-----------------------------------------------------------------
/*
static bool AreAnySet(const bool a[], const int n)
{
   bool bOneSet = false;   // is at least one set?
   bool bAllSet = true;    // are they all set?
   for (int i = 0; i < n; ++i)
   {
      if (a[i])
         bOneSet = true;
      else
         bAllSet = false;
   }

   return (bOneSet && !bAllSet);
}
*/

//---------------------------------------------------------------------------

/*
   Copies a Registry key (and all its subkeys and values).
   Specifically:
      hKeySrc\\szPathParentSrc\\szKeyNameSrc\\*
         is copied to
      hKeyDest\\szPathParentDest\\szKeyNameDest\\*

   For example:
      RegKeyCopy(HKEY_CURRENT_USER, "\\Software\\Company Src", "My App",
                 HKEY_LOCAL_MACHINE, "\\Software\\Company Dest", "My New App");
      copies
         HKEY_CURRENT_USER\\Software\\Company Src\\My App\\
                                                         Settings\\*
      to
         HKEY_LOCAL_MACHINE\\Software\\Company Dest\\My New App\\
                                                         Settings\\*
*/
// from registry_key::operator HKEY()
static HKEY OpenKey(HKEY hKeyParent, LPCTSTR szPath)
{
   skst::LogInOut tio(_T(__FUNCTION__));

   HKEY hKey;
   /*
      Try to open the key with maximum rights and degrade gracefully if they're not permitted.
   */
   if (::RegOpenKeyEx(hKeyParent, szPath, 0, KEY_ALL_ACCESS, &hKey) != ERROR_SUCCESS)
   {
      skst::MyLog::GetLog().LogV(_T("Failed to open key '%s' with KEY_ALL_ACCESS."), szPath);
      if (::RegOpenKeyEx(hKeyParent, szPath, 0, KEY_WRITE, &hKey) != ERROR_SUCCESS)
      {
         skst::MyLog::GetLog().LogV(_T("Failed to open key '%s' with KEY_WRITE."), szPath);
         if (::RegOpenKeyEx(hKeyParent, szPath, 0, STANDARD_RIGHTS_WRITE | KEY_SET_VALUE, &hKey) != ERROR_SUCCESS)
         {
            skst::MyLog::GetLog().LogV(_T("Failed to open key '%s' with STANDARD_RIGHTS_WRITE | KEY_SET_VALUE."), szPath);
            if (::RegOpenKeyEx(hKeyParent, szPath, 0, KEY_READ, &hKey) != ERROR_SUCCESS)
            {
               skst::MyLog::GetLog().LogV(_T("Failed to open key '%s' with KEY_READ."), szPath);
               if (::RegOpenKeyEx(hKeyParent, szPath, 0, KEY_QUERY_VALUE | KEY_ENUMERATE_SUB_KEYS, &hKey) != ERROR_SUCCESS)
               {
                  skst::MyLog::GetLog().LogV(_T("Failed to open key '%s' with KEY_QUERY_VALUE | KEY_ENUMERATE_SUB_KEYS."), szPath);
                  return nullptr;
               }
            }
         }
      }
   }
   return hKey;
}

bool skst::RegKeyCopy(HKEY hKeyRootSrc, LPCTSTR szPathParentSrc, LPCTSTR szKeyNameSrc,
                      HKEY hKeyRootDest, LPCTSTR szPathParentDest, LPCTSTR szKeyNameDest)
{
   skst::LogInOut tio(_T(__FUNCTION__));

   skst::MyLog::GetLog().LogV(_T("Attempting to copy '%s\\%s' to '%s\\%s'."), szPathParentSrc, szKeyNameSrc, szPathParentDest, szKeyNameDest);

   /*
      create destination key (if necessary) and its new subkey
   */
   HUSKEY hKeyTmp;
   if (::SHRegCreateUSKey(szPathParentDest, KEY_WRITE, nullptr, &hKeyTmp,
                          (hKeyRootDest == HKEY_CURRENT_USER) ? SHREGSET_HKCU : SHREGSET_HKLM) != ERROR_SUCCESS)
   {
      skst::MyLog::GetLog().LogV(_T("Unable to create parent destination key '%s'."), szPathParentDest);
      return false;
   }
   ::SHRegCloseUSKey(hKeyTmp);

   tstring strPathDest(szPathParentDest);
   strPathDest += _T("\\");
   strPathDest += szKeyNameDest;
   // REQ: 95 w/IE4, 98, 2k, NT4 w/IE4
   if (::SHRegCreateUSKey(strPathDest.c_str(), KEY_WRITE, nullptr, &hKeyTmp,
                          (hKeyRootDest == HKEY_CURRENT_USER) ? SHREGSET_HKCU : SHREGSET_HKLM) != ERROR_SUCCESS)
   {
      skst::MyLog::GetLog().LogV(_T("Unable to create destination key '%s' (%s)."), strPathDest.c_str(), szKeyNameDest);
      return false;
   }
   ::SHRegCloseUSKey(hKeyTmp);
   hKeyTmp = nullptr;
   // now open a "usable" key
   HKEY hKeyDest = ::OpenKey(hKeyRootDest, strPathDest.c_str());
   if (hKeyDest == nullptr)
   {
      skst::MyLog::GetLog().LogV(_T("Unable to open destination key '%s'."), strPathDest.c_str());
      return false;
   }

   /*
      open src key
   */
   HKEY hKeySrc = ::OpenKey(hKeyRootSrc, szPathParentSrc);
   if (hKeySrc == nullptr)
   {
      skst::MyLog::GetLog().LogV(_T("Unable to open parent source key '%s'."), szPathParentSrc);
      ::RegCloseKey(hKeyDest);
      return false;
   }

   // copy it
   // REQ: 95 w/IE5, 98, 2k, NT4 w/IE5
   const bool bOK = (::SHCopyKey(hKeySrc, szKeyNameSrc, hKeyDest, 0) == ERROR_SUCCESS);

   skst::MyLog::GetLog().Log(bOK ? _T("\tsuccessful") : _T("\tfailure"));

   ::RegCloseKey(hKeyDest);
   ::RegCloseKey(hKeySrc);

   return bOK;
}


/*
   This function moves the specified key and deletes
   its old parent (if the parent is now empty).
*/
bool skst::RegKeyMove(HKEY hKeyRootSrc, LPCTSTR szPathParentSrc, LPCTSTR szKeyNameSrc,
                      HKEY hKeyRootDest, LPCTSTR szPathParentDest, LPCTSTR szKeyNameDest)
{
   skst::LogInOut tio(_T(__FUNCTION__));

   skst::MyLog::GetLog().LogV(_T("Attempting to move '%s\\%s' to '%s\\%s'."), szPathParentSrc, szKeyNameSrc, szPathParentDest, szKeyNameDest);

   if (!skst::RegKeyCopy(hKeyRootSrc, szPathParentSrc, szKeyNameSrc, hKeyRootDest, szPathParentDest, szKeyNameDest))
      return false;

   /*
      delete source key
   */
   HKEY hKeySrc = ::OpenKey(hKeyRootSrc, szPathParentSrc);
   if (hKeySrc == nullptr)
   {
      skst::MyLog::GetLog().LogV(_T("Unable to open parent source key '%s'."), szPathParentSrc);
      return false;
   }

   // REQ: 95 w/IE4, 98, 2k, NT4 w/IE4
   if (::SHDeleteKey(hKeySrc, szKeyNameSrc) == ERROR_SUCCESS)
   {
      ::RegCloseKey(hKeySrc);

      /*
         delete src parent key if empty
      */
      // have to extract parent's key name from key path
      tstring strPathGrandparentSrc(szPathParentSrc);
      const tstring::size_type ix = strPathGrandparentSrc.rfind(_T('\\'));
      if (ix != tstring::npos)
      {
         const tstring strPathParentSrc(strPathGrandparentSrc.substr(ix + 1));
         strPathGrandparentSrc.erase(ix);
         hKeySrc = ::OpenKey(hKeyRootSrc, strPathGrandparentSrc.c_str());
         if (hKeySrc != nullptr)
         {
            // REQ: 95 w/IE4, 98, 2k, NT4 w/IE4
            if (::SHDeleteEmptyKey(hKeySrc, strPathParentSrc.c_str()) != ERROR_SUCCESS)
               skst::MyLog::GetLog().LogV(_T("Unable to delete empty source parent key--maybe it's not empty--but that's okay."), strPathParentSrc.c_str());
            ::RegCloseKey(hKeySrc);
         }
         else
            skst::MyLog::GetLog().LogV(_T("Unable to open grandparent source key '%s'."), strPathGrandparentSrc.c_str());
      }
      else
         skst::MyLog::GetLog().LogV(_T("Unable to find key name in '%s'."), szPathParentSrc);
   }
   else
   {
      skst::MyLog::GetLog().LogV(_T("Unable to delete source key '%s\\%s'."), szPathParentSrc, szKeyNameSrc);
      ::RegCloseKey(hKeySrc);
   }

   return true;
}
