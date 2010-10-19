//# MeqDomain.h: The domain for an expression
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

#if !defined(MNS_MEQDOMAIN_H)
#define MNS_MEQDOMAIN_H

// \file
// The domain for an expression

#include <iosfwd>
#include <ParmDB/ParmDomain.h>

namespace LOFAR
{
namespace BBS
{

// \ingroup BBSKernel
// \addtogroup MNS
// @{

// This class represents a domain for which an expression has to be
// evaluated. The x and y-values can be scaled to avoid having very large
// values (e.g. frequencies).
// Then
//    scaledvalue = (realvalue - offset) / scale

class MeqDomain
{
public:
  // Create an  x,y default domain of -1:1,-1:1..
  MeqDomain();

  // Create an x,y domain.
  MeqDomain (double startX, double endX, double startY, double endY);

  // Create from a ParmDomain. It checks if it is a 2-dim domain.
  explicit MeqDomain (const LOFAR::ParmDB::ParmDomain&);

  // Convert to a ParmDomain.
  LOFAR::ParmDB::ParmDomain toParmDomain() const;

  // Get the start, end, and step of the domain.
  double startX() const
    { return itsStartX; }
  double endX() const
    { return itsEndX; }
  double sizeX() const
    { return itsEndX - itsStartX; }
  double startY() const
    { return itsStartY; }
  double endY() const
    { return itsEndY; }
  double sizeY() const
    { return itsEndY - itsStartY; }

  bool operator== (const MeqDomain& that) const
  { return itsStartX == that.itsStartX  &&  itsEndX == that.itsEndX
       &&  itsStartY == that.itsStartY  &&  itsEndY == that.itsEndY; }

  friend std::ostream& operator<< (std::ostream&, const MeqDomain&);

private:
  double itsStartX;
  double itsEndX;
  double itsStartY;
  double itsEndY;
};

// @}

} // namespace BBS
} // namespace LOFAR

#endif
