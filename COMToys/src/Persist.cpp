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

CTPersist::CTPersist(const CLSID& clsid) : m_clsid(clsid)
{
}

CTPersist::~CTPersist()
{
}

STDMETHODIMP CTPersist::GetClassID(LPCLSID pClassID)
{
	CTTRACEFN(_T("CTPersist::GetClassID\n"));
	return pClassID ? (*pClassID=m_clsid, S_OK) : E_UNEXPECTED;
}
