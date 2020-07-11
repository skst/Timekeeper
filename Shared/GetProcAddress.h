//---------------------------------------------------------------------------
// (c) 2003-2009 12noon, Stefan K. S. Tucker
// (c) 2001-2002 Perpetual Motion Software, Stefan K. S. Tucker
//---------------------------------------------------------------------------

#pragma once

#include "MyWin/MyLog.h"

namespace skst
{
typedef BOOL					(WINAPI *			FCT_AnimateWindow)(IN HWND, IN DWORD, IN DWORD);
typedef BOOL					(WINAPI *			FCT_ChangeWindowMessageFilter)(UINT, DWORD);
typedef HRESULT				(STDAPICALLTYPE *	FCT_CloseThemeData)(HTHEME hTheme);
typedef HRESULT				(STDAPICALLTYPE *	FCT_DrawThemeBackground)(HTHEME hTheme, HDC hdc, int iPartId, int iStateId, const RECT *pRect, OPTIONAL const RECT *pClipRect);
typedef HWND					(STDAPICALLTYPE *	FCT_GetConsoleWindow)();
typedef LANGID					(WINAPI *			FCT_GetSystemDefaultUILanguage)();
typedef LANGID					(WINAPI *			FCT_GetUserDefaultUILanguage)();
typedef BOOL					(WINAPI *			FCT_IsThemeActive)();
typedef HDESK					(WINAPI *			FCT_OpenDesktop)(LPTSTR, DWORD, BOOL, ACCESS_MASK);
typedef HTHEME					(STDAPICALLTYPE *	FCT_OpenThemeData)(HWND hwnd, LPCWSTR pszClassList);
typedef /*WINUSERAPI*/ BOOL (WINAPI *			FCT_SetLayeredWindowAttributes)(HWND, COLORREF, BYTE, DWORD);
typedef HRESULT				(STDAPICALLTYPE *	FCT_SHGetFolderPath)(HWND, int, HANDLE, DWORD, LPTSTR);
typedef BOOL					(WINAPI *			FCT_SHGetSpecialFolderPath)(HWND hwndOwner, LPTSTR lpszPath, int nFolder, BOOL fCreate);

/*
   Normally you have to dynamically load procedure addresses like this:

      HINSTANCE hShell = ::LoadLibrary("shell32.dll");
      typedef BOOL (STDAPICALLTYPE * FCT_SHGetDiskFreeSpace)(PCTSTR, ULARGE_INTEGER *, ULARGE_INTEGER *, ULARGE_INTEGER *);
      FCT_SHGetDiskFreeSpace fctSHGetDiskFreeSpace = (FCT_SHGetDiskFreeSpace) ::GetProcAddress(hShell, "SHGetDiskFreeSpaceA");
      if (fctSHGetDiskFreeSpace == NULL)
         throw 0;
      if (hShell != NULL)
         ::FreeLibrary(hShell);

   This class provides a nice wrapper around ::GetProcAddress().
   It's designed to be used like this, one object per function:

      typedef BOOL (STDAPICALLTYPE * FCT_SHGetDiskFreeSpace)(PCTSTR, ULARGE_INTEGER *, ULARGE_INTEGER *, ULARGE_INTEGER *);

      GetProcAddress<FCT_SHGetDiskFreeSpace> gpa("shell32.dll", "SHGetDiskFreeSpaceA");
      if (gpa)
      {
         if (!(*gpa)(_T("c:\\"), &i64BytesFree, &i64, NULL))
            throw 0;
      }

   OR

      // BOOL SetLayeredWindowAttributes(HWND hwnd, COLORREF crKey, BYTE bAlpha, DWORD dwFlags);
      typedef BOOL (*FCT_SetLayeredWindowAttributes)(HWND, COLORREF, BYTE, DWORD);

      GetProcAddress<FCT_SetLayeredWindowAttributes> gpa("user32.dll", "SetLayeredWindowAttributes");
      if (gpa)
         (*gpa)(h, RGB(0, 0, 0), 0xD0, 0);
*/

template<class F> class GetProcAddress
{
protected:
   HMODULE _hLib;
   F _fct;

public:
   GetProcAddress(PCTSTR strNameLib, PCSTR strNameFct)
   {
      _fct = nullptr;

      _hLib = ::LoadLibrary(strNameLib);
      if (_hLib == nullptr)
      {
         skst::MyLog::GetLog().LogV(_T("Loading %s failed"), strNameLib);
         return;
      }

      _fct = (F) ::GetProcAddress(_hLib, strNameFct);
      if (_fct == nullptr)
         skst::MyLog::GetLog().LogV(_T("Loading %hs() failed"), strNameFct);
   }

   virtual ~GetProcAddress()
   {
      if (_hLib != nullptr)
      {
         ::FreeLibrary(_hLib);
         _hLib = nullptr;
         _fct = nullptr;
      }
   }

   operator bool() const
   {
      return (_fct != nullptr);
   }

   F& operator *() { return _fct; }
};

}

