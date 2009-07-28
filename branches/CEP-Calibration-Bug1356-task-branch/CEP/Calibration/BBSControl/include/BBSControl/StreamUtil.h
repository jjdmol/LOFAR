//#  StreamUtil.h: Specializations of the <Common/StreamUtil.h>.
//#
//#  Copyright (C) 2002-2008
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

#ifndef LOFAR_BBSCONTROL_STREAMUTIL_H
#define LOFAR_BBSCONTROL_STREAMUTIL_H

// \file
// Useful stream manipulation methods.

//# Includes
#include <Common/StreamUtil.h>

namespace LOFAR
{
  // \ingroup BBSControl
  // @{

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

  // @}

} // namespace LOFAR

#endif
