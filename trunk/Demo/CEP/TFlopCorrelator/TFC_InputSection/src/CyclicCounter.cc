//#  CyclicCounter.cc
//#  small class to control a cyclic counter from 0 to itsMaxNumbers-1
//#
//#  Copyright (C) 2002-2005
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

#include <CyclicCounter.h>

namespace LOFAR {

CyclicCounter::CyclicCounter(int maxnum) :
  itsValue(0),
  itsMaxNumbers(maxnum)
{
}

void CyclicCounter::checkValue()
{
  itsValue %= itsMaxNumbers;
  if (itsValue < 0) {
    itsValue += itsMaxNumbers;
  }
}

int CyclicCounter::checkValue(int value)
{
  value %= itsMaxNumbers; 
  if (value < 0) {
    value += itsMaxNumbers;
  }
  return value;
}

ostream& operator<<(ostream& os, const CyclicCounter& ss)
{
  os << ss.itsValue;
  return os;
}


}
