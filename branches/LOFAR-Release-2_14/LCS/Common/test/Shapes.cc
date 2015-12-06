//# Shapes.cc: Implementation of Shapes used by ObjectFactory test program.
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
#include "Shapes.h"
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
      ShapeFactory::instance().registerClass<Rectangle> ("Rectangle") 
      && ShapeFactory::instance().registerClass<Square> ("Square")
      && ShapeFactory::instance().registerClass<Ellipse>("Ellipse")
      && ShapeFactory::instance().registerClass<Circle> ("Circle");
  }

  void Rectangle::print(ostream& os) const
  {
    os << "Rectangle";
  }

  void Square::print(ostream& os) const
  {
    os << "Square";
  }

  void Ellipse::print(ostream& os) const
  {
    os << "Ellipse";
  }

  void Circle::print(ostream& os) const
  {
    os << "Circle";
  }

  ostream& operator<<(ostream& os, const Shape& s)
  {
    s.print(os);
    return os;
  }

}
