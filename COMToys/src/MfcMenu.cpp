////////////////////////////////////////////////////////////////
// ComToys(TM) Copyright 1999 Paul DiLascia
// If this code works, it was written by Paul DiLascia.
// If not, I don't know who wrote it.
//
#include "StdAfx.h"
#include "COMtoys/ComToys.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

DEBUG_BEGIN_INTERFACE_NAMES()
	DEBUG_INTERFACE_NAME(IContextMenu)
	DEBUG_INTERFACE_NAME(IContextMenu2)
	DEBUG_INTERFACE_NAME(IContextMenu3)
DEBUG_END_INTERFACE_NAMES();

CTMfcContextMenu::CTMfcContextMenu(CCmdTarget* pTarg, CMenu& menu)
	: m_ctxMenu(menu)
{
	m_pCmdTarget = pTarg;
	m_bMenuSeperator = FALSE;	// add separator after appending menu?
	m_bAutoMenuEnable = TRUE;	// like MFC frame: disable cmds with no handler
}

//////////////////
// Get menu string: default is resource string
//
CString CTMfcContextMenu::OnGetMenuString(UINT nID)
{
	CString s;
	s.LoadString(nID);
	return s;
}

//////////////////
// Add menu items to client's menu
//
STDMETHODIMP CTMfcContextMenu::QueryContextMenu(HMENU hmenu, UINT index,
	UINT idCmdFirst, UINT idCmdLast, UINT uFlags)
{
	CTTRACEFN(_T("CTMfcContextMenu::QueryContextMenu(0x%x,i=%d,first=%d,last=%d,flags=0x%x)\n"),
		hmenu, index, idCmdFirst, idCmdLast, uFlags);
	/*
		This occurs if you select everything in My Documents and right-click.
	*/
	ASSERT(idCmdFirst <= idCmdLast);		// If this happens, it's a Windows bug

	/*
      The offset is wrong. From the doc: "value set to the OFFSET of the
      LARGEST command identifier that was assigned, plus one".
      Previously, this code assumed the last id was the largest.

		Mr. Dilascia misunderstood how menu IDs work in this function.
		We have to return, basically, the number of menu command IDs
		we've consumed--the largest ID minus the smallest ID + 1.

		-skst (26 Dec 2007)
	*/
	//USHORT uSmallest = 0;
	//USHORT uLargest = 0;
	UINT nMyNewID = idCmdFirst;
	if (m_ctxMenu && !(uFlags & CMF_DEFAULTONLY))
   {
		for (int i=0; i<m_ctxMenu.GetMenuItemCount(); i++)
      {
			InitMenuItem(m_pCmdTarget, m_ctxMenu, i);

			const UINT nMyID = m_ctxMenu.GetMenuItemID(i);
			ASSERT(nMyNewID <= idCmdLast);			// skst: enforce the cmd id range
			if (nMyNewID <= idCmdLast)
         {
				/*
					Map the offset to the command to the actual menu command's ID.
					See the comments for CTMfcContextMenu::InvokeCommand().
					We use a map to store these values because the offsets aren't
					necessarily contiguous--we don't store separators or popups.
				*/
				_mapMenuIDs.insert(std::make_pair(nMyNewID - idCmdFirst, nMyID));

				CString sItem;
				m_ctxMenu.GetMenuString(i, sItem, MF_BYPOSITION);
				if (InsertMenu(hmenu, index, m_ctxMenu.GetMenuState(i, MF_BYPOSITION) | MF_BYPOSITION,
					            nMyNewID, (LPCTSTR)sItem))
            {
					index++;
				}

				++nMyNewID;
			}
		}

		if (m_bMenuSeperator)
			InsertMenu(hmenu, index, MF_SEPARATOR|MF_BYPOSITION, NULL, NULL); //skst: eliminate warning
	}

	return MAKE_HRESULT(SEVERITY_SUCCESS, 0, nMyNewID - idCmdFirst);
}

//////////////////
// Initialize menu item. This works for any item in a context menu
//
void CTMfcContextMenu::InitMenuItem(CCmdTarget* pTarg, CMenu& menu, UINT nIndex)
{
	CCmdUI state;
	state.m_pMenu = &m_ctxMenu;
	state.m_nIndex = nIndex;
	state.m_nIndexMax = menu.GetMenuItemCount();
	state.m_nID = menu.GetMenuItemID(nIndex);
	if (state.m_nID == (UINT)-1) {
		// possibly a popup menu, route to first item of that popup
		state.m_pSubMenu = menu.GetSubMenu(nIndex);
		if (state.m_pSubMenu == NULL ||
			(state.m_nID = state.m_pSubMenu->GetMenuItemID(0)) == 0 ||
			state.m_nID == (UINT)-1) {
			// first item of popup can't be routed to
		}
		state.DoUpdate(pTarg, FALSE);    // popups are never auto disabled

	} else {
		// normal menu item
		// Auto enable/disable if 'm_bAutoMenuEnable'
		// and command is _not_ a system command.
		state.m_pSubMenu = NULL;
		state.DoUpdate(pTarg, m_bAutoMenuEnable);
	}
}

