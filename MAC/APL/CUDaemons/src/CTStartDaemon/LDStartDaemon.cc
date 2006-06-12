//#  LDStartDaemon.cc: Program that can start others on command.
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
#include <lofar_config.h>
#include <Common/LofarLogger.h>

#include <GCF/GCF_ServiceInfo.h>
#include <APL/APLCommon/APL_Defines.h>
#include <APL/APLCommon/APLUtilities.h>
#include <APL/APLCommon/StartDaemon_Protocol.ph>
#include <APL/APLCommon/Controller_Protocol.ph>
#include "LogicalDeviceStarter.h"
#include "LDStartDaemon.h"

using namespace LOFAR::GCF::Common;
using namespace LOFAR::GCF::TM;
using namespace LOFAR::ACC::APS;

namespace LOFAR {
  namespace CUDaemons {

//
// LDStartDaemon(taskname)
//
LDStartDaemon::LDStartDaemon(const string& name) :
	GCFTask    			((State)&LDStartDaemon::initial_state, name),
	itsActionList		(),
	itsActiveCntlrs		(),
	itsListener			(0),
	itsListenRetryTimer	(0),
	itsClients			(),
	itsStarter 			(0),
	itsTimerPort		(0)
{
	LOG_TRACE_FLOW(formatString("LDStartDaemon(%s)", getName().c_str()));

	itsListener = new GCFTCPPort(*this, MAC_SVCMASK_STARTDAEMON, 
								 GCFPortInterface::MSPP, STARTDAEMON_PROTOCOL);
	ASSERTSTR(itsListener, "Unable to allocate listener port");

	itsStarter = new LogicalDeviceStarter(globalParameterSet());
	ASSERTSTR(itsStarter, "Unable to allocate starter object");
  
	itsTimerPort = new GCFTimerPort(*this, "TimerPort");
	ASSERTSTR(itsTimerPort, "Unable to allocate timer port");

	registerProtocol(STARTDAEMON_PROTOCOL, STARTDAEMON_PROTOCOL_signalnames);
}


//
// ~LDStartDaemon
//
LDStartDaemon::~LDStartDaemon()
{
	LOG_TRACE_FLOW(formatString("~LDStartDaemon(%s)", getName().c_str()));

	if (itsListener) {
		itsListener->close();
		delete itsListener;
	}

	if (itsStarter) {
		delete itsStarter;
	}

	if (itsTimerPort) {
		delete itsTimerPort;
	}

	itsActionList.clear();
	itsActiveCntlrs.clear();

}

// -------------------- Internal routines --------------------

//
// findAction(port)
//
LDStartDaemon::actionIter	LDStartDaemon::findAction(GCFPortInterface*		aPort)
{
	actionIter		iter = itsActionList.begin();
	actionIter		end  = itsActionList.end();
	while (iter != end && iter->parentPort != aPort) {
		iter++;
	}
	return (iter);
}


//
// findAction(timerID)
//
LDStartDaemon::actionIter	LDStartDaemon::findAction(uint32	aTimerID)
{
	actionIter		iter = itsActionList.begin();
	actionIter		end  = itsActionList.end();
	while (iter != end && iter->timerID != aTimerID) {
		iter++;
	}
	return (iter);
}


//
// findAction(cntlrName)
//
LDStartDaemon::actionIter	LDStartDaemon::findAction(const string&		cntlrName)
{
	actionIter		iter = itsActionList.begin();
	actionIter		end  = itsActionList.end();
	while (iter != end && iter->cntlrName != cntlrName) {
		iter++;
	}
	return (iter);
}


//
// findController(port)
//
LDStartDaemon::CTiter	LDStartDaemon::findController(GCFPortInterface*		aPort)
{
	CTiter		iter = itsActiveCntlrs.begin();
	CTiter		end  = itsActiveCntlrs.end();
	while (iter != end && iter->second != aPort) {
		iter++;
	}
	return (iter);
}


//
// sendCreatedMsg(action, result)
//
void LDStartDaemon::sendCreatedMsg(actionIter		action, int32	result)
{
	// send customer message that controller is on the air
	STARTDAEMONCreatedEvent createdEvent;
	createdEvent.cntlrType = action->cntlrType;
	createdEvent.cntlrName = action->cntlrName;
	createdEvent.result	   = result;
	action->parentPort->send(createdEvent);
}


//
// sendNewParentAndCreatedMsg()
//
void LDStartDaemon::sendNewParentAndCreatedMsg(actionIter		action)
{
	// connection with new controller is made, send 'newparent' message
	STARTDAEMONNewparentEvent	msg;
	msg.cntlrName	  = action->cntlrName;
	msg.parentHost	  = action->parentHost;
	msg.parentService = action->parentService;
	
	// map controllername to controllerport
	CTiter	controller = itsActiveCntlrs.find(msg.cntlrName);
	ASSERTSTR(isController(controller), msg.cntlrName << 
										" not found in controller list");
	controller->second->send(msg);
	LOG_DEBUG_STR("Sending NewParent(" << msg.cntlrName << "," <<
						msg.parentHost << "," << msg.parentService << ")");

	// send customer message that controller is on the air
	sendCreatedMsg(action, SD_RESULT_NO_ERROR);
	action->parentPort->close();
}


//
// handleClientDisconnect(port)
//
// A disconnect event was receiveed on a client port. Close the port
// and remove the port from our pool.
//
void LDStartDaemon::handleClientDisconnect(GCFPortInterface&	port)
{
	// end TCP connection
	port.close();

	// remove actions on this port
	actionIter		action = findAction(&port);
	while (isAction(action)) {
		itsTimerPort->cancelTimer(action->timerID);
		LOG_DEBUG_STR ("Disconnect:removing " << action->cntlrName << " from actionlist");
		itsActionList.erase(action);
		action = findAction(&port);
	}

	// cleanup active controller list
	CTiter	controller = findController(&port);
	while (isController(controller)) {
		LOG_DEBUG_STR ("Removing " << controller->first << " from controllerlist");
		itsActiveCntlrs.erase(controller);
		controller = findController(&port);
	}

	// cleanup connection in our pool if it is one of them
	vector<GCFPortInterface*>::iterator	iter = itsClients.begin();
	vector<GCFPortInterface*>::iterator	theEnd = itsClients.end();

	while(iter != theEnd) {
		if (*iter == &port) {
			LOG_DEBUG_STR ("Erasing client port " << &port);
			delete *iter;	// ??
			itsClients.erase(iter);
			return;
		}
		iter++;
	}
}

// -------------------- STATE MACHINES --------------------

//
// initial_state(event, port)
//
// The only target in this state is to get the listener port on the air.
//
GCFEvent::TResult LDStartDaemon::initial_state(GCFEvent& event, 
											   GCFPortInterface& /*port*/)
{
	LOG_DEBUG(formatString("LDStartDaemon(%s)::initial_state (%s)",getName().c_str(),evtstr(event)));
  
	GCFEvent::TResult status = GCFEvent::HANDLED;
	switch (event.signal) {
	case F_INIT:
		break;

	case F_ENTRY: {
		itsListener->open();		// may result in CONN or DISCONN event
		break;
	}
  
	case F_CONNECTED:			// Must be from itsListener port
		if (itsListener->isConnected()) {
			if (itsListenRetryTimer) {
				itsListener->cancelTimer(itsListenRetryTimer);
				itsListenRetryTimer = 0;
			}
			LOG_DEBUG ("Listener port opened, going to operational mode");
			TRAN(LDStartDaemon::operational_state);
		}
		break;

	case F_DISCONNECTED:		// Must be from itsListener port
		LOG_DEBUG("Could not open listener, retry in 1 second");
		itsListenRetryTimer = itsListener->setTimer(1.0);	// retry in 1 second
		break;

	case F_TIMER:					//Must be from listener port
		if (!itsListener->isConnected()) {	// still not connected?
			itsListener->open();			// try again.
		}
		break;
	
	default:
		LOG_DEBUG(formatString("LDStartDaemon(%s)::initial_state, default",getName().c_str()));
		status = GCFEvent::NOT_HANDLED;
		break;
	}    

	return (status);
}

//
// operational_state(event, port)
//
// This is the normal operational mode. Wait for clients (e.g. MACScheduler) to
// connect, wait for commands from the clients and handle those.
//
GCFEvent::TResult LDStartDaemon::operational_state (GCFEvent& event, 
													GCFPortInterface& port)
{
	LOG_DEBUG(formatString("LDStartDaemon(%s)::operational_state (%s)",
							getName().c_str(),evtstr(event)));

	GCFEvent::TResult status = GCFEvent::HANDLED;

	switch (event.signal) {
	case F_INIT:
		break;
  
	case F_ENTRY: 
		break;

	case F_ACCEPT_REQ: {
		GCFTCPPort*		newClient = new GCFTCPPort;
		newClient->init(*this, "client", GCFPortInterface::SPP, STARTDAEMON_PROTOCOL);
		itsListener->accept(*newClient);
		itsClients.push_back(newClient);	// remember port for future delete.
		break;
	}

	case F_CONNECTED:
		break;

	case F_DISCONNECTED:
		ASSERTSTR (&port != itsListener, "Listener port was closed, bailing out!");
		handleClientDisconnect(port);
		break;

	case F_CLOSED:
		break;

	case F_TIMER: {
			GCFTimerEvent& timerEvent=static_cast<GCFTimerEvent&>(event);
			actionIter		action = findAction(timerEvent.id);
			if (isAction(action)) {
				// controller did not start with required time. report failure.
				STARTDAEMONCreatedEvent		createdEvent;
				createdEvent.cntlrType = action->cntlrType;
				createdEvent.cntlrName = action->cntlrName;
				createdEvent.result    = SD_RESULT_START_FAILED;
				action->parentPort->send(createdEvent);
				LOG_WARN_STR("Timeout in startup of " << action->cntlrName);

				LOG_DEBUG_STR ("Removing " << action->cntlrName << " from actionlist");
				itsActionList.erase(action);
			}
		}
		break;

	case STARTDAEMON_CREATE: {
		STARTDAEMONCreateEvent 	createEvent(event);
		// prepare an answer message
		STARTDAEMONCreatedEvent createdEvent;
		createdEvent.cntlrType = createEvent.cntlrType;
		createdEvent.cntlrName = createEvent.cntlrName;

		// is controller already known?
		CTiter	controller = itsActiveCntlrs.find(createEvent.cntlrName);
		if (!isController(controller)) {		// no, controller is not active
			// Ask starter Object to start the controller.
			createdEvent.result = 
					itsStarter->startController (createEvent.cntlrType,
												 createEvent.cntlrName, 
												 createEvent.parentHost,
												 createEvent.parentService);
			// when creation failed, report it back.
			if (createdEvent.result != SD_RESULT_NO_ERROR) {
				LOG_WARN_STR("Startup of " << createEvent.cntlrName << " failed");
				port.send(createdEvent);
				break;
			}
		}

		// controller already registered?
		if (isController(controller) && controller->second != 0) {
			// send newparent message right away.
			STARTDAEMONNewparentEvent	npEvent;
			npEvent.cntlrName     = createEvent.cntlrName;
			npEvent.parentHost    = createEvent.parentHost;
			npEvent.parentService = createEvent.parentService;
			controller->second->send(npEvent);
			LOG_DEBUG_STR("Sending NewParent(" << npEvent.cntlrName << "," <<
						npEvent.parentHost << "," << npEvent.parentService << ")");
			// report result back to caller.
			createdEvent.result = SD_RESULT_NO_ERROR;
			port.send(createdEvent);
			break;
		}

		// wait for controller to register, add action to actionList
		action_t			action;
		action.cntlrName	 = createEvent.cntlrName;
		action.cntlrType	 = createEvent.cntlrType;
		action.parentHost	 = createEvent.parentHost;
		action.parentService = createEvent.parentService;
		action.parentPort	 = &port;
		action.timerID		 = itsTimerPort->setTimer(20.0);	// failure over 20 sec
		LOG_DEBUG_STR ("Adding " << action.cntlrName << " to actionList");
		itsActionList.push_back(action);
		break;
	}

	case STARTDAEMON_ANNOUNCEMENT: {
		STARTDAEMONAnnouncementEvent	inMsg(event);
		// known controller?
		CTiter	controller = itsActiveCntlrs.find(inMsg.cntlrName);
		// controller already registered?
		if (isController(controller) && controller->second != 0) {
			if (controller->second == &port) {	// on same port, no problem
				LOG_DEBUG_STR ("Double announcement received of " << inMsg.cntlrName);
				break;
			}
			else {								// on other port, report error.
				LOG_ERROR_STR ("Controller " << inMsg.cntlrName << 
						  " registered at two ports!!! Ignoring second registration!");
				break;
			}
		}

		// register the controller and its port
		itsActiveCntlrs[inMsg.cntlrName] = &port; 
		LOG_DEBUG_STR("Received announcement of " << inMsg.cntlrName << 
						", adding it to the controllerList");

		// are there a newParent actions waiting?
		actionIter	action = findAction(inMsg.cntlrName);
		while (isAction(action)) {
			// first stop failure-timer
			itsTimerPort->cancelTimer(action->timerID);

			sendNewParentAndCreatedMsg(action);
			LOG_DEBUG_STR ("Removing " << action->cntlrName << " from actionlist");
			itsActionList.erase(action);
			action = findAction(inMsg.cntlrName);
		}
		break;
	}

	default:
		LOG_DEBUG(formatString("LDStartDaemon(%s)::operational_state, default",getName().c_str()));
		status = GCFEvent::NOT_HANDLED;
		break;
	}

	return (status);
}


  }; // namespace CUDaemons
}; // namespace LOFAR
