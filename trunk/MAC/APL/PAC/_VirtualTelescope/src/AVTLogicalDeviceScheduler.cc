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
#include "../../../APLCommon/src/APLInterTaskPort.h"
#include "AVTLogicalDeviceScheduler.h"
#include "LogicalDevice_Protocol.ph"
#include "AVTDefines.h"
#include "AVTStationBeamformer.h"
#include "AVTVirtualTelescope.h"
#include "AVTUtilities.h"

#include "../test/PropertyDefines.h"

string AVTLogicalDeviceScheduler::m_schedulerTaskName(LDSNAME);

AVTLogicalDeviceScheduler::AVTLogicalDeviceScheduler() :
  GCFTask((State)&AVTLogicalDeviceScheduler::initial_state,m_schedulerTaskName),
  AVTPropertySetAnswerHandlerInterface(),
  m_propertySetAnswer(*this),
  m_properties(propertySetLDS,&m_propertySetAnswer),
  m_apcLDS("ApcLogicalDeviceScheduler", "LogicalDeviceScheduler", &m_propertySetAnswer),
  m_initialized(false),
  m_logicalDeviceMap()
{
  registerProtocol(LOGICALDEVICE_PROTOCOL, LOGICALDEVICE_PROTOCOL_signalnames);
  LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTLogicalDeviceScheduler::AVTLogicalDeviceScheduler(%s)",getName().c_str()));
  m_properties.load();
}


AVTLogicalDeviceScheduler::~AVTLogicalDeviceScheduler()
{
  LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTLogicalDeviceScheduler::~AVTLogicalDeviceScheduler(%s)",getName().c_str()));
  m_apcLDS.unload();
}

bool AVTLogicalDeviceScheduler::isInitialized()
{
  return m_initialized;
}

bool AVTLogicalDeviceScheduler::findClientPort(GCFPortInterface& port,string& key)
{
  SchedulableLogicalDeviceMapT::iterator it=m_logicalDeviceMap.begin();
  bool portFound(false);
  while(it!=m_logicalDeviceMap.end()&&!portFound)
  {
    portFound=(&port==it->second.clientPort.get());
    if(portFound)
    {
      key=it->first;
    }
    ++it;
  }
  return portFound;
}

GCFEvent::TResult AVTLogicalDeviceScheduler::initial_state(GCFEvent& event, GCFPortInterface& port)
{
  LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTLogicalDeviceScheduler(%s)::initial_state (%s)",getName().c_str(),evtstr(event)));

  GCFEvent::TResult status = GCFEvent::HANDLED;
  
  switch (event.signal)
  {
    case F_INIT_SIG:
      break;

    case F_ENTRY_SIG:
      break;

    case F_CONNECTED_SIG:
    {
      string clientName;
      if(findClientPort(port,clientName))
      {
        // send claim message:
        GCFEvent claimEvent(LOGICALDEVICE_CLAIM);
        port.send(claimEvent);
      }
      break;
    }

    case LOGICALDEVICE_CLAIMED:
    {
      GCFPVString status("Claimed");
      m_properties.setValue(PROPERTY_LDS_STATUS,status);

      string clientName;
      if(findClientPort(port,clientName))
      {
        // send prepare message:
        char prepareParameters[700];
        AVTUtilities::encodeParameters(m_logicalDeviceMap[clientName].parameters,prepareParameters,700);
        
        LOGICALDEVICEPrepareEvent prepareEvent(prepareParameters);
        // send prepare to Virtual Telescope. VT will send prepare to SBF and SRG
        port.send(prepareEvent);

        // the logical device resumes itself after preparation using the starttime
      }
      break;
    }
    case LOGICALDEVICE_PREPARED:
    {
      GCFPVString status("Prepared");
      m_properties.setValue(PROPERTY_LDS_STATUS,status);
      
      // mac 1 increment only: resume immediately
      string clientName;
      if(findClientPort(port,clientName))
      {
        // send resume message:
        GCFEvent claimEvent(LOGICALDEVICE_RESUME);
        port.send(claimEvent);
      }
      break;
    }
        
    case LOGICALDEVICE_RESUMED:
    {
      GCFPVString status("Resumed");
      m_properties.setValue(PROPERTY_LDS_STATUS,status);
      break;
    }
        
    case F_DISCONNECTED_SIG:
    {
      string clientName;
      if(findClientPort(port,clientName))
      {
        port.setTimer(10.0);
      }
      break;
    }
    
    case F_TIMER_SIG:
    {
      string clientName;
      if(findClientPort(port,clientName))
      {
        port.open();
      }
      break;
    }
    
    default:
      LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTLogicalDeviceScheduler(%s)::initial_state, default",getName().c_str()));
      status = GCFEvent::NOT_HANDLED;
      break;
  }
  return status;
}

