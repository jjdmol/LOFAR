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

#include "../../../APLCommon/src/APL_Defines.h"
#include "AVTLogicalDevice.h"
#define DECLARE_SIGNAL_NAMES
#include "LogicalDevice_Protocol.ph"

using namespace LOFAR;
using namespace AVT;
using namespace std;

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
  m_logicalDeviceServerPort(*this, m_serverPortName, GCFPortInterface::SPP, LOGICALDEVICE_PROTOCOL),
  m_clientInterTaskPort(0),
  m_apcLoaded(false)
{
  registerProtocol(LOGICALDEVICE_PROTOCOL, LOGICALDEVICE_PROTOCOL_signalnames);
  LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTLogicalDevice(%s)::AVTLogicalDevice",getName().c_str()));
  
  m_properties.load();
}


AVTLogicalDevice::~AVTLogicalDevice()
{
  LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTLogicalDevice(%s)::~AVTLogicalDevice",getName().c_str()));
  m_clientInterTaskPort=0;
  m_APC.unload();
  m_properties.unload();
}

string& AVTLogicalDevice::getServerPortName()
{
  return m_serverPortName;
}

void AVTLogicalDevice::setClientInterTaskPort(APLInterTaskPort* clientPort)
{
  m_clientInterTaskPort=clientPort;
}

