//---------------------------------------------------------------------------
// (c) 2002-2010 12noon, Stefan K. S. Tucker
// (c) 1991-2002 Perpetual Motion Software, Stefan K. S. Tucker
//---------------------------------------------------------------------------

#include "stdafx.h"
#include <cderr.h>      // for FNERR_INVALIDFILENAME
#include <afxwinappex.h>
#include <afxcontextmenumanager.h>
#include <afxpopupmenu.h>

#include "MyWin/mylog.h"
#include "MyWin/versions.h"
#include "MyWin/misc.h"

#include "pmslib.h"


//MORE MODERN ALTERNATIVE (Tests/Console-MFC/)
//   TCHAR szPath[_MAX_PATH];
//// //const HRESULT hr =
//	if (::SHGetSpecialFolderPath(::AfxGetMainWnd()->GetSafeHwnd(), szPath, CSIDL_PERSONAL, FALSE /*no create*/))
//		std::cout << _T("SHGetSpecialFolderPath() returned '") << szPath << _T("'") << std::endl;
//
//
//
//	/*
//		Managing the File System
//		http://msdn.microsoft.com/en-us/library/bb776887(VS.85).aspx
//	*/
//	IShellFolder *psfDesktop;
//	if (::SHGetDesktopFolder(&psfDesktop) != S_OK)
//		return;
////	CComQIPtr<IShellFolder> sh(psfDesktop);
//
//
//	//ULONG chEaten;
//	PIDLIST_RELATIVE pidlDocFiles;
//	HRESULT hr = psfDesktop->ParseDisplayName(NULL /*HWND*/,
//															NULL /* IBindCtx* */,
//															L"::{450d8fba-ad25-11d0-98a8-0800361b1103}",
//															NULL /*chEaten*/,
//															&pidlDocFiles,
//															NULL /*pdwAttributes*/);
//	if (hr != S_OK)
//	{
//		psfDesktop->Release();
//		return;
//	}
//
//	STRRET strret;
//	psfDesktop->GetDisplayNameOf(pidlDocFiles, SHGDN_FORPARSING, &strret);
//
//	if (::StrRetToBuf(&strret, pidlDocFiles, szPath, _countof(szPath)) == S_OK)
//		std::cout << _T("ParseDisplayName() returned '") << szPath << _T("'") << std::endl;
//
//	psfDesktop->Release();

CString PMS::GetFolderPathMyDocuments()
{
	/*
		SHGetSpecialFolderPath() is deprecated as of Windows 2000, but SHGetFolderPath()
		can't provide the actual My Documents path (only virtual).
	*/
	//::SHGetFolderPath(NULL /*hWndOwner - reserved*/, CSIDL_MYDOCUMENTS, NULL /*hToken*/, 0 /*dwFlags*/, szPath);
   TCHAR szPath[_MAX_PATH];
	if (!::SHGetSpecialFolderPath(nullptr /*hWndOwner - reserved*/, szPath, CSIDL_MYDOCUMENTS, FALSE /*no create*/))
	{
		skst::MyLog::GetLog().LogV(_T("SHGetSpecialFolderPath(CSIDL_MYDOCUMENTS) failed"));
		return _T("");
	}

	skst::MyLog::GetLog().LogV(_T("SHGetSpecialFolderPath(CSIDL_MYDOCUMENTS) returned '%s'"), szPath);
	return szPath;
}
CString PMS::GetFolderPathDesktop()
{
   TCHAR szPath[_MAX_PATH];
	if (!::SHGetSpecialFolderPath(nullptr /*hWndOwner - reserved*/, szPath, CSIDL_DESKTOPDIRECTORY, FALSE /*no create*/))
	{
		skst::MyLog::GetLog().LogV(_T("SHGetSpecialFolderPath(CSIDL_DESKTOPDIRECTORY) failed"));
		return _T("");
	}

	/*
		We can't log here because we call this to set the log file
		path and accessing the log will create the log file.
	*/
	//skst::MyLog::GetLog().LogV(_T("SHGetSpecialFolderPath(CSIDL_DESKTOPDIRECTORY) returned '%s'"), szPath);
	return szPath;
}


CString PMS::BinStrToHexCStr(tstring s)
{
   //---------------------------------------------------
   // convert the binary stream to a hex representation
   //---------------------------------------------------
   CString strResult;
   for (tstring::iterator i = s.begin(); i < s.end(); ++i)
   {
      TCHAR sTmp[3];
      _stprintf_s(sTmp, _countof(sTmp), _T("%02X"), i[0] & 0xff);
      strResult += sTmp;
   }

   return strResult;
}

