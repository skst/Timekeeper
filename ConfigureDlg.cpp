//---------------------------------------------------------------------------
// (c) 2006 12noon, Stefan K. S. Tucker
//---------------------------------------------------------------------------

#include "stdafx.h"

#include "MyWin/graphics.h"
#include "MyMFC/ColorChooser.h"

#include "ConfigureDlg.h"

IMPLEMENT_DYNAMIC(ConfigureDlg, CDialog)

ConfigureDlg::ConfigureDlg(CWnd* pParent)
	: CDialog(ConfigureDlg::IDD, pParent)
{
   _bFontDefault = true;

   _bColorDefaultText = false;

   _bColorTransparentBG = true;
   _bColorDefaultBG = false;

   // set up format context menu
   _menuBar.LoadMenu(IDM_FORMATS);
   _pMenuFormats = _menuBar.GetSubMenu(0);

   LoadOptions();
}


/*
   Can't do this in the ctor because the app name isn't set yet
*/
void ConfigureDlg::LoadOptions()
{
   ASSERT(_tcslen(::AfxGetAppName()) > 0);

	LoadFormat();

   _bFontDefault = !!::AfxGetApp()->GetProfileInt(CString(MAKEINTRESOURCE(IDS_INI_SECTION_CLOCK)), CString(MAKEINTRESOURCE(IDS_INI_CLOCK_FONT_DEFAULT)), true);

   // if there isn't an entry, init to icon title font
   BYTE *pData = NULL;
   UINT uiNumBytes = 0;
   if (!::AfxGetApp()->GetProfileBinary(CString(MAKEINTRESOURCE(IDS_INI_SECTION_CLOCK)), CString(MAKEINTRESOURCE(IDS_INI_CLOCK_FONT)), &pData, &uiNumBytes))
   {
      ASSERT(pData == NULL);
      TRACE("No font in Registry.\n");
      _bFontDefault = true;
   }
   else
   {
      ASSERT(uiNumBytes > 0);
      ASSERT(pData != NULL);
      ::memcpy(&_lfFontClock, pData, uiNumBytes);
      delete [] pData;
   }
   if (_bFontDefault)
   {
      ///*
      //   create font to use for static control
      //   (If Tahoma doesn't exist, it will pick something similar.)
      //   The sizes have some strange results at small point sizes:
      //   
      //   value - pt-size  (height)
      //      100 - 10 point (16)
      //      90 -  9 point (14)
      //      85 -  8 point (13)
      //      80 -  7 point (12)
      //*/
      //// "MS San Serif"
      //CFont font;
      //VERIFY(font.CreatePointFont(85 /*8-point*/, _T("Tahoma")));
      //font.GetLogFont(&_lfFontClock);

      ::SystemParametersInfo(SPI_GETICONTITLELOGFONT, sizeof _lfFontClock, &_lfFontClock, 0);
   }
   _font.DeleteObject();   // in case we're called from OnCancel()
   VERIFY(_font.CreateFontIndirect(&_lfFontClock));

   COLORREF crDefault = MyMFC::StaticColor::GetTextColorDefault();
   _crColorText = ::AfxGetApp()->GetProfileInt(CString(MAKEINTRESOURCE(IDS_INI_SECTION_CLOCK)), CString(MAKEINTRESOURCE(IDS_INI_COLOR_TEXT)), crDefault);
   _bColorDefaultText = !!::AfxGetApp()->GetProfileInt(CString(MAKEINTRESOURCE(IDS_INI_SECTION_CLOCK)), CString(MAKEINTRESOURCE(IDS_INI_COLOR_TEXT_DEFAULT)), true);
   if (_bColorDefaultText)
      _crColorText = crDefault;

   DWORD dwColor = ::GetSysColor(COLOR_3DFACE);
   crDefault = RGB(MyGetRValue(dwColor), MyGetGValue(dwColor), MyGetBValue(dwColor));
   _crColorBG = ::AfxGetApp()->GetProfileInt(CString(MAKEINTRESOURCE(IDS_INI_SECTION_CLOCK)), CString(MAKEINTRESOURCE(IDS_INI_COLOR_BG)), crDefault);
   _bColorDefaultBG = !!::AfxGetApp()->GetProfileInt(CString(MAKEINTRESOURCE(IDS_INI_SECTION_CLOCK)), CString(MAKEINTRESOURCE(IDS_INI_COLOR_BG_DEFAULT)), true);
   if (_bColorDefaultBG)
      _crColorBG = crDefault;
   _bColorTransparentBG = !!::AfxGetApp()->GetProfileInt(CString(MAKEINTRESOURCE(IDS_INI_SECTION_CLOCK)), CString(MAKEINTRESOURCE(IDS_INI_COLOR_BG_TRANSPARENT)), true);

   _eAlignmentStyle = (Alignment) ::AfxGetApp()->GetProfileInt(CString(MAKEINTRESOURCE(IDS_INI_SECTION_CLOCK)), CString(MAKEINTRESOURCE(IDS_INI_ALIGNMENT)), SS_LEFT);

	/*
		monitor the registry key for a change to the display format
	*/
	HKEY hAppKey = ::AfxGetApp()->GetAppRegistryKey();
	_regmonClock.Monitor(hAppKey, CString(MAKEINTRESOURCE(IDS_INI_SECTION_CLOCK)));
	::RegCloseKey(hAppKey);
}


