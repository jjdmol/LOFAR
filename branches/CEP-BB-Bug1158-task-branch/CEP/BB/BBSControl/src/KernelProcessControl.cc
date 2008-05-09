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
#include <BBSControl/Step.h>
#include <BBSControl/InitializeCommand.h>
#include <BBSControl/FinalizeCommand.h>
#include <BBSControl/NextChunkCommand.h>
#include <BBSControl/Strategy.h>
#include <BBSControl/Exceptions.h>
#include <BBSControl/StreamUtil.h>

#include <APS/ParameterSet.h>
#include <Blob/BlobStreamable.h>

#include <Common/LofarLogger.h>
#include <Common/Timer.h>
#include <Common/lofar_iomanip.h>
#include <Common/lofar_smartptr.h>

#include <Transport/DH_BlobStreamable.h>
#include <Transport/TH_Socket.h>
#include <Transport/CSConnection.h>

#include <stdlib.h>

using namespace LOFAR::ACC::APS;

namespace LOFAR 
{
  namespace BBS 
  {
    // Forces registration with Object Factory.
    namespace
    {
      InitializeCommand cmd1;
      FinalizeCommand cmd2;
      NextChunkCommand cmd3;
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

      try
      {
        ParameterSet* ps(globalParameterSet());
        ASSERT(ps);

        string host;
        string port;

        // Should we connect to a global solver?
        bool isGlobal(ps->getBool("Solver.Global"));

        if (isGlobal) {
          host = ps->getString("Solver.Host");
          port = ps->getString("Solver.Port");
        }
        else {
          ostringstream oss;
          oss << "=Solver_" << getenv("USER") << ps->getInt32("SolverId");
          host = "localhost";
          port = oss.str();
        }

        LOG_DEBUG_STR("Defining connection: solver@" << host << ":" << port);
        itsSolverConnection.reset
          (new BlobStreamableConnection
           (host, port, (isGlobal ? Socket::TCP : Socket::LOCAL)));

      }
      catch(Exception& e) {
        LOG_ERROR_STR(e);
        return false;
      }

      return true;
    }


    tribool KernelProcessControl::init()
    {
      LOG_DEBUG("KernelProcessControl::init()");

      try {
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

        LOG_DEBUG("Trying to connect to solver");
        if(!itsSolverConnection->connect())
        {
          LOG_ERROR("+ could not connect to solver");
          return false;
        }
        LOG_DEBUG("+ ok");

        itsCommandExecutor.reset
          (new CommandExecutor(itsCommandQueue, itsSolverConnection));
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
            LOG_ERROR("Need to recover from an aborted run, but "
                      "recovery has not yet been implemented.");
            return false;
          }

          LOG_DEBUG("New run, entering RUN state.");
          //                itsState = KernelProcessControl::FIRST_RUN;
          itsState = KernelProcessControl::RUN;
          break;
        }

      case KernelProcessControl::RUN:
        {
          NextCommandType cmd(itsCommandQueue->getNextCommand());

          if(cmd.first)
          {
            cmd.first->accept(*itsCommandExecutor);
            itsCommandQueue->addResult(cmd.second,
                                       itsCommandExecutor->getResult());

            if(cmd.first->type() == "Finalize")
              return false;
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
      return false;
    }


    tribool KernelProcessControl::release()
    {
      LOG_INFO("KernelProcessControl::release()");
      LOG_WARN("Not implemented yet");
      /* Here we should properly clean-up; i.e. close open sockets, etc. */
      return true;
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
      return false;
    }


    tribool KernelProcessControl::recover(const string& /*source*/)
    {
      LOG_TRACE_FLOW(AUTO_FUNCTION_NAME);
      LOG_WARN("Not supported");
      return false;
    }


    tribool KernelProcessControl::reinit(const string& /*configID*/)
    {
      LOG_TRACE_FLOW(AUTO_FUNCTION_NAME);
      LOG_WARN("Not supported");
      return false;
    }


    string KernelProcessControl::askInfo(const string& /*keylist*/)
    {
      LOG_TRACE_FLOW(AUTO_FUNCTION_NAME);
      return string("");
    }

  } // namespace BBS

} // namespace LOFAR
