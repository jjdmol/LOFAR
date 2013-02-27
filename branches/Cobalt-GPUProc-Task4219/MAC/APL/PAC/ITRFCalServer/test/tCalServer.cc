//
//  tCalServer.cc: Test program to test all kind of usage of the GCF ports.
//
//  Copyright (C) 2009
//  ASTRON (Netherlands Foundation for Research in Astronomy)
//  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
//  $Id: tCalServer.cc 13130 2009-04-20 14:18:58Z overeem $

#include <lofar_config.h>
#include <Common/LofarLogger.h>
#include <MACIO/MACServiceInfo.h>
#include <GCF/TM/GCF_Scheduler.h>
#include <APL/ICAL_Protocol/ICAL_Protocol.ph>
#include "tCalServer.h"

namespace LOFAR {
 using namespace GCF::TM;
 namespace ICAL {


// Constructors of both classes
tCalServer::tCalServer(string name) : 
	GCFTask        ((State)&tCalServer::connect, name),
	itsTimerPort   (0),
	itsConn 	   (0)
{ 
	itsTimerPort = new GCFTimerPort(*this, "timer");
	ASSERTSTR(itsTimerPort, "cannot allocate a timer");

	registerProtocol (ICAL_PROTOCOL, ICAL_PROTOCOL_STRINGS);
}

tCalServer::~tCalServer()
{
	delete itsTimerPort;
}

//
// connect
//
// Try to setup a connection with the CalServer.
//
GCFEvent::TResult tCalServer::connect(GCFEvent& event, GCFPortInterface& /*port*/)
{
	GCFEvent::TResult status = GCFEvent::HANDLED;
	LOG_DEBUG_STR ("tCalServer::connect: " << eventName(event.signal));

	switch (event.signal) {
	case F_ENTRY:
		break;

	case F_INIT: {
		itsConn = new GCFTCPPort(*this, MAC_SVCMASK_CALSERVER, GCFPortInterface::SAP, ICAL_PROTOCOL);
		ASSERTSTR(itsConn, "Can't allocate a TCPport");
		itsConn->open(); // nrRetry, timeout, retryItv
		break;
	}

	case F_DISCONNECTED:
		LOG_DEBUG("Cannot connect to the CalServer, is it running?");
		GCFScheduler::instance()->stop();
		break;

	case F_CONNECTED:
		LOG_DEBUG("Connected to CalServer, going to first test");
		sleep(1);
		TRAN(tCalServer::calStart);
		break;

	default:
		status = GCFEvent::NOT_HANDLED;
		break;
	}

	return status;
}

//
// calStart
//
// Send a CalStart event to the Calserver
//
GCFEvent::TResult tCalServer::calStart(GCFEvent& event, GCFPortInterface& port)
{
	GCFEvent::TResult status = GCFEvent::HANDLED;
	LOG_DEBUG_STR ("tCalServer::calStart: " << eventName(event.signal) << "@" << port.getName());

	switch (event.signal) {
	case F_ENTRY: {
		LOG_DEBUG("\n\n\n\nSending Calstart event...");
		RCUmask_t	rcuMask;
		for (int rcu = 0; rcu < 16; rcu++)  {
			rcuMask.set(rcu);
		}
		ICALStartEvent	start;
		start.name			= "firstSubArray";
		start.antennaSet	= "LBA_INNER";
		start.rcuMask		= rcuMask;
		start.rcumode		= 3;
		itsConn->send(start);
		}
		break;

	case ICAL_STARTACK: {
		ICALStartackEvent	ack(event);
		LOG_DEBUG_STR("Result for SubArray " << ack.name << " is " << ack.status);
		ASSERTSTR(ack.status == ICAL_SUCCESS, "CalStart event failed");
		TRAN(tCalServer::showSubArrays);
		}
		break;

	case F_DISCONNECTED:
		ASSERTSTR(false, "Unexpected disconnect");
		break;

	default:
		status = GCFEvent::NOT_HANDLED;
		break;
	}

	return status;
}

//
// showSubArrays
//
// Send a CalStart event to the Calserver
//
GCFEvent::TResult tCalServer::showSubArrays(GCFEvent& event, GCFPortInterface& port)
{
	GCFEvent::TResult status = GCFEvent::HANDLED;
	LOG_DEBUG_STR ("tCalServer::showSubArrays: " << eventName(event.signal) << "@" << port.getName());

	switch (event.signal) {
	case F_ENTRY:
		itsTimerPort->setTimer(2.0);
		break;

	case F_TIMER: {
		LOG_DEBUG("\n\n\n\nSending getSubArrays event...");
		ICALGetsubarrayEvent	start;
		start.subarrayname	= "";
		itsConn->send(start);
		}
		break;

	case ICAL_GETSUBARRAYACK: {
		ICALGetsubarrayackEvent	ack(event);
		LOG_DEBUG_STR("Result for GetSubArrays is " << ack.status);
		if (ack.status != ICAL_SUCCESS) {
			break;
		}
		SubArrayMap::iterator	iter = ack.subarraymap.begin();
		SubArrayMap::iterator	end  = ack.subarraymap.end();
		while (iter != end) {
			LOG_DEBUG_STR(*(iter->second));
			++iter;
		}
		TRAN(tCalServer::takeSubscription);
		}
		break;

	case F_DISCONNECTED:
		ASSERTSTR(false, "Unexpected disconnect");
		break;

	default:
		status = GCFEvent::NOT_HANDLED;
		break;
	}

	return status;
}

//
// takeSubscription
//
// Take a subscription on the subarray we registered
//
GCFEvent::TResult tCalServer::takeSubscription(GCFEvent& event, GCFPortInterface& port)
{
	GCFEvent::TResult status = GCFEvent::HANDLED;
	LOG_DEBUG_STR ("tCalServer::takeSubscription: " << eventName(event.signal) << "@" << port.getName());

	switch (event.signal) {
	case F_ENTRY: 
		itsTimerPort->setTimer(2.0);
		break;

	case F_TIMER: {
		LOG_DEBUG("\n\n\n\nTaking a subscription...");
		ICALSubscribeEvent	subEvent;
		subEvent.name	= "firstSubArray";
//		subEvent.subbandset = ...;
		itsConn->send(subEvent);
		}
		break;

	case ICAL_SUBSCRIBEACK: {
		ICALSubscribeackEvent	ack(event);
		LOG_DEBUG_STR("Result for subscription on SubArray " << ack.name << " is " << ack.status);
		ASSERTSTR(ack.status == ICAL_SUCCESS, "subscribe event failed");
		TRAN(tCalServer::waitForUpdate);
		}
		break;

	case F_DISCONNECTED:
		ASSERTSTR(false, "Unexpected disconnect");
		break;

	default:
		status = GCFEvent::NOT_HANDLED;
		break;
	}

	return status;
}

//
// waitForUpdate
//
// Wait till we receive an update event. The time this takes depends on the subbandlimits
// in the iCalServer.conf file.
//
GCFEvent::TResult tCalServer::waitForUpdate(GCFEvent& event, GCFPortInterface& port)
{
	GCFEvent::TResult status = GCFEvent::HANDLED;
	LOG_DEBUG_STR ("tCalServer::waitForUpdate: " << eventName(event.signal) << "@" << port.getName());

	switch (event.signal) {
	case F_ENTRY: 
		LOG_DEBUG_STR("\n\n\n\n\nWaiting for an update event, this might take 512 seconds");
		itsTimerPort->setTimer(520.0);
		break;

	case F_EXIT:
		itsTimerPort->cancelAllTimers();
		break;

	case F_TIMER:
		ASSERTSTR(false, "We did NOT receive an update, stopping program!!!");
		break;

	case ICAL_UPDATE: {
		itsTimerPort->cancelAllTimers();
		ICALUpdateEvent		update(event);
		LOG_DEBUG_STR("Result for update on SubArray " << update.name << " is " << update.status);
		LOG_DEBUG_STR("Gains: " << update.gains.getGains());
		TRAN(tCalServer::stopSubscription);
		}
		break;

	case F_DISCONNECTED:
		ASSERTSTR(false, "Unexpected disconnect");
		break;

	default:
		status = GCFEvent::NOT_HANDLED;
		break;
	}

	return status;
}

//
// stopSubscription
//
// Stop the subscription on the subarray we registered
//
GCFEvent::TResult tCalServer::stopSubscription(GCFEvent& event, GCFPortInterface& port)
{
	GCFEvent::TResult status = GCFEvent::HANDLED;
	LOG_DEBUG_STR ("tCalServer::stopSubscription: " << eventName(event.signal) << "@" << port.getName());

	switch (event.signal) {
	case F_ENTRY:
		itsTimerPort->setTimer(3.0);
		break;

	case F_TIMER: {
		LOG_DEBUG("\n\n\n\nStopping the subscription...");
		ICALUnsubscribeEvent	subEvent;
		subEvent.name	= "firstSubArray";
//		subEvent.subbandset = ...;
		itsConn->send(subEvent);
		}
		break;

	case ICAL_UNSUBSCRIBEACK: {
		ICALUnsubscribeackEvent	ack(event);
		LOG_DEBUG_STR("Result for releasing subscription on SubArray " << ack.name << " is " << ack.status);
		ASSERTSTR(ack.status == ICAL_SUCCESS, "unsubscribe event failed");
		TRAN(tCalServer::calStop);
		}
		break;

	case F_DISCONNECTED:
		ASSERTSTR(false, "Unexpected disconnect");
		break;

	default:
		status = GCFEvent::NOT_HANDLED;
		break;
	}

	return status;
}

//
// calStop
//
// Send a CalStop event to the Calserver
//
GCFEvent::TResult tCalServer::calStop(GCFEvent& event, GCFPortInterface& port)
{
	GCFEvent::TResult status = GCFEvent::HANDLED;
	LOG_DEBUG_STR ("tCalServer::calStop: " << eventName(event.signal) << "@" << port.getName());

	switch (event.signal) {
	case F_ENTRY: 
		LOG_DEBUG("\n\n\n\nWaiting 5 seconds...");
		itsTimerPort->setTimer(5.0);
		break;

	case F_TIMER: {
		LOG_DEBUG("Sending Calstop event...");
		ICALStopEvent	stop;
		stop.name	= "firstSubArray";
		itsConn->send(stop);
		}
		break;

	case ICAL_STOPACK: {
		ICALStopackEvent	ack(event);
		LOG_DEBUG_STR("Result for stopping SubArray " << ack.name << " is " << ack.status);
		ASSERTSTR(ack.status == ICAL_SUCCESS, "CalStop event failed");
		GCFScheduler::instance()->stop();
		}
		break;

	case F_DISCONNECTED:
		ASSERTSTR(false, "Unexpected disconnect");
		break;

	default:
		status = GCFEvent::NOT_HANDLED;
		break;
	}

	return status;
}

 } // namespace ICAL
} // namespace LOFAR

using namespace LOFAR::GCF::TM;
using namespace LOFAR::ICAL;

//
// MAIN()
//
int main(int argc, char* argv[])
{
	GCFScheduler*	theScheduler(GCFScheduler::instance());
	theScheduler->init(argc, argv, "tCalServer");

	tCalServer	clientTask("clientTask");
	clientTask.start(); // make initial transition

	theScheduler->run();

	return (0);
}
