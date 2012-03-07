//
//  tITCPort.cc: Test program to test all kind of usage of the ITC ports.
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
//  $Id$

// The program has two tasks, a server task and a client task. They connect to each other
// by a TCP port and an ITC port. Once both connections are made the server starts sending
// messages over one of the ports, the client always send the answer back over the other port.


#include <lofar_config.h>
#include <Common/LofarLogger.h>

#include "tITCPort.h"
#include "Echo_Protocol.ph"

namespace LOFAR {
 namespace GCF {
  namespace TM {

EchoPingEvent 	gPing;
EchoPingEvent 	gPing2;
EchoPingEvent 	gPing3;
tClient*		gClientTask;
GCFITCPort*		gITCPort;

#define ASSERT_NOT_EQUAL(a,b) \
	ASSERTSTR ((a).seqnr==(b).seqnr && \
		   (a).ping_time.tv_sec==(b).ping_time.tv_sec && \
		   (a).ping_time.tv_usec==(b).ping_time.tv_usec && \
		   (a).someName==(b).someName, "Server: Returned message is different:" << endl << \
			"send.ping = " << (b).seqnr << endl << \
			"recv.ping = " << (a).seqnr << endl << \
			"send.time = " << (b).ping_time.tv_sec << "," << (b).ping_time.tv_usec << endl << \
			"recv.time = " << (a).ping_time.tv_sec  << "," << (a).ping_time.tv_usec << endl << \
			"send.name = " << (b).someName << endl << \
			"recv.name = " << (a).someName);

// Constructors of both classes
tServer::tServer(string name) : 
	GCFTask         ((State)&tServer::openITC, name),
	itsClientTCP	(0),
	itsTimerPort	(0),
	itsSendCount	(0)
{ 
  registerProtocol(ECHO_PROTOCOL, ECHO_PROTOCOL_STRINGS);
}

tClient::tClient(string name) : 
	GCFTask         ((State)&tClient::initial, name),
	itsTCPPort      (0),
	itsITCPort      (0),
	itsTimerPort	(0)
{ 
  registerProtocol(ECHO_PROTOCOL, ECHO_PROTOCOL_STRINGS);
}

// Server::openITC
//
// open ITC
GCFEvent::TResult	tServer::openITC(GCFEvent&	event, GCFPortInterface&	port)
{
	LOG_DEBUG_STR ("Server@openITC: " << eventName(event.signal) << "@" << port.getName());

	switch (event.signal) {
	case F_ENTRY: {
		itsTimerPort = new GCFTimerPort (*this, "serverTimer");
		LOG_DEBUG("Server: creating ITC stream");
		gITCPort = new GCFITCPort(*this, *gClientTask, "ITC stream", GCFPortInterface::SAP, ECHO_PROTOCOL);
        ASSERTSTR(gITCPort, "Server: Failed to create an ITC port");
		LOG_DEBUG("Server: openening ITC stream");
		itsTimerPort->setTimer(5.0);	// max wait time for open
		gITCPort->open();	// result in F_CONN
		}
	break;

	case F_TIMER:
	case F_DISCONNECTED:
		ASSERTSTR(false, "Server: Failed to open ITC stream");
	break;

	case F_CONNECTED: {
		itsTimerPort->cancelAllTimers();
		LOG_DEBUG("Server: ITC port opened, switching to the 'wait for TCP conn' state");
		gClientTask->setITCPort(gITCPort);
		TRAN (tServer::wait4TCP);
		}
	break;

	default:
		LOG_DEBUG("Server@openITC: default");

	} // switch

	return (GCFEvent::HANDLED);
}

// Server::wait4TCP
//
// wait for TCP connection with the client
GCFEvent::TResult	tServer::wait4TCP(GCFEvent&	event, GCFPortInterface&	port)
{
	LOG_DEBUG_STR ("Server@wait4TCP: " << eventName(event.signal) << "@" << port.getName());

	switch (event.signal) {
	case F_ENTRY: {
		LOG_DEBUG("Server: waiting for TCP connection from client");
		itsClientTCP = new GCFTCPPort(*this, "TCPstream:v1.0", GCFPortInterface::SPP, ECHO_PROTOCOL);
        ASSERTSTR(itsClientTCP, "Server: Failed to create a TCP port");
		itsTimerPort->setTimer(5.0);	// max wait time for open
		itsClientTCP->open();	// result in F_CONN
		}
	break;

	case F_TIMER:
	case F_DISCONNECTED:
		ASSERTSTR(false, "Server: client did not connect");
	break;

	case F_CONNECTED: {
		itsTimerPort->cancelAllTimers();
		LOG_DEBUG("Server: TCP connection with client is OK, going into the first test");
		TRAN (tServer::test1);
		}
	break;

	default:
		LOG_DEBUG("Server@wait4TCP: default");

	} // switch

	return (GCFEvent::HANDLED);
}

//
// TEST 1
//
// The server sends a message to the client over TCP and waits for the
// message to come back over the ITC port.
// If those messages are equal goto test2
GCFEvent::TResult tServer::test1(GCFEvent& event, GCFPortInterface& port)
{
	GCFEvent::TResult status = GCFEvent::HANDLED;
	LOG_DEBUG_STR ("Server@test1: " << eventName(event.signal) << "@" << port.getName());

	switch (event.signal) {
	case F_ENTRY: {
		// create PingEvent
		timeval ping_time;
		gettimeofday(&ping_time, 0);

		gPing.seqnr     = 1;
		gPing.ping_time = ping_time;
		gPing.someName  = "ITCport test number 1";

		// send the event
		itsClientTCP->send(gPing);
		LOG_INFO_STR("### Server: PING sent over TCPport (seqnr=" << gPing.seqnr);
		itsTimerPort->setTimer(5.0);	// max wait time for open
		}
	break;

	case F_TIMER: 
		ASSERTSTR(false, "Server: client did not returned an answer over ITC");
	break;
			
    case ECHO_PING: {
		itsTimerPort->cancelAllTimers();
		EchoPingEvent ping(event);
		LOG_DEBUG_STR ("Server:PING received, seqnr=" << ping.seqnr << endl << ping);
		ASSERT_NOT_EQUAL(ping, gPing);
		LOG_DEBUG ("Server: returned message is OK, going to test2");
		TRAN(tServer::test2);
		}
	break;

	case F_DISCONNECTED:
		ASSERTSTR(false, "Server: Received an DISCO");
	break;

	default:
		LOG_DEBUG("Server@test: default");
		break;
	}

	return status;
}

//
// TEST 2
//
// The server sends a message to the client over ITC and waits for the
// message to come back over the TCP port.
// If those messages are equal finish program, tests are done.
GCFEvent::TResult tServer::test2(GCFEvent& event, GCFPortInterface& port)
{
	GCFEvent::TResult status = GCFEvent::HANDLED;
	LOG_DEBUG_STR ("Server@test2: " << eventName(event.signal) << "@" << port.getName());

	switch (event.signal) {
	case F_ENTRY: {
		// create PingEvent
		timeval ping_time;
		gettimeofday(&ping_time, 0);

		gPing.seqnr     = 5674739;
		gPing.ping_time = ping_time;
		gPing.someName  = "Another testmessage to test the 'clone'effect of the ITCport.";

		// send the event
		gITCPort->send(gPing);
		LOG_INFO_STR("### Server: PING sent over ITC port (seqnr=" << gPing.seqnr);
		itsTimerPort->setTimer(5.0);	// max wait time for open
		}
	break;

	case F_TIMER: 
		ASSERTSTR(false, "Server: client did not returned an answer over TCP");
	break;
			
    case ECHO_PING: {
		itsTimerPort->cancelAllTimers();
		EchoPingEvent ping(event);
		LOG_DEBUG_STR ("Server:PING received, seqnr=" << ping.seqnr);
		ASSERT_NOT_EQUAL (ping, gPing);
		LOG_DEBUG ("Server: returned message is OK, going to TEST 3.");
		TRAN(tServer::test3A);
		}
	break;

	case F_DISCONNECTED:
		ASSERTSTR(false, "Server: Received an DISCO");
	break;

	default:
		LOG_DEBUG("Server@test2: default");
		break;
	}

	return status;
}

//
// TEST 3A
// The server sends 2 messages over the ITC port and responses on the returned answers with NEXT_STATE
// When the task goes to the next state the answer must be inserted in the eventstack by the scheduler.
//
GCFEvent::TResult tServer::test3A(GCFEvent& event, GCFPortInterface& port)
{
	GCFEvent::TResult status = GCFEvent::HANDLED;
	LOG_DEBUG_STR ("Server@test3A: " << eventName(event.signal) << "@" << port.getName());

	switch (event.signal) {
	case F_ENTRY: {
		// create PingEvent
		timeval ping_time;
		gettimeofday(&ping_time, 0);

		gPing2.seqnr     = 3021119;
		gPing2.ping_time = ping_time;
		gPing2.someName  = "First ping message that will be postphoned to next state on a TCPport";

		// send the event
		gITCPort->send(gPing2);
		LOG_INFO_STR("### Server: PING2 sent over ITC (seqnr=" << gPing2.seqnr);
		itsSendCount = 1;
		itsTimerPort->setTimer(5.0);	// max wait time for open
		}
	break;

	case F_TIMER: 
		if (itsSendCount == 2) {
			TRAN(tServer::test3B);
			return (GCFEvent::HANDLED);
		}
		ASSERTSTR(false, "Server@test3A: client did not returned an answer over TCP");
	break;
			
    case ECHO_PING: {
		itsTimerPort->cancelAllTimers();
		EchoPingEvent ping(event);
		LOG_DEBUG_STR ("Server:PING received, seqnr=" << ping.seqnr);
		if (itsSendCount == 1) {
			ASSERT_NOT_EQUAL (ping, gPing2);
			LOG_DEBUG ("Server: returned message is OK, Sending ping3 event");
			gPing3.seqnr     = 20492186;
			gPing3.ping_time = gPing2.ping_time;
			gPing3.ping_time.tv_sec += 5467;
			gPing3.someName  = "Second ping message that will be postphoned to next state on a TCPport";

			// send the event
			gITCPort->send(gPing3);
			LOG_INFO_STR("Server: PING3 sent (seqnr=" << gPing3.seqnr);
			itsSendCount++;
			itsTimerPort->setTimer(5.0);	// max wait time for answer
			return (GCFEvent::NEXT_STATE);
		}
		// pingevent 3 returned.
		ASSERT_NOT_EQUAL (ping, gPing3);
		LOG_DEBUG ("Server: returned message is OK, asking for postphone and switching to new state");
		itsTimerPort->setTimer(0.1);	// do state switch on timer.
		return (GCFEvent::NEXT_STATE);

		}
	break;

	case F_DISCONNECTED:
		ASSERTSTR(false, "Server: Received an DISCO");
	break;

	default:
		LOG_DEBUG("Server@test3A: default");
		break;
	}

	return status;
}

//
// TEST 3B
//
// Expecting 2 messages of the previous test.
GCFEvent::TResult tServer::test3B(GCFEvent& event, GCFPortInterface& port)
{
	GCFEvent::TResult status = GCFEvent::HANDLED;
	LOG_DEBUG_STR ("Server@test3B: " << eventName(event.signal) << "@" << port.getName());

	switch (event.signal) {
	case F_ENTRY: {
		itsSendCount = 0;
		LOG_INFO("### Server: waiting for postphoned ping-events over TCP ...");
		itsTimerPort->setTimer(5.0);	// max wait time for open
		}
	break;

	case F_TIMER: 
		if (itsSendCount == 2) {
			LOG_DEBUG("Postphonig over TCP is OK, trying the same over ITC...");
			TRAN(tServer::test4A);
			return (GCFEvent::HANDLED);
		}
		ASSERTSTR(false, "Server@test3B: Scheduler did not inject requested messages");
	break;
			
    case ECHO_PING: {
		itsTimerPort->cancelAllTimers();
		EchoPingEvent ping(event);
		LOG_DEBUG_STR ("Server:PING received, seqnr=" << ping.seqnr);
		if (itsSendCount == 0) {
			ASSERT_NOT_EQUAL (ping, gPing2);
			LOG_DEBUG ("Server: returned postphoned message is OK");
			itsTimerPort->setTimer(5.0);	// max wait time for answer
			itsSendCount++;
			return (GCFEvent::HANDLED);
		}
		// pingevent 3 returned.
		ASSERT_NOT_EQUAL (ping, gPing3);
		LOG_DEBUG ("Server: returned message is OK, asking for postphone and switching to new state");
		itsTimerPort->setTimer(0.1);	// do state switch on timer.
		itsSendCount++;
		return (GCFEvent::HANDLED);
		}
	break;

	case F_DISCONNECTED:
		ASSERTSTR(false, "Server: Received an DISCO");
	break;

	default:
		LOG_DEBUG("Server@test3A: default");
		break;
	}

	return status;
}
//
// TEST 4A
//
// The server sends 2 messages over the TCP port and responses on the returned answers with NEXT_STATE
// When the task goes to the next state the answer must be inserted in the eventstack by the scheduler.
GCFEvent::TResult tServer::test4A(GCFEvent& event, GCFPortInterface& port)
{
	GCFEvent::TResult status = GCFEvent::HANDLED;
	LOG_DEBUG_STR ("Server@test4A: " << eventName(event.signal) << "@" << port.getName());

	switch (event.signal) {
	case F_ENTRY: {
		// create PingEvent
		timeval ping_time;
		gettimeofday(&ping_time, 0);

		gPing2.seqnr     = 9111203;
		gPing2.ping_time = ping_time;
		gPing2.someName  = "First ping message that will be postphoned to next state on a ITCport";

		// send the event
		itsClientTCP->send(gPing2);
		LOG_INFO_STR("### Server: PING2 sent over TCP (seqnr=" << gPing2.seqnr);
		itsSendCount = 1;
		itsTimerPort->setTimer(5.0);	// max wait time for open
		}
	break;

	case F_TIMER: 
		if (itsSendCount == 2) {
			TRAN(tServer::test4B);
			return (GCFEvent::HANDLED);
		}
		ASSERTSTR(false, "Server@test4A: client did not returned an answer over ITC");
	break;
			
    case ECHO_PING: {
		itsTimerPort->cancelAllTimers();
		EchoPingEvent ping(event);
		LOG_DEBUG_STR ("Server:PING received, seqnr=" << ping.seqnr);
		if (itsSendCount == 1) {
			ASSERT_NOT_EQUAL (ping, gPing2);
			LOG_DEBUG ("Server: returned message is OK, Sending ping3 event");
			gPing3.seqnr     = 68129402;
			gPing3.someName  = "Second ping message that will be postphoned to next state on a ITCport";

			// send the event
			itsClientTCP->send(gPing3);
			LOG_INFO_STR("Server: PING3 sent (seqnr=" << gPing3.seqnr);
			itsSendCount++;
			itsTimerPort->setTimer(5.0);	// max wait time for answer
			return (GCFEvent::NEXT_STATE);
		}
		// pingevent 3 returned.
		ASSERT_NOT_EQUAL (ping, gPing3);
		LOG_DEBUG ("Server: returned message is OK, asking for postphone and switching to new state");
		itsTimerPort->setTimer(0.1);	// do state switch on timer.
		return (GCFEvent::NEXT_STATE);

		}
	break;

	case F_DISCONNECTED:
		ASSERTSTR(false, "Server: Received an DISCO");
	break;

	default:
		LOG_DEBUG("Server@test3A: default");
		break;
	}

	return status;
}

//
// TEST 4B
//
// Expecting 2 messages of the previous test.
GCFEvent::TResult tServer::test4B(GCFEvent& event, GCFPortInterface& port)
{
	GCFEvent::TResult status = GCFEvent::HANDLED;
	LOG_DEBUG_STR ("Server@test4B: " << eventName(event.signal) << "@" << port.getName());

	switch (event.signal) {
	case F_ENTRY: {
		itsSendCount = 0;
		LOG_INFO("### Server: waiting for postphoned ping-events over ITC ...");
		itsTimerPort->setTimer(5.0);	// max wait time for open
		}
	break;

	case F_TIMER: 
		if (itsSendCount == 2) {
			LOG_DEBUG("Test 4B passed, going to test 5A");
			TRAN(tServer::test5A);
			return (GCFEvent::HANDLED);
		}
		ASSERTSTR(false, "Server@test4B: Scheduler did not inject requested messages");
	break;
			
    case ECHO_PING: {
		itsTimerPort->cancelAllTimers();
		EchoPingEvent ping(event);
		LOG_DEBUG_STR ("Server:PING received, seqnr=" << ping.seqnr);
		if (itsSendCount == 0) {
			ASSERT_NOT_EQUAL (ping, gPing2);
			LOG_DEBUG ("Server: returned postphoned message is OK");
			itsTimerPort->setTimer(5.0);	// max wait time for answer
			itsSendCount++;
			return (GCFEvent::HANDLED);
		}
		// pingevent 3 returned.
		ASSERT_NOT_EQUAL (ping, gPing3);
		LOG_DEBUG ("Server: returned message is OK, asking for postphone and switching to new state");
		itsTimerPort->setTimer(0.1);	// do state switch on timer.
		itsSendCount++;
		return (GCFEvent::HANDLED);
		}
	break;

	case F_DISCONNECTED:
		ASSERTSTR(false, "Server: Received an DISCO");
	break;

	default:
		LOG_DEBUG("Server@test4A: default");
		break;
	}

	return status;
}

//
// TEST 5A
// The server generates 2 timer events on the TCPport timer and forwards them to the next state.
//
GCFEvent::TResult tServer::test5A(GCFEvent& event, GCFPortInterface& port)
{
	GCFEvent::TResult status = GCFEvent::HANDLED;
	LOG_DEBUG_STR ("Server@test5A: " << eventName(event.signal) << "@" << port.getName());

	switch (event.signal) {
	case F_ENTRY: {
		// create PingEvent
		itsTimerPort->setTimer(0.1, 0.0, (void*)(&gPing2));
		itsTimerPort->setTimer(0.2, 0.0, (void*)(&gPing3));
		itsTimerPort->setTimer(1.0);
		LOG_INFO_STR("### Server: initiated 2 timer events with a ping message connected to it, waiting for timers");
		itsSendCount = 0;
		}
	break;

	case F_TIMER: 
		itsSendCount++;
		if (itsSendCount > 2) {
			LOG_DEBUG("Postphoned 2 timer events, going to next state and wait there for them to receive them");
			TRAN(tServer::test5B);
			return (GCFEvent::HANDLED);
		}
		LOG_DEBUG("Postphoning F_TIMER event...");
		return (GCFEvent::NEXT_STATE);
	break;
			
	case F_DISCONNECTED:
		ASSERTSTR(false, "Server: Received an DISCO");
	break;

	default:
		LOG_DEBUG("Server@test5A: default");
		break;
	}

	return status;
}

//
// TEST 5B
//
// Expecting 2 TIMER messages of the previous test.
GCFEvent::TResult tServer::test5B(GCFEvent& event, GCFPortInterface& port)
{
	GCFEvent::TResult status = GCFEvent::HANDLED;
	LOG_DEBUG_STR ("Server@test5B: " << eventName(event.signal) << "@" << port.getName());

	switch (event.signal) {
	case F_ENTRY: {
		itsSendCount = 0;
		itsTimerPort->setTimer(5.0);	// max wait time for forwarded events
		}
	break;

	case F_TIMER:  {
		GCFTimerEvent& timerEvent=static_cast<GCFTimerEvent&>(event);
		EchoPingEvent* pingPtr = (EchoPingEvent*)timerEvent.arg;
		ASSERTSTR(pingPtr, "Expected an ping event attached to the timer event, bailing out...");
		LOG_DEBUG_STR ("Server:PING received, seqnr=" << pingPtr->seqnr);
		if (itsSendCount == 0) {
			ASSERT_NOT_EQUAL (*pingPtr, gPing2);
			LOG_DEBUG ("Server: returned postphoned message is OK");
			itsSendCount++;
			return (GCFEvent::HANDLED);
		}
		// pingevent 3 returned.
		ASSERT_NOT_EQUAL (*pingPtr, gPing3);
		LOG_DEBUG ("Server: forwarding timer events over a timerPort OK, trying it over TCP...");
		itsTimerPort->cancelAllTimers();
		TRAN(tServer::forwardOverTCP);
		return (GCFEvent::HANDLED);
		}
	break;

	case F_DISCONNECTED:
		ASSERTSTR(false, "Server: Received an DISCO");
	break;

	default:
		LOG_DEBUG("Server@test5b: default");
		break;
	}

	return status;
}

//
// forwardOverTCP
// The server generates 2 timer events on the TCPport timer and forwards them to the next state.
//
GCFEvent::TResult tServer::forwardOverTCP(GCFEvent& event, GCFPortInterface& port)
{
	GCFEvent::TResult status = GCFEvent::HANDLED;
	LOG_DEBUG_STR ("Server@forwardOverTCP: " << eventName(event.signal) << "@" << port.getName());

	switch (event.signal) {
	case F_ENTRY: {
		// create PingEvent
		itsClientTCP->setTimer(0.1, 0.0, (void*)(&gPing2));
		itsClientTCP->setTimer(0.2, 0.0, (void*)(&gPing3));
		itsTimerPort->setTimer(1.0);
		LOG_INFO_STR("### Server: initiated 2 timer events with a ping message connected to it, waiting for timers");
		itsSendCount = 0;
		}
	break;

	case F_TIMER: 
		itsSendCount++;
		if (itsSendCount > 2) {
			LOG_DEBUG("Postphoned 2 timer events, going to next state and wait there for them to receive them");
			TRAN(tServer::delayedOverTCP);
			return (GCFEvent::HANDLED);
		}
		LOG_DEBUG("Postphoning F_TIMER event...");
		return (GCFEvent::NEXT_STATE);
	break;
			
	case F_DISCONNECTED:
		ASSERTSTR(false, "Server: Received an DISCO");
	break;

	default:
		LOG_DEBUG("Server@forwardOverTCP: default");
		break;
	}

	return status;
}

//
// delayedOverTCP
// Expecting 2 TIMER messages of the previous test.
//
GCFEvent::TResult tServer::delayedOverTCP(GCFEvent& event, GCFPortInterface& port)
{
	GCFEvent::TResult status = GCFEvent::HANDLED;
	LOG_DEBUG_STR ("Server@delayedOverTCP: " << eventName(event.signal) << "@" << port.getName());

	switch (event.signal) {
	case F_ENTRY: {
		itsSendCount = 0;
		itsTimerPort->setTimer(5.0);	// max wait time for forwarded events
		}
	break;

	case F_TIMER:  {
		GCFTimerEvent& timerEvent=static_cast<GCFTimerEvent&>(event);
		EchoPingEvent* pingPtr = (EchoPingEvent*)timerEvent.arg;
		ASSERTSTR(pingPtr, "Expected an ping event attached to the timer event, bailing out...");
		LOG_DEBUG_STR ("Server:PING received, seqnr=" << pingPtr->seqnr);
		if (itsSendCount == 0) {
			ASSERT_NOT_EQUAL (*pingPtr, gPing2);
			LOG_DEBUG ("Server: returned postphoned message is OK");
			itsSendCount++;
			return (GCFEvent::HANDLED);
		}
		// pingevent 3 returned.
		ASSERT_NOT_EQUAL (*pingPtr, gPing3);
		LOG_DEBUG ("Server: forwarding timer events over a TCP OK, trying it over ITC...");
		itsTimerPort->cancelAllTimers();
		TRAN(tServer::forwardOverITC);
		return (GCFEvent::HANDLED);
		}
	break;

	case F_DISCONNECTED:
		ASSERTSTR(false, "Server: Received an DISCO");
	break;

	default:
		LOG_DEBUG("Server@delaydOverTCP: default");
		break;
	}

	return status;
}

//
// forwardOverITC
// The server generates 2 timer events on the ITCport timer and forwards them to the next state.
//
GCFEvent::TResult tServer::forwardOverITC(GCFEvent& event, GCFPortInterface& port)
{
	GCFEvent::TResult status = GCFEvent::HANDLED;
	LOG_DEBUG_STR ("Server@forwardOverITC: " << eventName(event.signal) << "@" << port.getName());

	switch (event.signal) {
	case F_ENTRY: {
		// create PingEvent
		gITCPort->setTimer(0.1, 0.0, (void*)(&gPing2));
		gITCPort->setTimer(0.2, 0.0, (void*)(&gPing3));
		itsTimerPort->setTimer(1.0);
		LOG_INFO_STR("### Server: initiated 2 timer events with a ping message connected to it, waiting for timers");
		itsSendCount = 0;
		}
	break;

	case F_TIMER: 
		itsSendCount++;
		if (itsSendCount > 2) {
			LOG_DEBUG("Postphoned 2 timer events, going to next state and wait there for them to receive them");
			TRAN(tServer::delayedOverITC);
			return (GCFEvent::HANDLED);
		}
		LOG_DEBUG("Postphoning F_TIMER event...");
		return (GCFEvent::NEXT_STATE);
	break;
			
	case F_DISCONNECTED:
		ASSERTSTR(false, "Server: Received an DISCO");
	break;

	default:
		LOG_DEBUG("Server@forwardOverITC: default");
		break;
	}

	return status;
}

//
// delayedOverITC
// Expecting 2 TIMER messages of the previous test.
//
GCFEvent::TResult tServer::delayedOverITC(GCFEvent& event, GCFPortInterface& port)
{
	GCFEvent::TResult status = GCFEvent::HANDLED;
	LOG_DEBUG_STR ("Server@delayedOverITC: " << eventName(event.signal) << "@" << port.getName());

	switch (event.signal) {
	case F_ENTRY: {
		itsSendCount = 0;
		itsTimerPort->setTimer(5.0);	// max wait time for forwarded events
		}
	break;

	case F_TIMER:  {
		GCFTimerEvent& timerEvent=static_cast<GCFTimerEvent&>(event);
		EchoPingEvent* pingPtr = (EchoPingEvent*)timerEvent.arg;
		ASSERTSTR(pingPtr, "Expected an ping event attached to the timer event, bailing out...");
		LOG_DEBUG_STR ("Server:PING received, seqnr=" << pingPtr->seqnr);
		if (itsSendCount == 0) {
			ASSERT_NOT_EQUAL (*pingPtr, gPing2);
			LOG_DEBUG ("Server: returned postphoned message is OK");
			itsSendCount++;
			return (GCFEvent::HANDLED);
		}
		// pingevent 3 returned.
		ASSERT_NOT_EQUAL (*pingPtr, gPing3);
		LOG_DEBUG ("Server: forwarding timer events over a ITC OK, ALL TESTED PASSED SUCCESSFUL!!!");
		GCFScheduler::instance()->stop();
		return (GCFEvent::HANDLED);
		}
	break;

	case F_DISCONNECTED:
		ASSERTSTR(false, "Server: Received an DISCO");
	break;

	default:
		LOG_DEBUG("Server@delaydOverITC: default");
		break;
	}

	return status;
}


// -------------------- CLIENT TASK --------------------

// Client::initial
//
// wait till server gave me my ITCport.
//
GCFEvent::TResult	tClient::initial(GCFEvent&	event, GCFPortInterface&	port)
{
	LOG_DEBUG_STR ("Client@initial: " << eventName(event.signal) << "@" << port.getName());

	switch (event.signal) {
	case F_ENTRY:
		LOG_DEBUG("Client: waiting for ITC port to be set by server");
		itsTimerPort = new GCFTimerPort (*this, "clientTimer");
		// Note: the 'setITC' call will initiate an F_TIMER event.
	break;

	case F_TIMER:
		ASSERTSTR(gITCPort, "Client: Timer event but no ITC port yet. Bailing out");
		LOG_DEBUG("Client: Got my ITCport, going to connect over TCP");
		TRAN(tClient::openTCP);
	break;

	default:
		LOG_DEBUG("Client@initial: default");

	} // switch

	return (GCFEvent::HANDLED);
}

// Client::openTCP
//
// setup the TCP connection with the server
//
GCFEvent::TResult	tClient::openTCP(GCFEvent&	event, GCFPortInterface&	port)
{
	LOG_DEBUG_STR ("Client@openTCP: " << eventName(event.signal) << "@" << port.getName());

	switch (event.signal) {
	case F_ENTRY: {
		LOG_DEBUG("Client: Opening TCP connection with Server");
		itsTCPPort = new GCFTCPPort(*this, "TCPstream:v1.0", GCFPortInterface::SAP, ECHO_PROTOCOL);
		ASSERTSTR(itsTCPPort, "Client: cannot create an TCP port");
		itsTCPPort->open();
		itsTimerPort->setTimer(5.0);
		}
	break;

	case F_TIMER:
		ASSERTSTR(false, "Client: Timeout on connect over TCP");
	break;

	case F_CONNECTED: {
		LOG_DEBUG("Client: Connected over TCP, going to receiver mode");
		itsTimerPort->cancelAllTimers();
		TRAN(tClient::receiverMode);
		}
	break;

	case F_DISCONNECTED:
		ASSERTSTR(false, "Client: Received an DISCO");
	break;

	default:
		LOG_DEBUG("Client@openTCP: default");

	} // switch

	return (GCFEvent::HANDLED);
}

//
// client::receiverMode
// 
// Copy messages from ITC to TCP and vice versa
GCFEvent::TResult tClient::receiverMode(GCFEvent& event, GCFPortInterface& port)
{
	LOG_DEBUG_STR ("Client@receiverMode: " << eventName(event.signal) << "@" << port.getName());

	switch (event.signal) {
    case ECHO_PING: {
			EchoPingEvent ping(event);
			LOG_INFO_STR("Client: PING received, seqnr=" << ping.seqnr << ", time=" << ping.ping_time.tv_sec << "." << ping.ping_time.tv_usec);
			if (&port == itsTCPPort) {
				LOG_DEBUG("Client: Sending back message");
				LOG_DEBUG_STR("event=" << event);
				LOG_DEBUG_STR("ping=" << ping);
				EchoPingEvent*	clonedPing = ping.clone();
				LOG_DEBUG_STR("clonedPing = " << *clonedPing);
				gITCPort->sendBack(*(ping.clone()));
				LOG_DEBUG("Client: Done");
			}
			else {
				LOG_DEBUG("Client: Sending back message");
				LOG_DEBUG_STR("event=" << event);
				LOG_DEBUG_STR("ping=" << ping);
				itsTCPPort->send(*(ping.clone()));
				LOG_DEBUG("Client: Done");
			}
		}
		break;

	case F_DISCONNECTED:
		ASSERTSTR(false, "Client: Received an DISCO");
	break;

	default:
		break;
	}

	return (GCFEvent::HANDLED);
}

  } // namespace TM
 } // namespace GCF
} // namespace LOFAR

using namespace LOFAR::GCF::TM;

//
// MAIN()
//
int main(int argc, char* argv[])
{
	GCFScheduler::instance()->init(argc, argv, argv[0]);

	tServer	serverTask("SERVER");
	gClientTask = new tClient("CLIENT");

	serverTask.start(); // make initial transition
	gClientTask->start(); // make initial transition

	GCFScheduler::instance()->run();

	return (0);
}
