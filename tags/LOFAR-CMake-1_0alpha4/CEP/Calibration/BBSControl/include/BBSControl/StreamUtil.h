//# StreamUtil.h: Specializations of the <Common/StreamUtil.h>.
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

#ifndef LOFAR_BBSCONTROL_STREAMUTIL_H
#define LOFAR_BBSCONTROL_STREAMUTIL_H

// \file
// Useful stream manipulation methods.

//# Includes
#include <Common/StreamUtil.h>

namespace LOFAR
{
  // Specialization of writeVector for vector of string. We need this, because
  // we want to enclose each string with quotes.
  template<>
  void writeVector (ostream& os, const vector<string>& vec,
                    const char* separator,
                    const char* prefix, const char* postfix);

  template<typename T>
  string toString(const vector<T>& vec)
  {
    ostringstream oss;
    oss << vec;
    return oss.str();
  } 

} // namespace LOFAR

#endif
