//#  LogicalDevice.cc: Implementation of the Virtual Telescope task
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
#include <GCF/GCF_PVString.h>
#include <GCF/GCF_PVInteger.h>
#include "APLCommon/APL_Defines.h"
#include "APLCommon/APLUtilities.h"
#include "APLCommon/LogicalDevice.h"

#define DECLARE_SIGNAL_NAMES
#include "APLCommon/LogicalDevice_Protocol.ph"

using namespace LOFAR::ACC;

namespace LOFAR
{
namespace APLCommon
{

const string LogicalDevice::LD_STATE_STRING_INITIAL     = string("Initial");
const string LogicalDevice::LD_STATE_STRING_IDLE        = string("Idle");
const string LogicalDevice::LD_STATE_STRING_CLAIMING    = string("Claiming");
const string LogicalDevice::LD_STATE_STRING_CLAIMED     = string("Claimed");
const string LogicalDevice::LD_STATE_STRING_PREPARING   = string("Preparing");
const string LogicalDevice::LD_STATE_STRING_SUSPENDED   = string("Suspended");
const string LogicalDevice::LD_STATE_STRING_ACTIVE      = string("Active");
const string LogicalDevice::LD_STATE_STRING_RELEASING   = string("Releasing");
const string LogicalDevice::LD_STATE_STRING_RELEASED    = string("Released");

const string LogicalDevice::LD_PROPNAME_COMMAND         = string("command");
const string LogicalDevice::LD_PROPNAME_STATUS          = string("status");

const string LogicalDevice::LD_COMMAND_SCHEDULE         = string("SCHEDULE");
const string LogicalDevice::LD_COMMAND_CANCELSCHEDULE   = string("CANCELSCHEDULE");
const string LogicalDevice::LD_COMMAND_CLAIM            = string("CLAIM");
const string LogicalDevice::LD_COMMAND_PREPARE          = string("PREPARE");
const string LogicalDevice::LD_COMMAND_RESUME           = string("RESUME");
const string LogicalDevice::LD_COMMAND_SUSPEND          = string("SUSPEND");
const string LogicalDevice::LD_COMMAND_RELEASE          = string("RELEASE");

LogicalDevice::LogicalDevice(const string& taskName, const string& parameterFile) throw (APLCommon::ParameterFileNotFoundException, APLCommon::ParameterNotFoundException) :
  ::GCFTask((State)&LogicalDevice::initial_state,taskName),
  PropertySetAnswerHandlerInterface(),
  m_propertySetAnswer(*this),
  m_propertySet(),
  m_referencesPropSet(),
  m_serverPortName(taskName+string("_server")),
  m_parentPort(),
  m_serverPort(*this, m_serverPortName, ::GCFPortInterface::SPP, LOGICALDEVICE_PROTOCOL),
  m_childPorts(),
  m_apcLoaded(false),
  m_logicalDeviceState(LOGICALDEVICE_STATE_IDLE),
  m_prepareTimerId(0),
  m_startTimerId(0),
  m_stopTimerId(0),
  m_parameterSet()
{
  registerProtocol(LOGICALDEVICE_PROTOCOL, LOGICALDEVICE_PROTOCOL_signalnames);
  LOG_DEBUG(formatString("LogicalDevice(%s)::LogicalDevice",getName().c_str()));

  string psName;
  string psType;
  
  try
  {
    m_parameterSet.adoptFile(parameterFile);
  }
  catch(Exception& e)
  {
    THROW(APLCommon::ParameterFileNotFoundException,e.message());
  }
  
  try
  {
    psName = m_parameterSet.getString("psName");
    psType = m_parameterSet.getString("psType");
  }
  catch(Exception& e)
  {
    THROW(APLCommon::ParameterNotFoundException,e.message());
  }
  
  m_propertySet = boost::shared_ptr<GCFMyPropertySet>(new GCFMyPropertySet(
      psName.c_str(),
      psType.c_str(),
      PS_CAT_TEMPORARY,
      &m_propertySetAnswer));
  m_propertySet->enable();
  
  string refPSName = psName + "__ref";
  m_referencesPropSet = boost::shared_ptr<GCFMyPropertySet>(new GCFMyPropertySet(
      refPSName.c_str(),
      "TLDReference",
      PS_CAT_TEMPORARY,
      &m_propertySetAnswer));
  m_referencesPropSet->enable();
}


LogicalDevice::~LogicalDevice()
{
  LOG_DEBUG(formatString("LogicalDevice(%s)::~LogicalDevice",getName().c_str()));
  m_propertySet->disable();
  m_referencesPropSet->disable();
}

string& LogicalDevice::getServerPortName()
{
  return m_serverPortName;
}

void LogicalDevice::addChildPort(boost::shared_ptr<GCFPVSSPort> childPort)
{
  m_childPorts.push_back(childPort);
}

bool LogicalDevice::isPrepared(vector<string>& /*parameters*/)
{
  return false;
}

LogicalDevice::TLogicalDeviceState LogicalDevice::getLogicalDeviceState() const
{
  return m_logicalDeviceState;
}

void LogicalDevice::handlePropertySetAnswer(::GCFEvent& answer)
{
  switch(answer.signal)
  {
    case F_MYPS_ENABLED:
    {
      GCFPropSetAnswerEvent* pPropAnswer=static_cast<GCFPropSetAnswerEvent*>(&answer);
      if(pPropAnswer->result == GCF_NO_ERROR)
      {
        // property set loaded, now load apc?
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
        //apcLoaded();
      }
      else
      {
        LOG_ERROR(formatString("%s : apc %s NOT LOADED",getName().c_str(),pConfAnswer->pApcName));
      }
      break;
    }
    
    case F_VCHANGEMSG:
    {
      // check which property changed
      GCFPropValueEvent* pPropAnswer=static_cast<GCFPropValueEvent*>(&answer);
      if ((pPropAnswer->pValue->getType() == LPT_STRING) &&
          (strstr(pPropAnswer->pPropName, LD_PROPNAME_COMMAND.c_str()) != 0))
      {
        // command received
        string commandString(((GCFPVString*)pPropAnswer->pValue)->getValue());
        vector<string> parameters;
        string command;
        APLUtilities::decodeCommand(commandString,command,parameters);
        
        // SCHEDULE <fileName>
        if(command==string(LD_COMMAND_SCHEDULE))
        {
          if(parameters.size()==1)
          {
          }
          else
          {
            TLDResult result = LD_RESULT_INCORRECT_NUMBER_OF_PARAMETERS;
            m_propertySet->setValue(LD_PROPNAME_STATUS,GCFPVInteger(result));
          }
        }
        // CANCELSCHEDULE <fileName>
        else if(command==string(LD_COMMAND_CANCELSCHEDULE))
        {
          if(parameters.size()==1)
          {
          }
          else
          {
            TLDResult result = LD_RESULT_INCORRECT_NUMBER_OF_PARAMETERS;
            m_propertySet->setValue(LD_PROPNAME_STATUS,GCFPVInteger(result));
          }
        }
        // CLAIM
        else if(command==string(LD_COMMAND_CLAIM))
        {
          if(parameters.size()==0)
          {
          }
          else
          {
            TLDResult result = LD_RESULT_INCORRECT_NUMBER_OF_PARAMETERS;
            m_propertySet->setValue(LD_PROPNAME_STATUS,GCFPVInteger(result));
          }
        }
        // PREPARE
        else if(command==string(LD_COMMAND_PREPARE))
        {
          if(parameters.size()==0)
          {
          }
          else
          {
            TLDResult result = LD_RESULT_INCORRECT_NUMBER_OF_PARAMETERS;
            m_propertySet->setValue(LD_PROPNAME_STATUS,GCFPVInteger(result));
          }
        }
        // RESUME
        else if(command==string(LD_COMMAND_RESUME))
        {
          if(parameters.size()==0)
          {
          }
          else
          {
            TLDResult result = LD_RESULT_INCORRECT_NUMBER_OF_PARAMETERS;
            m_propertySet->setValue(LD_PROPNAME_STATUS,GCFPVInteger(result));
          }
        }
        // SUSPEND
        else if(command==string(LD_COMMAND_SUSPEND))
        {
          if(parameters.size()==0)
          {
          }
          else
          {
            TLDResult result = LD_RESULT_INCORRECT_NUMBER_OF_PARAMETERS;
            m_propertySet->setValue(LD_PROPNAME_STATUS,GCFPVInteger(result));
          }
        }
        // RELEASE
        else if(command==string(LD_COMMAND_RELEASE))
        {
          if(parameters.size()==0)
          {
          }
          else
          {
            TLDResult result = LD_RESULT_INCORRECT_NUMBER_OF_PARAMETERS;
            m_propertySet->setValue(LD_PROPNAME_STATUS,GCFPVInteger(result));
          }
        }
        else
        {
          TLDResult result = LD_RESULT_UNKNOWN_COMMAND;
          m_propertySet->setValue(LD_PROPNAME_STATUS,GCFPVInteger(result));
        }
      }
      break;
    }  

    default:
      break;
  }  
}

bool LogicalDevice::_isParentPort(::GCFPortInterface& port)
{
  return (&port == &m_parentPort); // comparing two pointers. yuck?
}
   
bool LogicalDevice::_isServerPort(::GCFPortInterface& port)
{
  return (&port == &m_serverPort); // comparing two pointers. yuck?
}
   
bool LogicalDevice::_isChildPort(::GCFPortInterface& port)
{
  bool found=false;
  TPVSSPortVector::iterator it=m_childPorts.begin();
  while(!found && it != m_childPorts.end())
  {
    found = (&port == (*it).get()); // comparing two pointers. yuck?
    ++it;
  }
  return found;
}
   
void LogicalDevice::_sendToAllChilds(::GCFEvent& event)
{
  // send to all childs
  TPVSSPortVector::iterator it=m_childPorts.begin();
  while(it != m_childPorts.end())
  {
    (*it)->send(event);
    ++it;
  }
}

void LogicalDevice::_disconnectedHandler(::GCFPortInterface& port)
{
  port.close();
  if(_isServerPort(port))
  {
  }
  else if(_isChildPort(port))
  {
    concreteChildDisconnected(port);
  }
  else if(_isParentPort(port))
  {
    concreteParentDisconnected(port);
  }
}

bool LogicalDevice::_isAPCLoaded() const
{
  return m_apcLoaded;
}

void LogicalDevice::_apcLoaded()
{
  m_apcLoaded=true;
}

void LogicalDevice::_doStateTransition(const TLogicalDeviceState& newState)
{
  switch(newState)
  {
    case LOGICALDEVICE_STATE_NOSTATE:
      // no transition
      break;
    case LOGICALDEVICE_STATE_IDLE:
      TRAN(LogicalDevice::idle_state);
      break;
    case LOGICALDEVICE_STATE_CLAIMING:
      TRAN(LogicalDevice::claiming_state);
      break;
    case LOGICALDEVICE_STATE_CLAIMED:
      TRAN(LogicalDevice::claimed_state);
      break;
    case LOGICALDEVICE_STATE_PREPARING:
      TRAN(LogicalDevice::preparing_state);
      break;
    case LOGICALDEVICE_STATE_SUSPENDED:
      TRAN(LogicalDevice::suspended_state);
      break;
    case LOGICALDEVICE_STATE_ACTIVE:
      TRAN(LogicalDevice::active_state);
      break;
    case LOGICALDEVICE_STATE_RELEASING:
      TRAN(LogicalDevice::releasing_state);
      break;
    case LOGICALDEVICE_STATE_RELEASED:
      break;
    default:
      // no transition
      break;
  }
}

void LogicalDevice::_handleTimers(::GCFEvent& event, ::GCFPortInterface& port)
{
  if(event.signal == F_TIMER)
  {
    GCFTimerEvent& timerEvent=static_cast<GCFTimerEvent&>(event);
    if(timerEvent.id == m_prepareTimerId)
    {
      port.cancelTimer(timerEvent.id);
      LOG_DEBUG(formatString("(%s) PrepareTimer %d triggered and cancelled",__func__,timerEvent.id));
      // this is a prepare timer for the schedule of a logical device. claim the device
      LOGICALDEVICEClaimEvent claimEvent;
      port.send(claimEvent);
      // when the device (and all it's children) is claimed, the prepare message is sent
      // automatically
    }
    else if(timerEvent.id == m_startTimerId)
    {
      port.cancelTimer(timerEvent.id);
      LOG_DEBUG(formatString("(%s) StartTimer %d triggered and cancelled",__func__,timerEvent.id));
      // this is a start timer for the schedule of a logical device. resume the device
      LOGICALDEVICEResumeEvent resumeEvent;
      port.send(resumeEvent);
    }
    else if(timerEvent.id == m_stopTimerId)
    {
      port.cancelTimer(timerEvent.id);
      LOG_DEBUG(formatString("(%s) StopTimer %d triggered and cancelled",__func__,timerEvent.id));
      // this is a stop timer for the schedule of a logical device. suspend the device
      LOGICALDEVICESuspendEvent suspendEvent;
      port.send(suspendEvent);
    }
  }
}

::GCFEvent::TResult LogicalDevice::initial_state(::GCFEvent& event, ::GCFPortInterface& port)
{
  LOG_DEBUG(formatString("LogicalDevice(%s)::initial_state (%s)",getName().c_str(),evtstr(event)));

  ::GCFEvent::TResult status = ::GCFEvent::HANDLED;
  
  switch (event.signal)
  {
    case F_INIT:
      break;

    case F_ENTRY:
    {
      m_logicalDeviceState = LOGICALDEVICE_STATE_IDLE;
      ::GCFPVString status(LD_STATE_STRING_INITIAL);
      m_propertySet->setValue(LD_PROPNAME_STATUS,status);
      
      // connect to parent.
      string parentPort = m_parameterSet.getString(m_parameterSet.getName() + string(".parentPort"));
      m_parentPort.setDestAddr(parentPort);
      m_parentPort.open();
      
      // set timers
      // specified times are in UTC, seconds since 1-1-1970
      time_t timeNow = time(0);
      time_t prepareTime = m_parameterSet.getInt(m_parameterSet.getName() + string(".prepareTime"));
      time_t startTime   = m_parameterSet.getInt(m_parameterSet.getName() + string(".startTime"));
      time_t stopTime    = m_parameterSet.getInt(m_parameterSet.getName() + string(".stopTime"));
      m_prepareTimerId = m_serverPort.setTimer(prepareTime - timeNow);
      m_startTimerId = m_serverPort.setTimer(startTime - timeNow);
      m_stopTimerId = m_serverPort.setTimer(stopTime - timeNow);
      break;
    }
  
    case F_CONNECTED:
      if(_isParentPort(port))
      {
        LOGICALDEVICEConnectEvent connectEvent;
        port.send(connectEvent);
      }
      break;

    case LOGICALDEVICE_CONNECTED:
    {
      LOGICALDEVICEConnectedEvent connectedEvent;
      if(connectedEvent.result == LD_RESULT_NO_ERROR)
      {
        _doStateTransition(LOGICALDEVICE_STATE_IDLE);
      }
      break;
    }
      
    case F_DISCONNECTED:
      port.close();
      break;

    case F_TIMER:
      break;
    
    default:
      LOG_DEBUG(formatString("LogicalDevice(%s)::initial_state, default",getName().c_str()));
      status = ::GCFEvent::NOT_HANDLED;
      break;
  }    
  if(status == ::GCFEvent::NOT_HANDLED)
  {
    TLogicalDeviceState newState;
    status = concrete_initial_state(event, port, newState);
    _doStateTransition(newState);
  }
  return status;
}

::GCFEvent::TResult LogicalDevice::idle_state(::GCFEvent& event, ::GCFPortInterface& port)
{
  LOG_DEBUG(formatString("LogicalDevice(%s)::idle_state (%s)",getName().c_str(),evtstr(event)));
  ::GCFEvent::TResult status = ::GCFEvent::HANDLED;

  switch (event.signal)
  {
    case F_INIT:
      break;
      
    case F_ENTRY:
    {
      m_logicalDeviceState = LOGICALDEVICE_STATE_IDLE;
      // open the server port to allow childs to connect
      m_serverPort.open();
      
      // send initialized event to the parent
      LOGICALDEVICEScheduledEvent scheduledEvent;
      scheduledEvent.result=LD_RESULT_NO_ERROR;//OK
      m_parentPort.send(scheduledEvent);
      
      ::GCFPVString status(LD_STATE_STRING_IDLE);
      m_propertySet->setValue(LD_PROPNAME_STATUS,status);
      break;
    }
  
    case F_ACCEPT_REQ:
    {
      boost::shared_ptr<GCFPVSSPort> client(new GCFPVSSPort);
      client->init(*this, m_serverPortName, GCFPortInterface::SPP, LOGICALDEVICE_PROTOCOL);
      m_serverPort.accept(*(client.get()));
      addChildPort(client);
      
      // add reference
      break;
    }

    case F_DISCONNECTED:
      _disconnectedHandler(port);
      break;
    
    case F_TIMER:
      _handleTimers(event,port);
      break;

    case LOGICALDEVICE_CONNECT:
    {
      LOGICALDEVICEConnectedEvent connectedEvent;
      connectedEvent.result = LD_RESULT_NO_ERROR;
      port.send(connectedEvent);
      break;
    }
      
    case LOGICALDEVICE_CLAIM:
      TRAN(LogicalDevice::claiming_state);
      concreteClaim(port);
      break;
      
    default:
      LOG_DEBUG(formatString("LogicalDevice(%s)::idle_state, default",getName().c_str()));
      status = ::GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}

::GCFEvent::TResult LogicalDevice::claiming_state(::GCFEvent& event, ::GCFPortInterface& port)
{
  LOG_DEBUG(formatString("LogicalDevice(%s)::claiming_state (%s)",getName().c_str(),evtstr(event)));
  ::GCFEvent::TResult status = ::GCFEvent::HANDLED;

  switch (event.signal)
  {
    case F_INIT:
      break;
      
    case F_ENTRY:
    {
      m_logicalDeviceState = LOGICALDEVICE_STATE_CLAIMING;
      ::GCFPVString status(LD_STATE_STRING_CLAIMING);
      m_propertySet->setValue(LD_PROPNAME_STATUS,status);
      
      // send claim event to childs
      LOGICALDEVICEClaimEvent claimEvent;
      _sendToAllChilds(claimEvent);
      break;
    }
  
    case F_DISCONNECTED:
      _disconnectedHandler(port);
      break;
      
    // the LOGICALDEVICE_CLAIMED event cannot result in a transition to 
    // the claimed state here, because the logical device may have several 
    // children that all send their LOGICALDEVICE_CLAIMED message. 
    // The parent can only enter the claimed state when all children have
    // sent their LOGICALDEVICE_CLAIMED message. That cannot be decided in
    // this baseclass.
    
    default:
      LOG_DEBUG(formatString("LogicalDevice(%s)::claiming_state, default",getName().c_str()));
      // call the implementation of the derived class
      TLogicalDeviceState newState;
      status = concrete_claiming_state(event,port,newState);
      _doStateTransition(newState);
      break;
  }

  return status;
}

::GCFEvent::TResult LogicalDevice::claimed_state(::GCFEvent& event, ::GCFPortInterface& port)
{
  LOG_DEBUG(formatString("LogicalDevice(%s)::claimed_state (%s)",getName().c_str(),evtstr(event)));
  ::GCFEvent::TResult status = ::GCFEvent::HANDLED;

  switch (event.signal)
  {
    case F_INIT:
      break;
      
    case F_ENTRY:
    {
      m_logicalDeviceState = LOGICALDEVICE_STATE_CLAIMED;
      ::GCFPVString status(LD_STATE_STRING_CLAIMED);
      m_propertySet->setValue(LD_PROPNAME_STATUS,status);

      // send claimed message to the parent.
      LOGICALDEVICEClaimedEvent claimedEvent;
      m_parentPort.send(claimedEvent);
      break;
    }
    
    case F_DISCONNECTED:
      _disconnectedHandler(port);
      break;
    
    case F_TIMER:
      _handleTimers(event,port);
      break;
   
    case LOGICALDEVICE_PREPARE:
    {
      TRAN(LogicalDevice::preparing_state);
      
      concretePrepare(port);
      break;
    }
    
    case LOGICALDEVICE_RELEASE:
      TRAN(LogicalDevice::releasing_state);
      concreteRelease(port);
      break;

    default:
      LOG_DEBUG(formatString("LogicalDevice(%s)::claimed_state, default",getName().c_str()));
      status = ::GCFEvent::NOT_HANDLED;
      break;
    }

  return status;
}

::GCFEvent::TResult LogicalDevice::preparing_state(::GCFEvent& event, ::GCFPortInterface& port)
{
  LOG_DEBUG(formatString("LogicalDevice(%s)::preparing_state (%s)",getName().c_str(),evtstr(event)));
  ::GCFEvent::TResult status = ::GCFEvent::HANDLED;

  switch (event.signal)
  {
    case F_INIT:
      break;
      
    case F_ENTRY:
    {
      m_logicalDeviceState = LOGICALDEVICE_STATE_PREPARING;
      ::GCFPVString status(LD_STATE_STRING_PREPARING);
      m_propertySet->setValue(LD_PROPNAME_STATUS,status);

      // send prepare event to childs
      LOGICALDEVICEPrepareEvent prepareEvent;
      _sendToAllChilds(prepareEvent);
      break;
    }
    
    case F_EXIT:
      break;
      
    case F_DISCONNECTED:
      _disconnectedHandler(port);
      break;
      
    // the LOGICALDEVICE_PREPARED event cannot result in a transition to 
    // the suspended state here, because the logical device may have several 
    // children that all send their LOGICALDEVICE_PREPARED message. 
    // The parent can only enter the prepared state when all children have
    // sent their LOGICALDEVICE_PREPARED message. That cannot be decided in
    // this baseclass.
    
    default:
      LOG_DEBUG(formatString("LogicalDevice(%s)::preparing_state, default",getName().c_str()));
      // call the implementation of the derived class
      TLogicalDeviceState newState;
      status = concrete_preparing_state(event,port,newState);
      _doStateTransition(newState);
      break;
  }
  return status;
}

::GCFEvent::TResult LogicalDevice::suspended_state(::GCFEvent& event, ::GCFPortInterface& port)
{
  LOG_DEBUG(formatString("LogicalDevice(%s)::suspended_state (%s)",getName().c_str(),evtstr(event)));
  ::GCFEvent::TResult status = ::GCFEvent::HANDLED;

  switch (event.signal)
  {
    case F_INIT:
      break;
      
    case F_ENTRY:
    {
      m_logicalDeviceState = LOGICALDEVICE_STATE_SUSPENDED;
      ::GCFPVString status(LD_STATE_STRING_SUSPENDED);
      m_propertySet->setValue(LD_PROPNAME_STATUS,status);

      // send to parent
      LOGICALDEVICESuspendedEvent suspendedEvent;
      m_parentPort.send(suspendedEvent);
      break;
    }
  
    case F_DISCONNECTED:
      _disconnectedHandler(port);
      break;
    
    case F_TIMER:
      _handleTimers(event,port);
      break;
    
    case LOGICALDEVICE_PREPARE:
    {
      TRAN(LogicalDevice::preparing_state);

      concretePrepare(port);
      break;
    }
    
    case LOGICALDEVICE_RESUME:
    {
      // send resume event to childs
      LOGICALDEVICEResumeEvent resumeEvent;
      _sendToAllChilds(resumeEvent);

      TRAN(LogicalDevice::active_state);
      concreteResume(port);
      break;
    }
    
    case LOGICALDEVICE_RELEASE:
    {
      // send release event to childs
      LOGICALDEVICEResumeEvent releaseEvent;
      _sendToAllChilds(releaseEvent);

      TRAN(LogicalDevice::releasing_state);
      concreteRelease(port);
      break;
    }
    
    default:
      LOG_DEBUG(formatString("LogicalDevice(%s)::suspended_state, default",getName().c_str()));
      status = ::GCFEvent::NOT_HANDLED;
      break;
  }
  return status;
}

::GCFEvent::TResult LogicalDevice::active_state(::GCFEvent& event, ::GCFPortInterface& port)
{
  LOG_DEBUG(formatString("LogicalDevice(%s)::active_state (%s)",getName().c_str(),evtstr(event)));
  ::GCFEvent::TResult status = ::GCFEvent::HANDLED;

  switch (event.signal)
  {
    case F_INIT:
      break;      
      
    case F_ENTRY:
    {
      m_logicalDeviceState = LOGICALDEVICE_STATE_ACTIVE;
      ::GCFPVString status(LD_STATE_STRING_ACTIVE);
      m_propertySet->setValue(LD_PROPNAME_STATUS,status);

      // send resumed message to parent.
      LOGICALDEVICEResumedEvent resumedEvent;
      m_parentPort.send(resumedEvent);
      break;
    }
  
    case F_DISCONNECTED:
      _disconnectedHandler(port);
      break;
    
    case F_TIMER:
      _handleTimers(event,port);
      break;
    
    case LOGICALDEVICE_PREPARE:
      // invalid message in this state
      LOG_DEBUG(formatString("LogicalDevice(%s)::active_state, PREPARE NOT ALLOWED",getName().c_str()));
      break;
      
    case LOGICALDEVICE_SUSPEND:
    {
      TRAN(LogicalDevice::suspended_state);

      // send suspend event to childs
      LOGICALDEVICESuspendEvent suspendEvent;
      _sendToAllChilds(suspendEvent);

      concreteSuspend(port);
      break;
    }
    
    default:
      LOG_DEBUG(formatString("LogicalDevice(%s)::active_state, default",getName().c_str()));
      status = concrete_active_state(event,port);
      break;
  }

  return status;
}

::GCFEvent::TResult LogicalDevice::releasing_state(::GCFEvent& event, ::GCFPortInterface& port)
{
  LOG_DEBUG(formatString("LogicalDevice(%s)::releasing_state (%s)",getName().c_str(),evtstr(event)));
  ::GCFEvent::TResult status = ::GCFEvent::HANDLED;

  switch (event.signal)
  {
    case F_INIT:
      break;
      
    case F_ENTRY:
    {
      m_logicalDeviceState = LOGICALDEVICE_STATE_RELEASING;
      ::GCFPVString status(LD_STATE_STRING_RELEASING);
      m_propertySet->setValue(LD_PROPNAME_STATUS,status);
      break;
    }
  
    case F_DISCONNECTED:
      _disconnectedHandler(port);
      break;

    // the LOGICALDEVICE_RELEASED event cannot result in a transition to 
    // the idle state here, because the logical device may have several 
    // children that all send their LOGICALDEVICE_RELEASED message. 
    // The parent can only enter the idle state when all children have
    // sent their LOGICALDEVICE_RELEASED message. That cannot be decided in
    // this baseclass.
    
    default:
      LOG_DEBUG(formatString("LogicalDevice(%s)::releasing_state, default",getName().c_str()));
      // call the implementation of the derived class
      TLogicalDeviceState newState;
      status = concrete_releasing_state(event,port,newState);
      _doStateTransition(newState);
      break;
  }

  return status;
}

};
};
