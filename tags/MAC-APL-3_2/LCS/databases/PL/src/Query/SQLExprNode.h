//#  SQLExprNode.h: Expression nodes for SQL-like operators
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

#ifndef LOFAR_PL_QUERY_SQLEXPRNODE_H
#define LOFAR_PL_QUERY_SQLEXPRNODE_H

// \file SQLExprNode.h
// Expression nodes for SQL-like operators

//# Includes
//# Never #include <config.h> or #include <lofar_config.h> in a header file!
#include <PL/Query/Expr.h>
#include <PL/Query/ExprNode.h>
#include <PL/Collection.h>
#include <string>

namespace LOFAR
{
  namespace PL
  {
    namespace Query
    {

      // \ingroup QueryExpr
      // \defgroup SQLExprNode SQL-like Expression Nodes
      //
      // A SQL-like expression is an expression that represents a SQL
      // specific operator, like \c BETWEEN, \c IN, and \c LIKE.
      //
      // @{

      // This class represents a BETWEEN expression node. A BETWEEN expression
      // is an expression that takes one operator (BETWEEN or NOT BETWEEN) and
      // three operands. It is used to check whether the first operand is in
      // the range delimited by the second and third operand.
      class BetweenExprNode : public ExprNode
      {
      public:
        // Construct a between expression node. 
        BetweenExprNode(const std::string& oper, const Expr& value, 
                        const Expr& lower, const Expr& upper);

        virtual ~BetweenExprNode();

        virtual void print(std::ostream& os) const;

        virtual Expr getConstraint() const;

      private:

        // \name The operation
        const std::string itsOperation;

        // \name The operands
        // @{
        const Expr itsValue;
        const Expr itsLower;
        const Expr itsUpper;
        // @}

      };
     

      // This class represents an IN expression node. An IN expression is an
      // expression that takes one operator (IN or NOT IN) and two
      // operands. It is used to check whether the first operand is present in
      // the second operand.
      // \note When \c rhs contains no elements only the \c lhs will be
      // returned when you invoke print() on InExprNode.
      class InExprNode : public ExprNode
      {
      public:
        // Construct an IN expression node.
        InExprNode(const std::string oper,
                   const Expr& lhs, const Collection<Expr>& rhs);

        virtual ~InExprNode();

        virtual void print(std::ostream& os) const;
 
        virtual Expr getConstraint() const;

     private:
 
        // \name The operation
        const std::string itsOperation;

        // \name The operands
        // @{
        const Expr             itsLeft;
        const Collection<Expr> itsRight;
        // @}

      };


      // This class represents a LIKE expression node. A LIKE expression is an
      // expression that takes one operator (LIKE or NOT LIKE) and two
      // operands. It is used to check whether the first operand has a pattern
      // match with the second operand.
      //
      // The user-supplied pattern can contain the wildcards "*" and "?",
      // where "*" means any number of characters, and "?" means exactly one
      // character. These wildcards will be translated to their
      // SQL-equivalents "%" and "_" respectively. 
      //
      // We will use the backslash character (\) as escape character in the
      // LIKE clause. As the backslash character needs to be escaped itself,
      // we always have to provide two backslash characters as the escape
      // token.
      //      
      // Hence we get the following substitution scheme:
      // \verbatim
      //   '*'   -->  '%'
      //   '?'   -->  '_'
      //   '\*'  -->  '*'
      //   '\?'  -->  '?'
      //   '_'   -->  '\\_'
      //   '%'   -->  '\\%'
      // \endverbatim
      class LikeExprNode : public ExprNode
      {
      public:
        LikeExprNode(const std::string& oper, const Expr& value, 
                     const std::string& pattern);

        virtual ~LikeExprNode();
        
        virtual void print(std::ostream& os) const;

        virtual Expr getConstraint() const;

     private:
 
        // \name The operation
        const std::string itsOperation;

        // \name The operands
        // @{
        const Expr        itsOperand;
        const std::string itsPattern;
        // @}

      };

      // @}

    } // namespace Query
    
  } // namespace PL
  
} // namespace LOFAR

#endif
