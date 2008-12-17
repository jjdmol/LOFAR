//#  SolverProcessControl.h: Implementation of the ProcessControl
//#     interface for the BBS solver component.
//#
//#  Copyright (C) 2004
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
//#  Note: This source is read best with tabstop 4.
//#
//#  $Id$

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

#include <BBSControl/SolverProcessControl.h>
#include <BBSControl/BlobStreamableConnection.h>
#include <BBSControl/CommandQueue.h>
#include <BBSControl/Messages.h>
#include <BBSControl/InitializeCommand.h>
#include <BBSControl/NextChunkCommand.h>
#include <BBSControl/FinalizeCommand.h>
#include <BBSControl/StreamUtil.h>
#include <BBSControl/Exceptions.h>
#include <BBSControl/Types.h>

#include <Common/ParameterSet.h>
#include <Blob/BlobStreamable.h>
#include <Common/Exceptions.h>
#include <Common/LofarLogger.h>
#include <Common/lofar_iomanip.h>
#include <Common/lofar_numeric.h>
#include <Common/lofar_smartptr.h>

#include <stdlib.h>

namespace LOFAR
{

  namespace BBS
  {
    using LOFAR::operator<<;

    namespace
    {
      InitializeCommand initCmd;
      NextChunkCommand  nextChunkCmd;
      FinalizeCommand   finalizeCmd;
    }

    //##----   P u b l i c   m e t h o d s   ----##//

    SolverProcessControl::SolverProcessControl() :
      ProcessControl(),
      itsState(UNDEFINED),
      itsNrKernels(0)
    {
      LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW, "");
    }


