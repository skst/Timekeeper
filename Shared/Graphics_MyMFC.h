//---------------------------------------------------------------------------
// (c) 2006 12noon, Stefan K. S. Tucker
//---------------------------------------------------------------------------

#pragma once

namespace MyMFC
{

extern   CSize    CalculateTextSize(LPCTSTR szText, CFont& font);
extern   CSize    CalculateTextSize(LPCTSTR szText, HFONT hFont);

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

/*
   This little class forces the dialog to the foreground.

   Example:
      CForegroundDlg(IDD_MY_DIALOG).DoModal();
*/
class CForegroundDlg : public CDialog
{
public:
   CForegroundDlg(const UINT id) : CDialog(id) {}
   BOOL OnInitDialog() 
   {
      SetForegroundWindow();
      return CDialog::OnInitDialog();
   }
};

}