void AVTLogicalDeviceScheduler::handlePropertySetAnswer(GCFEvent& answer)
{
//  LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTLogicalDeviceScheduler(%s)::handlePropertySetAnswer (%s)",getName().c_str(),evtstr(answer)));

  switch(answer.signal)
  {
    case F_MYPLOADED_SIG:
    {
      // property set loaded, now load apc
      m_apcLDS.load(false);
      break;
    }
    
    case F_APCLOADED_SIG:
    {
      m_initialized=true;
      break;
    }
    
    case F_VCHANGEMSG_SIG:
    {
      // check which property changed
      GCFPropValueEvent* pPropAnswer = static_cast<GCFPropValueEvent*>(&answer);
      assert(pPropAnswer);
      if ((pPropAnswer->pValue->getType() == GCFPValue::LPT_STRING) &&
          (strstr(pPropAnswer->pPropName, PROPERTY_LDS_COMMAND) != 0))
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
          if(parameters.size()==10)
          {
            LogicalDeviceInfoT            srgInfo;
            LogicalDeviceInfoT            sbfInfo;
            SchedulableLogicalDeviceInfoT vtInfo;
            
            SchedulableLogicalDeviceMapT::iterator ldIt=m_logicalDeviceMap.find(parameters[0]);
            if(ldIt!=m_logicalDeviceMap.end())
            {
              // virtual telescope already exists, send the prepare event
              ldIt->second.startTime=atoi(parameters[3].c_str());
              ldIt->second.stopTime=atoi(parameters[4].c_str());
              ldIt->second.parameters.clear();
              ldIt->second.parameters.push_back(parameters[3]); // send start time also to VT to trigger activation
              ldIt->second.parameters.push_back(parameters[4]); // send stop time also to VT to trigger deactivation
              ldIt->second.parameters.push_back(parameters[5]); // frequency
              ldIt->second.parameters.push_back(parameters[6]); // subbands
              ldIt->second.parameters.push_back(parameters[7]); // direction type
              ldIt->second.parameters.push_back(parameters[8]); // angle1
              ldIt->second.parameters.push_back(parameters[9]); // angle2

              // send prepare message:
              char prepareParameters[700];
              AVTUtilities::encodeParameters(ldIt->second.parameters,prepareParameters,700);
              
              LOGICALDEVICEPrepareEvent prepareEvent(prepareParameters);
              // send prepare to Virtual Telescope. VT will send prepare to SBF and SRG
              ldIt->second.clientPort->send(prepareEvent);
            }
            else
            {              
              // create SRG
  //            boost::shared_ptr<> srg(new AVTStationReceptorGroup());        
  //            srgInfo.logicalDevice=srg;
              
              // create SBF
              string sbfApcName(SBFAPCNAME);
              string bsName(BSNAME);
              boost::shared_ptr<AVTStationBeamformer> sbf(new AVTStationBeamformer(parameters[1],primaryPropertySetSBF,sbfApcName,parameters[0]+string("_")+parameters[1],bsName));
              sbfInfo.logicalDevice=sbf;
              
              // create VT
              string vtApcName(VTAPCNAME);
              boost::shared_ptr<AVTVirtualTelescope> vt(new AVTVirtualTelescope(parameters[0],primaryPropertySetVT,vtApcName,parameters[0],*sbf.get()));
              vtInfo.logicalDevice=vt;
  //            vtInfo.clientPort.reset(new GCFPort(*this, vt->getServerPortName(), GCFPortInterface::SAP, LOGICALDEVICE_PROTOCOL));
              vtInfo.clientPort.reset(new APLInterTaskPort(*vt.get(),*this, vt->getServerPortName(), GCFPortInterface::SAP, LOGICALDEVICE_PROTOCOL));
              vt->setClientInterTaskPort(vtInfo.clientPort.get());
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
              vt->start();
              
              // increment1: ignore starting and stopping time, start the damn thing right now!
              // in the future, the scheduler will claim, prepare and resume the scheduled Logical Devices.
              // connect and send the claim and prepare messages            
              vtInfo.clientPort->open();
              // after receiving connected event, messages are sent            
            }
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

