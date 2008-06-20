//#  ProcCtrlRemote.cc: Proxy for the ACC controlled process.
//#
//#  Copyright (C) 2008
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

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

//# Includes
#include <PLC/ProcCtrlRemote.h>
#include <APS/ParameterSet.h>
#include <Common/LofarLogger.h>

namespace LOFAR
{
  namespace ACC
  {
    namespace PLC
    {
      using namespace APS;

      ProcCtrlRemote::ProcCtrlRemote(ProcessControl* aProcCtrl) :
        ProcCtrlProxy(aProcCtrl)
      {
        LOG_TRACE_FLOW(AUTO_FUNCTION_NAME);
      }

      int ProcCtrlRemote::exec(const ParameterSet& arg)
      {
        LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW, "");

        string prefix = globalParameterSet()->getString("_parsetPrefix");
			
        // connect to Application Controller
        ProcControlServer pcServer(globalParameterSet()->getString(prefix+"_ACnode"),
                                   globalParameterSet()->getUint16(prefix+"_ACport"),
                                   this);

        // Tell AC who we are.
        string procID = arg.getString("ProcID");
        LOG_DEBUG_STR("Registering at ApplController as " << procID);
        sleep(1);
        pcServer.registerAtAC(procID);

        // Main processing loop
        bool ok(true);
        bool quiting(false);
        while (!quiting) {

          LOG_TRACE_STAT("Polling ApplController for message");
          if (pcServer.pollForMessage()) {
            LOG_TRACE_COND("Message received from ApplController");

            // get pointer to received data
            DH_ProcControl* newMsg = pcServer.getDataHolder();

            if (newMsg->getCommand() == PCCmdQuit) {
              quiting = true;
            } 

            if (!(ok = ok && pcServer.handleMessage(newMsg))) {
              LOG_ERROR("ProcControlServer::handleMessage() failed");
            }
          } 
          else  {
            // no new command received. If we are in the runstate 
            // call the run-routine again.
            if (inRunState()) {
              DH_ProcControl tmpMsg(PCCmdRun);
              ok = ok && pcServer.handleMessage(&tmpMsg);
            }
          }
        }
        LOG_INFO_STR("Shutting down: ApplicationController");
        pcServer.unregisterAtAC("");		// send to AC before quiting

        return (!ok);
      }
      
    } // namespace PLC
    
  } // namespace ACC

} // namespace LOFAR
