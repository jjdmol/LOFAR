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
#include <BBSControl/Strategy.h>
#include <BBSControl/Step.h>
#include <BBSControl/InitializeCommand.h>
#include <BBSControl/NextChunkCommand.h>
#include <BBSControl/SynchronizeCommand.h>
#include <BBSControl/FinalizeCommand.h>
#include <BBSControl/CommandQueue.h>

#include <Common/ParameterSet.h>
#include <Blob/BlobIStream.h>
#include <Blob/BlobIBufStream.h>
#include <Common/LofarLogger.h>
#include <Common/Exceptions.h>
#include <Common/lofar_fstream.h>
#include <Common/lofar_algorithm.h>

#include <casa/Quanta/Quantum.h>
#include <casa/Quanta/MVTime.h>

#include <unistd.h>    // for sleep()



namespace LOFAR
{
  namespace BBS
  {
    // Unnamed namespace, used to define local (static) variables, etc.
    namespace
    {
      uint nrLocalCtrls(0);     // Number of local controllers
      uint nrGlobalSolvers(0);  // Number of global solvers
      uint nrClients(0);        // Number of clients (sum of the above two)

      // IDs of the last "next chunk" and "finalize" commands (if any).
      CommandId nextChunkId(0);
      CommandId finalizeId(0);
      
      bool dummy1 = BlobStreamableFactory::instance().registerClass<RegularAxis>("RegularAxis");
      bool dummy2 = BlobStreamableFactory::instance().registerClass<OrderedAxis>("OrderedAxis");      
    }


    //##--------   P u b l i c   m e t h o d s   --------##//

    GlobalProcessControl::GlobalProcessControl() :
      ProcessControl(),
      itsState(UNDEFINED),
      itsFreqStart(0.0),
      itsFreqEnd(0.0),
      itsTimeStart(0),
      itsTimeEnd(0),
      itsChunkStart(0)
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
    	itsStrategy.reset(new Strategy(*globalParameterSet()));

    	// Retrieve the steps in the strategy in sequential order.
        itsSteps = itsStrategy->getAllSteps();
    	LOG_DEBUG_STR("# of steps in strategy: " << itsSteps.size());

        // Read MetaMeasurement file.	
        ifstream fin(itsStrategy->dataSet().c_str());
        BlobIBufStream bufs(fin);
        BlobIStream ins(bufs);
        ins >> itsMetaMeasurement;
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
          (new CommandQueue(globalParameterSet()->getString("BBDB.Name"),
                            globalParameterSet()->getString("BBDB.Username"),
                            globalParameterSet()->getString("BBDB.Host"),
                            globalParameterSet()->getString("BBDB.Port")));

        // Register for the "result" trigger, which fires when a new result is
        // posted to the blackboard database.
        itsCommandQueue->registerTrigger(CommandQueue::Trigger::Result);

        // Retrieve the number of kernels.
        nrLocalCtrls = globalParameterSet()->getUint32("Control.KernelCount");
        LOG_DEBUG_STR("Number of kernels: " << nrLocalCtrls);

        // Retrieve the number of global solvers.
        nrGlobalSolvers =
          globalParameterSet()->getUint32("Control.SolverCount");
        LOG_DEBUG_STR("Number of global solvers: " << nrGlobalSolvers);

        // Send the strategy.
        itsCommandQueue->setStrategy(*itsStrategy);

        // Send an "initialize" command to the queue.
        CommandId id = itsCommandQueue->addCommand(InitializeCommand());
        LOG_DEBUG_STR("Initialize command has ID: " << id);

        // Get a reference to the results registered in our local result map
        // for the given command-id.
        ResultMapType::mapped_type& results = itsResults[id];

        // Wait until all "clients" have acknowledged the "initialize"
        // command.
        nrClients = nrLocalCtrls + nrGlobalSolvers;
        while (results.size() < nrClients) {

          LOG_DEBUG("Waiting for result trigger ...");
          if (itsCommandQueue->waitForTrigger(CommandQueue::Trigger::Result)) {

            // Retrieve the new results from the command queue.
            vector<ResultType> newResults = itsCommandQueue->getNewResults(id);

            for (uint i = 0; i < newResults.size(); ++i) {
              LOG_DEBUG_STR(newResults[i].first << " returned: " << 
                            newResults[i].second.asString());
            }

            // Add the new results to our local results vector.
            results.insert(results.end(), 
                           newResults.begin(), newResults.end());
          }
          ASSERT(results.size() <= nrClients);
          LOG_DEBUG_STR(results.size() << " out of " << nrClients << 
                        " clients have responded");
        }

