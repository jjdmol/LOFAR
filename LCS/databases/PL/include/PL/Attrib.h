//#  Attrib.h: Global methods for converting attributes to table columns.
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

#ifndef LOFAR_PL_ATTRIB_H
#define LOFAR_PL_ATTRIB_H

// \file Attrib.h
// Global methods for converting attributes to table columns.

#include <lofar_config.h>

//# Includes
#include <PL/TPersistentObject.h>
#include <PL/Query/Expr.h>
#include <PL/Query/ColumnExprNode.h>
#include <string>

namespace LOFAR
{
  namespace PL
  {

    // \addtogroup PL
    // @{

    // Convert the field specifier \a nm into a column expression node that
    // represents the table name and column name plus zero or more join
    // expressions for the PersistentObject \a po.
    //
    // makeAttrib() is the actual workhorse. It will try to recursively match
    // \a nm against a map of known attributes for the PersistentObject \a
    // po. The recursion ends when there are no more subfields in the field
    // specifier \a nm (a subfield is delimited by a dot (".") or a double
    // colon ("::")).
    //
    // For example: suppose \a nm contains the string \c
    // "foo.bar". makeAttrib() will first try to match the whole string \c
    // "foo.bar" against the map of known attributes for the PersistentObject
    // \a po. If no match is found, it'll try to match the string \c "foo"
    // against the same map. If a match is found, it'll assume that this match
    // represents a class type specifier (a class type specifier is the
    // concatenation of the '@' character and the RTTI name of the associated
    // class). Next it'll try to match this class type specifier with one of
    // the entries in the list of owned POs for PersistentObject \a po. If a
    // match is found, then makeAttrib() is called with that match as its
    // first argument and the string \c "bar" (being the right-hand-side of
    // the split field specifier \a nm) as the second argument.
    //
    // \note The map of known attributes must be created manually as part of
    // the template specialization code that must be written for each
    // TPersistentObject<T>.
    //
    // \sa PersistentObject::attribMap(), TPersistentObject<T>::theirAttribMap
    Query::ColumnExprNode* makeAttrib(const PersistentObject& po, 
                                      const std::string& nm);

    // Convert the field specifier \a nm into a table name and column name
    // plus zero or more join expressions for the specified PersistentObject
    // \a po.
    Query::Expr attrib(const PersistentObject& po, const std::string& nm)
    {
      return makeAttrib(po, nm);
    }
    
    // Convert the field specifier \a nm into a table name and column name
    // plus zero or more join expressions for a TPersistentObject of type \a
    // T.
    template<typename T>
    Query::Expr attrib(const std::string& nm)
    {
      return makeAttrib(TPersistentObject<T>(), nm);
    }

    // @}

  } // namespace PL

} // namespace LOFAR

#endif
