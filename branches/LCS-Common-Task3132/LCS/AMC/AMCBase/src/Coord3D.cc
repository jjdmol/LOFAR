//# Coord3D.cc: Class representing a point in three-dimensional space.
//#
//# Copyright (C) 2002
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
#include <AMCBase/Coord3D.h>
#include <AMCBase/Exceptions.h>
#include <Common/lofar_iostream.h>
#include <Common/lofar_math.h>
#include <Common/LofarLogger.h>

namespace LOFAR
{
  namespace AMC
  {

    //## --   Public member functions   -- ##//

    Coord3D::Coord3D() : 
      itsXYZ(3, 0.0)
    {
    }
    

    Coord3D::Coord3D (double lon, double lat, double r) : 
      itsXYZ(3, 0.0)
    {
      double tmp = cos(lat);
      itsXYZ[0] = r * cos(lon) * tmp;
      itsXYZ[1] = r * sin(lon) * tmp;
      itsXYZ[2] = r * sin(lat);
    }


    Coord3D::Coord3D(const vector<double>& xyz) : 
      itsXYZ(3, 0.0)
    {
      ASSERT(xyz.size() == 3);
      itsXYZ = xyz;
    }


    double Coord3D::longitude() const
    {
      return atan2(itsXYZ[1], itsXYZ[0]);
    }
    

    double Coord3D::latitude() const
    {
      if (isZero()) return 0;
      else return asin(itsXYZ[2] / radius());
    }
    

    double Coord3D::radius() const
    {
      return sqrt(*this * *this);
    }
    

    double Coord3D::normalize()
    {
      if (isZero()) return 0;
      double r(radius());
      scale(1/r);
      return r;
    }


    Coord3D& Coord3D::operator+=(const Coord3D& that)
    {
      for (uint i = 0; i < 3; ++i) {
        itsXYZ[i] += that.itsXYZ[i];
      }
      return *this;
    }


    Coord3D& Coord3D::operator-=(const Coord3D& that)
    {
      for (uint i = 0; i < 3; ++i) {
        itsXYZ[i] -= that.itsXYZ[i];
      }
      return *this;
    }


    Coord3D& Coord3D::operator*=(double a)
    {
      scale(a);
      return *this;
    }


    Coord3D& Coord3D::operator/=(double a)
    {
      if (a == 0) THROW (MathException, "Attempt to divide by zero");
      scale(1/a);
      return *this;
    }


    //## --   Private member functions   -- ##//

    void Coord3D::scale(double a)
    {
      for (uint i = 0; i < 3; ++i) {
        itsXYZ[i] *= a;
      }
    }
    

    //## -- Free functions   -- ##//

    double operator*(const Coord3D& lhs, const Coord3D& rhs)
    {
      double sum(0);
      const vector<double>& x = lhs.get();
      const vector<double>& y = rhs.get();
      for (uint i = 0; i < 3; ++i) {
        sum += x[i] * y[i];
      }
      return sum;
    }


    ostream& operator<<(ostream& os, const Coord3D& pos)
    {
      const vector<double>& v = pos.get();
      return os << '[' << v[0] << ", " << v[1] << ", " << v[2] << ']';
    }


  } // namespace AMC

} // namespace LOFAR
