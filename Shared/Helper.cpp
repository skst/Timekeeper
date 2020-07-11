//---------------------------------------------------------------------------
// (c) 2002-2010 12noon, Stefan K. S. Tucker
// (c) 1991-2002 Perpetual Motion Software, Stefan K. S. Tucker
//---------------------------------------------------------------------------

#include "stdafx.h"

#include "pmslib.h"


/*
   This function opens a passed URL in the appropriate application.
*/
bool
PMS::OpenURL(LPCTSTR szURL)
{
	//skst::MyLog::GetLog().LogV(_T(__FUNCTION__) _T("(%s)"), szURL);
	const HINSTANCE h = ::ShellExecute(nullptr, _T("open"),
													szURL, nullptr /*args*/, nullptr /*dir*/,
													SW_SHOWNORMAL);
	return (h >= HINSTANCE(HINSTANCE_ERROR));
}


//---------------------------------------------------------------------------
/*
   This routine exchanges dialog data for a spin button.
*/
//---------------------------------------------------------------------------

void PMS::DDX_SpinButton(CDataExchange *pDX, int nIDC, int& value)
{
	HWND hWndCtrl = pDX->PrepareCtrl(nIDC);
	if (pDX->m_bSaveAndValidate)
		value = (int) LOWORD(::SendMessage(hWndCtrl, UDM_GETPOS, 0, 0l));
	else
		::SendMessage(hWndCtrl, UDM_SETPOS, 0, MAKELPARAM((short) value, 0));
}

void PMS::DDX_Check(CDataExchange* pDX, int nIDC, bool& value)
{
	pDX->PrepareCtrl(nIDC);
   HWND hWndCtrl;
   pDX->m_pDlgWnd->GetDlgItem(nIDC, &hWndCtrl);
	if (pDX->m_bSaveAndValidate)
	{
		const int n = (int)::SendMessage(hWndCtrl, BM_GETCHECK, 0, 0L);
      if (n == BST_UNCHECKED)
         value = false;
      else if (n == BST_CHECKED)
         value = true;
      else
		   ASSERT(FALSE);
	}
	else
	{
      if (value == BST_UNCHECKED)
         ::SendMessage(hWndCtrl, BM_SETCHECK, 0, 0L);
      else if (value == BST_CHECKED)
         ::SendMessage(hWndCtrl, BM_SETCHECK, 1, 0L);
      else
      {
			TRACE(traceAppMsg, 0, "Warning: dialog data checkbox value (%d) out of range.\n", value);
			value = 0;  // default to off
         ::SendMessage(hWndCtrl, BM_SETCHECK, 0, 0L);
		}
	}
}
