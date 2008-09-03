//# ParmDomain.h: The domain for a parm value
//#
//# Copyright (C) 2005
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

#ifndef LOFAR_PARMDB_PARMDOMAIN_H
#define LOFAR_PARMDB_PARMDOMAIN_H

// \file
// The domain for a parameter value.

#include <vector>
#include <iosfwd>

namespace LOFAR {
namespace ParmDB {

// \ingroup ParmDB
// @{

// This class represents a domain for which an expression has to be
// evaluated. The x and y-values can be scaled to avoid having very large
// values (e.g. frequencies).

class ParmDomain
{
public:
  // Create an empty domain.
  ParmDomain();

  // Create a 2-dim domain.
  explicit ParmDomain (double startX, double endX,
		       double startY, double endY);

  // Create an N-dim domain.
  ParmDomain (const std::vector<double>& start,
	      const std::vector<double>& end);

  // Get access to start or end.
  // <group>
  const std::vector<double>& getStart() const
  { return itsStart; }
  const std::vector<double>& getEnd() const
  { return itsEnd; }
  // </group>

  bool operator== (const ParmDomain& that) const;

  friend std::ostream& operator<< (std::ostream&, const ParmDomain&);

private:
  std::vector<double> itsStart;
  std::vector<double> itsEnd;
};

// @}

} // namespace ParmDB
} // namespace LOFAR

#endif
