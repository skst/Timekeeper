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

////////////////
// CTComObj is a do-nothing base class for now.
//
CTComObj::CTComObj()
{
	m_dwRef=1;
}

CTComObj::~CTComObj()
{
}

void CTComObj::OnFinalRelease()
{
}

STDMETHODIMP_ (ULONG) CTComObj::AddRef()
{
	CTTRACEFN(_T("CTComObj::AddRef, returning %d\n"),m_dwRef+1);
	return ++m_dwRef;
}

STDMETHODIMP_ (ULONG) CTComObj::Release()
{
	CTTRACEFN(_T("CTComObj::Release, returning %d\n"),m_dwRef-1);
	ASSERT(m_dwRef>0);
	m_dwRef--;
	if (m_dwRef==0)
		OnFinalRelease();
	return m_dwRef;
}

STDMETHODIMP CTComObj::QueryInterface(REFIID iid, LPVOID* ppv)
{
	CTTRACEFN(_T("CTComObj::QueryInterface(%s)\n"),DbgName(iid));
	*ppv = GetInterface(iid);
	HRESULT hr = *ppv ? (AddRef(), S_OK) : E_NOINTERFACE;
	CTTRACE(_T(">CTComObj::QueryInterface returns %s, *ppv=%p, count=%d\n"),
		DbgName(hr), *ppv, m_dwRef);
	return hr;
}

LPUNKNOWN CTComObj::GetInterface(REFIID iid)
{
	if (iid==IID_IUnknown)
		return (IUnknown*)this;
	return NULL;
}