//////////////////
// Invoke a context menu command
//
STDMETHODIMP CTMfcContextMenu::InvokeCommand(LPCMINVOKECOMMANDINFO lpcmi)
{
	CTTRACE(_T("CTMfcContextMenu::InvokeCommand\n"));
	if (HIWORD(lpcmi->lpVerb)==0) {
		// save CMINVOKECOMMANDINFO so caller can use it if needed
		//
		m_cmi = *lpcmi;

		/*
			MSDN IContextmenu::InvokeCommand() documentation:
			"The command can be specified by its menu identifier offset ..."
			This is not an ID but an offset into the menu.
			So we look up the offset we saved in the map.
		*/
		const UINT idReal = _mapMenuIDs[LOWORD(lpcmi->lpVerb)];
		if (idReal == 0)
			return E_FAIL;

		// Simulate WM_COMMAND. MFC will route to appropriate cmd target
		//
		m_pCmdTarget->OnCmdMsg(idReal, CN_COMMAND, NULL, NULL);
		return NOERROR;
	}
	return E_NOTIMPL;
}

//////////////////
// Copy command string or help text into caller's buffer
//
STDMETHODIMP CTMfcContextMenu::GetCommandString(UINT_PTR nID, UINT uFlags,
	UINT*, LPSTR pszName, UINT cchMax)
{
	CTTRACE(_T("CTMfcContextMenu::GetCommandString(%u, %u)\n"), (const unsigned int) nID, uFlags);
	HRESULT hr = E_INVALIDARG;

	ASSERT(m_ctxMenu);

	CString str;
	BOOL bCopy = TRUE;						 // assume I will return the string
	// skst: Fix flag comparison--it's a bitmask, not a value
	if (uFlags & GCS_HELPTEXT)
	{
		// get prompt string
		const CString sRes(OnGetMenuString((const UINT) nID));
		AfxExtractSubString(str, sRes, 0);
		hr = S_OK;							// skst: need to say it's ok
		CTTRACE(_T("GCS_HELPTEXT: \"%s\"\n"), (LPCTSTR) str);
	}
	else if ((uFlags & GCS_VERB) || (uFlags & GCS_VALIDATE))
	{
		// get menu item itself
		if (m_ctxMenu.GetMenuString((const UINT) nID, str, MF_BYCOMMAND))
		{
			if (uFlags & GCS_VALIDATE)
			{
				CTTRACE(_T("GCS_VALIDATE\n"));
				// don't copy for GCS_VALIDATE
				bCopy = FALSE;
			}
			hr = S_OK;
			CTTRACE(_T("GCS_VERB || GCS_VALIDATE: \"%s\"\n"), (LPCTSTR) str);
		}
	}
	if (bCopy)
	{
		// skst
		/*
			"Even though pszName is declared as an LPSTR, you must cast it to UINT_PTR
			and return a Unicode string if uFlags is set to either GCS_HELPTEXTW or
			GCS_VERBW. GCS_UNICODE can be used as a bitmask to test uFlags for 'W' and
			'A' versions of the flag it contains."
			REF: http://msdn.microsoft.com/en-us/library/windows/desktop/bb776094.aspx
		*/
		/*
			Even if we haven't compiled for Unicode,
			Windows (XP anyway) can call with the GCS_UNICODE
			flag to indicate it wants the string in Unicode.
			So we convert the ASCII string we've retrieved.
			-skst (21 Oct 2007)
		*/
		if (uFlags & GCS_UNICODE)
		{
			const CStringW wstr(str);		// convert ASCII string to Unicode
			WCHAR *pwsz = (WCHAR*)(UINT_PTR)pszName;
			wcsncpy_s(pwsz, cchMax, wstr, cchMax / (sizeof TCHAR));
		}
		else
		{
			const CStringA astr(str);
			strncpy_s(pszName, cchMax, astr, cchMax);
		}

		pszName[cchMax-1]=0;
	}

	return hr;
}

