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

#include "../../../APLCommon/src/APL_Defines.h"
#include "AVTLogicalDevice.h"
#define DECLARE_SIGNAL_NAMES
#include "LogicalDevice_Protocol.ph"

AVTLogicalDevice::AVTLogicalDevice(string& taskName, 
                                   const TPropertySet& primaryPropertySet,
                                   const string& APCName,
                                   const string& APCScope) :
  GCFTask((State)&AVTLogicalDevice::initial_state,taskName),
  AVTPropertySetAnswerHandlerInterface(),
  AVTAPCAnswerHandlerInterface(),
  m_propertySetAnswer(*this),
  m_APCAnswer(*this),
  m_properties(primaryPropertySet,&m_propertySetAnswer),
  m_APC(APCName,APCScope,&m_APCAnswer),
  m_serverPortName(taskName+string("_server")),
  m_logicalDeviceServerPort(*this, m_serverPortName, GCFPortInterface::SPP, LOGICALDEVICE_PROTOCOL)
{
  LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTLogicalDevice::AVTLogicalDevice(%s)",getName().c_str()));
}


AVTLogicalDevice::~AVTLogicalDevice()
{
  LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTLogicalDevice::~AVTLogicalDevice(%s)",getName().c_str()));
}

string& AVTLogicalDevice::getServerPortName()
{
  return m_serverPortName;
}

bool AVTLogicalDevice::_isLogicalDeviceServerPort(GCFPortInterface& port)
{
  return (&port == &m_logicalDeviceServerPort); // comparing two pointers. yuck?
}
   
void AVTLogicalDevice::_disconnectedHandler(GCFPortInterface& port)
{
  if(_isLogicalDeviceServerPort(port))
  {
  }
  else
  {
    concreteDisconnected(port);
  }
}

GCFEvent::TResult AVTLogicalDevice::initial_state(GCFEvent& event, GCFPortInterface& port)
{
  LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTLogicalDevice::initial_state (%s)",getName().c_str()));

  GCFEvent::TResult status = GCFEvent::HANDLED;
  
  if(_isLogicalDeviceServerPort(port))
  {
    switch (event.signal)
    {
      case F_INIT_SIG:
        LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTLogicalDevice::initial_state, F_INIT_SIG (%s)",getName().c_str()));
        break;
  
      case F_ENTRY_SIG:
        LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTLogicalDevice::initial_state, F_ENTRY_SIG (%s)",getName().c_str()));
        // open all ports
        m_logicalDeviceServerPort.open();
        break;
  
      case F_CONNECTED_SIG:
        LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTLogicalDevice::initial_state, F_CONNECTED_SIG (%s)",getName().c_str()));
        break;
  
      case F_DISCONNECTED_SIG:
        LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTLogicalDevice::initial_state, F_DISCONNECTED_SIG (%s)",getName().c_str()));
        m_logicalDeviceServerPort.setTimer(1.0); // try again after 1 second
        break;
  
      case F_TIMER_SIG:
        LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTLogicalDevice::initial_state, F_TIMER_SIG (%s)",getName().c_str()));
        m_logicalDeviceServerPort.open(); // try again
        break;
  
      default:
        LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTLogicalDevice::initial_state, default (%s)",getName().c_str()));
        status = GCFEvent::NOT_HANDLED;
        break;
    }
  }
  else
  {
    // call the implementation of the derived class
    // transition to the idle state is done in the derived class
    status = concrete_initial_state(event,port);
  }
  return status;
}