static int HexToBin(const TCHAR c)
{
   if ((c >= _T('0')) && (c <= _T('9')))
      return (c - '0');

   if ((c >= _T('A')) && (c <= _T('F')))
      return (c - 'A' + 10);

   ASSERT(false);
   return 0;
}

tstring PMS::HexCStrToBinStr(const CString& s)
{
   //---------------------------------------------------
   // convert the hex string to a binary representation
   //---------------------------------------------------
   tstring strResult;
   for (int i = 0; i < s.GetLength(); i += 2)
   {
      int c = ::HexToBin(s[i]) * 16;
      c += ::HexToBin(s[i + 1]);
      strResult += TCHAR(c & 0xFF);
   }

   return strResult;
}


//---------------------------------------------------------------------------
/*
   This routine adds commas (or whatever the system separator is) to the
   passed string (which is presumably a number).
*/
//---------------------------------------------------------------------------
CString
PMS::AddCommas(CString& s)   // string to get commas
{
   /*
      format the number according to user's locale
   */
   const int nChars = ::GetNumberFormat(LOCALE_USER_DEFAULT, 0, s, nullptr, nullptr, 0);
   TCHAR *sz = new TCHAR[nChars];
   ::GetNumberFormat(LOCALE_USER_DEFAULT, 0, s, nullptr, sz, nChars);
   s = sz;
   delete [] sz;

   // result: "12,345,678.00"

   /*
      strip decimal and following digits
   */
   // get decimal for locale
   TCHAR szPt[5];
   ::GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_SDECIMAL, szPt, sizeof szPt);
   const int nPos = s.Find(szPt);
   if (nPos != -1)
      s.Delete(nPos, s.GetLength() - nPos);

   return s;

#if 0
   /*
      get thousands separator for locale
   */
   TCHAR szSep[5];
   ::GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_STHOUSAND, szSep, sizeof szSep);

   /*
      get size of thousands group (e.g., US = 3)
      "3;2;0" == "12,34,56,789"
   */
   TCHAR szGroups[10];
   ::GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_SGROUPING, szGroups, sizeof szGroups);

   CString src(s);   // make a copy from which to extract characters

   s.Empty();        // wipe out the destination string

   /*
      process the string in groups of "the current size of a group" (N)
   */
   int iGroupSize = szGroups[0] - '0';
   while (src.GetLength() > iGroupSize)
   {
      // prepend separator & last N characters in the original string
      s = szSep + src.Right(iGroupSize) + s;

      // strip off those last N characters
      src.Delete(src.GetLength() - iGroupSize, iGroupSize);
//TO DO: if the next group size > 0, get the next group size
   }

   //------------------------------------------
   // prepend what's left in the source string
   //------------------------------------------
   s = src + s;

   return s;
#endif
}

CString FormatNumberWithCommas(const int n)
{
   CString s;
   s.Format(_T("%d"), n);
   return PMS::AddCommas(s);
}


/*
   This function opens a passed URL in the appropriate application.
*/
bool
PMS::OpenURL(LPCTSTR szURL)
{
	skst::MyLog::GetLog().LogV(_T(__FUNCTION__) _T("(%s)"), szURL);
	const HINSTANCE h = ::ShellExecute(nullptr, _T("open"),
													szURL, nullptr /*args*/, nullptr /*dir*/,
													SW_SHOWNORMAL);
	return (h >= HINSTANCE(HINSTANCE_ERROR));
}


//---------------------------------------------------------------------------
// This routine draws the passed bitmap at the top of the passed
// device-context.  It centers the bitmap in the middle of the passed width.
//---------------------------------------------------------------------------
void PMS::PMSDrawBitmap(CDC *pdc, int iWidth, CBitmap *pcBmp)
{
   BITMAP tBitmap;
   pcBmp->GetObject(sizeof(tBitmap), (void *) &tBitmap);

   CDC cMemDC;
   cMemDC.CreateCompatibleDC(pdc);

   CBitmap *pOldBmp = cMemDC.SelectObject(pcBmp);
   if (pOldBmp != nullptr)
   {
      CPoint cOrg(0, 0);
      cMemDC.DPtoLP(&cOrg);

      CPoint cSize(tBitmap.bmWidth, tBitmap.bmHeight);
      cMemDC.DPtoLP(&cSize);

      pdc->BitBlt((iWidth - tBitmap.bmWidth) / 2, 1,
                  cSize.x, cSize.y,
                  &cMemDC, cOrg.x, cOrg.y,
                  SRCCOPY);
   }
   cMemDC.SelectObject(pOldBmp);
}


