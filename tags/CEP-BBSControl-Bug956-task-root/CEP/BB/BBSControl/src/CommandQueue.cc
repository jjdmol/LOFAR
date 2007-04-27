//#  CommandQueue.cc: Command queue of the blackboard system.
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

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

//# Includes
#include <BBSControl/CommandQueue.h>
#include <BBSControl/CommandQueueTransactors.h>
#include <BBSControl/QueryBuilder/AddCommand.h>
#include <BBSControl/BBSSingleStep.h>
#include <BBSControl/BBSSolveStep.h>
#include <BBSControl/BBSStrategy.h>
#include <BBSControl/BBSStructs.h>
#include <BBSControl/Exceptions.h>
#include <APS/ParameterSet.h>
#include <APS/Exceptions.h>
#include <Common/StreamUtil.h>
#include <Common/LofarLogger.h>
#include <Common/lofar_typeinfo.h>

//# Now here's an ugly kludge: libpqxx defines four different top-level
//# exception classes. In order to avoid a lot of code duplication we clumped
//# together four catch blocks in order to catch all pqxx related exceptions.
//# A DatabaseException will be thrown, containing the original pqxx
//# exception class type and the description.
#if defined(CATCH_PQXX_AND_RETHROW)
# error CATCH_PQXX_AND_RETHROW is already defined and should not be redefined
#else
# define CATCH_PQXX_AND_RETHROW					\
  catch (pqxx::broken_connection& e) {				\
    THROW (DatabaseException, "pqxx::broken_connection:\n"	\
	   << e.what());					\
  }								\
  catch (pqxx::sql_error& e) {					\
    THROW (DatabaseException, "pqxx::sql_error:\n"		\
	   << "Query: " << e.query() << endl << e.what());	\
  }								\
  catch (pqxx::in_doubt_error& e) {				\
    THROW (DatabaseException, "pqxx::in_doubt_error:\n"		\
	   << e.what());					\
  }								\
  catch (pqxx::internal_error& e) {				\
    THROW (DatabaseException, "pqxx::internal_error:\n"		\
	   << e.what());					\
  }
#endif

namespace LOFAR
{
  using ACC::APS::ParameterSet;
  using ACC::APS::APSException;

  namespace BBS 
  {
    using LOFAR::operator<<;


    //##--------   P u b l i c   m e t h o d s   --------##//

    CommandQueue::CommandQueue(const string& dbname, const string& user,
			       const string& host, const string& port)
    {
      LOG_TRACE_LIFETIME(TRACE_LEVEL_COND, "");

      string opts("dbname=" + dbname  + " user=" + user + 
		  " host=" + host + " port=" + port);
    try {
	LOG_DEBUG_STR("Connecting to database using options: " << opts);
	itsConnection.reset(new pqxx::connection(opts));
    } CATCH_PQXX_AND_RETHROW;
    }


    void CommandQueue::setTimeOut(double sec)
    {
      timeval tv;
      tv.tv_sec  = static_cast<long>(sec);
      tv.tv_usec = static_cast<long>(round((sec - tv.tv_sec) * 1e6));
      if (tv.tv_usec > 999999) {
        tv.tv_sec  += 1;
        tv.tv_usec -= 1000000;
      } else if (tv.tv_usec < -999999) {
        tv.tv_sec  -= 1;
        tv.tv_usec += 1000000;
      }
      ASSERT(-1000000 < tv.tv_usec && tv.tv_usec < 1000000);
      itsTimeOut = tv;
    }


    void CommandQueue::addCommand(const Command& aCommand) const
    {
      LOG_TRACE_LIFETIME(TRACE_LEVEL_COND, "");

      // Create a query object that "knows" how to build a query for inserting
      // \a aCommand into the command queue.
      QueryBuilder::AddCommand query;

      // Let \a aCommand accept the visiting query object, which, as a result,
      // will build an insert query for the correct command type.
      aCommand.accept(query);

      // Execute the query.
      execQuery(query.getQuery());
    }


