//#  StringUtil.cc: implementation of the string utilities class.
//#
//#  Copyright (C) 2002-2003
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//#  This program is free software; you can redistribute it and/or modify
//#  it under the terms of the GNU General Public License as published by
//#  the Free Software Foundation; either version 2 of the License, or
//#  (at your option) any later version.
//#
//#  This program is distributed in the hope that it will be useful,
//#  but WITHOUT ANY WARRANTY; without even the implied warranty of
//#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//#  GNU General Public License for more details.
//#
//#  You should have received a copy of the GNU General Public License
//#  along with this program; if not, write to the Free Software
//#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#
//#  $Id$

#include <Common/LofarTypes.h>
#include <Common/StringUtil.h>
#include <iostream>
#include <stdarg.h>

namespace LOFAR
{
  
  vector<string> StringUtil::split(const string& s, char c)
  {
    vector<string> v;
    string::size_type i, j;
    i = j = 0;
    while (j != string::npos) {
      j = s.find(c,i);
      if (j == string::npos) v.push_back(s.substr(i));
      else v.push_back(s.substr(i,j-i));
      i = j+1;
    }
    return v;
  }

//
// formatString(format, ...) --> string
//
// Define a global function the accepts printf like arguments and returns 
// a string.
//
const string formatString(const	char* format, ...) {
	char		tmp_cstring[10240];
	va_list		ap;

	va_start (ap, format);
	vsnprintf(tmp_cstring, sizeof(tmp_cstring), format, ap);
	va_end   (ap);

	return   string(tmp_cstring);
}

//
// timeString(aTime [,format]) --> string
//
// Define a global function that convert a timestamp into a humanreadable 
// format.
//
const string timeString(time_t		aTime, 
							 bool		gmt,
							 char* 		format)
{
	char	theTimeString [256];
	strftime(theTimeString, 256, format, gmt ? gmtime(&aTime) 
														: localtime(&aTime));

	return (theTimeString);
}

//
// rtrim(char* CString [,len=0])
//
// Skip trailing spaces. If len of string is already known at invocation
// it may be given thus avoiding a relative expensive strlen call.
//
// NOTE: original string is truncated!
//
int32 rtrim(char*	aCstring, int32		len, char*	whiteSpace)
{
	if (!aCstring || !(*aCstring)) {		// aCstring must be valid
		return (0);
	}

	if (!len || aCstring[len] != '\0') {	// len unknown or wrong?
		len = strlen(aCstring);				// recalculate it.
	}
	--len;									// set on last char

	while ((len >= 0) && (strchr(whiteSpace, aCstring[len]))) {
		aCstring[len--] = '\0';
	}

	return (len+1);
}

//
// char* ltrim(char*	Cstring)
//
// skip leading spaces. A pointer to the first non-whitespace char is
// returned.
char*	ltrim(char*	aCstring, char*	whiteSpace)
{
	aCstring += strspn(aCstring, whiteSpace);

	return (aCstring);
}



} // namespace LOFAR
