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
#include <Common/LofarLogger.h>
#include <Common/lofar_iostream.h>

namespace LOFAR
{
  namespace PL
  {

    // Try to resolve the argument \a nm into a PersistentObject::Pointer. A
    // class type specifier \e must start with a '@' character; this is the
    // way that the software can discriminate a class type specifier from a
    // table column name. When \a nm is identified as a valid class type
    // specifier, we will try to find a match by comparing it against the RTTI
    // names of in the list of owned POs. If a match is found, the pointer to
    // this PersistentObject is returned, else a null pointer is returned.
    PersistentObject::Pointer resolveClassType(const PersistentObject& po,
                                               const string& nm)
    {
      PersistentObject::Pointer p;
      if (nm.empty()) return p;
      ASSERTSTR (nm[0] == '@', "Expected class type specifier in " << nm);
      string cls(nm.substr(1));
      LOG_TRACE_COND_STR ("Searching \"" << cls << "\" among ownedPOs ...");
      if (cls.empty()) return p;
      PersistentObject::POContainer::const_iterator it;
      for (it = po.ownedPOs().begin(); it != po.ownedPOs().end(); ++it) {
        LOG_TRACE_CALC_STR (typeid(**it).name() << " : match?  ");
        if (typeid(**it).name() == cls) {
          p = *it;
          LOG_TRACE_CALC_STR (" yes");
          break;
        }
        else LOG_TRACE_CALC_STR (" no");
      }
      return p;
    }


    Query::ColumnExprNode* makeAttrib(const PersistentObject& po, 
                                      const string& nm)
    {
      const PersistentObject::attribmap_t& attrMap = po.attribMap();
      PersistentObject::attribmap_t::const_iterator it;
      
      LOG_TRACE_COND_STR ("Searching for \"" << nm << "\"");
      LOG_TRACE_CALC_STR ("Showing contents of attrMap: ");
      for (it = attrMap.begin(); it != attrMap.end(); ++it) {
        LOG_TRACE_CALC_STR (" " << it->first << " = " << it->second);
      }

      // Let's do some sanity checks before we start to parse the string: it
      // is a syntax error if \a nm is an empty string, or if the first or
      // last character of \a nm is a dot (".") or a colon (":").
      if (nm.empty() || 
          nm[0] == '.' || nm[nm.size()-1] == '.' ||
          nm[0] == ':' || nm[nm.size()-1] == ':')
        THROW(QueryError, "Syntax error in specification of attribute \"" 
              << nm << "\"");
      

      // First let's try to find a match for the whole string \a nm. If the
      // whole string matches then the attribute \a nm we're looking for has a
      // table column representation in the table associated with the
      // PersistentObject po. Hence we don't need to generate a join
      // expression for the ColumnExprNode.
      if ((it = attrMap.find(nm)) != attrMap.end()) {
        LOG_TRACE_CALC_STR ("Found a match:  " << nm << " --> " << it->second);
        return new Query::ColumnExprNode(po.tableName(), it->second);
      }
      LOG_TRACE_CALC_STR ("No match for \"" << nm << "\"");

      // Now we need to search for a match using only part of the string \a
      // nm.  We will start at the end of the string and move towards the
      // beginning searching for either a dot (.) or a double colon (::). We
      // will try to find a match that resolves to a class type. If we find
      // one, we can call makeAttrib using the matching class instance as
      // first argument. The right-hand side of the string is the attribute we
      // will then look for. Hence, that is the second argument to
      // makeAttrib. An exception will be thrown when the beginning of the
      // string \a nm is reached and no match is found.
      string::size_type idx = nm.size();
      string lhs, rhs;
      while (idx-- > 0) {
        if ((idx = nm.find_last_of(":.",idx)) == string::npos) break;
        if (nm[idx] == '.') {
          if (nm[idx-1] == '.' || nm[idx-1] == ':')
            THROW(QueryError, "Syntax error in specification of attribute \"" 
                  << nm << "\"");
          lhs = nm.substr(0,idx);
          rhs = nm.substr(idx+1);
        }
        else {
          idx--;
          // Colons should appear in pairs, otherwise it's a synatx error.
          if (nm[idx] != ':' || nm[idx-1] == ':' || nm[idx-1] == '.')
            THROW(QueryError, "Syntax error in specification of attribute \"" 
                  << nm << "\"");
          lhs = nm.substr(0,idx+2);
          rhs = nm.substr(idx+2);
        }
        if ((it = attrMap.find(lhs)) == attrMap.end()) {
          LOG_TRACE_CALC_STR ("No match for \"" << lhs << "\"");
          continue;
        }
        LOG_TRACE_CALC_STR ("Found a match: " << lhs << " --> " << it->second);
        LOG_TRACE_COND_STR ("Trying to resolve class type for \"" << it->second << "\"");
        PersistentObject::Pointer p = resolveClassType(po,it->second);
        if (p)
        {
          Query::Expr left(new Query::ColumnExprNode(po.tableName(),"OBJID"));
          Query::Expr right(new Query::ColumnExprNode(p->tableName(),"OWNER"));
          Query::Expr join(left==right);
          LOG_TRACE_COND_STR ("About to call makeAttrib using \"" << rhs 
                   << "\" as second argument");
          Query::ColumnExprNode* exprNode = makeAttrib(*p, rhs);
          exprNode->addConstraint(join);
          return exprNode;
        }
      }

      // Once we've reached this point, we didn't find a matching table/column
      // name associated with the attrib \a nm. We'll throw an exception to
      // notify our caller.
      THROW(QueryError, "Failed to map attribute \"" << nm 
            << "\" onto a database column name");

    }

  } // namespace PL

} // namespace LOFAR
