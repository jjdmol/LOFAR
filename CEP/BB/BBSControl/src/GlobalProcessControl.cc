//#  GlobalProcessControl.cc: Implementation of ACC/PLC ProcessControl class.
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

#include <lofar_config.h>

#include <BBSControl/GlobalProcessControl.h>
#include <BBSControl/Exceptions.h>
#include <BBSControl/BBSStrategy.h>
#include <BBSControl/BBSStep.h>
#include <BBSControl/InitializeCommand.h>
#include <BBSControl/NextChunkCommand.h>
#include <BBSControl/SynchronizeCommand.h>
#include <BBSControl/FinalizeCommand.h>
#include <BBSControl/CommandQueue.h>
#include <Common/LofarLogger.h>
#include <Common/Exceptions.h>

#include <BBSControl/CommandId.h>
#include <BBSControl/LocalControlId.h>

#include <unistd.h>    // for sleep()

using namespace LOFAR::ACC::APS;

namespace LOFAR
{
  namespace BBS
  {
    // Unnamed namespace, used to define local (static) variables, etc.
    namespace
    {
      // Number of local controllers
      uint nrLocalCtrls(0);

      // ID of the last "next chunk" command (if any).
      CommandId nextChunkId(0);
    }


    //##--------   P u b l i c   m e t h o d s   --------##//

    GlobalProcessControl::GlobalProcessControl() :
      ProcessControl(),
      itsState(UNDEFINED)
    {
      LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW, "");
    }
    

