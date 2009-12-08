//# StreamUtil.cc: Specializations of the <Common/StreamUtil.cc>.
//#
//# Copyright (C) 2002-2008
//# ASTRON (Netherlands Institute for Radio Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands
//#
//# This file is part of the LOFAR software suite.
//# The LOFAR software suite is free software: you can redistribute it and/or
//# modify it under the terms of the GNU General Public License as published
//# by the Free Software Foundation, either version 3 of the License, or
//# (at your option) any later version.
//#
//# The LOFAR software suite is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License along
//# with the LOFAR software suite. If not, see <http://www.gnu.org/licenses/>.
//#
//# $Id$

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

//# Includes
#include <BBSControl/StreamUtil.h>

namespace LOFAR
{
  template<>
  void writeVector (ostream& os, const vector<string>& vec,
                    const char* separator,
                    const char* prefix, const char* postfix)
  {
    os << prefix;
    for (unsigned int i = 0; i < vec.size(); i++) {
      if (i > 0) os << separator;
      os << '"';
      const string& s = vec[i];
      for (unsigned int j = 0; j < s.size(); ++j) {
        if (s[j] == '"') os << '\\';
        os << s[j];
      }
      os << '"';
    }
    os << postfix;
  }

} // namespace LOFAR
