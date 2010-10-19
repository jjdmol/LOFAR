//# GlobalProcessControl.cc: Implementation of ACC/PLC ProcessControl class.
//#
//# Copyright (C) 2002-2007
//# ASTRON (Netherlands Institute for Radio Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands
//#
//# This file is part of the LOFAR software suite.
//# The LOFAR software suite is free software: you can redistribute it and/or
//# modify it under the terms of the GNU General Public License as published
//# by the Free Software Foundation, either version 3 of the License, or
//# (at your option) any later version.
//#
//# The LOFAR software suite is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License along
//# with the LOFAR software suite. If not, see <http://www.gnu.org/licenses/>.
//#
//# $Id$

#include <lofar_config.h>

#include <BBSControl/GlobalProcessControl.h>
#include <BBSControl/Exceptions.h>
#include <BBSControl/Strategy.h>
#include <BBSControl/Step.h>
#include <BBSControl/InitializeCommand.h>
#include <BBSControl/NextChunkCommand.h>
#include <BBSControl/SynchronizeCommand.h>
#include <BBSControl/FinalizeCommand.h>
#include <BBSControl/CalSession.h>

#include <Common/ParameterSet.h>
#include <Common/LofarLogger.h>
#include <Common/lofar_algorithm.h>

#include <casa/Quanta/Quantum.h>
#include <casa/Quanta/MVTime.h>

// sleep()
#include <unistd.h>

namespace LOFAR
{
  namespace BBS
  {
    using LOFAR::operator<<;

    // Unnamed namespace, used to define local (static) variables, etc.
    namespace
    {
      struct LessKernel
      {
        bool operator()(const pair<ProcessId, double> &lhs,
          const pair<ProcessId, double> &rhs)
        {
          return lhs.second < rhs.second;
        }
      };
    }

    //##--------   P u b l i c   m e t h o d s   --------##//

    GlobalProcessControl::GlobalProcessControl() :
      ProcessControl(),
      itsState(UNDEFINED),
      itsWaitId(-1),
      itsFreqStart(0.0),
      itsFreqEnd(0.0),
      itsTimeStart(0),
      itsTimeEnd(0),
      itsChunkStart(0),
      itsChunkSize(0)
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
      	itsStrategy = Strategy(*globalParameterSet());
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

        // Initialize the calibration session.
        string key = ps->getString("BBDB.Key", "default");
        itsCalSession.reset(new CalSession(key,
            ps->getString("BBDB.Name"),
            ps->getString("BBDB.User"),
            ps->getString("BBDB.Password", ""),
            ps->getString("BBDB.Host", "localhost"),
            ps->getString("BBDB.Port", "5432")));

        // Try to become the controller of the session.
        if(!itsCalSession->registerAsControl()) {
          LOG_ERROR_STR("Could not register as control. There may be stale"
            " state in the database for key: " << key);
          return false;
        }

        // Initialize the register and switch the session state to allow workers
        // to register.
        itsCalSession->initWorkerRegister(itsVdsDesc, itsStrategy.useSolver());
        itsCalSession->setState(CalSession::WAITING_FOR_WORKERS);

        // Wait for workers to register.
        while(itsCalSession->slotsAvailable()) {
          sleep(3);
        }

        // All workers have registered. Assign indices sorted on frequency.
        itsCalSession->setState(CalSession::COMPUTING_WORKER_INDEX);
        createWorkerIndex();

        // Determine the frequency range of the observation.
        const ProcessId firstKernel =
          itsCalSession->getWorkerByIndex(CalSession::KERNEL, 0);
        const ProcessId lastKernel =
          itsCalSession->getWorkerByIndex(CalSession::KERNEL,
            itsCalSession->getWorkerCount(CalSession::KERNEL) - 1);
        itsFreqStart = itsCalSession->getGrid(firstKernel)[0]->range().first;
        itsFreqEnd = itsCalSession->getGrid(lastKernel)[0]->range().second;

        LOG_INFO_STR("Observation frequency range: [" << itsFreqStart << ","
          << itsFreqEnd << "]");

        // Determine global time axis and verify consistency across all parts.
        itsGlobalTimeAxis = getGlobalTimeAxis();

        // Apply TimeWindow selection.
        itsTimeStart = 0;
        itsTimeEnd = itsGlobalTimeAxis->size() - 1;

        const vector<string> &window = itsStrategy.getTimeWindow();

