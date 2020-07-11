////////////////////////////////////////////////////////////////
// ComToys(TM) Copyright 1999 Paul DiLascia
// If this code works, it was written by Paul DiLascia.
// If not, I don't know who wrote it.
//
#include "StdAfx.h"
//VS.NET #include <atlimpl.cpp>
#include "COMtoys/ComToys.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// To turn debugging on, set this TRUE
BOOL ComToys::bTRACE = FALSE;

//////////////////
// Helper function to allocate memory using IMalloc
//
DLLFUNC HRESULT CTOleAlloc(ULONG cb, LPVOID* ppv)
{
	CTCHECKARG(ppv);
	CComPtr<IMalloc> spMalloc;

	HRESULT hr = ::CoGetMalloc(MEMCTX_TASK, &spMalloc);
	if (FAILED(hr)) {
		*ppv = NULL;
		TRACE(_T("***Warning: CoGetMalloc failed %08lx"), hr);
	} else {
		*ppv=spMalloc->Alloc(cb);
	}
	return FAILED(hr) ? hr : *ppv==NULL ? E_OUTOFMEMORY : S_OK;
}

