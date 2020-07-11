//---------------------------------------------------------------------------
// (c) 2002-2009 12noon, Stefan K. S. Tucker
// (c) 1994-2001 Perpetual Motion Software, Stefan K. S. Tucker
//---------------------------------------------------------------------------

#pragma once

#include "PMSlib.h"
#include "PMSRes.h"
#include "StatLink.h"

class CAboutDlg : public CDialog
{
protected:
   LPCTSTR m_sIcon;			// name (OR "makeint") of icon to use in dlg box
   CImageList m_ilLogo;
   CString m_cExtraString; // extra string to display (e.g., registration)
   HINSTANCE m_hAppInst;
   CStaticHyperlink m_web;
   CStaticHyperlink m_mailto;

public:
   CAboutDlg(HINSTANCE hAppInst, CWnd *pParent, LPCTSTR lpIconName,
               const CString& cReg, BOOL bRegString);

   // Dialog Data
   //{{AFX_DATA(CAboutDlg)
   enum { IDD = IDD_PMS_ABOUT };
      // NOTE: the ClassWizard will add data members here
   //}}AFX_DATA

   protected:
      virtual void DoDataExchange(CDataExchange* pDX);   // DDX/DDV support

   // Generated message map functions
   //{{AFX_MSG(CAboutDlg)
   virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	//}}AFX_MSG
   DECLARE_MESSAGE_MAP()
};

