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
	DEBUG_INTERFACE_NAME(IDockingWindow)
DEBUG_END_INTERFACE_NAMES()

STDMETHODIMP CTDockingWindow::ShowDW(BOOL fShow)
{
	if (m_hWndDockingWindow)
		::ShowWindow(m_hWndDockingWindow, fShow ? SW_SHOW : SW_HIDE);
	return S_OK;
}

STDMETHODIMP CTDockingWindow::CloseDW(DWORD dwReserved)
{
	if (m_hWndDockingWindow)
		::SendMessage(m_hWndDockingWindow, WM_CLOSE, 0, 0);
	return S_OK;
}

STDMETHODIMP CTDockingWindow::ResizeBorderDW(LPCRECT prcBorder,
		IUnknown* punkDWSite, BOOL fReserved)
{
	return E_NOTIMPL; 	// Not yet
}

