//# Direction.h: Class representing a direction.
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
    class Direction : public Coord3D
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

      // Create a direction from the cartesian coordinates \a xyz and the
      // reference type \a typ.
      explicit Direction(const vector<double>& xyz, Types typ = J2000);

      // Return the reference type.
      Types type() const
      { return itsType; }

      // Return whether direction type is valid.
      bool isValid() const
      { return itsType != INVALID; }

      // Return the direction type as a string.
      const string& showType() const;

    private:
      // Type of direction.
      Types itsType;
    };

    // Output a direction in ASCII format.
    ostream& operator<< (ostream&, const Direction&);

    // Compare two directions for equality. 
    // \note Two invalid directions can \e never be equal.
    bool operator==(const Direction& lhs, const Direction& rhs);

    // @}

  } // namespace AMC

} // namespace LOFAR


#endif
