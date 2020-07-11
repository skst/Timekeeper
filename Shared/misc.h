//---------------------------------------------------------------------------
// (c) 2002-2009 12noon, Stefan K. S. Tucker
// (c) 2002 Perpetual Motion Software, Stefan K. S. Tucker
//---------------------------------------------------------------------------

#pragma once

#include <string>

#include "skstutil.h"

namespace skst
{

//------------------------------
// MSJ Nov 97 v12 n11 Win32 Q&A
//--------------------------------------------------
// Example:
//    #pragma MSG("Add err-checking later")
// 
// Output:
//    c:\Project\fish.cpp(296): Add err-checking later
//----------------------------------------------------------------------------
// was: #define chMSG(desc) message(__FILE__ "(" chSTR2(__LINE__) "):" #desc)
// #pragma chMSG(Add err-checking later)
//----------------------------------------------------------------------------

#define  chSTR(x)    #x
#define  chSTR2(x)   chSTR(x)
#define  MSG(desc)   message(__FILE__ "(" chSTR2(__LINE__) "): " desc)

/*
   display various Windows defines
*/
#define  EMIT_VALUE(def)   message(#def _T(" = ") chSTR2(def))

// displays macro name itself if not defined
//#pragma EMIT_VALUE(WINVER)
//#pragma EMIT_VALUE(_WIN32_WINDOWS)
//#pragma EMIT_VALUE(_WIN32_WINNT)
//#pragma EMIT_VALUE(_WIN32_IE)


//Remove for Visual Studio 2005
// From afximpl.h
// determine number of elements in an array (not bytes)
//// From afximpl.h
//// determine number of elements in an array (not bytes)
//#define _countof(array) (sizeof(array)/sizeof(array[0]))


/////////////////////////////////////////////////////////////////////////////
// Miscellaneous functions

extern	HICON			LoadIconSmall(const HINSTANCE hInst, const UINT id);

extern   tstring		GetAppPath(const HINSTANCE hInst);
extern   tstring		GetAppPathOnly(const HINSTANCE hInst);
extern   tstring		GetAppOnly(const HINSTANCE hInst);
extern   tstring		GetFileNameWithNewExtension(const HINSTANCE hInst, LPCTSTR szExt);
extern   tstring		GetLocalPath(const HINSTANCE hInst, LPCTSTR szFilename);

extern   tstring		GetLastErrorString();
extern   tstring		GetCommDlgExtendedErrorString();

extern   tstring		GetLocaleInfoString(LCTYPE lcType);
extern   tstring		GetDroppedFilePath(HDROP h, UINT ix);
extern   tstring		MyGetFolderPath(const int iType);

extern   tstring		IntToString(const int i);

extern   BSTR        ASCIItoBSTR(LPCSTR str);
extern   std::string	BSTRtoASCII(const BSTR bstr);

extern   void        AddEllipsis(LPTSTR szDest, LPCTSTR szSrc, const int nchSize);

extern   tstring		MyFormatV(LPCTSTR szFormat, ...);

extern   void        DeleteSelf();
extern   void        BeepSpeaker(DWORD dwFreq, DWORD dwDuration);

extern   void        CloseScreenSaver();

/*
   These functions can be integrated into the 'registry_key' class:

      registry_key keySrc(HKCU, "Software\\Perpetual Motion");
      registry_key keyDest(HKCU, "Software\\12noon");
      keySrc.Move("Alarm++", keyDest, "Alarm++");

   or they could just be called by such functions.
*/
extern   bool        RegKeyCopy(HKEY hKeyRootSrc, LPCTSTR szPathParentSrc, LPCTSTR szKeyNameSrc,
                                HKEY hKeyRootDest, LPCTSTR szPathParentDest, LPCTSTR szKeyNameDest);
extern   bool        RegKeyMove(HKEY hKeyRootSrc, LPCTSTR szPathParentSrc, LPCTSTR szKeyNameSrc,
                                HKEY hKeyRootDest, LPCTSTR szPathParentDest, LPCTSTR szKeyNameDest);

// from md5.cpp
extern   bool        DigestMD5(LPCTSTR sz, tstring& strDigest);
extern   bool        DigestMD5_ANSI(LPCSTR sz, tstring& strDigest);

}
