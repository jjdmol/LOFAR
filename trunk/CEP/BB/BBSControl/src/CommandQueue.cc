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
#include <BBSControl/Command.h>
#include <BBSControl/CommandId.h>
#include <BBSControl/CommandQueue.h>
#include <BBSControl/CommandResult.h>
#include <BBSControl/LocalControlId.h>
#include <BBSControl/QueryBuilder/AddCommand.h>
#include <BBSControl/BBSStep.h>
#include <BBSControl/BBSStrategy.h>
#include <BBSControl/Exceptions.h>
#include <APS/ParameterSet.h>
#include <APS/Exceptions.h>
#include <Common/LofarLogger.h>

//# For getpid() and pid_t.
#include <unistd.h>

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


    namespace
    {
      timeval asTimeval(double d)
      {
        timeval tv;
        tv.tv_sec = static_cast<long>(d);
        tv.tv_usec = static_cast<long>(round((d-tv.tv_sec)*1e6));
        if (tv.tv_usec > 999999) {
          tv.tv_sec += 1;
          tv.tv_usec -= 1000000;
        }
        if (tv.tv_usec < -999999) {
          tv.tv_sec -= 1;
          tv.tv_usec += 1000000;
        }
        return tv;
      }
    }


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


    CommandId CommandQueue::addCommand(const Command& aCommand) const
    {
      LOG_TRACE_LIFETIME(TRACE_LEVEL_COND, "");

      // Create a query object that "knows" how to build a query for inserting
      // \a aCommand into the command queue.
      QueryBuilder::AddCommand query;

      // Let \a aCommand accept the visiting query object, which, as a result,
      // will build an insert query for the correct command type.
      aCommand.accept(query);

      // Execute the query.
      return execQuery(query.getQuery()).getUint32("result");
    }


    NextCommandType CommandQueue::getNextCommand() const
    {
      LOG_TRACE_LIFETIME(TRACE_LEVEL_COND, "");

      // Get the next command from the command queue. This call will only
      // return the command type.
      ostringstream query;
      query << "SELECT * FROM blackboard.get_next_command(" << getpid() << ")";

      // Note the use of query.str(""): this clears the ostringstream so it can
      // be reused later on.
      ParameterSet ps = execQuery(query.str());
      query.str("");

      // If command type is an empty string, then we've probably received an
      // empty result row; return a null pointer.
      string type = ps.getString("Type");
      if (type.empty()) return make_pair(shared_ptr<Command>(), CommandId());

      // Get the command-id.
      CommandId id = ps.getInt32("id");

      // Retrieve the command parameters associated with the received command.
      query << "SELECT * FROM blackboard.get_" << toLower(type) << "_args"
            << "(" << id << ")";

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

        // Create a new BBSStep and return it.
        return make_pair(BBSStep::create(name, ps, 0), id);
      } 
      catch (APSException&) {
        // In the catch clause we handle all other commands. They can be
        // constructed using the CommandFactory and initialized using read().
        shared_ptr<Command> command(CommandFactory::instance().create(type));
        ASSERTSTR(command, "Failed to create a `" << type << 
                  "' command object");
        command->read(ps);
        return make_pair(command, id);
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


    bool CommandQueue::setStrategyDone() const
    {
      LOG_TRACE_LIFETIME(TRACE_LEVEL_COND, "");

      // Compose the query.
      string query("SELECT * FROM blackboard.set_strategy_done() AS result");

      // Execute the query and return the result
      return execQuery(query).getBool("result");
    }


    shared_ptr<const BBSStrategy> CommandQueue::getStrategy() const
    {
      LOG_TRACE_LIFETIME(TRACE_LEVEL_COND, "");

      // Compose the query.
      ostringstream query;
      query << "SELECT * FROM blackboard.get_strategy()";

      // Execute the query. The result will be returned as a ParameterSet.
      ParameterSet ps = execQuery(query.str());

      // If DataSet is an empty string, then we've probably received an empty
      // result row; return a null pointer.
      if (ps.getString("DataSet").empty()) {
        return shared_ptr<const BBSStrategy>();
      }

      // Create a new BBSStrategy object.
      BBSStrategy* strategy = dynamic_cast<BBSStrategy*>
        (CommandFactory::instance().create("BBSStrategy"));

      // Initialize the BBSStrategy object with the parameters just retrieved.
      strategy->read(ps);

      // Wrap the new BBSStrategy object in a managed pointer and return it.
      return shared_ptr<const BBSStrategy>(strategy);
    }


    bool CommandQueue::addResult(const CommandId& commandId, 
                                 const CommandResult& result) const
    {
      LOG_TRACE_LIFETIME(TRACE_LEVEL_COND, "");

      // Compose the query.
      ostringstream query;
      query << "SELECT * FROM blackboard.add_result(" 
            << commandId.asInt() << "," 
            << getpid() << ","
            << result.asInt()    << ",'" 
            << result.message()  << "') AS result";

      // Execute the query and return the result
      return execQuery(query.str()).getBool("result");
    }


    ResultMapType CommandQueue::getNewResults() const
    {
      LOG_TRACE_LIFETIME(TRACE_LEVEL_COND, "");

      // Compose the query.
      string query("SELECT * FROM blackboard.get_new_results()");

      // Execute the query and fetch the result
      ParameterSet ps = execQuery(query, true);
      uint nRows = ps.getUint32("_nrows");
      LOG_TRACE_CALC_STR("Query returned " << nRows << " rows");

      // Create a result map.
      ResultMapType results;

      for (uint row = 0; row < nRows; ++row) {
        ostringstream prefix;
        prefix << "_row(" << row << ")."; 
        CommandId      cmdId (ps.getUint32(prefix.str() + "command_id"));
        LocalControlId ctrlId(ps.getString(prefix.str() + "node"),
                              ps.getInt32(prefix.str() + "pid"));
        CommandResult  cmdRes(ps.getUint32(prefix.str() + "result_code"),
                              ps.getString(prefix.str() + "message"));
        LOG_TRACE_CALC_STR("Row: " << row << " [" << cmdId << "],[[" 
                           << ctrlId << "],[" << cmdRes << "]]");
        // Looking up results[cmdId] each iteration may cause a performance
        // problem. However, most of the times nRows will be small (probably
        // just 1), so leave it for the time being.
        results[cmdId].push_back(ResultType(ctrlId, cmdRes));
      }
      // Return the result map.
      return results;
    }


    vector<ResultType> CommandQueue::getNewResults(const CommandId& id) const 
    {
      LOG_TRACE_LIFETIME(TRACE_LEVEL_COND, "");

      // Compose the query.
      ostringstream query;
      query << "SELECT * FROM blackboard.get_new_results(" << id << ")";

      // Execute the query and fetch the result
      ParameterSet ps = execQuery(query.str(), true);
      uint nRows = ps.getUint32("_nrows");
      LOG_TRACE_CALC_STR("Query returned " << nRows << " rows");

      // Create a results vector and reserve space.
      vector<ResultType> results;
      results.reserve(nRows);

      // Fill the vector with the new results.
      for (uint row = 0; row < nRows; ++row) {
        ostringstream prefix;
        prefix << "_row(" << row << ").";
        LocalControlId id(ps.getString(prefix.str() + "node"),
                          ps.getInt32(prefix.str() + "pid"));
        CommandResult res(ps.getUint32(prefix.str() + "result_code"),
                          ps.getString(prefix.str() + "message"));
        LOG_TRACE_CALC_STR("Row: " << row << "[" << id << "],[" << res << "]");
        results.push_back(ResultType(id,res));
      }

      // Return the new results.
      return results;
    }


    bool CommandQueue::isNewRun(bool isGlobalCtrl) const
    {
      LOG_TRACE_LIFETIME(TRACE_LEVEL_COND, "");

      // Compose the query.
      ostringstream query;
      query << "SELECT * FROM blackboard.is_new_run('"
            << (isGlobalCtrl ? "TRUE" : "FALSE") << "',"
            << getpid()
            << ") AS result";
            
      // Execute the query and return the result.
      return execQuery(query.str()).getBool("result");
    }
    

    bool CommandQueue::waitForTrigger(Trigger::Type type, double timeOut) const
    {
      LOG_TRACE_LIFETIME(TRACE_LEVEL_COND, "");

      LOG_TRACE_COND("Waiting for notification");
      uint notifs;
      if (timeOut < 0) {
        notifs = itsConnection->await_notification();
      } else {
        timeval tv = asTimeval(timeOut);
        notifs = itsConnection->await_notification(tv.tv_sec, tv.tv_usec);
      }

      if (notifs) {
        LOG_TRACE_COND("Received notification ...");
        if (Trigger::testAndClearFlags(type) == type) {
          LOG_TRACE_COND("... of the correct type");
          return true;
        }
      }
      return false;
    }

      
    //##--------   P r i v a t e   m e t h o d s   --------##//

    string CommandQueue::ExecQuery::emptyString;

    CommandQueue::ExecQuery::ExecQuery(const string& query) :
      pqxx::transactor<>("ExecQuery"),
      itsQuery(query),
      itsResult(emptyString)
    {
    }


    CommandQueue::ExecQuery::ExecQuery(const string& query, string& result) :
      pqxx::transactor<>("ExecQuery"),
      itsQuery(query),
      itsResult(result)
    {
    }


    void CommandQueue::ExecQuery::operator()(argument_type& transaction)
    {
      LOG_DEBUG_STR("Executing query : " << itsQuery);
      itsPQResult = transaction.exec(itsQuery);
    }


    void CommandQueue::ExecQuery::on_commit()
    {
      ostringstream oss;
      uint rows(itsPQResult.size());
      uint cols(itsPQResult.columns());

      oss << "_nrows = " << rows << endl;
      for (uint row = 0; row < rows; ++row) {
        for (uint col = 0; col < cols; ++col) {
          oss << "_row(" << row << ")."
              << itsPQResult[row][col].name() << " = "
              << itsPQResult[row][col].c_str() << endl;
        }
      }
      itsResult = oss.str();
    }


    CommandQueue::Trigger::Init::Init()
    {
      theirTypes[Command] = "insert_command";
      theirTypes[Result]  = "insert_result";
    }


    CommandQueue::Trigger::Trigger(const CommandQueue& queue, Type type) :
      pqxx::trigger(*queue.itsConnection,
                    theirTypes.find(type) == theirTypes.end() 
                    ? "" : theirTypes[type])
    {
      LOG_TRACE_LIFETIME(TRACE_LEVEL_COND, "");
      ASSERT(theirTypes.find(type) != theirTypes.end());
      LOG_DEBUG_STR("Created trigger of type: " << theirTypes[type]);
    }


    void CommandQueue::Trigger::operator()(int be_pid)
    {
      LOG_DEBUG_STR("Received notification: " << name() << 
                    "; pid = " << be_pid);
      TypeMap::const_iterator it;
      for (it = theirTypes.begin(); it != theirTypes.end(); ++it) {
        if (it->second == name()) {
          LOG_TRACE_COND_STR("Raising flag [" << 
                             it->first << "," << it->second << "]");
          raiseFlags(it->first);
          break;
        }
      }
    }


    ParameterSet CommandQueue::execQuery(const string& query,
                                         bool doAlwaysPrefix) const
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

      // If the result consists of only one row, and we do not always want to
      // prefix, then strip off the prefix "_row(0)". 
      if (ps.getUint32("_nrows") == 1 && !doAlwaysPrefix) {
        ps = ps.makeSubset("_row(0).");
      }
      
      // Return the ParameterSet
      return ps;
    }


    bool CommandQueue::registerTrigger(Trigger::Type type)
    {
      shared_ptr<Trigger> trigger(new Trigger(*this, type));
      return itsTriggers.insert(make_pair(type, trigger)).second;
    }
  

    bool CommandQueue::deregisterTrigger(Trigger::Type type)
    {
      return itsTriggers.erase(type);
    }


    //##--------   S t a t i c   d a t a m e m b e r s   --------##//

    double                         CommandQueue::theirDefaultTimeOut = 5.0;
    CommandQueue::Trigger::TypeMap CommandQueue::Trigger::theirTypes;
    CommandQueue::Trigger::Type    CommandQueue::Trigger::theirFlags;
    CommandQueue::Trigger::Init    CommandQueue::Trigger::theirInit;


  } // namespace BBS
  
} // namespace LOFAR

