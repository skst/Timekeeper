//---------------------------------------------------------------------------
// (c) 2010 12noon, Perpetual Motion Software, Stefan K. S. Tucker
//---------------------------------------------------------------------------

#pragma once

#include "breakpoint.h"
#include "misc.h"
#include "versions.h"
#include "mylog.h"
#include "myhandle.h"
#include "versioninfo.h"
#include "registrar.h"
#include "shelllink.h"
#include "registry.h"
//#include "activedesktop.h"

/////////////////////////////////////////////////////////////////////////////
// skst Namespace
namespace skst
{

/////////////////////////////////////////////////////////////////////////////
// Templates

/*
	Template function to cast data to a smaller
	type and ensure we don't lose anything.
*/
template<typename Small, typename Big> 
inline Small size_cast(Big const& b)
{
	// Cast into a local, so that the debugger can look at it
	const Small s = static_cast<Small>(b);
	// Check that we don't lose data this way
	assert(b == static_cast<Big>(s));
	return s;
}


/*
   from VC6SP4 <memory>
*/
// TEMPLATE CLASS auto_ptr_array (from auto_ptr)
template<class _Ty>
	class auto_ptr_array {
public:
	typedef _Ty element_type;
	explicit auto_ptr_array(_Ty *_P = 0)
		: _Owns(_P != 0), _Ptr(_P) {}
	auto_ptr_array(const auto_ptr_array<_Ty>& _Y)
		: _Owns(_Y._Owns), _Ptr(_Y.release()) {}
	auto_ptr_array<_Ty>& operator=(const auto_ptr_array<_Ty>& _Y)
		{if (this != &_Y)
			{if (_Ptr != _Y.get())
				{if (_Owns)
					delete [] _Ptr;            // skst
				_Owns = _Y._Owns; }
			else if (_Y._Owns)
				_Owns = true;
			_Ptr = _Y.release(); }
		return (*this); }
	~auto_ptr_array()
		{if (_Owns)
			delete [] _Ptr; }                // skst
	_Ty& operator*() const
		{return (*get()); }
#pragma warning(push)
#pragma warning(disable : 4284)
	_Ty *operator->() const
		{return (get()); }
#pragma warning(pop)
	_Ty *get() const
		{return (_Ptr); }
	_Ty *release() const
		{((auto_ptr_array<_Ty> *)this)->_Owns = false;
		return (_Ptr); }

   //------------------------
   // my assignment operator
   //------------------------
   void operator=(_Ty *p)
   {
      // free existing ptr
      delete [] _Ptr;

      _Ptr = p;
   }

   //----------------------------
   // my element access operator
   //----------------------------
   _Ty& operator[](int i)
   const
   {
      assert(_Ptr != nullptr);
      assert(i >= 0);

      return _Ptr[i];
   }

private:
	bool _Owns;
	_Ty *_Ptr;
	};

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

/*
   This class is used to automatically init/uninit COM. Just declare a
   global variable.

   CoInitializeEx() is only included
      #if (_WIN32_WINNT >= 0x0400 ) || defined(_WIN32_DCOM) // DCOM
   So, we use CoInitialize() instead
*/
class StartCOM
{
public:
   StartCOM()
   {
      ::CoInitialize(nullptr);   // aka ::CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
   }
   ~StartCOM()
   {
      ::CoUninitialize();
   }
};

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

/*
   This class is used when we want to do something for only a limited time

   const skst::TimeLimit cTimeLimit(250);
   while (!cTimeLimit.TimesUp())
   {
      // do something
   }
*/

class Timer
{
protected:
   DWORD _dwStart;      // msecs
   DWORD _dwDuration;   // # of msecs after which "time's up"

public:
   Timer(const DWORD dwMilliseconds)
   {
      Restart(dwMilliseconds);
   }
   virtual ~Timer() {}

   // restart the timer w/new # of msecs
   void Restart(const DWORD dwMilliseconds)
   {
      _dwDuration = dwMilliseconds;
      _dwStart = ::GetTickCount();
   }

   // restart the timer
   void Restart()
   {
      _dwStart = ::GetTickCount();
   }

   // has the time expired?
   bool TimesUp() const
   {
      return (::GetTickCount() >= _dwStart + _dwDuration);
   }

   // how many msecs have elapsed already?
   DWORD Elapsed() const
   {
      return ::GetTickCount() - _dwStart;
   }
};

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

class MyTime
{
protected:
   SYSTEMTIME _dt;
   
public:
   MyTime()
   {
      ::GetLocalTime(&_dt);
   }

   MyTime(const FILETIME& ft)
   {
      // convert UTC to local time and then to system time
      FILETIME ftLocal;
      ::FileTimeToLocalFileTime(&ft, &ftLocal);
      ::FileTimeToSystemTime(&ftLocal, &_dt);
   }
   virtual ~MyTime() {}

   tstring Format() const
   {
      // format date/time
      TCHAR szDate[50];
      ::GetDateFormat(LOCALE_USER_DEFAULT, 0, &_dt, _T("ddd dd MMM yyyy"), szDate, sizeof szDate);

      TCHAR szTime[20];
//This is only used in MyLog, so we can just change it.
//      ::GetTimeFormat(LOCALE_USER_DEFAULT, TIME_NOSECONDS, &_dt, _T("hh:mmt"), szTime, sizeof szTime);
      ::GetTimeFormat(LOCALE_USER_DEFAULT, 0, &_dt, _T("HH:mm:ss"), szTime, sizeof szTime);

      tstring str(szDate);
      str += _T(" ");
      str += szTime;

      return str;
   }
};

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

/*
   Based on the CRecycleFile class MSDNmag April 2001 article
   by Paul DiLascia.

   It can delete files and folders -- and send them to the Recycle Bin.

   Suggested use:
      Recycle bin;
      bin.RecycleFile("c:\\fish\\delme.txt"); // del file
      if (bin.RecycleFile("c:\\delme"))       // del folder
			; // failed
*/
class Recycle : public SHFILEOPSTRUCT
{
public:
   Recycle()
   {
      pTo = nullptr;
      wFunc = FO_DELETE;
      // no confirmation means "Yes to All" for all msgs, such as "Too big to fit. Do it anyway?"
      fFlags = FOF_ALLOWUNDO | FOF_SILENT | FOF_NOCONFIRMATION | FOF_NOERRORUI;
      lpszProgressTitle = nullptr;
      fAnyOperationsAborted = FALSE;
      hwnd = nullptr;
      hNameMappings = nullptr;
   }

   virtual ~Recycle() {}

   int RecycleFile(LPCTSTR szFile)
   {
      TCHAR szDel[_MAX_PATH + 1];
      _tcscpy_s(szDel, _countof(szDel), szFile);
      szDel[_tcslen(szDel) + 1] = _T('\0');
      pFrom = szDel;

      skst::MyLog::GetLog().LogV(_T("Recycling %s..."), szDel);

      return ::SHFileOperation(this);	// success == 0
   }
};

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

}  // namespace
