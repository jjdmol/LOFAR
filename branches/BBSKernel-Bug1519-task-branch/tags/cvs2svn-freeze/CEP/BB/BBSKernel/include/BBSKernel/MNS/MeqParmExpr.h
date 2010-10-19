//# MeqParmExpr.h: A parm expression
//#
//# Copyright (C) 2006
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

#ifndef MNS_MEQPARMEXPR_H
#define MNS_MEQPARMEXPR_H

// \file
// A parm which is an expression of other parms.

//# Includes
#include <BBSKernel/MNS/MeqExpr.h>
#include <ParmDB/ParmDB.h>

namespace LOFAR
{
namespace BBS
{

// \ingroup BBSKernel
// \addtogroup MNS
// @{

//# Forward declarations
class MeqParmGroup;

// MeqParmExpr represents a parm which is an expression of other parms.
// The current implementation is very limited, as it can only handle
// the multiplication or substraction of two parms.
// This is sufficient for the first needs.
// It should be rather simple to make more complicated expressions possible.

class MeqParmExpr: public MeqExprRep
{
public:
  // Create from an expression.
  MeqParmExpr (const string& expr, MeqParmGroup&, LOFAR::ParmDB::ParmDB* table);

  virtual ~MeqParmExpr();

  // Calculate the value.
  MeqMatrix getResultValue (const std::vector<const MeqMatrix*>&);

private:
  MeqExpr itsExpr1;
  MeqExpr itsExpr2;
  int     itsType;   // 1=subtract, 2=multiply
};

// @}

} // namespace BBS
} // namespace LOFAR

#endif
