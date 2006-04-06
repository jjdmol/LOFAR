//# Position.h: Class to hold an earth coordinate as lon,lat,height
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

//# Includes
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
    // cartesian coordinates. A position can be constructed from longitude,
    // latitude and height, or from a vector containing cartesian coordinates
    // (x,y,z). The context where the object is used defines the coordinate
    // system and frame, so the class can be used in any kind of frame (like
    // ITRF and geocentric). The correct interpretation of the coordinates
    // should be done by the user of this class.
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

      // The destructor must be virtual destructor, because Direction is
      // derived from Position.
      virtual ~Position() {}

      // Create a position by giving the longitude \a lon and latitude \a lat
      // in radians and the \a h in meters. Reference type can be either ITRF
      // (default), or WGS84.
      Position(double lon, double lat, double h, Types typ = ITRF);

      // Create a position from the cartesian coordinates \a xyz and the
      // reference type \a typ.
      explicit Position(const vector<double>& xyz, Types typ = ITRF);
      
      // Return the position in cartesian coordinates.
      const vector<double>& get() const
      { return itsXYZ; }

      // Return the longitude in radians.
      double longitude() const;
      
      // Return the latitude in radians.
      double latitude() const;
      
      // Return the height in meters.
      double height() const;
      
      // Calculate the dot product of \c this and \a that.
      // \note We could have defined a global operator*() taking two Positions
      // as argument, however, then we wouldn't be able to calculate the dot
      // product of a Postion and a Direction.
      double operator*(const Position& that) const;

      // Return whether position type is valid.
      virtual bool isValid() const
      { return itsType != INVALID; }

      // Return the position type as an \c int.
      virtual int type() const
      { return itsType; }

      // Return the position type as a string.
      virtual const string& showType() const;

    protected:
      // Normalize the position vector to unit length. A Position need not be
      // normalized, but a Direction does. Therefore, this method was made
      // protected.
      // \return The original length of the position vector.
      double normalize();

    private:
      // Position is stored internally in cartesian coordinates.
      vector<double> itsXYZ;

      // Type of earth coordinate.
      Types itsType;
    };

    // Output a psition in ASCII format.
    ostream& operator<< (ostream&, const Position&);

    // Compare two positions for equality.
    // \note Two invalid positions can \e never be equal.
    bool operator==(const Position& lhs, const Position& rhs);

    // @}

  } // namespace AMC

} // namespace LOFAR

#endif
