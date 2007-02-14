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
//# A BBSControlException will be thrown, containing the original pqxx
//# exception class type and the description.
#if defined(CATCH_PQXX_AND_RETHROW)
# error CATCH_PQXX_AND_RETHROW is already defined and should not be redefined
#else
# define CATCH_PQXX_AND_RETHROW						\
  catch (pqxx::broken_connection& e) {				\
    THROW (BBSControlException, "pqxx::broken_connection:\n"	\
	   << e.what());					\
  }								\
  catch (pqxx::sql_error& e) {					\
    THROW (BBSControlException, "pqxx::sql_error:\n"		\
	   << "Query: " << e.query() << endl << e.what());	\
  }								\
  catch (pqxx::in_doubt_error& e) {				\
    THROW (BBSControlException, "pqxx::in_doubt_error:\n"	\
	   << e.what());					\
  }								\
  catch (pqxx::internal_error& e) {				\
    THROW (BBSControlException, "pqxx::internal_error:\n"	\
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
      LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW, "");

      string opts("dbname=" + dbname  + " user=" + user + 
		  " host=" + host + " port=" + port);
      try {
	LOG_DEBUG_STR("Connecting to database using options: " << opts);
	itsConnection.reset(new pqxx::connection(opts));
      } CATCH_PQXX_AND_RETHROW;
    }


    void CommandQueue::addStep(const BBSSingleStep& step)
    {
      LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW, "");
      ostringstream query;

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
	const BBSSolveStep& solveStep = dynamic_cast<const BBSSolveStep&>(step);
	query << ",'" << solveStep.maxIter()                  << "'"
	      << ",'" << solveStep.epsilon()                  << "'"
	      << ",'" << solveStep.minConverged()             << "'"
	      << ",'" << solveStep.parms()                    << "'"
	      << ",'" << solveStep.exclParms()                << "'"
	      << ",'" << solveStep.domainSize().bandWidth     << "'"
	      << ",'" << solveStep.domainSize().timeInterval  << "'";
      } catch (bad_cast&) {}

      // Finalize the query.
      query << ")";

      // Execute the query. 
      execQuery(query.str());
    } 


    const BBSSingleStep* CommandQueue::getNextStep()
    {
      LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW, "");

      // Compose the query.
      ostringstream query;
      query << "SELECT * FROM blackboard.get_next_step("
	    << itsCurrentId
	    << ")";

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
	query << "SELECT * FROM blackboard.get_solve_arguments("
	      << itsCurrentId
	      << ")";

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

      // Create a BBSStep from the parameter set.
      const BBSStep* step = BBSStep::create(name, ps, 0);
      return dynamic_cast<const BBSSingleStep*>(step);
    }


    void CommandQueue::setStrategy(const BBSStrategy& strategy)
    {
      LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW, "");

      // Compose the query.
      ostringstream query;
      query << "SELECT * FROM blackboard.set_strategy(" 
	    << "('" << strategy.dataSet()                 << "'"
	    << ",'" << strategy.parmDB().localSky         << "'"
	    << ",'" << strategy.parmDB().instrument       << "'"
	    << ",'" << strategy.parmDB().history          << "'"
	    << ",'" << strategy.stations()                << "'"
	    << ",'" << strategy.inputData()               << "'"
	    << ",'" << strategy.domainSize().bandWidth    << "'"
	    << ",'" << strategy.domainSize().timeInterval << "'"
	    << ",'" << strategy.correlation().selection   << "'"
	    << ",'" << strategy.correlation().type        << "')";

      // Execute the query.
      execQuery(query.str());
    }


    const BBSStrategy* CommandQueue::getStrategy()
    {
      LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW, "");

      // Compose the query.
      ostringstream query;
      query << "SELECT * FROM blackboard.get_strategy()";

      // Execute the query. The result will be returned as a ParameterSet.
      ParameterSet ps = execQuery(query.str());

      // If DataSet is an empty string, then we've probably received an empty
      // result row; return a null pointer.
      if (ps.getString("DataSet").empty()) return 0;

      // Create a BBSStrategy object using the parameter set and return it.
      return new BBSStrategy(ps);
    }


    //##--------   P r i v a t e   m e t h o d s   --------##//

    ParameterSet CommandQueue::execQuery(const string& query)
    {
      // Create a transactor object and execute the query. The result will be
      // stored, as a string in \a result.
      string result;
      try {
	itsConnection->perform(ExecQuery(query, result));
      } CATCH_PQXX_AND_RETHROW;

      // Create an empty parameter set and add the result to it.
      ParameterSet ps;
      ps.adoptBuffer(result);

      // Return the ParameterSet
      return ps;
    }

  } // namespace BBS
  
} // namespace LOFAR

