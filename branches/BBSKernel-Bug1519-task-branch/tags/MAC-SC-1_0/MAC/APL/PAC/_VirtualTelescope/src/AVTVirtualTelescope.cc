//#  AVTVirtualTelescope.cc: Implementation of the Virtual Telescope task
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
#undef PACKAGE
#undef VERSION
#include <lofar_config.h>
#include <Common/LofarLogger.h>

#include <GCF/GCF_PValue.h>
#include <GCF/GCF_PVString.h>

#include <APLCommon/APL_Defines.h>
#include "AVTDefines.h"
#include "AVTPropertyDefines.h"
#include "AVTVirtualTelescope.h"
#include "AVTStationBeamformer.h"
#include "AVTStationReceptorGroup.h"
#include "LogicalDevice_Protocol.ph"
#include "AVTUtilities.h"
#include "AVTResourceManager.h"

using namespace LOFAR;
using namespace LOFAR::AVT;
using namespace LOFAR::APLCommon;
using namespace LOFAR::GCF::Common;
using namespace LOFAR::GCF::TM;
using namespace LOFAR::GCF::PAL;
using namespace std;

INIT_TRACER_CONTEXT(AVTVirtualTelescope,LOFARLOGGER_PACKAGE);

AVTVirtualTelescope::AVTVirtualTelescope(string& taskName, 
                                         const string& scope,
                                         const string& APCName,
                                         AVTStationBeamformer& sbf,
                                         AVTStationReceptorGroup& srg) :
  AVTLogicalDevice(taskName,scope,TYPE_LCU_PAC_VT,APCName),
  m_stationBeamformer(sbf),
  m_stationReceptorGroup(srg),
// use process-internal-inter-task-port 
  m_beamFormerClient(sbf,*this, sbf.getServerPortName(), GCFPortInterface::SAP, LOGICALDEVICE_PROTOCOL),
  m_beamFormerConnected(false),
  m_stationReceptorGroupClient(srg,*this, srg.getServerPortName(), GCFPortInterface::SAP, LOGICALDEVICE_PROTOCOL),
  m_stationReceptorGroupConnected(false),
  m_qualityCheckTimerId(0),
  m_startTime(0),
  m_stopTime(0),
  m_frequency(0.0)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,getName().c_str());
  m_stationBeamformer.addClientInterTaskPort(&m_beamFormerClient);
  m_stationReceptorGroup.addClientInterTaskPort(&m_stationReceptorGroupClient);
}


AVTVirtualTelescope::~AVTVirtualTelescope()
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,getName().c_str());
}

bool AVTVirtualTelescope::checkQualityRequirements()
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,getName().c_str());
  bool requirementsMet=false;
  
  // quality requirements for this Virtual Telescope:
  // - not more than 1 antenna unavailable
  // - not more than 1 antenna in alarm
  // - beamformer available
  
  requirementsMet=m_stationBeamformer.checkQualityRequirements();
  if(requirementsMet)
  {
    requirementsMet=m_stationReceptorGroup.checkQualityRequirements(); // max failing antennas
  }
  
  return requirementsMet;
}

void AVTVirtualTelescope::setStartTime(const time_t startTime)
{
  // activate timer that triggers the prepare command
  m_startTime=startTime;
}

void AVTVirtualTelescope::setStopTime(const time_t stopTime)
{
  // activate timer that triggers the suspend command
  m_stopTime=stopTime;
}

void AVTVirtualTelescope::setFrequency(const double frequency)
{
  m_frequency=frequency;
}

bool AVTVirtualTelescope::_isBeamFormerClient(GCFPortInterface& port) const
{
  return (&port == &m_beamFormerClient); // comparing two pointers. yuck?
}

bool AVTVirtualTelescope::_isStationReceptorGroupClient(GCFPortInterface& port) const
{
  return (&port == &m_stationReceptorGroupClient); // comparing two pointers. yuck?
}