    SolverProcessControl::~SolverProcessControl()
    {
      LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW, "");
    }


    tribool SolverProcessControl::define()
    {
      LOG_INFO("SolverProcessControl::define()");
      try {
        itsNrKernels = globalParameterSet()->getUint32("Solver.KernelCount");
        itsSenderId = 
          SenderId(SenderId::SOLVER,
            globalParameterSet()->getUint32("Solver.Id"));
      }
      catch(Exception& e) {
        LOG_ERROR_STR(e);
        return false;
      }
      return true;
    }


    tribool SolverProcessControl::init()
    {
      LOG_INFO("SolverProcessControl::init()");
      try {
        // Socket acceptor. In the global case, it will become a TCP listen
        // socket; in the local case a Unix domain socket. The acceptor can be
        // a stack object, since we don't need it anymore, once all kernels
        // have connected.
        Socket acceptor("SolverProcessControl");

        // Socket connector. In the global case, this will become a TCP socket
        // accepted by the listening socket; in the local case, a Unix domain
        // socket. The connector is a heap object, since we need it in the
        // run() method.
        shared_ptr<BlobStreamableConnection> connector;

        if (isGlobal()) {
          LOG_DEBUG("This is a global solver.");

          // Create a new CommandQueue. This will open a connection to the
          // blackboard database.
          itsCommandQueue.reset
            (new CommandQueue(globalParameterSet()->getString("BBDB.Name"),
                              globalParameterSet()->getString("BBDB.Username"),
                              globalParameterSet()->getString("BBDB.Host"),
                              globalParameterSet()->getString("BBDB.Port")));

          // Register for the "command" trigger, which fires when a new
          // command is posted to the blackboard database.
          itsCommandQueue->registerTrigger(CommandQueue::Trigger::Command);

          // Create a TCP listen socket that will accept incoming kernel
          // connections.
          acceptor.initServer(globalParameterSet()->getString("Solver.Port"), 
                              Socket::TCP);
        }
        else {
          LOG_DEBUG("This is a local solver.");

          // Create a Unix domain socket to connect to "our" kernel.
          ostringstream oss;
          oss << "=Solver_" << getenv("USER") 
              << globalParameterSet()->getInt32("Solver.Id");
          acceptor.initServer(oss.str(), Socket::LOCAL);
        }

        //  Wait for all kernels to connect.
        LOG_DEBUG_STR("Waiting for " << itsNrKernels << 
                      " kernel(s) to connect ...");
        itsKernels.resize(itsNrKernels);

        for (uint i = 0; i < itsNrKernels; ++i) {
          connector.reset(new BlobStreamableConnection(acceptor.accept()));
          if (!connector->connect()) {
            THROW (IOException, "Failed to connect kernel");
          }

          scoped_ptr<const KernelIdMsg>
            msg(dynamic_cast<KernelIdMsg*>(connector->recvObject()));
          if (!msg) {
            THROW (SolverControlException, 
                   "Illegal message. Expected a KernelIdMsg");
          }

          KernelId id(msg->getKernelId());
          try {
            itsKernels.at(id) = KernelConnection(connector, id);
          } catch (std::out_of_range&) {
            connector.reset();
            LOG_ERROR_STR("Kernel ID (" << id << ") out of range. "
                          "Disconnected kernel");
          }

          LOG_DEBUG_STR("Kernel " << i+1 << " of " << itsNrKernels << 
                        " connected (id=" << id << ")");
        }

        // Switch to GET_COMMAND state, indicating that we're ready to receive
        // commands.
        setState(GET_COMMAND);
      }

      catch (Exception& e) {
        LOG_ERROR_STR(e);
        return false;
      }

      // All went well.
      return true;
    }


    tribool SolverProcessControl::run()
    {
      LOG_INFO("SolverProcessControl::run()");

      try {
        switch(itsState) {

        default: {
          LOG_ERROR("RunState UNKNOWN: this is a program logic error!");
          return false;
          break;
        }

        case UNDEFINED: {
          LOG_WARN("RunState UNDEFINED; init() must be called first!");
          return false;
          break;
        }

        case WAIT: {
          LOG_TRACE_FLOW_STR("RunState::" << showState());
          LOG_DEBUG("Waiting for command trigger ...");
          if (itsCommandQueue->
              waitForTrigger(CommandQueue::Trigger::Command)) {
            setState(GET_COMMAND);
          }
          break;
        }

        case GET_COMMAND: {
          LOG_TRACE_FLOW_STR("RunState::" << showState());
          LOG_DEBUG("Retrieving command ...");
          itsCommand = itsCommandQueue->getNextCommand();

          if(itsCommand.first)
          {
            // If Command is a SolveStep, we should initiate a solve operation.
            shared_ptr<const SolveStep> solveStep = 
              dynamic_pointer_cast<const SolveStep>(itsCommand.first);
            if (solveStep) {
              setSolveTasks(solveStep->calibrationGroups(), 
                            solveStep->solverOptions());
              setState(SOLVE);
            } 
            else {
              itsCommandQueue->
                addResult(itsCommand.second, CommandResult::OK, itsSenderId);
              // If we've received a "finalize" command, we should quit.
              if (dynamic_pointer_cast<const FinalizeCommand>(itsCommand.first))
                setState(QUIT);
            }
          } 
          else {
            LOG_DEBUG("Received empty command. Ignored");
            setState(WAIT);
          }
          break;
        }

        case SOLVE: {
          LOG_TRACE_FLOW_STR("RunState::" << showState());

          // Call the run() method on each kernel group. In the current
          // implementation, this is a serialized operation. Once we run each
          // kernel group in its own thread, we can parallellize
          // things. Threads will also make it possible to return swiftly from
          // the current method, so that we can promptly react to ACC
          // commands.
          //
          // [Q] Should we let run() return a bool or do we handle error
          // conditions with exceptions. I think the former (bools) is a
          // better choice, since we're planning on running each task in a
          // separate thread, and it is usually a bad thing to handle an
          // exception in a different thread than in which it was thrown.
          bool done = true;
          for (uint i = 0; i < itsSolveTasks.size(); ++i) {
            done = itsSolveTasks[i].run() && done;
          }
          // OK, all SolveTasks are done; we can post the result.
          if(done) {
            itsCommandQueue->
              addResult(itsCommand.second, CommandResult::OK, itsSenderId);
            setState(GET_COMMAND);
          }
          break;
        }

        case QUIT: {
          LOG_TRACE_FLOW_STR("RunState::" << showState());
          clearRunState();
        }

        } // switch
      }
      catch(Exception& e) {
        LOG_ERROR_STR(e);
        return false;
      }

      return true;
    }


    tribool SolverProcessControl::pause(const string& /*condition*/)
    {
      LOG_INFO("SolverProcessControl::pause()");
      LOG_WARN("Not supported");
      return indeterminate;
    }


    tribool SolverProcessControl::release()
    {
      LOG_INFO("SolverProcessControl::release()");
      itsState = UNDEFINED;
      /* Here we should properly close all connections to the outside world.
         I.e. our connection to the command queue, and the connections
         that we've accepted from the kernels.
      */
      return indeterminate;
    }


    tribool SolverProcessControl::quit()
    {
      LOG_INFO("SolverProcessControl::quit()");
      return true;
    }


    tribool SolverProcessControl::snapshot(const string& /*destination*/)
    {
      LOG_INFO("SolverProcessControl::snapshot()");
      LOG_WARN("Not supported");
      return indeterminate;
    }


    tribool SolverProcessControl::recover(const string& /*source*/)
    {
      LOG_INFO("SolverProcessControl::recover()");
      LOG_WARN("Not supported");
      return indeterminate;
    }


    tribool SolverProcessControl::reinit(const string& /*configID*/)
    {
      LOG_INFO("SolverProcessControl::reinit()");
      LOG_WARN("Not supported");
      return indeterminate;
    }


    std::string SolverProcessControl::askInfo(const string& /*keylist*/)
    {
      LOG_INFO("SolverProcessControl::askInfo()");
      LOG_WARN("Not supported");
      return string();
    }


    //##--------   P r i v a t e   m e t h o d s   --------##//

    bool SolverProcessControl::isGlobal() const
    {
      return true;
      return itsNrKernels > 1;
    }


    void SolverProcessControl::setState(RunState state) 
    {
      itsState = state;
      LOG_DEBUG_STR("Switching to " << showState() << " state");
    }


    const string& SolverProcessControl::showState() const
    {
      //# Caution: Always keep this array of strings in sync with the enum
      //#          State that is defined in the header file!
      static const string states[N_States+1] = {
        "WAIT",
        "GET_COMMAND",
        "SOLVE",
        "QUIT",
        "<UNDEFINED>"  //# This should ALWAYS be last !!
      };
      if (UNDEFINED < itsState && itsState < N_States) return states[itsState];
      else return states[N_States];
    }


    void SolverProcessControl::setSolveTasks(const vector<uint>& groups,
        const SolverOptions& options)
    {
      LOG_TRACE_LIFETIME(TRACE_LEVEL_COND, "");
      LOG_DEBUG_STR("Kernel groups: " << groups);

      // Sanity check
      if (itsKernels.size() < accumulate(groups.begin(), groups.end(), 0U)) {
        THROW (SolverControlException, 
               "Sum of kernels in subgroups exceeds total number of kernels");
      }

      // Initialization
      itsSolveTasks.clear();
      vector<KernelConnection>::const_iterator beg(itsKernels.begin());
      vector<KernelConnection>::const_iterator end(beg);

      // Create kernel groups, passing the correct subrange of kernel
      // connections to each kernel group.
      for (uint i = 0; i < groups.size(); ++i) {
        advance(end, groups[i]);
        itsSolveTasks.push_back
          (SolveTask(vector<KernelConnection>(beg, end), options));
        beg = end;
      }
    }

  } // namespace BBS

} // namespace LOFAR
