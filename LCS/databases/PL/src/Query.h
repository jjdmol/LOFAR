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
 	itsSqlString(aString) 
      {}

      QueryObject(const char* const aString) : 
        itsSqlString(aString) 
      {}
      //@}

      // Constructor that takes a Query Expression.
      QueryObject(const Query::Expr& aExpr) : 
        itsQueryExpr(aExpr)
      {}

      // Return the composed query as an SQL string.
      // \todo In a future version, we will probably not store the query
      // as plain SQL; at least not to begin with. So, getSql() will then
      // have to generate (and cache) the SQL string based on the information
      // stored in this object.
      std::string getSql() const;

    private:
      Query::Expr itsQueryExpr;
      std::string itsSqlString;

    };

  } // namespace PL

} // namespace LOFAR

#endif
