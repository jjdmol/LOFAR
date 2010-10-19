//#  ColumnExprNode.h: Column expression node.
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

#ifndef LOFAR_PL_QUERY_COLUMNEXPRNODE_H
#define LOFAR_PL_QUERY_COLUMNEXPRNODE_H

// \file ColumnExprNode.h
// Column expression node.

//# Includes
//# Never #include <config.h> or #include <lofar_config.h> in a header file!
#include <PL/Query/Expr.h>
#include <PL/Query/ExprNode.h>
#include <string>

namespace LOFAR
{
  namespace PL
  {
    namespace Query
    {
      // \addtogroup QueryExpr
      // @{

      // This class represents a column expression node. A column expression
      // is an expression that takes two operands, a table name and a column
      // name. A column expression node provides the glue between the class
      // attribute in the object-oriented world and the table name and column
      // name in the relational database world.
      //
      // The key role in this class is reserved for the attribute \c
      // itsConstraint. This attribute stores the constraints that must be
      // applied when more than one class is involved in an expression. In
      // "SQL speak" we would say that the constraints represent the joins
      // that must be made when composing the SQL expression.
      //
      // Example: [TBW]
      //
      class ColumnExprNode : public ExprNode
      {
      public:

        // Construct a column expression node.
        ColumnExprNode(const std::string& tableName,
                       const std::string& columnName);

        virtual ~ColumnExprNode();
        
        // Print the expression node into an output stream.
        virtual void print(std::ostream& os) const;
        
        // Return the constraint associated with this expression node.
        virtual Expr getConstraint() const;

        // Add a constraint to this expression node.
        void addConstraint(const Expr& expr);

      private:

        const std::string itsTableName;
        const std::string itsColumnName;
        Expr              itsConstraint;

      };

      // @}

    } // namespace Query

  } // namespace PL

} // namespace LOFAR

#endif
