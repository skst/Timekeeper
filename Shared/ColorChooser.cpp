#include "stdafx.h"
#include <strstream>
#include <iomanip>

#include "ColorChooser.h"
using PMS::ColorChooserDlg;

IMPLEMENT_DYNAMIC(ColorChooserDlg, CColorDialog)

skst::Registry::EType ColorChooserDlg::_eRegKeyType = skst::Registry::ETYPE_USER;
tstring ColorChooserDlg::_strPathKey;

COLORREF ColorChooserDlg::_crMRU = RGB(0,0,0);
COLORREF ColorChooserDlg::_crCustom[] = { RGB(0,0,0), RGB(0,0,0), RGB(0,0,0), RGB(0,0,0),
                                          RGB(0,0,0), RGB(0,0,0), RGB(0,0,0), RGB(0,0,0),
                                          RGB(0,0,0), RGB(0,0,0), RGB(0,0,0), RGB(0,0,0),
                                          RGB(0,0,0), RGB(0,0,0), RGB(0,0,0), RGB(0,0,0), };

void ColorChooserDlg::SetRegistryLocation(const skst::Registry::EType eType, LPCTSTR szKey)
{
   _eRegKeyType = eType;
   _strPathKey = szKey;
}


ColorChooserDlg::ColorChooserDlg(CWnd* pParentWnd, const COLORREF& crColor)
   : CColorDialog(crColor, CC_RGBINIT | CC_SOLIDCOLOR, pParentWnd)
{
   /*
      LOAD from Registry
   */
   if (!_strPathKey.empty())
   {
      // pass default value so we can create the key if it doesn't exist
      skst::Registry keyOptions(_eRegKeyType, _strPathKey.c_str(), _T(""));

      // load MRU color from Registry
	   _crMRU = keyOptions.GetDWORD(_T("MRU"), _crMRU);

      // load 16 custom colors from Registry
      for (int i = 0; i < _countof(_crCustom); ++i)
      {
#if defined(_UNICODE)
			std::wostringstream str;
         str << _T("Custom") << std::setw(2) << i << _T('\0');   // strstream doesn't terminate
#else
         std::ostringstream str;
         str << _T("Custom") << std::setw(2) << std::setfill('0') << i << _T('\0');   // strstream doesn't terminate
#endif
         _crCustom[i] = keyOptions.GetDWORD(str.str().c_str(), _crCustom[i]);
      }
   }

   m_cc.rgbResult = _crMRU;
   m_cc.lpCustColors = _crCustom;
}

BEGIN_MESSAGE_MAP(ColorChooserDlg, CColorDialog)
END_MESSAGE_MAP()

// ColorChooserDlg message handlers

BOOL ColorChooserDlg::OnColorOK()
{
   _crMRU = GetColor();

   /*
      SAVE to Registry
   */
   if (!_strPathKey.empty())
   {
      skst::Registry keyOptions(_eRegKeyType, _strPathKey.c_str(), _T(""));

      // save MRU color to Registry
	   keyOptions.SetDWORD(_T("MRU"), _crMRU);

      // save 16 custom colors to Registry
      for (int i = 0; i < (sizeof _crCustom / sizeof _crCustom[0]); ++i)
      {
#if defined(_UNICODE)
			std::wostringstream str;
         str << _T("Custom") << std::setw(2) << i << _T('\0');   // strstream doesn't terminate
         keyOptions.SetDWORD(str.str().c_str(), _crCustom[i]);
#else
         std::ostrstream str;
         str << "Custom" << std::setw(2) << std::setfill('0') << i << '\0';   // strstream doesn't terminate
         keyOptions.SetDWORD(str.str(), _crCustom[i]);
#endif
      }
   }

   return CColorDialog::OnColorOK();
}
