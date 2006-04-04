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
#include <Common/lofar_iostream.h>
#include <Common/lofar_math.h>
#include <Common/LofarLogger.h>
#include <limits>

using namespace std;

namespace LOFAR
{
  namespace AMC
  {

    Position::Position() : 
      itsXYZ(3, 0.0), itsType(ITRF)
    {
    }
    

    Position::Position (double lon, double lat, double h, Types typ) :
      itsXYZ(3, std::numeric_limits<double>::quiet_NaN()), 
      itsType((INVALID < typ && typ < N_Types) ? typ : INVALID) 
    {
      if (isValid()) {
        double tmp = cos(lat);
        itsXYZ[0] = h * cos(lon) * tmp;
        itsXYZ[1] = h * sin(lon) * tmp;
        itsXYZ[2] = h * sin(lat);
      }
    }


    Position::Position(const vector<double>& xyz, Types typ) :
      itsXYZ(3, std::numeric_limits<double>::quiet_NaN()), 
      itsType((INVALID < typ && typ < N_Types) ? typ : INVALID)
    {
      ASSERT(xyz.size() == 3);
      if (isValid()) itsXYZ = xyz;
    }


    double Position::longitude() const
    {
      return atan2(itsXYZ[1], itsXYZ[0]);
    }
    

    double Position::latitude() const
    {
      double h(height());
      if (h == 0) return asin(itsXYZ[2]);
      else return (asin(itsXYZ[2] / h));
    }
    

    double Position::height() const
    {
      return sqrt(itsXYZ[0] * itsXYZ[0] + 
                  itsXYZ[1] * itsXYZ[1] +
                  itsXYZ[2] * itsXYZ[2]);
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


    ostream& operator<<(ostream& os, const Position& pos)
    {
      if (!pos.isValid()) 
        return os << pos.showType();
      else 
        return os << "["  << pos.longitude() << ", " << pos.latitude() 
                  << ", " << pos.height() << "] (" << pos.showType() << ")";
    }


    bool operator==(const Position& lhs, const Position& rhs)
    {
      return 
        lhs.isValid() && rhs.isValid() &&
        lhs.type()    == rhs.type()    &&
        lhs.get()     == rhs.get();
    }


    double operator*(const Position& lhs, const Position& rhs)
    {
      double result(0);
      vector<double> x(lhs.get());
      vector<double> y(rhs.get());
      ASSERT(x.size() == y.size());
      for (uint i = 0; i < x.size(); ++i) {
        result += x[i] * y[i];
      }
      return result;
    }
    
  } // namespace AMC

} // namespace LOFAR
