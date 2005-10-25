//# SkyCoord.h: Class to hold a sky coordinate as 2 angles
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

#ifndef LOFAR_AMCBASE_SKYCOORD_H
#define LOFAR_AMCBASE_SKYCOORD_H

// \file SkyCoord.h
// Class to hold a sky coordinate as 2 angles

//# Forward Declarations.
#include <Common/lofar_iosfwd.h>
#include <Common/lofar_string.h>

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
    class SkyCoord
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

      // Default constructor uses 0 for the angles and J2000 as reference
      // type.
      SkyCoord()
        : itsAngle0(0), itsAngle1(0), itsType(J2000) {}

      // Create a sky coordinate by giving the longitude \a angle0 and
      // latitude \a angle1 in radians and the reference type \a typ.
      SkyCoord (double angle0, double angle1, Types typ = J2000);

      // Return angle0 in radians. This could be, for example, right ascension
      // (RA) or azimuth (AZ).
      double angle0() const
      { return itsAngle0; }

      // Return angle1 in radians. This could be, for example, declination
      // (DEC) or elevation (EL).
      double angle1() const
      { return itsAngle1; }

      // Return the reference type.
      Types type() const
      { return itsType; }

      // Return the reference type as a string.
      const string& showType() const;

      // Return whether sky coordinate type is valid.
      bool isValid() const
      { return itsType != INVALID; }

    private:
      // Angle0 in radians.
      double itsAngle0;

      // Angle1 in radians.
      double itsAngle1;

      // Type of sky coordinate.
      Types itsType;
    };

    // Output a SkyCoord in ASCII format.
    ostream& operator<< (ostream&, const SkyCoord&);

    // Compare two SkyCoord objects for equality. 
    // \note Two invalid objects can \e never be equal.
    bool operator==(const SkyCoord& lhs, const SkyCoord& rhs);

    // @}

  } // namespace AMC

} // namespace LOFAR


#endif
