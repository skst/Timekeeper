////////////////////////////////////////////////////////////////
// PixieLib(TM) Copyright 1997-1999 Paul DiLascia
// If this code works, it was written by Paul DiLascia.
// If not, I don't know who wrote it.
//
// ---
// General purpose debugging utilities.
//
#ifndef DEBUG_H
#define DEBUG_H

#ifndef countof
#define countof(x)	(sizeof(x)/sizeof(x[0]))
#endif

#ifdef _DEBUG

// tell linker to look in ole32 lib (for StringFromClsid)
#pragma comment(linker, "/defaultlib:ole32.lib")

// This macro lets you put a debug test into your, so you can write
//
//     DEBUG_IF((x=5),TRACE("Something is rotten here.\n"));
//
// instead of
//
// #ifdef _DEBUG
//     if (x=5)
//       TRACE("Something is rotten here.\n");
// #endif
//
#define DEBUG_IF(condition, action) if (condition) action;

//////////////////
// TRACEFN is a macro that lets you generate indented TRACE output so you
// can see the call stack. To use it:
//
//		SomeFn(...)
//		{
//			TRACEFN("Entering SomeFn...\n");
//			.
//			.
//		}
//
// Now all trace output after TRACEFN will be indented one space, until SomeFn
// returns. You can put TRACEFN in multiple functions to see indented trace
// output. For an example of this, see the HOOK sample program.
//
// NOTE: YOU MUST NOT USE TRACEFN IN A ONE-LINE IF STATEMENT!
// This will fail:
//
// if (foo)
//    TRACEFN(...)
//
// Instead, you must enclose the TRACE in squiggle-brackets
//
// if (foo) {
//		TRACEFN(...)
// }
//
#define TRACEFN CTraceFn __fooble; TRACE
//
// This class implements TRACEFN. Don't ever use directly!
//
class DLLCLASS CTraceFn {
private:
	static int	nIndent;				// current indent level
	friend void AFX_CDECL AfxTrace(LPCTSTR lpszFormat, ...);
public:
	CTraceFn()  { nIndent++; }		// constructor bumps indent
	~CTraceFn() { nIndent--; }		// destructor restores it
};

//////////////////
// The following stuff is for getting human-readable names of things so
// you can show them in TRACE statements. For example,
//
// TRACE("Window is: %s\n", _TR(pWnd));
//
// Will generate output showing the name and title of the window, etc.
//

// Macro _TR casts _DbgName(x) to LPCTSTR for use with in printf so you
// can write
//
//   TRACE("Message is %s\n", _TR(uMsg));
//
// instead of
//
//   TRACE("Message is %s\n", (LPCTSTR)_DbgName(uMsg));
//
#define _TR(x)			(LPCTSTR)_DbgName(x)

// overloaded fns to get names of things.
extern DLLFUNC CString _DbgName(CWnd* pWnd); // get name of window
extern DLLFUNC CString _DbgName(UINT uMsg);	// ... WM_ message
extern DLLFUNC CString _DbgName(REFIID iid);	// get name of COM interface
extern DLLFUNC CString _DbgName(SCODE sc);	// get name of COM SCODE

// To debug the names of COM interfaces, just write
// lines to your program:
//
//		DEBUG_BEGIN_INTERFACE_NAMES()
//			DEBUG_INTERFACE_NAME(IFoo)
//			DEBUG_INTERFACE_NAME(IBar)
//			...
//		DEBUG_END_INTERFACE_NAMES();
//
// PixieLib already knows the names of many common interfaces (see Debug.cpp)
//
struct DBGINTERFACENAME {
	const IID* piid;	// ptr to GUID
	PCTSTR name;		// human-readable name of interface
};

// Used to add interface names to global list -- Use macro
class DLLCLASS CInterfaceNames {
protected:
	static CInterfaceNames* s_pFirst;
	CInterfaceNames*			m_pNext;
	DBGINTERFACENAME*			m_pEntries;
	UINT							m_nEntries;
public:
	CInterfaceNames(DBGINTERFACENAME* pdin, UINT n);
	static const DBGINTERFACENAME* FindEntry(REFIID iid);
};

#define DEBUG_BEGIN_INTERFACE_NAMES()								\
static DBGINTERFACENAME _myDBI[] = {								\

#define DEBUG_INTERFACE_NAME(iface)									\
	{ &IID_##iface, _T(#iface) },										\

#define DEBUG_END_INTERFACE_NAMES()									\
};																				\
static CInterfaceNames _initMyDBI(_myDBI, countof(_myDBI));	\

// Macro casts to LPCTSTR for use with TRACE/printf/CString::Format
//
#define DbgName(x)	(LPCTSTR)_DbgName(x)

#else // Below NOT _DEBUG ----------------------------------------------------

#define DEBUG_IF(cond, action) ;
#define _TR(x)			((LPCTSTR)NULL)
#define DbgName(x)	((LPCTSTR)NULL)
#define TRACEFN TRACE

#define DEBUG_BEGIN_INTERFACE_NAMES()
#define DEBUG_END_INTERFACE_NAMES()
#define DEBUG_INTERFACE_NAME(iface)

#endif // _DEBUG

#endif // DEBUG_H
