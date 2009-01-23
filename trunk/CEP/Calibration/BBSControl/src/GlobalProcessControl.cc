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
#include <BBSControl/SharedState.h>

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

#include <Common/StreamUtil.h>


namespace LOFAR
{
  namespace BBS
  {
    // Unnamed namespace, used to define local (static) variables, etc.
    namespace
    {
      // IDs of the last "next chunk" and "finalize" commands (if any).
      CommandId nextChunkId(0);
      CommandId finalizeId(0);
    }

    using LOFAR::operator<<;
    
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
        ParameterSet *ps = globalParameterSet();
        ASSERT(ps);
        
        // Read Observation descriptor.
        itsVdsDesc = CEP::VdsDesc(ps->getString("Observation"));

        string key = ps->getString("BBDB.Key", "default");
        itsSharedState.reset(new SharedState(key,
            ps->getString("BBDB.Name"),
            ps->getString("BBDB.User"),
            ps->getString("BBDB.Host", "localhost"),
            ps->getString("BBDB.Port", "5432")));

        if(!itsSharedState->registerAsControl())
        {
          LOG_ERROR_STR("Could not register as control. There may be stale"
            " state in the database for key: " << key);
          return false;
        }

        itsSharedState->initRegister(itsVdsDesc, ps->getBool("UseSolver",
            false));

        if(!itsSharedState->setRunState(SharedState::WAITING_FOR_WORKERS))
        {        
            THROW(BBSControlException, "Unable to set run state");
        }

        // Wait for workers to register.
        while(itsSharedState->slotsAvailable())
        {
            sleep(3);
        }

        if(!itsSharedState->setRunState(SharedState::COMPUTING_WORKER_INDEX))
        {        
            THROW(BBSControlException, "Unable to set run state");
        }
        
        createWorkerIndex();
        itsTimeAxis = itsSharedState->getGlobalTimeAxis();
        ASSERT(itsTimeAxis);
        
        if(!itsSharedState->setRunState(SharedState::PROCESSING))
        {        
            THROW(BBSControlException, "Unable to set run state");
        }

        InitializeCommand initCmd(*globalParameterSet());
        bool cmdDone = false;

        CommandId cmdId = itsSharedState->addCommand(initCmd);
        LOG_DEBUG_STR("Initialize command has ID: " << cmdId);
        
        // Wait for workers to execute initialize command.
        cmdDone = false;
        while(!cmdDone)
        {
            if(itsSharedState->waitForResult())
            {
                CommandStatus status = itsSharedState->getCommandStatus(cmdId);
                LOG_DEBUG_STR("Command status: #result: " << status.nResults
                    << " #fail: " << status.nFail);
                LOG_DEBUG_STR(status.nResults << " out of "
                    << itsSharedState->getWorkerCount()
                    << " workers have responded");
                    
                if(status.nFail > 0)
                {
                    LOG_ERROR_STR("One or more workers reported failure.");
                    itsSharedState->setRunState(SharedState::FAILED);
                    return false;
                }
                    
                cmdDone = (status.nResults == itsSharedState->getWorkerCount());
            }
        }

        // Set the steps iterator to the *end* of the steps vector. This
        // sounds odd, but it is a safety net to insure that all local
        // controllers execute a "next chunk" command prior to any step.
        itsStepsIterator = itsSteps.end();

        // Switch to NEXT_CHUNK state, indicating that the next thing we
        // should do is send a "next chunk" command.
        setState(NEXT_CHUNK);

        // Get frequency range of the observation.
        const SharedState::WorkerDescriptor &kernel0 =
            itsSharedState->getWorkerByIndex(SharedState::KERNEL, 0);
        itsFreqStart = kernel0.grid[0]->range().first;
        size_t nKernels = itsSharedState->getWorkerCount(SharedState::KERNEL);
        const SharedState::WorkerDescriptor &kernelN =
            itsSharedState->getWorkerByIndex(SharedState::KERNEL, nKernels - 1);
        itsFreqEnd = kernelN.grid[0]->range().second;

        // Get time window.
        vector<string> window =
            globalParameterSet()->getStringVector("TimeWindow",
                vector<string>());
                
        itsTimeStart = 0;
        itsTimeEnd = itsTimeAxis->size() - 1;

        casa::Quantity time;
        if(!window.empty() && casa::MVTime::read(time, window[0]))
        {
            const size_t tslot = itsTimeAxis->locate(time.getValue("s"));
            if(tslot < itsTimeAxis->size())
            {
                itsTimeStart = tslot;
            }
        }
        if(window.size() > 1 && casa::MVTime::read(time, window[1]))
        {
            itsTimeEnd = itsTimeAxis->locate(time.getValue("s"), false);
        }
        
        itsChunkStart = itsTimeStart;
        itsChunkSize = globalParameterSet()->getUint32("ChunkSize", 0);
        if(itsChunkSize == 0) {
          // If chunk size equals 0, take the whole region of interest
          // as a single chunk.
          itsChunkSize = itsTimeEnd - itsTimeStart + 1;
        }
        LOG_DEBUG_STR("Time range: [" << itsTimeStart << "," << itsTimeEnd
                      << "]");
        LOG_DEBUG_STR("Chunk size: " << itsChunkSize << " time slot(s)");
        
      }
        catch (Exception& e) {
            LOG_ERROR_STR(e);
            return false;
        }
     
