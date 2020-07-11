//---------------------------------------------------------------------------
// (c) 2004-2009 12noon, Stefan K. S. Tucker
// (c) 1991-2003 Perpetual Motion Software, Stefan K. S. Tucker
//---------------------------------------------------------------------------

#pragma once

// for CProgressDlg
#include <afxcmn.h>
#include <afxmt.h>

// for ButtonBrowse
#include <shlwapi.h>
#pragma comment(lib,"shlwapi.lib")

#include "MyWin/stl.h"
#include "MyWin/skstutil.h"

#include "MyMFC/myassert.h"
#include "MyMFC/pmsres.h"
#include "MyMFC/pmsliblink.h"

#pragma comment(lib, "version.lib")

/////////////////////////////////////////////////////////////////////////////
// PMS Namespace

#include "buttonbrowse.h"

namespace PMS
{

#include "memorydc.h"

/////////////////////////////////////////////////////////////////////////////
// Miscellaneous functions

extern   CString  GetFolderPathMyDocuments();
extern	CString	GetFolderPathDesktop();

extern   CString	BinStrToHexCStr(tstring s);
extern   tstring	HexCStrToBinStr(const CString& s);

extern   CString  AddCommas(CString& s);
extern   CString  FormatNumberWithCommas(const int n);
extern   void     PMSDrawBitmap(CDC *pdc, int iWidth, CBitmap *pcBmp);

extern   bool     OpenURL(LPCTSTR szURL);

extern   CString  BrowseForFileWithControl(CWnd& wnd, const UINT idDefExt, const UINT idFilters, const UINT idTitle, const DWORD fgsExtra);
extern   CString  BrowseForFile(LPCTSTR szPath, CWnd& wndParent, const UINT idDefExt, const UINT idFilters, const UINT idTitle, const DWORD fgsExtra);
extern   void     InvokeMenuCmd(CWnd *pWnd, const int idCmd);
extern   void     DisplayPopupMenu(CWnd *pWnd, const UINT idMenu, const UINT idDefaultCmd, const CPoint *pPoint);

//--------------------
// About box function
//--------------------
extern   void     PMSAbout(HINSTANCE hAppInst, HWND hWnd, LPCTSTR pIconName,
                           LPCTSTR szString, BOOL bRegString);


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

/*
   This class asserts the object is valid on ctor and dtor.

sample usage:

	void fish(int i)
	{
		AssertValidInOut av(this);
		
		// do some stuff

		if (i == 5)
			return;	// av's dtor does the assert-valid
		
		// do some more stuff

		// av's dtor does the assert-valid
	}
*/
class AssertValidInOut
{
protected:
	CObject *_p;

public:
	AssertValidInOut(CObject *p) : _p(p) { ASSERT_VALID(_p); }
	virtual ~AssertValidInOut() { ASSERT_VALID(_p); }
};

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

/*
   This class is used by a combination of a static control and a spin button.
   In the dlg box's header file, add this declaration in the AFX_DATA section:
      // Dialog Data
      //{{AFX_DATA(CMyDialog)
      CSpinButtonTextCtrl	m_spin;
	   //}}AFX_DATA

   In the cpp file's DoDataExchange(), add:
  	   //{{AFX_DATA_MAP(CMyDialog)
	   DDX_Control(pDX, IDC_EVENT_SIZE_SPIN, m_spin);
	   //}}AFX_DATA_MAP
      PMS::DDX_SpinButton(pDX, IDC_EVENT_SIZE_SPIN, m_iDialogSize);

   In OnInitDialog(), add:
      m_spin.AddString(IDS_A_STRING);
      m_spin.SetPos(iInitialValue);
*/
class CSpinButtonTextCtrl : public CSpinButtonCtrl
{
   protected:
      CStringArray m_strings;

   public:
      CSpinButtonTextCtrl() {}

      void  AddString(UINT id);
      void  SetRange() {}        // hide function
      int   SetPos(int iPos);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSpinButtonTextCtrl)
	//}}AFX_VIRTUAL

// Implementation
public:
   virtual ~CSpinButtonTextCtrl() {}