bool AVTVirtualTelescope::allInState(GCFPortInterface& port, TLogicalDeviceState state, bool requireSlaveActive) const
{
  bool inState = false;
  AVTResourceManagerPtr resourceManager(AVTResourceManager::instance());
  if(_isBeamFormerClient(port))
  {
    // if the VT is the master, the state of the BF must be the same
    if(resourceManager->isMaster(getName(),m_stationBeamformer.getName()))
    {
      inState = (state == m_stationBeamformer.getLogicalDeviceState());
    }
    else
    {
      if(requireSlaveActive)
      {
        inState = (LOGICALDEVICE_STATE_ACTIVE == m_stationBeamformer.getLogicalDeviceState());
      }
      else
      {
        inState = true;
      }
    }
  }
  
  else if(_isStationReceptorGroupClient(port))
  {
    if(resourceManager->isMaster(getName(),m_stationReceptorGroup.getName()))
    {
      inState = (state == m_stationReceptorGroup.getLogicalDeviceState());
    }
    else
    {
      if(requireSlaveActive)
      {
        inState = (LOGICALDEVICE_STATE_ACTIVE == m_stationReceptorGroup.getLogicalDeviceState());
      }
      else
      {
        inState = true;
      }
    }
  } 
  return inState;
}

void AVTVirtualTelescope::concreteDisconnected(GCFPortInterface& port)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,getName().c_str());
  // go to initial state only if the connection with the beamformer is lost.
  if(_isBeamFormerClient(port))
  {
    m_beamFormerConnected=false;
    TRAN(AVTLogicalDevice::initial_state);
  }
  if(_isStationReceptorGroupClient(port))
  {
    m_stationReceptorGroupConnected=false;
    TRAN(AVTLogicalDevice::initial_state);
  }
}

GCFEvent::TResult AVTVirtualTelescope::concrete_initial_state(GCFEvent& event, GCFPortInterface& port)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,formatString("%s - event=%s",getName().c_str(),evtstr(event)).c_str());
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (event.signal)
  {
    case F_INIT:
      m_stationBeamformer.start();
      // do not start the SRG as it is a shared logical device. It should have been started already
      break;

    case F_ENTRY:
      // open all ports
      m_beamFormerClient.open();
      m_stationReceptorGroupClient.open();
      break;

    case F_CONNECTED:
    {
      // go to operational only if there is a connection with the beam former and station receptor group.
      if(_isBeamFormerClient(port))
      {
        m_beamFormerConnected=true;
      }
      else if(_isStationReceptorGroupClient(port))
      {
        m_stationReceptorGroupConnected=true;
      }
      if(m_beamFormerConnected && m_stationReceptorGroupConnected)
      {
        TRAN(AVTLogicalDevice::idle_state);
      }
      break;
    }

    case F_DISCONNECTED:
      if(_isBeamFormerClient(port) || _isStationReceptorGroupClient(port))
      {
        port.setTimer(3.0); // try again
      }
      break;

    case F_TIMER:
      if(_isBeamFormerClient(port) && !m_beamFormerConnected)
      {
        m_beamFormerClient.open(); // try again
      }
      else if(_isStationReceptorGroupClient(port) && !m_stationReceptorGroupConnected)
      {
        m_stationReceptorGroupClient.open(); // try again
      }
      if(m_beamFormerConnected && m_stationReceptorGroupConnected)
      {
        TRAN(AVTLogicalDevice::idle_state);
      }
      break;

    default:
      LOG_DEBUG(formatString("AVTVirtualTelescope(%s)::concrete_initial_state, default (%s)",getName().c_str(),evtstr(event)));
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}

GCFEvent::TResult AVTVirtualTelescope::concrete_claiming_state(GCFEvent& event, GCFPortInterface& port, bool& stateFinished)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,formatString("%s - event=%s",getName().c_str(),evtstr(event)).c_str());
  GCFEvent::TResult status = GCFEvent::HANDLED;
  
  switch (event.signal)
  {
    case LOGICALDEVICE_CLAIMED:
    {
      AVTResourceManagerPtr resourceManager(AVTResourceManager::instance());
      // claimed event is received from the beam former or station receptor group
      if(allInState(port,LOGICALDEVICE_STATE_CLAIMED,true)) // require slave LD's to be active
      {
        // check quality requirements
        if(checkQualityRequirements())
        {
          stateFinished=true;
        }
        else
        {
          TRAN(AVTLogicalDevice::releasing_state);
          concreteRelease(port);
        }
      }
      break;
    }
    
    default:
      LOG_DEBUG(formatString("AVTVirtualTelescope(%s)::concrete_claiming_state, default",getName().c_str()));
      status = GCFEvent::NOT_HANDLED;
      break;
  }
  
  return status;
}

