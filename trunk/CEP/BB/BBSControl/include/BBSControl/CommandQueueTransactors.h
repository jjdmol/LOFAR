//#  CommandQueueTransactors.h: Transaction functors for the BBS command queue
//#
//#  Copyright (C) 2002-2007
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

#ifndef LOFAR_BBSCONTROL_COMMANDQUEUETRANSACTORS_H
#define LOFAR_BBSCONTROL_COMMANDQUEUETRANSACTORS_H

//# Never #include <config.h> or #include <lofar_config.h> in a header file!

// \file
// Transaction functors for the BBS command queue

//# Includes
#if defined(HAVE_PQXX)
# include <pqxx/transactor>
# include <pqxx/result>
#else
# error libpqxx, the C++ API to PostgreSQL, is required
#endif

#include <Common/lofar_string.h>
#include <Common/lofar_iosfwd.h>

namespace LOFAR
{
  namespace BBS
  {
    //# Forward Declarations
    class BBSSingleStep;

    // \addtogroup BBSControl
    // @{

    // Functor class for executing a query. 
    class ExecQuery : public pqxx::transactor<>
    {
    public:
      // Constructor for insert/update-like queries. These queries do not
      // return a result.
      explicit ExecQuery(const string& query);

      // Constructor for select-like queries. The result is returned as a
      // string.
      ExecQuery(const string& query, string& result);

      // This method will be invoked by the perform() method of your
      // pqxx::connection class to execute the query stored in itsQuery. The
      // result, if any, will be stored in itsPQResult.
      void operator()(argument_type& transaction);

      // This method will be invoked by the perform() method of your
      // pqxx::connection class, when the transaction succeeded. The result of
      // the query, stored in itsPQResult, will be converted to a string and
      // assigned to itsResult.
      // 
      // The result string will be formatted as a collection of key/value
      // pairs, where each key is uniquely defined as
      // "_row(<row-number>).<column-name>", i.e. "_row(0)." for the first
      // row, "_row(1)." for the second, etc. The key "_nrows" will contain
      // the number of rows in the result.
      void on_commit();

    private:
      // Empty string, used to initialize itsResult properly, when the
      // one-argument constructor is used.
      static string emptyString;

      // String containing the query to be executed.
      const string itsQuery;

      // Reference to the string that will hold the query result.
      string& itsResult;

      // The result of the executed query must be stored internally, because
      // it will be written in operator() and will be read in on_commit().
      pqxx::result itsPQResult;
    };

    // @}

  } // namespace BBS

} // namespace LOFAR

#endif
