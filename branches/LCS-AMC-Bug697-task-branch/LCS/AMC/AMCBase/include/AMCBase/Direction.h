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
#include <AMCBase/Position.h>

namespace LOFAR
{
  namespace AMC
  {

    // \addtogroup AMCBase
    // @{

    // This class represents a direction on the sky. The direction is stored
    // in cartesian coordinates as so-called direction cosines. A direction
    // can be constructed from a pair of angles (longitude and latitude) or a
    // vector of direction cosines. The context where the object is used
    // defines the coordinate system and frame, so the class can be used for
    // any pair of sky coordinates (like RA/DEC and AZ/ELEV). The correct
    // interpretation of the coordinates should be done by the user of this
    // class.
    // \note Direction is a specialization of Position, because a direction
    // vector is in fact a position vector with a unit length.
    class Direction : public Position
    {
    public:
      // Types of direction. Currently, only three types are supported: J2000,
      // AZEL and ITRF.
      enum Types {
        INVALID = -1,   ///< Used when specified value is out of range.
        J2000,         
        AZEL,
        ITRF,
        //# Insert new types HERE !!
        N_Types         ///< Number of reference types.
      };

      // Default constructor uses 0 for the longitude and latitude, and J2000
      // as reference type.
      Direction();

      // Create a direction by giving the longitude \a lon and latitude \a lat
      // in radians and the reference type \a typ.
      Direction(double lon, double lat, Types typ = J2000);

      // Create a direction from the cartesian coordinates \a xyz and the
      // reference type \a typ.
      explicit Direction(const vector<double>& xyz, Types typ = J2000);

//       // Return the direction as direction cosines.
//       vector<double> get() const
//       { return itsXYZ; }

//       // Return the longitude in radians. This could be, for example, right
//       // ascension (RA) or azimuth (AZ).
//       double longitude() const;

//       // Return the latitude in radians. This could be, for example,
//       // declination (DEC) or elevation (EL).
//       double latitude() const;

      // Return whether direction type is valid.
      virtual bool isValid() const
      { return itsType != INVALID; }

      // Return the direction type as an \c int.
      virtual int type() const
      { return itsType; }

      // Return the direction type as a string.
      virtual const string& showType() const;

    private:
//       // Direction is stored internally as direction cosines.
//       vector<double> itsXYZ;

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
