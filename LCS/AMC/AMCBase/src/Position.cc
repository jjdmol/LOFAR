//# Position.cc: Class to hold an earth coordinate as lon,lat,height
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

    double flattening(Position::Types type)
    {
      switch(type) {
      case Position::WGS84:
        // GPS week 1150, realisation 2007
        return 1.0 / 298.257223563;
      case Position::ITRF:
        // Model ITRF2005, realisation 2007
        return 1.0 / 298.257222101;
      default:
        return 0;
      }
    }


    // Equatorial radius in meters, WGS84 and ITRF as above.
    double equatorialRadius()
    {
      return 6378137.0;
    }


    // Function C as defined in the Astronomical Almanac 2008, section K.12.
    double functionC(double geographicalLatitude, double flattening)
    {
      double cosphi;       // Cosine of the geographical latitude.
      double sinphi;       // Sine of the geographical latitude.
      cosphi = cos(geographicalLatitude);
      sinphi = sin(geographicalLatitude);
      return 
        1/sqrt(cosphi*cosphi+(1-flattening)*(1-flattening)*sinphi*sinphi);
    }


    // Function S as defined in the Astronomical Almanac 2008, section K.12.
    double functionS(double flattening, 
                     double c /* result of calling functionC */)
    {
      return (1-flattening)*(1-flattening)*c;
    }
    

    //## --------  Public member functions  -------- ##//

    Position::Position() : 
      itsCoord(),
      itsType(ITRF)
    {
    }
    

    Position::Position (double lon, double lat, double h, Types typ) :
      itsCoord(),
      itsType((INVALID < typ && typ < N_Types) ? typ : INVALID) 
    {
      double c = functionC(lat, flattening(typ));
      double s = functionS(flattening(typ), c);
      double a = equatorialRadius();
      vector<double> xyz(3);
      // Equations according to Astronomical Almanac 2008, section K.12.
      xyz[0] = (a*c + h) * cos(lat) * cos(lon);
      xyz[1] = (a*c + h) * cos(lat) * sin(lon);
      xyz[2] = (a*s + h) * sin(lat);
      itsCoord = Coord3D(xyz);
    }


    Position::Position(const Coord3D& coord, Types typ) :
      itsCoord(coord),
      itsType((INVALID < typ && typ < N_Types) ? typ : INVALID)
    {
    }


    const string& Position::showType() const
    {
      //# Caution: Always keep this array of strings in sync with the enum
      //#          Types that is defined in the header file!
      static const string types[N_Types+1] = {
        "ITRF",
        "WGS84",
        "<INVALID>"
      };
      if (isValid()) return types[itsType];
      else return types[N_Types];
    }


    Position& Position::operator+=(const Position& that)
    {
      if (itsType != that.itsType) {
        THROW (TypeException, showType() << " != " << that.showType());
      }
      itsCoord += that.itsCoord;
      return *this;
    }


    Position& Position::operator-=(const Position& that)
    {
      if (itsType != that.itsType) {
        THROW (TypeException, showType() << " != " << that.showType());
      }
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


    double Position::operator*(const Position& that)
    {
      if (itsType != that.itsType) {
        THROW (TypeException, showType() << " != " << that.showType());
      }
      return itsCoord * that.coord();
    }


    double Position::operator*(const Direction& that)
    {
      // Here we must convert the type to string before comparing, because
      // we're comparing Position::Types with Direction::Types.
      if (showType() != that.showType()) {
        THROW (TypeException, showType() << " != " << that.showType());
      }
      return itsCoord * that.coord();
    }


    //## --------  Global functions  -------- ##//

    ostream& operator<<(ostream& os, const Position& pos)
    {
      if (!pos.isValid()) 
        return os << pos.showType();
      else 
        return os << pos.coord().get() << "(" << pos.showType() << ")";
    }


    bool operator==(const Position& lhs, const Position& rhs)
    {
      return 
        lhs.isValid() && rhs.isValid() &&
        lhs.type()    == rhs.type()    &&
        lhs.coord()   == rhs.coord();
    }


  } // namespace AMC

} // namespace LOFAR
