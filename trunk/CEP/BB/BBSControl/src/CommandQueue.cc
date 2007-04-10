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
#include <BBSControl/BBSSingleStep.h>
#include <BBSControl/BBSSolveStep.h>
#include <BBSControl/BBSStrategy.h>
#include <BBSControl/BBSStructs.h>
#include <BBSControl/Exceptions.h>
#include <APS/ParameterSet.h>
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

  namespace BBS 
  {
    using LOFAR::operator<<;


    //##--------   P u b l i c   m e t h o d s   --------##//

    CommandQueue::CommandQueue(const string& dbname, const string& user,
			       const string& host, const string& port) :
      itsCurrentId(0)
    {
      LOG_TRACE_LIFETIME(TRACE_LEVEL_COND, "");

      string opts("dbname=" + dbname  + " user=" + user + 
		  " host=" + host + " port=" + port);
    try {
	LOG_DEBUG_STR("Connecting to database using options: " << opts);
	itsConnection.reset(new pqxx::connection(opts));
    } CATCH_PQXX_AND_RETHROW;
    }


    int CommandQueue::addStep(const BBSStep& aStep) const
    {
      LOG_TRACE_LIFETIME(TRACE_LEVEL_COND, "");
      ostringstream query;

      try {
	// The argument \a aStep must be convertible to a BBSSingleStep
	const BBSSingleStep& step = dynamic_cast<const BBSSingleStep&>(aStep);

	// First build the SELECT part of the query.
	query << "SELECT * FROM blackboard.add_" 
	      << toLower(step.operation()) 
	      << "_step";

	// The first 8 arguments are the same for all stored procedures.
	query << "('" << step.getName()               << "'"
	      << ",'" << step.baselines().station1    << "'"
	      << ",'" << step.baselines().station2    << "'"
	      << ",'" << step.correlation().selection << "'"
	      << ",'" << step.correlation().type      << "'"
	      << ",'" << step.sources()               << "'"
	      << ",'" << step.instrumentModels()      << "'"
	      << ",'" << step.outputData()            << "'";

	// The stored procedure for a BBSSolveStep needs more arguments.
	try {
	  const BBSSolveStep& solveStep = 
	    dynamic_cast<const BBSSolveStep&>(step);
	  query << ",'" << solveStep.maxIter()                  << "'"
		<< ",'" << solveStep.epsilon()                  << "'"
		<< ",'" << solveStep.minConverged()             << "'"
		<< ",'" << solveStep.parms()                    << "'"
		<< ",'" << solveStep.exclParms()                << "'"
		<< ",'" << solveStep.domainSize().bandWidth     << "'"
		<< ",'" << solveStep.domainSize().timeInterval  << "'";
	} catch (bad_cast&) {}
	
	// Finalize the query.
	query << ") AS command_id";
	
	// Execute the query.
	ParameterSet ps = execQuery(query.str());

	// Return the ID of the step that we've just inserted.
	return ps.getInt32("command_id");

      } catch (bad_cast&) {
	THROW (CommandQueueException, 
	       "Step `" << aStep.getName() << "' is not a BBSSingleStep");
      }
    }


    const BBSStep* CommandQueue::getNextStep()
    {
      LOG_TRACE_LIFETIME(TRACE_LEVEL_COND, "");

      // Compose the query.
      ostringstream query;
      query << "SELECT * FROM blackboard.get_next_step"
	    << "(" << itsCurrentId << ")";

      // Execute the query. The result will be returned as a ParameterSet.
      ParameterSet ps = execQuery(query.str());

      // If name is an empty string, then we've probably received an empty
      // result row; return a null pointer.
      string name = ps.getString("Name");
      if (name.empty()) return 0;
      
      // Set the current command_id to the new one in the parameter set.
      itsCurrentId = ps.getUint32("command_id");

      // If next step is a "solve" step, we must retrieve extra arguments.
      if (toUpper(ps.getString("Operation")) == "SOLVE") {

	// Compose the query
	query.str("");    // clear the stringstream buffer.
	query << "SELECT * FROM blackboard.get_solve_arguments"
	      << "(" << itsCurrentId << ")";

	// Execute the query; add the result to the ParameterSet \a ps.
	ps.adoptCollection(execQuery(query.str()), "Solve.");
      }

      // Create a prefix "Step.<Name>." for the keys in the parameter set.
      string prefix = "Step." + name + ".";
      
      // Add prefix to all the keys in the ParameterSet. We need the detour
      // with a string buffer, because ParameterSet cannot adopt itself.
      string buf;
      ps.writeBuffer(buf);
      ps.clear();
      ps.adoptBuffer(buf, prefix);

      // Create a BBSStep from the parameter set and return it.
      return BBSStep::create(name, ps, 0);
    }


    void CommandQueue::setStrategy(const BBSStrategy& strategy) const
    {
      LOG_TRACE_LIFETIME(TRACE_LEVEL_COND, "");

      // Compose the query.
      ostringstream query;
      query << "SELECT * FROM blackboard.set_strategy" 
	    << "('" << strategy.dataSet()                    << "'"
	    << ",'" << strategy.parmDB().localSky            << "'"
	    << ",'" << strategy.parmDB().instrument          << "'"
	    << ",'" << strategy.parmDB().history             << "'"
	    << ",'" << strategy.stations()                   << "'"
	    << ",'" << strategy.inputData()                  << "'"
	    << ",'" << strategy.regionOfInterest().frequency << "'"
	    << ",'" << strategy.regionOfInterest().time      << "'"
	    << ",'" << strategy.domainSize().bandWidth       << "'"
	    << ",'" << strategy.domainSize().timeInterval    << "'"
	    << ",'" << strategy.correlation().selection      << "'"
	    << ",'" << strategy.correlation().type           << "')"
	    << " AS result";

      // Execute the query.
      if (!execQuery(query.str()).getBool("result")) {
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

      // Nasty but (currently) unavoidable conversion...
      // We could move DataSet (and ParmDB.*) to Strategy as well?
      ParameterSet tmp;
      for(ParameterSet::const_iterator it = ps.begin(); it != ps.end(); ++it)
      {
        if(it->first == "DataSet"
          || it->first == "ParmDB.Instrument"
          || it->first == "ParmDB.LocalSky"
          || it->first == "ParmDB.History")
        {
            tmp.add(it->first, it->second);
        }
        else
            tmp.add("Strategy." + it->first, it->second);
      }

      // Create a BBSStrategy object using the parameter set and return it.
      return new BBSStrategy(tmp);
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

