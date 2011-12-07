//# Direction.cc: Class to hold a sky coordinate as 2 angles
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
#include <AMCBase/Direction.h>
#include <AMCBase/Position.h>
#include <AMCBase/Exceptions.h>
#include <Common/LofarLogger.h>
#include <Common/lofar_iostream.h>

namespace LOFAR
{
  namespace AMC
  {

    Direction::Direction() : 
      itsCoord(0.0, 0.0, 1.0), 
      itsType(J2000)
    {
    }


    Direction::Direction (double lon, double lat, Types typ) :
      itsCoord(lon, lat, 1.0),
      itsType((INVALID < typ && typ < N_Types) ? typ : INVALID) 
    {
    }
    

    Direction::Direction(const Coord3D& coord, Types typ) :
      itsCoord(coord),
      itsType((INVALID < typ && typ < N_Types) ? typ : INVALID)
    {
      itsCoord.normalize();
    }


    const string& Direction::showType() const
    {
      //# Caution: Always keep this array of strings in sync with the enum
      //#          Types that is defined in the header file!
      static const string types[N_Types+1] = {
        "J2000",
        "AZEL",
        "ITRF",
        "<INVALID>"
      };
      if (isValid()) return types[itsType];
      else return types[N_Types];
    }


    Direction& Direction::operator+=(const Direction& that)
    {
      if (itsType != that.itsType) {
        THROW (TypeException, showType() << " != " << that.showType());
      }
      itsCoord += that.itsCoord;
      return *this;
    }


    Direction& Direction::operator-=(const Direction& that)
    {
      if (itsType != that.itsType) {
        THROW (TypeException, showType() << " != " << that.showType());
      }
      itsCoord -= that.itsCoord;
      return *this;
    }


    Direction& Direction::operator*=(double a)
    {
      itsCoord *= a;
      return *this;
    }


    Direction& Direction::operator/=(double a)
    {
      itsCoord /= a;
      return *this;
    }


    double Direction::operator*(const Direction& that) const
    {
      if (itsType != that.itsType) {
        THROW (TypeException, showType() << " != " << that.showType());
      }
      return itsCoord * that.coord();
    }


    double Direction::operator*(const Position& that) const
    {
      if (itsType != ITRF) {
        THROW (TypeException, "Direction type must be ITRF");
      }
      return itsCoord * that.coord();
    }


    //## --------  Global functions  -------- ##//

    ostream& operator<<(ostream& os, const Direction& dir)
    {
      if (!dir.isValid()) 
        return os << dir.showType();
      else
        return os << "[" << dir.longitude() << ", " << dir.latitude()
                  << "] (" << dir.showType() << ")";
    }


    bool operator==(const Direction& lhs, const Direction& rhs)
    {
      return 
        lhs.isValid() && rhs.isValid() &&
        lhs.type()    == rhs.type()    &&
        lhs.coord()   == rhs.coord();
    }

  } // namespace AMC

} // namespace LOFAR
