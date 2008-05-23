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
#include <BBSControl/StreamUtil.h>
#include <BBSControl/Exceptions.h>

#include <APS/ParameterSet.h>
#include <Blob/BlobStreamable.h>
#include <Common/Exceptions.h>
#include <Common/LofarLogger.h>
#include <Common/lofar_iomanip.h>
#include <Common/lofar_numeric.h>
#include <Common/lofar_smartptr.h>

#include <stdlib.h>

namespace LOFAR
{
  using namespace ACC::APS;

  namespace BBS
  {
    using LOFAR::operator<<;

    namespace
    {
      InitializeCommand initCmd;
      NextChunkCommand  nextChunkCmd;
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
        // Get the number of kernels that will connect 
        itsNrKernels = globalParameterSet()->getUint32("Solver.nrKernels");
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
            (new CommandQueue(globalParameterSet()->getString("BBDB.DBName"),
                              globalParameterSet()->getString("BBDB.UserName"),
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

          LOG_DEBUG_STR("Kernel " << i << " of " << itsNrKernels << 
                        " connected (id=" << id << ")");
        }

        // Switch to IDLE state, indicating that we're ready to receive
        // commands. Note that we will only react to Solve commands; others
        // will be silently ignored.
        itsState = IDLE;
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

        default:
          LOG_ERROR("ProcessState UNKNOWN: this is a program logic error!");
          return false;
          break;

        case UNDEFINED:
          LOG_WARN("ProcessState UNDEFINED; init() must be called first!");
          return false;
          break;

        case IDLE:
          LOG_TRACE_FLOW("ProcessState::IDLE");

          LOG_DEBUG("Waiting for command trigger ...");
          if (itsCommandQueue->
              waitForTrigger(CommandQueue::Trigger::Command)) {
            // Get the next command.
            const NextCommandType& nextCmd = itsCommandQueue->getNextCommand();
            shared_ptr<const Command> cmd = nextCmd.first;
            const CommandId cmdId = nextCmd.second;

            itsSolveStep = dynamic_pointer_cast<const SolveStep>(cmd);
            if (itsSolveStep) {
              // It's a SolveStep. Setup kernel groups and change state.
              setSolveTasks(itsSolveStep->kernelGroups());
              itsState = SOLVING;
            }
            else {
              // Not a SolveStep, send "Ok" result to command queue
              // immediately.
              itsCommandQueue->addResult(cmdId, CommandResult::OK);
            }
          }
          break;
          
        case SOLVING:
          LOG_TRACE_FLOW("ProcessState::SOLVING");

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
          // exception in a different thread than in wich it was thrown.
          bool done = true;
          for (uint i = 0; i < itsSolveTasks.size(); ++i) {
            done = itsSolveTasks[i].run() && done;
          }
          
          // OK, all SolveTasks are done (since we're currently running one
          // thread!!). We can change state.
          if(done) {
            itsState = IDLE;
          }
          break;

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
      return false;
    }


    tribool SolverProcessControl::release()
    {
      LOG_INFO("SolverProcessControl::release()");
      itsState = UNDEFINED;
      /* Here we should properly close all connections to the outside world.
         I.e. our connection to the command queue, and the connections
         that we've accepted from the kernels.
      */
      return true;
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
      return false;
    }


    tribool SolverProcessControl::recover(const string& /*source*/)
    {
      LOG_INFO("SolverProcessControl::recover()");
      LOG_WARN("Not supported");
      return false;
    }


    tribool SolverProcessControl::reinit(const string& /*configID*/)
    {
      LOG_INFO("SolverProcessControl::reinit()");
      LOG_WARN("Not supported");
      return false;
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


#if 0
    void SolverProcessControl::setKernelGroups(const vector<uint>& groups)
    {
      LOG_TRACE_LIFETIME(TRACE_LEVEL_COND, "");
      LOG_DEBUG_STR("Kernel groups: " << groups);
      itsKernelGroups.clear();
      itsKernelGroupIds.clear();
      for (uint i = 0; i < groups.size(); ++i) {
        itsKernelGroups.push_back(KernelGroup(groups[i]));
        for (uint j = 0; j < groups[i]; ++j) {
          itsKernelGroupIds.push_back(i);
        }
      }
      // The sum of the number of kernels in each group must match the total
      // number of kernels.
      if (itsKernelGroupIds.size() != itsNrKernels) {
        THROW (SolverControlException, "Number of kernels in kernel groups ("
               << itsKernelGroupIds.size() 
               << ") does not match total number of kernels (" 
               << itsNrKernels << ")");
      }
    }
#endif

    void SolverProcessControl::setSolveTasks(const vector<uint>& groups)
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
          (SolveTask(vector<KernelConnection>(beg, end)));
        beg = end;
      }
    }

  } // namespace BBS

} // namespace LOFAR