void ConfigureDlg::LoadFormat()
{
   // default to short date/time format for locale
   _strFormat = ::AfxGetApp()->GetProfileString(CString(MAKEINTRESOURCE(IDS_INI_SECTION_CLOCK)), CString(MAKEINTRESOURCE(IDS_INI_CLOCK_FORMAT)), _T("%c"));
}


BOOL ConfigureDlg::OnInitDialog()
{
   CDialog::OnInitDialog();

	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
   // set large icon for top-level window
   SetIcon(::AfxGetApp()->LoadIcon(IDR_MAINFRAME), TRUE);
   // small icon
   const HICON h = (HICON) ::LoadImage(::AfxGetResourceHandle(), MAKEINTRESOURCE(IDR_MAINFRAME), IMAGE_ICON,
                                       ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON), LR_SHARED);
	SetIcon(h, FALSE);   // 16x16 icon

   // set up format button bitmap
   const HBITMAP hBmpArrowR = (HBITMAP) ::LoadImage(::AfxGetInstanceHandle(), MAKEINTRESOURCE(IDB_ARROW_R),
                                                   IMAGE_BITMAP, 0, 0, LR_DEFAULTSIZE | LR_SHARED | LR_LOADTRANSPARENT | LR_LOADMAP3DCOLORS);
   ((CButton*) GetDlgItem(IDC_FORMATS))->SetBitmap(hBmpArrowR);

   // set up controls
   _ctlAlignment.AddStringData(IDS_ALIGN_LEFT, ALIGN_LEFT);
   _ctlAlignment.AddStringData(IDS_ALIGN_CENTER, ALIGN_CENTER);
   _ctlAlignment.AddStringData(IDS_ALIGN_RIGHT, ALIGN_RIGHT);

   _ctlSwatchText.SubclassDlgItem(IDC_COLOR_TEXT_SWATCH, this);
   /*
      Even if the text color is the default text color, we can't set the
      swatch's background to "default" because the default TEXT color
      is NOT the default BACKGROUND color.
   */
   _ctlSwatchText.SetBgColor(_crColorText);

   _ctlSwatchBG.SubclassDlgItem(IDC_COLOR_BG_SWATCH, this);
   if (_bColorDefaultBG)
      _ctlSwatchBG.SetBgColorDefault();
   else
      _ctlSwatchBG.SetBgColor(_crColorBG);

   UpdateData(FALSE);

   /*
      set the sample to a custom font (so that when we "delete the old one" we don't delete the system font)
   */
   _ctlSample.SetFont(&_font);

   FormatSample();

   return TRUE;  // return TRUE unless you set the focus to a control
   // EXCEPTION: OCX Property Pages should return FALSE
}