    GlobalProcessControl::~GlobalProcessControl()
    {
      LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW, "");
    }


    tribool GlobalProcessControl::define()
    {
      LOG_INFO("GlobalProcessControl::define()");
      
      try {
	// Retrieve the strategy from the parameter set.
	itsStrategy.reset(new BBSStrategy(*globalParameterSet()));

	// Retrieve the steps in the strategy in sequential order.
        itsSteps = itsStrategy->getAllSteps();
	LOG_DEBUG_STR("# of steps in strategy: " << itsSteps.size());
      }
      catch (Exception& e) {
	LOG_ERROR_STR(e);
        return false;
      }
      // All went well.
      return true;
    }


    tribool GlobalProcessControl::init()
    {
      LOG_INFO("GlobalProcessControl::init()");

      try {
        // Create a new CommandQueue. This will open a connection to the
        // blackboard database.
        itsCommandQueue.reset
          (new CommandQueue(globalParameterSet()->getString("BBDB.DBName"),
          globalParameterSet()->getString("BBDB.UserName"),
          globalParameterSet()->getString("BBDB.Host"),
          globalParameterSet()->getString("BBDB.Port")));

        // Register for the "result" trigger, which fires when a new result is
        // posted to the blackboard database.
        itsCommandQueue->registerTrigger(CommandQueue::Trigger::Result);

        // Retrieve the number of local controllers.
        nrLocalCtrls = globalParameterSet()->getUint32("NrLocalCtrls");
        LOG_DEBUG_STR("Number of local controllers: " << nrLocalCtrls);

        // Send the strategy.
        itsCommandQueue->setStrategy(*itsStrategy);

        // Send an "initialize" command to the queue.
        CommandId id = itsCommandQueue->addCommand(InitializeCommand());
        LOG_DEBUG_STR("Initialize command has ID: " << id);

        // Get a reference to the results registered in our local result map
        // for the given command-id.
        ResultMapType::mapped_type& results = itsResults[id];

        // Wait until all local controllers have acknowledged the "initialize"
        // command.
        while (results.size() < nrLocalCtrls) {

          LOG_DEBUG("Waiting for result trigger ...");
          if (itsCommandQueue->waitForTrigger(CommandQueue::Trigger::Result)) {

            // Retrieve the new results from the command queue.
            vector<ResultType> newResults = itsCommandQueue->getNewResults(id);

            // Add the new results to our local results vector.
            results.insert(results.end(), 
                           newResults.begin(), newResults.end());
          }
          LOG_DEBUG_STR(results.size() << " out of " << nrLocalCtrls << 
                        " local controllers have responded");
        }

        // Did all local controllers respond with an "OK" status?
        // Here we have to iterate over all elements in result -- a count()
        // will not do, because result is a vector<pair<LocalControlId,
        // CmdResult> > instead of a "plain" vector<CmdResult>. Could this be
        // avoided by choosing a different STL container for ResultMapType??
        LOG_TRACE_CALC("Results:");
        uint notOk(0);
        for (uint i = 0; i < results.size(); ++i) {
          LOG_TRACE_CALC_STR(results[i].first << " - " << results[i].second);
          if (!results[i].second) {
            LOG_DEBUG_STR("Local controller " << results[i].first <<
                          " returned: " << results[i].second.asString());
            notOk++;
          }
        }
        LOG_DEBUG_STR(notOk << " out of " << nrLocalCtrls << 
                      " local controllers returned an error status");

        // Set the steps iterator to the *end* of the steps vector. This
        // sounds odd, but it is a safety net to insure that all local
        // controllers execute a "next chunk" command prior to any step.
        itsStepsIterator = itsSteps.end();

        // Switch to NEXT_CHUNK state, indicating that the next thing we
        // should do is send a "next chunk" command.
        itsState = NEXT_CHUNK;

        // Only return true if none of the local controllers returned an error
        // status.
        return notOk == 0;
      }
      catch (Exception& e) {
	LOG_ERROR_STR(e);
	return false;
      }
    }


    tribool GlobalProcessControl::run()
    {
      LOG_INFO("GlobalProcessControl::run()");

      try {
        
        switch(itsState) {

        case NEXT_CHUNK: {
          // Send a "next chunk" command. 
          LOG_TRACE_FLOW("RunState::NEXT_CHUNK");

          nextChunkId = itsCommandQueue->addCommand(NextChunkCommand());
          LOG_DEBUG_STR("Next-chunk command has ID: " << nextChunkId);

          itsState = NEXT_CHUNK_WAIT;
          break;
        }


        case NEXT_CHUNK_WAIT: {
          // Wait for a "result trigger" form the database. If trigger
          // received within time-out period, fetch new results. If any of the
          // new results contain an OK status to the "next chunk" command,
          // switch to RUN state.
          LOG_TRACE_FLOW("RunState::NEXT_CHUNK_WAIT");

          // Get a reference to the results registered in our local result map
          // for the given command-id.
          ResultMapType::mapped_type& results = itsResults[nextChunkId];

          LOG_DEBUG("Waiting for result trigger ...");
          if (itsCommandQueue->waitForTrigger(CommandQueue::Trigger::Result)) {

            // Retrieve the new results from the command queue.
            vector<ResultType> newResults = 
              itsCommandQueue->getNewResults(nextChunkId);

            // Add the new results to our local results vector.
            results.insert(results.end(), 
                           newResults.begin(), newResults.end());

            // If any of the local controllers have responded with an OK
            // status, switch to RUN state and set the steps iterator to the
            // beginning of the strategy.
            for (uint i = 0; i < newResults.size(); ++i) {
              LOG_DEBUG_STR("Local controller " << newResults[i].first << 
                            " returned: "<< newResults[i].second.asString());
              if (newResults[i].second) {
                LOG_DEBUG("Switching to RUN state");
                itsState = RUN;
                itsStepsIterator = itsSteps.begin();
                break;
              }
            }
          }

          // If we're still in NEXT_CHUNK_WAIT state and if all local
          // controllers have responded, check if all returned an OUT_OF_DATA
          // status. If so, we're done and must switch to FINALIZE state.
          if (itsState == NEXT_CHUNK_WAIT && results.size() == nrLocalCtrls) {
            uint nrOutOfData(0);
            for (uint i = 0; i < results.size(); ++i) {
              if (results[i].second.is(CommandResult::OUT_OF_DATA)) {
                nrOutOfData++;
              }
            }
            if (nrOutOfData == nrLocalCtrls) {
              LOG_DEBUG("All local controllers responded with OUT_OF_DATA. "
                        "Switching to FINALIZE state");
              itsState = FINALIZE;
            } else {
              LOG_DEBUG("One or more local controllers returned with "
                        "an error status");
              // Returning false might not be the best thing to do in the end
              // -- doing a LOG_WARN() is probably better -- but it will
              // (hopefully) make debugging easier.
              return false;
            }
          }
          break;
        }


        case RUN: {
          // Send the next "step" command and switch to the WAIT state, unless
          // we're at the end of the strategy. In that case we should switch
          // to the NEXT_CHUNK state.
          LOG_TRACE_FLOW("RunState::RUN");

          if (itsStepsIterator != itsSteps.end()) {
            itsCommandQueue->addCommand(**itsStepsIterator++);
//             LOG_DEBUG("Switching to WAIT state");
//             itsState = WAIT;
          } else {
            LOG_DEBUG("Switching to NEXT_CHUNK state");
            itsState = NEXT_CHUNK;
          }
          break;
        }


        case WAIT: {
          // Wait for a trigger from the database. If trigger received within
          // time-out period, switch to RUN state.
          LOG_TRACE_FLOW("RunState::WAIT");

          if (itsCommandQueue->waitForTrigger(CommandQueue::Trigger::Result)) {
            itsState = RUN;
          }
          break;
        }


        case FINALIZE: {
          // Send "finalize" command. Wait until all local controllers have
          // responded. If all local controllers returned an OK status, mark
          // the strategy as 'done'.
          LOG_TRACE_FLOW("RunState::FINALIZE");

          CommandId id = itsCommandQueue->addCommand(FinalizeCommand());
          LOG_DEBUG_STR("Finalize command has ID: " << id);

          // Get a reference to the results registered in our local result map.
          ResultMapType::mapped_type& results = itsResults[id];

          // Wait until all local controllers have acknowledged the "finalize"
          // command.
          while (results.size() < nrLocalCtrls) {

            LOG_DEBUG("Waiting for result trigger ...");
            
            if (itsCommandQueue->
                waitForTrigger(CommandQueue::Trigger::Result)) {

            // Retrieve the new results from the command queue.
            vector<ResultType> newResults = itsCommandQueue->getNewResults(id);

            // Add the new results to our local results vector.
            results.insert(results.end(), 
                           newResults.begin(), newResults.end());
            }
            LOG_DEBUG_STR(results.size() << " out of " << nrLocalCtrls << 
                          " local controllers have responded");
          }
          
          // Did all local controllers respond with an "OK" status?
          LOG_TRACE_CALC("Results:");
          uint notOk(0);
          for (uint i = 0; i < results.size(); ++i) {
            LOG_TRACE_CALC_STR(results[i].first << " - " << results[i].second);
            if (!results[i].second) {
              LOG_DEBUG_STR("Local controller " << results[i].first <<
                            " returned: " << results[i].second.asString());
              notOk++;
            }
          }
          LOG_DEBUG_STR(notOk << " out of " << nrLocalCtrls << 
                        " local controllers returned an error status");

          // Only set strategy state flag to "done" if none of the local
          // controllers returned an error status.
          if (notOk == 0) {
            itsCommandQueue->setStrategyDone();
          }

          // Switch to QUIT state.
          itsState = QUIT;
          break;
        }


        case QUIT: {
          // We're done. All we have to do now is wait for ACC to invoke
          // quit(). Sleep for a second to avoid continuous polling by
          // ACCmain.
          LOG_TRACE_FLOW("RunState::QUIT");
          sleep(1);
          break;
        }


        default: {
          THROW (GlobalControlException, "Wrong RunState: " << itsState);
          break;
        }
          
        } // switch
      }
      catch (Exception& e) {
        LOG_ERROR_STR(e);
        return false;
      }
      
      // All went well.
      return true;
    }
    

    tribool GlobalProcessControl::release()
    {
      LOG_INFO("GlobalProcessControl::release()");
      LOG_WARN("Not supported");
      return false;
    }

    tribool GlobalProcessControl::quit()
    {
      // Do we want to send a "finalize" command first? It might be a good
      // way to properly quit. But then again, how long should we wait for
      // the local controllers to respond. A terribly slow local controller
      // could still be "miles away" from processing the "finalize" command,
      // because several "steps" are still in the queue. It's not likely it
      // will be able to quit gracefully before the ACC quit timer expires.
      LOG_INFO("GlobalProcessControl::quit()");
      return itsState == QUIT;
    }


    tribool GlobalProcessControl::pause(const string& /*condition*/)
    {
      LOG_INFO("GlobalProcessControl::pause()");
      LOG_WARN("Not supported");
      return false;
    }


    tribool GlobalProcessControl::snapshot(const string& /*destination*/)
    {
      LOG_INFO("GlobalProcessControl::snapshot()");
      LOG_WARN("Not supported");
      return false;
    }


    tribool GlobalProcessControl::recover(const string& /*source*/)
    {
      LOG_INFO("GlobalProcessControl::recover()");
      LOG_WARN("Not supported");
      return false;
    }


    tribool GlobalProcessControl::reinit(const string& /*configID*/)
    {
      LOG_INFO("GlobalProcessControl::reinit()");
      LOG_WARN("Not supported");
      return false;
    }

    string GlobalProcessControl::askInfo(const string& /*keylist*/)
    {
      LOG_INFO("GlobalProcessControl::askInfo()");
      LOG_WARN("Not supported");
      return string();
    }


    //##--------   P r i v a t e   m e t h o d s   --------##//


