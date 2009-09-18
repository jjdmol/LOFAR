//# JonesNode.h: A node in a Jones matrix expression.
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

#if !defined(EXPR_JONESNODE_H)
#define EXPR_JONESNODE_H

// \file
// A node in a Jones matrix expression.

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
class JonesResult;


// This class is a node in a Jones matrix expression.

class JonesNode: public JonesExprRep
{
public:
  // Construct from four Jones elements.
  JonesNode (const Expr& elem11, const Expr& elem12,
        const Expr& elem21, const Expr& elem22);

  virtual ~JonesNode();

  // Calculate the result of its members.
  virtual JonesResult getJResult (const Request&);

private:

  Expr itsExpr11;
  Expr itsExpr12;
  Expr itsExpr21;
  Expr itsExpr22;
};

// @}

} // namespace BBS
} // namespace LOFAR

#endif