void ConfigureDlg::DoDataExchange(CDataExchange* pDX)
{
   CDialog::DoDataExchange(pDX);
   DDX_Control(pDX, IDC_SAMPLE, _ctlSample);
   DDX_Control(pDX, IDC_FORMAT, _ctlFormat);
   DDX_Control(pDX, IDC_ALIGNMENT, _ctlAlignment);
   DDX_Text(pDX, IDC_FORMAT, _strFormat);
   _ctlAlignment.DDX_Box(pDX, IDC_ALIGNMENT, _eAlignmentStyle);
   PMS::DDX_Check(pDX, IDC_TRANSPARENT_BG, _bColorTransparentBG);
}


BEGIN_MESSAGE_MAP(ConfigureDlg, CDialog)
   ON_STN_CLICKED(IDC_COLOR_TEXT_SWATCH, OnStnClickedColorTextSwatch)
   ON_STN_CLICKED(IDC_COLOR_BG_SWATCH, OnStnClickedColorBgSwatch)
   ON_BN_CLICKED(IDC_DEFAULT, OnBnClickedDefault)
   ON_BN_CLICKED(IDC_FONT, OnBnClickedFont)
   ON_CBN_SELCHANGE(IDC_ALIGNMENT, OnCbnSelchangeAlignment)
   ON_EN_CHANGE(IDC_FORMAT, OnEnChangeFormat)
   ON_BN_CLICKED(IDC_TRANSPARENT_BG, OnBnClickedTransparentBg)
   ON_BN_CLICKED(IDC_FORMATS, OnBnClickedFormats)

   ON_COMMAND(ID_CLOCKFORMATS_12MMSS                  , OnClockFormats)
   ON_COMMAND(ID_CLOCKFORMATS_12MMAMPM                , OnClockFormats)
   ON_COMMAND(ID_CLOCKFORMATS_24MMSS                  , OnClockFormats)

   ON_COMMAND(ID_CLOCKFORMATS_MDYYYY                  , OnClockFormats)
   ON_COMMAND(ID_CLOCKFORMATS_YYYYMMDD                , OnClockFormats)
   ON_COMMAND(ID_CLOCKFORMATS_DAYDMMM                 , OnClockFormats)
   ON_COMMAND(ID_CLOCKFORMATS_DAYMMMMDYYYY            , OnClockFormats)

   ON_COMMAND(ID_CLOCKFORMATS_SHORTDATETIME           , OnClockFormats)
   ON_COMMAND(ID_CLOCKFORMATS_LONGDATETIME            , OnClockFormats)
   ON_COMMAND(ID_CLOCKFORMATS_SHORTDATE               , OnClockFormats)
   ON_COMMAND(ID_CLOCKFORMATS_LONGDATE                , OnClockFormats)
   ON_COMMAND(ID_CLOCKFORMATS_TIME                    , OnClockFormats)

   ON_COMMAND(ID_CLOCKFORMATS_ABBREVIATEDWEEKDAYNAME  , OnClockFormats)
   ON_COMMAND(ID_CLOCKFORMATS_FULLWEEKDAYNAME         , OnClockFormats)
   ON_COMMAND(ID_CLOCKFORMATS_ABBREVIATEDMONTHNAME    , OnClockFormats)
   ON_COMMAND(ID_CLOCKFORMATS_FULLMONTHNAME           , OnClockFormats)
   ON_COMMAND(ID_CLOCKFORMATS_MONTH0                  , OnClockFormats)
   ON_COMMAND(ID_CLOCKFORMATS_MONTH                   , OnClockFormats)
   ON_COMMAND(ID_CLOCKFORMATS_D0                      , OnClockFormats)
   ON_COMMAND(ID_CLOCKFORMATS_D                       , OnClockFormats)
   ON_COMMAND(ID_CLOCKFORMATS_YEARWITHOUTCENTURY0     , OnClockFormats)
   ON_COMMAND(ID_CLOCKFORMATS_YEARWITHOUTCENTURY      , OnClockFormats)
   ON_COMMAND(ID_CLOCKFORMATS_YEARWITHCENTURY0        , OnClockFormats)
   ON_COMMAND(ID_CLOCKFORMATS_YEARWITHCENTURY         , OnClockFormats)

   ON_COMMAND(ID_CLOCKFORMATS_YEARDAY0                , OnClockFormats)
   ON_COMMAND(ID_CLOCKFORMATS_YEARDAY                 , OnClockFormats)

   ON_COMMAND(ID_CLOCKFORMATS_H024                    , OnClockFormats)
   ON_COMMAND(ID_CLOCKFORMATS_H24                     , OnClockFormats)
   ON_COMMAND(ID_CLOCKFORMATS_H012                    , OnClockFormats)
   ON_COMMAND(ID_CLOCKFORMATS_H12                     , OnClockFormats)
   ON_COMMAND(ID_CLOCKFORMATS_M0                      , OnClockFormats)
   ON_COMMAND(ID_CLOCKFORMATS_M                       , OnClockFormats)
   ON_COMMAND(ID_CLOCKFORMATS_S0                      , OnClockFormats)
   ON_COMMAND(ID_CLOCKFORMATS_S                       , OnClockFormats)
   ON_COMMAND(ID_CLOCKFORMATS_AMPM                    , OnClockFormats)
   ON_COMMAND(ID_CLOCKFORMATS_TZ                      , OnClockFormats)

   ON_COMMAND(ID_CLOCKFORMATS_WEEKOFYEAR_SUNDAY0      , OnClockFormats)
   ON_COMMAND(ID_CLOCKFORMATS_WEEKOFYEAR_SUNDAY       , OnClockFormats)
   ON_COMMAND(ID_CLOCKFORMATS_WEEKOFYEAR_MONDAY0      , OnClockFormats)
   ON_COMMAND(ID_CLOCKFORMATS_WEEKOFYEAR_MONDAY       , OnClockFormats)
   ON_COMMAND(ID_CLOCKFORMATS_WEEKDAYASNUMBER         , OnClockFormats)

   ON_COMMAND(ID_CLOCKFORMATS_PERCENTSIGN             , OnClockFormats)
   ON_COMMAND(ID_CLOCKFORMATS_LINESEPARATOR           , OnClockFormats)
