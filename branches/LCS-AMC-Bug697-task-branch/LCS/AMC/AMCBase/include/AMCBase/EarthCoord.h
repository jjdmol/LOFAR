//# EarthCoord.h: Class to hold an earth coordinate as lon,lat,height
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

#ifndef LOFAR_AMCBASE_EARTHCOORD_H
#define LOFAR_AMCBASE_EARTHCOORD_H

// \file
// Class to hold an earth coordinate as lon,lat,height

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

    // This class represents a position on earth. The position is stored using
    // longitude, latitude and height. The context where the object is used
    // defines the coordinate system and frame, so the class can be used in
    // any kind of frame (like ITRF and geocentric). The correct
    // interpretation of the coordinates should be done by the user of this
    // class.
    class EarthCoord
    {
    public:
      // Types of earth coordinates. 
      enum Types {
        INVALID = -1,   ///< Used when specified value is out of range.
        ITRF,
        WGS84,
        //# Insert new types HERE !!
        N_Types         ///< Number of reference types.
      };

      // Default constructor uses 0 for longitude, latitude and height, and
      // ITRF as reference type.
      EarthCoord();
      
      // Create an earth coordinate by giving the longitude \a lon and
      // latitude \a lat in radians and the \a h in meters. Reference type can
      // be either ITRF (default), or WGS84.
      EarthCoord(double lon, double lat, double h = 0, Types typ = ITRF);

      // Create an earth coordinate from the cartesian coordinates \a xyz and
      // the reference type \a typ.
      explicit EarthCoord(const vector<double>& xyz, Types typ = ITRF);
      
      // Return the earth coordinate in cartesian coordinates.
      vector<double> get() const
      { return itsXYZ; }

      // Return the reference type.
      Types type() const
      { return itsType; }

      // Return whether sky coordinate type is valid.
      bool isValid() const
      { return itsType != INVALID; }

      // Return the longitude in radians.
      double longitude() const;
      
      // Return the latitude in radians.
      double latitude() const;
      
      // Return the height in meters.
      double height() const;
      
      // Return the reference type as a string.
      const string& showType() const;

    private:
      // Longitude, latitude and height are stored internally in cartesian
      // coordinates.
      vector<double> itsXYZ;

      // Type of earth coordinate.
      Types itsType;
    };

    // Output an EarthCoord in ASCII format.
    ostream& operator<< (ostream&, const EarthCoord&);

    // Compare two EarthCoord objects for equality.
    // \note Two invalid objects can \e never be equal.
    bool operator==(const EarthCoord& lhs, const EarthCoord& rhs);

    // Calculate the dot product of \a lhs and \a rhs
    double operator*(const EarthCoord& lhs, const EarthCoord& rhs);
      
    // @}

  } // namespace AMC

} // namespace LOFAR

#endif