GCFEvent::TResult AVTVirtualTelescope::concrete_preparing_state(GCFEvent& event, GCFPortInterface& port, bool& stateFinished, bool& error)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,formatString("%s - event=%s",getName().c_str(),evtstr(event)).c_str());
  GCFEvent::TResult status = GCFEvent::HANDLED;
  stateFinished=false;
  error=false;

  switch (event.signal)
  {
    case LOGICALDEVICE_PREPARED:
    {
      AVTResourceManagerPtr resourceManager(AVTResourceManager::instance());
      // prepared event is received from the beam former or station receptor group
      if(allInState(port,LOGICALDEVICE_STATE_SUSPENDED,true)) // require slave LD's to be active
      {
        stateFinished=true;
      }
      break;
    }
        
    default:
      LOG_DEBUG(formatString("AVTVirtualTelescope(%s)::concrete_preparing_state, default",getName().c_str()));
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}

GCFEvent::TResult AVTVirtualTelescope::concrete_active_state(GCFEvent& event, GCFPortInterface& /*port*/)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,formatString("%s - event=%s",getName().c_str(),evtstr(event)).c_str());
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (event.signal)
  {
    case F_TIMER:
    {
      GCFTimerEvent& timerEvent=static_cast<GCFTimerEvent&>(event);
      
      if(timerEvent.id == (unsigned long)m_qualityCheckTimerId)
      {
        if(!checkQualityRequirements())
        {
          LOG_FATAL(formatString("AVTVirtualTelescope(%s): quality too low",getName().c_str(),__func__));
          GCFDummyPort dummyPort(this,string("VT_command_dummy"),LOGICALDEVICE_PROTOCOL);
          LOGICALDEVICESuspendEvent suspendEvent;
          dispatch(suspendEvent,dummyPort); 
        }
        // set quality check timer
        m_qualityCheckTimerId = m_beamFormerClient.setTimer(5.0); // every 5 seconds
      }
      else 
      {
        status = GCFEvent::NOT_HANDLED;
      }
      break;
    }
        
    default:
      LOG_DEBUG(formatString("AVTVirtualTelescope(%s)::concrete_active_state, default",getName().c_str()));
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}

GCFEvent::TResult AVTVirtualTelescope::concrete_releasing_state(GCFEvent& event, GCFPortInterface& port, bool& stateFinished)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,formatString("%s - event=%s",getName().c_str(),evtstr(event)).c_str());
  GCFEvent::TResult status = GCFEvent::HANDLED;
  
  switch (event.signal)
  {
    case LOGICALDEVICE_RELEASED:
    {
      AVTResourceManagerPtr resourceManager(AVTResourceManager::instance());
      // released event is received from the beam former
      if(allInState(port,LOGICALDEVICE_STATE_RELEASED,false)) // ignore slave LD's
      {
        stateFinished=true;
      }
      break;
    }
    default:
      LOG_DEBUG(formatString("AVTVirtualTelescope(%s)::concrete_releasing_state, default",getName().c_str()));
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}