#if 0
    bool GlobalProcessControl::execCommand(const Command& cmd)
    {
      // Send the command \a cmd to the queue
      CommandId id = itsCommandQueue->addCommand(cmd);
      LOG_DEBUG_STR(cmd.type() << " command has ID: " << id);

      // Get a reference to the results registered in our local result map
      // for the given command-id.
      ResultMapType::mapped_type& results = itsResults[id];

      // Wait until all local controllers have posted a result.
      while (results.size() < itsNrLocalCtrls) {

        LOG_DEBUG("Waiting for result trigger ...");
        if (itsCommandQueue->waitForTrigger(CommandQueue::Trigger::Result)) {

          // Retrieve the new results from the command queue.
          vector<ResultType> newResults = itsCommandQueue->getNewResults(id);

          // Add the new results to our local results vector.
          results.insert(results.end(), 
                         newResults.begin(), newResults.end());
        }
        LOG_DEBUG_STR(results.size() << " out of " << itsNrLocalCtrls << 
                      " local controllers have responded");
      }

      // Did all local controllers respond with an "OK" status?
      // Here we have to iterate over all elements in result -- a count()
      // will not do, because result is a vector<pair<LocalControlId,
      // CmdResult> > instead of a "plain" vector<CmdResult>. Could this be
      // avoided by choosing a different STL container for ResultMapType??
      LOG_TRACE_CALC("Results:");
      uint notOk(0);
      for (uint i = 0; i < results.size(); ++i) {
        LOG_TRACE_CALC_STR(results[i].first << " - " << results[i].second);
        if (!results[i].second) {
          LOG_DEBUG_STR("Local controller " << results[i].first <<
                        " returned: " << results[i].second.asString());
          notOk++;
        }
      }
      LOG_DEBUG_STR(notOk << " out of " << itsNrLocalCtrls << 
                    " local controllers returned an error status");

    }
