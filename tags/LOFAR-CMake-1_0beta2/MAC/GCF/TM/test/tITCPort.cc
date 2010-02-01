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

#include <lofar_config.h>
#include <Common/LofarLogger.h>

#include "tITCPort.h"
#include "Echo_Protocol.ph"

namespace LOFAR {
 namespace GCF {
  namespace TM {

EchoPingEvent 	gPing;
tClient*		gClientTask;
GCFITCPort*		gITCPort;

// Constructors of both classes
tServer::tServer(string name) : 
	GCFTask         ((State)&tServer::openITC, name),
	itsClientTCP	(0),
	itsTimerPort	(0)
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
		itsTimerPort = new GCFTimerPort (*this, "timerPort");
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
		LOG_INFO_STR("Server: PING sent (seqnr=" << gPing.seqnr);
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
		ASSERTSTR (ping.seqnr==gPing.seqnr && 
				   ping.ping_time.tv_sec==gPing.ping_time.tv_sec && 
				   ping.ping_time.tv_usec==gPing.ping_time.tv_usec &&
				   ping.someName==gPing.someName, "Server: Returned message is different:" << endl <<
					"send.ping = " << gPing.seqnr << endl <<
					"recv.ping = " << ping.seqnr << endl <<
					"send.time = " << gPing.ping_time.tv_sec << "," << gPing.ping_time.tv_usec << endl <<
					"recv.time = " << ping.ping_time.tv_sec  << "," << ping.ping_time.tv_usec << endl <<
					"send.name = " << gPing.someName << endl <<
					"recv.name = " << ping.someName);

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
		LOG_INFO_STR("Server: PING sent (seqnr=" << gPing.seqnr);
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
		ASSERTSTR (ping.seqnr==gPing.seqnr && 
				   ping.ping_time.tv_sec==gPing.ping_time.tv_sec && 
				   ping.ping_time.tv_usec==gPing.ping_time.tv_usec &&
				   ping.someName==gPing.someName, "Server: Returned message is different:" << endl <<
					"send.ping = " << gPing.seqnr << endl <<
					"recv.ping = " << ping.seqnr << endl <<
					"send.time = " << gPing.ping_time.tv_sec << "," << gPing.ping_time.tv_usec << endl <<
					"recv.time = " << ping.ping_time.tv_sec  << "," << ping.ping_time.tv_usec << endl <<
					"send.name = " << gPing.someName << endl <<
					"recv.name = " << ping.someName);

		LOG_DEBUG ("Server: returned message is OK, finished testing.");
		GCFScheduler::instance()->stop();
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
		itsTimerPort = new GCFTimerPort (*this, "timerPort");
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
