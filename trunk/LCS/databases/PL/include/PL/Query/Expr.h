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

//# Includes
#include <lofar_config.h>
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
        // in through \a node and store it in a reference counted pointer.
        Expr(ExprNode* const node);

        // Print the expression into an output stream.
        // \attention This method will \e only print the expression itself, \e
        // not its associated constraint.
        void print(std::ostream& os) const;

        // Test whether this expression is a null expression.
        bool isNull() const;

        // Return the constraints associated with this expression.
        Expr getConstraint() const;

        // @name Unary operators
        //@{
        Expr operator+ () const;
        Expr operator- () const;
        Expr operator! () const;
        //@}

        // @name SQL-like operators
        //@{

        // The BETWEEN operator is used to test if a value is within a given
        // closed interval.
        Expr between(const Expr& lhs, const Expr& rhs) const;

        // The NOT BETWEEN operator is used to test if a value is not within a
        // given closed interval.
        // \see Expr::between()
        Expr notBetween(const Expr& lhs, const Expr& rhs) const;

        // The IN operator is used to test if an expression is contained in a
        // set of expressions.
        Expr in (const Collection<Expr>& set) const;

        // The NOT IN operator is used to test if an expression is not
        // contained in a set of expressions.
        // \see Expr::in()
        Expr notIn(const Collection<Expr>& set) const;

        // The LIKE operator is used to test if a value matches with the
        // pattern string in \a str. You can use the wildcard characters \c *
        // and \c ?, where \c * expands to zero or more characters, and \c ? 
        // expands to exactly one character. If you want to match a literal \c
        // * or \c ?, you should use the escape character \c \. Consequently,
        // if you want to match a literal \c \, you should escape it with
        // another \c \.
        //
        // \attention Remember that the backslash character \c \ is also used
        // as an escape character in the C/C++ language. Hence, if you want
        // the pattern string \a str to contain \c "\", then \a str should
        // contain \c "\\". Consequently, if you want the pattern string \a
        // str to expand to \c "\", then \a str should contain \c "\\\\".
        Expr like(const std::string& str) const;

        // The NOT LIKE operator is used to test if a value does not have a
        // match with a pattern expression.
        // \see Expr::like()
        Expr notLike(const std::string& str) const;

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
        // \note The logical binary operators behave different from the other
        // binary operators, because \a lhs and \a rhs are allowed to be null
        // expressions. If \a lhs is a null expression, then \a rhs is
        // returned; if \a rhs is a null expression, then \a lhs is returned.
        friend Expr operator&& (const Expr& lhs, const Expr& rhs);
        friend Expr operator|| (const Expr& lhs, const Expr& rhs);
        //@}

        // @name I/O stream operators
        //@{

        // Print the expression \a exp and the associated constraints onto the
        // output stream \a os.
        friend std::ostream& operator<< (std::ostream& os, const Expr& exp);
        //@}

        // The actual node of the expression query.
        boost::shared_ptr<ExprNode> itsNode;
      };

    } // namespace Query

  } // namespace PL

} // namespace LOFAR

#endif
