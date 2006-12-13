//#  util.cc: Utilities, for internal use only.
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
#include <BBSControl/util.h>
#include <Common/LofarLogger.h>

namespace LOFAR
{
  namespace BBS
  {
    string toPSstring(const char* str)
    {
      string s(str);

      // When a backslash (\c \\) is read, \c bs is set to true. The next
      // character will cause it to toggle to false again. The backslash and
      // the character following it will be left untouched in the output.
      bool bs(false);

      // Quote character will be set when a quote in seen (either single or
      // double). It will be reset when the second (matching) quote is seen.
      char q('\0');

      for (string::size_type i = 0; i < s.size(); ++i) {
	// If previous character was a backslash, skip current character.
	if (bs) {
	  bs = false;
	  continue;
	}
	if (bs = s[i] == '\\') continue; // skip if current char is backslash
	if (s[i] == '\'' || s[i] == '"') {
	  if (q == s[i]) q = '\0';  // closing quote
	  else q = s[i];            // opening quote
	}
	if (q == '\0') {            // we're not between quotes
	  switch(s[i]) {
	  case '{': s[i] = '['; break;
	  case '}': s[i] = ']'; break;
	  default: break;
	  }
	}
      }
      ASSERTSTR(q == '\0', "Unbalanced quotes in string `" << str << "'");
      return s;
    }

  } // namespace BBS

} // namespace LOFAR

