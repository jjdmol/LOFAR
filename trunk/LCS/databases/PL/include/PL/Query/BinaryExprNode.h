//#  BinaryExprNode.h: Binary expression node.
//#
//#  Copyright (C) 2002-2003
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//#  This program is free software; you can redistribute it and/or modify
//#  it under the terms of the GNU General Public License as published by
//#  the Free Software Foundation; either version 2 of the License, or
//#  (at your option) any later version.
//#
//#  This program is distributed in the hope that it will be useful,
//#  but WITHOUT ANY WARRANTY; without even the implied warranty of
//#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//#  GNU General Public License for more details.
//#
//#  You should have received a copy of the GNU General Public License
//#  along with this program; if not, write to the Free Software
//#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#
//#  $Id$

#ifndef LOFAR_PL_QUERY_BINARYEXPRNODE_H
#define LOFAR_PL_QUERY_BINARYEXPRNODE_H

// \file BinaryExprNode.h
// Binary expression node.

//# Includes
#include <lofar_config.h>
#include <PL/Query/Expr.h>
#include <PL/Query/ExprNode.h>
#include <string>

namespace LOFAR
{
  namespace PL
  {
    namespace Query
    {
      // \ingroup QueryExpr
      // \defgroup BinaryExprNode Binary Expression Nodes
      // A binary expression is an expression that takes one operator and two
      // operands.
      // @{

      // This class represents a binary expression node. A binary expression
      // is an expression that takes one operator and two operands, one
      // left-hand-side operand and one right-hand-side operand.
      class BinaryExprNode : public ExprNode
      {
      public:
        BinaryExprNode(const std::string& oper, 
                       const Expr& lhs, const Expr& rhs);

        virtual ~BinaryExprNode();

        virtual void print(std::ostream& os) const;

        virtual Expr getConstraint() const;

      private:

        // \name The operation
        const std::string itsOperation;

        // \name The operands
        // @{
        const Expr        itsLeft;
        const Expr        itsRight;
        // @}

      };

      // @}

    } // namespace Query

  } // namespace PL

} // namespace LOFAR

#endif
