#pragma once

#include "Shared/Pmslib.h"
#include "Shared/StaticColor.h"
#include "Shared/RegistryMonitor.h"

#include "resource.h"

/*
   POTENTIAL BUG:

   Note that some functions, updating control's text and formatting a control,
   use the member data of this class. Some of that member data is modified
   when the dialog is active (e.g., alignment and text color) so it can affect
   callers who use these functions while this dlg is active. Perhaps they
   should be moved to an internal/external member data strategy.

*/

class ConfigureDlg : public CDialog
{
	DECLARE_DYNAMIC(ConfigureDlg)

public:
   enum Alignment
   {
      ALIGN_LEFT   = SS_LEFT,
      ALIGN_CENTER = SS_CENTER,
      ALIGN_RIGHT  = SS_RIGHT,
   };

protected:
   bool _bFontDefault;
   LOGFONT _lfFontClock;
   CFont _font;                     // to delete font on dtor

   CMenu _menuBar;
   CMenu *_pMenuFormats;

	RegistryMonitor _regmonClock;


   MyMFC::StaticColor _ctlSample;

   CEdit _ctlFormat;
   CString _strFormat;  // format of date/time/whatever displayed in band (in strftime() format)

	MyMFC::StaticColor _ctlSwatchText;
	MyMFC::StaticColor _ctlSwatchBG;

   bool _bColorDefaultText;
   COLORREF _crColorText;

   bool _bColorTransparentBG;
   bool _bColorDefaultBG;
   COLORREF _crColorBG;

   PMS::CBox<CComboBox,Alignment> _ctlAlignment;
   Alignment _eAlignmentStyle;      // SS_LEFT, SS_CENTER, SS_RIGHT

public:
	ConfigureDlg(CWnd* pParent);
	virtual ~ConfigureDlg() {}

   CString UpdateControlText(MyMFC::StaticColor& ctl);

   const LOGFONT& GetFont() const { return _lfFontClock; }

   bool IsClockColorTextDefault() const { return _bColorDefaultText; }
   const COLORREF& GetClockColorText() const { return _crColorText; }

   bool IsClockColorBgTransparent() const { return _bColorTransparentBG; }
   bool IsClockColorBgDefault() const { return _bColorDefaultBG; }
   const COLORREF& GetClockColorBG() const { return _crColorBG; }

   Alignment GetAlignmentStyle() const { return _eAlignmentStyle; }

	void LoadFormat();

protected:
	enum { IDD = IDD_CONFIGURE };

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

   static ConfigureDlg *_pDlgFontPicker;
   static UINT_PTR CALLBACK FctHookFontDlg(HWND hdlg, UINT uiMsg, WPARAM wParam, LPARAM lParam);
   void OnFontApply(const LOGFONT& lf);

   void LoadOptions();
   static CString UpdateControlText(MyMFC::StaticColor& ctl, LPCTSTR szFormat, const COleDateTime& dt);
   void FormatSample();
   void FormatStaticControl(MyMFC::StaticColor& ctl, const Alignment& eAlignmentStyle, const bool bColorTransparentBG);
   void SetNewFont(MyMFC::StaticColor& ctl, const LOGFONT& lf);

	DECLARE_MESSAGE_MAP()
   virtual BOOL OnInitDialog();
   virtual void OnOK();
   virtual void OnCancel();
   afx_msg void OnBnClickedFont();
   afx_msg void OnBnClickedDefault();
   afx_msg void OnStnClickedColorTextSwatch();
   afx_msg void OnStnClickedColorBgSwatch();
   afx_msg void OnCbnSelchangeAlignment();
   afx_msg void OnEnChangeFormat();
   afx_msg void OnBnClickedTransparentBg();
   afx_msg void OnBnClickedFormats();
   afx_msg void OnClockFormats();
};
