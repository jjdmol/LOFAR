//#  AVTLogicalDevice.cc: Implementation of the Virtual Telescope task
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
#include <APLCommon/APL_Defines.h>

#include "AVTLogicalDevice.h"
#include "AVTPropertyDefines.h"

#define DECLARE_SIGNAL_NAMES
#include "LogicalDevice_Protocol.ph"

using namespace LOFAR;
using namespace AVT;
using namespace std;

AVTLogicalDevice::AVTLogicalDevice(string& taskName, 
                                   const string& scope,
                                   const string& type,
                                   const string& APCName) :
  GCFTask((State)&AVTLogicalDevice::initial_state,taskName),
  AVTPropertySetAnswerHandlerInterface(),
  m_propertySetAnswer(*this),
  m_properties(scope.c_str(),type.c_str(),false,&m_propertySetAnswer),
  m_APC(APCName),
  m_serverPortName(taskName+string("_server")),
  m_logicalDeviceServerPort(*this, m_serverPortName, GCFPortInterface::SPP, LOGICALDEVICE_PROTOCOL),
  m_clientInterTaskPorts(),
  m_apcLoaded(false),
  m_logicalDeviceState(LOGICALDEVICE_STATE_IDLE)
{
  registerProtocol(LOGICALDEVICE_PROTOCOL, LOGICALDEVICE_PROTOCOL_signalnames);
  LOG_TRACE_FLOW(formatString("AVTLogicalDevice(%s)::AVTLogicalDevice",getName().c_str()));
  
  m_properties.enable();
}


AVTLogicalDevice::~AVTLogicalDevice()
{
  LOG_TRACE_FLOW(formatString("AVTLogicalDevice(%s)::~AVTLogicalDevice",getName().c_str()));
  m_properties.disable();
}

string& AVTLogicalDevice::getServerPortName()
{
  return m_serverPortName;
}

void AVTLogicalDevice::addClientInterTaskPort(APLInterTaskPort* clientPort)
{
  m_clientInterTaskPorts.push_back(clientPort);
}

bool AVTLogicalDevice::isPrepared(vector<string>& /*parameters*/)
{
  return false;
}

TLogicalDeviceState AVTLogicalDevice::getLogicalDeviceState() const
{
  return m_logicalDeviceState;
}

bool AVTLogicalDevice::_isLogicalDeviceServerPort(GCFPortInterface& /*port*/)
{
  return true; //(&port == &m_logicalDeviceServerPort); // comparing two pointers. yuck?
}
   
void AVTLogicalDevice::_disconnectedHandler(GCFPortInterface& port)
{
  port.close();
/*
 *   if(_isLogicalDeviceServerPort(port))
  {
  }
  else
  {
*/
    concreteDisconnected(port);
//  }
}

bool AVTLogicalDevice::isAPCLoaded() const
{
  return m_apcLoaded;
}

void AVTLogicalDevice::apcLoaded()
{
  m_apcLoaded=true;
}

GCFEvent::TResult AVTLogicalDevice::initial_state(GCFEvent& event, GCFPortInterface& port)
{
  LOG_TRACE_FLOW(formatString("AVTLogicalDevice(%s)::initial_state (%s)",getName().c_str(),evtstr(event)));

  GCFEvent::TResult status = GCFEvent::HANDLED;
  
  switch (event.signal)
  {
    case F_INIT:
      break;

    case F_ENTRY:
    {
      m_logicalDeviceState = LOGICALDEVICE_STATE_IDLE;
      GCFPVString status(LD_STATE_STRING_INITIAL);
      m_properties.setValue(PROPNAME_STATUS,status);
      break;
    }
  
    case F_CONNECTED:
      break;

    case F_DISCONNECTED:
      port.close();
      break;

    case F_TIMER:
      break;

    default:
      LOG_TRACE_FLOW(formatString("AVTLogicalDevice(%s)::initial_state, default",getName().c_str()));
      status = GCFEvent::NOT_HANDLED;
      break;
  }    
  // call the implementation of the derived class
  // transition to the idle state is done in the derived class
  status = concrete_initial_state(event,port);

  return status;
}

