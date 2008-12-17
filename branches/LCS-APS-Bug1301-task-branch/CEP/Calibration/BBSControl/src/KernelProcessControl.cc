//#  KernelProcessControl.cc: 
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
//#  Note: This source is read best with tabstop 4.
//#
//#  $Id$

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

#include <BBSControl/KernelProcessControl.h>
#include <BBSControl/CommandQueue.h>
#include <BBSControl/Messages.h>
#include <BBSControl/Step.h>
#include <BBSControl/InitializeCommand.h>
#include <BBSControl/FinalizeCommand.h>
#include <BBSControl/NextChunkCommand.h>
#include <BBSControl/Strategy.h>
#include <BBSControl/Exceptions.h>
#include <BBSControl/StreamUtil.h>

#include <Common/ParameterSet.h>
#include <Common/Exceptions.h>
#include <Blob/BlobStreamable.h>

#include <Common/LofarLogger.h>
#include <Common/Timer.h>
#include <Common/lofar_iomanip.h>
#include <Common/lofar_smartptr.h>

#include <Transport/DH_BlobStreamable.h>
#include <Transport/TH_Socket.h>
#include <Transport/CSConnection.h>

#include <stdlib.h>


namespace LOFAR 
{
  namespace BBS 
  {
    // Forces registration with Object Factory.
    namespace
    {
      InitializeCommand cmd1;
      FinalizeCommand   cmd2;
      NextChunkCommand  cmd3;
    }


    //##----   P u b l i c   m e t h o d s   ----##//
    KernelProcessControl::KernelProcessControl()
      :   ProcessControl(),
          itsState(KernelProcessControl::INIT)
    {
      LOG_TRACE_FLOW(AUTO_FUNCTION_NAME);
    }


    KernelProcessControl::~KernelProcessControl()
    {
      LOG_TRACE_FLOW(AUTO_FUNCTION_NAME);
    }


    tribool KernelProcessControl::define()
    {
      LOG_DEBUG("KernelProcessControl::define()");

      ParameterSet* ps(globalParameterSet());
      ASSERT(ps);

      try {
        string host, port;
        
        host = ps->getString("Solver.Host");
        port = ps->getString("Solver.Port");

        LOG_DEBUG_STR("Defining connection: solver@" << host << ":" << port);
        itsSolver.reset(new BlobStreamableConnection(host, port, Socket::TCP));
      }
      catch(APSException &e) {
        LOG_WARN("No global solver specified in parset, proceeding without"
            " it.");
      }
      
      return true;
    }


    tribool KernelProcessControl::init()
    {
      LOG_DEBUG("KernelProcessControl::init()");

      try {
        uint32 kernelId =
            globalParameterSet()->getUint32("Kernel.Id");

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

        if(itsSolver)
        {
            if(!itsSolver->connect())
            {        
                LOG_ERROR("Unable to connect to global solver.");
                return false;
            }
                            
            // Make our kernel id known to the global solver.
            itsSolver->sendObject(KernelIdMsg(kernelId));
        }

        itsCommandExecutor.reset(new CommandExecutor(kernelId, itsCommandQueue,
            itsSolver));
      }
      catch(Exception& e)
      {
        LOG_ERROR_STR(e);
        return false;
      }
      // All went well.
      return true;
    }


    tribool KernelProcessControl::run()
    {
      LOG_DEBUG("KernelProcessControl::run()");

      switch(itsState)
      {
      case KernelProcessControl::INIT:
        {
          if(!itsCommandQueue->isNewRun(false))
          {
            LOG_ERROR("Need to recover from an aborted run, but recovery has"
              " not been implemented yet.");
            return false;
          }

          LOG_DEBUG("New run, entering RUN state.");
          itsState = KernelProcessControl::RUN;
          break;
        }

      case KernelProcessControl::RUN:
        {
          NextCommandType cmd(itsCommandQueue->getNextCommand());

          if(cmd.first)
          {
            // Execute the command.
            cmd.first->accept(*itsCommandExecutor);
            const CommandResult &result = itsCommandExecutor->getResult();
            SenderId id(SenderId::KERNEL, itsCommandExecutor->getKernelId());

            // Report the result to the global controller.
            itsCommandQueue->addResult(cmd.second, result, id);
            
            // If an error occurred, log a descriptive message and exit.
            if(result.is(CommandResult::ERROR))
            {
              LOG_ERROR_STR("Error executing " << cmd.first->type()
                << " command: " << result.message());
              return false;
            }

            // If the command was a finalize command, log that we are done
            // and exit.
            if(cmd.first->type() == "Finalize")
            {
              LOG_INFO("Run completed succesfully.");
              clearRunState();
            }
          }
          else
            itsState = KernelProcessControl::WAIT;

          break;
        }

      case KernelProcessControl::WAIT:
        {
          LOG_DEBUG("Waiting for next Command...");

          if(itsCommandQueue->
             waitForTrigger(CommandQueue::Trigger::Command))
          {
            itsState = KernelProcessControl::RUN;
          }
          break;
        }

      default:
        {
          THROW(LocalControlException, "Unknown state identifier "
                "encountered");
        }
      }

      return true;
    }


    tribool KernelProcessControl::pause(const string& /*condition*/)
    {
      LOG_TRACE_FLOW(AUTO_FUNCTION_NAME);
      LOG_WARN("Not supported");
      return indeterminate;
    }


    tribool KernelProcessControl::release()
    {
      LOG_INFO("KernelProcessControl::release()");
      LOG_WARN("Not supported");
      /* Here we should properly clean-up; i.e. close open sockets, etc. */
      return indeterminate;
    }


    tribool KernelProcessControl::quit()
    {
      LOG_DEBUG("KernelProcessControl::quit()");
      return true;
    }


    tribool KernelProcessControl::snapshot(const string& /*destination*/)
    {
      LOG_TRACE_FLOW(AUTO_FUNCTION_NAME);
      LOG_WARN("Not supported");
      return indeterminate;
    }


    tribool KernelProcessControl::recover(const string& /*source*/)
    {
      LOG_TRACE_FLOW(AUTO_FUNCTION_NAME);
      LOG_WARN("Not supported");
      return indeterminate;
    }


    tribool KernelProcessControl::reinit(const string& /*configID*/)
    {
      LOG_TRACE_FLOW(AUTO_FUNCTION_NAME);
      LOG_WARN("Not supported");
      return indeterminate;
    }


    string KernelProcessControl::askInfo(const string& /*keylist*/)
    {
      LOG_TRACE_FLOW(AUTO_FUNCTION_NAME);
      return string("");
    }

  } // namespace BBS

} // namespace LOFAR