bool AVTLogicalDevice::isPrepared(vector<string>& /*parameters*/)
{
  return false;
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
  LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTLogicalDevice(%s)::initial_state (%s)",getName().c_str(),evtstr(event)));

  GCFEvent::TResult status = GCFEvent::HANDLED;
  
  switch (event.signal)
  {
    case F_INIT:
      break;

    case F_ENTRY:
    {
      GCFPVString status("Initial");
      m_properties.setValue("status",status);
      break;
    }
  
    case F_CONNECTED:
      break;

    case F_DISCONNECTED:
      break;

    case F_TIMER:
      break;

    default:
      LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTLogicalDevice(%s)::initial_state, default",getName().c_str()));
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
  LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTLogicalDevice(%s)::idle_state (%s)",getName().c_str(),evtstr(event)));
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (event.signal)
  {
    case F_INIT:
      break;
      
    case F_ENTRY:
    {
      // open the server port to allow clients to connect
      m_logicalDeviceServerPort.open();
      LOGICALDEVICEInitializedEvent initializedEvent;
//      m_logicalDeviceServerPort.send(initializedEvent);
      if(m_clientInterTaskPort!=0)
      {
        m_clientInterTaskPort->sendBack(initializedEvent);
      }
      
      GCFPVString status("Idle");
      m_properties.setValue("status",status);
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
      LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTLogicalDevice(%s)::idle_state, default",getName().c_str()));
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}

GCFEvent::TResult AVTLogicalDevice::claiming_state(GCFEvent& event, GCFPortInterface& port)
{
  LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTLogicalDevice(%s)::claiming_state (%s)",getName().c_str(),evtstr(event)));
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (event.signal)
  {
    case F_INIT:
      break;
      
    case F_ENTRY:
    {
      GCFPVString status("Claiming");
      m_properties.setValue("status",status);
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
      LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTLogicalDevice(%s)::claiming_state, default",getName().c_str()));
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
  LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTLogicalDevice(%s)::claimed_state (%s)",getName().c_str(),evtstr(event)));
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (event.signal)
  {
    case F_INIT:
      break;
      
    case F_ENTRY:
    {
      GCFPVString status("Claimed");
      m_properties.setValue("status",status);

      // send claimed message to all my clients.
      LOGICALDEVICEClaimedEvent claimedEvent;
//      m_logicalDeviceServerPort.send(claimedEvent);
      if(m_clientInterTaskPort!=0)
      {
        m_clientInterTaskPort->sendBack(claimedEvent);
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
      LOGICALDEVICEPrepareEvent* pPrepareEvent=static_cast<LOGICALDEVICEPrepareEvent*>(&event);
      if(pPrepareEvent!=0)
      {
        parameters=pPrepareEvent->parameters;
      }
      concretePrepare(port,parameters);
      break;
    }
    
    case LOGICALDEVICE_RELEASE:
      TRAN(AVTLogicalDevice::releasing_state);
      concreteRelease(port);
      break;

    default:
      LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTLogicalDevice(%s)::claimed_state, default",getName().c_str()));
      status = GCFEvent::NOT_HANDLED;
      break;
    }

  return status;
}

GCFEvent::TResult AVTLogicalDevice::preparing_state(GCFEvent& event, GCFPortInterface& port)
{
  LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTLogicalDevice(%s)::preparing_state (%s)",getName().c_str(),evtstr(event)));
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (event.signal)
  {
    case F_INIT:
      break;
      
    case F_ENTRY:
    {
      GCFPVString status("Preparing");
      m_properties.setValue("status",status);
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
      LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTLogicalDevice(%s)::preparing_state, default",getName().c_str()));
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
          if(m_clientInterTaskPort!=0)
          {
            m_clientInterTaskPort->sendBack(preparedEvent);
          }
        }
      }
      break;
  }
  return status;
}

GCFEvent::TResult AVTLogicalDevice::suspended_state(GCFEvent& event, GCFPortInterface& port)
{
  LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTLogicalDevice(%s)::suspended_state (%s)",getName().c_str(),evtstr(event)));
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (event.signal)
  {
    case F_INIT:
      break;
      
    case F_ENTRY:
    {
      GCFPVString status("Suspended");
      m_properties.setValue("status",status);
      if(m_clientInterTaskPort!=0)
      {
        LOGICALDEVICESuspendedEvent suspendedEvent;
        m_clientInterTaskPort->sendBack(suspendedEvent);
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
      LOGICALDEVICEPrepareEvent* pPrepareEvent=static_cast<LOGICALDEVICEPrepareEvent*>(&event);
      if(pPrepareEvent!=0)
      {
        parameters=pPrepareEvent->parameters;
      }
      concretePrepare(port,parameters);
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
      LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTLogicalDevice(%s)::suspended_state, default",getName().c_str()));
      status = GCFEvent::NOT_HANDLED;
      break;
  }
  return status;
}

GCFEvent::TResult AVTLogicalDevice::active_state(GCFEvent& event, GCFPortInterface& port)
{
  LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTLogicalDevice(%s)::active_state (%s)",getName().c_str(),evtstr(event)));
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (event.signal)
  {
    case F_INIT:
      
    case F_ENTRY:
    {
      GCFPVString status("Active");
      m_properties.setValue("status",status);

      // send resumed message to all my clients.
      LOGICALDEVICEResumedEvent resumedEvent;
//      m_logicalDeviceServerPort.send(resumedEvent);
      if(m_clientInterTaskPort!=0)
      {
        m_clientInterTaskPort->sendBack(resumedEvent);
      }
      break;
    }
  
    case F_DISCONNECTED:
      _disconnectedHandler(port);
      break;
    
    case LOGICALDEVICE_PREPARE:
      // invalid message in this state
      LOFAR_LOG_ERROR(VT_STDOUT_LOGGER,("AVTLogicalDevice(%s)::active_state, PREPARE NOT ALLOWED",getName().c_str()));
      break;
      
    case LOGICALDEVICE_SUSPEND:
      TRAN(AVTLogicalDevice::suspended_state);
      concreteSuspend(port);
      break;

    default:
      LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTLogicalDevice(%s)::active_state, default",getName().c_str()));
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}

GCFEvent::TResult AVTLogicalDevice::releasing_state(GCFEvent& event, GCFPortInterface& port)
{
  LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTLogicalDevice(%s)::releasing_state (%s)",getName().c_str(),evtstr(event)));
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (event.signal)
  {
    case F_INIT:
      break;
      
    case F_ENTRY:
    {
      GCFPVString status("Releasing");
      m_properties.setValue("status",status);
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
      LOFAR_LOG_TRACE(VT_STDOUT_LOGGER,("AVTLogicalDevice(%s)::releasing_state, default",getName().c_str()));
      // call the implementation of the derived class
      bool stateFinished=false;
      status = concrete_releasing_state(event,port,stateFinished);
      if(stateFinished)
      {
        TRAN(AVTLogicalDevice::idle_state);
        LOGICALDEVICEReleasedEvent releasedEvent;
//        m_logicalDeviceServerPort.send(releasedEvent);
        if(m_clientInterTaskPort!=0)
        {
          m_clientInterTaskPort->sendBack(releasedEvent);
        }
      }
      break;
  }

  return status;
}
