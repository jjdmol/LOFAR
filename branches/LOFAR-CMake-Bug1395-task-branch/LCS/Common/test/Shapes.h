//# Shapes.h: Declaration of Shapes used by ObjectFactory test program.
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

#ifndef LOFAR_COMMON_TEST_SHAPES_H
#define LOFAR_COMMON_TEST_SHAPES_H

#include <Common/lofar_iosfwd.h>
#include <Common/lofar_string.h>
#include <Common/Singleton.h>
#include <Common/ObjectFactory.h>

namespace LOFAR
{
  // Forward declarations
  class Shape;
//   template <typename T> class Singleton;
//   template <typename Signature, typename TypeId> class ObjectFactory;
  typedef Singleton< ObjectFactory<Shape(), string> > ShapeFactory;

  // Base class of shapes used by the ObjectFactory
  class Shape
  {
  public:
    virtual ~Shape() {}
    virtual void print(ostream& os) const = 0;
  };

  // Rectangle
  class Rectangle : public Shape
  {
  public:
    virtual void print(ostream& os) const;
  };

  // Square
  class Square : public Rectangle
  {
  public:
    virtual void print(ostream& os) const;
  };

  // Ellipse
  class Ellipse : public Shape
  {
  public:
    virtual void print(ostream& os) const;
  };

  // Circle
  class Circle : public Ellipse
  {
  public:
    virtual void print(ostream& os) const;
  };

  ostream& operator<<(ostream& os, const Shape& s);
}

#endif