        casa::Quantity time;
        if(!window.empty() && casa::MVTime::read(time, window[0])) {
          const pair<size_t, bool> result =
            itsGlobalTimeAxis->find(time.getValue("s"));

          if(result.second) {
            itsTimeStart = result.first;
          }
        }

        if(window.size() > 1 && casa::MVTime::read(time, window[1])) {
          const pair<size_t, bool> result =
            itsGlobalTimeAxis->find(time.getValue("s"), false);

          if(result.first < itsGlobalTimeAxis->size()) {
            itsTimeEnd = result.first;
          }
        }

        itsChunkStart = itsTimeStart;
        itsChunkSize = itsStrategy.getChunkSize();
        if(itsChunkSize == 0) {
          // If chunk size equals 0, take the whole observation as a single
          // chunk.
          itsChunkSize = itsTimeEnd - itsTimeStart + 1;
        }

        LOG_INFO_STR("Selected time range: [" << itsTimeStart << ","
          << itsTimeEnd << "]");
        LOG_INFO_STR("Chunk size: " << itsChunkSize << " timestamp(s)");

        // Switch session state.
        itsCalSession->setState(CalSession::PROCESSING);

        // Send InitializeCommand and wait for a reply from all workers.
        const size_t nWorkers = itsCalSession->getWorkerCount();

        InitializeCommand initCmd(itsStrategy);
        CommandId initId = itsCalSession->postCommand(initCmd);
        LOG_DEBUG_STR("Initialize command has ID: " << initId);

        // Wait for workers to execute initialize command.
        bool ok = false;
        while(!ok) {
          if(itsCalSession->waitForResult()) {
            CommandStatus status = itsCalSession->getCommandStatus(initId);

            if(status.failed > 0) {
              LOG_ERROR_STR("" << status.failed << " worker(s) failed at"
                " initialization");
              itsCalSession->setState(CalSession::FAILED);
              return false;
            }

            ok = (status.finished == nWorkers);
          }
        }

        // Switch the controller to the NEXT_CHUNK state, indicating that the
        // next thing we should do is post a "next chunk" command.
        setState(NEXT_CHUNK);
      }
      catch(Exception& e) {
        LOG_ERROR_STR(e);

        // Best effort attempt to update the session state.
        if(itsCalSession) {
          try {
            itsCalSession->setState(CalSession::FAILED);
          } catch(Exception &e) {
            LOG_ERROR_STR(e);
          } catch(...) {
          }
        }

        return false;
      }

