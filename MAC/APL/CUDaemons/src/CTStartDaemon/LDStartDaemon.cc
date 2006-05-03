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
#include <GCF/PAL/GCF_PVSSInfo.h>
#include <APL/APLCommon/APL_Defines.h>
#include <APL/APLCommon/APLUtilities.h>
#include "APL/APLCommon/LogicalDevice.h"

#ifdef CREATING_TASKS
#include "APL/STSCtl/LogicalDeviceFactoryBase.h"
#else
#include "APL/STSCtl/LDstarter.h"
#endif
#include "APL/STSCtl/StartDaemon.h"
#include "StartDaemon_Protocol.ph"

using namespace LOFAR::GCF::Common;
using namespace LOFAR::GCF::TM;
using namespace LOFAR::GCF::PAL;

namespace LOFAR {
  namespace STSCtl {

const string StartDaemon::PSTYPE_STARTDAEMON("TAplStartDaemon");
const string StartDaemon::SD_PROPNAME_COMMAND("command");
const string StartDaemon::SD_PROPNAME_STATUS("status");
const string StartDaemon::SD_COMMAND_SCHEDULE("SCHEDULE");
const string StartDaemon::SD_COMMAND_DESTROY_LD("DESTROY_LOGICALDEVICE");
const string StartDaemon::SD_COMMAND_STOP("STOP");

INIT_TRACER_CONTEXT(StartDaemon,LOFARLOGGER_PACKAGE);

//
// StartDaemon(systemname(?))
//
// The pvss System Name is necessary to distinguish between startdaemon servers on different systems
StartDaemon::StartDaemon(const string& name) :
	GCFTask((State)&StartDaemon::initial_state,name),
	PropertySetAnswerHandlerInterface(),
	m_propertySetAnswer(*this),
	m_properties(name.c_str(),PSTYPE_STARTDAEMON.c_str(),PS_CAT_TEMPORARY,&m_propertySetAnswer),
	m_serverPortName(string("server")),
	m_serverPort(),
	m_childPorts(),
	m_factories(),
	m_logicalDevices(),
	m_garbageCollection(),
	m_garbageCollectionTimerId(0)
{
#ifndef USE_PVSSPORT
	LOG_WARN("Using GCFTCPPort in stead of GCFPVSSPort");
#endif

	// PVSS must have been initialized before the local system name can 
	// be retrieved
	string taskName = GCFPVSSInfo::getLocalSystemName()+string("_")+name;
	setName(taskName);

	// it is important to init the m_serverPort AFTER setting the name 
	// of the task.
	m_serverPort.init(*this, m_serverPortName, GCFPortInterface::MSPP, STARTDAEMON_PROTOCOL),
  
	registerProtocol(STARTDAEMON_PROTOCOL, STARTDAEMON_PROTOCOL_signalnames);
	LOG_DEBUG(formatString("StartDaemon(%s)::StartDaemon",getName().c_str()));
  
	m_properties.enable();
}


//
// ~StartDaemon
//
StartDaemon::~StartDaemon()
{
	LOG_DEBUG(formatString("StartDaemon(%s)::~StartDaemon",getName().c_str()));
	m_properties.disable();
}

#ifdef CREATING_TASKS
//
// registerFactory(ldType, shared_ptr<LDdactoryBase>)
//
void StartDaemon::registerFactory(TLogicalDeviceTypes ldType,
							  boost::shared_ptr<LogicalDeviceFactoryBase> factory)
{
	m_factories[ldType] = factory;
}
#else
//
// registerFactory(ldType, shared_ptr<LDstarter>)
//
void StartDaemon::registerFactory(TLogicalDeviceTypes ldType,
							  boost::shared_ptr<LDstarter> factory)
{
	m_factories[ldType] = factory;
}
#endif

//
// createLogicalDevice(ldType, taskName, fileName)
//
TSDResult StartDaemon::createLogicalDevice(const TLogicalDeviceTypes ldType, 
										   const string& taskName, 
										   const string& fileName)
{
	// Search ldType in factory
	TFactoryMap::iterator itFactories = m_factories.find(ldType);
	if(itFactories == m_factories.end()) {
		LOG_FATAL_STR("Requested Logical Device (" << ldType <<
								") cannot be created by this StartDaemon");
		return (SD_RESULT_UNSUPPORTED_LD);
	}

	// Does logical device already exist?
	TLogicalDeviceMap::iterator itDevices = m_logicalDevices.find(taskName);
	if(itDevices == m_logicalDevices.end()) {
		// LD does not exist, try to create it.
		try {
#ifdef CREATING_TASKS
			boost::shared_ptr<LogicalDevice> ld = 
				itFactories->second->createLogicalDevice(taskName,fileName,this);
			m_logicalDevices[taskName] = ld;
			ld->start(); // make initial transition
#else
			m_logicalDevices[taskName] = itFactories->second->createLogicalDevice(taskName,fileName);
			LOG_DEBUG_STR(toString(getpid()) << ":cid of Logical Device = " << m_logicalDevices[taskName]);
#endif
			return (SD_RESULT_NO_ERROR);
		}
		catch(APLCommon::ParameterFileNotFoundException& e) {
			LOG_FATAL(e.message());
			return (SD_RESULT_FILENOTFOUND);
		}
		catch(APLCommon::ParameterNotFoundException& e) {
			LOG_FATAL(e.message());
			return (SD_RESULT_PARAMETERNOTFOUND);
		}
		catch(APLCommon::WrongVersionException& e) {
			LOG_FATAL(e.message());
			return (SD_RESULT_WRONG_VERSION);
		}
		catch(Exception& e) {
			LOG_FATAL(e.message());
			return (SD_RESULT_UNSPECIFIED_ERROR);
		}
	}

#ifdef CREATINGS_TASKS
	// LD already exists.
	if (itFactories->second->sharingAllowed()) {
		LOG_INFO(formatString("LogicalDevice %s allows sharing. Updating parameters.",taskName.c_str()));
		itFactories->second->createLogicalDevice(taskName,fileName,this);
		// ignoring the returned pointer because the instance is already in the logicalDeviceMap
		return (SD_RESULT_NO_ERROR);
	}

	LOG_FATAL_STR("Requested Logical Device (" << taskName <<
									") already exists and sharing is not allowed");
    return (SD_RESULT_ALREADY_EXISTS);
#else
	// [TODO] implement identical test (???)
	return (SD_RESULT_NO_ERROR);
#endif
}

//
// destroyLogicalDevice(name)
//
TSDResult StartDaemon::destroyLogicalDevice(const string& name)
{
	// Try to find the device in our map
	TLogicalDeviceMap::iterator it = m_logicalDevices.find(name);
	if(it == m_logicalDevices.end()) {
		return (SD_RESULT_LD_NOT_FOUND);		// not found, return error.
	}

#ifdef CREATING_TASKS
	// check state of device to destroy
	if (it->second->getLogicalDeviceState() != 
									LogicalDevice::LOGICALDEVICE_STATE_GOINGDOWN) {
		return (SD_RESULT_WRONG_STATE);
	}
#else
	// [TODO] implement identical test(???)
#endif

	LOG_DEBUG(formatString("Adding '%s' to the collection of garbage",name.c_str()));
	// add the LD to the collection of garbage.
	m_garbageCollection[it->first] = it->second;
	m_garbageCollectionTimerId = m_serverPort.setTimer(0L,1000L); // 1000 us

#ifndef CREATING_TASKS
	// [TODO] DO THE REAL KILL
#endif
  
	// erase the LD from the collection of LD's.
	m_logicalDevices.erase(it);

	return (SD_RESULT_NO_ERROR);
}

//
// _isChildPort(GCFport)
//
bool StartDaemon::_isChildPort(GCFPortInterface& port)
{
	bool found=false;

	TPortVector::iterator it=m_childPorts.begin();
	while(!found && it != m_childPorts.end()) {
		found = (&port == (*it).get()); // comparing two pointers. yuck?
		++it;
	}

	return (found);
}
   
//
// _disconnectedHandler(GCFport)
//
// Handle a disconnected event on one of the ports
//
void StartDaemon::_disconnectedHandler(GCFPortInterface& port)
{
	port.close();
	if(_isServerPort(port)) {
		return;
	}

	if(!_isChildPort(port)) {
		return;
	}

	bool found=false;
	TPortVector::iterator it=m_childPorts.begin();
	while(!found && it != m_childPorts.end()) {
		found = (&port == (*it).get()); // comparing two pointers. yuck?
		if(found) {
			m_childPorts.erase(it);
		}
		else { 
			++it;
		}
	}
}

//
// initial_state(event, port)
//
GCFEvent::TResult StartDaemon::initial_state(GCFEvent& event, GCFPortInterface& port)
{
	LOG_DEBUG(formatString("StartDaemon(%s)::initial_state (%s)",getName().c_str(),evtstr(event)));
  
	GCFEvent::TResult status = GCFEvent::HANDLED;
	switch (event.signal) {
		case F_INIT:
		break;

		case F_ENTRY: {
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

	return (status);
}

//
// idle_state(event, port)
//
GCFEvent::TResult StartDaemon::idle_state(GCFEvent& event, GCFPortInterface& port)
{
	LOG_DEBUG(formatString("StartDaemon(%s)::idle_state (%s)",getName().c_str(),evtstr(event)));

	GCFEvent::TResult status = GCFEvent::HANDLED;
	switch (event.signal) {
		case F_INIT:
		break;
      
		case F_ENTRY: {
			// open the server port to allow childs to connect
			m_serverPort.open();
			break;
		}

		case F_ACCEPT_REQ: {
			boost::shared_ptr<TThePortTypeInUse> server(new TThePortTypeInUse);
			server->init(*this, m_serverPortName, GCFPortInterface::SPP, STARTDAEMON_PROTOCOL);
			m_serverPort.accept(*(server.get()));
			m_childPorts.push_back(server);
			break;
		}
  
		case F_DISCONNECTED:
			_disconnectedHandler(port);
		break;
      
		case F_TIMER: {
			GCFTimerEvent& timerEvent=static_cast<GCFTimerEvent&>(event);
			// [REO] What happens if timer is NOT the garbageCollection timer?
			if(timerEvent.id == m_garbageCollectionTimerId) {
				LOG_DEBUG("Cleaning up the LogicalDevice garbage...");
				// garbage collection
				TLogicalDeviceMap::iterator it = m_garbageCollection.begin();
				while(it != m_garbageCollection.end()) {
					LOG_DEBUG(formatString("Destroying '%s'",it->first.c_str()));
					m_garbageCollection.erase(it);
					it = m_garbageCollection.begin();
				}
				port.cancelTimer(timerEvent.id);
				m_garbageCollectionTimerId = 0;
			}
			break;
		}
    
		case STARTDAEMON_SCHEDULE: {
			// [REO] On a schedule-event a LD is created???
			STARTDAEMONScheduleEvent scheduleEvent(event);
			STARTDAEMONScheduledEvent scheduledEvent;

			scheduledEvent.result = createLogicalDevice(
													scheduleEvent.logicalDeviceType,
													scheduleEvent.taskName, 
													scheduleEvent.fileName);

			scheduledEvent.VIrootID = scheduleEvent.taskName;

			m_properties.setValue(SD_PROPNAME_STATUS,
								  GCFPVInteger(scheduledEvent.result));
			port.send(scheduledEvent);
			break;
		}
    
		case STARTDAEMON_DESTROY_LOGICALDEVICE: {
			STARTDAEMONDestroyLogicaldeviceEvent destroyEvent(event);
			STARTDAEMONDestroyLogicaldeviceEvent* pDestroyEvent = &destroyEvent;
			if(destroyEvent.name.length() == 0) {
				// the destroy event was sent from within this application, so it has
				// not been packed. A static cast will do just fine.
				pDestroyEvent = static_cast<STARTDAEMONDestroyLogicaldeviceEvent*>(&event);
			} 
      
			TSDResult result = SD_RESULT_UNSPECIFIED_ERROR;
			if(pDestroyEvent != 0) {
				result = destroyLogicalDevice(pDestroyEvent->name);
			}
			m_properties.setValue(SD_PROPNAME_STATUS,GCFPVInteger(result));
			break;
		}
    
		case STARTDAEMON_STOP: {
			stop();
			m_properties.setValue(SD_PROPNAME_STATUS,
								  GCFPVInteger(SD_RESULT_SHUTDOWN));
			break;
		}
    
		default:
			LOG_DEBUG(formatString("StartDaemon(%s)::idle_state, default",getName().c_str()));
			status = GCFEvent::NOT_HANDLED;
			break;
	}

	return (status);
}

//
// handlePropertySetAnswer(event)
//
void StartDaemon::handlePropertySetAnswer(GCFEvent& answer)
{
	switch(answer.signal) {
		case F_MYPS_ENABLED: {
			GCFPropSetAnswerEvent* pPropAnswer =
									static_cast<GCFPropSetAnswerEvent*>(&answer);
			if(pPropAnswer->result == GCF_NO_ERROR) {
				// property set loaded, now load apc?
			}
			else {
				LOG_ERROR(formatString("%s : PropertySet %s NOT ENABLED",
										getName().c_str(),pPropAnswer->pScope));
			}
			break;
		}

		case F_PS_CONFIGURED: {
			GCFConfAnswerEvent* pConfAnswer =
									static_cast<GCFConfAnswerEvent*>(&answer);
			if(pConfAnswer->result == GCF_NO_ERROR) {
				LOG_DEBUG(formatString("%s : apc %s Loaded",
										getName().c_str(),pConfAnswer->pApcName));
				//apcLoaded();
			}
			else {
				LOG_ERROR(formatString("%s : apc %s NOT LOADED",
										getName().c_str(),pConfAnswer->pApcName));
			}
			break;
		}

		case F_VCHANGEMSG: {
			// check which property changed
			GCFPropValueEvent* pPropAnswer=static_cast<GCFPropValueEvent*>(&answer);
			if ((pPropAnswer->pValue->getType() == LPT_STRING) &&
				(strstr(pPropAnswer->pPropName, SD_PROPNAME_COMMAND.c_str()) != 0)) {
				// command received
				string commandString(((GCFPVString*)pPropAnswer->pValue)->getValue());
				vector<string> parameters;
				string command;
				APLUtilities::decodeCommand(commandString,command,parameters);

				// SCHEDULE <type>,<taskname>,<filename>
				if(command==string(SD_COMMAND_SCHEDULE)) {
					if(parameters.size()==3) {
						TLogicalDeviceTypes logicalDeviceType = 
							APLUtilities::convertLogicalDeviceType(parameters[0]);
						string taskName = parameters[1];
						string fileName = parameters[2];            

						TSDResult result = createLogicalDevice (logicalDeviceType,
																taskName,
																fileName);
						m_properties.setValue(SD_PROPNAME_STATUS,
											  GCFPVInteger(result));
					}
					else {
						TSDResult result = SD_RESULT_INCORRECT_NUMBER_OF_PARAMETERS;
						m_properties.setValue(SD_PROPNAME_STATUS,
											  GCFPVInteger(result));
					}
					return;
				}

				// DESTROY_LOGICALDEVICE <name>
				if(command==string(SD_COMMAND_DESTROY_LD)) {
					if(parameters.size()==1) {
						string name = parameters[0];

						TSDResult result = destroyLogicalDevice(name);
						m_properties.setValue(SD_PROPNAME_STATUS,
											  GCFPVInteger(result));
					}
					else {
						TSDResult result = SD_RESULT_INCORRECT_NUMBER_OF_PARAMETERS;
						m_properties.setValue(SD_PROPNAME_STATUS,
											  GCFPVInteger(result));
					}
					return;
				}

				// STOP
				if(command==string(SD_COMMAND_STOP)) {
					m_properties.setValue(SD_PROPNAME_STATUS,
										  GCFPVInteger(SD_RESULT_SHUTDOWN));
					stop();
					return;
				}

				TSDResult result = SD_RESULT_UNKNOWN_COMMAND;
				m_properties.setValue(SD_PROPNAME_STATUS,GCFPVInteger(result));
			} // command received
			break;
		}  

		default:
			break;
	}  
}

// -------------------- INLINE FUNCTIONS --------------------

//
// _isServerPort(GCFport)
//
bool StartDaemon::_isServerPort(GCFPortInterface& port)
{
	return (&port == &m_serverPort); // comparing two pointers. yuck?
}
   

  }; // namespace STSCtl
}; // namespace LOFAR
