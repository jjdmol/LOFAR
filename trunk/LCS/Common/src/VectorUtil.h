//#  VectorUtil.h: Utility functions on the std::vector class
//#
//#  Copyright (C) 2004
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

#ifndef LOFAR_COMMON_VECTORUTIL_H
#define LOFAR_COMMON_VECTORUTIL_H

// \file VectorUtil.h
//  Utility functions on the std::vector class

#include <vector>
#include <ostream>


namespace LOFAR
{
  // Write a vector to an ostream enclosed in square brackets and
  // using a comma as separator.
  template<class T>
  std::ostream& operator<< (std::ostream& os, const std::vector<T>& vec)
    { writeVector (os, vec, ",", "[", "]"); return os; }

  // Write a vector to an ostream with a given separator, prefix and postfix.
  template<class T>
  void writeVector (std::ostream& os, const std::vector<T>& vec,
		    const char* separator=",",
		    const char* prefix="[", const char* postfix="]")
  {
    os << prefix;
    for (unsigned int i=0; i<vec.size(); i++) {
      if (i>0) os << separator;
      os << vec[i];
    }
    os << postfix;
  }
  
} // namespace LOFAR

#endif
