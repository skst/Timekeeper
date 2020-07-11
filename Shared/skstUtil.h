//---------------------------------------------------------------------------
// (c) 2001-2017 12noon, Perpetual Motion Software, Stefan K. S. Tucker
//---------------------------------------------------------------------------

#pragma once

#if defined(ASSERT)
#undef assert
#define assert ASSERT
#else
#include <assert.h>
#endif

//#if defined(_SHLOBJ_H_) && !defined(_WININET_)
//#  pragma message("You must #include <wininet.h> before ANY shlobj.h (e.g., afxdisp.h, afxdtctl.h) or compile error in ActiveDesktop for COMPONENTSOPT")
//#endif

#include <objbase.h>			// CoInitializeEx()                                   ShellLink, Registrar
#include <atlbase.h>		   // ATL stuff (CComQIPtr)                              ShellLink, Registrar, ActiveDesktop
/*
	I uninstalled the Platform SDK (Windows Server 2003 R2) and this file went away.
	I don't know if there are any undesirable side effects.
	-skst (26 Sep 2008)
*/
//#include <\Program Files\Microsoft Platform SDK\Include\ComDef.h>
#include <..\include\comdef.h>   // IShellLink                                   ShellLink
#include <wininet.h>       // Must be included before ANY shlobj.h               ActiveDesktop
#include <shlobj.h>        // IShellLink (includes shlguid.h for shell GUIDs)    ActiveDesktop, ShellLink, SHGetFolderPath in ::MyGetFolderPath
#include <shellapi.h>      // HDROP                                              ::GetDroppedFilePath

#pragma warning(push,3)
#include <string>
#pragma warning(disable : 4018)
#include <vector>
#pragma warning(pop)


/*
	Create flexible string class.
	http://msdn.microsoft.com/en-us/magazine/cc188714.aspx
*/
#if defined(_UNICODE)
typedef std::wstring tstring;
#else
typedef std::string tstring;
#endif


//-----------------------------------------------------------------
// If the app including this file is using the Shared MFC library,
// link to the version of skstlib that was compiled that way.
//-----------------------------------------------------------------
#if defined(_WIN64)
	#if defined(_AFXDLL)
		#if defined(_UNICODE)
			#error __FILE__ ": Must add UNICODE to these ifdefs to get the correct library"
		#endif
		#if defined(_DEBUG)
			#pragma message("Using MyWin MFC Shared DLL 64-bit Debug...")
			#pragma comment(lib, "MyWinx64HD.lib")
		#else
			#pragma message("Using MyWin MFC Shared DLL 64-bit Release...")
			#pragma comment(lib, "MyWinx64H.lib")
		#endif
	#else
		#if defined(_DEBUG)
			#if defined(UNICODE)
				#pragma message("Using MyWin Static Lib 64-bit Unicode Debug...")
				#pragma comment(lib, "MyWinx64UD.lib")
			#else
				#pragma message("Using MyWin Static Lib 64-bit Debug...")
				#pragma comment(lib, "MyWinx64D.lib")
			#endif
		#else
			#if defined(UNICODE)
				#pragma message("Using MyWin Static Lib 64-bit Unicode Release...")
				#pragma comment(lib, "MyWinx64U.lib")
			#else
				#pragma message("Using MyWin Static Lib 64-bit Release...")
				#pragma comment(lib, "MyWinx64.lib")
			#endif
		#endif
	#endif
#else
	#if defined(_AFXDLL)
		#if defined(_UNICODE)
			#error __FILE__ ": Must add UNICODE to these ifdefs to get the correct library"
		#endif
		#if defined(_DEBUG)
			#pragma message("Using MyWin MFC Shared DLL Debug...")
			#pragma comment(lib, "MyWinHD.lib")
		#else
			#pragma message("Using MyWin MFC Shared DLL Release...")
			#pragma comment(lib, "MyWinH.lib")
		#endif
	#else
		#if defined(_UNICODE)
			#if defined(_DEBUG)
				#pragma message("Using MyWin Static Lib Unicode Debug...")
				#pragma comment(lib, "MyWinUD.lib")
			#else
				#pragma message("Using MyWin Static Lib Unicode Release...")
				#pragma comment(lib, "MyWinU.lib")
			#endif
		#else
			#if defined(_DEBUG)
				#pragma message("Using MyWin Static Lib Debug...")
				#pragma comment(lib, "MyWinD.lib")
			#else
				#pragma message("Using MyWin Static Lib Release...")
				#pragma comment(lib, "MyWin.lib")
			#endif
		#endif
	#endif
#endif
