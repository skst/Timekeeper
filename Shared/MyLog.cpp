//---------------------------------------------------------------------------
// (c) 2002-2009 12noon, Stefan K. S. Tucker
// (c) 2000-2002 Perpetual Motion Software, Stefan K. S. Tucker
//---------------------------------------------------------------------------

#include "stdafx.h"

#include "skstutil.h"
#include "skstTools.h"
#include "internal.h"
#include "mylog.h"
#include "misc.h"

using skst::MyLog;
using skst::CriticalSection;

// storage for static data members
bool MyLog::_bStarted = false;
MyLog MyLog::_log;
tstring MyLog::_strLogFile;

int skst::LogInOut::_iLevel = 0;

#if defined(_DEBUG)
bool MyLog::_bLogEnabled = true;    // default to logging if Debug
#else
bool MyLog::_bLogEnabled = false;   // default to no logging if Release
#endif


void MyLog::SetLogFilePath(LPCTSTR szLogFilePath)
{
	if (_strLogFile.empty())
	{
		_strLogFile = szLogFilePath;
		::OutputDebugString(skst::MyFormatV(_T("Log file: %s\n"), _strLogFile.c_str()).c_str());
	}
	else
		::OutputDebugString(skst::MyFormatV(_T("Warning: log file path already set to %s.\n\tNot setting it to %s.\n"), _strLogFile.c_str(), szLogFilePath).c_str());
}


void MyLog::Construct()
{
	_bStarted = true;

	/*
		If the log file name hasn't been set yet, save it with the executable.
	*/
	SetLogFilePath(skst::GetFileNameWithNewExtension(nullptr, _T(".log")).c_str());

	/*
		If we're logging in a Release build, delete the previous log file.
	*/
#if !defined(_DEBUG)
	// Leave any pre-existing log file if logging isn't turned on
	if (_bLogEnabled)
	   Erase();	// delete log file
#endif

   //------------------
   // log startup info
   //------------------
   ::OutputDebugString(skst::MyFormatV(_T("\nLogging to %s\n"), _strLogFile.c_str()).c_str());

   LogV(_T("*** Executable: %s"), skst::GetAppPath(nullptr).c_str());

   // date/time created
   /*
      Get formatted creation date/time string.
      Same as:

      CFileStatus fs;
      if (CFile::GetStatus(::GetAppPath(), fs))
      {
         return COleDateTime(fs.m_mtime.GetTime()).Format();
      }
   */
   WIN32_FIND_DATA tData;
   if (::FindFirstFile(skst::GetAppPath(nullptr).c_str(), &tData) != INVALID_HANDLE_VALUE)
   {
      skst::MyTime dt(tData.ftCreationTime);
      LogV(_T("\tCreated: %s"), dt.Format().c_str());
      dt = tData.ftLastWriteTime;
      LogV(_T("\tModified: %s"), dt.Format().c_str());
   }

   skst::TraceVersions();

   /*
      Display language information
   */
   // BUG: "%#04lx" outputs "0x409" NOT "0x0409"
//Not in Windows NT4
//   LogV(_T("System default UI language = 0x%04lx"), ::GetSystemDefaultUILanguage());
//   LogV(_T("User default UI language = 0x%04lx"), ::GetUserDefaultUILanguage());
	LogV(_T("System default UI language = 0x%04lx"), ::GetSystemDefaultUILanguage());
	LogV(_T("User default UI language = 0x%04lx"), ::GetUserDefaultUILanguage());
}


MyLog::~MyLog()
{
   if (_strLogFile.empty())
      return;

   // can't Log anything from here
}


void MyLog::Erase() const
{
	Recycle bin;
	if (bin.RecycleFile(_strLogFile.c_str()))
	{
		::OutputDebugString(skst::MyFormatV(_T("Error recycling file: %s. Trying to delete...\n"), _strLogFile.c_str()).c_str());
		if (!::DeleteFile(_strLogFile.c_str()))
			::OutputDebugString(skst::MyFormatV(_T("Error deleting file: %s\n"), _strLogFile.c_str()).c_str());
	}
}


/*
   The Log*() functions add a newline to the end and output the
   formatted string to both OutputDebugString() and the log file.
*/
void
MyLog::LogV(LPCTSTR szFormat, ...)
{
   if (!_bLogEnabled)
      return;

	va_list argList;
	va_start(argList, szFormat);
   const tstring strFormatted(::_MyFormatArgList(szFormat, argList));
   va_end(argList);

   Log(strFormatted.c_str());
}

void
MyLog::Log(LPCTSTR s)
{
   if (!_bLogEnabled)
      return;

   const tstring msg(skst::MyFormatV(_T("[%s] %s\r\n"), skst::MyTime().Format().c_str(), s));
   ::OutputDebugString(msg.c_str());
   Append(msg.c_str());
}


void
MyLog::Append(LPCTSTR s)
{
   _cs.Lock();

   try
   {
      OpenForAppend();

      const DWORD dwBytes = (DWORD)_tcslen(s) * sizeof TCHAR;
      DWORD dwBytesWritten;
      ::WriteFile(_hFile, s, dwBytes, &dwBytesWritten, nullptr);
      if (dwBytesWritten != dwBytes)
         ::OutputDebugString(skst::MyFormatV(_T("Error writing %ld bytes (only wrote %ld).\n"), dwBytes, dwBytesWritten).c_str());

      ::CloseHandle(_hFile);
      _hFile = INVALID_HANDLE_VALUE;
   }
   catch (...)
   {
      ::OutputDebugString(skst::MyFormatV(_T("LOG ERROR: %s\n"), s).c_str());
   }

   _cs.Unlock();
}


// TODO: fix MyLog::DumpStack() issue
/*
void
MyLog::DumpStack()
{
#if defined(_DEBUG)
   m_cs.Lock();

   CFile *p = NULL;
   try
   {
      OpenForAppend();

      p = afxDump.m_pFile;
      afxDump.m_pFile = this;

      ::AfxDumpStack(); // to the "redirected" CFile

      // to ::OutputDebugString
      ::AfxDumpStack(AFX_STACK_DUMP_TARGET_ODS);

      Close();
   }
   catch (CFileException *px)
   {
      TCHAR sz[200];
      px->GetErrorMessage(sz, sizeof sz);
      TRACE(_T("LOG ERROR: %s\n"), sz);
      delete px;
   }
   catch (...)
   {
   }

   if (p != NULL)
   {
      afxDump.m_pFile = p;
      p = NULL;
   }

   m_cs.Unlock();
#endif
}
*/


void
MyLog::OpenForAppend()
{
   // don't open it twice
   if (_hFile != INVALID_HANDLE_VALUE)
      return;

   _hFile = ::CreateFile(_strLogFile.c_str(), GENERIC_WRITE, FILE_SHARE_READ, nullptr /*sec*/,
                         OPEN_ALWAYS,
                         FILE_ATTRIBUTE_NORMAL | FILE_ATTRIBUTE_NOT_CONTENT_INDEXED, nullptr);
   if (_hFile == INVALID_HANDLE_VALUE)
   {
      ::OutputDebugString(skst::MyFormatV(_T("Error opening file: %s\n"), _strLogFile.c_str()).c_str());
      return;
   }

   // seek to end
   ::SetFilePointer(_hFile, 0, 0, FILE_END);
}

