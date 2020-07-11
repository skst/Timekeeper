//---------------------------------------------------------------------------
// (c) 2001-2017 12noon, Perpetual Motion Software, Stefan K. S. Tucker
//---------------------------------------------------------------------------

#pragma once

#if defined(ASSERT)
#undef assert
#define assert ASSERT
#else
#include <assert.h>
#endif

#include <shellapi.h>      // HDROP                                              ::GetDroppedFilePath


/*
	Create flexible string class.
	http://msdn.microsoft.com/en-us/magazine/cc188714.aspx
*/
#if defined(_UNICODE)
typedef std::wstring tstring;
#else
typedef std::string tstring;
#endif