        // Did all "clients" respond with an "OK" status?
        // Here we have to iterate over all elements in result -- a count()
        // will not do, because result is a vector<pair<SenderId,
        // CmdResult> > instead of a "plain" vector<CmdResult>. Could this be
        // avoided by choosing a different STL container for ResultMapType??
        LOG_TRACE_CALC("Results:");
        uint notOk(0);
        for (uint i = 0; i < results.size(); ++i) {
          LOG_TRACE_CALC_STR(results[i].first << " - " << results[i].second);
          if (!results[i].second) {
            LOG_DEBUG_STR(results[i].first << " returned: " << 
                          results[i].second.asString());
            notOk++;
          }
        }
        LOG_DEBUG_STR(notOk << " out of " << nrClients << 
                      " clients returned an error status");

        // Set the steps iterator to the *end* of the steps vector. This
        // sounds odd, but it is a safety net to insure that all local
        // controllers execute a "next chunk" command prior to any step.
        itsStepsIterator = itsSteps.end();

        // Switch to NEXT_CHUNK state, indicating that the next thing we
        // should do is send a "next chunk" command.
        setState(NEXT_CHUNK);

        // Determine ROI in frequency.
        const RegionOfInterest &roi = itsStrategy->regionOfInterest();
        pair<double, double> range = itsMetaMeasurement.getFreqRange();
        itsFreqStart = range.first;
        itsFreqEnd = range.second;

        // Determine ROI in time.
        const Axis::ShPtr timeAxis = itsMetaMeasurement.getTimeAxis();

        itsTimeStart = 0;
        itsTimeEnd = timeAxis->size() - 1;
        casa::Quantity time;
        if(!roi.time.empty() && casa::MVTime::read(time, roi.time[0])) {
          const size_t tslot = timeAxis->locate(time.getValue("s"));
          if(tslot < timeAxis->size()) {
            itsTimeStart = tslot;
          }
        }
        if(roi.time.size() > 1 && casa::MVTime::read(time, roi.time[1])) {
          itsTimeEnd = timeAxis->locate(time.getValue("s"), false);
        }
        itsChunkStart = itsTimeStart;
        itsChunkSize = itsStrategy->chunkSize();
        if(itsChunkSize == 0) {
          // If chunk size equals 0, take the whole region of interest
          // as a single chunk.
          itsChunkSize = itsTimeEnd - itsTimeStart + 1;
        }
        LOG_DEBUG_STR("Time range: [" << itsTimeStart << "," << itsTimeEnd
                      << "]");
        LOG_DEBUG_STR("Chunk size: " << itsChunkSize << " time slot(s)");

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
          ASSERT(itsChunkStart <= itsTimeEnd);

          const Axis::ShPtr timeAxis = itsMetaMeasurement.getTimeAxis();
          const double start = timeAxis->lower(itsChunkStart);
          const double end =
            timeAxis->upper(std::min(itsChunkStart + itsChunkSize - 1,
                                     itsTimeEnd));
          
          itsChunkStart += itsChunkSize;
          
          NextChunkCommand cmd(itsFreqStart, itsFreqEnd, start, end);          
          nextChunkId = itsCommandQueue->addCommand(cmd);
          LOG_DEBUG_STR("Next-chunk command has ID: " << nextChunkId);