/*
   This routine returns the file path if the user selects a file.
   The starting path is the value of the passed control. If the user
   presses OK, the selected path is stored back into the passed control.

   The passed ids are for string resources.
      default extension:   "exe"
      filters:             "Sounds (*.wav)|*.wav|All Files (*.*)|*.*||"
      title:               "Select File"
      flags:               any in addition to OFN_HIDEREADONLY | OFN_FILEMUSTEXIST
*/
CString PMS::BrowseForFileWithControl(CWnd& wnd,
                                       const UINT idDefExt,
                                       const UINT idFilters,
                                       const UINT idTitle,
                                       const DWORD fgsExtra)
{
   skst::LogInOut tio(_T(__FUNCTION__));

   /*
      First, try starting with the path in the control.
      The user might have pasted something in there and want to use it.
   */
   CString strPathCur;
   wnd.GetWindowText(strPathCur);

   const CString strPath(PMS::BrowseForFile(strPathCur, *wnd.GetParent(), idDefExt, idFilters, idTitle, fgsExtra));

   wnd.SetWindowText(strPath);
   return strPath;
}


/*
   See header for BrowseForFileWithControl().
*/
CString PMS::BrowseForFile(LPCTSTR szPathCur,
                           CWnd& wndParent,
                           const UINT idDefExt,
                           const UINT idFilters,
                           const UINT idTitle,
                           const DWORD fgsExtra)
{
   skst::LogInOut tio(_T(__FUNCTION__));

   const CString strExt(MAKEINTRESOURCE(idDefExt));
   const CString strFilters(MAKEINTRESOURCE(idFilters));

   skst::MyLog::GetLog().LogV(_T("\tInitial path: '%s'"), szPathCur);

   /*
      If the initial path ends with a backslash, CFileDialog chokes.
      If the initial file path is a folder, CFileDialog enters the parent
      folder and displays the folder name as the selected file. Oh well.
   */
   CString strPath(szPathCur);
   // "strPath[strPath.GetLength() - 1]" is equivalent to "strPath.Right(1)" but more efficient
   if ((strPath.GetLength() > 0) && (strPath[strPath.GetLength() - 1] == _T('\\')))
      strPath.Delete(strPath.GetLength() - 1);

   CFileDialog cf(TRUE, strExt,   // extension appended if user doesn't do one
                  strPath,
                  OFN_HIDEREADONLY | OFN_FILEMUSTEXIST | fgsExtra,
                  strFilters, &wndParent,
                  sizeof OPENFILENAME);
                  // if we don't do this, it won't work on 98
   const CString strTitle(MAKEINTRESOURCE(idTitle));
   cf.GetOFN().lpstrTitle = strTitle;
   // TODO: could set lpstrInitialDir here (e.g., only pass in initial path, not filename)

   if (cf.DoModal() == IDOK)
      strPath = cf.GetPathName();
   else
   {
      /*
         if there's an error, it could be because the incoming path is invalid.
         If so, we try again with an empty string.
      */
      const DWORD dwError = ::CommDlgExtendedError();
      if (dwError == 0)
      {
         // Everything's fine. The user just hit Cancel.
         // use initial path
      }
      else if (dwError == FNERR_INVALIDFILENAME)
      {
         skst::MyLog::GetLog().LogV(_T("\tInvalid filename '%s'. Trying again with initial path empty."), (LPCTSTR) strPath);

         CFileDialog cf2(TRUE, strExt,   // extension appended if user doesn't do one
                         _T(""),
                         OFN_HIDEREADONLY | OFN_FILEMUSTEXIST | fgsExtra,
                         strFilters, &wndParent,
                         sizeof OPENFILENAME);
                         // if we don't do this, it won't work on 98
         cf2.GetOFN().lpstrTitle = strTitle;
         if (cf2.DoModal() == IDOK)
            strPath = cf2.GetPathName();
         // else, use initial path
      }
      else
      {
         skst::MyLog::GetLog().LogV(_T("\tCommDlgExtendedError(): %s"), skst::GetCommDlgExtendedErrorString().c_str());
         // use initial path
      }
   }

   skst::MyLog::GetLog().LogV(_T("\tResult path: '%s'"), (LPCTSTR) strPath);
   // The returned path can be empty if the initial path is empty and the user hits Cancel.
   return strPath;
}


/*
   This routine invokes the specified command.
*/
void PMS::InvokeMenuCmd(CWnd *pWnd, const int idCmd)
{
   if (idCmd != 0)
	   pWnd->SendMessage(WM_COMMAND, MAKEWPARAM(idCmd, 0));
}


