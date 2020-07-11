////////////////////////////////////////////////////////////////
// ComToys(TM) Copyright 1999 Paul DiLascia
// If this code works, it was written by Paul DiLascia.
// If not, I don't know who wrote it.
//
#include "StdAfx.h"
#include <afxpriv.h>	// for MFC WM_ messages
#include "COMtoys/ComToys.h"
#include "COMtoys/Debug.h"

#ifdef _DEBUG

#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;

int CTraceFn::nIndent=-1;	    // current indent level

//Remove for Visual Studio 2005
// From afximpl.h
// determine number of elements in an array (not bytes)
//#define _countof(array) (sizeof(array)/sizeof(array[0]))

////////////////
// These functions are copied from dumpout.cpp in the MFC source,
// with my modification to do indented TRACEing
//
void AFXAPI AfxDump(const CObject* pOb)
{
	afxDump << pOb;
}

void AFX_CDECL AfxTrace(LPCTSTR lpszFormat, ...)
{
#ifdef _DEBUG // all AfxTrace output is controlled by afxTraceEnabled
	if (!afxTraceEnabled)
		return;
#endif

	va_list args;
	va_start(args, lpszFormat);

	int nBuf;
	TCHAR szBuffer[512];

   nBuf = _vstprintf_s(szBuffer, sizeof szBuffer, lpszFormat, args);
	ASSERT(nBuf < _countof(szBuffer));

	// PD: Here are my added lines to do the indenting. Search
	// for newlines and insert prefix before each one. Yawn.
	//
	static BOOL bStartNewLine = TRUE;
	TCHAR* nextline;
	for (TCHAR* start = szBuffer; *start; start=nextline+1) {
		if (bStartNewLine) {
if (true)//VS.NET			if ((afxTraceFlags & traceMultiApp) && (AfxGetApp() != NULL))
				afxDump << AfxGetApp()->m_pszExeName << _T(": ");
			afxDump << CString(' ',CTraceFn::nIndent);
			bStartNewLine = FALSE;
		}
		nextline = _tcschr(start, '\n');
		if (nextline) {
			*nextline = 0; // terminate string at newline
			bStartNewLine = TRUE;
		}
		afxDump << start;
		if (!nextline)
			break;
		afxDump << _T("\n"); // the one I terminated
	}		
	va_end(args);
}

//////////////////
// Get window name in the form classname[HWND,title]
// Searches all the parents for a window with a title.
//
CString _DbgName(CWnd* pWnd)
{
	CString sTitle;
	HWND hwnd = pWnd->GetSafeHwnd();
	if (hwnd==NULL)
		sTitle = _T("NULL");
	else if (!::IsWindow(hwnd))
		sTitle = _T("[bad window]");
	else {
		sTitle = _T("[no title]");
		for (CWnd* pw = pWnd; pw; pw = pw->GetParent()) {
			if (pw->GetWindowTextLength() > 0) {
				pw->GetWindowText(sTitle);
				break;
			}
		}
	}
	CString s;
	s.Format(_T("%s[0x%04x,\"%s\"]"), 
		pWnd ? CA2CT(pWnd->GetRuntimeClass()->m_lpszClassName) : _T("NULL"),
		hwnd, (LPCTSTR)sTitle);
	return s;
}

