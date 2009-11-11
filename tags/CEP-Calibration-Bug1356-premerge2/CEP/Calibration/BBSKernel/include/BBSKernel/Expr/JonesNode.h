//# JonesNode.h: A node in a Jones matrix expression.
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

#if !defined(EXPR_JONESNODE_H)
#define LOFAR_BBSKERNEL_EXPR_JONESNODE_H

// \file
// A node in a Jones matrix expression.

//# Includes
#include <BBSKernel/Expr/JonesExpr.h>
#include <BBSKernel/Expr/Expr.h>

namespace LOFAR
{
namespace BBS
{

// \addtogroup Expr
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