END_MESSAGE_MAP()


// ConfigureDlg message handlers

void ConfigureDlg::OnCancel()
{
   /*
      The user hit Cancel, so reload all options that may have been changed.
   */
   LoadOptions();

   CDialog::OnCancel();
}

void ConfigureDlg::OnOK()
{
   CDialog::OnOK();

//   TRACE("OnOK: format: %s; font: %s\n", (LPCTSTR) _strFormat, _lfFontClock.lfFaceName);

   ::AfxGetApp()->WriteProfileString(CString(MAKEINTRESOURCE(IDS_INI_SECTION_CLOCK)), CString(MAKEINTRESOURCE(IDS_INI_CLOCK_FORMAT)), _strFormat);

   ::AfxGetApp()->WriteProfileInt(CString(MAKEINTRESOURCE(IDS_INI_SECTION_CLOCK)), CString(MAKEINTRESOURCE(IDS_INI_CLOCK_FONT_DEFAULT)), _bFontDefault);
   ::AfxGetApp()->WriteProfileBinary(CString(MAKEINTRESOURCE(IDS_INI_SECTION_CLOCK)), CString(MAKEINTRESOURCE(IDS_INI_CLOCK_FONT)), (BYTE*) &_lfFontClock, sizeof _lfFontClock);

   ::AfxGetApp()->WriteProfileInt(CString(MAKEINTRESOURCE(IDS_INI_SECTION_CLOCK)), CString(MAKEINTRESOURCE(IDS_INI_COLOR_TEXT_DEFAULT)), _bColorDefaultText);
   ::AfxGetApp()->WriteProfileInt(CString(MAKEINTRESOURCE(IDS_INI_SECTION_CLOCK)), CString(MAKEINTRESOURCE(IDS_INI_COLOR_TEXT)), _crColorText);

   ::AfxGetApp()->WriteProfileInt(CString(MAKEINTRESOURCE(IDS_INI_SECTION_CLOCK)), CString(MAKEINTRESOURCE(IDS_INI_COLOR_BG_TRANSPARENT)), !!_bColorTransparentBG);
   ::AfxGetApp()->WriteProfileInt(CString(MAKEINTRESOURCE(IDS_INI_SECTION_CLOCK)), CString(MAKEINTRESOURCE(IDS_INI_COLOR_BG_DEFAULT)), _bColorDefaultBG);
   ::AfxGetApp()->WriteProfileInt(CString(MAKEINTRESOURCE(IDS_INI_SECTION_CLOCK)), CString(MAKEINTRESOURCE(IDS_INI_COLOR_BG)), _crColorBG);

   ::AfxGetApp()->WriteProfileInt(CString(MAKEINTRESOURCE(IDS_INI_SECTION_CLOCK)), CString(MAKEINTRESOURCE(IDS_INI_ALIGNMENT)), _eAlignmentStyle);
}


