//# Position.h: Class for storing a position on earth given as (lon,lat,height)
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

    // Convert the geodetic longitude, latitude and height in WGS84 to
    // geocentric coordinates in ITRF.
    // \see  Explanatory Supplement to the Astronomical Almanac (1992),
    //       Section 4.22
    Coord3D wgs84ToItrf(double lon, double lat, double h);

    // Earth parameters needed when converting from geodetic to geocentric
    // coordinates.
    struct Earth
    {
      // Flattening
      static double flattening();
      // Equatorial radius
      static double equatorialRadius();
    };


    // This class represents a position on earth. Internally, the position is
    // stored in cartesian coordinates, relative to the geocentric ITRF frame,
    // using class Coord3D. A position can be constructed from longitude,
    // latitude and height, or from a vector containing cartesian coordinates
    // (x,y,z). 
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

      // Default constructor creates a position at the origin of the ITRF
      // reference frame.
      Position();

      // Create a position by giving the longitude \a lon and latitude \a lat
      // in radians and the \a h in meters. Reference type can be either \c
      // ITRF (default), or \c WGS84. The parameters \a lon, \a lat, and \a h
      // are interpreted differently for different values of \a typ.
      // - \a typ = \c ITRF: position is geocentric in spherical coordinates.
      //   \param lon spherical longitude in radians
      //   \param lat spherical latitude in radians
      //   \param h   distance to the geocenter
      // - \a typ = \c WGS84: position is geodetic, relative to the \c WGS84
      //                      ellipsoid.
      //   \param lon geodetic longitude in radians
      //   \param lat geodetic latitude in radians
      //   \param h   height above the \c WGS84 ellipsoid.
      Position(double lon, double lat, double h, Types typ = ITRF);

      // Create a position from the 3D-coordinate \a coord and the reference
      // type \a typ. Reference type can be either \c ITRF (default), or \c
      // WGS84. The parameter \a coord is interpreted differently for
      // different values of \a typ.
      // - \a typ = \c ITRF: position is geocentric in cartesian coordinates.
      //   \param coord represents the \a x, \a y, and \a z coordinates of the
      //          position, relative to the ITRF.
      // - \a typ = \c WGS84: position is geodetic, relative to the \c WGS84
      //                      ellipsoid.
      //   \param coord contains the direction cosines of the \e geodetical 
      //          longitude and latitude. The norm of \a coord is taken as the
      //          height above the WGS84 ellipsoid.
      Position(const Coord3D& coord, Types typ = ITRF);
      
      // Return the position coordinates in the ITRF.
      const Coord3D& coord() const
      { return itsCoord; }

      // Add Position \a that to \c this. 
      Position& operator+=(const Position& that);

      // Subtract Position \a that from \c this. 
      Position& operator-=(const Position& that);

      // Multiply \c this with the scalar \a a.
      Position& operator*=(double a);

      // Divide \c this by a scalar \a a.
      Position& operator/=(double a);

      // Calculate the inner product of \c this and the Position \a that.
      double operator*(const Position& that) const;

      // Calculate the inner product of \c this and the Direction \a that.
      // \throw TypeException if the reference types of the Direction \a that
      // is not \c ITRF.
      double operator*(const Direction& that) const;

    private:
      // The position coordinates in the ITRF.
      Coord3D itsCoord;
    };

    // Calculate the sum of two Positions. 
    inline Position operator+(const Position& lhs, const Position& rhs)
    { return Position(lhs) += rhs; }

    // Calculate the difference between two Positions. 
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
    bool operator==(const Position& lhs, const Position& rhs);

    // @}

  } // namespace AMC

} // namespace LOFAR

#endif
