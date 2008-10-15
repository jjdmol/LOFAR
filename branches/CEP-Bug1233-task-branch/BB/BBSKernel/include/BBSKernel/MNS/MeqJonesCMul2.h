//# JonesCMul2.h: Calculate left*conj(right)
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
#ifndef MNS_MEQJONESCMUL2_H
#define MNS_MEQJONESCMUL2_H

// \file
// Calculate left*conj(transpose(right))

//# Includes
#include <BBSKernel/MNS/MeqJonesExpr.h>

namespace LOFAR
{
namespace BBS
{

// \ingroup BBSKernel
// \ingroup MNS
// @{


// Calculate left*conj(transpose(right)).

class JonesCMul2: public JonesExprRep
{
public:
  JonesCMul2 (const JonesExpr& left, const JonesExpr& right);

  ~JonesCMul2();

  // Get the result of the expression for the given domain.
  JonesResult getJResult (const Request&);

private:
  JonesExpr itsLeft;
  JonesExpr itsRight;
};

// @}

} // namespace BBS
} // namespace LOFAR

#endif
