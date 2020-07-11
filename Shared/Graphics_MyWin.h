//---------------------------------------------------------------------------
// (c) 2006 12noon, Stefan K. S. Tucker
//---------------------------------------------------------------------------

#pragma once

/*
   Microsoft's versions of these macros (wingdi.h) fail the compiler's
   smaller-type-conversion test. You have to cast off the leading
   bits so that it isn't losing data when it's downcast. Sheesh.
   Thanks, Microsoft.
*/
#define MyGetRValue(rgb)      ((BYTE)((rgb) & 0xFF))
#define MyGetGValue(rgb)      ((BYTE)(((WORD)((rgb) & 0xFF00)) >> 8))
#define MyGetBValue(rgb)      ((BYTE)(((rgb) & 0xFF0000) >> 16))


namespace MyWin
{

extern   void        MoveRectangleInsideScreen(RECT& r);
extern   void        MoveRectangleInsideWorkarea(RECT& r);
extern   void        MoveRectangleInsideRectangle(const RECT& rBig, RECT& rSmall);

}
