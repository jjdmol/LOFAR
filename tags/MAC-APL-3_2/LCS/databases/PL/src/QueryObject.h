//#  QueryObject.h: A user-friendly interface for composing queries.
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

#ifndef LOFAR_PL_QUERYOBJECT_H
#define LOFAR_PL_QUERYOBJECT_H

// \file QueryObject.h
// A user-friendly interface for composing queries.

//# Includes
#include <PL/Query/Expr.h>
#include <Common/lofar_string.h>

namespace LOFAR
{
  namespace PL
  {
    // \addtogroup PL
    // @{

    //
    // The query class provides a user-friendly interface for composing
    // queries.
    //
    class QueryObject
    {
    public:
      // Default constructor. Creates an empty query object.
      QueryObject() {}

      // @{
      // Constructor that takes an SQL string. 
      // \todo Do we want to do some sanity checking on \c aString ?
      QueryObject(const string& aString) : 
 	itsSqlString(aString), itsUseString(true) 
      {}

      QueryObject(const char* const aString) : 
        itsSqlString(aString), itsUseString(true) 
      {}
      // @}

      // Constructor that takes a Query Expression.
      QueryObject(const Query::Expr& aExpr) : 
        itsQueryExpr(aExpr), itsUseString(false)
      {}

      // Return whether this object is empty. A QueryObject is considered
      // empty when either 
      // - \c itsSqlString is empty and \c itsUseString is true, or
      // - \c itsQueryExpr is empty and \c itsUseString is false.
      bool isEmpty() const;

      // Return this QueryObject as an SQL WHERE clause. Depending on the
      // value of \c itsUseString, this method will either return \c
      // itsSqlString, or \c itsQueryExpr as a string.
      string getSql() const;

    private:
      // The query stored as a query expression.
      Query::Expr itsQueryExpr;

      // The query stored as a string.
      string itsSqlString;

      // This flag indicates whether we should use \c itsQueryExpr or \c
      // itsSqlString in the getSql() method. It depends on how the
      // QueryObject was constructed; either using a \c string argument or
      // using a Query::Expr argument. If itsUseString is \c true, getSql()
      // will return \c itsSqlString; if itsUseString is \c false, getSql()
      // will return the \c itsQueryExpr as a string.
      bool itsUseString;

    };

    // @}

  } // namespace PL

} // namespace LOFAR

#endif
