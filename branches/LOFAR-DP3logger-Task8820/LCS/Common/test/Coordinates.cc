//# Coordinates.cc: Implementation of Coordinates used by ObjectFactory test
//# program.
//#
//# Copyright (C) 2006
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
#include "Coordinates.h"
#include <Common/lofar_iostream.h>

namespace LOFAR
{
  // Use an anonymous namespace. This ensures that variables declared inside
  // it get there own private storage area and are only visible in this
  // compilation unit.
  namespace
  {
    // Forces the classes to be registered during static initialization.
    // We don't care about the resturn value; maybe we should ;-)
    bool dummy = 
      CoordinateFactory::instance().registerClass<Cartesian>("Cartesian")
      && CoordinateFactory::instance().registerClass<Cylindrical>("Cylindrical")
      && CoordinateFactory::instance().registerClass<Spherical>("Spherical"); 
  }

  void Cartesian::print(ostream& os) const
  {
    os << "Cartesian: (x, y, z) = (" 
       << itsX << ", " << itsY << ", " << itsZ << ")";
  }

  void Cylindrical::print(ostream& os) const
  {
    os << "Cylindrical: (r, theta, z) = ("
       << itsR << ", " << itsTheta << ", " << itsZ << ")";
  }

  void Spherical::print(ostream& os) const
  {
    os << "Spherical: (r, theta, phi) = ("
       << itsR << ", " << itsTheta << ", " << itsPhi << ")";
  }

  ostream& operator<<(ostream& os, const Coordinate& s)
  {
    s.print(os);
    return os;
  }

}