void ConfigureDlg::OnEnChangeFormat()
{
   FormatSample();

	// Disable OK button if there's no format text.
	GetDlgItem(IDOK)->EnableWindow(_ctlFormat.GetWindowTextLength() > 0);
}


void ConfigureDlg::OnBnClickedFont()
{
   // don't allow effects (strikeout, underline, and color) because we don't want to use that color
   // (if we did, we'd have to ignore color selection or try to synchronize them or ...)
   LOGFONT lf = _lfFontClock; // if we passed _lfFontClock, the dlg would modify it directly
   CFontDialog dlg(&lf, CF_SCREENFONTS | /*CF_EFFECTS |*/ CF_INITTOLOGFONTSTRUCT | CF_APPLY, NULL, this);

   // set up hook to handle Apply button
   dlg.m_cf.lpfnHook = FctHookFontDlg;
   ASSERT(_pDlgFontPicker == NULL);
   _pDlgFontPicker = this;

   if (dlg.DoModal() == IDOK)
   {
      dlg.GetCurrentFont(&_lfFontClock);
      _bFontDefault = false;
   }
   _pDlgFontPicker = NULL;
   FormatSample();   // format sample with new font or return sample to original font (in case the user hit Apply)
}

ConfigureDlg *ConfigureDlg::_pDlgFontPicker = NULL;

UINT_PTR CALLBACK ConfigureDlg::FctHookFontDlg(HWND hdlg, UINT uiMsg, WPARAM wParam, LPARAM /*lParam*/)
{
   if (uiMsg == WM_COMMAND)
   {
      // hardcode id of Apply button (Microsoft doesn't tell us the id. Thanks, Microsoft.)
      if (wParam == WM_USER + 2)
      {
         LOGFONT lf;
         ::SendMessage(hdlg, WM_CHOOSEFONT_GETLOGFONT, 0, (LPARAM) &lf);
         ASSERT(_pDlgFontPicker != NULL);
         _pDlgFontPicker->OnFontApply(lf);

         return TRUE;
      }
   }
   return FALSE;
}

void ConfigureDlg::OnFontApply(const LOGFONT& lf)
{
   TRACE(__FUNCTION__ "\n");

   SetNewFont(_ctlSample, lf);
}



void ConfigureDlg::OnCbnSelchangeAlignment()
{
   FormatSample();
}


void ConfigureDlg::OnStnClickedColorTextSwatch()
{
   PMS::ColorChooserDlg dlg(this, _crColorText);
   if (dlg.DoModal() != IDOK)
      return;

   _bColorDefaultText = false;
   _crColorText = dlg.GetColor();
   _ctlSwatchText.SetBgColor(_crColorText);

   FormatSample();
}

void ConfigureDlg::OnStnClickedColorBgSwatch()
{
   PMS::ColorChooserDlg dlg(this, _crColorBG);
   if (dlg.DoModal() != IDOK)
      return;

   _bColorTransparentBG = false;
   _bColorDefaultBG = false;
   _crColorBG = dlg.GetColor();
   _ctlSwatchBG.SetBgColor(_crColorBG);

   FormatSample();
}

void ConfigureDlg::OnBnClickedTransparentBg()
{
   FormatSample();
}


void ConfigureDlg::OnBnClickedDefault()
{
   _bFontDefault = true;
   ::SystemParametersInfo(SPI_GETICONTITLELOGFONT, sizeof _lfFontClock, &_lfFontClock, 0);

   _bColorDefaultText = true;
   _crColorText = MyMFC::StaticColor::GetTextColorDefault();
   _ctlSwatchText.SetBgColor(_crColorText);

   _bColorTransparentBG = true;
   _bColorDefaultBG = true;
   _crColorBG = ::GetSysColor(COLOR_3DFACE);
   _ctlSwatchBG.SetBgColorDefault();

   UpdateData(FALSE);

   FormatSample();
}