    const Command* CommandQueue::getNextCommand()
    {
      LOG_TRACE_LIFETIME(TRACE_LEVEL_COND, "");

      // Get the next command from the command queue. This call will only
      // return the command type.
      ParameterSet ps = 
        execQuery("SELECT * FROM blackboard.get_next_command()");

      // If command type is an empty string, then we've probably received an
      // empty result row; return a null pointer.
      string type = ps.getString("Type");
      if (type.empty()) return 0;

      // Retrieve the command parameters associated with the received command.
      ostringstream query;
      query << "SELECT * FROM blackboard.get_" << toLower(type) << "_args"
            << "(" << ps.getInt32("id") << ")";

      // Execute the query. Add the result to the ParameterSet \a ps.
      ps.adoptCollection(execQuery(query.str()));

      try {
        // If the ParameterSet \a ps contains a key "Name", then the command is
        // a BBSStep. In that case we need to prefix all keys with
        // "Step.<name>".  (Yeah, I know, it's kinda clunky).
        string name = ps.getString("Name");

        // Self-adopt is not supported on a ParameterSet; hence this detour.
        string buf;
        ps.writeBuffer(buf);
        ps.clear();
        ps.adoptBuffer(buf, "Step." + name + ".");
        LOG_DEBUG_STR(ps);

        // Create a new BBSStep and return it.
        return BBSStep::create(name, ps, 0);
      } 
      catch (APSException&) {
        // In the catch clause we handle all other commands. They can be
        // constructed using the CommandFactory and initialized using read().
        Command* command = CommandFactory::instance().create(type);
        ASSERTSTR(command, "Failed to create a `" << type << 
                  "' command object");
        LOG_DEBUG_STR(ps);
        command->read(ps);
        return command;
      }
    }


    void CommandQueue::setStrategy(const BBSStrategy& strategy) const
    {
      LOG_TRACE_LIFETIME(TRACE_LEVEL_COND, "");

      // Create a query object that "knows" how to build a query for inserting
      // \a aCommand into the command queue.
      QueryBuilder::AddCommand query;

      // Let \a aCommand accept the visiting query object, which, as a result,
      // will build an insert query for the correct command type.
      strategy.accept(query);

      // Execute the query. Throw exception if strategy was set already.
      if (!execQuery(query.getQuery()).getBool("result")) {
        THROW (CommandQueueException, "Strategy can only be set once");
      }
    }


    const BBSStrategy* CommandQueue::getStrategy() const
    {
      LOG_TRACE_LIFETIME(TRACE_LEVEL_COND, "");

      // Compose the query.
      ostringstream query;
      query << "SELECT * FROM blackboard.get_strategy()";

      // Execute the query. The result will be returned as a ParameterSet.
      ParameterSet ps = execQuery(query.str());

      // If DataSet is an empty string, then we've probably received an empty
      // result row; return a null pointer.
      if (ps.getString("DataSet").empty()) return 0;

      // Create a new BBSStrategy object.
      BBSStrategy* strategy = dynamic_cast<BBSStrategy*>
        (CommandFactory::instance().create("BBSStrategy"));

      // Initialize the BBSStrategy object with the parameters just retrieved.
      strategy->read(ps);

      // Return the new BBSStrategy object.
      return strategy;

//       // Nasty but (currently) unavoidable conversion...
//       // We could move DataSet (and ParmDB.*) to Strategy as well?
//       ParameterSet tmp;
//       for(ParameterSet::const_iterator it = ps.begin(); it != ps.end(); ++it)
//       {
//         if(it->first == "DataSet"
//           || it->first == "ParmDB.Instrument"
//           || it->first == "ParmDB.LocalSky"
//           || it->first == "ParmDB.History")
//         {
//             tmp.add(it->first, it->second);
//         }
//         else
//             tmp.add("Strategy." + it->first, it->second);
//       }

//       // Create a BBSStrategy object using the parameter set and return it.
//       return new BBSStrategy(tmp);
    }


    bool CommandQueue::isNewRun(bool isGlobalCtrl) const
    {
      LOG_TRACE_LIFETIME(TRACE_LEVEL_COND, "");

      // Compose the query.
      ostringstream query;
      query << "SELECT * FROM blackboard.is_new_run('"
        << (isGlobalCtrl ? "TRUE" : "FALSE")
        << "') AS result";

      // Execute the query and return the result.
      return execQuery(query.str()).getBool("result");
    }


    //##--------   P r i v a t e   m e t h o d s   --------##//

    ParameterSet CommandQueue::execQuery(const string& query) const
    {
      // Create a transactor object and execute the query. The result will be
      // stored, as a string in \a result.
      string result;
      try {
	itsConnection->perform(ExecQuery(query, result));
	LOG_TRACE_VAR_STR("Result of query: " << result);
      } CATCH_PQXX_AND_RETHROW;

      // Create an empty parameter set and add the result to it.
      ParameterSet ps;
      ps.adoptBuffer(result);

      // Return the ParameterSet
      return ps;
    }

  } // namespace BBS
  
} // namespace LOFAR

