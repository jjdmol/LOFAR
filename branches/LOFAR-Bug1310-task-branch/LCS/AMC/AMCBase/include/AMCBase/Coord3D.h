//# Coord3D.h: Class representing a point in three-dimensional space.
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

#ifndef LOFAR_AMCBASE_COORD3D_H
#define LOFAR_AMCBASE_COORD3D_H

// \file
// Class representing a vector in three-dimensional space.

//# Includes
#include <Common/lofar_iosfwd.h>
#include <Common/lofar_vector.h>

namespace LOFAR
{
  namespace AMC
  {

    // \addtogroup AMCBase
    // @{

    // This class represents a point in three-dimensional space. It is stored
    // using cartesian coordinates, and can be constructed from longitude,
    // latitude and radius, or from a vector containing cartesian coordinates
    // (x,y,z). It supports addition, subtraction, multiplication (which is
    // defined as the dot product).
    class Coord3D
    {
    public:
      // Default constructor defines a point located at the origin of the
      // coordinate system.
      Coord3D();

      // Create a 3D-coordinate by giving the longitude \a lon and latitude
      // \a lat in radians and the \a r in units of length.
      Coord3D(double lon, double lat, double r);

      // Create a 3D-coordinate from the cartesian coordinates \a xyz where
      // \a x, \a y, and \a z are in units of length.
      Coord3D(const vector<double>& xyz);
      
      // Return the 3D-coordinate in cartesian coordinates.
      const vector<double>& get() const
      { return itsXYZ; }

      // Return true if the 3D-coordinate is located at the origin of the
      // coordinate system.
      bool isZero() const
      { return itsXYZ[0] == 0 && itsXYZ[1] == 0 && itsXYZ[2] == 0; }

      // Return the longitude in radians.
      double longitude() const;
      
      // Return the latitude in radians.
      double latitude() const;
      
      // Return the radius, i.e. the distance to the origin.
      double radius() const;

      // Normalize the 3D-coordinate placing it on the unit sphere.
      // \return The original distance to the origin.
      double normalize();

      // Add the 3D-coordinate \a that to \c this.
      Coord3D& operator+=(const Coord3D& that);

      // Subtract the 3D-coordinate \a that from \c this.
      Coord3D& operator-=(const Coord3D& that);

      // Multiply the 3D-coordinate with the scalar \a a.
      Coord3D& operator*=(double a);

      // Divide the 3D-coordinate by a scalar \a a.
      // \throw MathException when \a == 0
      Coord3D& operator/=(double a);

    private:
      // Scale the xyz-coordinates by a factor \a a.
      void scale(double a);

      // The 3D-coordinate is stored internally in cartesian coordinates.
      vector<double> itsXYZ;

    };

    // Calculate the sum of two 3D-coordinates
    inline Coord3D operator+(const Coord3D& lhs, const Coord3D& rhs)
    { return Coord3D(lhs) += rhs; }

    // Calculate the difference between two 3D-coordinates.
    inline Coord3D operator-(const Coord3D& lhs, const Coord3D& rhs)
    { return Coord3D(lhs) -= rhs; }

    // Multiply the 3D-coordinate \a v with a scalar \a a.
    inline Coord3D operator*(double a, const Coord3D& v)
    { return Coord3D(v) *= a; }

    // Multiply the 3D-coordinate \a v with a scalar \a a.
    inline Coord3D operator*(const Coord3D& v, double a)
    { return Coord3D(v) *= a; }

    // Divide the 3D-coordinate \a v by a scalar \a a.
    // \throw MathException when \a == 0
    inline Coord3D operator/(const Coord3D& v, double a)
    { return Coord3D(v) /= a; }
    
    // Calculate the inner product of two vectors originating at the origin
    // and pointing to 3D-coordinates \a lhs and \a rhs respectively.
    double operator*(const Coord3D& lhs, const Coord3D& rhs);

    // Compare two 3D-coordinates for equality.
    inline bool operator==(const Coord3D& lhs, const Coord3D& rhs)
    { return lhs.get() == rhs.get(); }

    // Output a 3D-coordinate in ASCII format.
    ostream& operator<< (ostream&, const Coord3D&);

    // @}

  } // namespace AMC

} // namespace LOFAR

#endif