struct {
	UINT		msg;
	LPCTSTR	name;
} MsgData[] = {
	{ WM_CREATE,_T("WM_CREATE") },
	{ WM_DESTROY,_T("WM_DESTROY") },
	{ WM_MOVE,_T("WM_MOVE") },
	{ WM_SIZE,_T("WM_SIZE") },
	{ WM_ACTIVATE,_T("WM_ACTIVATE") },
	{ WM_SETFOCUS,_T("WM_SETFOCUS") },
	{ WM_KILLFOCUS,_T("WM_KILLFOCUS") },
	{ WM_ENABLE,_T("WM_ENABLE") },
	{ WM_SETREDRAW,_T("WM_SETREDRAW") },
	{ WM_SETTEXT,_T("WM_SETTEXT") },
	{ WM_GETTEXT,_T("WM_GETTEXT") },
	{ WM_GETTEXTLENGTH,_T("WM_GETTEXTLENGTH") },
	{ WM_PAINT,_T("WM_PAINT") },
	{ WM_CLOSE,_T("WM_CLOSE") },
	{ WM_QUERYENDSESSION,_T("WM_QUERYENDSESSION") },
	{ WM_QUIT,_T("WM_QUIT") },
	{ WM_QUERYOPEN,_T("WM_QUERYOPEN") },
	{ WM_ERASEBKGND,_T("WM_ERASEBKGND") },
	{ WM_SYSCOLORCHANGE,_T("WM_SYSCOLORCHANGE") },
	{ WM_ENDSESSION,_T("WM_ENDSESSION") },
	{ WM_SHOWWINDOW,_T("WM_SHOWWINDOW") },
	{ WM_WININICHANGE,_T("WM_WININICHANGE") },
	{ WM_SETTINGCHANGE,_T("WM_SETTINGCHANGE") },
	{ WM_DEVMODECHANGE,_T("WM_DEVMODECHANGE") },
	{ WM_ACTIVATEAPP,_T("WM_ACTIVATEAPP") },
	{ WM_FONTCHANGE,_T("WM_FONTCHANGE") },
	{ WM_TIMECHANGE,_T("WM_TIMECHANGE") },
	{ WM_CANCELMODE,_T("WM_CANCELMODE") },
	{ WM_SETCURSOR,_T("WM_SETCURSOR") },
	{ WM_MOUSEACTIVATE,_T("WM_MOUSEACTIVATE") },
	{ WM_CHILDACTIVATE,_T("WM_CHILDACTIVATE") },
	{ WM_QUEUESYNC,_T("WM_QUEUESYNC") },
	{ WM_GETMINMAXINFO,_T("WM_GETMINMAXINFO") },
	{ WM_PAINTICON,_T("WM_PAINTICON") },
	{ WM_ICONERASEBKGND,_T("WM_ICONERASEBKGND") },
	{ WM_NEXTDLGCTL,_T("WM_NEXTDLGCTL") },
	{ WM_SPOOLERSTATUS,_T("WM_SPOOLERSTATUS") },
	{ WM_DRAWITEM,_T("WM_DRAWITEM") },
	{ WM_MEASUREITEM,_T("WM_MEASUREITEM") },
	{ WM_DELETEITEM,_T("WM_DELETEITEM") },
	{ WM_VKEYTOITEM,_T("WM_VKEYTOITEM") },
	{ WM_CHARTOITEM,_T("WM_CHARTOITEM") },
	{ WM_SETFONT,_T("WM_SETFONT") },
	{ WM_GETFONT,_T("WM_GETFONT") },
	{ WM_SETHOTKEY,_T("WM_SETHOTKEY") },
	{ WM_GETHOTKEY,_T("WM_GETHOTKEY") },
	{ WM_QUERYDRAGICON,_T("WM_QUERYDRAGICON") },
	{ WM_COMPAREITEM,_T("WM_COMPAREITEM") },
	{ WM_COMPACTING,_T("WM_COMPACTING") },
	{ WM_COMMNOTIFY,_T("WM_COMMNOTIFY") },
	{ WM_WINDOWPOSCHANGING,_T("WM_WINDOWPOSCHANGING") },
	{ WM_WINDOWPOSCHANGED,_T("WM_WINDOWPOSCHANGED") },
	{ WM_POWER,_T("WM_POWER") },
	{ WM_COPYDATA,_T("WM_COPYDATA") },
	{ WM_CANCELJOURNAL,_T("WM_CANCELJOURNAL") },
#if(WINVER >= 0x0400)
	{ WM_NOTIFY,_T("WM_NOTIFY") },
	{ WM_INPUTLANGCHANGEREQUEST,_T("WM_INPUTLANGCHANGEREQUEST") },
	{ WM_INPUTLANGCHANGE,_T("WM_INPUTLANGCHANGE") },
	{ WM_TCARD,_T("WM_TCARD") },
	{ WM_HELP,_T("WM_HELP") },
	{ WM_USERCHANGED,_T("WM_USERCHANGED") },
	{ WM_NOTIFYFORMAT,_T("WM_NOTIFYFORMAT") },
	{ WM_CONTEXTMENU,_T("WM_CONTEXTMENU") },
	{ WM_STYLECHANGING,_T("WM_STYLECHANGING") },
	{ WM_STYLECHANGED,_T("WM_STYLECHANGED") },
	{ WM_DISPLAYCHANGE,_T("WM_DISPLAYCHANGE") },
	{ WM_GETICON,_T("WM_GETICON") },
	{ WM_SETICON,_T("WM_SETICON") },
#endif /* WINVER >= 0x0400 */
	{ WM_NCCREATE,_T("WM_NCCREATE") },
	{ WM_NCDESTROY,_T("WM_NCDESTROY") },
	{ WM_NCCALCSIZE,_T("WM_NCCALCSIZE") },
	{ WM_NCHITTEST,_T("WM_NCHITTEST") },
	{ WM_NCPAINT,_T("WM_NCPAINT") },
	{ WM_NCACTIVATE,_T("WM_NCACTIVATE") },
	{ WM_GETDLGCODE,_T("WM_GETDLGCODE") },
	{ WM_NCMOUSEMOVE,_T("WM_NCMOUSEMOVE") },
	{ WM_NCLBUTTONDOWN,_T("WM_NCLBUTTONDOWN") },
	{ WM_NCLBUTTONUP,_T("WM_NCLBUTTONUP") },
	{ WM_NCLBUTTONDBLCLK,_T("WM_NCLBUTTONDBLCLK") },
	{ WM_NCRBUTTONDOWN,_T("WM_NCRBUTTONDOWN") },
	{ WM_NCRBUTTONUP,_T("WM_NCRBUTTONUP") },
	{ WM_NCRBUTTONDBLCLK,_T("WM_NCRBUTTONDBLCLK") },
	{ WM_NCMBUTTONDOWN,_T("WM_NCMBUTTONDOWN") },
	{ WM_NCMBUTTONUP,_T("WM_NCMBUTTONUP") },
	{ WM_NCMBUTTONDBLCLK,_T("WM_NCMBUTTONDBLCLK") },
	{ WM_KEYDOWN,_T("WM_KEYDOWN") },
	{ WM_KEYUP,_T("WM_KEYUP") },
	{ WM_CHAR,_T("WM_CHAR") },
	{ WM_DEADCHAR,_T("WM_DEADCHAR") },
	{ WM_SYSKEYDOWN,_T("WM_SYSKEYDOWN") },
	{ WM_SYSKEYUP,_T("WM_SYSKEYUP") },
	{ WM_SYSCHAR,_T("WM_SYSCHAR") },
	{ WM_SYSDEADCHAR,_T("WM_SYSDEADCHAR") },
	{ WM_KEYDOWN,_T("WM_KEYDOWN") },
	{ WM_KEYUP,_T("WM_KEYUP") },
	{ WM_CHAR,_T("WM_CHAR") },
	{ WM_DEADCHAR,_T("WM_DEADCHAR") },
	{ WM_SYSKEYDOWN,_T("WM_SYSKEYDOWN") },
	{ WM_SYSKEYUP,_T("WM_SYSKEYUP") },
	{ WM_SYSCHAR,_T("WM_SYSCHAR") },
	{ WM_SYSDEADCHAR,_T("WM_SYSDEADCHAR") },
#if(WINVER >= 0x0400)
	{ WM_IME_STARTCOMPOSITION,_T("WM_IME_STARTCOMPOSITION") },
	{ WM_IME_ENDCOMPOSITION,_T("WM_IME_ENDCOMPOSITION") },
	{ WM_IME_COMPOSITION,_T("WM_IME_COMPOSITION") },
	{ WM_IME_KEYLAST,_T("WM_IME_KEYLAST") },
#endif
	{ WM_INITDIALOG,_T("WM_INITDIALOG") },
	{ WM_COMMAND,_T("WM_COMMAND") },
	{ WM_SYSCOMMAND,_T("WM_SYSCOMMAND") },
	{ WM_TIMER,_T("WM_TIMER") },
	{ WM_HSCROLL,_T("WM_HSCROLL") },
	{ WM_VSCROLL,_T("WM_VSCROLL") },
	{ WM_INITMENU,_T("WM_INITMENU") },
	{ WM_INITMENUPOPUP,_T("WM_INITMENUPOPUP") },
	{ WM_MENUSELECT,_T("WM_MENUSELECT") },
	{ WM_MENUCHAR,_T("WM_MENUCHAR") },
	{ WM_ENTERIDLE,_T("WM_ENTERIDLE") },
	{ WM_CTLCOLORMSGBOX,_T("WM_CTLCOLORMSGBOX") },
	{ WM_CTLCOLOREDIT,_T("WM_CTLCOLOREDIT") },
	{ WM_CTLCOLORLISTBOX,_T("WM_CTLCOLORLISTBOX") },
	{ WM_CTLCOLORBTN,_T("WM_CTLCOLORBTN") },
	{ WM_CTLCOLORDLG,_T("WM_CTLCOLORDLG") },
	{ WM_CTLCOLORSCROLLBAR,_T("WM_CTLCOLORSCROLLBAR") },
	{ WM_CTLCOLORSTATIC,_T("WM_CTLCOLORSTATIC") },
	{ WM_MOUSEMOVE,_T("WM_MOUSEMOVE") },
	{ WM_LBUTTONDOWN,_T("WM_LBUTTONDOWN") },
	{ WM_LBUTTONUP,_T("WM_LBUTTONUP") },
	{ WM_LBUTTONDBLCLK,_T("WM_LBUTTONDBLCLK") },
	{ WM_RBUTTONDOWN,_T("WM_RBUTTONDOWN") },
	{ WM_RBUTTONUP,_T("WM_RBUTTONUP") },
	{ WM_RBUTTONDBLCLK,_T("WM_RBUTTONDBLCLK") },
	{ WM_MBUTTONDOWN,_T("WM_MBUTTONDOWN") },
	{ WM_MBUTTONUP,_T("WM_MBUTTONUP") },
	{ WM_MBUTTONDBLCLK,_T("WM_MBUTTONDBLCLK") },
	{ WM_MOUSEMOVE,_T("WM_MOUSEMOVE") },
	{ WM_LBUTTONDOWN,_T("WM_LBUTTONDOWN") },
	{ WM_LBUTTONUP,_T("WM_LBUTTONUP") },
	{ WM_LBUTTONDBLCLK,_T("WM_LBUTTONDBLCLK") },
	{ WM_RBUTTONDOWN,_T("WM_RBUTTONDOWN") },
	{ WM_RBUTTONUP,_T("WM_RBUTTONUP") },
	{ WM_RBUTTONDBLCLK,_T("WM_RBUTTONDBLCLK") },
	{ WM_MBUTTONDOWN,_T("WM_MBUTTONDOWN") },
	{ WM_MBUTTONUP,_T("WM_MBUTTONUP") },
	{ WM_MBUTTONDBLCLK,_T("WM_MBUTTONDBLCLK") },
	{ WM_PARENTNOTIFY,_T("WM_PARENTNOTIFY") },
	{ WM_ENTERMENULOOP,_T("WM_ENTERMENULOOP") },
	{ WM_EXITMENULOOP,_T("WM_EXITMENULOOP") },
#if(WINVER >= 0x0400)
	{ WM_NEXTMENU,_T("WM_NEXTMENU") },
	{ WM_SIZING,_T("WM_SIZING") },
	{ WM_CAPTURECHANGED,_T("WM_CAPTURECHANGED") },
	{ WM_MOVING,_T("WM_MOVING") },
	{ WM_POWERBROADCAST,_T("WM_POWERBROADCAST") },
	{ WM_DEVICECHANGE,_T("WM_DEVICECHANGE") },
	{ WM_IME_SETCONTEXT,_T("WM_IME_SETCONTEXT") },
	{ WM_IME_NOTIFY,_T("WM_IME_NOTIFY") },
	{ WM_IME_CONTROL,_T("WM_IME_CONTROL") },
	{ WM_IME_COMPOSITIONFULL,_T("WM_IME_COMPOSITIONFULL") },
	{ WM_IME_SELECT,_T("WM_IME_SELECT") },
	{ WM_IME_CHAR,_T("WM_IME_CHAR") },
	{ WM_IME_KEYDOWN,_T("WM_IME_KEYDOWN") },
	{ WM_IME_KEYUP,_T("WM_IME_KEYUP") },
#endif
	{ WM_MDICREATE,_T("WM_MDICREATE") },
	{ WM_MDIDESTROY,_T("WM_MDIDESTROY") },
	{ WM_MDIACTIVATE,_T("WM_MDIACTIVATE") },
	{ WM_MDIRESTORE,_T("WM_MDIRESTORE") },
	{ WM_MDINEXT,_T("WM_MDINEXT") },
	{ WM_MDIMAXIMIZE,_T("WM_MDIMAXIMIZE") },
	{ WM_MDITILE,_T("WM_MDITILE") },
	{ WM_MDICASCADE,_T("WM_MDICASCADE") },
	{ WM_MDIICONARRANGE,_T("WM_MDIICONARRANGE") },
	{ WM_MDIGETACTIVE,_T("WM_MDIGETACTIVE") },
	{ WM_MDISETMENU,_T("WM_MDISETMENU") },
	{ WM_ENTERSIZEMOVE,_T("WM_ENTERSIZEMOVE") },
	{ WM_EXITSIZEMOVE,_T("WM_EXITSIZEMOVE") },
	{ WM_DROPFILES,_T("WM_DROPFILES") },
	{ WM_MDIREFRESHMENU,_T("WM_MDIREFRESHMENU") },
	{ WM_CUT,_T("WM_CUT") },
	{ WM_COPY,_T("WM_COPY") },
	{ WM_PASTE,_T("WM_PASTE") },
	{ WM_CLEAR,_T("WM_CLEAR") },
	{ WM_UNDO,_T("WM_UNDO") },
	{ WM_RENDERFORMAT,_T("WM_RENDERFORMAT") },
	{ WM_RENDERALLFORMATS,_T("WM_RENDERALLFORMATS") },
	{ WM_DESTROYCLIPBOARD,_T("WM_DESTROYCLIPBOARD") },
	{ WM_DRAWCLIPBOARD,_T("WM_DRAWCLIPBOARD") },
	{ WM_PAINTCLIPBOARD,_T("WM_PAINTCLIPBOARD") },
	{ WM_VSCROLLCLIPBOARD,_T("WM_VSCROLLCLIPBOARD") },
	{ WM_SIZECLIPBOARD,_T("WM_SIZECLIPBOARD") },
	{ WM_ASKCBFORMATNAME,_T("WM_ASKCBFORMATNAME") },
	{ WM_CHANGECBCHAIN,_T("WM_CHANGECBCHAIN") },
	{ WM_HSCROLLCLIPBOARD,_T("WM_HSCROLLCLIPBOARD") },
	{ WM_QUERYNEWPALETTE,_T("WM_QUERYNEWPALETTE") },
	{ WM_PALETTEISCHANGING,_T("WM_PALETTEISCHANGING") },
	{ WM_PALETTECHANGED,_T("WM_PALETTECHANGED") },
	{ WM_HOTKEY,_T("WM_HOTKEY") },
#if(WINVER >= 0x0400)
	{ WM_PRINT,_T("WM_PRINT") },
	{ WM_PRINTCLIENT,_T("WM_PRINTCLIENT") },
#endif
	// Below are MFC messages
	{ WM_QUERYAFXWNDPROC,_T("*WM_QUERYAFXWNDPROC") },
	{ WM_SIZEPARENT,_T("*WM_SIZEPARENT") },
	{ WM_SETMESSAGESTRING,_T("*WM_SETMESSAGESTRING") },
	{ WM_IDLEUPDATECMDUI,_T("*WM_IDLEUPDATECMDUI") },
	{ WM_INITIALUPDATE,_T("*WM_INITIALUPDATE") },
	{ WM_COMMANDHELP,_T("*WM_COMMANDHELP") },
	{ WM_HELPHITTEST,_T("*WM_HELPHITTEST") },
	{ WM_EXITHELPMODE,_T("*WM_EXITHELPMODE") },
	{ WM_RECALCPARENT,_T("*WM_RECALCPARENT") },
	{ WM_SIZECHILD,_T("*WM_SIZECHILD") },
	{ WM_KICKIDLE,_T("*WM_KICKIDLE") },
	{ WM_QUERYCENTERWND,_T("*WM_QUERYCENTERWND") },
	{ WM_DISABLEMODAL,_T("*WM_DISABLEMODAL") },
	{ WM_FLOATSTATUS,_T("*WM_FLOATSTATUS") },
	{ WM_ACTIVATETOPLEVEL,_T("*WM_ACTIVATETOPLEVEL") },
//VS.NET	{ WM_QUERY3DCONTROLS,_T("*WM_QUERY3DCONTROLS") },
	{ WM_SOCKET_NOTIFY,_T("*WM_SOCKET_NOTIFY") },
	{ WM_SOCKET_DEAD,_T("*WM_SOCKET_DEAD") },
	{ WM_POPMESSAGESTRING,_T("*WM_POPMESSAGESTRING") },
	{ WM_OCC_LOADFROMSTREAM,_T("*WM_OCC_LOADFROMSTREAM") },
	{ WM_OCC_LOADFROMSTORAGE,_T("*WM_OCC_LOADFROMSTORAGE") },
	{ WM_OCC_INITNEW,_T("*WM_OCC_INITNEW") },
	{ WM_QUEUE_SENTINEL,_T("*WM_QUEUE_SENTINEL") },
	{ 0,NULL }
};

