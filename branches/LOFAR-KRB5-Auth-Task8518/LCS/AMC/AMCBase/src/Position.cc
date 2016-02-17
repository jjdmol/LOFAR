//# Position.cc: Class to hold an earth coordinate as lon,lat,height
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

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

//# Includes
#include <AMCBase/Position.h>
#include <AMCBase/Direction.h>
#include <AMCBase/Exceptions.h>
#include <Common/lofar_iostream.h>
#include <Common/lofar_math.h>

namespace LOFAR
{
  namespace AMC
  {

    double Earth::flattening()
    {
      return 1.0 / 298.257223563;
    }


    double Earth::equatorialRadius()
    {
      return 6378137.0;
    }


    Coord3D wgs84ToItrf(double lon, double lat, double h)
    {
      double f      = Earth::flattening();        // flattening
      double a      = Earth::equatorialRadius();  // equatorial radius
      double e2     = f*(2-f);                    // square of eccentricity
      double sinlat = sin(lat);                   // sine of latitude
      double coslat = cos(lat);                   // cosine of latitude

      // Radius of curvature in the prime vertical
      double rN = a / sqrt(1 - e2*sinlat*sinlat);

      // Calculate geocentric coordinates
      vector<double> xyz(3);
      xyz[0] = (rN + h) * coslat * cos(lon);
      xyz[1] = (rN + h) * coslat * sin(lon);
      xyz[2] = ((1-e2)*rN + h) * sinlat;

      return Coord3D(xyz);
    }


    //## --------  Public member functions  -------- ##//

    Position::Position() : 
      itsCoord(Coord3D())
    {
    }
    

    Position::Position (double lon, double lat, double h, Types typ)
    {
      switch(typ) {
      case ITRF:
        itsCoord = Coord3D(lon, lat, h);
        break;
      case WGS84:
        itsCoord = wgs84ToItrf(lon, lat, h);
        break;
      default:
        THROW(TypeException, "Invalid type (typ=" << typ << ") specified");
      }
    }


    Position::Position(const Coord3D& coord, Types typ)
    {
      switch(typ) {
      case ITRF:
        itsCoord = coord;
        break;
      case WGS84:
        itsCoord = 
          wgs84ToItrf(coord.longitude(), coord.latitude(), coord.radius());
        break;
      default:
        THROW(TypeException, "Invalid type (typ=" << typ << ") specified");
      }
    }


    Position& Position::operator+=(const Position& that)
    {
      itsCoord += that.itsCoord;
      return *this;
    }


    Position& Position::operator-=(const Position& that)
    {
      itsCoord -= that.itsCoord;
      return *this;
    }


    Position& Position::operator*=(double a)
    {
      itsCoord *= a;
      return *this;
    }


    Position& Position::operator/=(double a)
    {
      itsCoord /= a;
      return *this;
    }


    double Position::operator*(const Position& that) const
    {
      return itsCoord * that.coord();
    }


    double Position::operator*(const Direction& that) const
    {
      if (that.type() != Direction::ITRF) {
        THROW (TypeException, "Direction type must be ITRF");
      }
      return itsCoord * that.coord();
    }


    //## --------  Global functions  -------- ##//

    ostream& operator<<(ostream& os, const Position& pos)
    {
      return os << pos.coord();
    }


    bool operator==(const Position& lhs, const Position& rhs)
    {
      return lhs.coord() == rhs.coord();
    }


  } // namespace AMC

} // namespace LOFAR
