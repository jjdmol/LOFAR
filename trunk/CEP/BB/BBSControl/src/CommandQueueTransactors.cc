//#  CommandQueueTransactors.cc: Transaction functors for the BBS command queue
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
#include <BBSControl/CommandQueueTransactors.h>
// #include <BBSControl/BBSSingleStep.h>
// #include <BBSControl/BBSSolveStep.h>
// #include <BBSControl/BBSSubtractStep.h>
// #include <BBSControl/BBSCorrectStep.h>
// #include <BBSControl/BBSPredictStep.h>
// #include <BBSControl/BBSShiftStep.h>
// #include <BBSControl/BBSRefitStep.h>
#include <BBSControl/Exceptions.h>
// #include <BBSControl/pqutil.h>
#include <Common/LofarLogger.h>
#include <Common/StreamUtil.h>
#include <Common/lofar_typeinfo.h>

namespace LOFAR
{
  namespace BBS 
  {
    using LOFAR::operator<<;


    //##--------  ExecQuery  --------##//

    string ExecQuery::emptyString;

    ExecQuery::ExecQuery(const string& query) :
      pqxx::transactor<>("ExecQuery"),
      itsQuery(query),
      itsResult(emptyString)
    {
    }


    ExecQuery::ExecQuery(const string& query, string& result) :
      pqxx::transactor<>("ExecQuery"),
      itsQuery(query),
      itsResult(result)
    {
    }


    void ExecQuery::operator()(argument_type& transaction)
    {
      LOG_DEBUG_STR("Executing query : " << itsQuery);
      itsPQResult = transaction.exec(itsQuery);
      // Assert that the result contains exactly one row; if not the
      // stored procedure should have thrown an exception.
      ASSERT(itsPQResult.size() == 1); 
    }


    void ExecQuery::on_commit()
    {
      ostringstream oss;
      for (uint i = 0; i < itsPQResult.columns(); ++i) {
	oss << itsPQResult[0][i].name()  << " = " 
	    << itsPQResult[0][i].c_str() << endl;
      }
      itsResult = oss.str();
    }


#if 0

    //##--------  AddStep  --------##//

    AddStep::AddStep(const BBSSingleStep& step) :
      itsStep(step)
    {
    }
    

    void AddStep::operator()(argument_type& t)
    {
      LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW, "");
      ostringstream query;

      // First build the SELECT part of the query.
      query << "SELECT * FROM blackboard.add_" 
 	    << toLower(itsStep.operation()) 
	    << "_step";

      // The first 8 arguments are the same for all stored procedures.
      query << "('" << itsStep.getName()               << "'"
	    << ",'" << itsStep.baselines().station1    << "'"
	    << ",'" << itsStep.baselines().station2    << "'"
	    << ",'" << itsStep.correlation().selection << "'"
	    << ",'" << itsStep.correlation().type      << "'"
	    << ",'" << itsStep.sources()               << "'"
	    << ",'" << itsStep.instrumentModels()      << "'"
	    << ",'" << itsStep.outputData()            << "'";

      // The stored procedure for a BBSSolveStep needs more arguments.
      try {
	const BBSSolveStep& step = dynamic_cast<const BBSSolveStep&>(itsStep);
	query << ",'" << step.maxIter()                  << "'"
	      << ",'" << step.epsilon()                  << "'"
	      << ",'" << step.minConverged()             << "'"
	      << ",'" << step.parms()                    << "'"
	      << ",'" << step.exclParms()                << "'"
	      << ",'" << step.domainSize().bandWidth     << "'"
	      << ",'" << step.domainSize().timeInterval  << "'";
      } catch (bad_cast&) {}

      // Finalize the query.
      query << ")";

      // Execute the query.
      LOG_DEBUG_STR("Executing query : " << query.str());
      t.exec(query.str());
    }


    //##--------  GetNextStep  --------##//

    GetNextStep::GetNextStep(ostream& os, uint& currentId) :
      itsStream(os),
      itsCurrentId(currentId),
      itsNewId(0)
    {
    }


    int GetNextStep::operator()(argument_type& t)
    {
      LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW, "");
      ostringstream q;
      pqxx::result r;

      // Compose the query.
      q << "SELECT * FROM blackboard.get_next_step(" << itsCurrentId << ")";

      // Execute the query.
      LOG_DEBUG_STR("Executing query : " << q.str());
      r = t.exec(q.str());

      // Set the ID of the new command
      r[0]["command_id"].to(itsNewId);

      // Write the result to the output stream itsStream
      string prefix = string("Step.") + r[0]["\"Name\""].c_str() + ".";
      for (uint i = 0; i < r.columns(); ++i) {
	itsStream << prefix 
		  << r[0][i].name()  << " = "  
		  << r[0][i].c_str() << endl;
      }

      // If next step is a "solve" step, we must retrieve extra arguments.
      if (toUpper(r[0]["\"Operation\""].c_str()) == "SOLVE") {
	q.str("");

	// Compose the query.
	q << "SELECT * FROM blackboard.get_solve_arguments(" 
	  << itsNewId 
	  << ")";

	// Execute the query.
	LOG_DEBUG_STR("Executing query : " << q.str());
	r = t.exec(q.str());

	// Write the result to the output stream itsStream
	for (uint i = 0; i < r.columns(); ++i) {
	  itsStream << prefix
		    << r[0][i].name() << " = "  
		    << r[0][i].c_str() << endl;
	}
      }
      
    }


    void GetNextStep::on_commit()
    {
      // Update the current command ID
      itsCurrentId = itsNewId;
    }

#endif

  } // namespace BBS
  
} // namespace LOFAR
