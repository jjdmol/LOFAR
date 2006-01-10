//# MeqDomain.cc: The domain for an expression
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

#include <lofar_config.h>
#include <ParmDB/ParmDomain.h>
#include <Common/LofarLogger.h>

namespace LOFAR {
namespace ParmDB {

ParmDomain::ParmDomain (double startX, double endX,
			double startY, double endY)
{
  ASSERTSTR (startX < endX, "ParmDomain: startX " << startX <<
	     " must be < endX " << endX);
  ASSERTSTR (startY < endY, "ParmDomain: startY " << startY <<
	     " must be < endY " << endY);
  itsStart.resize (2);
  itsEnd.resize (2);
  itsStart[0] = startX;
  itsStart[1] = startY;
  itsEnd[0] = endX;
  itsEnd[1] = endY;
}

  ParmDomain::ParmDomain (const std::vector<double>& start,
			  const std::vector<double>& end)
{
  ASSERTSTR (start.size() == end.size()  &&  start.size() > 0,
	     "ParmDomain: start and end vector must have equal length > 0");
  for (uint i=0; i<start.size(); ++i) {
    ASSERTSTR (start[i] < end[i], "ParmDomain: start " << start[i] <<
	     " must be < end " << end[i]);
  }
  itsStart = start;
  itsEnd = end;
}

std::ostream& operator<< (std::ostream& os, const ParmDomain& domain)
{
  os << '[';
  for (uint i=0; i<domain.getStart().size(); ++i) {
    if (i > 0) os << ',';
    os << '(' << domain.getStart()[i] << ',' << domain.getEnd()[i] << ')';
  }
  return os;
}

} // namespace ParmDB
} // namespace LOFAR
