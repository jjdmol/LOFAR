//#  LogicalUnaryExprNode.h: one line description
//#
//#  Copyright (C) 2002-2004
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

#ifndef LOFAR_PL_QUERY_LOGICALUNARYEXPRNODE_H
#define LOFAR_PL_QUERY_LOGICALUNARYEXPRNODE_H

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

      //# Forward Declarations

      // @ingroup UnaryExprNode
      //
      // This class represents a logcial unary expression node. A logical
      // unary expression is a unary expression that represents a unary
      // logcial operator (e.g. \c operator!). The main difference
      // between an ordinary UnaryExprNode and a LogicalUnaryExprNode is that
      // the latter does not have an associated constraint. The reason for
      // this is that a constraint can only be associated with the \e branch
      // emanating from a logical expression node, not with the node itself.
      //
      // \note We do not need to override the method getConstraint(), because
      // there is no constraint associated with a logical unary expression
      // node. Hence, we can use the default implementation in ExprNode.
      class LogicalUnaryExprNode : public ExprNode
      {
      public:
        // Construct a unary expression node. 
        LogicalUnaryExprNode(const std::string& oper, 
                             const Expr& value);

        virtual ~LogicalUnaryExprNode() {}

        virtual void print(std::ostream& os) const;

      private:

        // The operation
        const std::string itsOperation;

        // The operand
        const Expr        itsOperand;

      };

    } // namespace Query

  } // namespace PL

} // namespace LOFAR

#endif
