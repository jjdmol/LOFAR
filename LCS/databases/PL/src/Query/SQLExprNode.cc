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

      void BetweenExprNode::print(std::ostream& os) const
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

      void InExprNode::print(std::ostream& os) const
      {
        os << "(" << itsLeft;
        if (!itsRight.empty()) {
          ostringstream oss;
          Collection<Expr>::const_iterator it;
          for (it = itsRight.begin(); it != itsRight.end(); ++it) {
            oss << *it  << ",";
          }
          string s(oss.str());    // convert oss to a string
          s.erase(s.size()-1);    // strip trailing comma
          os << itsOperation << "(" << s << ")";
        }
        os << ")";
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

      void LikeExprNode::print(std::ostream& os) const
      {
        ostringstream oss;
        oss << itsRight;
        string rhs(oss.str());
        string pattern;

        // Scan the pattern expression in itsRight for occurrences of wildcard
        // characters and make the proper substitutions.
        for(string::const_iterator it = rhs.begin(); it != rhs.end(); ++it) {
          switch (*it) {
          case '*':
            pattern += "%";
            break;
          case '?':
            pattern += "_";
            break;
          case '%':
            pattern += "\\\\%";
            break;
          case '_':
            pattern += "\\\\_";
            break;
          case '\\':
            if (++it != rhs.end())
              if (*it == '\\') pattern += "\\\\\\\\";
              else pattern += *it;
            break;
          default:
            pattern += *it;
            break;
          }
        }

        os << "(" << itsLeft << itsOperation << pattern << " ESCAPE '\\\\')";
      }


    } // namespace Query

  } // namespace PL

} // namespace LOFAR
