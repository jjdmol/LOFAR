//#  UnaryExprNode.cc: one line description
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

#include <PL/Query/UnaryExprNode.h>
#include <iostream>

namespace LOFAR
{
  namespace PL
  {
    namespace Query
    {
      UnaryExprNode::UnaryExprNode(const std::string& oper, 
                                   const Expr& value) : 
        itsOperation(oper), 
        itsOperand(value)
      {
      }

      UnaryExprNode::~UnaryExprNode()
      {
      }

      void UnaryExprNode::print(std::ostream& os) const
      {
        if (itsOperand.isNull()) return;
//         os << "(";
        os << itsOperation;
        itsOperand.print(os);
//         os << ")";
      }

      Expr UnaryExprNode::getConstraint() const
      {
        return itsOperand.getConstraint();
      }

    } // namespace Query

  } // namespace PL

} // namespace LOFAR
