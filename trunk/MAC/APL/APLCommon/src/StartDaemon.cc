//#  StartDaemon.cc: Base class for logical device factories.
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
#include "APLCommon/APL_Defines.h"

#define DECLARE_SIGNAL_NAMES
#include "APLCommon/StartDaemon.h"

namespace LOFAR
{
namespace APLCommon
{

static const string StartDaemon::PSTYPE_STARTDAEMON("TAplStartDaemon");
static const string StartDaemon::SD_PROPNAME_COMMAND("command");
static const string StartDaemon::SD_PROPNAME_STATUS("status");

StartDaemon::StartDaemon(const string& name) :
  ::GCFTask((State)&StartDaemon::initial_state,"StartDaemon"),
  PropertySetAnswerHandlerInterface(),
  m_propertySetAnswer(*this),
  m_properties(name.c_str(),PSTYPE_STARTDAEMON.c_str(),PS_CAT_TEMPORARY,&m_propertySetAnswer),
  m_serverPortName(name + string("server")),
  m_serverPort(*this, m_serverPortName, ::GCFPortInterface::SPP, LOGICALDEVICE_PROTOCOL),
  m_childPorts(),
  m_factories(),
  m_logicalDevices()
{
  registerProtocol(STARTDAEMON_PROTOCOL, STARTDAEMON_PROTOCOL_signalnames);
  LOG_DEBUG(formatString("StartDaemon(%s)::StartDaemon",getName().c_str()));
  
  m_properties.enable();
}


StartDaemon::~StartDaemon()
{
  LOG_DEBUG(formatString("StartDaemon(%s)::~StartDaemon",getName().c_str()));
  m_properties.disable();
}

void StartDaemon::registerFactory(TLogicalDeviceTypes ldType,boost::shared_ptr<LogicalDeviceFactory> factory)
{
  m_factories[ldType] = factory;
}

TSDResult StartDaemon::createLogicalDevice(const TLogicalDeviceTypes ldType, const string& fileName)
{
  TSDResult result = SD_NO_ERROR;
  TFactoryMap::iterator it = m_factories.find(ldType);
  if(m_factories.end() != it)
  {
    try
    {
      boost::shared_ptr<LogicalDevice> ld = (*it)->createLogicalDevice(fileName);
      m_logicalDevices.push_back(ld);
    }
    catch(ParameterFileNotFoundException& e)
    {
      LOG_FATAL(e.message());
      result = SD_FILENOTFOUND;
    }
  }
  else
  {
    std::ostream os;
    LOG_FATAL_STR(os<<"Requested Logical Device ("<<ldType<<") cannot be created by this StartDaemon");
    result = SD_UNSUPPORTED_LD;
  }
  return result;
}

bool StartDaemon::_isServerPort(::GCFPortInterface& port)
{
  return (&port == &m_serverPort); // comparing two pointers. yuck?
}
   
bool StartDaemon::_isChildPort(::GCFPortInterface& port)
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
   
void StartDaemon::_disconnectedHandler(::GCFPortInterface& port)
{
  port.close();
  if(_isServerPort(port))
  {
  }
  else if(_isChildPort(port))
  {
    bool found=false;
    TPVSSPortVector::iterator it=m_childPorts.begin();
    while(!found && it != m_childPorts.end())
    {
      found = (&port == (*it).get()); // comparing two pointers. yuck?
      if(found)
      {
        m_clientPorts.erase(it);
      }
      else
      { 
        ++it;
      }
    }
  }
}

::GCFEvent::TResult StartDaemon::initial_state(::GCFEvent& event, ::GCFPortInterface& port)
{
  LOG_DEBUG(formatString("StartDaemon(%s)::initial_state (%s)",getName().c_str(),evtstr(event)));
  ::GCFEvent::TResult status = ::GCFEvent::HANDLED;
  
  switch (event.signal)
  {
    case F_INIT:
      break;

    case F_ENTRY:
    {
      TRAN(StartDaemon::idle_state);
      break;
    }
  
    case F_CONNECTED:
      break;

    case F_DISCONNECTED:
      port.close();
      break;
    
    default:
      LOG_DEBUG(formatString("StartDaemon(%s)::initial_state, default",getName().c_str()));
      status = ::GCFEvent::NOT_HANDLED;
      break;
  }    

  return status;
}

::GCFEvent::TResult StartDaemon::idle_state(::GCFEvent& event, ::GCFPortInterface& port)
{
  LOG_DEBUG(formatString("StartDaemon(%s)::idle_state (%s)",getName().c_str(),evtstr(event)));
  ::GCFEvent::TResult status = ::GCFEvent::HANDLED;

  switch (event.signal)
  {
    case F_INIT:
      break;
      
    case F_ENTRY:
    {
      // open the server port to allow childs to connect
      m_serverPort.open();
      break;
    }

    case F_ACCEPT_REQ:
    {
      boost::shared_ptr<GCFPVSSPort> client(new GCFPVSSPort);
      client->init(*this, m_serverPortName, GCFPortInterface::SPP, STARTDAEMON_PROTOCOL);
      m_serverPort.accept(client.get());
      m_childPorts.push_back(client);
    }
    break;
  
    case F_DISCONNECTED:
      _disconnectedHandler(port);
      break;
    
    case STARTDAEMON_SCHEDULE:
    {
      STARTDAEMONScheduleEvent scheduleEvent(event);
      STARTDAEMONScheduledEvent scheduledEvent;
      
      scheduledEvent.result = createLogicalDevice(scheduleEvent.logicalDeviceType,scheduleEvent.fileName);
      
      port.send(scheduledEvent);
      break;
    }
    default:
      LOG_DEBUG(formatString("StartDaemon(%s)::idle_state, default",getName().c_str()));
      status = ::GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}


};
};