/*
   POTENTIAL BUG:

   We know that the band itself uses this class to format the clock's display.
   That means that if we use UpdateData() to retrieve the data, it will cause
   the band to update the clock with the changes the user has made, which is
   undoubtedly undesirable.

   So, we get the volatile data manually (i.e., GetWindowText() and our own
   data exchange object).

   The other data modifies the volatile member variables themselves
   (e.g., StaticColor) but that's okay because we know the band doesn't update
   those on the fly. This will be a bug if that changes.
*/
void ConfigureDlg::FormatSample()
{
   /*
      set the sample text to a date/time that will expose all facets of format
   */
   CString str;
   _ctlFormat.GetWindowText(str);

   /*
      Set the sample text to a date/time that will expose all facets of format.
      Display a Wednesday--it's the longest (in English).
   */
   UpdateControlText(_ctlSample, str, COleDateTime(2005, 9, 7, 20, 8, 47));

   /*
      format the control's various properties
   */
   Alignment eAlignmentStyle;
   CDataExchange dx(this, TRUE);
   _ctlAlignment.DDX_Box(&dx, IDC_ALIGNMENT, eAlignmentStyle);
   FormatStaticControl(_ctlSample, eAlignmentStyle, ((CButton*) GetDlgItem(IDC_TRANSPARENT_BG))->GetCheck() == BST_CHECKED);
}


/*
   MSBUG (VS.NET 2005):

	Unfortunately, Microsoft has a bug in COleDateTime::Format(LPCTSTR pFormat).
	I know! I'm shocked, too.
	They always set the isdst member to zero, which means that even when DST is
	in effect, the %z format specifier will return Standard Time. Duh.
*/
// Copied (and modified) from COleDateTime::Format(LPCTSTR pFormat):
inline CString My_COleDateTime_Format(const COleDateTime& odt, LPCTSTR pFormat)
{
	ATLENSURE_THROW(pFormat != NULL, E_INVALIDARG);
	
	// If null, return empty string
	if(odt.GetStatus() == COleDateTime::DateTimeStatus::null)
		return _T("");

	// If invalid, return DateTime global string
	if(odt.GetStatus() == COleDateTime::DateTimeStatus::invalid)
	{
		CString str;
		if(str.LoadString(ATL_IDS_DATETIME_INVALID))
			return str;
		return szInvalidDateTime;
	}

	UDATE ud;
	if (S_OK != VarUdateFromDate(odt.m_dt, 0, &ud))
	{
		CString str;
		if(str.LoadString(ATL_IDS_DATETIME_INVALID))
			return str;
		return szInvalidDateTime;
	}

	struct tm tmTemp;
	tmTemp.tm_sec	= ud.st.wSecond;
	tmTemp.tm_min	= ud.st.wMinute;
	tmTemp.tm_hour	= ud.st.wHour;
	tmTemp.tm_mday	= ud.st.wDay;
	tmTemp.tm_mon	= ud.st.wMonth - 1;
	tmTemp.tm_year	= ud.st.wYear - 1900;
	tmTemp.tm_wday	= ud.st.wDayOfWeek;
	tmTemp.tm_yday	= ud.wDayOfYear - 1;
	/*
		mktime() and tm_isdst:
		Zero (0) to indicate that standard time is in effect.
		A value greater than 0 to indicate that daylight savings time is in effect.
		A value less than zero to have the C run-time library code compute whether
		standard time or daylight savings time is in effect.
	*/
//	tmTemp.tm_isdst	= 0;
	tmTemp.tm_isdst	= -1;	// instruct Windows to figure out DST
	mktime(&tmTemp);			// this should only set DST correctly

	CString strDate;
	LPTSTR lpszTemp = strDate.GetBufferSetLength(256);
	_tcsftime(lpszTemp, strDate.GetLength(), pFormat, &tmTemp);
	strDate.ReleaseBuffer();

	return strDate;
}