////////////////
// This class is basically just an array of 1024 strings,
// the names of each WM_ message. Constructor initializes it.
//
class CWndMsgMap {
	static LPCTSTR Names[];				// array of WM_ message names
public:
	CWndMsgMap();							// constructor initializes them
	CString GetMsgName(UINT msg);		// get name of message
};
LPCTSTR CWndMsgMap::Names[WM_USER];	// name of each WM_ message

//////////////////
// Initialize array from sparse data
//
CWndMsgMap::CWndMsgMap()
{
	// copy sparse MsgData into table
	memset(Names, 0, sizeof(Names));
	for (int i=0; MsgData[i].msg; i++)		
		Names[MsgData[i].msg] = MsgData[i].name;
}

////////////////
// Get the name of a WM_ message
//
CString CWndMsgMap::GetMsgName(UINT msg)
{
	CString name;
	if (msg>=WM_USER)
		name.Format(_T("WM_USER+%d"), msg-WM_USER);
	else if (Names[msg])
		name = Names[msg];
	else
		name.Format(_T("0x%04x"), msg);
	return name;
}

//////////////////
// Get name of WM_ message.
//
CString _DbgName(UINT uMsg)
{
	static CWndMsgMap wndMsgMap; // instantiate 1st time called
	return wndMsgMap.GetMsgName(uMsg);
}

