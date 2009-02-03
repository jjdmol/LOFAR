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
#include <BBSControl/Messages.h>
#include <BBSControl/StreamUtil.h>
#include <BBSControl/Exceptions.h>
#include <BBSControl/Types.h>

#include <BBSControl/InitializeCommand.h>
#include <BBSControl/FinalizeCommand.h>
#include <BBSControl/NextChunkCommand.h>
#include <BBSControl/RecoverCommand.h>
#include <BBSControl/SynchronizeCommand.h>

#include <BBSControl/MultiStep.h>
#include <BBSControl/PredictStep.h>
#include <BBSControl/SubtractStep.h>
#include <BBSControl/AddStep.h>
#include <BBSControl/CorrectStep.h>
#include <BBSControl/SolveStep.h>
#include <BBSControl/ShiftStep.h>
#include <BBSControl/RefitStep.h>
#include <BBSControl/NoiseStep.h>

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
      InitializeCommand cmd0;
      NextChunkCommand  cmd1;
      FinalizeCommand   cmd2;
    }

    //##----   P u b l i c   m e t h o d s   ----##//

    SolverProcessControl::SolverProcessControl() :
      ProcessControl(),
      itsState(UNDEFINED)
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
      return true;
    }


    tribool SolverProcessControl::init()
    {
      LOG_INFO("SolverProcessControl::init()");
      try {
        const ParameterSet *ps = globalParameterSet();
        ASSERT(ps);
        
        // Initialize the calibration session.
        string key = ps->getString("BBDB.Key", "default");
        itsCalSession.reset(new CalSession(key,
          ps->getString("BBDB.Name"),
          ps->getString("BBDB.User"),
          ps->getString("BBDB.Password", ""),
          ps->getString("BBDB.Host", "localhost"),
          ps->getString("BBDB.Port", "5432")));
            
        // Initialize a TCP listen socket that will accept incoming kernel
        // connections.
        int32 backlog = ps->getInt32("ConnectionBacklog", 10);

        // Set socket name.
        itsSocket.setName("Solver");
        
        // Get range of ports to test from parset, or default to [6500, 6599].
        vector<uint> range = ps->getUintVector("PortRange", vector<uint>());
        uint portLow = range.size() > 0 ? range[0] : 6500;
        uint portHigh = range.size() > 1 ? range[1] : 6599;

        // Try to bind to a port from the specified range.
        uint port = 0;
        int32 sockErrno = Socket::BIND;
        for(port = portLow; port <= portHigh; ++port) {
          // Socket::initServer takes port as a string.
          ostringstream tmp;
          tmp << port;

          // Try to bind socket.
          sockErrno = itsSocket.initServer(tmp.str(), Socket::TCP, backlog);
          if(sockErrno != Socket::BIND) {
            break;
          }
        }

        if(sockErrno != Socket::SK_OK) {
          LOG_ERROR_STR("Unable to initialize server socket using a port from"
            " the range [" << portLow << "," << portHigh << "]");
          return false;
        }
        
        LOG_INFO_STR("Listening on port: " << port);
        
        // Poll until Control is ready to accept workers.
        LOG_INFO_STR("Waiting for Control...");
        while(itsCalSession->getState() == CalSession::WAITING_FOR_CONTROL) {
          sleep(3);
        }

        // Try to register as solver.
        if(!itsCalSession->registerAsSolver(port)) {
          LOG_ERROR("Registration denied.");
          return false;
        }
        
        LOG_INFO_STR("Registration OK.");
        setState(RUN);
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
            LOG_ERROR_STR("Unexpected state: " << showState());
            return false;
            break;
          }
          
          case WAIT: {
            // Wait for a command. Note that this call falls through whenever
            // a new command is inserted.
            if(itsCalSession->waitForCommand())
            {
              setState(RUN);
            }
            break;
          }

          case RUN: {
            pair<CommandId, shared_ptr<Command> > command =
              itsCalSession->getCommand();

            // Did we receive a valid command?
            if(command.second) {
              LOG_DEBUG_STR("Executing a " << command.second->type()
                << "command.");

              // Try to execute the command.
              CommandResult result = command.second->accept(*this);

              // Report the result to the global controller.
              itsCalSession->postResult(command.first, result);
      
              // If an error occurred, log a descriptive message and exit.
              if(result.is(CommandResult::ERROR)) {
                LOG_ERROR_STR("Error executing " << command.second->type()
                  << " command: " << result.message());
                return false;
              }

              // If the command was a finalize command, log that we are done
              // and exit.
              if(command.second->type() == "Finalize") {
                LOG_INFO("Run completed succesfully.");
                clearRunState();
              }
            }
            else {
              LOG_DEBUG("No queued commands found for this process; will"
                " continue waiting.");
              setState(WAIT);
            }
            break;
          }
        } // switch(itsState)        
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
      setState(UNDEFINED);
      /* Here we should properly close all connections to the outside world.
         I.e. our connection to the shared state, and the connections
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


    CommandResult SolverProcessControl::visit(const InitializeCommand&)
    {
      LOG_TRACE_FLOW(AUTO_FUNCTION_NAME);

      // Create a TCP socket accepted by the listening socket.
      shared_ptr<BlobStreamableConnection> connection;

      const size_t nKernels = itsCalSession->getWorkerCount(CalSession::KERNEL);

      //  Wait for all kernels to connect.
      LOG_DEBUG_STR("Waiting for " << nKernels << " kernel(s) to connect...");

      itsKernels.resize(nKernels);
      for(uint i = 0; i < nKernels; ++i) {
        connection.reset(new BlobStreamableConnection(itsSocket.accept()));
        if(!connection->connect()) {
          THROW(IOException, "Unable to connect to kernel");
        }

        scoped_ptr<const ProcessIdMsg>
          msg(dynamic_cast<ProcessIdMsg*>(connection->recvObject()));
        if(!msg) {
          THROW(SolverControlException, "Protocol error. Expected a"
            " ProcessIdMsg");
        }
          
        if(!itsCalSession->isKernel(msg->getProcessId())) {
          connection.reset();
          THROW(SolverControlException, "Process " << msg->getProcessId()
            << "is not a registered kernel process; disconnected");
        }
  
        KernelIndex index = itsCalSession->getIndex(msg->getProcessId());

        try {
          itsKernels.at(index) = KernelConnection(connection, index);
        } catch (std::out_of_range&) {
          connection.reset();
          THROW(SolverControlException, "Kernel index (" << index << ") out of"
            " range; disconnected");
        }

        LOG_DEBUG_STR("Kernel " << i+1 << " of " << nKernels << " connected"
          " (index=" << index << ")");
      }

      return CommandResult(CommandResult::OK, "Ok.");
    }

    CommandResult SolverProcessControl::visit(const FinalizeCommand&)
    {
        LOG_TRACE_FLOW(AUTO_FUNCTION_NAME);
        return CommandResult(CommandResult::OK, "Ok.");
    }

    CommandResult SolverProcessControl::visit(const NextChunkCommand &command)
    {
        LOG_TRACE_FLOW(AUTO_FUNCTION_NAME);
        return unsupported(dynamic_cast<const Command&>(command));
    }

    CommandResult SolverProcessControl::visit(const RecoverCommand &command)
    {
        LOG_TRACE_FLOW(AUTO_FUNCTION_NAME);
        return unsupported(command);
    }

    CommandResult SolverProcessControl::visit(const SynchronizeCommand &command)
    {
        LOG_TRACE_FLOW(AUTO_FUNCTION_NAME);
        return unsupported(command);
    }

    CommandResult SolverProcessControl::visit(const MultiStep &command)
    {
        LOG_TRACE_FLOW(AUTO_FUNCTION_NAME);
        return unsupported(command);
    }

    CommandResult SolverProcessControl::visit(const PredictStep &command)
    {
        LOG_TRACE_FLOW(AUTO_FUNCTION_NAME);
        return unsupported(command);
    }

    CommandResult SolverProcessControl::visit(const SubtractStep &command)
    {
        LOG_TRACE_FLOW(AUTO_FUNCTION_NAME);
        return unsupported(command);
    }

    CommandResult SolverProcessControl::visit(const AddStep &command)
    {
        LOG_TRACE_FLOW(AUTO_FUNCTION_NAME);
        return unsupported(command);
    }

    CommandResult SolverProcessControl::visit(const CorrectStep &command)
    {
        LOG_TRACE_FLOW(AUTO_FUNCTION_NAME);
        return unsupported(command);
    }

    CommandResult SolverProcessControl::visit(const SolveStep &command)
    {
      LOG_TRACE_FLOW(AUTO_FUNCTION_NAME);
      
      // Initialize a solve task for each calibration group.
      setSolveTasks(command.calibrationGroups(), command.solverOptions());

      // Call the run() method on each kernel group. In the current
      // implementation, this is a serialized operation. Once we run each
      // kernel group in its own thread, we can parallellize
      // things. Threads will also make it possible to return swiftly from
      // the current method, so that we can promptly react to ACC command.
      //
      // [Q] Should we let run() return a bool or do we handle error
      // conditions with exceptions. I think the former (bools) is a
      // better choice, since we're planning on running each task in a
      // separate thread, and it is usually a bad thing to handle an
      // exception in a different thread than in which it was thrown.
      bool done = false;
      while(!done)
      {
        done = true;
        for(uint i = 0; i < itsSolveTasks.size(); ++i) {
          done = itsSolveTasks[i].run() && done;
        }
      }
      
      return CommandResult(CommandResult::OK, "Ok.");
    }

    CommandResult SolverProcessControl::visit(const ShiftStep &command)
    {
        LOG_TRACE_FLOW(AUTO_FUNCTION_NAME);
        return unsupported(command);
    }

    CommandResult SolverProcessControl::visit(const RefitStep &command)
    {
        LOG_TRACE_FLOW(AUTO_FUNCTION_NAME);
        return unsupported(command);
    }

    CommandResult SolverProcessControl::visit(const NoiseStep &command)
    {
        LOG_TRACE_FLOW(AUTO_FUNCTION_NAME);
        return unsupported(command);
    }


    //##--------   P r i v a t e   m e t h o d s   --------##//

    CommandResult SolverProcessControl::unsupported(const Command &command)
      const
    {
      ostringstream message;
      message << "Received unsupported command (" << command.type() << ")";
      return CommandResult(CommandResult::ERROR, message.str());
    }

    void SolverProcessControl::setState(State state)
    {
      itsState = state;
      LOG_DEBUG_STR("Switching to " << showState() << " state");
    }

    const string& SolverProcessControl::showState() const
    {
      //# Caution: Always keep this array of strings in sync with the enum
      //#          State that is defined in the header file!
      static const string states[N_State+1] = {
        "UNDEFINED",
        "WAIT",
        "RUN",
        "<UNKNOWN>"  //# This should ALWAYS be last !!
      };
      if(UNDEFINED < itsState && itsState < N_State) return states[itsState];
      else return states[N_State];
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
        itsSolveTasks.push_back(SolveTask(vector<KernelConnection>(beg, end),
          options));
        beg = end;
      }
    }

  } // namespace BBS

} // namespace LOFAR
