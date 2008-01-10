#pragma once


class CalendarWindow : public CMonthCalCtrl
{
	DECLARE_DYNAMIC(CalendarWindow)

protected:
   bool _bShowToday;
   bool _bShowWeekNums;

   bool _bSavePosition;
   int _xPos;
   int _yPos;

   bool _bSaveSize;
   int _nNumMonthsX;
   int _nNumMonthsY;

   LOGFONT _lfFont;
   CFont _font;
   /*
      WM_SETFONT
      The application should call the DeleteObject function to delete the font
      when it is no longer needed; for example, after it destroys the control. 
   */

   CSize _sizeMinClient;      // the minimum size of the client area of a 1x1 calendar

public:
	CalendarWindow();
   virtual ~CalendarWindow() {}

   bool Create();
   bool Create(const int xPos, const int yPos, const int nNumMonthsX, const int nNumMonthsY, const bool bShowToday, const bool bShowWeekNums);

   bool SavePosition() const { return _bSavePosition; }
   inline void SavePositionEnable() { _SavePosition(true); }
   inline void SavePositionDisable() { _SavePosition(false); }

   bool SaveSize() const { return _bSaveSize; }
   inline void SaveSizeEnable() { _SaveSize(true); }
   inline void SaveSizeDisable() { _SaveSize(false); }

   bool ShowToday() const { return _bShowToday; }
   inline void TodayShow() { _TodayShow(true); }
   inline void TodayHide() { _TodayShow(false); }

   bool ShowWeekNumbers() const { return _bShowWeekNums; }
   inline void WeekNumbersShow() { _WeekNumbersShow(true); }
   inline void WeekNumbersHide() { _WeekNumbersShow(false); }

   void ChooseFont();

protected:
   void _TodayShow(const bool b);
   void _WeekNumbersShow(const bool b);
   void _SavePosition(const bool b);
   void _SaveSize(const bool b);

   CSize GetRequiredDimensions(const int nNumMonthsX, const int nNumMonthsY);
   void Resize();
   void CalculateMinimumClientArea();

   DECLARE_MESSAGE_MAP()

   afx_msg void OnDestroy();
   afx_msg void OnGetMinMaxInfo(MINMAXINFO* lpMMI);
   afx_msg void OnSizing(UINT fwSide, LPRECT pRect);
   afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
   afx_msg void OnWindowPosChanged(WINDOWPOS* lpwndpos);
};
