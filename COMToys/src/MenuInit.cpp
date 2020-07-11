#include "stdafx.h"
#include "COMtoys/MenuInit.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//////////////////
// This static function can be sed to initialize any menu
// when you get WM_INITMENUPOPUP
//
void CPopupMenuInit::Init(CCmdTarget* pTarg,
	CMenu* pMenu, BOOL bAutoMenuEnable)
{
	CCmdUI state;
	state.m_pMenu = pMenu;
	ASSERT(state.m_pOther == NULL);
	ASSERT(state.m_pParentMenu == NULL);

	CWnd *pWnd = DYNAMIC_DOWNCAST(CWnd, pTarg);
	if (pWnd) {

		// determine if menu is popup in top-level menu and set m_pOther to
		//  it if so (m_pParentMenu == NULL indicates that it is secondary popup)
		HMENU hParentMenu;
		if (AfxGetThreadState()->m_hTrackingMenu == pMenu->m_hMenu)
			state.m_pParentMenu = pMenu;    // parent == child for tracking popup
		else if ((hParentMenu = ::GetMenu(*pWnd)) != NULL)
		{
			CWnd* pParent = pWnd->GetTopLevelParent();
			// child windows don't have menus -- need to go to the top!
			if (pParent != NULL &&
				(hParentMenu = ::GetMenu(pParent->m_hWnd)) != NULL)
			{
				int nIndexMax = ::GetMenuItemCount(hParentMenu);
				for (int nIndex = 0; nIndex < nIndexMax; nIndex++)
				{
					if (::GetSubMenu(hParentMenu, nIndex) == pMenu->m_hMenu)
					{
						// when popup is found, m_pParentMenu is containing menu
						state.m_pParentMenu = CMenu::FromHandle(hParentMenu);
						break;
					}
				}
			}
		}
	}

	state.m_nIndexMax = pMenu->GetMenuItemCount();
	for (state.m_nIndex = 0; state.m_nIndex < state.m_nIndexMax;
	  state.m_nIndex++)
	{
		state.m_nID = pMenu->GetMenuItemID(state.m_nIndex);
		if (state.m_nID == 0)
			continue; // menu separator or invalid cmd - ignore it

		ASSERT(state.m_pOther == NULL);
		ASSERT(state.m_pMenu != NULL);
		if (state.m_nID == (UINT)-1)
		{
			// possibly a popup menu, route to first item of that popup
			state.m_pSubMenu = pMenu->GetSubMenu(state.m_nIndex);
			if (state.m_pSubMenu == NULL ||
				(state.m_nID = state.m_pSubMenu->GetMenuItemID(0)) == 0 ||
				state.m_nID == (UINT)-1)
			{
				continue;       // first item of popup can't be routed to
			}
			state.DoUpdate(pTarg, FALSE);    // popups are never auto disabled
		}
		else
		{
			// normal menu item
			// Auto enable/disable if 'bAutoMenuEnable'
			// and command is _not_ a system command.
			state.m_pSubMenu = NULL;
			state.DoUpdate(pTarg, bAutoMenuEnable && state.m_nID < 0xF000);
		}

		// adjust for menu deletions and additions
		UINT nCount = pMenu->GetMenuItemCount();
		if (nCount < state.m_nIndexMax)
		{
			state.m_nIndex -= (state.m_nIndexMax - nCount);
			while (state.m_nIndex < nCount &&
				pMenu->GetMenuItemID(state.m_nIndex) == state.m_nID)
			{
				state.m_nIndex++;
			}
		}
		state.m_nIndexMax = nCount;
	}
}

CPopupMenuInitHandler::CPopupMenuInitHandler()
{
	m_bAutoMenuEnable=TRUE;
}

CPopupMenuInitHandler::~CPopupMenuInitHandler()
{
}

LRESULT CPopupMenuInitHandler::WindowProc(UINT msg, WPARAM wp, LPARAM lp)
{
	if (msg==WM_INITMENUPOPUP) {
		OnMenuInitPopup(CMenu::FromHandle((HMENU)wp), LOWORD(lp), HIWORD(lp));
	}
	return CSubclassWnd::WindowProc(msg, wp, lp);
}

//////////////////
// This function is mostly copied from CFrameWnd::OnMenuInitPopup
//
void CPopupMenuInitHandler::OnMenuInitPopup(CMenu* pMenu,
	UINT nIndex, BOOL bSysMenu)
{
	if (bSysMenu)
		return;     // don't support system menu

	ASSERT(pMenu);
	// check the enabled state of various menu items

	CWnd* pWnd = CWnd::FromHandle(m_hWnd);
	CPopupMenuInit::Init(pWnd, pMenu, m_bAutoMenuEnable);
}

