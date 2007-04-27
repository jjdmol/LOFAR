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
//#include <BBSControl/Command.h>
#include <BBSControl/BBSStep.h>
#include <BBSControl/InitializeCommand.h>
#include <BBSControl/NextChunkCommand.h>
#include <BBSControl/BBSStrategy.h>
/*
#include <BBSControl/BBSPredictStep.h>
#include <BBSControl/BBSSubtractStep.h>
#include <BBSControl/BBSCorrectStep.h>
#include <BBSControl/BBSSolveStep.h>
#include <BBSControl/BBSSubtractStep.h>
*/
#include <BBSControl/Exceptions.h>

//#include <BBSKernel/Solver.h>
#include <BBSKernel/BBSStatus.h>

#include <APS/ParameterSet.h>
#include <Blob/BlobStreamable.h>

#include <Common/LofarLogger.h>
#include <Common/StreamUtil.h>
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
    //##----   P u b l i c   m e t h o d s   ----##//
    KernelProcessControl::KernelProcessControl() :
        ProcessControl(),
        itsState(KernelProcessControl::INIT)
    {
        LOG_TRACE_FLOW(AUTO_FUNCTION_NAME);
    }


    tribool KernelProcessControl::define()
    {
        LOG_DEBUG("KernelProcessControl::define()");

        try {
/*
            itsControllerConnection.reset(new BlobStreamableConnection(
                    globalParameterSet()->getString("Controller.Host"),
                    globalParameterSet()->getString("Controller.Port")));
*/
            
            char *user = getenv("USER");
            ASSERT(user);
            string userString(user);
            itsSolverConnection.reset(new BlobStreamableConnection(
                "localhost",
                "=Solver_" + userString,
                Socket::LOCAL));
        }
        catch(Exception& e)
        {
            LOG_ERROR_STR(e);
            return false;
        }

        return true;
    }


    tribool KernelProcessControl::init()
    {
        LOG_DEBUG("KernelProcessControl::init()");

        try {
/*
            LOG_DEBUG_STR("Trying to connect to controller@"
                << globalParameterSet()->getString("Controller.Host") << ":"
                << globalParameterSet()->getString("Controller.Port"   )
                << "...");

            if(!itsControllerConnection->connect())
            {
                LOG_ERROR("+ could not connect to controller");
                return false;
            }
            LOG_DEBUG("+ ok");
*/

            itsCommandQueue.reset(new CommandQueue(
                globalParameterSet()->getString("BBDB.DBName")));

            LOG_DEBUG("Trying to connect to solver@localhost");
            if(!itsSolverConnection->connect())
            {
                LOG_ERROR("+ could not connect to solver");
                return false;
            }
            LOG_DEBUG("+ ok");
            
            itsCommandExecutor.reset(new CommandExecutor(itsCommandQueue,
                itsSolverConnection));
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
                    LOG_ERROR("Need to recover from an aborted run, but \
                        recovery has not yet been implemented.");
                    return false;
                }

                LOG_DEBUG("New run, entering RUN state.");
                itsState = KernelProcessControl::FIRST_RUN;
                break;
                }

            case KernelProcessControl::FIRST_RUN:
                {
                scoped_ptr<const BBSStrategy>strategy(
                    itsCommandQueue->getStrategy());

                if(!strategy)
                {
                    sleep(3);
                    break;
                }

                {
                    LOG_DEBUG("Executing an implicit INITIALIZE command.");
                    InitializeCommand cmd;
                    cmd.accept(*itsCommandExecutor);
                }
                {
                    LOG_DEBUG("Executing an implicit NEXT_CHUNK command.");
                    NextChunkCommand cmd;
                    cmd.accept(*itsCommandExecutor);
                }
                itsState = KernelProcessControl::RUN;
                break;
                }

            case KernelProcessControl::RUN:
                {
                scoped_ptr<const Command> cmd(itsCommandQueue->getNextCommand());

                if(cmd)
                {
                    LOG_DEBUG("Received command, remaining in RUN state.");

                    cmd->accept(*itsCommandExecutor);
                }
                else
                {
                    LOG_DEBUG("Command Queue empty, entering WAIT state.");
                    itsState = KernelProcessControl::WAIT;
                }
                break;
                }

            case KernelProcessControl::WAIT:
                {
                /* await notification here */
                LOG_DEBUG("WAIT state not yet implemented, returning to \
                    RUN state.");
                sleep(3);
                itsState = KernelProcessControl::RUN;
                break;
                }

            default:
                {
                THROW(LocalControlException, "Unknown state identifier \
                    encountered");
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


    std::string KernelProcessControl::askInfo(const string& /*keylist*/)
    {
        LOG_TRACE_FLOW(AUTO_FUNCTION_NAME);
        return std::string("");
    }

/*
    //##----   P r i v a t e   m e t h o d s   ----##//
    bool KernelProcessControl::dispatch(const BlobStreamable *message)
    {
        // If the message contains a `strategy', handle the `strategy'.
        const BBSStrategy *strategy = dynamic_cast<const BBSStrategy*>(message);
        if(strategy)
        {
            if(handle(strategy))
            {
                itsControllerConnection->sendObject(BBSStatus(BBSStatus::OK));
                return true;
            }
            else
            {
                
itsControllerConnection->sendObject(BBSStatus(BBSStatus::ERROR));                
return false;            }
        }

        // If the message contains a `step', handle the `step'.
        const BBSStep *step = dynamic_cast<const BBSStep*>(message);
        if(step)
        {
            if(handle(step))
            {
                itsControllerConnection->sendObject(BBSStatus(BBSStatus::OK));
                return true;
            }
            else
            {
                
itsControllerConnection->sendObject(BBSStatus(BBSStatus::ERROR));                
return false;            }
        }

        // We received a message we can't handle
        ostringstream oss;
        oss << "Received message of unsupported type";
//            << itsControllerConnection.itsDataHolder->classType() << "'. 
Skipped.";        LOG_WARN(oss.str());
        itsControllerConnection->sendObject(BBSStatus(BBSStatus::ERROR, 
oss.str()));        return false;
    }
*/
} // namespace BBS
} // namespace LOFAR