void AVTVirtualTelescope::handlePropertySetAnswer(GCFEvent& answer)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,getName().c_str());
  switch(answer.signal)
  {
    case F_MYPS_ENABLED:
    {
      GCFPropSetAnswerEvent* pPropAnswer=static_cast<GCFPropSetAnswerEvent*>(&answer);
      if(pPropAnswer->result == GCF_NO_ERROR)
      {
        // property set loaded, now load apc
        m_properties.configure(m_APC);
      }
      else
      {
        LOG_ERROR(formatString("%s : PropertySet %s NOT ENABLED",getName().c_str(),pPropAnswer->pScope));
      }
      break;
    }
    
    case F_PS_CONFIGURED:
    {
      GCFConfAnswerEvent* pConfAnswer=static_cast<GCFConfAnswerEvent*>(&answer);
      if(pConfAnswer->result == GCF_NO_ERROR)
      {
        LOG_DEBUG(formatString("%s : apc %s Loaded",getName().c_str(),pConfAnswer->pApcName));
        apcLoaded();
      }
      else
      {
        LOG_WARN(formatString("%s : apc %s NOT LOADED",getName().c_str(),pConfAnswer->pApcName));
      }
      break;
    }
    
    case F_VCHANGEMSG:
    {
      // check which property changed
      GCFPropValueEvent* pPropAnswer=static_cast<GCFPropValueEvent*>(&answer);
      if ((pPropAnswer->pValue->getType() == LPT_STRING) &&
          (strstr(pPropAnswer->pPropName, PROPNAME_COMMAND) != 0))
      {
        // command received
        string commandString(((GCFPVString*)pPropAnswer->pValue)->getValue());
        vector<string> parameters;
        string command;
        AVTUtilities::decodeCommand(commandString,command,parameters);
        
        // PREPARE <starttime>,<stoptime>,<frequency>,<subbands>,<directiontype>,<directionangle1>,<directionangle2>
        if(command==string(LD_COMMAND_PREPARE))
        {
          if(parameters.size()==7)
          {
            // send prepare message:
            string prepareParameters;
            AVTUtilities::encodeParameters(parameters,prepareParameters);
            
            // send message to myself using a dummyport. VT will send it to SBF and SRG
            GCFDummyPort dummyPort(this,string("VT_command_dummy"),LOGICALDEVICE_PROTOCOL);
            LOGICALDEVICEPrepareEvent prepareEvent;
            prepareEvent.parameters = prepareParameters;
            
            unsigned int packSize;
            char* buffer = (char*)prepareEvent.pack(packSize); // otherwise, the parameters are not available
            GCFEvent e;
            e.signal = prepareEvent.signal;
            e.length = prepareEvent.length;
            char* event_buf = new char[sizeof(e) + e.length];
            memcpy(event_buf, &e, sizeof(e));
            memcpy(event_buf+sizeof(e), buffer+packSize-e.length, e.length);
            
            dispatch(*((GCFEvent*)event_buf),dummyPort);
            
            delete[] event_buf;
          }
        }
        // SUSPEND
        else if(command==string(LD_COMMAND_SUSPEND))
        {
          // send message to myself using a dummyport. VT will send it to SBF and SRG
          GCFDummyPort dummyPort(this,string("VT_command_dummy"),LOGICALDEVICE_PROTOCOL);
          LOGICALDEVICESuspendEvent suspendEvent;
          dispatch(suspendEvent,dummyPort); 
        }
        // RESUME
        else if(command==string(LD_COMMAND_RESUME))
        {
          // send message to myself using a dummyport. VT will send it to SBF and SRG
          GCFDummyPort dummyPort(this,string("VT_command_dummy"),LOGICALDEVICE_PROTOCOL);
          LOGICALDEVICEResumeEvent resumeEvent;
          dispatch(resumeEvent,dummyPort);
        }
        // RELEASE
        else if(command==string(LD_COMMAND_RELEASE))
        {
          // send message to myself using a dummyport. VT will send it to SBF and SRG
          GCFDummyPort dummyPort(this,string("VT_command_dummy"),LOGICALDEVICE_PROTOCOL);
          LOGICALDEVICEReleaseEvent releaseEvent;
          dispatch(releaseEvent,dummyPort);
        }
      }
      break;
    }  

    default:
      break;
  }  
}

void AVTVirtualTelescope::concreteClaim(GCFPortInterface& port)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,getName().c_str());
  // claim my own resources
  AVTResourceManagerPtr resourceManager(AVTResourceManager::instance());
  resourceManager->requestResource(getName(),m_stationBeamformer.getName());
  resourceManager->requestResource(getName(),m_stationReceptorGroup.getName());
  
  // send claim message to BeamFormer and SRG
  bool eventSent=false;
  LOGICALDEVICEClaimEvent claimEvent;
  if(resourceManager->isMaster(getName(),m_stationBeamformer.getName()))
  {
    m_beamFormerClient.send(claimEvent);
    eventSent=true;
  }
  if(resourceManager->isMaster(getName(),m_stationReceptorGroup.getName()))
  {
    m_stationReceptorGroupClient.send(claimEvent);
    eventSent=true;
  }
  if(!eventSent)
  {
    LOGICALDEVICEClaimedEvent claimedEvent;
    dispatch(claimedEvent,port);
  }
}