CString _DbgName(SCODE sc)
{
	extern LPCTSTR AFXAPI AfxGetScodeString(SCODE sc);
	CString s = AfxGetScodeString(sc);
	if (s.IsEmpty())
		s.Format(_T("0x%x"),sc);
	return s;
}

// Used for debugging--Interfaces I want to watch
DEBUG_BEGIN_INTERFACE_NAMES()
	DEBUG_INTERFACE_NAME(IUnknown)
	DEBUG_INTERFACE_NAME(IOleWindow)
	DEBUG_INTERFACE_NAME(IDockingWindow)
	DEBUG_INTERFACE_NAME(IObjectWithSite)
	DEBUG_INTERFACE_NAME(IClassFactory)
	DEBUG_INTERFACE_NAME(IPersistFile)
	DEBUG_INTERFACE_NAME(IMarshal)
	DEBUG_INTERFACE_NAME(IMalloc)
	DEBUG_INTERFACE_NAME(IMallocSpy)
	DEBUG_INTERFACE_NAME(IStdMarshalInfo)
	DEBUG_INTERFACE_NAME(IExternalConnection)
	DEBUG_INTERFACE_NAME(IMultiQI)
	DEBUG_INTERFACE_NAME(IEnumUnknown)
	DEBUG_INTERFACE_NAME(IBindCtx)
	DEBUG_INTERFACE_NAME(IEnumMoniker)
	DEBUG_INTERFACE_NAME(IRunnableObject)
	DEBUG_INTERFACE_NAME(IRunningObjectTable)
	DEBUG_INTERFACE_NAME(IPersist)
	DEBUG_INTERFACE_NAME(IPersistStream)
	DEBUG_INTERFACE_NAME(IMoniker)
	DEBUG_INTERFACE_NAME(IROTData)
	DEBUG_INTERFACE_NAME(IEnumString)
	DEBUG_INTERFACE_NAME(ISequentialStream)
	DEBUG_INTERFACE_NAME(IStream)
	DEBUG_INTERFACE_NAME(IPersistStreamInit)
	DEBUG_INTERFACE_NAME(IEnumSTATSTG)
	DEBUG_INTERFACE_NAME(IStorage)
	DEBUG_INTERFACE_NAME(IPersistFile)
	DEBUG_INTERFACE_NAME(IPersistStorage)
	DEBUG_INTERFACE_NAME(ILockBytes)
	DEBUG_INTERFACE_NAME(IEnumFORMATETC)
	DEBUG_INTERFACE_NAME(IEnumSTATDATA)
	DEBUG_INTERFACE_NAME(IRootStorage)
	DEBUG_INTERFACE_NAME(IAdviseSink)
	DEBUG_INTERFACE_NAME(IAdviseSink2)
	DEBUG_INTERFACE_NAME(IDataObject)
	DEBUG_INTERFACE_NAME(IDataAdviseHolder)
	DEBUG_INTERFACE_NAME(IMessageFilter)
	DEBUG_INTERFACE_NAME(IRpcChannelBuffer)
	DEBUG_INTERFACE_NAME(IRpcProxyBuffer)
	DEBUG_INTERFACE_NAME(IRpcStubBuffer)
	DEBUG_INTERFACE_NAME(IPSFactoryBuffer)
	DEBUG_INTERFACE_NAME(IPropertyStorage)
	DEBUG_INTERFACE_NAME(IPropertySetStorage)
	DEBUG_INTERFACE_NAME(IEnumSTATPROPSTG)
	DEBUG_INTERFACE_NAME(IEnumSTATPROPSETSTG)
	DEBUG_INTERFACE_NAME(IFillLockBytes)
	DEBUG_INTERFACE_NAME(IProgressNotify)
	DEBUG_INTERFACE_NAME(ILayoutStorage)
	DEBUG_INTERFACE_NAME(IEnumConnections)
	DEBUG_INTERFACE_NAME(IConnectionPoint)
	DEBUG_INTERFACE_NAME(IEnumConnectionPoints)
	DEBUG_INTERFACE_NAME(IConnectionPointContainer)
	DEBUG_INTERFACE_NAME(IClassFactory2)
	DEBUG_INTERFACE_NAME(IProvideClassInfo)
	DEBUG_INTERFACE_NAME(IProvideClassInfo2)
	DEBUG_INTERFACE_NAME(IProvideMultipleClassInfo)
	DEBUG_INTERFACE_NAME(IOleControl)
	DEBUG_INTERFACE_NAME(IOleControlSite)
	DEBUG_INTERFACE_NAME(IPropertyPage)
	DEBUG_INTERFACE_NAME(IPropertyPage2)
	DEBUG_INTERFACE_NAME(IPropertyPageSite)
	DEBUG_INTERFACE_NAME(IPropertyNotifySink)
	DEBUG_INTERFACE_NAME(ISpecifyPropertyPages)
	DEBUG_INTERFACE_NAME(IPersistMemory)
	DEBUG_INTERFACE_NAME(IPersistStreamInit)
	DEBUG_INTERFACE_NAME(IPersistPropertyBag)
	DEBUG_INTERFACE_NAME(ISimpleFrameSite)
	DEBUG_INTERFACE_NAME(IFont)
	DEBUG_INTERFACE_NAME(IPicture)
	DEBUG_INTERFACE_NAME(IFontEventsDisp)
	DEBUG_INTERFACE_NAME(IFontDisp)
	DEBUG_INTERFACE_NAME(IPictureDisp)
	DEBUG_INTERFACE_NAME(IOleInPlaceObjectWindowless)
	DEBUG_INTERFACE_NAME(IOleInPlaceSiteEx)
	DEBUG_INTERFACE_NAME(IOleInPlaceSiteWindowless)
	DEBUG_INTERFACE_NAME(IViewObjectEx)
	DEBUG_INTERFACE_NAME(IOleUndoUnit)
	DEBUG_INTERFACE_NAME(IOleParentUndoUnit)
	DEBUG_INTERFACE_NAME(IEnumOleUndoUnits)
	DEBUG_INTERFACE_NAME(IOleUndoManager)
	DEBUG_INTERFACE_NAME(IPointerInactive)
	DEBUG_INTERFACE_NAME(IObjectWithSite)
	DEBUG_INTERFACE_NAME(IErrorLog)
	DEBUG_INTERFACE_NAME(IPropertyBag)
	DEBUG_INTERFACE_NAME(IPerPropertyBrowsing)
	DEBUG_INTERFACE_NAME(IPropertyBag2)
	DEBUG_INTERFACE_NAME(IPersistPropertyBag2)
	DEBUG_INTERFACE_NAME(IAdviseSinkEx)
	DEBUG_INTERFACE_NAME(IQuickActivate)
	DEBUG_INTERFACE_NAME(IOleDocument)
	DEBUG_INTERFACE_NAME(IOleDocumentSite)
	DEBUG_INTERFACE_NAME(IOleDocumentView)
	DEBUG_INTERFACE_NAME(IEnumOleDocumentViews)
	DEBUG_INTERFACE_NAME(IContinueCallback)
	DEBUG_INTERFACE_NAME(IPrint)
	DEBUG_INTERFACE_NAME(IOleCommandTarget)
	DEBUG_INTERFACE_NAME(IDiscardableBrowserProperty)
	DEBUG_INTERFACE_NAME(IFileViewerW)
	DEBUG_INTERFACE_NAME(IShellLinkW)
	DEBUG_INTERFACE_NAME(IExtractIconW)
	DEBUG_INTERFACE_NAME(IShellExecuteHookW)
	DEBUG_INTERFACE_NAME(INewShortcutHookW)
	DEBUG_INTERFACE_NAME(IFileViewerA)
	DEBUG_INTERFACE_NAME(IShellLinkA)
	DEBUG_INTERFACE_NAME(IExtractIconA)
	DEBUG_INTERFACE_NAME(IShellExecuteHookA)
	DEBUG_INTERFACE_NAME(INewShortcutHookA)