	// Generated message map functions
protected:
	//{{AFX_MSG(CSpinButtonTextCtrl)
	afx_msg void OnDeltaPos(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

extern void DDX_SpinButton(CDataExchange *pDX, int nIDC, int& value);
extern void DDX_Check(CDataExchange* pDX, int nIDC, bool& value);

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

/*
   This template is for associating enumeration data with
   listbox/combobox entries.

   Example:

   dlg.h:
      CBox<CComboBox,EnumType> _cb;       // can also be CListBox
      EnumType _e;

   dlg.cpp:
      DoDataExchange()
         DDX_Control(pDX, IDC_BOX, _cb);
         _cb.DDX_Box(pDX, IDC_BOX, _e);

      OnInitDialog()
         _cb.AddStringData(IDS_ITEM_1, ENUM_VALUE_1);
         _cb.AddStringData(IDS_ITEM_2, ENUM_VALUE_2);

   Technically, DDX_Box doesn't need the id passed -- it can use GetCtrlID().
   And why not embed 'EnumType _e' in CBox and add
      'EnumType GetValue() { return _e; }'
   BUT...
   We do this to be consistent with the DDX thing and to avoid requiring
   a ctor to initialize the member enum with some default value.
*/

template<class B, class E>
class CBox : public B
{
public:
   CBox() {}
   virtual ~CBox() {}

   /*
      Adds string associated with passed id to end of box
      and associates passed data with item-data.
   */
   void AddStringData(const UINT id, const E& data)
   {
      AddStringData(CString(MAKEINTRESOURCE(id)), data);
   }
   void AddStringData(LPCTSTR sz, const E& data)
   {
      const UINT ix = AddString(sz);
      SetItemData(ix, data);
   }

   /*
      This routine exchanges dialog data for a box. The passed
      data is actually the value associated with item-data.
   */
   static void DDX_Box(CDataExchange *pDX, int nIDC, E& value)
   {
	   HWND hWndCtrl = pDX->PrepareCtrl(nIDC);

      B& box = * (B*) CWnd::FromHandle(hWndCtrl);
      ASSERT(box.GetSafeHwnd() != nullptr);

      if (pDX->m_bSaveAndValidate)
      {
		   const int ix = box.GetCurSel();
         if (ix < 0)
            return;
         value = (E) box.GetItemData(ix);
      }
	   else
      {
         // find item with matching item-data
         for (int i = 0; i < box.GetCount(); ++i)
         {
#pragma warning(push)
#pragma warning(once : 4800)
            if ((E) box.GetItemData(i) == value)
#pragma warning(pop)
            {
               box.SetCurSel(i);
               return;
            }
         }

         /*
            if no matching value, set "no selection"
         */
         ASSERT(LB_ERR == -1);
         ASSERT(CB_ERR == -1);
         box.SetCurSel(-1);
      }
   }

};

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

// algorithm to find the second element of a pair<>
   // TEMPLATE FUNCTION find_pair_second
template<class _II, class _Ty> inline
_II find_pair_second(_II _F, _II _L, const _Ty& _V)
{
   for (; _F != _L; ++_F)
		if (_F->second == _V)
			break;
	return (_F);
}

/*
   This template is for associating enumeration data with
   radio button entries.

   Example:

   dlg.h:
      CDDXRadioButtons<EnumType> _rb;
      EnumType _e;

   dlg.cpp:
      DoDataExchange()
         _rb.DDX(pDX, _e);

      OnInitDialog()
         _rb.AddButtonData(IDC_BUTTON_1, ENUM_VALUE_1);
         _rb.AddButtonData(IDC_BUTTON_2, ENUM_VALUE_2);
*/

template<class E>
class CDDXRadioButtons
{
protected:
   typedef std::map<UINT,E> ButtonData;
   ButtonData m_buttons;

public:
   CDDXRadioButtons() {}
   virtual ~CDDXRadioButtons() {}

   void AddButtonData(UINT id, E e) { m_buttons[id] = e; }
   void DDX(CDataExchange *pDX, E& value) const
   {
      if (pDX->m_bSaveAndValidate)
      {
         ButtonData::const_iterator it = std::find_if(m_buttons.begin(), m_buttons.end(), IsChecked(*pDX->m_pDlgWnd));
         ASSERT(it != m_buttons.end());
         if (it == m_buttons.end())
            return;  //throw logic_error("No button is selected.");

         value = it->second;
      }
      else
      {
         ButtonData::const_iterator it = find_pair_second(m_buttons.begin(), m_buttons.end(), value);
         // this will be end() the first time because CDialog::OnInitDialog() is called in order
         // to set up the member variable for the control itself (before we can call AddButtonData())
         if (it == m_buttons.end())
            return;  //throw logic_error("No button is selected.");

         // check button
         CButton& btn = * static_cast<CButton*>(pDX->m_pDlgWnd->GetDlgItem(it->first));
         btn.SetCheck(1);
      }
   }

   // functor to find the checked radio button
   class IsChecked
   {
   protected:
      const CWnd& m_wnd;

   public:
      IsChecked(CWnd& wnd) : m_wnd(wnd) {}

      bool operator()(const typename ButtonData::value_type& pr)
      {
         const UINT id = pr.first;

         ASSERT(m_wnd.GetDlgItem(id) != nullptr);
         CButton& btn = * static_cast<CButton*>(m_wnd.GetDlgItem(id));
         return (btn.GetCheck() == 1);
      }
   };
};

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

}  // namespace

