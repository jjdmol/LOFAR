//# Coord3D.cc: Class representing a point in three-dimensional space.
//#
//# Copyright (C) 2002
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

    Coord3D operator+(const Coord3D& lhs, const Coord3D& rhs)
    {
      Coord3D v(lhs);
      v += rhs;
      return v;
    }


    Coord3D operator-(const Coord3D& lhs, const Coord3D& rhs)
    {
      Coord3D v(lhs);
      v -= rhs;
      return v;
    }


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


    Coord3D operator*(double a, const Coord3D& v)
    {
      Coord3D tmp(v);
      tmp *= a;
      return tmp;
    }


    Coord3D operator*(const Coord3D& v, double a)
    {
      return a * v;
    }


    Coord3D operator/(const Coord3D& v, double a)
    {
      Coord3D tmp(v);
      tmp /= a;
      return tmp;
    }


    bool operator==(const Coord3D& lhs, const Coord3D& rhs)
    {
      return lhs.get() == rhs.get();
    }


    ostream& operator<<(ostream& os, const Coord3D& pos)
    {
      const vector<double>& v = pos.get();
      return os << '[' << v[0] << ", " << v[1] << ", " << v[2] << ']';
    }


  } // namespace AMC

} // namespace LOFAR
