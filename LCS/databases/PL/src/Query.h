//#  Query.h: one line description
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

#ifndef LOFAR_PL_QUERY_H
#define LOFAR_PL_QUERY_H

//# Includes
#include <PL/Query/Expr.h>

//# Forward Declarations

namespace LOFAR
{
  namespace PL
  {
    //
    // The query class provides a user-friendly interface for composing
    // queries.
    //
    // \todo Do we need the constructors that take a string argument? At the
    // moment they interfere with the constructor that takes an
    // expression. Furthermore, there's currently no link between the two
    // private data members \c itsQueryExpr and \c itsSqlString, although this
    // link should definitely be made! Probably \c itsSqlString should be
    // dependent on \c itsQueryExpr (or maybe \c itsSqlString should be
    // removed completely).
    //
    class QueryObject
    {
    public:
      // Default constructor. Creates an empty query object.
      QueryObject() {}

      //@{
      // Constructor that takes an SQL string. 
      // \todo Do we want to do some sanity checking on \c aString ?
      QueryObject(const std::string& aString) : 
 	itsSqlString(aString), itsUseString(true) 
      {}

      QueryObject(const char* const aString) : 
        itsSqlString(aString), itsUseString(true) 
      {}
      //@}

      // Constructor that takes a Query Expression.
      QueryObject(const Query::Expr& aExpr) : 
        itsQueryExpr(aExpr), itsUseString(false)
      {}

      // Return whether this object is empty. A QueryObject is considered
      // empty when either 
      // - \c itsSqlString is empty and \c itsUseString is true, or
      // - \c itsQueryExpr is empty and \c itsUseString is false.
      bool empty() const;

      // Return this QueryObject as an SQL WHERE clause. Depending on the
      // value of \c itsUseString, this method will either return \c
      // itsSqlString, or \c itsQueryExpr as a string.
      std::string getSql() const;

    private:
      // The query stored as a query expression.
      Query::Expr itsQueryExpr;

      // The query stored as a string.
      std::string itsSqlString;

      // This flag indicates whether we should use \c itsQueryExpr or \c
      // itsSqlString in the getSql() method. It depends on how the
      // QueryObject was constructed; either using a \c string argument or
      // using a Query::Expr argument. If itsUseString is \c true, getSql()
      // will return \c itsSqlString; if itsUseString is \c false, getSql()
      // will return the \c itsQueryExpr as a string.
      bool itsUseString;

    };

  } // namespace PL

} // namespace LOFAR

#endif
