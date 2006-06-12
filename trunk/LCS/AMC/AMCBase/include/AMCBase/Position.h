//# Position.h: Class for storing a position on earth given as (lon,lat,height)
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

#ifndef LOFAR_AMCBASE_POSITION_H
#define LOFAR_AMCBASE_POSITION_H

// \file
// Class for storing a position on earth given as (lon,lat,height)

//# Includes
#include <AMCBase/Coord3D.h>
#include <Common/lofar_iosfwd.h>
#include <Common/lofar_string.h>

namespace LOFAR
{
  namespace AMC
  {

    //# Forward declarations
    class Direction;

    // \addtogroup AMCBase
    // @{

    // This class represents a position on earth. The position is stored in
    // cartesian coordinates, using class Coord3D. A position can be
    // constructed from longitude, latitude and height, or from a vector
    // containing cartesian coordinates (x,y,z). The context where the object
    // is used defines the coordinate system and frame, so the class can be
    // used in any kind of frame (like ITRF and geocentric). The correct
    // interpretation of the coordinates should be done by the user of this
    // class.
    class Position
    {
    public:
      // Types of position. 
      enum Types {
        INVALID = -1,   ///< Used when specified value is out of range.
        ITRF,
        WGS84,
        //# Insert new types HERE !!
        N_Types         ///< Number of reference types.
      };

      // Default constructor uses 0 for longitude, latitude and height, and
      // ITRF as reference type.
      Position();

      // Create a position by giving the longitude \a lon and latitude \a lat
      // in radians and the \a h in meters. Reference type can be either \c
      // ITRF (default), or \c WGS84.
      Position(double lon, double lat, double h, Types typ = ITRF);

      // Create a position from the 3D-coordinate \a coord and the reference
      // type \a typ.
      Position(const Coord3D& coord, Types typ = ITRF);
      
      // Return the longitude in radians.
      double longitude() const
      { return itsCoord.longitude(); }
      
      // Return the latitude in radians.
      double latitude() const
      { return itsCoord.latitude(); }
      
      // Return the height in meters.
      double height() const
      { return itsCoord.radius(); }
      
      // Return the position coordinates.
      const Coord3D& coord() const
      { return itsCoord; }

      // Return the reference type.
      Types type() const
      { return itsType; }

      // Return whether position type is valid.
      bool isValid() const
      { return itsType != INVALID; }

      // Return the position type as a string.
      const string& showType() const;

      // Add Position \a that to \c this. 
      // \throw AssertError if the reference types of \c this and \a that
      // differ.
      Position& operator+=(const Position& that);

      // Subtract Position \a that from \c this. 
      // \throw AssertError if the reference types of \c this and \a that
      // differ.
     Position& operator-=(const Position& that);

      // Multiply \c this with the scalar \a a.
      Position& operator*=(double a);

      // Divide \c this by a scalar \a a.
      Position& operator/=(double a);

      // Calculate the inner product of \c this and the Position \a that.
      // \throw AssertError if the reference types of \c this and \a that
      // differ.
      double operator*(const Position& that);

      // Calculate the inner product of \c this and the Direction \a that.
      // \throw AssertError if the reference types of \c this and \a that
      // differ.
      double operator*(const Direction& that);

    private:
      // The position coordinates.
      Coord3D itsCoord;

      // Reference type of the position coordinates.
      Types itsType;
    };

    // Calculate the sum of two Positions. 
    // \throw AssertError if the reference types of \c this and \a that
    // differ.
    inline Position operator+(const Position& lhs, const Position& rhs)
    { return Position(lhs) += rhs; }

    // Calculate the difference between two Positions. 
    // \throw AssertError if the reference types of \c this and \a that
    // differ.
    inline Position operator-(const Position& lhs, const Position& rhs)
    { return Position(lhs) -= rhs; }

    // Multiply the Position \a v with a scalar \a a.
    inline Position operator*(double a, const Position& v)
    { return Position(v) *= a; }

    // Multiply the Position \a v with a scalar \a a.
    inline Position operator*(const Position& v, double a)
    { return Position(v) *= a; }

    // Divide the Position \a v by a scalar \a a.
    inline Position operator/(const Position& v, double a)
    { return Position(v) /= a; }
    
    // Output a position in ASCII format.
    ostream& operator<< (ostream&, const Position&);

    // Compare two positions for equality.
    // \note Two invalid positions can \e never be equal.
    bool operator==(const Position& lhs, const Position& rhs);

    // @}

  } // namespace AMC

} // namespace LOFAR

#endif
