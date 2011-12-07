//# Coordinates.h: Declaration of Coordinates used by ObjectFactory test
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

#ifndef LOFAR_COMMON_TEST_COORDINATES_H
#define LOFAR_COMMON_TEST_COORDINATES_H

#include <Common/lofar_iosfwd.h>
#include <Common/lofar_string.h>
#include <Common/Singleton.h>
#include <Common/ObjectFactory.h>

namespace LOFAR
{
  // Forward declarations
  class Coordinate;
  typedef Singleton< ObjectFactory<Coordinate(double, double, double), string> > CoordinateFactory;

  // Base class of coordinates used by the ObjectFactory
  class Coordinate
  {
  public:
    virtual ~Coordinate() {}
    virtual void print(ostream& os) const = 0;
  };

  // Cartesian coordinates
  class Cartesian : public Coordinate
  {
  public:
    Cartesian(double x, double y, double z) : 
      itsX(x), itsY(y), itsZ(z) {}
    virtual void print(ostream& os) const;
  private:
    double itsX, itsY, itsZ;
  };

  // Cylindrical coordinates
  class Cylindrical : public Coordinate
  {
  public:
    Cylindrical(double r, double theta, double z) : 
      itsR(r), itsTheta(theta), itsZ(z) {}
    virtual void print(ostream& os) const;
  private:
    double itsR, itsTheta, itsZ;
  };

  // Spherical coordinates
  class Spherical : public Coordinate
  {
  public:
    Spherical(double r, double theta, double phi) :
      itsR(r), itsTheta(theta), itsPhi(phi) {}
    virtual void print(ostream& os) const;
  private:
    double itsR, itsTheta, itsPhi;
  };

  ostream& operator<<(ostream& os, const Coordinate& s);
}

#endif