#endif


#if 0
    vector<ResultType>
    GlobalProcessControl::waitForResults(const CommandId& id)
    {
      LOG_DEBUG("Waiting for result trigger ...");
      if (itsCommandQueue->waitForTrigger(CommandQueue::Trigger::Result)) {
        
        // Retrieve the new results from the command queue.
        vector<ResultType> newResults = itsCommandQueue->getNewResults(id);

        // Get a reference to the results registered in our local result map
        // for the given command-id. If we're worried about exception safety,
        // we should not use a reference here, but get a copy which should
        // later be swapped with the orginal.
        ResultMapType::mapped_type& results = itsResults[id];
        
        // Add the new results to our local results vector.
        results.insert(results.end(), newResults.begin(), newResults.end());
        
        // Return the new results.
        return newResults;
      }
    }


    ResultMapType GlobalProcessControl::waitForResults()
    {
      LOG_DEBUG("Waiting for result trigger ...");
      if (itsCommandQueue->waitForTrigger(CommandQueue::Trigger::Result)) {
        
        // Retrieve the new results from the command queue.
        ResultMapType newResults = itsCommandQueue->getNewResults();

        // Insert the results in our local result map. If we're worried about
        // exception safety, we should make a copy of our local result map,
        // which should later be swapped with the orginal.
        itsResults.insert(newResults.begin(), newResults.end());

        // Return the new results.
        return newResults;
      }
    }
#endif


  } // namespace BBS
    
} // namespace LOFAR
