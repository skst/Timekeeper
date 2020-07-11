//---------------------------------------------------------------------------
// (c) 2006-2009 12noon, Stefan K. S. Tucker
//---------------------------------------------------------------------------

#pragma once

#include "MyWin/registry.h"

namespace PMS
{

class ColorChooserDlg : public CColorDialog
{
	DECLARE_DYNAMIC(ColorChooserDlg)

protected:
   /*
      [optional] Where to store this info in the Registry
   */
   static skst::Registry::EType _eRegKeyType;
   static tstring _strPathKey;

   // most recently used color
   static COLORREF _crMRU;
   // up to 16 custom colors from the user
   static COLORREF _crCustom[16];   // doc for CHOOSECOLOR struct says there are 16

public:
	ColorChooserDlg(CWnd* pParentWnd, const COLORREF& crColor);
   virtual ~ColorChooserDlg() {}

   // optional: set up where to save information in the Registry
   static _declspec(deprecated) void SetRegistryLocation(const skst::Registry::EType eType, LPCTSTR szKey);

protected:
	DECLARE_MESSAGE_MAP()
   virtual BOOL OnColorOK();
};

}
