//#  AVTLogicalDeviceScheduler.cc: Implementation of the Virtual Telescope task
//#
//#  Copyright (C) 2002-2004
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

#include <GCF/GCF_Defines.h>
#include <GCF/GCF_PValue.h>
#include <GCF/GCF_PVString.h>

#include "../../../APLCommon/src/APL_Defines.h"
#include "AVTLogicalDeviceScheduler.h"
#define DECLARE_SIGNAL_NAMES
#include "LogicalDevice_Protocol.ph"
#include "../test/PropertyDefines.h"
#include "AVTDefines.h"
#include "AVTStationBeamformer.h"
#include "AVTVirtualTelescope.h"
#include "AVTUtilities.h"

string AVTLogicalDeviceScheduler::m_schedulerTaskName("AVTLogicalDeviceScheduler");

AVTLogicalDeviceScheduler::AVTLogicalDeviceScheduler() :
  GCFTask((State)&AVTLogicalDeviceScheduler::initial_state,m_schedulerTaskName),
  AVTPropertySetAnswerHandlerInterface(),
  m_propertySetAnswer(*this),
  m_properties(propertySetLDS,&m_propertySetAnswer),
  m_logicalDeviceMap()
{
  LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTLogicalDeviceScheduler::AVTLogicalDeviceScheduler(%s)",getName().c_str()));
}


AVTLogicalDeviceScheduler::~AVTLogicalDeviceScheduler()
{
  LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTLogicalDeviceScheduler::~AVTLogicalDeviceScheduler(%s)",getName().c_str()));
}

GCFEvent::TResult AVTLogicalDeviceScheduler::initial_state(GCFEvent& event, GCFPortInterface& /*port*/)
{
  LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTLogicalDeviceScheduler::initial_state (%s)",getName().c_str()));

  GCFEvent::TResult status = GCFEvent::HANDLED;
  
  switch (event.signal)
  {
    case F_INIT_SIG:
      LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTLogicalDeviceScheduler::initial_state, F_INIT_SIG (%s)",getName().c_str()));
      break;

    case F_ENTRY_SIG:
      LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTLogicalDeviceScheduler::initial_state, F_ENTRY_SIG (%s)",getName().c_str()));
      break;

    case F_CONNECTED_SIG:
      LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTLogicalDeviceScheduler::initial_state, F_CONNECTED_SIG (%s)",getName().c_str()));
      break;

    case F_DISCONNECTED_SIG:
      LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTLogicalDeviceScheduler::initial_state, F_DISCONNECTED_SIG (%s)",getName().c_str()));
      break;

    case F_TIMER_SIG:
      LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTLogicalDeviceScheduler::initial_state, F_TIMER_SIG (%s)",getName().c_str()));
      break;

    default:
      LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTLogicalDeviceScheduler::initial_state, default (%s)",getName().c_str()));
      status = GCFEvent::NOT_HANDLED;
      break;
  }
  return status;
}

void AVTLogicalDeviceScheduler::handlePropertySetAnswer(GCFEvent& answer)
{
  LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTLogicalDeviceScheduler::handlePropertySetAnswer (%s)",getName().c_str()));

  switch(answer.signal)
  {
    case F_VCHANGEMSG_SIG:
    {
      // check which property changed
      GCFPropValueEvent* pPropAnswer = static_cast<GCFPropValueEvent*>(&answer);
      assert(pPropAnswer);
      if ((pPropAnswer->pValue->getType() == GCFPValue::LPT_STRING) &&
          (strstr(pPropAnswer->pPropName, propertyLDScommand.c_str()) != 0))
      {
        // command received
        string commandString(((GCFPVString*)pPropAnswer->pValue)->getValue());
        vector<string> parameters;
        string command;
        AVTUtilities::decodeCommand(commandString,command,parameters);
        
        // SCHEDULE <vt_name>,<bf_name>,<srg_name>,<starttime>,<stoptime>,
        //          <frequency>,<subbands>,<direction>
        if(command==string("SCHEDULE"))
        {
          if(parameters.size()==8)
          {
            LogicalDeviceInfoT            srgInfo;
            LogicalDeviceInfoT            sbfInfo;
            SchedulableLogicalDeviceInfoT vtInfo;
            
            // create SRG
//            boost::shared_ptr<> srg(new AVTStationReceptorGroup());        
//            srgInfo.logicalDevice=srg;
            
            // create SBF
            boost::shared_ptr<AVTStationBeamformer> sbf(new AVTStationBeamformer(parameters[1],primaryPropertySetSBF,sSBFAPCName,sSBFAPCScope,sBSName));
            sbfInfo.logicalDevice=sbf;
            
            // create VT
            boost::shared_ptr<AVTVirtualTelescope> vt(new AVTVirtualTelescope(parameters[0],primaryPropertySetVT,sVTAPCName,sVTAPCScope,*sbf.get()));
            vtInfo.logicalDevice=vt;
            vtInfo.clientPort.reset(new GCFPort(*this, vt->getServerPortName(), GCFPortInterface::SAP, LOGICALDEVICE_PROTOCOL));
            vtInfo.children[parameters[1]]=sbfInfo;
            vtInfo.children[parameters[2]]=srgInfo;
            vtInfo.startTime=atoi(parameters[3].c_str());
            vtInfo.stopTime=atoi(parameters[4].c_str());
            vtInfo.parameters.push_back(parameters[3]); // send start time also to VT to trigger activation
            vtInfo.parameters.push_back(parameters[4]); // send stop time also to VT to trigger deactivation
            vtInfo.parameters.push_back(parameters[5]); // frequency
            vtInfo.parameters.push_back(parameters[6]); // subbands
            vtInfo.parameters.push_back(parameters[7]); // direction type
            vtInfo.parameters.push_back(parameters[8]); // angle1
            vtInfo.parameters.push_back(parameters[9]); // angle2
            
            m_logicalDeviceMap[parameters[0]]=vtInfo;
            
            // increment1: ignore starting and stopping time, start the damn thing right now!
            // in the future, the scheduler will claim, prepare and resume the scheduled Logical Devices.
            
            // claim
            GCFEvent claimEvent(LOGICALDEVICE_CLAIM);
            vtInfo.clientPort->send(claimEvent);
            
            // prepare
            char prepareParameters[700];
            AVTUtilities::encodeParameters(vtInfo.parameters,prepareParameters,700);
            
            LOGICALDEVICEPrepareEvent prepareEvent(prepareParameters);
            // send prepare to Virtual Telescope. VT will send prepare to SBF and SRG
            vtInfo.clientPort->send(prepareEvent);
            
            // the logical device resumes itself after preparation using the starttime
          }
        }
        // RELEASE <name>
        else if(command==string("RELEASE"))
        {
          if(parameters.size()==1)
          {
            SchedulableLogicalDeviceMapT::iterator it=m_logicalDeviceMap.find(parameters[0]);
            if(it!=m_logicalDeviceMap.end())
            {
              // remove the object from the map. The Port instance and children Logical Device instances
              // are destroyed too because they are smart pointers.
              m_logicalDeviceMap.erase(it);
            }
          } 
        }
      }
      break;
    }  
    default:
      break;
  }  
}

