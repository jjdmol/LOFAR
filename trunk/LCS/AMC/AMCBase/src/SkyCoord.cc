//# SkyCoord.cc: Class to hold a sky coordinate as 2 angles
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
#include <AMCBase/SkyCoord.h>
#include <Common/lofar_iostream.h>
#include <cmath>

namespace LOFAR
{
  namespace AMC
  {

    SkyCoord::SkyCoord (double angle0, double angle1, Types typ)
      : itsAngle0(angle0), itsAngle1(angle1) 
    {
      if (INVALID < typ && typ < N_Types) itsType = typ; 
      else itsType = INVALID;
    }
    

    const string& SkyCoord::showType() const
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


    vector<double> SkyCoord::xyz() const
    {
      vector<double> p(3);
      double tmp = std::cos(itsAngle1);
      p[0] = std::cos(itsAngle0) * tmp;
      p[1] = std::sin(itsAngle0) * tmp;
      p[2] = std::sin(itsAngle1);
      return p;
    }


    ostream& operator<<(ostream& os, const SkyCoord& sky)
    {
      if (!sky.isValid()) 
        return os << sky.showType();
      else
        return os << "[" << sky.angle0() << ", " << sky.angle1()
                  << "] (" << sky.showType() << ")";
    }


    bool operator==(const SkyCoord& lhs, const SkyCoord& rhs)
    {
      return 
        lhs.isValid() && rhs.isValid() &&
        lhs.angle0()  == rhs.angle0()  && 
        lhs.angle1()  == rhs.angle1()  &&
        lhs.type()    == rhs.type();
    }

  } // namespace AMC

} // namespace LOFAR
