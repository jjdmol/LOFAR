//#  Attrib.cc: Global methods for converting attributes to table columns.
//#
//#  Copyright (C) 2002-2004
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

#include <PL/TPersistentObject.h>
#include <PL/Query/ColumnExprNode.h>
#include <Common/Debug.h>
#include <iostream>

namespace LOFAR
{
  namespace PL
  {

    using std::string;
    using std::cout;
    using std::endl;

    // Try to resolve the argument \c nm into a PersistentObject::Pointer. A
    // class type specifier \e must start with a '@' character; this is the
    // way that the software can discriminate a class type specifier from a
    // table column name. When \c nm is identified as a valid class type
    // specifier, we will try to find a match by comparing it against the RTTI
    // names of in the list of owned POs. If a match is found, the pointer to
    // this PersistentObject is returned, else a null pointer is returned.
    PersistentObject::Pointer resolveClassType(const PersistentObject& po,
                                               const std::string& nm)
    {
      PersistentObject::Pointer p;
      if (nm.empty()) return p;
      AssertStr(nm[0] == '@', "Expected class type specifier in " << nm);
      string cls(nm.substr(1));
      cout << "Searching \"" << cls << "\" among ownedPOs ..." << endl;
      if (cls.empty()) return p;
      PersistentObject::POContainer::const_iterator it;
      for (it = po.ownedPOs().begin(); it != po.ownedPOs().end(); ++it) {
        cout << typeid(**it).name() << " : match?  ";
        if (typeid(**it).name() == cls) {
          p = *it;
          cout << "yes" << endl;
        }
        else cout << "no" << endl;
      }
      return p;
    }


    Query::ColumnExprNode* makeAttrib(const PersistentObject& po, 
                                      const std::string& nm)
    {
      const PersistentObject::attribmap_t& attrMap = po.attribMap();
      PersistentObject::attribmap_t::const_iterator it;
      string attr(nm);
      
      cout << "Searching for \"" << attr << "\"" << endl;
      cout << "Showing contents of attrMap: " << endl;
      for (it = attrMap.begin(); it != attrMap.end(); ++it) {
        cout << " " << it->first << " = " << it->second << endl;
      }

      // First let's try to find a match for the whole string attr. If the
      // whole string matches then the attribute attr we're looking for has a
      // table column representation in the table associated with the
      // PersistentObject po. Hence we don't need to generate a join
      // expression for the ColumnExprNode.
      if ((it = attrMap.find(attr)) != attrMap.end()) {
        cout << "Found a match:  " << attr << " --> " << it->second << endl;
        return new Query::ColumnExprNode(po.tableName(), it->second);
      }
      cout << "No match for \"" << attr << "\"" << endl;

      // Now we need to search for a match using only part of the string attr.
      // We will start at the end of the string and move towards the beginning
      // until the next dot character. We will try to find a match that
      // resolves to a class type. If we find one, we can call makeAttrib
      // using the matching class instance as first argument. The right-hand
      // side of the string is the attribute we will then look for. Hence,
      // that is the second argument to makeAttrib. An exception will be
      // thrown when the beginning of the string attr is reached and no match
      // is found.
      string::size_type idx = attr.size();
      while (idx-- > 0) {
        if ((idx = attr.rfind(".",idx)) == string::npos) break;
        string lhs = attr.substr(0,idx);
        string rhs = attr.substr(idx+1);
        if ((it = attrMap.find(lhs)) == attrMap.end()) continue;
        cout << "Trying to resolve class type for \"" << it->second << "\"" 
             << endl;
        PersistentObject::Pointer p = resolveClassType(po,it->second);
        if (p)
        {
          Query::Expr left(new Query::ColumnExprNode(po.tableName(),"OBJID"));
          Query::Expr right(new Query::ColumnExprNode(p->tableName(),"OWNER"));
          Query::Expr join(left==right);
          Query::ColumnExprNode* exprNode = makeAttrib(*p, rhs);
          exprNode->addConstraint(join);
          return exprNode;
        }
      }

      // Once we've reached this point, we didn't find a matching table/column
      // name associated with the attrib attr. We'll throw an exception to
      // notify our caller.
      THROW(QueryError, "Failed to map attribute \"" << attr 
            << "\" onto a database column name");

      return new Query::ColumnExprNode("","");
    }

  } // namespace PL

} // namespace LOFAR