void AVTVirtualTelescope::concretePrepare(GCFPortInterface& /*port*/,string& parameters)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,getName().c_str());
  
  bool unableToPrepare = false;
  AVTResourceManagerPtr resourceManager(AVTResourceManager::instance());
  LOGICALDEVICEPrepareEvent prepareEvent;
  vector<string> decodedParameters;
  AVTUtilities::decodeParameters(parameters,decodedParameters);
  // if the prepare parameters are already applied, then we don't have to send the prepare msg
  // if they do not match, then we send the prepare msg if we are master. Otherwise, error
  if(!m_stationBeamformer.isPrepared(decodedParameters))
  {  
    if(resourceManager->isMaster(getName(),m_stationBeamformer.getName()))
    {
      // send prepare message to BeamFormer
      // all parameters are forwarded to the beamformer
      prepareEvent.parameters = parameters;
      m_beamFormerClient.send(prepareEvent);
    }
    else
    {
      LOG_ERROR(formatString("Virtual Telescope %s is not master of %s",getName().c_str(),m_stationBeamformer.getName().c_str()));
      unableToPrepare = true;
    }
  }  
  if(!unableToPrepare)
  {
    if(!m_stationReceptorGroup.isPrepared(decodedParameters))
    {  
      if(resourceManager->isMaster(getName(),m_stationReceptorGroup.getName()))
      {
        // send prepare message to StationReceptorGroup
        // all parameters are forwarded to the SRG
        prepareEvent.parameters = parameters;
        m_stationReceptorGroupClient.send(prepareEvent);
      }
      else
      {
        LOG_ERROR(formatString("Virtual Telescope %s is not master of %s",getName().c_str(),m_stationReceptorGroup.getName().c_str()));
        unableToPrepare = true;
      }
    }  
  }
  if(!unableToPrepare)
  {
    // prepare my own resources
    // parameters: start time,stop time,frequency,subbands,direction type,angle1,angle2
    setStartTime(atoi(decodedParameters[0].c_str()));
    setStopTime(atoi(decodedParameters[1].c_str()));
    setFrequency(atof(decodedParameters[2].c_str()));
  }
  else
  {
    LOG_WARN(formatString("Unable to prepare Virtual Telescope %s",getName().c_str()));
  }
}

void AVTVirtualTelescope::concreteResume(GCFPortInterface& port)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,getName().c_str());
  // resume my own resources
  
  // send resume message to BeamFormer and SRGT
  bool eventSent=false;
  LOGICALDEVICEResumeEvent resumeEvent;
  AVTResourceManagerPtr resourceManager(AVTResourceManager::instance());
  if(resourceManager->isMaster(getName(),m_stationBeamformer.getName()))
  {
    m_beamFormerClient.send(resumeEvent);
    eventSent=true;
  }
  if(resourceManager->isMaster(getName(),m_stationReceptorGroup.getName()))
  {
    m_stationReceptorGroupClient.send(resumeEvent);
    eventSent=true;
  }
  if(!eventSent)
  {
    LOGICALDEVICEResumedEvent resumedEvent;
    dispatch(resumedEvent,port);
  }
  
  // set quality check timer
  m_qualityCheckTimerId = m_beamFormerClient.setTimer(5.0); // every 5 seconds
}

void AVTVirtualTelescope::concreteSuspend(GCFPortInterface& /*port*/)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,getName().c_str());
  // suspend my own resources
  
  // send suspend message to BeamFormer and SRG
  LOGICALDEVICESuspendEvent suspendEvent;
  AVTResourceManagerPtr resourceManager(AVTResourceManager::instance());
  if(resourceManager->isMaster(getName(),m_stationBeamformer.getName()))
  {
    m_beamFormerClient.send(suspendEvent);
  }
  if(resourceManager->isMaster(getName(),m_stationReceptorGroup.getName()))
  {
    m_stationReceptorGroupClient.send(suspendEvent);
  }
}

void AVTVirtualTelescope::concreteRelease(GCFPortInterface& /*port*/)
{
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,getName().c_str());
  // release my own resources
  
  // claim my own resources
  AVTResourceManagerPtr resourceManager(AVTResourceManager::instance());
  resourceManager->releaseResource(getName(),m_stationBeamformer.getName());
  resourceManager->releaseResource(getName(),m_stationReceptorGroup.getName());

  // send release message to BeamFormer and SRG
  LOGICALDEVICEReleaseEvent releaseEvent;
  m_beamFormerClient.send(releaseEvent);
  //m_stationReceptorGroupClient.send(releaseEvent);
}

