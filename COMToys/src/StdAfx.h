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
//#define	_WIN32_WINDOWS	0x0600
//#define	WINVER			0x0600

#define _WIN32_WINNT		0x0601

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN            // Exclude rarely-used stuff from Windows headers
#endif

#include <SDKDDKVer.h>


#include <afxwin.h>         // MFC core and standard components
#include <afxdisp.h>        // MFC OLE automation classes
#include <afxpriv.h>        // For Unicode conversion macros
#include <afxctl.h>			 // control stuff
#include <afxmt.h>			 // Multithreading
#include <wininet.h>//VS.NET
/*
   VCInstallDir = C:\Program Files\Microsoft Visual Studio 8\VC\
   We need the comdef.h in the MS Platform SDK at
      C:\Program Files\Microsoft Platform SDK\Include\ComDef.h
*/
//#include <\Program Files\Microsoft Platform SDK\Include\ComDef.h>
#include <comdef.h>			 // basic COM defs (_bstr_t etc., includes comutil.h)
#include <shlobj.h>			 // for IDeskBand ettc
#include <atlbase.h>			 // ATL stuff