      return true;
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

          const double start = itsTimeAxis->lower(itsChunkStart);
          const double end = itsTimeAxis->upper(std::min(itsChunkStart
            + itsChunkSize - 1, itsTimeEnd));
          
          itsChunkStart += itsChunkSize;
          
          NextChunkCommand cmd(itsFreqStart, itsFreqEnd, start, end);          
          nextChunkId =  itsSharedState->addCommand(cmd, SharedState::KERNEL);
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

            if(itsSharedState->waitForResult())
            {
                CommandStatus status = itsSharedState->getCommandStatus(nextChunkId);

/*
                vector<pair<ProcessId, CommandResult> > results =
                    itsSharedState->getResults(nextChunkId);
                
                for(size_t i = 0; i < results.size(); ++i)
                {
                    if(!results[i].second)
                    {
                        LOG_ERROR_STR("Worker " << results[i].first.hostname
                            << ":" << results[i].first.pid << " returned "
                            << results[i].second.asString() << " ("
                            << results[i].second.message() << ")");
                    }
                }
*/
                
//                if(status.nResults - status.nFail > 0)
                if(status.nResults == itsSharedState->getWorkerCount(SharedState::KERNEL))
                {
                    setState(RUN);
                    itsStepsIterator = itsSteps.begin();
                }
            }                    

            /*
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
            */
          break;
        }


        case RUN: {
          // Send the next "step" command and switch to the WAIT state, unless
          // we're at the end of the strategy. In that case we should switch
          // to the NEXT_CHUNK state.
          LOG_TRACE_FLOW("RunState::RUN");

          if (itsStepsIterator != itsSteps.end()) {
            if((*itsStepsIterator)->type() != "Solve")
                itsSharedState->addCommand(**itsStepsIterator++, SharedState::KERNEL);
            else
                itsSharedState->addCommand(**itsStepsIterator++);
            //             setState(WAIT);
          } else if(itsChunkStart > itsTimeEnd) {
            setState(FINALIZE);
          } else {
            setState(NEXT_CHUNK);
          }
          break;
        }


            /*
        case WAIT: {
          // Wait for a trigger from the database. If trigger received within
          // time-out period, switch to RUN state.
          LOG_TRACE_FLOW("RunState::WAIT");

          if (itsCommandQueue->waitForTrigger(CommandQueue::Trigger::Result)) {
            setState(RUN);
          }
          break;
        }
          */


        case FINALIZE: {
          // Send "finalize" command. Wait until all local controllers have
          // responded. If all local controllers returned an OK status, mark
          // the strategy as 'done'.
          LOG_TRACE_FLOW("RunState::FINALIZE");

          finalizeId = itsSharedState->addCommand(FinalizeCommand());
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

            if(itsSharedState->waitForResult())
            {
                CommandStatus status = itsSharedState->getCommandStatus(finalizeId);
                
                if(status.nResults == itsSharedState->getWorkerCount())
                {
                    if(status.nFail != 0)
                    {
                        itsSharedState->setRunState(SharedState::FAILED);
                    }
                    else
                    {
                        itsSharedState->setRunState(SharedState::DONE);
                    }
                
                    // Switch to QUIT state.
                    setState(QUIT);
                }
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


    void GlobalProcessControl::createWorkerIndex()
    {
        vector<SharedState::WorkerDescriptor> kernels =
            itsSharedState->getWorkersByType(SharedState::KERNEL);
        ASSERT(kernels.size() > 0);
            
        vector<pair<ProcessId, double> > index(kernels.size());
        for(size_t i = 0; i < kernels.size(); ++i)
        {
            index[i] = make_pair(kernels[i].id, kernels[i].grid[0]->lower(0));
        }
        
        stable_sort(index.begin(), index.end(), LessKernel());

        for(size_t i = 0; i < kernels.size(); ++i)
        {
            if(!itsSharedState->setIndex(index[i].first, i))
            {
                THROW(BBSControlException, "Unable to set worker index.");
            }
        }

        vector<SharedState::WorkerDescriptor> solvers =
            itsSharedState->getWorkersByType(SharedState::SOLVER);
            
        for(size_t i = 0; i < solvers.size(); ++i)
        {
            if(!itsSharedState->setIndex(solvers[i].id, i))
            {
                THROW(BBSControlException, "Unable to set worker index.");
            }
        }
    }
    
    /*
    Axis::ShPtr GlobalProcessControl::createTimeAxis()
    {
        vector<SharedState::WorkerDescriptor> kernels =
            itsSharedState->getWorkersByType(SharedState::KERNEL);
        ASSERT(kernels.size() > 0);

        int s1, e1, s2, e2;
        Axis::ShPtr axis = kernels[0].grid[1];
        for(size_t i = 1; i < kernels.size(); ++i)
        {
            axis = axis->combine(*kernels[i].grid[1], s1, e1, s2, e2);
        }

        return axis;
    }
    */
    
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


  } // namespace BBS
    
} // namespace LOFAR