//---------------------------------------------------------------------------
/*
   This routine displays a popup menu.  The menu is indicated by a
   passed menu id and position.
   If a point is passed, it is positioned at those *screen* coordinates.
   It also sets the specified menu item to be the default (bold).
   If the flag is set, indicating this menu is being displayed by
   a system tray icon, it does a few workarounds for some MS bugs.
*/
//---------------------------------------------------------------------------
void PMS::DisplayPopupMenu(CWnd *pWnd, const UINT idMenu, const UINT idDefaultCmd, const CPoint *pPoint)
{
   ASSERT(pWnd != nullptr);

   CMenu menu;
   VERIFY(menu.LoadMenu(idMenu));

   // assume the menu is the first menu on the menu bar
   CMenu *pSubMenu = menu.GetSubMenu(0);
   ASSERT(pSubMenu != nullptr);
	ASSERT(::IsMenu(pSubMenu->m_hMenu));

   // make passed menu item bold
   if (idDefaultCmd != 0)
      pSubMenu->SetDefaultItem(idDefaultCmd /*id*/, FALSE /*bypos*/);

   /*
		PRB: Menus for Notification Icons Do Not Work Correctly
		http://support.microsoft.com/kb/135788

      If a tray icon is putting up a context menu, we have to do a little
      dance around what Microsoft calls a "feature." A Windows 95 bug that
      requires us to call this and the PostMessage() later. Q135788
      Thanks, Microsoft!!
      (The workaround is harmless when we're bringing up a "regular"
      menu --as opposed to a tray menu-- so we do it all the time.)
   */
   pWnd->SetForegroundWindow();

   // if there is no passed point, get the mouse position
   CPoint point;
   if (pPoint == nullptr)
   {
      ::GetCursorPos(&point);
//TODO: If this menu is displayed via the keyboard, we should probably use the center of the parent wnd
//       unfortunately, this particular code path is also used when the user right-clicks on the tray icon
/*
      CRect r;
      pWnd->GetClientRect(r);
      pWnd->ClientToScreen(r);
      point = r.CenterPoint();
*/
   }
   else
      point = *pPoint;


	// display menu (with icons)
	/*
		HOWTO: How to Use TrackPopupMenu() and Update UI Handlers
		http://support.microsoft.com/kb/139469
	*/
	// WAS: pSubMenu->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y, pWnd);
	const UINT idCmd = dynamic_cast<CWinAppEx*>(::AfxGetApp())->GetContextMenuManager()->TrackPopupMenu(pSubMenu->Detach(), point.x, point.y, pWnd);
	// process selection
	if (idCmd == 0)
		return;

	pWnd->PostMessage(WM_COMMAND, MAKEWPARAM(idCmd, 0), 0);

	pWnd->PostMessage(WM_NULL);
}


//---------------------------------------------------------------------------
/*
   This routine exchanges dialog data for a spin button.
*/
//---------------------------------------------------------------------------

void PMS::DDX_SpinButton(CDataExchange *pDX, int nIDC, int& value)
{
	HWND hWndCtrl = pDX->PrepareCtrl(nIDC);
	if (pDX->m_bSaveAndValidate)
		value = (int) LOWORD(::SendMessage(hWndCtrl, UDM_GETPOS, 0, 0l));
	else
		::SendMessage(hWndCtrl, UDM_SETPOS, 0, MAKELPARAM((short) value, 0));
}

void PMS::DDX_Check(CDataExchange* pDX, int nIDC, bool& value)
{
	pDX->PrepareCtrl(nIDC);
   HWND hWndCtrl;
   pDX->m_pDlgWnd->GetDlgItem(nIDC, &hWndCtrl);
	if (pDX->m_bSaveAndValidate)
	{
		const int n = (int)::SendMessage(hWndCtrl, BM_GETCHECK, 0, 0L);
      if (n == BST_UNCHECKED)
         value = false;
      else if (n == BST_CHECKED)
         value = true;
      else
		   ASSERT(FALSE);
	}
	else
	{
      if (value == BST_UNCHECKED)
         ::SendMessage(hWndCtrl, BM_SETCHECK, 0, 0L);
      else if (value == BST_CHECKED)
         ::SendMessage(hWndCtrl, BM_SETCHECK, 1, 0L);
      else
      {
			TRACE(traceAppMsg, 0, "Warning: dialog data checkbox value (%d) out of range.\n", value);
			value = 0;  // default to off
         ::SendMessage(hWndCtrl, BM_SETCHECK, 0, 0L);
		}
	}
}