static void _invalid_param_handler(const wchar_t *expression,
                                    const wchar_t *function, 
                                    const wchar_t *file, 
                                    unsigned int line,
                                    uintptr_t /*pReserved*/)
{
	DEBUG_ONLY(line);
   const CString strExpr(expression);
   const CString strFunction(function);
   const CString strFile(file);
   TRACE(_T("Warning: Invalid parameter to function %s. File %s, line %d.\n"), (LPCTSTR) strFunction, (LPCTSTR) strFile, line);
   TRACE(_T("\tExpression: %s\n"), (LPCTSTR) strExpr);
}

/*
   These functions update a static control's text with a formatted string.
   Depending on whether the string contains a line separator (\n) character,
   the control's style is modified to allow multiple lines.
*/
/*
   %a       Abbreviated weekday name
   %A       Full weekday name
   %b       Abbreviated month name
   %B       Full month name
   %c       Date and time representation appropriate for locale
   %d       Day of month as decimal number (01 – 31)
   %H       Hour in 24-hour format (00 – 23)
   %I       Hour in 12-hour format (01 – 12)
   %j       Day of year as decimal number (001 – 366)
   %m       Month as decimal number (01 – 12)
   %M       Minute as decimal number (00 – 59)
   %p       Current locale's A.M./P.M. indicator for 12-hour clock
   %S       Second as decimal number (00 – 59)
   %U       Week of year as decimal number, with Sunday as first day of week (00 – 53)
   %w       Weekday as decimal number (0 – 6; Sunday is 0)
   %W       Week of year as decimal number, with Monday as first day of week (00 – 53)
   %x       Date representation for current locale
   %X       Time representation for current locale
   %y       Year without century, as decimal number (00 – 99)
   %Y       Year with century, as decimal number
   %z, %Z   Time-zone name or abbreviation; no characters if time zone is unknown
   %%       Percent sign

   As in the printf function, the # flag may prefix any formatting code.
   In that case, the meaning of the format code is changed as follows.

   Format code meaning:
      %#a, %#A, %#b, %#B, %#p, %#X, %#z, %#Z, %#%
      # flag is ignored.

      %#c
      Long date and time representation, appropriate for current locale. For example: "Tuesday, March 14, 1995, 12:41:29".

      %#x
      Long date representation, appropriate to current locale. For example: "Tuesday, March 14, 1995".

      %#d, %#H, %#I, %#j, %#m, %#M, %#S, %#U, %#w, %#W, %#y, %#Y
      Remove leading zeros (if any).
*/
/*
   N.B.: We can't pass CStatic because SetWindowText is NOT virtual!
   And we need it to be virtual because it does some stuff for transparent backgrounds.
*/
CString ConfigureDlg::UpdateControlText(MyMFC::StaticColor& ctl)
{
	// if there's a change in the monitored registry key, re-load the format
	if (_regmonClock.IsModified())
		LoadFormat();

	return UpdateControlText(ctl, _strFormat, COleDateTime::GetCurrentTime());
}
CString ConfigureDlg::UpdateControlText(MyMFC::StaticColor& ctl, LPCTSTR szFormat, const COleDateTime& dt)
{
   /*
      MSBUG (VS.NET 2005):

      It hasn't gotten much better in this version of VStudio.
      COleDateTime::Format() doesn't check the return value of _tcsftime()
      nor does it make any effort to recover gracefully from a bad format.
      If a bad format is passed, that function calls the invalid
      parameter handler, which promptly crashes the application.

      Nice touch, Microsoft.

      So, instead, we have to do the heavy lifting of setting our own
      invalid parameter handler and, if the user has entered a bad
      format, not crashing.

      (Plus, formats such as "... %#" are invalid, too, and the below
      code doesn't catch that.)


      MSBUG (VS.NET 2002):

      Unfortunately, _tcsftime() has a bug where if a percent character (%)
      is the last character in the format string (and there aren't two of them,
      indicating that it should be turned into an actual percent character),
      then it appends some garbage characters to the string.

      So we delete the last character if it's a percent character and the
      penultimate character isn't a percent character either. Yeah, to be
      complete, we have to count the number of percent characters at the end
      of the string and only delete it if the number is odd. Ugh.

      Nice edge-case checking there, guys! Thanks, Microsoft.
   */
/*
   // count the # of percent characters
   int nPercent = 0;
   while (szFormat[_tcslen(szFormat) - nPercent - 1] == _T('%'))
      ++nPercent;
   // if it's an odd number, delete the last one
   CString strFormat(szFormat);
   if (nPercent % 2 == 1)
   {
		strFormat.GetBuffer(strFormat.GetLength() - 1);
		strFormat.ReleaseBufferSetLength(strFormat.GetLength() - 1);
   }
*/

   /*
      Format the date/time
   */
   const int iOldMode = _CrtSetReportMode(_CRT_ASSERT, 0);
   const _invalid_parameter_handler oldHandler = ::_set_invalid_parameter_handler(_invalid_param_handler);
//BUG:   CString s(dt.Format(szFormat));
   CString s(My_COleDateTime_Format(dt, szFormat));
   ::_set_invalid_parameter_handler(oldHandler);
   _CrtSetReportMode(_CRT_ASSERT, iOldMode);
   if (ctl.GetSafeHwnd() != NULL)
   {
      if (_tcsstr(szFormat, _T("\\n")) != NULL)
      {
         ctl.ModifyStyle(SS_CENTERIMAGE | SS_ENDELLIPSIS, 0);
         s.Replace("\\n", "\n");
      }
      else
         ctl.ModifyStyle(0, SS_CENTERIMAGE | SS_ENDELLIPSIS);

      ctl.SetWindowText(s);
   }

   return s;
}


