//#  ConstExprNode.h: one line description
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

#ifndef LOFAR_PL_QUERY_CONSTEXPRNODE_H
#define LOFAR_PL_QUERY_CONSTEXPRNODE_H

//# Includes
#include <lofar_config.h>
#include <PL/Query/ExprNode.h>
#include <string>

namespace LOFAR
{
  namespace PL
  {
    namespace Query
    {
      //# Forward Declarations

      // @defgroup ConstExprNode Constant Expression Nodes
      // @ingroup ExprNode
      //
      // These classes represent constant expression nodes. A constant
      // expression is an expression that can be evaluated compile-time and
      // has a primitive type like int, double or string.

      //@{

      class NullExprNode : public ExprNode
      {
      public:
        NullExprNode();
        virtual ~NullExprNode();
        virtual void print(std::ostream&) const;
      };
      
      // Expression node class for an int.
      class IntExprNode : public ExprNode
      {
      public:
        IntExprNode(int i);
        virtual ~IntExprNode();
        virtual void print(std::ostream& os) const;
      private:
        const int itsOperand;
      };

      // Expression node class for a double.
      class DoubleExprNode : public ExprNode
      {
      public:
        DoubleExprNode(double d);
        virtual ~DoubleExprNode();
        virtual void print(std::ostream& os) const;
      private:
        const double itsOperand;
      };
      
      // Expression node class for a string.
      class StringExprNode : public ExprNode
      {
      public:
        StringExprNode(const std::string& s);
        virtual ~StringExprNode();
        virtual void print(std::ostream& os) const;
      private:
        const std::string itsOperand;
      };

      //@}

    } // namespace Query

  } // namespace PL

} // namespace LOFAR

#endif
