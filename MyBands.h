//---------------------------------------------------------------------------
// (c) 2006-2014 12noon, Stefan K. S. Tucker
//---------------------------------------------------------------------------

#pragma once

#include "MyWin/ToolTipCtrl.h"

#include "COMtoys/BandObj.h"

#include "MyMFC/StaticColor.h"

#include "Calendar.h"
#include "ConfigureDlg.h"
#include "Resource.h"


/*
   Use this to add a Logo on the left/top.
*/
//#define _LOGO
/*
   It can be either an icon or a bitmap.
*/
//#define _LOGO_ICON
#if defined(_LOGO)
//#if defined(_LOGO_ICON)
//   #pragma message("LOGO ICON enabled")
//#else
//   #pragma message("LOGO BITMAP enabled")
//#endif
   #pragma message("LOGO enabled")
#endif

//////////////////
// Application class: derive from CBandObjApp
//
class CMyBandsDll : public CBandObjDll
{
public:
   CMyBandsDll()
   {
#if defined(_DEBUG)
#  if 1
      CBandObj::bTRACE = FALSE;
      ComToys::bTRACE = FALSE;
#  endif
#endif
   }
   virtual ~CMyBandsDll() {}
	virtual BOOL InitInstance();

protected:
	DECLARE_MESSAGE_MAP()
};


//////////////////
// Desk band lives in task bar
//
class CMyDeskBand : public CBandObj
{
protected:
   skst::MyToolTipCtrl _tips;

#if defined(_LOGO)
   CStatic _ctlAbout;
   CSize _sizeAbout;
#endif

   UINT_PTR _idTimerUpdateDisplay;

   // clock support
   MyMFC::StaticColor _ctlClock;

   CSize _sizeClockText;

   // text properties (from Registry)
   ConfigureDlg _dlgConfiguration;
   CFont _fontClock;

   // calendar support
   CalendarWindow _cal;

public:
	CMyDeskBand();
   virtual ~CMyDeskBand() {}

protected:
   void BuildContextMenu();

	// override
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs) override;
	virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo) override;

	virtual void    IPersistStreamGetSizeMax(ULARGE_INTEGER *pcbSize) override;
	virtual HRESULT IPersistStreamSave(IStream *pStream) override;
	virtual HRESULT IPersistStreamLoad(IStream *pStream) override;

	virtual STDMETHODIMP SetCompositionState(BOOL fCompositionEnabled) override
	{
		if (fCompositionEnabled != m_bCompositionEnabled)
		{
			__super::SetCompositionState(fCompositionEnabled);
			FormatClock();
			UpdateClockText();
		}
		return S_OK;
	}


	virtual void DoSize(UINT nType, int cx, int cy);

   void SetBandSizes();
   void UpdateClockText();
   void FormatClock();

	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg int  OnCreate(LPCREATESTRUCT lpCreateStruct);
   afx_msg void OnDestroy();
	afx_msg void OnTimer(UINT_PTR idEvent);
   afx_msg BOOL OnToolTipNotify(UINT id, NMHDR *pNMHDR, LRESULT *pResult);
   afx_msg void OnProducts();
	afx_msg void OnHelp();
	afx_msg void OnAbout();
   afx_msg void OnClockOptions();
	afx_msg void OnCalendar();
   afx_msg void OnUpdateCalendarShowToday(CCmdUI *pCmdUI);
   afx_msg void OnCalendarShowToday();
   afx_msg void OnUpdateCalShowWeekNums(CCmdUI *pCmdUI);
   afx_msg void OnCalShowWeekNums();
   afx_msg void OnUpdateCalSaveLastPos(CCmdUI *pCmdUI);
   afx_msg void OnCalSaveLastPos();
   afx_msg void OnUpdateCalSaveLastSize(CCmdUI *pCmdUI);
   afx_msg void OnCalSaveLastSize();
   afx_msg void OnCalSelectFont();
   DECLARE_MESSAGE_MAP();
	DECLARE_DYNCREATE(CMyDeskBand)
public:
	afx_msg void OnAdjustDateTime();
	afx_msg void OnSettingChange(UINT uFlags, LPCTSTR lpszSection);
};