      return true;
    }


    tribool GlobalProcessControl::run()
    {
      LOG_INFO("GlobalProcessControl::run()");

      try {
        switch(itsState) {
          default: {
            LOG_ERROR_STR("Unexpected state: " << showState());
            return false;
            break;
          }

          case NEXT_CHUNK: {
            // Send a "next chunk" command.
            LOG_TRACE_FLOW("State::NEXT_CHUNK");
            ASSERT(itsChunkStart <= itsTimeEnd);

            const double start = itsGlobalTimeAxis->lower(itsChunkStart);
            const double end = itsGlobalTimeAxis->upper(std::min(itsChunkStart
              + itsChunkSize - 1, itsTimeEnd));

            itsChunkStart += itsChunkSize;

            NextChunkCommand cmd(itsFreqStart, itsFreqEnd, start, end);
            itsWaitId =  itsCalSession->postCommand(cmd, CalSession::KERNEL);
            LOG_DEBUG_STR("Next-chunk command has ID: " << itsWaitId);

            setState(NEXT_CHUNK_WAIT);
            break;
          }


          case NEXT_CHUNK_WAIT: {
            // Wait for a "result trigger" form the database. If trigger
            // received within time-out period, fetch new results. If any of the
            // new results contain an OK status to the "next chunk" command,
            // switch to RUN state.
            LOG_TRACE_FLOW("State::NEXT_CHUNK_WAIT");

            if(itsCalSession->waitForResult()) {
              CommandStatus status = itsCalSession->getCommandStatus(itsWaitId);

              if(status.finished > status.failed) {
                setState(RUN);
                itsStrategyIterator = StrategyIterator(itsStrategy);
              }
            }

            break;
          }

          case RUN: {
            // Send the next "step" command and switch to the WAIT state, unless
            // we're at the end of the strategy. In that case we should switch
            // to the NEXT_CHUNK state.
            LOG_TRACE_FLOW("State::RUN");

            if(!itsStrategyIterator.atEnd()) {
              if((*itsStrategyIterator)->type() != "Solve") {
                itsCalSession->postCommand(**itsStrategyIterator,
                  CalSession::KERNEL);
              } else {
                itsCalSession->postCommand(**itsStrategyIterator);
              }

              // Advance to the next command.
              ++itsStrategyIterator;
            } else if(itsChunkStart > itsTimeEnd) {
              setState(FINALIZE);
            } else {
              setState(NEXT_CHUNK);
            }
            break;
          }

          case FINALIZE: {
            // Send "finalize" command. Wait until all local controllers have
            // responded. If all local controllers returned an OK status, mark
            // the strategy as 'done'.
            LOG_TRACE_FLOW("State::FINALIZE");

            itsWaitId = itsCalSession->postCommand(FinalizeCommand());
            LOG_DEBUG_STR("Finalize command has ID: " << itsWaitId);

            setState(FINALIZE_WAIT);
            break;
          }

          case FINALIZE_WAIT: {
            // Wait for a "result trigger" from the database. If trigger
            // received within time-out period, fetch new results. When all
            // "clients" have acknowledged the "finalize" command, we can switch
            // to the QUIT state.
            LOG_TRACE_FLOW("State::FINALIZE_WAIT");

            if(itsCalSession->waitForResult()) {
              CommandStatus status = itsCalSession->getCommandStatus(itsWaitId);

              if(status.finished == itsCalSession->getWorkerCount()) {
                if(status.failed == 0) {
                  itsCalSession->setState(CalSession::DONE);
                } else {
                  itsCalSession->setState(CalSession::FAILED);
                }

                setState(QUIT);
              }
            }
            break;
          }

          case QUIT: {
            LOG_TRACE_FLOW("State::QUIT");
            // Notify ACC that we are done.
            clearRunState();
            break;
          }
        } // switch
      }
      catch(Exception& e) {
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

    Axis::ShPtr GlobalProcessControl::getGlobalTimeAxis() const {
      vector<ProcessId> kernels =
        itsCalSession->getWorkersByType(CalSession::KERNEL);
      ASSERT(kernels.size() > 0);

      Axis::ShPtr globalAxis;
      for(size_t i = 0; i < kernels.size(); ++i)
      {
        Axis::ShPtr localAxis = itsCalSession->getGrid(kernels[i])[1];
        if(!localAxis) {
          THROW(BBSControlException, "Time axis not known for kernel process: "
            << kernels[i]);
        }

        if(globalAxis && globalAxis != localAxis) {
          THROW(CalSessionException, "Time axis inconsistent for kernel"
            " process: " << kernels[i]);
        } else {
          globalAxis = localAxis;
        }
      }

      return globalAxis;
    }


    void GlobalProcessControl::createWorkerIndex()
    {
      vector<ProcessId> kernels =
        itsCalSession->getWorkersByType(CalSession::KERNEL);
      ASSERT(kernels.size() > 0);

      // Sort kernels processes on start frequency.
      vector<pair<ProcessId, double> > index(kernels.size());
      for(size_t i = 0; i < kernels.size(); ++i)
      {
        index[i] = make_pair(kernels[i],
          itsCalSession->getGrid(kernels[i])[0]->lower(0));
      }

      stable_sort(index.begin(), index.end(), LessKernel());

      // Update the worker register.
      for(size_t i = 0; i < index.size(); ++i)
      {
        itsCalSession->setWorkerIndex(index[i].first, i);
      }

      vector<ProcessId> solvers =
        itsCalSession->getWorkersByType(CalSession::SOLVER);

      // Update the worker register.
      for(size_t i = 0; i < solvers.size(); ++i)
      {
        itsCalSession->setWorkerIndex(solvers[i], i);
      }
    }

    void GlobalProcessControl::setState(State state)
    {
      itsState = state;
      LOG_DEBUG_STR("Switching to " << showState() << " state");
    }

    const string& GlobalProcessControl::showState() const
    {
      //# Caution: Always keep this array of strings in sync with the enum
      //#          State that is defined in the header file!
      static const string states[N_State+1] = {
        "NEXT_CHUNK",
        "NEXT_CHUNK_WAIT",
        "RUN",
        "FINALIZE",
        "FINALIZE_WAIT",
        "QUIT",
        "<UNDEFINED>"  //# This should ALWAYS be last !!
      };
      if (UNDEFINED < itsState && itsState < N_State) return states[itsState];
      else return states[N_State];
    }

  } // namespace BBS
} // namespace LOFAR
