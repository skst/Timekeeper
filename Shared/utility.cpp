//---------------------------------------------------------------------------
// (c) 2006 12noon, Stefan K. S. Tucker
//---------------------------------------------------------------------------

#include "stdafx.h"

#include "utility.h"

/*
   This function rounds the passed number so that the result
   is the nearest multiple of the passed denominator.
*/
int MyWin::RoundToNearestMultiple(const int n, const int denominator)
{
   assert(denominator != 0);
   return (n + (denominator / 2)) / denominator * denominator;
}
