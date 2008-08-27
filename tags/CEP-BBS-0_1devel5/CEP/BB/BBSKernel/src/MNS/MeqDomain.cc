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
#include <BBSKernel/MNS/MeqDomain.h>
#include <Common/LofarLogger.h>

namespace LOFAR
{
namespace BBS
{

MeqDomain::MeqDomain()
: itsStartX (0),
  itsEndX   (1),
  itsStartY (0),
  itsEndY   (1)
{}

MeqDomain::MeqDomain (double startX, double endX,
		      double startY, double endY)
: itsStartX (startX),
  itsEndX   (endX),
  itsStartY (startY),
  itsEndY   (endY)
{
  ASSERTSTR (startX < endX, "MeqDomain: startX " << startX <<
	     " must be < endX " << endX);
  ASSERTSTR (startY < endY, "MeqDomain: startY " << startY <<
	     " must be < endY " << endY);
}

MeqDomain::MeqDomain (const LOFAR::ParmDB::ParmDomain& pdomain)
{
  ASSERTSTR (pdomain.getStart().size() == 2,
	     "BBS only supports 2-dim funklets and domains");
  itsStartX = pdomain.getStart()[0];
  itsEndX   = pdomain.getEnd()[0];
  itsStartY = pdomain.getStart()[1];
  itsEndY   = pdomain.getEnd()[1];
}

LOFAR::ParmDB::ParmDomain MeqDomain::toParmDomain() const
{
  return LOFAR::ParmDB::ParmDomain (startX(), endX(), startY(), endY());
}

std::ostream& operator<< (std::ostream& os, const MeqDomain& domain)
{
  os << "[(" << domain.startX() << ',' << domain.endX() << "),("
     << domain.startY() << ',' << domain.endY() << ")]";
  return os;
}

} // namespace BBS
} // namespace LOFAR
