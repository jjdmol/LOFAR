//# EarthCoord.cc: Class to hold an earth coordinate as lon,lat,height
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
#include <AMCBase/EarthCoord.h>
#include <Common/lofar_iostream.h>
#include <cmath>

namespace LOFAR
{
  namespace AMC
  {

    EarthCoord::EarthCoord (double longitude, double latitude, double height,
                            Types typ)
      : itsLong(longitude), itsLat(latitude), itsHeight(height)
    {
      if (INVALID < typ && typ < N_Types) itsType = typ; 
      else itsType = INVALID;
    }


    const string& EarthCoord::showType() const
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


    vector<double> EarthCoord::xyz() const
    {
      vector<double> p(3);
      double tmp = std::cos(itsLat);
      p[0] = itsHeight * std::cos(itsLong) * tmp;
      p[1] = itsHeight * std::sin(itsLong) * tmp;
      p[2] = itsHeight * std::sin(itsLat);
      return p;
    }


    ostream& operator<<(ostream& os, const EarthCoord& pos)
    {
      if (!pos.isValid()) 
        return os << pos.showType();
      else 
        return os << "["  << pos.longitude() << ", " << pos.latitude() 
                  << ", " << pos.height() << "] (" << pos.showType() << ")";
    }


    bool operator==(const EarthCoord& lhs, const EarthCoord& rhs)
    {
      return 
        lhs.isValid()   && rhs.isValid()   &&
        lhs.longitude() == rhs.longitude() &&
        lhs.latitude()  == rhs.latitude()  &&
        lhs.height()    == rhs.height()    &&
        lhs.type()      == rhs.type();
    }

  } // namespace AMC

} // namespace LOFAR