/*
   This function sets the passed static control's attributes according to the
   member data. (See the note in the header about the potential issues with
   member data being used while the dialog is active.)
*/
void ConfigureDlg::FormatStaticControl(MyMFC::StaticColor& ctl, const Alignment& eAlignmentStyle, const bool bColorTransparentBG)
{
   /*
      styles
   */
   ctl.ModifyStyle(SS_LEFT | SS_CENTER | SS_RIGHT, eAlignmentStyle);
   ctl.Invalidate();

   // use this font
   SetNewFont(ctl, _lfFontClock);

   /*
      set colors
   */
   if (_bColorDefaultText)
      ctl.SetTextColorDefault();
   else
      ctl.SetTextColor(_crColorText);

   // we're not doing UpdateData() so we have to check the ctrl manually.
   if (bColorTransparentBG)
      ctl.SetBgColorTransparent();
   else if (_bColorDefaultBG)
      ctl.SetBgColorDefault();
   else
      ctl.SetBgColor(_crColorBG);
}


void ConfigureDlg::SetNewFont(MyMFC::StaticColor& ctl, const LOGFONT& lf)
{
   /*
      replace current sample font with the passed one
   */
   HGDIOBJ hFontOld = _font.Detach();
   ASSERT(hFontOld != NULL);

   VERIFY(_font.CreateFontIndirect(&lf));
   ASSERT(ctl.GetSafeHwnd() != NULL);
   ctl.SetFont(&_font);

   ::DeleteObject(hFontOld);
}


void ConfigureDlg::OnBnClickedFormats()
{
   CPoint pt;
   ::GetCursorPos(&pt);
   _pMenuFormats->TrackPopupMenu(TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_VERTICAL, pt.x, pt.y, this);
}

/*
   All formats are separated from their description (menu name) by a tab (\t).
   The text after the tab is what's entered in the edit control.
*/
void ConfigureDlg::OnClockFormats()
{
   TRACE(__FUNCTION__ "\n");

   /*
      get the most recent message (the menu command notification)
   */
   const MSG *pMsg = GetCurrentMessage();
   ASSERT(HIWORD(pMsg->wParam) == 0);

   /*
      get the menu command string and pull out the text after the tab
   */
   const WORD wMenuID = LOWORD(pMsg->wParam);
   CString str;
   _pMenuFormats->GetMenuString(wMenuID, str, MF_BYCOMMAND);
   const int pos = str.Find(_T("\t"));
   ASSERT(pos != -1);

   // insert that text into the format control
   _ctlFormat.ReplaceSel(str.Mid(pos + 1), TRUE);
   _ctlFormat.SetFocus();
}
