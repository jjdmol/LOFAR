//#  SQLExprNode.cc: implementation of SQL-like epxression nodes.
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

#include <PL/Query/SQLExprNode.h>
#include <iostream>
#include <sstream>

using std::string;
using std::ostringstream;

namespace LOFAR
{
  namespace PL
  {
    namespace Query
    {

      BetweenExprNode::BetweenExprNode(const std::string& oper, 
                                       const Expr& value,
                                       const Expr& lower, const Expr& upper) :
        itsOperation(oper),
        itsValue(value),
        itsLower(lower), itsUpper(upper)
      {
      }

      BetweenExprNode::~BetweenExprNode()
      {
      }

      void BetweenExprNode::print(std::ostream& os)
      {
        os << "(" << itsValue << itsOperation 
           << itsLower << " AND " << itsUpper << ")";
      }

      
      InExprNode::InExprNode(const std::string oper,
                             const Expr& lhs, const Collection<Expr>& rhs) :
        itsOperation(oper),
        itsLeft(lhs), itsRight(rhs)
      {
      }

      InExprNode::~InExprNode()
      {
      }

      void InExprNode::print(std::ostream& os)
      {
        ostringstream oss;
        Collection<Expr>::const_iterator it;
        for (it = itsRight.begin(); it != itsRight.end(); ++it) {
          oss << *it  << ",";
        }
        // Convert oss to a string; strip last comma if there is one
        string s(oss.str());
        string::size_type idx;
        if ((idx = s.rfind(",")) != string::npos) {
          s.erase(idx);
        }
        os << "(" << itsLeft << itsOperation << "(" << s << "))";
      }


      LikeExprNode::LikeExprNode(const std::string& oper,
                                 const Expr& lhs, const Expr& rhs) :
        itsOperation(oper),
        itsLeft(lhs), itsRight(rhs)
      {
      }

      LikeExprNode::~LikeExprNode()
      {
      }

      void LikeExprNode::print(std::ostream& os)
      {
        // Scan the pattern expression for occurrences of "*" and "?".
        // We have to check whether these characters are escaped!
        std::string s;
        bool isEscaped = false;
//         for(string::size_type idx = 0; idx < itsRight.size(); ++idx) {
//           isEscaped = (itsRight[idx] == '\\');
//           if (isEscaped) {
            
//           }
//         }
      }

    } // namespace Query

  } // namespace PL

} // namespace LOFAR
