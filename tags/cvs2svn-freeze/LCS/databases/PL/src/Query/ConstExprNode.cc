//#  ConstExprNode.cc: one line description
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

#include <PL/Query/ConstExprNode.h>
#include <Common/lofar_iostream.h>

namespace LOFAR
{
  namespace PL
  {
    namespace Query
    {

      NullExprNode::NullExprNode() 
      {
      }

      NullExprNode::~NullExprNode()
      {
      }

      void NullExprNode::print(ostream&) const
      {
      }

      bool NullExprNode::isNull() const
      {
        return true;
      }


      IntExprNode::IntExprNode(int i) : 
        itsOperand(i)
      {
      }

      IntExprNode::~IntExprNode()
      {
      }
      
      void IntExprNode::print(ostream& os) const
      {
        os << itsOperand; 
      }

      DoubleExprNode::DoubleExprNode(double d) : 
        itsOperand(d)
      {
      }

      DoubleExprNode::~DoubleExprNode()
      {
      }
      
      void DoubleExprNode::print(ostream& os) const
      {
        os << itsOperand;
      }

      StringExprNode::StringExprNode(const string& s) : 
        itsOperand(s)
      {
      }

      StringExprNode::~StringExprNode()
      {
      }
      
      void StringExprNode::print(ostream& os) const
      {
        // If the string contains one or more single quote (') characters
        // we will need to escape them by duplicating them.
        string s(itsOperand);
        string::size_type idx(string::npos);
        while((idx = s.find("'",idx+1)) != string::npos) {
          s.insert(idx++,"'");
        }
        os << "'" << s << "'";
      }

    } // namespace Query

  } // namespace PL

} // namespace LOFAR
