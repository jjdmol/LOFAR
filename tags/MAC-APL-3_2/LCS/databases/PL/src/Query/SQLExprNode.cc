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
#include <PL/Exception.h>
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
        if (value.isNull() || lower.isNull() || upper.isNull())
          THROW(QueryError, "Null expression argument is not allowed");
      }

      BetweenExprNode::~BetweenExprNode()
      {
      }

      void BetweenExprNode::print(std::ostream& os) const
      {
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


      InExprNode::InExprNode(const std::string oper,
                             const Expr& lhs, const Collection<Expr>& rhs) :
        itsOperation(oper),
        itsLeft(lhs), itsRight(rhs)
      {
        // Check if \a rhs contains only null expressions; this is not allowed.
        Collection<Expr>::const_iterator it;
        bool onlyNulls = true;
        for(it = rhs.begin(); it != rhs.end() && onlyNulls; ++it) {
          onlyNulls = it->isNull();
        }
        if (lhs.isNull() || onlyNulls) 
          THROW(QueryError, "Null expression argument is not allowed");
      }

      InExprNode::~InExprNode()
      {
      }

      void InExprNode::print(std::ostream& os) const
      {
        itsLeft.print(os);
        ostringstream oss;
        Collection<Expr>::const_iterator it;
        for(it = itsRight.begin(); it != itsRight.end(); ++it) {
          if (!it->isNull()) {
            it->print(oss);
            oss << ",";
          }
        }
        string s(oss.str());                    // convert oss to a string
        if (!s.empty()) s.erase(s.size()-1);    // strip trailing comma
        os << itsOperation << "(" << s << ")";
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


      LikeExprNode::LikeExprNode(const std::string& oper, const Expr& value, 
                                 const std::string& pattern) :
        itsOperation(oper),
        itsOperand(value), itsPattern(pattern)
      {
        if (value.isNull())
          THROW(QueryError, "Null expression argument is not allowed");
      }

      LikeExprNode::~LikeExprNode()
      {
      }

      void LikeExprNode::print(std::ostream& os) const
      {
        // This string will contain the output pattern in SQL format.
        string pattern;

        // Scan the pattern expression in \c itsPattern for occurrences of
        // wildcard characters and make the proper substitutions.
        for(string::const_iterator it = itsPattern.begin(); 
            it != itsPattern.end(); ++it) {
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
            if (++it != itsPattern.end())
              if (*it == '\\') pattern += "\\\\\\\\";
              else pattern += *it;
            break;
          default:
            pattern += *it;
            break;
          }
        }

        itsOperand.print(os);
        // We need to convert \a pattern to an Expr object here, because the
        // Expr object will take care of converting the C/C++-style string to
        // a SQL-style string.
        os << itsOperation << Expr(pattern) << " ESCAPE '\\\\'";
      }


      Expr LikeExprNode::getConstraint() const
      {
        return itsOperand.getConstraint();
      }


    } // namespace Query

  } // namespace PL

} // namespace LOFAR
