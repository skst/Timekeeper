////////////////////////////////////////////////////////////////
// PixieLib(TM) Copyright 1997-1998 Paul DiLascia
// If this code works, it was written by Paul DiLascia.
// If not, I don't know who wrote it.
//
#pragma once

#include "Subclass.h"

class CPopupMenuInit {
public:
	static void Init(CCmdTarget* pTarg,
		CMenu* pMenu, BOOL bAutoMenuEnable=TRUE);
};

class CPopupMenuInitHandler: public CSubclassWnd {
public:
	BOOL m_bAutoMenuEnable;
	CPopupMenuInitHandler();
	virtual ~CPopupMenuInitHandler();

	BOOL Install(CWnd* pWnd) {
		return HookWindow(pWnd);
	}
	void Remove() {
		HookWindow((HWND)NULL);
	}

protected:
	virtual void OnMenuInitPopup(CMenu* pMenu, UINT nIndex, BOOL bSysMenu);
	virtual LRESULT WindowProc(UINT msg, WPARAM wp, LPARAM lp);
};