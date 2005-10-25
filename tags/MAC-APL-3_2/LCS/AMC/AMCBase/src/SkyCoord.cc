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
      static const string types[SkyCoord::N_Types] = {
        "J2000",
        "AZEL",
        "ITRF"
      };
      return types[itsType];
    }


    ostream& operator<<(ostream& os, const SkyCoord& sky)
    {
      if (!sky.isValid()) os << "<INVALID>";
      else os << "[" << sky.angle0() << ", " << sky.angle1()
              << "] (" << sky.showType() << ")";
      return os;
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