GCFEvent::TResult AVTLogicalDevice::idle_state(GCFEvent& event, GCFPortInterface& port)
{
  LOG_TRACE_FLOW(formatString("AVTLogicalDevice(%s)::idle_state (%s)",getName().c_str(),evtstr(event)));
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (event.signal)
  {
    case F_INIT:
      break;
      
    case F_ENTRY:
    {
      m_logicalDeviceState = LOGICALDEVICE_STATE_IDLE;
      // open the server port to allow clients to connect
      m_logicalDeviceServerPort.open();
      LOGICALDEVICEInitializedEvent initializedEvent;
//      m_logicalDeviceServerPort.send(initializedEvent);
      // send to all clients
      vector<APLInterTaskPort*>::iterator it=m_clientInterTaskPorts.begin();
      while(it != m_clientInterTaskPorts.end())
      {
        (*it)->sendBack(initializedEvent);
        ++it;
      }
      
      GCFPVString status(LD_STATE_STRING_IDLE);
      m_properties.setValue(PROPNAME_STATUS,status);
      break;
    }
  
    case F_DISCONNECTED:
      _disconnectedHandler(port);
      break;
    
    case LOGICALDEVICE_CLAIM:
      TRAN(AVTLogicalDevice::claiming_state);
      concreteClaim(port);
      break;

    default:
      LOG_TRACE_FLOW(formatString("AVTLogicalDevice(%s)::idle_state, default",getName().c_str()));
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}

GCFEvent::TResult AVTLogicalDevice::claiming_state(GCFEvent& event, GCFPortInterface& port)
{
  LOG_TRACE_FLOW(formatString("AVTLogicalDevice(%s)::claiming_state (%s)",getName().c_str(),evtstr(event)));
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (event.signal)
  {
    case F_INIT:
      break;
      
    case F_ENTRY:
    {
      m_logicalDeviceState = LOGICALDEVICE_STATE_CLAIMING;
      GCFPVString status(LD_STATE_STRING_CLAIMING);
      m_properties.setValue(PROPNAME_STATUS,status);
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
      LOG_TRACE_FLOW(formatString("AVTLogicalDevice(%s)::claiming_state, default",getName().c_str()));
      // call the implementation of the derived class
      bool stateFinished=false;
      status = concrete_claiming_state(event,port,stateFinished);
      if(stateFinished)
      {
        TRAN(AVTLogicalDevice::claimed_state);
      }
      break;
  }

  return status;
}

GCFEvent::TResult AVTLogicalDevice::claimed_state(GCFEvent& event, GCFPortInterface& port)
{
  LOG_TRACE_FLOW(formatString("AVTLogicalDevice(%s)::claimed_state (%s)",getName().c_str(),evtstr(event)));
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (event.signal)
  {
    case F_INIT:
      break;
      
    case F_ENTRY:
    {
      m_logicalDeviceState = LOGICALDEVICE_STATE_CLAIMED;
      GCFPVString status(LD_STATE_STRING_CLAIMED);
      m_properties.setValue(PROPNAME_STATUS,status);

      // send claimed message to all my clients.
      LOGICALDEVICEClaimedEvent claimedEvent;
//      m_logicalDeviceServerPort.send(claimedEvent);
      // send to all clients
      vector<APLInterTaskPort*>::iterator it=m_clientInterTaskPorts.begin();
      while(it != m_clientInterTaskPorts.end())
      {
        (*it)->sendBack(claimedEvent);
        ++it;
      }
      break;
    }
    
    case F_DISCONNECTED:
      _disconnectedHandler(port);
      break;
   
    case LOGICALDEVICE_PREPARE:
    {
      TRAN(AVTLogicalDevice::preparing_state);
      
      string parameters;
      LOGICALDEVICEPrepareEvent prepareEvent(event);
      concretePrepare(port,prepareEvent.parameters);
      break;
    }
    
    case LOGICALDEVICE_RELEASE:
      TRAN(AVTLogicalDevice::releasing_state);
      concreteRelease(port);
      break;

    default:
      LOG_TRACE_FLOW(formatString("AVTLogicalDevice(%s)::claimed_state, default",getName().c_str()));
      status = GCFEvent::NOT_HANDLED;
      break;
    }

  return status;
}

GCFEvent::TResult AVTLogicalDevice::preparing_state(GCFEvent& event, GCFPortInterface& port)
{
  LOG_TRACE_FLOW(formatString("AVTLogicalDevice(%s)::preparing_state (%s)",getName().c_str(),evtstr(event)));
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (event.signal)
  {
    case F_INIT:
      break;
      
    case F_ENTRY:
    {
      m_logicalDeviceState = LOGICALDEVICE_STATE_PREPARING;
      GCFPVString status(LD_STATE_STRING_PREPARING);
      m_properties.setValue(PROPNAME_STATUS,status);
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
      LOG_TRACE_FLOW(formatString("AVTLogicalDevice(%s)::preparing_state, default",getName().c_str()));
      // call the implementation of the derived class
      bool stateFinished=false;
      bool error=false;
      status = concrete_preparing_state(event,port,stateFinished,error);
      if(error)
      {
        TRAN(AVTLogicalDevice::claimed_state);
      }
      else
      {
        if(stateFinished)
        {
          TRAN(AVTLogicalDevice::suspended_state);
          LOGICALDEVICEPreparedEvent preparedEvent;
//          m_logicalDeviceServerPort.send(preparedEvent);
          // send to all clients
          vector<APLInterTaskPort*>::iterator it=m_clientInterTaskPorts.begin();
          while(it != m_clientInterTaskPorts.end())
          {
            (*it)->sendBack(preparedEvent);
            ++it;
          }
        }
      }
      break;
  }
  return status;
}

GCFEvent::TResult AVTLogicalDevice::suspended_state(GCFEvent& event, GCFPortInterface& port)
{
  LOG_TRACE_FLOW(formatString("AVTLogicalDevice(%s)::suspended_state (%s)",getName().c_str(),evtstr(event)));
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (event.signal)
  {
    case F_INIT:
      break;
      
    case F_ENTRY:
    {
      m_logicalDeviceState = LOGICALDEVICE_STATE_SUSPENDED;
      GCFPVString status(LD_STATE_STRING_SUSPENDED);
      m_properties.setValue(PROPNAME_STATUS,status);
      // send to all clients
      LOGICALDEVICESuspendedEvent suspendedEvent;
      vector<APLInterTaskPort*>::iterator it=m_clientInterTaskPorts.begin();
      while(it != m_clientInterTaskPorts.end())
      {
        (*it)->sendBack(suspendedEvent);
        ++it;
      }
      break;
    }
  
    case F_DISCONNECTED:
      _disconnectedHandler(port);
      break;
    
    case LOGICALDEVICE_PREPARE:
    {
      TRAN(AVTLogicalDevice::preparing_state);

      string parameters;
      LOGICALDEVICEPrepareEvent prepareEvent(event);
      concretePrepare(port,prepareEvent.parameters);
      break;
    }
    
    case LOGICALDEVICE_RESUME:
      TRAN(AVTLogicalDevice::active_state);
      concreteResume(port);
      break;

    case LOGICALDEVICE_RELEASE:
      TRAN(AVTLogicalDevice::releasing_state);
      concreteRelease(port);
      break;

    default:
      LOG_TRACE_FLOW(formatString("AVTLogicalDevice(%s)::suspended_state, default",getName().c_str()));
      status = GCFEvent::NOT_HANDLED;
      break;
  }
  return status;
}

GCFEvent::TResult AVTLogicalDevice::active_state(GCFEvent& event, GCFPortInterface& port)
{
  LOG_TRACE_FLOW(formatString("AVTLogicalDevice(%s)::active_state (%s)",getName().c_str(),evtstr(event)));
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (event.signal)
  {
    case F_INIT:
      break;      
      
    case F_ENTRY:
    {
      m_logicalDeviceState = LOGICALDEVICE_STATE_ACTIVE;
      GCFPVString status(LD_STATE_STRING_ACTIVE);
      m_properties.setValue(PROPNAME_STATUS,status);

      // send resumed message to all my clients.
      LOGICALDEVICEResumedEvent resumedEvent;
//      m_logicalDeviceServerPort.send(resumedEvent);
      // send to all clients
      vector<APLInterTaskPort*>::iterator it=m_clientInterTaskPorts.begin();
      while(it != m_clientInterTaskPorts.end())
      {
        (*it)->sendBack(resumedEvent);
        ++it;
      }
      break;
    }
  
    case F_DISCONNECTED:
      _disconnectedHandler(port);
      break;
    
    case LOGICALDEVICE_PREPARE:
      // invalid message in this state
      LOG_TRACE_FLOW(formatString("AVTLogicalDevice(%s)::active_state, PREPARE NOT ALLOWED",getName().c_str()));
      break;
      
    case LOGICALDEVICE_SUSPEND:
      TRAN(AVTLogicalDevice::suspended_state);
      concreteSuspend(port);
      break;

    default:
      LOG_TRACE_FLOW(formatString("AVTLogicalDevice(%s)::active_state, default",getName().c_str()));
      status = concrete_active_state(event,port);
      break;
  }

  return status;
}

GCFEvent::TResult AVTLogicalDevice::releasing_state(GCFEvent& event, GCFPortInterface& port)
{
  LOG_TRACE_FLOW(formatString("AVTLogicalDevice(%s)::releasing_state (%s)",getName().c_str(),evtstr(event)));
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (event.signal)
  {
    case F_INIT:
      break;
      
    case F_ENTRY:
    {
      m_logicalDeviceState = LOGICALDEVICE_STATE_RELEASING;
      GCFPVString status(LD_STATE_STRING_RELEASING);
      m_properties.setValue(PROPNAME_STATUS,status);
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
      LOG_TRACE_FLOW(formatString("AVTLogicalDevice(%s)::releasing_state, default",getName().c_str()));
      // call the implementation of the derived class
      bool stateFinished=false;
      status = concrete_releasing_state(event,port,stateFinished);
      if(stateFinished)
      {
        TRAN(AVTLogicalDevice::idle_state);
        LOGICALDEVICEReleasedEvent releasedEvent;
//        m_logicalDeviceServerPort.send(releasedEvent);
        // send to all clients
        vector<APLInterTaskPort*>::iterator it=m_clientInterTaskPorts.begin();
        while(it != m_clientInterTaskPorts.end())
        {
          (*it)->sendBack(releasedEvent);
          ++it;
        }
      }
      break;
  }

  return status;
}
