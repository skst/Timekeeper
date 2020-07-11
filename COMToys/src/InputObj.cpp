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
	DEBUG_INTERFACE_NAME(IInputObject)
DEBUG_END_INTERFACE_NAMES()

STDMETHODIMP CTInputObject::UIActivateIO(BOOL fActivate, LPMSG pMsg)
{
	if (fActivate && m_hWndInputObj)
		::SetFocus(m_hWndInputObj);
	return S_OK;
}

STDMETHODIMP CTInputObject::HasFocusIO()
{
	HWND hwndFocus = ::GetFocus();
	HWND hwndMe = m_hWndInputObj;
	return (hwndFocus==hwndMe || ::IsChild(hwndMe, hwndFocus)) ? S_OK : S_FALSE;
}

STDMETHODIMP CTInputObject::TranslateAcceleratorIO(LPMSG pMsg)
{
	if (WM_KEYFIRST<=pMsg->message && pMsg->message<=WM_KEYLAST) {
		// You can override GetDefaultAccelerator to provide dynamic accels
		HACCEL hAccel = m_hAccel;
		if (hAccel) {
			HWND hwndMe = m_hWndInputObj;
			if (hwndMe && ::TranslateAccelerator(hwndMe, hAccel, pMsg))
				return S_OK;
		}
	}
	return S_FALSE; 
}