DEBUG_END_INTERFACE_NAMES()

CInterfaceNames* CInterfaceNames::s_pFirst = NULL;

CInterfaceNames::CInterfaceNames(DBGINTERFACENAME* pdin, UINT n)
{
	ASSERT(pdin);
	m_pEntries = pdin;
	m_pNext    = s_pFirst;
	s_pFirst   = this;
	m_nEntries = n;
}

const DBGINTERFACENAME* CInterfaceNames::FindEntry(REFIID iid)
{
	for (CInterfaceNames* pNames = s_pFirst;
		  pNames;
		  pNames=pNames->m_pNext) {

		for (UINT i=0; i<pNames->m_nEntries; i++) {
			const DBGINTERFACENAME* pdin = &pNames->m_pEntries[i];
			if (memcmp(pdin->piid, &iid, sizeof(IID))==0) {
				return pdin;
			}
		}
	}
	return NULL;
}	
		  
//////////////////
// Get human-readable COM interface name.
//
CString _DbgName(REFIID iid)
{
	CString s;

	const DBGINTERFACENAME* pdin = CInterfaceNames::FindEntry(iid);
	if (pdin)
		s = pdin->name;
	else {
		LPOLESTR lp=NULL;
		StringFromCLSID(iid, &lp);
		s = lp;
	}
	return s;
}

#endif // DEBUG

