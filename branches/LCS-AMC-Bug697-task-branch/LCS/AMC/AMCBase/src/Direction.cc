//# Direction.cc: Class to hold a sky coordinate as 2 angles
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
#include <AMCBase/Direction.h>
#include <Common/lofar_iostream.h>

namespace LOFAR
{
  namespace AMC
  {

    Direction::Direction() : 
      Coord3D(0.0, 0.0, 1.0), 
      itsType(J2000)
    {
    }


    Direction::Direction (double lon, double lat, Types typ) :
      Coord3D(lon, lat, 1.0),
      itsType((INVALID < typ && typ < N_Types) ? typ : INVALID) 
    {
    }
    

    Direction::Direction(const vector<double>& xyz, Types typ) : 
      Coord3D(xyz), 
      itsType((INVALID < typ && typ < N_Types) ? typ : INVALID)
    {
      normalize();
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
        lhs.get()     == rhs.get();
    }

  } // namespace AMC

} // namespace LOFAR
