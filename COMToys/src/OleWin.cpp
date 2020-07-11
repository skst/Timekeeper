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
	DEBUG_INTERFACE_NAME(IOleWindow)
DEBUG_END_INTERFACE_NAMES();

STDMETHODIMP CTOleWindow::GetWindow(HWND* pHwnd)
{
	if (pHwnd)
		*pHwnd = m_hWndOleWindow;
	return S_OK;
}

STDMETHODIMP CTOleWindow::ContextSensitiveHelp (BOOL)
{
	return E_NOTIMPL;	// sorry, not yet
}