GCFEvent::TResult AVTLogicalDevice::idle_state(GCFEvent& event, GCFPortInterface& port)
{
  LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTLogicalDevice::idle_state (%s)",getName().c_str()));
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (event.signal)
  {
    case F_DISCONNECTED_SIG:
      LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTLogicalDevice::idle_state, F_DISCONNECTED_SIG (%s)",getName().c_str()));
      _disconnectedHandler(port);
      break;
    
    case LOGICALDEVICE_CLAIM:
      LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTLogicalDevice::idle_state, LOGICALDEVICE_CLAIM (%s)",getName().c_str()));
      if(_isLogicalDeviceServerPort(port))
      {
        TRAN(AVTLogicalDevice::claiming_state);
        concreteClaim(port);
      }
      else
      {
        status = GCFEvent::NOT_HANDLED;
      }
      break;

    default:
      LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTLogicalDevice::idle_state, default (%s)",getName().c_str()));
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}

GCFEvent::TResult AVTLogicalDevice::claiming_state(GCFEvent& event, GCFPortInterface& port)
{
  LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTLogicalDevice::claiming_state (%s)",getName().c_str()));
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (event.signal)
  {
    case F_DISCONNECTED_SIG:
      LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTLogicalDevice::claiming_state, F_DISCONNECTED_SIG (%s)",getName().c_str()));
      _disconnectedHandler(port);
      break;
      
    // the LOGICALDEVICE_CLAIMED event cannot result in a transition to 
    // the claimed state here, because the logical device may have several 
    // children that all send their LOGICALDEVICE_CLAIMED message. 
    // The parent can only enter the claimed state when all children have
    // sent their LOGICALDEVICE_CLAIMED message. That cannot be decided in
    // this baseclass.
    
    default:
      LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTLogicalDevice::claiming_state, default (%s)",getName().c_str()));
      // call the implementation of the derived class
      bool stateFinished=false;
      status = concrete_claiming_state(event,port,stateFinished);
      if(stateFinished)
      {
        port.send(LOGICALDEVICE_CLAIMED);
        TRAN(AVTLogicalDevice::claimed_state);
      }
      break;
  }

  return status;
}

GCFEvent::TResult AVTLogicalDevice::claimed_state(GCFEvent& event, GCFPortInterface& port)
{
  LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTLogicalDevice::claimed_state (%s)",getName().c_str()));
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (event.signal)
  {
    case F_DISCONNECTED_SIG:
      LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTLogicalDevice::claimed_state, F_DISCONNECTED_SIG (%s)",getName().c_str()));
      _disconnectedHandler(port);
      break;
   
    case LOGICALDEVICE_PREPARE:
      LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTLogicalDevice::claimed_state, LOGICALDEVICE_PREPARE (%s)",getName().c_str()));
      if(_isLogicalDeviceServerPort(port))
      {
        TRAN(AVTLogicalDevice::preparing_state);
        
        string parameters;
        LOGICALDEVICEPrepareEvent* pPrepareEvent=static_cast<LOGICALDEVICEPrepareEvent*>(&event);
        if(pPrepareEvent!=0)
        {
          parameters=pPrepareEvent->parameters;
        }
        concretePrepare(port,parameters);
      }
      else
      {
        status = GCFEvent::NOT_HANDLED;
      }
      break;

    case LOGICALDEVICE_RELEASE:
      LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTLogicalDevice::claimed_state, LOGICALDEVICE_RELEASE (%s)",getName().c_str()));
      if(_isLogicalDeviceServerPort(port))
      {
        TRAN(AVTLogicalDevice::releasing_state);
        concreteRelease(port);
      }
      else
      {
        status = GCFEvent::NOT_HANDLED;
      }
      break;

    default:
      LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTLogicalDevice::claimed_state, default (%s)",getName().c_str()));
      status = GCFEvent::NOT_HANDLED;
      break;
    }

  return status;
}

GCFEvent::TResult AVTLogicalDevice::preparing_state(GCFEvent& event, GCFPortInterface& port)
{
  LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTLogicalDevice::preparing_state (%s)",getName().c_str()));
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (event.signal)
  {
    case F_DISCONNECTED_SIG:
      LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTLogicalDevice::preparing_state, F_DISCONNECTED_SIG (%s)",getName().c_str()));
      _disconnectedHandler(port);
      break;
      
    // the LOGICALDEVICE_PREPARED event cannot result in a transition to 
    // the prepared state here, because the logical device may have several 
    // children that all send their LOGICALDEVICE_PREPARED message. 
    // The parent can only enter the prepared state when all children have
    // sent their LOGICALDEVICE_PREPARED message. That cannot be decided in
    // this baseclass.
    
    default:
      LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTLogicalDevice::preparing_state, default (%s)",getName().c_str()));
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
          port.send(LOGICALDEVICE_PREPARED);
          TRAN(AVTLogicalDevice::suspended_state);
        }
      }
      break;
  }
  return status;
}

