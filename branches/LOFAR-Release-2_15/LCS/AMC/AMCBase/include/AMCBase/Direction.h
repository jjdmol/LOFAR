//# Direction.h: Class representing a direction.
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

#ifndef LOFAR_AMCBASE_DIRECTION_H
#define LOFAR_AMCBASE_DIRECTION_H

// \file
// Class representing a direction.

//# Includes
#include <AMCBase/Coord3D.h>
#include <Common/lofar_string.h>

namespace LOFAR
{
  namespace AMC
  {

    //# Forward declarations
    class Position;

    // \addtogroup AMCBase
    // @{

    // This class represents a direction on the sky. The direction is stored
    // in cartesian coordinates as so-called direction cosines, using the
    // Coord3D class. A direction can be constructed from a pair of angles
    // (longitude and latitude) or a vector of direction cosines. The context
    // where the object is used defines the coordinate system and frame, so
    // the class can be used for any pair of sky coordinates (like RA/DEC and
    // AZ/ELEV). The correct interpretation of the coordinates should be done
    // by the user of this class.
    class Direction
    {
    public:
      // Types of direction. Currently, only three types are supported: \c
      // J2000, \c AZEL and \c ITRF.
      enum Types {
        INVALID = -1,   ///< Used when specified value is out of range.
        J2000,         
        AZEL,
        ITRF,
        //# Insert new types HERE !!
        N_Types         ///< Number of reference types.
      };

      // Default constructor uses 0 for the longitude and latitude, and \c
      // J2000 as reference type.
      Direction();

      // Create a direction by giving the longitude \a lon and latitude \a lat
      // in radians and the reference type \a typ.
      Direction(double lon, double lat, Types typ = J2000);

      // Create a direction from the 3D-coordinate \a coord and the reference
      // type \a typ. 
      // \note The direction vector will be normalized.
      Direction(const Coord3D& coord, Types typ = J2000);
      
      // Return the longitude in radians.
      double longitude() const
      { return itsCoord.longitude(); }
      
      // Return the latitude in radians.
      double latitude() const
      { return itsCoord.latitude(); }
      
      // Return the direction coordinates.
      const Coord3D& coord() const
      { return itsCoord; }

      // Return the reference type.
      Types type() const
      { return itsType; }

      // Return whether direction type is valid.
      bool isValid() const
      { return itsType != INVALID; }

      // Return the direction type as a string.
      const string& showType() const;

      // Add Direction \a that to \c this. 
      // \throw TypeException if the reference types of \c this and \a that
      // differ.
      Direction& operator+=(const Direction& that);

      // Subtract Direction \a that from \c this. 
      // \throw TypeException if the reference types of \c this and \a that
      // differ.
      Direction& operator-=(const Direction& that);

      // Multiply \c this with the scalar \a a.
      Direction& operator*=(double a);

      // Divide \c this by a scalar \a a.
      Direction& operator/=(double a);

      // Calculate the inner product of \c this and the Direction \a that.
      // \throw TypeException if the reference types of \c this and \a that
      // differ.
      double operator*(const Direction& that) const;

      // Calculate the inner product of \c this and the Position \a that.
      // \throw TypeException if the reference types of \c this and \a that
      // differ.
      double operator*(const Position& that) const;

    private:
      // The direction coordinates.
      Coord3D itsCoord;

      // Type of direction.
      Types itsType;
    };

    // Calculate the sum of two Directions. 
    // \throw TypeException if the reference types of \c this and \a that
    // differ.
    inline Direction operator+(const Direction& lhs, const Direction& rhs)
    { return Direction(lhs) += rhs; }

    // Calculate the difference between two Directions. 
    // \throw TypeException if the reference types of \c this and \a that
    // differ.
    inline Direction operator-(const Direction& lhs, const Direction& rhs)
    { return Direction(lhs) -= rhs; }

    // Multiply the Direction \a d with a scalar \a a.
    inline Direction operator*(double a, const Direction& d)
    { return Direction(d) *= a; }

    // Multiply the Direction \a d with a scalar \a a.
    inline Direction operator*(const Direction& d, double a)
    { return Direction(d) *= a; }

    // Divide the Direction \a d by a scalar \a a.
    inline Direction operator/(const Direction& d, double a)
    { return Direction(d) /= a; }
    
    // Output a direction in ASCII format.
    ostream& operator<< (ostream&, const Direction&);

    // Compare two directions for equality. 
    // \note Two invalid directions can \e never be equal.
    bool operator==(const Direction& lhs, const Direction& rhs);

    // @}

  } // namespace AMC

} // namespace LOFAR


#endif
