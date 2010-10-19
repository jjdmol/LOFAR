//# MeqJonesCMul3.h: Calculate left*mid*conj(right)
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

#ifndef MNS_MEQJONESCMUL3_H
#define MNS_MEQJONESCMUL3_H

// \file
// Calculate left*mid*conj(right)

//# Includes
#include <BBSKernel/MNS/MeqJonesExpr.h>

namespace LOFAR
{
namespace BBS
{

// \ingroup BBSKernel
// \addtogroup MNS
// @{


// Calculate left*mid*conj(right).

class MeqJonesCMul3: public MeqJonesExprRep
{
public:
  MeqJonesCMul3 (const MeqJonesExpr& left,
         const MeqJonesExpr& mid,
         const MeqJonesExpr& right);

  ~MeqJonesCMul3();

  // Get the result of the expression for the given domain.
  MeqJonesResult getJResult (const MeqRequest&);

private:
#ifdef EXPR_GRAPH
  virtual std::string getLabel();
#endif

  MeqJonesExpr itsLeft;
  MeqJonesExpr itsMid;
  MeqJonesExpr itsRight;
};

// @}

} // namespace BBS
} // namespace LOFAR


#endif
