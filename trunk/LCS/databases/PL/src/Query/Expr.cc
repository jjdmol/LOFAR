//#  Expr.cc: Represents the WHERE clause of a query.
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

#include <PL/Query/Expr.h>
#include <PL/Query/ConstExprNode.h>
#include <PL/Query/UnaryExprNode.h>
#include <PL/Query/BinaryExprNode.h>
#include <PL/Query/SQLExprNode.h>
#include <iostream>

namespace LOFAR
{
  namespace PL
  {
    namespace Query
    {

      ///////////////////////////////////////////////////////////////////
      //                         Constructors                          //
      ///////////////////////////////////////////////////////////////////

      Expr::Expr() :
        itsNode(new NullExprNode())
      {
      }
      
      Expr::Expr(int value) : 
        itsNode(new IntExprNode(value))
      {
      }
      
      Expr::Expr(double value) : 
        itsNode(new DoubleExprNode(value))
      {
      }

      Expr::Expr(const std::string& value) : 
        itsNode(new StringExprNode(value))
      {
      }
      
      Expr::Expr(const char* const value) :
        itsNode(new StringExprNode(value))
      {
      }

      Expr::Expr(ExprNode* const node) : 
        itsNode(node)
      {
      }


      ///////////////////////////////////////////////////////////////////
      //                          Public methods                       //
      ///////////////////////////////////////////////////////////////////

      void Expr::print(std::ostream& os) const
      {
        itsNode->print(os);
      }

      bool Expr::isNull() const
      {
        return itsNode->isNull();
      }

      Expr Expr::getConstraint() const
      {
        return itsNode->getConstraint();
      }


      ///////////////////////////////////////////////////////////////////
      //                          Unary operators                      //
      ///////////////////////////////////////////////////////////////////

      Expr Expr::operator+ () const
      {
        return new UnaryExprNode("+", *this);
      }

      Expr Expr::operator- () const
      {
        return new UnaryExprNode("-", *this);
      }

      Expr Expr::operator! () const
      {
        return new UnaryExprNode("NOT ", *this);
      }


      ///////////////////////////////////////////////////////////////////
      //                       SQL-like operators                      //
      ///////////////////////////////////////////////////////////////////

      Expr Expr::between (const Expr& lhs, const Expr& rhs) const
      {
        return new BetweenExprNode(" BETWEEN ", *this, lhs, rhs);
      }

      Expr Expr::notBetween (const Expr& lhs, const Expr& rhs) const
      {
        return new BetweenExprNode(" NOT BETWEEN ", *this, lhs, rhs);
      }

      Expr Expr::in (const Collection<Expr>& set) const
      {
        return new InExprNode(" IN ", *this, set);
      }

      Expr Expr::notIn (const Collection<Expr>& set) const
      {
        return new InExprNode(" NOT IN ", *this, set);
      }

      Expr Expr::like (const std::string& str) const
      {
        return new LikeExprNode(" LIKE ", *this, str);
      }

      Expr Expr::notLike (const std::string& str) const
      {
        return new LikeExprNode(" NOT LIKE ", *this, str);
      }

      
      ///////////////////////////////////////////////////////////////////
      //                     Arithmetic operators                      //
      ///////////////////////////////////////////////////////////////////
 
      Expr operator+ (const Expr& lhs, const Expr& rhs)
      {
        return new BinaryExprNode("+", lhs, rhs);
      }

      Expr operator- (const Expr& lhs, const Expr& rhs)
      {
        return new BinaryExprNode("-", lhs, rhs);
      }

      Expr operator* (const Expr& lhs, const Expr& rhs)
      {
        return new BinaryExprNode("*", lhs, rhs);
      }

      Expr operator/ (const Expr& lhs, const Expr& rhs)
      {
        return new BinaryExprNode("/", lhs, rhs);
      }

      
      ///////////////////////////////////////////////////////////////////
      //                     Comparison operators                      //
      ///////////////////////////////////////////////////////////////////

      Expr operator== (const Expr& lhs, const Expr& rhs)
      {
        return new BinaryExprNode("=", lhs, rhs);
      }

      Expr operator!= (const Expr& lhs, const Expr& rhs)
      {
        return new BinaryExprNode("<>", lhs, rhs);
      }

      Expr operator>= (const Expr& lhs, const Expr& rhs)
      {
        return new BinaryExprNode(">=", lhs, rhs);
      }

      Expr operator>  (const Expr& lhs, const Expr& rhs)
      {
        return new BinaryExprNode(">", lhs, rhs);
      }

      Expr operator<= (const Expr& lhs, const Expr& rhs)
      {
        return new BinaryExprNode("<=", lhs, rhs);
      }

      Expr operator<  (const Expr& lhs, const Expr& rhs)
      {
        return new BinaryExprNode("<", lhs, rhs);
      }


      ///////////////////////////////////////////////////////////////////
      //                        Logical operators                      //
      ///////////////////////////////////////////////////////////////////

      Expr operator&& (const Expr& lhs, const Expr& rhs)
      {
        if (lhs.isNull()) return rhs;
        if (rhs.isNull()) return lhs;
        return new BinaryExprNode(" AND ", lhs, rhs);
      }

      Expr operator|| (const Expr& lhs, const Expr& rhs)
      {
        if (lhs.isNull()) return rhs;
        if (rhs.isNull()) return lhs;
        return new BinaryExprNode(" OR ", lhs, rhs);
      }


      ///////////////////////////////////////////////////////////////////
      //                     I/O stream operators                      //
      ///////////////////////////////////////////////////////////////////

      std::ostream& operator<< (std::ostream& os, const Expr& exp)
      {
        exp.print(os);
        Expr cs(exp.getConstraint());
        if (!cs.isNull()) {
          os << " AND ";
          cs.print(os);
        }
        return os;
      }


    } // namespace Query

  } // namespace PL

} // namespace LOFAR
