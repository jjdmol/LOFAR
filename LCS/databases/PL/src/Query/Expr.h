//#  Expr.h: Represents the WHERE clause of a query.
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

#ifndef LOFAR_PL_QUERY_EXPR_H
#define LOFAR_PL_QUERY_EXPR_H

#if defined(HAVE_CONFIG_H)
#include <config.h>
#endif

//# Includes
#include <PL/Query/ExprNode.h>
#include <boost/shared_ptr.hpp>
#include <iosfwd>
#include <string>

namespace LOFAR
{
  namespace PL
  {
    template<typename T> class Collection;

    namespace Query
    {

      // This class represents the WHERE clause of a query. 
      class Expr
      {
      public:

        // @name Constructors for literal types.
        //@{
        Expr();
        Expr(int value);
        Expr(double value);
        Expr(const std::string& value);
        Expr(const char* const value);
        //@}

        // Construct an Expr from an ExprNode pointer.
        // @attention Expr will take ownership of the pointer that was passed
        // in through \p node and store it in a reference counted pointer.
        Expr(ExprNode* const node);

        // @name Unary operators
        //@{
        Expr operator+ () const;
        Expr operator- () const;
        Expr operator! () const;
        //@}

        //@{
        // The BETWEEN operator is used to test if a value is within an
        // interval.
        Expr between(const Expr& lhs, const Expr& rhs) const;
        Expr notBetween(const Expr& lhs, const Expr& rhs) const;
        //@}

        //@{
        // The IN operator is used to test if an expression is contained in a
        // set of expressions.
        Expr in (const Collection<Expr>& set) const;
        Expr notIn(const Collection<Expr>& set) const;
        //@}

        //@{
        // The LIKE operator is used to test if a value has a match with a
        // pattern expression.
        Expr like(const Expr& exp) const;
        Expr notLike(const Expr& exp) const;
        //@}

      private:

        // @name Arithmetic operators
        //@{
        friend Expr operator+ (const Expr& lhs, const Expr& rhs);
        friend Expr operator- (const Expr& lhs, const Expr& rhs);
        friend Expr operator* (const Expr& lhs, const Expr& rhs);
        friend Expr operator/ (const Expr& lhs, const Expr& rhs);
        //@}
        
        // @name Comparison operators
        //@{
        friend Expr operator== (const Expr& lhs, const Expr& rhs);
        friend Expr operator!= (const Expr& lhs, const Expr& rhs);
        friend Expr operator>= (const Expr& lhs, const Expr& rhs);
        friend Expr operator>  (const Expr& lhs, const Expr& rhs);
        friend Expr operator<= (const Expr& lhs, const Expr& rhs);
        friend Expr operator<  (const Expr& lhs, const Expr& rhs);
        //@}

        // @name Logical operators
        //@{
        friend Expr operator&& (const Expr& lhs, const Expr& rhs);
        friend Expr operator|| (const Expr& lhs, const Expr& rhs);
        //@}

        // I/O stream operators
        friend std::ostream& operator<< (std::ostream& os, const Expr& exp);

        // The actual node of the expression query.
        boost::shared_ptr<ExprNode> itsNode;
      };

    } // namespace Query

  } // namespace PL

} // namespace LOFAR

#endif
