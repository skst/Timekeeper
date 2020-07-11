//---------------------------------------------------------------------------
// (c) 2000-2009 12noon, Perpetual Motion Software, Stefan K. S. Tucker
//---------------------------------------------------------------------------

#pragma once

#include <string>
#include <time.h>

#include "skstutil.h"

namespace skst
{

/*
   based on MFC's CCriticalSection (needed for MyLog)
*/

class CriticalSection : public CRITICAL_SECTION
{
public:
   CriticalSection()
   {
      ::InitializeCriticalSection(this);
   }

   virtual ~CriticalSection()
   {
      ::DeleteCriticalSection(this);
   }

   void Lock()
   {
      ::EnterCriticalSection(this);
   }

   void Unlock()
   {
      ::LeaveCriticalSection(this);
   }
};

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

class MyLog
{
   protected:
		static bool _bStarted;
      static MyLog _log;   // only one log per app

      static tstring _strLogFile;
      static bool _bLogEnabled;

      CriticalSection _cs;
      HANDLE _hFile;

   public:
		static void SetLogFilePath(LPCTSTR szLogFilePath);
      static void SetLogging(const bool b) { _bLogEnabled = b; }

      virtual ~MyLog();

      static MyLog& GetLog()
      {
         if (!_bStarted)
			{
				// TODO: Construct() calls skst::TraceVersions() which calls GetLog() ... BEFORE we've fully constructed.
            _log.Construct();
			}

         return _log;
      }

      void  Erase() const;

      void  Log(LPCTSTR s);
      void  LogV(LPCTSTR szFormat, ...);

   protected:
      MyLog() : _hFile(INVALID_HANDLE_VALUE) {}
      void  Construct();

      void  Append(LPCTSTR s);
      void  OpenForAppend(); // throw(CFileException);
};

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

// internal function for LogInOut
inline tstring Timestamp()
{
   __time64_t ltime;
   _time64(&ltime);
   TCHAR sz[50];
   tstring str;
   if (_tctime64_s(sz, _countof(sz), &ltime))
   {
      str = sz;
      // remove the trailing newline ('\n')
      str.erase(str.end() - 1);
   }
   return str;
}

/*
   This class outputs a function trace on the way in and on the way out.
   It also keeps track of the nesting depth and indents appropriately.

sample usage:

	void fish(int i)
	{
		LogInOut io("fish");
      // do some stuff
	}

   OR

   void MyClass::Fish(int i)
	{
		LogInOut io(__FUNCTION__);
      // do some stuff
	}
*/
class LogInOut
{
protected:
	static int _iLevel;
   tstring _strFct;
   const bool _bTimeStamp;

public:
	LogInOut(LPCTSTR szFct, const bool bTimeStamp = false)
   	: _strFct(szFct)
   	, _bTimeStamp(bTimeStamp)
	{
      if (_bTimeStamp)
         skst::MyLog::GetLog().LogV(_T("[%s] "), Timestamp().c_str());

      skst::MyLog::GetLog().LogV(_T(" -->%*s[%s()]"), _iLevel * 2, _T(""), szFct);
		++_iLevel;
	}
	virtual ~LogInOut()
	{
		--_iLevel;

      if (_bTimeStamp)
         skst::MyLog::GetLog().LogV(_T("[%s] "), Timestamp().c_str());

      skst::MyLog::GetLog().LogV(_T("<-- %*s[%s()]"), _iLevel * 2, _T(""), _strFct.c_str());
	}

protected:
   LogInOut(const LogInOut& a) : _strFct(a._strFct), _bTimeStamp(a._bTimeStamp) {}
   LogInOut& operator=(const LogInOut& /*a*/)
   {
// these are const members and we never use the assignment operator anyway
//      _strFct = a._strFct;
//      _bTimeStamp = a._bTimeStamp;
   }
};

}

