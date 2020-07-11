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

CTPersistStream::CTPersistStream(BOOL& bModified) : m_bModified(bModified)
{
}

CTPersistStream::~CTPersistStream()
{
}

STDMETHODIMP CTPersistStream::IsDirty()
{
	return m_bModified ? S_OK : S_FALSE;
}

STDMETHODIMP CTPersistStream::Load(LPSTREAM)
{
	return S_OK; // not implemented yet
}

STDMETHODIMP CTPersistStream::Save(LPSTREAM lpStream, BOOL bClearDirty)
{
	if (bClearDirty)
		m_bModified = FALSE;
	return S_OK; // not implemented yet
}

STDMETHODIMP CTPersistStream::GetSizeMax(ULARGE_INTEGER* pcbSize)
{
	if (pcbSize)
		pcbSize->LowPart = pcbSize->HighPart = 0;
	return S_OK;
}

//////////////// IPersistStreamInit ////////////////

CTPersistStreamInit::CTPersistStreamInit(BOOL& bModified) :
	CTPersistStream(bModified)
{
}

CTPersistStreamInit::~CTPersistStreamInit()
{
}

STDMETHODIMP CTPersistStreamInit::InitNew()
{
	return S_OK;
}