          setState(NEXT_CHUNK_WAIT);
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
              LOG_DEBUG_STR(newResults[i].first << " returned: " << 
                            newResults[i].second.asString());
              if (newResults[i].first.type() == SenderId::KERNEL &&
                  newResults[i].second) {
                setState(RUN);
                itsStepsIterator = itsSteps.begin();
                break;
              }
            }
          }
          
          // If we're still in NEXT_CHUNK_WAIT state and if all local
          // controllers have responded, check if all returned an OUT_OF_DATA
          // status. If so, we're done and must switch to FINALIZE state.
          if (itsState == NEXT_CHUNK_WAIT && results.size() == nrClients) {
            uint nrOutOfData(0);
            for (uint i = 0; i < results.size(); ++i) {
              if (results[i].first.type() == SenderId::KERNEL &&
                  results[i].second == CommandResult::OUT_OF_DATA) {
                nrOutOfData++;
              }
            }
            if (nrOutOfData == nrLocalCtrls) {
              LOG_DEBUG("All local controllers responded with OUT_OF_DATA.");
              setState(FINALIZE);
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
            //             setState(WAIT);
          } else if(itsChunkStart > itsTimeEnd) {
            setState(FINALIZE);
          } else {
            setState(NEXT_CHUNK);
          }
          break;
        }


        case WAIT: {
          // Wait for a trigger from the database. If trigger received within
          // time-out period, switch to RUN state.
          LOG_TRACE_FLOW("RunState::WAIT");

          if (itsCommandQueue->waitForTrigger(CommandQueue::Trigger::Result)) {
            setState(RUN);
          }
          break;
        }


        case FINALIZE: {
          // Send "finalize" command. Wait until all local controllers have
          // responded. If all local controllers returned an OK status, mark
          // the strategy as 'done'.
          LOG_TRACE_FLOW("RunState::FINALIZE");

          finalizeId = itsCommandQueue->addCommand(FinalizeCommand());
          LOG_DEBUG_STR("Finalize command has ID: " << finalizeId);

          setState(FINALIZE_WAIT);
          break;
        }


        case FINALIZE_WAIT: {
          // Wait for a "result trigger" from the database. If trigger
          // received within time-out period, fetch new results. When all
          // "clients" have acknowledged the "finalize" command, we can switch
          // to the QUIT state.
          LOG_TRACE_FLOW("RunState::FINALIZE_WAIT");

          // Get a reference to the results registered in our local result map.
          ResultMapType::mapped_type& results = itsResults[finalizeId];

          // Did all "clients" acknowledge the "finalize" command?
          if (results.size() < nrClients) { 
            // No, wait for a result.
            LOG_DEBUG("Waiting for result trigger ...");
            
            if (itsCommandQueue->
                waitForTrigger(CommandQueue::Trigger::Result)) {

              // Retrieve the new results from the command queue.
              vector<ResultType> newResults = 
                itsCommandQueue->getNewResults(finalizeId);

              // Add the new results to our local results vector.
              results.insert(results.end(), 
                             newResults.begin(), newResults.end());
            }
            LOG_DEBUG_STR(results.size() << " out of " << nrClients << 
                          " clients have responded");
          }
          else { 
            // Yes, all clients have responded.
            ASSERT(results.size() <= nrClients);

            // Did all local controllers respond with an "OK" status?
            LOG_TRACE_CALC("Results:");
            uint notOk(0);
            for (uint i = 0; i < results.size(); ++i) {
              LOG_TRACE_CALC_STR(results[i].first << " - " << 
                                 results[i].second);
              if (!results[i].second) {
                LOG_DEBUG_STR(results[i].first << " returned: " << 
                              results[i].second.asString());
                notOk++;
              }
            }
            LOG_DEBUG_STR(notOk << " out of " << nrClients << 
                          " clients returned an error status");

            // Only set strategy state flag to "done" if none of the local
            // controllers returned an error status.
            if (notOk == 0) {
              itsCommandQueue->setStrategyDone();
            }

            // Switch to QUIT state.
            setState(QUIT);
          }
          break;
        }

        case QUIT: {
          // We're done. All we have to do now is wait for ACC to invoke
          // quit(). Sleep for a second to avoid continuous polling by
          // ACCmain.
          LOG_TRACE_FLOW("RunState::QUIT");
          clearRunState();
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
      return indeterminate;
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
      return indeterminate;
    }


    tribool GlobalProcessControl::snapshot(const string& /*destination*/)
    {
      LOG_INFO("GlobalProcessControl::snapshot()");
      LOG_WARN("Not supported");
      return indeterminate;
    }


    tribool GlobalProcessControl::recover(const string& /*source*/)
    {
      LOG_INFO("GlobalProcessControl::recover()");
      LOG_WARN("Not supported");
      return indeterminate;
    }


    tribool GlobalProcessControl::reinit(const string& /*configID*/)
    {
      LOG_INFO("GlobalProcessControl::reinit()");
      LOG_WARN("Not supported");
      return indeterminate;
    }

    string GlobalProcessControl::askInfo(const string& /*keylist*/)
    {
      LOG_INFO("GlobalProcessControl::askInfo()");
      LOG_WARN("Not supported");
      return string();
    }


    //##--------   P r i v a t e   m e t h o d s   --------##//


    void GlobalProcessControl::setState(RunState state) 
    {
      itsState = state;
      LOG_DEBUG_STR("Switching to " << showState() << " state");
    }

    const string& GlobalProcessControl::showState() const
    {
      //# Caution: Always keep this array of strings in sync with the enum
      //#          State that is defined in the header file!
      static const string states[N_States+1] = {
        "NEXT_CHUNK",
        "NEXT_CHUNK_WAIT",
        "RUN",
        "WAIT",
        "RECOVER",
        "FINALIZE",
        "FINALIZE_WAIT",
        "QUIT",
        "<UNDEFINED>"  //# This should ALWAYS be last !!
      };
      if (UNDEFINED < itsState && itsState < N_States) return states[itsState];
      else return states[N_States];
    }
     

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
      // Here we have to iterate over all elements in result -- a count() will
      // not do, because result is a vector< pair<SenderId, CmdResult> >
      // instead of a "plain" vector<CmdResult>. Could this be avoided by
      // choosing a different STL container for ResultMapType??
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
