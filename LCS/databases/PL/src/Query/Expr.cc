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
#include <iostream>

using std::ostream;

namespace LOFAR
{
  namespace PL
  {
    namespace Query
    {
      ///////////////////////////////////////////////////////////////////
      //                         Constructors                          //
      ///////////////////////////////////////////////////////////////////

      Expr::Expr()
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

      Expr::Expr(ExprNode* const node) : 
        itsNode(node)
      {
      }


      ///////////////////////////////////////////////////////////////////
      //                          Unary operators                      //
      ///////////////////////////////////////////////////////////////////

      Expr Expr::operator+ () const
      {
        return *this;
      }

      Expr Expr::operator- () const
      {
        return new UnaryExprNode("-", *this);
      }

      Expr Expr::operator! () const
      {
        return new UnaryExprNode("NOT", *this);
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
        return new BinaryExprNode("AND", lhs, rhs);
      }

      Expr operator|| (const Expr& lhs, const Expr& rhs)
      {
        return new BinaryExprNode("OR", lhs, rhs);
      }


      ///////////////////////////////////////////////////////////////////
      //                     I/O stream operators                      //
      ///////////////////////////////////////////////////////////////////

      std::ostream& operator<< (std::ostream& os, const Expr& exp)
      {
        exp.itsNode->print(os);
        return os;
      }

    } // namespace Query

  } // namespace PL

} // namespace LOFAR
