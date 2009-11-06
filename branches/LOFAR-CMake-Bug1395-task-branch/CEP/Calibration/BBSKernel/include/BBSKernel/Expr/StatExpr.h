//# StatExpr.h: The Jones expression for a station
//#
//# Copyright (C) 2002
//# ASTRON (Netherlands Institute for Radio Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands
//#
//# This file is part of the LOFAR software suite.
//# The LOFAR software suite is free software: you can redistribute it and/or
//# modify it under the terms of the GNU General Public License as published
//# by the Free Software Foundation, either version 3 of the License, or
//# (at your option) any later version.
//#
//# The LOFAR software suite is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License along
//# with the LOFAR software suite. If not, see <http://www.gnu.org/licenses/>.
//#
//# $Id$

#if !defined(EXPR_STATEXPR_H)
#define EXPR_STATEXPR_H

// \file
// The Jones expression for a station

//# Includes
#include <BBSKernel/Expr/JonesExpr.h>
#include <BBSKernel/Expr/Expr.h>

namespace LOFAR
{
namespace BBS
{

// \ingroup Expr
// @{

//# Forward Declarations
class Expr;

// This class is a node in a Jones matrix expression.

class StatExpr: public JonesExprRep
{
public:
  // Construct from the various subexpressions.
  StatExpr (const Expr& faradayRotation,
           const Expr& dipoleRotation,
           const Expr& dipoleEllipticity,
           const Expr& gain1,
           const Expr& gain2);

  virtual ~StatExpr();

  // Calculate the result of its members.
  virtual JonesResult getJResult (const Request&);

private:
  Expr itsFarRot;
  Expr itsDipRot;
  Expr itsDipEll;
  Expr itsGain1;
  Expr itsGain2;
};

// @}

} // namespace BBS
} // namespace LOFAR

#endif
