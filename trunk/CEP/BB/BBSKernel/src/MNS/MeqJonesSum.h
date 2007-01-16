//# MeqJonesSum.h: A summation of MeqJonesExpr
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

#ifndef MNS_MEQJONESSUM_H
#define MNS_MEQJONESSUM_H

// \file
// A sum of MeqJonesExpr

//# Includes
#include <BBSKernel/MNS/MeqJonesExpr.h>
#include <vector>

namespace LOFAR
{
namespace BBS
{

// \ingroup BBSKernel
// \addtogroup MNS
// @{


// This class adds the results of multiple MeqJonesExpr objects.

class MeqJonesSum: public MeqJonesExprRep
{
public:
  // Construct from four Jones elements.
  MeqJonesSum (const std::vector<MeqJonesExpr>& expr);

  virtual ~MeqJonesSum();

  // Calculate the result of its members.
  virtual MeqJonesResult getJResult (const MeqRequest&);

private:
#ifdef EXPR_GRAPH
  virtual std::string getLabel();
#endif

  std::vector<MeqJonesExpr> itsExpr;
};

// @}

} // namespace BBS
} // namespace LOFAR

#endif
