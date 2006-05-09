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
    class Position : public Coord3D
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

      // Create a position from the cartesian coordinates \a xyz and the
      // reference type \a typ.
      explicit Position(const vector<double>& xyz, Types typ = ITRF);
      
      // Create a position from a Coord3D object. We need this constructor,
      // because we want to be able to do the following:
      // \code
      //   Position p1, p2, p3;
      //   p3 = p1 + p2;
      // \endcode
      // However, addition, subtraction, etc. were factored out to the base
      // class Coord3D. Hence, there's only a
      // \code
      //   Coord3D operator+(const Coord3D& lhs, const Coord3D& rhs);
      // \endcode
      // So, we must tell the compiler how it can construct a Position from a
      // Coord3D object.
      //
      // \todo This clearly is a `smell' of bad design. In fact Position
      // should not be implemented in terms of inheritance but of composition;
      // it is not an <em>is-a</em> relation, but a <em>has-a</em> relation.
      Position(const Coord3D& that);

      // Return the height in meters.
      double height() const
      { return radius(); }
      
      // Return the reference type.
      Types type() const
      { return itsType; }

      // Return whether position type is valid.
      bool isValid() const
      { return itsType != INVALID; }

      // Return the position type as a string.
      const string& showType() const;

    private:
      // Reference type of the position coordinates.
      Types itsType;
    };

    // Output a position in ASCII format.
    ostream& operator<< (ostream&, const Position&);

    // Compare two positions for equality.
    // \note Two invalid positions can \e never be equal.
    bool operator==(const Position& lhs, const Position& rhs);

    // @}

  } // namespace AMC

} // namespace LOFAR

#endif
