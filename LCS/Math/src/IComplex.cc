//# IComplex.h: Complex class for 8, 16, and 32 bit integers
//#
//# Copyright (C) 2003
//# ASTRON (Netherlands Foundation for Research in Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//# This program is free software; you can redistribute it and/or modify
//# it under the terms of the GNU General Public License as published by
//# the Free Software Foundation; either version 2 of the License, or
//# (at your option) any later version.
//#
//# This program is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License
//# along with this program; if not, write to the Free Software
//# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#
//# $Id$

#include <Math/IComplex.h>
#include <Common/lofar_iostream.h>

namespace LOFAR
{
  ostream& operator<< (ostream& os, const IComplex8& val)
  {
    os << '(' << int(val.re()) << ',' << int(val.im()) << ')';
    return os;
  }

  ostream& operator<< (ostream& os, const IComplex16& val)
  {
    os << '(' << val.re() << ',' << val.im() << ')';
    return os;
  }

  ostream& operator<< (ostream& os, const IComplex32& val)
  {
    os << '(' << val.re() << ',' << val.im() << ')';
    return os;
  }

} // namespace LOFAR