GCFEvent::TResult AVTLogicalDevice::suspended_state(GCFEvent& event, GCFPortInterface& port)
{
  LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTLogicalDevice::suspended_state (%s)",getName().c_str()));
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (event.signal)
  {
    case F_DISCONNECTED_SIG:
      LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTLogicalDevice::suspended_state, F_DISCONNECTED_SIG (%s)",getName().c_str()));
      _disconnectedHandler(port);
      break;
    
    case LOGICALDEVICE_PREPARE:
      LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTLogicalDevice::suspended_state, LOGICALDEVICE_PREPARE (%s)",getName().c_str()));
      if(_isLogicalDeviceServerPort(port))
      {
        TRAN(AVTLogicalDevice::preparing_state);

        string parameters;
        LOGICALDEVICEPrepareEvent* pPrepareEvent=static_cast<LOGICALDEVICEPrepareEvent*>(&event);
        if(pPrepareEvent!=0)
        {
          parameters=pPrepareEvent->parameters;
        }
        concretePrepare(port,parameters);
      }
      else
      {
        status = GCFEvent::NOT_HANDLED;
      }
      break;

    case LOGICALDEVICE_RESUME:
      LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTLogicalDevice::suspended_state, LOGICALDEVICE_RESUME (%s)",getName().c_str()));
      if(_isLogicalDeviceServerPort(port))
      {
        TRAN(AVTLogicalDevice::active_state);
        concreteResume(port);
      }
      else
      {
        status = GCFEvent::NOT_HANDLED;
      }
      break;

    case LOGICALDEVICE_RELEASE:
      LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTLogicalDevice::suspended_state, LOGICALDEVICE_RELEASE (%s)",getName().c_str()));
      if(_isLogicalDeviceServerPort(port))
      {
        TRAN(AVTLogicalDevice::releasing_state);
        concreteRelease(port);
      }
      else
      {
        status = GCFEvent::NOT_HANDLED;
      }
      break;

    default:
      LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTLogicalDevice::suspended_state, default (%s)",getName().c_str()));
      status = GCFEvent::NOT_HANDLED;
      break;
  }
  return status;
}

GCFEvent::TResult AVTLogicalDevice::active_state(GCFEvent& event, GCFPortInterface& port)
{
  LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTLogicalDevice::active_state (%s)",getName().c_str()));
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (event.signal)
  {
    case F_DISCONNECTED_SIG:
      LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTLogicalDevice::active_state, F_DISCONNECTED_SIG (%s)",getName().c_str()));
      _disconnectedHandler(port);
      break;
    
    case LOGICALDEVICE_SUSPEND:
      LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTLogicalDevice::active_state, LOGICALDEVICE_SUSPEND (%s)",getName().c_str()));
      if(_isLogicalDeviceServerPort(port))
      {
        TRAN(AVTLogicalDevice::suspended_state);
        concreteSuspend(port);
      }
      else
      {
        status = GCFEvent::NOT_HANDLED;
      }
      break;

    default:
      LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTLogicalDevice::active_state, default (%s)",getName().c_str()));
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}

GCFEvent::TResult AVTLogicalDevice::releasing_state(GCFEvent& event, GCFPortInterface& port)
{
  LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTLogicalDevice::releasing_state (%s)",getName().c_str()));
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (event.signal)
  {
    case F_DISCONNECTED_SIG:
      LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTLogicalDevice::releasing_state, F_DISCONNECTED_SIG (%s)",getName().c_str()));
      _disconnectedHandler(port);
      break;

    // the LOGICALDEVICE_RELEASED event cannot result in a transition to 
    // the idle state here, because the logical device may have several 
    // children that all send their LOGICALDEVICE_RELEASED message. 
    // The parent can only enter the idle state when all children have
    // sent their LOGICALDEVICE_RELEASED message. That cannot be decided in
    // this baseclass.
    
    default:
      LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTLogicalDevice::releasing_state, default (%s)",getName().c_str()));
      // call the implementation of the derived class
      bool stateFinished=false;
      status = concrete_releasing_state(event,port,stateFinished);
      if(stateFinished)
      {
        port.send(LOGICALDEVICE_RELEASED);
        TRAN(AVTLogicalDevice::idle_state);
      }
      break;
  }

  return status;
}
