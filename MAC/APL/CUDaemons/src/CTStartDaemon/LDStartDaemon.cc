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
	itsListener			(0),
	itsListenRetryTimer	(0),
	itsClients			(),
	itsStarter 			(0)
{
	LOG_TRACE_FLOW(formatString("LDStartDaemon(%s)", getName().c_str()));

	itsListener = new GCFTCPPort(*this, MAC_SVCMASK_STARTDAEMON, 
								 GCFPortInterface::MSPP, STARTDAEMON_PROTOCOL);
	ASSERTSTR(itsListener, "Unable to allocate listener port");

	itsStarter = new LogicalDeviceStarter(globalParameterSet());
	ASSERTSTR(itsStarter, "Unable to allocate starter object");
  
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
}

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
  
	case F_ENTRY: {
		break;
	}

	case F_ACCEPT_REQ: {
		GCFTCPPort*		newClient = new GCFTCPPort;
		newClient->init(*this, "client", GCFPortInterface::SPP, 
										 STARTDAEMON_PROTOCOL);
		itsListener->accept(*newClient);
		itsClients.push_back(newClient);
		break;
	}

	case F_DISCONNECTED:
		ASSERTSTR (&port != itsListener, "Listener port was closed, bailing out!");

		handleClientDisconnect(port);
		break;

	case F_CLOSED:
		break;

	case F_TIMER:
		break;

	case STARTDAEMON_CREATE: {
		// [REO] On a schedule-event a LD is created???
		STARTDAEMONCreateEvent 	createEvent(event);
		STARTDAEMONCreatedEvent createdEvent;
		createdEvent.logicalDeviceType = createEvent.logicalDeviceType;
		createdEvent.taskName		   = createEvent.taskName;
		createdEvent.result = 
				itsStarter->createLogicalDevice (createEvent.logicalDeviceType,
												 createEvent.taskName, 
												 createEvent.parentHost,
												 createEvent.parentService);

		port.send(createdEvent);
		break;
	}

	default:
		LOG_DEBUG(formatString("LDStartDaemon(%s)::operational_state, default",getName().c_str()));
		status = GCFEvent::NOT_HANDLED;
		break;
	}

	return (status);
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

	// cleanup connection in our pool
	vector<GCFPortInterface*>::iterator	iter = itsClients.begin();
	vector<GCFPortInterface*>::iterator	theEnd = itsClients.end();

	while(iter != theEnd) {
		if (*iter == &port) {
			LOG_DEBUG_STR ("Erasing client port " << &port);
			itsClients.erase(iter);
			return;
		}
	}
}


  }; // namespace CUDaemons
}; // namespace LOFAR
