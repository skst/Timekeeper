// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers

/*
	7		0x0601
	Vista	0x0600
	2003	0x0502
	XP		0x0501
	2K/Me	0x0500
	98		0x0410
	95		0x0400
*/
#define	_WIN32_WINDOWS	0x0501
#define	WINVER			0x0501

#include <afxwin.h>         // MFC core and standard components
#include <wininet.h>       // You must #include <wininet.h> before ANY shlobj.h (e.g., afxdisp.h, afxdtctl.h)
#include <afxdisp.h>        // For COleDateTime (in afxcomtime.h)
#include <afxpriv.h>        // For Unicode conversion macros

#include "MyWin/stl.h"
#include <afxdtctl.h>
