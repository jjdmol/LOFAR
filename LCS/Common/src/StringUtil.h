//#  StringUtil.h: one line description
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

#ifndef LOFAR_COMMON_STRINGUTIL_H
#define LOFAR_COMMON_STRINGUTIL_H

// \fileStringUtil.h
// 

#include <Common/LofarTypes.h>
#include <Common/lofar_string.h>
#include <Common/lofar_vector.h>

namespace LOFAR
{

  // This class contains useful string manipulation methods.
  class StringUtil
  {
  public:
    // Split a string into substrings using \c c as the separator character.
    // The result may contain empty strings, e.g. when \c str contains two or
    // more consecutive occurrences of the separations character.
    //
    // For example:
    // \code
    //    res = StringUtil::split( "aa,bb,,dd,", ',' )
    // \endcode
    //
    // would yield the following array:
    // \verbatim
    //   res[0] = "aa"
    //   res[1] = "bb"
    //   res[2] = ""
    //   res[3] = "dd"
    //   res[4] = ""
    // \endverbatim
    static vector<string> split(const string& str, char c);
  };

//
// formatString(format, ...) --> string
//
// The function formatString accepts printf-like arguments and returns a
// formatted string. It can be used e.g. in cout constructions:
// cout << formatString("Opening connection with host %%s", hostName);
//# In real life this must be %s ofcourse but doxygen need a double %%.
const string formatString(const	char* format, ...);

//
// timeString(aTime [, gmt, format]) --> string
//
// The function timeString format the given timestamp into a human-readable
// format. The default format is yyyy-mm-dd hh:mm:ss
const string timeString(time_t     aTime, 
						bool       gmt = true,
						char*      format = "%F %T");

//
// rtrim(char* CString [,len])
//
// Skip trailing spaces. If len of string is already known at invocation
// it may be given thus avoiding a relative expensive strlen call.
//
// Returns the length of the new string
//
// NOTE: original string is truncated!
//
int32 	rtrim(char*	aCstring, int32		len = 0, char* whiteSpace = " 	");

//
// char* ltrim(char*	Cstring)
//
// Skip leading spaces. A pointer to the first non-whitespace char is
// returned. (points into original string)
char*	ltrim(char*	aCstring, char* whiteSpace = " 	");

//
// rtrim(aString)
//
// Removes trailing whitespace from the string.
//
inline void rtrim(string&		aString, const string& whiteSpace = " 	")
{
	aString = aString.erase(aString.find_last_not_of(whiteSpace)+1);
}

//
// ltrim(aString)
//
// Removes leading whitespace from the string.
//
inline void ltrim(string&		aString, const string&	whiteSpace = " 	")
{
	aString = aString.erase(0, aString.find_first_not_of(whiteSpace));
}


} // namespace LOFAR

#endif
