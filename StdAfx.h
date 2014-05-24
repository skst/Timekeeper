// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

/*
	8.1	0x0603
	8		0x0602
	7		0x0601
	Vista	0x0600
	2003	0x0502
	XP		0x0501
	2K/Me	0x0500
	98		0x0410
	95		0x0400
*/
#include <WinSDKVer.h>

//#define _WIN32_WINDOWS	0x0600
//#define WINVER				0x0600

#define _WIN32_WINNT		0x0600

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN            // Exclude rarely-used stuff from Windows headers
#endif

#include "targetver.h"


#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS      // some CString constructors will be explicit

#include <afxwin.h>         // MFC core and standard components
#include <wininet.h>       // You must #include <wininet.h> before ANY shlobj.h (e.g., afxdisp.h, afxdtctl.h)
#include <afxdisp.h>        // For COleDateTime (in afxcomtime.h)
#include <afxpriv.h>        // For Unicode conversion macros

#include "MyWin/stl.h"
#include <afxdtctl.h>
