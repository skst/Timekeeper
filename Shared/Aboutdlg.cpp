//---------------------------------------------------------------------------
// (c) 2002 12noon, Stefan K. S. Tucker.
// (c) 1996-2002 Perpetual Motion Software, Stefan K. S. Tucker.
// All Rights Reserved
//---------------------------------------------------------------------------

#include "stdafx.h"
#include <winver.h>

#include "MyWin/VersionInfo.h"

#include "aboutdlg.h"

//---------------------------------------------------------------------------
/*
   This routine displays a PMS Standard About box.
 
   hAppInst - application instance handle
   hWnd - parent window handle
   lpIconName - icon (name of result of MAKEINTRESOURCE())
   lpszString - A pointer to the name of the registered user.
                An empty string displays "EVALUATION COPY".
                Or a normal string. (See flag below.)
                NULL displays nothing (and ignores the below flag).
   bRegString - a flag indicating if the string is to be treated as
                a registration string (name or EVALUATION COPY) (TRUE) or
                as a "normal" string (displayed verbatim) (FALSE).
*/
//---------------------------------------------------------------------------
void PMS::PMSAbout(HINSTANCE hAppInst,
                  HWND hParent,
                  LPCTSTR lpIconName,
                  LPCTSTR szString,
                  BOOL bRegString)
{
   if (hAppInst == nullptr)
   {
      TRACE("PMSAbout(): The application instance handle cannot be NULL.\n");
      ::MessageBeep(MB_ICONEXCLAMATION);
   }
   else
   {
      CWnd *pcWnd = CWnd::FromHandle(hParent);

      if (szString == nullptr)
         bRegString = FALSE;

      CAboutDlg(hAppInst, pcWnd, lpIconName, szString, bRegString).DoModal();
   }
}

/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog

CAboutDlg::CAboutDlg(HINSTANCE hAppInst,
                     CWnd *pParent,
                     LPCTSTR lpIconName,
                     const CString& cReg,
                     BOOL bRegString) :
   CDialog(CAboutDlg::IDD, pParent),
   m_web(IDC_FINGER),
   m_mailto(IDC_FINGER)
{
   m_hAppInst = hAppInst;
   m_sIcon = lpIconName;

   //---------------------------------------------------------------
   // determine what string to display at the bottom of the dlg box
   //---------------------------------------------------------------
   if (cReg.IsEmpty())
      m_cExtraString = "EVALUATION COPY";
   else if (!bRegString)
      m_cExtraString = cReg;
   else
      m_cExtraString = "Registered to " + cReg;

   //{{AFX_DATA_INIT(CAboutDlg)
      // NOTE: the ClassWizard will add member initialization here
   //}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
   CDialog::DoDataExchange(pDX);
   //{{AFX_DATA_MAP(CAboutDlg)
      // NOTE: the ClassWizard will add DDX and DDV calls here
   //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
   //{{AFX_MSG_MAP(CAboutDlg)
	ON_WM_PAINT()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAboutDlg message handlers

BOOL CAboutDlg::OnInitDialog()
{
   CenterWindow();

   /*
      setup URL controls
   */
   m_web.SetErrorID(IDP_ERR_LAUNCH_BROWSER1);
   m_mailto.SetErrorID(IDP_ERR_LAUNCH_BROWSER1);

   // we load the bmp ourselves because it's 24-bit color
   CBitmap bmp;
   bmp.LoadBitmap(IDB_12NOON); // IDB_PMS (24-color)
   BITMAP bmInfo;
   bmp.GetObject(sizeof bmInfo, &bmInfo);

//   m_ilLogo.Create(bmInfo.bmWidth, bmInfo.bmHeight, ILC_COLOR24 | ILC_MASK, 1 /*initial*/, 1 /*grow*/);
   m_ilLogo.Create(bmInfo.bmWidth, bmInfo.bmHeight, ILC_COLOR | ILC_MASK, 1 /*initial*/, 1 /*grow*/);
   m_ilLogo.Add(&bmp, RGB(0xff, 0xff, 0xff) /*white is transparent*/);

   // setup links
   m_web.SubclassDlgItem(IDC_URL_WEB, this);
   m_web.SetWindowText(CString(MAKEINTRESOURCE(PMS_IDS_WEB_SITE)));
	m_web.SetURL(PMS_IDS_WEB_PMS);

   m_mailto.SubclassDlgItem(IDC_URL_MAILTO, this);
   m_mailto.SetWindowText(CString(MAKEINTRESOURCE(PMS_IDS_MAILTO_PMS)));

   //
   SetDlgItemText(IDC_PMS_EXTRASTRING, m_cExtraString);

   // load version info
   skst::VersionInfo cVer(m_hAppInst);

   GetDlgItem(IDC_PMS_PRODNAME)->SetWindowText(cVer.GetProductName().c_str());
   GetDlgItem(IDC_PMS_VERSION)->SetWindowText(cVer.GetProductVersion().c_str());
   GetDlgItem(IDC_PMS_COMPANY)->SetWindowText(cVer.GetCompanyName().c_str());
   GetDlgItem(IDC_PMS_COPYRIGHT)->SetWindowText(cVer.GetCopyright().c_str());

   // set the About box icon
   ASSERT(m_sIcon != nullptr);
   HICON hIcon = ::LoadIcon(m_hAppInst, m_sIcon);
   if (hIcon != nullptr)
   {
      ASSERT(GetDlgItem(IDC_PMS_APPICON) != nullptr);
      ((CStatic *) GetDlgItem(IDC_PMS_APPICON))->SetIcon(hIcon);
      DestroyIcon(hIcon);
   }

   return CDialog::OnInitDialog();
}


void CAboutDlg::OnPaint() 
{
	CPaintDC dc(this); // device context for painting

   // get top-left coord from frame control
   // (we can't destroy it afterwards because we need it every time, so the control is invisible)
   CRect rect;
   CWnd *pLocation = GetDlgItem(IDC_BITMAP);
   ASSERT(!pLocation->IsWindowVisible());
   pLocation->GetClientRect(rect);
   pLocation->MapWindowPoints(this, rect);

   m_ilLogo.Draw(&dc, 0 /*index*/, rect.TopLeft(), ILD_TRANSPARENT);
	
	// Do not call CDialog::OnPaint() for painting messages
}

