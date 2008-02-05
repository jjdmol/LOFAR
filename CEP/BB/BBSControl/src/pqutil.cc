//#  pqutil.cc: PostgreSQL related utilities; for internal use only.
//#
//#  Copyright (C) 2002-2004
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

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

//# Includes
#include <BBSControl/pqutil.h>
#include <Common/lofar_sstream.h>
#include <Common/StreamUtil.h>

namespace LOFAR
{
  namespace BBS
  {
    string toPSstring(const char* in)
    {
      string out(in);

      // When a backslash (\c \\) is read, \c bs is set to true. The next
      // character will cause it to toggle to false again. The backslash and
      // the character following it will be left untouched in the output.
      bool bs(false);

      // Toggle indicating whether we're inside a quoted string or not. A
      // quoted string is a string delimited by double quotes ("). No
      // character substitutions will be done within a quoted string.
      bool q(false);

      for (string::iterator it = out.begin(); it != out.end(); ++it) {
	// If previous character was a backslash, skip current character.
        if (bs) {
          bs = false;
          continue;
        }
	if (bs = *it == '\\') continue; // skip if current char is backslash

	// Toggle if current char is a quote.
  	if (*it == '"') q = !q;
	if (!q) {            // we're not between quotes
	  switch(*it) {
	  case '{': *it = '['; break;
	  case '}': *it = ']'; break;
	  default: break;
	  }
	}
      }
      return out;
    }


    string toPQstring(const string& in)
    {
      string out;

      // Toggle indicating whether we're inside a quoted string or not. A
      // quoted string is a string delimited by double quotes (").
      bool q(false);

      for (string::const_iterator it = in.begin(); it != in.end(); ++it) {
	switch(*it) {
	case '\\' :  // backslash must be doubled
	  out += "\\\\"; 
	  break;
	case '\'' :  // single quote must be doubled
	  out += "''";
	  break;
	case '"'  :  // toggle quote character.
	  q = !q;
	  out += *it;
	  break;
	case '['  :  // replace '[' with '{' when not inside quoted string
	  if (q) out += *it;
	  else out += '{';
	  break;
	case ']'  :  // replace ']' with '}' when not inside quoted string
	  if (q) out += *it;
	  else out += '}';
	  break;
	case '{'  :  // '{' must be escaped when not inside quoted string
	  if (!q) out += "\\\\";
	  out += *it;
	  break;
	case '}'  :  // '}' must be escaped when not inside quoted string
	  if (!q) out += "\\\\";
	  out += *it;
	  break;
	default:
	  out += *it;
	  break;
	}
      }
      return out;
    }
    

    string toPQstring(const vector<string>& in)
    {
      ostringstream oss;
      oss << in;
      return toPQstring(oss.str());
    }


  } // namespace BBS
  
} // namespace LOFAR

