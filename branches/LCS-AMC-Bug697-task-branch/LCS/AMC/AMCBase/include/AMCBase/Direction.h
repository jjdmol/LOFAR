//# Direction.h: Class to hold a sky coordinate as 2 angles
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
// Class to hold a sky coordinate as 2 angles

//# Forward Declarations.
#include <Common/lofar_iosfwd.h>
#include <Common/lofar_string.h>
#include <Common/lofar_vector.h>

namespace LOFAR
{
  namespace AMC
  {

    // \addtogroup AMCBase
    // @{

    // This class represents a position in the sky. The position is stored as
    // a direction using a pair of angles. The context where the object is
    // used defines the coordinate system and frame, so the class can be used
    // for any pair of sky coordinates (like RA/DEC and AZ/ELEV). The correct
    // interpretation of the coordinates should be done by the user of this
    // class.
    class Direction
    {
    public:
      // Types of sky coordinates. Currently, only three types are supported:
      // J2000, AZEL and ITRF.
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

      // Create a sky coordinate by giving the longitude \a lon and latitude
      // \a lat in radians and the reference type \a typ.
      Direction(double lon, double lat, Types typ = J2000);

      // Create a sky coordinate from the direction cosines \a xyz and the
      // reference type \a typ.
      explicit Direction(const vector<double>& xyz, Types typ = J2000);

      // Return the sky coordinate as direction cosines.
      vector<double> get() const
      { return itsXYZ; }

      // Return the reference type.
      Types type() const
      { return itsType; }

      // Return whether sky coordinate type is valid.
      bool isValid() const
      { return itsType != INVALID; }

      // Return the longitude in radians. This could be, for example, right
      // ascension (RA) or azimuth (AZ).
      double longitude() const;

      // Return the latitude in radians. This could be, for example,
      // declination (DEC) or elevation (EL).
      double latitude() const;

      // Return the reference type as a string.
      const string& showType() const;

    private:
      // Longitude and latitude are stored internally as direction cosines.
      vector<double> itsXYZ;

      // Type of sky coordinate.
      Types itsType;
    };

    // Output a Direction in ASCII format.
    ostream& operator<< (ostream&, const Direction&);

    // Compare two Direction objects for equality. 
    // \note Two invalid objects can \e never be equal.
    bool operator==(const Direction& lhs, const Direction& rhs);

    // @}

  } // namespace AMC

} // namespace LOFAR


#endif
