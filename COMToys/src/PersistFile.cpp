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

CTPersistFile::CTPersistFile(BOOL& bModified)
	: m_bModified(bModified)
{
}

CTPersistFile::~CTPersistFile()
{
}

STDMETHODIMP CTPersistFile::IsDirty()
{
	CTTRACEFN(_T("CTPersistFile::IsDirty\n"));
	return m_bModified ? S_OK : S_FALSE;
}

STDMETHODIMP CTPersistFile::Load(LPCOLESTR pszFileName, DWORD dwMode)
{
	CTTRACEFN(_T("CTPersistFile::Load\n"));
	return E_NOTIMPL; // not implemented yet--you must override
}

STDMETHODIMP CTPersistFile::Save(LPCOLESTR pszFileName, BOOL fRemember)
{
	CTTRACEFN(_T("CTPersistFile::Save\n"));
	return E_NOTIMPL; // not implemented yet--you must overrride
}

STDMETHODIMP CTPersistFile::SaveCompleted(LPCOLESTR pszFileName)
{
	CTTRACEFN(_T("CTPersistFile::SaveCompleted\n"));
	return S_OK;
}

//////////////////
// Get filename -- calls v fn to get LPCTSTR, then copy to new OLESTR
//
STDMETHODIMP CTPersistFile::GetCurFile(LPOLESTR __RPC_FAR *ppFileName)
{
	CTTRACEFN(_T("CTPersistFile::GetCurFile\n"));

	if (!ppFileName)
		return E_UNEXPECTED;

	*ppFileName = NULL;

	// get path name
	CString s = OnGetPathName();
	if (s.IsEmpty())
		return E_FAIL;

	// allocate a new string
	LPVOID pMem = NULL;
	int len = (s.GetLength() + 1)*sizeof(OLECHAR);
	HRESULT hr = CTOleAlloc(len, &pMem);
	if (FAILED(hr))
		return hr;

	// Convert to wide char and copy to caller's buffer
	USES_CONVERSION;
	*ppFileName=(LPOLESTR)pMem;
	wcscpy_s(*ppFileName, len, T2COLE((PCTSTR)s));

	return NOERROR;
}

LPCTSTR CTPersistFile::OnGetPathName()
{
	return NULL; // not implemented yet
}