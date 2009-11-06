//# Box.cc:
//#
//# Copyright (C) 2008
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

#include <lofar_config.h>
#include <ParmDB/Box.h>

namespace LOFAR {
namespace BBS {

  Box unite (const Box& lhs, const Box& rhs)
  {
    Point start(min(lhs.lowerX(), rhs.lowerX()),
                min(lhs.lowerY(), rhs.lowerY()));
    Point end(max(lhs.upperX(), rhs.upperX()),
              max(lhs.upperY(), rhs.upperY()));
    return Box(start, end);
  }


  Box intersect (const Box& lhs, const Box& rhs)
  {
    Point start(max(lhs.lowerX(), rhs.lowerX()),
                max(lhs.lowerY(), rhs.lowerY()));
    Point end(min(lhs.upperX(), rhs.upperX()),
              min(lhs.upperY(), rhs.upperY()));
    if(start.first < end.first
       && !casa::near(start.first, end.first)
       && start.second < end.second
       && !casa::near(start.second, end.second))
      {
        return Box(start, end);
      }
    return Box();
  }

  Box::Box (const vector<double>& values)
  {
    double stx = -1e30;
    double sty = -1e30;
    double enx =  1e30;
    double eny =  1e30;
    int sz = values.size();
    if (sz > 4) sz=4;
    switch (sz) {
    case 4:
      eny = values[3];
    case 3:
      enx = values[2];
    case 2:
      sty = values[1];
    case 1:
      stx = values[0];
      break;
    default:
      break;
    }
    ASSERT (stx <= enx);
    ASSERT (sty <= eny);
    itsStart = Point(stx, sty);
    itsEnd   = Point(enx, eny);
  }

} //# namespace BBS
} //# namespace LOFAR
