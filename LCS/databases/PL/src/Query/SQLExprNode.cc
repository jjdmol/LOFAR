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
        if (isNull()) return;
        os << "(";
        itsValue.print(os);
        os << itsOperation;
        itsLower.print(os);
        os << " AND ";
        itsUpper.print(os);
        os << ")";
      }

      Expr BetweenExprNode::getConstraint() const
      {
        return 
          itsValue.getConstraint() && 
          itsLower.getConstraint() && 
          itsUpper.getConstraint();
      }

      bool BetweenExprNode::isNull() const
      {
        return itsValue.isNull() || itsLower.isNull() || itsUpper.isNull();
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
        if (isNull()) return;
        itsLeft.print(os);
        if (!itsRight.empty()) {
          ostringstream oss;
          Collection<Expr>::const_iterator it;
          for (it = itsRight.begin(); it != itsRight.end(); ++it) {
            if (!it->isNull()) {
              it->print(oss);
              oss << ",";
            }
          }
          string s(oss.str());    // convert oss to a string
          s.erase(s.size()-1);    // strip trailing comma
          os << itsOperation << "(" << s << ")";
        }
      }

      Expr InExprNode::getConstraint() const
      {
        Expr expr(itsLeft.getConstraint());
        Collection<Expr>::const_iterator it;
        for(it = itsRight.begin(); it != itsRight.end(); ++it) {
          expr = expr && it->getConstraint();
        }
        return expr;
      }

      bool InExprNode::isNull() const
      {
        return itsLeft.isNull() || itsRight.empty();
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
        if (isNull()) return;

        // We must print the pattern expression in itsRight into an
        // ostringstream, because we need its contents as a string.
        ostringstream oss;
        itsRight.print(oss);
        string rhs(oss.str());

        // This string will contain the output pattern in SQL format.
        string pattern;

        // Scan the pattern expression in itsRight (represented as a string by
        // rhs) for occurrences of wildcard characters and make the proper
        // substitutions.
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

        itsLeft.print(os);
        os << itsOperation << pattern << " ESCAPE '\\\\'";
      }


      Expr LikeExprNode::getConstraint() const
      {
        return itsLeft.getConstraint() && itsRight.getConstraint();
      }

      bool LikeExprNode::isNull() const
      {
        return itsLeft.isNull();
      }


    } // namespace Query

  } // namespace PL

} // namespace LOFAR
