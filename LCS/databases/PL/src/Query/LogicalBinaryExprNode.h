//#  LogicalBinaryExprNode.h: one line description
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

#ifndef LOFAR_PL_QUERY_LOGICALBINARYEXPRNODE_H
#define LOFAR_PL_QUERY_LOGICALBINARYEXPRNODE_H

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
      // @ingroup BinaryExprNode
      //
      // This class represents a logcial binary expression node. A logical
      // binary expression is a binary expression that represents a binary
      // logcial operator (e.g. \c operator&& and \c operator||). The main
      // difference between an ordinary BinaryExprNode and a
      // LogicalBinaryExprNode is that the latter does not have an associated
      // constraint. The reason for this is that a constraint can only be
      // associated with the \e branch emanating from a logical expression
      // node, not with the node itself.
      //
      // \note We do not need to override the method getConstraint(), because
      // there is no constraint associated with a logical binary expression
      // node. Hence, we can use the default implementation in ExprNode.
      class LogicalBinaryExprNode : public ExprNode
      {
      public:
        // Construct a binary expression node. 
        LogicalBinaryExprNode(const std::string& oper, 
                              const Expr& lhs, const Expr& rhs);

        virtual ~LogicalBinaryExprNode() {}

        virtual void print(std::ostream& os) const;

        virtual bool isNull() const;

      private:

        // @name The operation
        const std::string itsOperation;

        // @name The operands
        //@{
        const Expr        itsLeft;
        const Expr        itsRight;
        //@}

      };

    } // namespace Query

  } // namespace PL

} // namespace LOFAR

#endif
