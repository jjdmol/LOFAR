//#  Person.cc: implementation of the Person class
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

#include "Person.h"
#include <iostream>

Person::Person() :
  itsName("<unknown>"), itsAddress("<unknown>"), itsAge(-1), itsGender(-1)
{
}

Person::Person(const std::string& aName, const std::string& anAddress, 
               int anAge, int aGender) :
  itsName(aName), itsAddress(anAddress), itsAge(anAge), itsGender(aGender)
  // \note In a real world implementation we would of course check the input
  // arguments for validity; e.g. is \a anAge non-negative and is \a aGender
  // equal to 0 or 1.
{
}

std::string Person::getName() const
{
  return itsName;
}

void Person::setAge(int anAge)
{
  itsAge = anAge;
}

void Person::print(std::ostream& os) const
{
  os << "Person: " << itsName << "; " << itsAddress << "; "
     << itsAge << " years old; " << (itsGender == 0 ? "Male" : "Female");
}

std::ostream& operator<<(std::ostream& os, const Person& p)
{
  p.print(os);
  return os;
}
