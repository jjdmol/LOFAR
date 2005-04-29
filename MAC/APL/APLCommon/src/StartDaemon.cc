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
#include <GCF/GCF_PVInteger.h>
#include "APLCommon/APL_Defines.h"
#include "APLCommon/APLUtilities.h"
#include "APLCommon/LogicalDevice.h"
#include "APLCommon/LogicalDeviceFactory.h"

#include "APLCommon/StartDaemon.h"

#define ADJUSTEVENTSTRINGPARAMFROMBSE(str) \
{ \
  LOG_DEBUG("Adjust " #str " string size for test tool"); \
  str = str.c_str(); \
}


using namespace LOFAR::GCF::Common;
using namespace LOFAR::GCF::TM;
using namespace LOFAR::GCF::PAL;

namespace LOFAR
{
namespace APLCommon
{

const string StartDaemon::PSTYPE_STARTDAEMON("TAplStartDaemon");
const string StartDaemon::SD_PROPNAME_COMMAND("command");
const string StartDaemon::SD_PROPNAME_STATUS("status");
const string StartDaemon::SD_COMMAND_SCHEDULE("SCHEDULE");
const string StartDaemon::SD_COMMAND_DESTROY_LOGICALDEVICE("DESTROY_LOGICALDEVICE");
const string StartDaemon::SD_COMMAND_STOP("STOP");

INIT_TRACER_CONTEXT(StartDaemon,LOFARLOGGER_PACKAGE);

StartDaemon::StartDaemon(const string& name) :
  GCFTask((State)&StartDaemon::initial_state,name),
  PropertySetAnswerHandlerInterface(),
  m_propertySetAnswer(*this),
  m_properties(name.c_str(),PSTYPE_STARTDAEMON.c_str(),PS_CAT_TEMPORARY,&m_propertySetAnswer),
  m_serverPortName(string("server")),
  m_serverPort(*this, m_serverPortName, GCFPortInterface::MSPP, STARTDAEMON_PROTOCOL),
  m_childPorts(),
  m_factories(),
  m_logicalDevices(),
  m_garbageCollection(),
  m_garbageCollectionTimerId(0)
{
#ifdef USE_TCPPORT_INSTEADOF_PVSSPORT
  LOG_WARN("Using GCFTCPPort in stead of GCFPVSSPort");
#endif

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

TSDResult StartDaemon::createLogicalDevice(const TLogicalDeviceTypes ldType, const string& taskName, const string& fileName)
{
  TSDResult result = SD_RESULT_NO_ERROR;
  TFactoryMap::iterator itFactories = m_factories.find(ldType);
  if(m_factories.end() != itFactories)
  {
    try
    {
      TLogicalDeviceMap::iterator itDevices = m_logicalDevices.find(taskName);
      if(itDevices != m_logicalDevices.end())
      {
        result = SD_RESULT_ALREADY_EXISTS;
      }
      else
      {
        boost::shared_ptr<LogicalDevice> ld = itFactories->second->createLogicalDevice(taskName,fileName,this);
        m_logicalDevices[taskName] = ld;
        ld->start(); // make initial transition
      }
    }
    catch(APLCommon::ParameterFileNotFoundException& e)
    {
      LOG_FATAL(e.message());
      result = SD_RESULT_FILENOTFOUND;
    }
    catch(APLCommon::ParameterNotFoundException& e)
    {
      LOG_FATAL(e.message());
      result = SD_RESULT_PARAMETERNOTFOUND;
    }
    catch(APLCommon::WrongVersionException& e)
    {
      LOG_FATAL(e.message());
      result = SD_RESULT_WRONG_VERSION;
    }
    catch(Exception& e)
    {
      LOG_FATAL(e.message());
      result = SD_RESULT_UNSPECIFIED_ERROR;
    }
  }
  else
  {
    LOG_FATAL_STR("Requested Logical Device ("<<ldType<<") cannot be created by this StartDaemon");
    result = SD_RESULT_UNSUPPORTED_LD;
  }
  return result;
}

TSDResult StartDaemon::destroyLogicalDevice(const string& name)
{
  TSDResult result = SD_RESULT_NO_ERROR;
  TLogicalDeviceMap::iterator it = m_logicalDevices.find(name);
  if(it == m_logicalDevices.end())
  {
    result = SD_RESULT_LD_NOT_FOUND;
  }
  else
  {
    if(it->second->getLogicalDeviceState() == LogicalDevice::LOGICALDEVICE_STATE_GOINGDOWN)
    {
      // add the LD to the collection of garbage.
      m_garbageCollection[it->first] = it->second;
      m_garbageCollectionTimerId = m_serverPort.setTimer(0L,1000L); // 1000 microseconds
      
      // erase the LD from the collection of LD's.
      m_logicalDevices.erase(it);
    }
    else
    {
      result = SD_RESULT_WRONG_STATE;
    }
  }
  return result;
}

bool StartDaemon::_isServerPort(GCFPortInterface& port)
{
  return (&port == &m_serverPort); // comparing two pointers. yuck?
}
   
bool StartDaemon::_isChildPort(GCFPortInterface& port)
{
  bool found=false;
  TPortVector::iterator it=m_childPorts.begin();
  while(!found && it != m_childPorts.end())
  {
    found = (&port == (*it).get()); // comparing two pointers. yuck?
    ++it;
  }
  return found;
}
   
void StartDaemon::_disconnectedHandler(GCFPortInterface& port)
{
  port.close();
  if(_isServerPort(port))
  {
  }
  else if(_isChildPort(port))
  {
    bool found=false;
    TPortVector::iterator it=m_childPorts.begin();
    while(!found && it != m_childPorts.end())
    {
      found = (&port == (*it).get()); // comparing two pointers. yuck?
      if(found)
      {
        m_childPorts.erase(it);
      }
      else
      { 
        ++it;
      }
    }
  }
}

GCFEvent::TResult StartDaemon::initial_state(GCFEvent& event, GCFPortInterface& port)
{
  LOG_DEBUG(formatString("StartDaemon(%s)::initial_state (%s)",getName().c_str(),evtstr(event)));
  GCFEvent::TResult status = GCFEvent::HANDLED;
  
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
      status = GCFEvent::NOT_HANDLED;
      break;
  }    

  return status;
}

GCFEvent::TResult StartDaemon::idle_state(GCFEvent& event, GCFPortInterface& port)
{
  LOG_DEBUG(formatString("StartDaemon(%s)::idle_state (%s)",getName().c_str(),evtstr(event)));
  GCFEvent::TResult status = GCFEvent::HANDLED;

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
      boost::shared_ptr<TThePortTypeInUse> server(new TThePortTypeInUse);
      server->init(*this, m_serverPortName, GCFPortInterface::SPP, STARTDAEMON_PROTOCOL);
      m_serverPort.accept(*(server.get()));
      m_childPorts.push_back(server);
      break;
    }
  
    case F_DISCONNECTED:
      _disconnectedHandler(port);
      break;
      
    case F_TIMER:
    {
      GCFTimerEvent& timerEvent=static_cast<GCFTimerEvent&>(event);
      if(timerEvent.id == m_garbageCollectionTimerId)
      {
        // garbage collection
        TLogicalDeviceMap::iterator it = m_garbageCollection.begin();
        while(it != m_garbageCollection.end())
        {
          m_garbageCollection.erase(it);
          it = m_garbageCollection.begin();
        }
        port.cancelTimer(timerEvent.id);
        m_garbageCollectionTimerId = 0;
      }
      break;
    }
    
    case STARTDAEMON_SCHEDULE:
    {
      STARTDAEMONScheduleEvent scheduleEvent(event);

ADJUSTEVENTSTRINGPARAMFROMBSE(scheduleEvent.taskName)
ADJUSTEVENTSTRINGPARAMFROMBSE(scheduleEvent.fileName)
      
      STARTDAEMONScheduledEvent scheduledEvent;
      
      scheduledEvent.result = createLogicalDevice(scheduleEvent.logicalDeviceType,scheduleEvent.taskName, scheduleEvent.fileName);
      
      scheduledEvent.VIrootID = scheduleEvent.taskName;
      
      m_properties.setValue(SD_PROPNAME_STATUS,GCFPVInteger(scheduledEvent.result));
      port.send(scheduledEvent);
      break;
    }
    
    case STARTDAEMON_DESTROY_LOGICALDEVICE:
    {
      STARTDAEMONDestroyLogicaldeviceEvent destroyEvent(event);
      STARTDAEMONDestroyLogicaldeviceEvent* pDestroyEvent = &destroyEvent;
      if(destroyEvent.name.length() == 0)
      {
        // the destroy event was sent from within this application, so it has
        // not been packed. A static cast will do just fine.
        pDestroyEvent = static_cast<STARTDAEMONDestroyLogicaldeviceEvent*>(&event);
      } 
      
      TSDResult result = SD_RESULT_UNSPECIFIED_ERROR;
      if(pDestroyEvent != 0)
      {
        result = destroyLogicalDevice(pDestroyEvent->name);
      }
      m_properties.setValue(SD_PROPNAME_STATUS,GCFPVInteger(result));
      break;
    }
    
    case STARTDAEMON_STOP:
    {
      stop();
      m_properties.setValue(SD_PROPNAME_STATUS,GCFPVInteger(SD_RESULT_SHUTDOWN));
      break;
    }
    
    default:
      LOG_DEBUG(formatString("StartDaemon(%s)::idle_state, default",getName().c_str()));
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}

void StartDaemon::handlePropertySetAnswer(GCFEvent& answer)
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
          (strstr(pPropAnswer->pPropName, SD_PROPNAME_COMMAND.c_str()) != 0))
      {
        // command received
        string commandString(((GCFPVString*)pPropAnswer->pValue)->getValue());
        vector<string> parameters;
        string command;
        APLUtilities::decodeCommand(commandString,command,parameters);
        
        // SCHEDULE <type>,<taskname>,<filename>
        if(command==string(SD_COMMAND_SCHEDULE))
        {
          if(parameters.size()==3)
          {
            TLogicalDeviceTypes logicalDeviceType = static_cast<TLogicalDeviceTypes>(atoi(parameters[0].c_str()));
            string taskName = parameters[1];
            string fileName = parameters[2];            
            
            TSDResult result = createLogicalDevice(logicalDeviceType,taskName,fileName);
            m_properties.setValue(SD_PROPNAME_STATUS,GCFPVInteger(result));
          }
          else
          {
            TSDResult result = SD_RESULT_INCORRECT_NUMBER_OF_PARAMETERS;
            m_properties.setValue(SD_PROPNAME_STATUS,GCFPVInteger(result));
          }
        }
        // DESTROY_LOGICALDEVICE <name>
        else if(command==string(SD_COMMAND_DESTROY_LOGICALDEVICE))
        {
          if(parameters.size()==1)
          {
            string name = parameters[0];
            
            TSDResult result = destroyLogicalDevice(name);
            m_properties.setValue(SD_PROPNAME_STATUS,GCFPVInteger(result));
          }
          else
          {
            TSDResult result = SD_RESULT_INCORRECT_NUMBER_OF_PARAMETERS;
            m_properties.setValue(SD_PROPNAME_STATUS,GCFPVInteger(result));
          }
        }
        // STOP
        else if(command==string(SD_COMMAND_STOP))
        {
          m_properties.setValue(SD_PROPNAME_STATUS,GCFPVInteger(SD_RESULT_SHUTDOWN));
          stop();
        }
        else
        {
          TSDResult result = SD_RESULT_UNKNOWN_COMMAND;
          m_properties.setValue(SD_PROPNAME_STATUS,GCFPVInteger(result));
        }
      }
      break;
    }  

    default:
      break;
  }  
}


};
};
