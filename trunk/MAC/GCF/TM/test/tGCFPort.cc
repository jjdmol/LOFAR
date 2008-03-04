//
//  tGCFPort.cc: Test program to test all kind of usage of the GCF ports.
//
//  Copyright (C) 2006
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

#include "tGCFPort.h"
#include "Echo_Protocol.ph"

namespace LOFAR {
 namespace GCF {
  namespace TM {


// Constructors of both classes
tServer::tServer(string name) : 
	GCFTask         ((State)&tServer::test1, name),
	itsServerPort   (0),
	itsTimeoutTimer (0),
	itsFinished		(false)
{ 
  registerProtocol(ECHO_PROTOCOL, ECHO_PROTOCOL_STRINGS);
}

tClient::tClient(string name) : 
	GCFTask         ((State)&tClient::test1, name),
	itsClientPort   (0),
	itsTimeoutTimer (0),
	itsFinishedTimer(0)
{ 
  registerProtocol(ECHO_PROTOCOL, ECHO_PROTOCOL_STRINGS);
}

void tServer::finishTest(uint32	testnr) 
{
	LOG_INFO_STR ("Server:Ending test" << testnr);
	itsServerPort->cancelAllTimers();
	delete itsServerPort;
	itsServerPort    = 0;
	itsTimeoutTimer  = 0;
	itsFinished		 = false;
}

void tClient::finishTest(uint32	testnr) 
{
	LOG_INFO_STR ("Client:Ending test" << testnr);
	itsClientPort->cancelAllTimers();
	delete itsClientPort;
	itsClientPort    = 0;
	itsTimeoutTimer  = 0;
	itsFinishedTimer = 0;
}

//
// TEST 1
//
// The server opens a TCP port with name SERVER:test1 using the settings in
// the configuration file.
// The client tries to resolve the label CLIENT:1stTest into the right
// service name of the server using the config files and serviceBroker.
//
GCFEvent::TResult tServer::test1(GCFEvent& event, GCFPortInterface& /*port*/)
{
	GCFEvent::TResult status = GCFEvent::HANDLED;
	LOG_DEBUG_STR ("Server@test1: " << eventName(event.signal));

	switch (event.signal) {
	case F_INIT:
		break;

	case F_ENTRY:
		itsServerPort = new GCFPort(*this, "test1", GCFPortInterface::SPP,
									 ECHO_PROTOCOL);
		ASSERTSTR(itsServerPort, "Failed to open serverport in test1");
		LOG_INFO("Server:Starting test1");
		itsTimeoutTimer = itsServerPort->setTimer(30.0);	// max duration of test
		itsServerPort->open();
		break;

	case F_TIMER: {
			// Max testtime reached? Force transition to next test.
			GCFTimerEvent& timerEvent = static_cast<GCFTimerEvent&>(event);
			if (timerEvent.id == itsTimeoutTimer) {
				LOG_WARN("SERVER: TEST 1 FAILED ON TIMEOUT");
				gError |= 0x01;
				itsServerPort->close();	// results in disconnected event.
				itsFinished = true;
				break;
			}
			// 'open' timer expired, retry to open the server port.
			if (!itsServerPort->isConnected()) {
				itsServerPort->open();
			}
		}
		break;

	case F_CONNECTED:
		if (itsServerPort->isConnected()) {
			// Alright, the server port is open wait for client or
			// max testtime timer.
		}
		break;

	case F_CLOSED:
	case F_DISCONNECTED:
		if (itsFinished) {	// timer exists when msg was received.
			finishTest(1);
			TRAN(tServer::test2);
		}
		else {
			itsServerPort->setTimer(1.0); // try again after 1 second
		}
		break;

    case ECHO_PING: {
		EchoPingEvent ping(event);
		LOG_INFO_STR ("Server:PING received, seqnr=" << ping.seqnr);

		// Construct echo message and send it.
		timeval echo_time;
		gettimeofday(&echo_time, 0);
		EchoEchoEvent echo;
		echo.seqnr = ping.seqnr;
		echo.ping_time = ping.ping_time;
		echo.echo_time = echo_time;

		itsServerPort->send(echo);
		itsFinished = true;

		break;
    }
	default:
		status = GCFEvent::NOT_HANDLED;
		break;
	}

	return status;
}

GCFEvent::TResult tClient::test1(GCFEvent& event, GCFPortInterface& port)
{
	GCFEvent::TResult status = GCFEvent::HANDLED;
	LOG_DEBUG_STR ("Client@test1: " << eventName(event.signal));

	switch (event.signal) {
	case F_INIT:
		break;

	case F_ENTRY:
		itsClientPort = new GCFPort(*this, "1stTest", GCFPortInterface::SAP,
									 ECHO_PROTOCOL);
		ASSERTSTR(itsClientPort, "Failed to create a clientport in test1");
		LOG_INFO("Client:Starting test1");
		itsTimeoutTimer = itsClientPort->setTimer(30.0);	// max duration of test
		itsClientPort->open();
		break;

	case F_TIMER: {
			// Max testtime reached? Force transition to next test.
			GCFTimerEvent& timerEvent=static_cast<GCFTimerEvent&>(event);
			if ((timerEvent.id == itsTimeoutTimer) || 
				(timerEvent.id == itsFinishedTimer)) {

				if (timerEvent.id == itsTimeoutTimer) {
					LOG_WARN("CLIENT: TEST 1 FAILED ON TIMEOUT");
					gError |= 0x01;
				}
				itsClientPort->close();	// results in disconnected event.
				itsFinishedTimer = ~0;
				break;
			}
			// 'open' timer expired, retry to open the client port.
			if (!itsClientPort->isConnected()) {
				itsClientPort->open();
			}
		}
		break;

	case F_CONNECTED:
		if (itsClientPort->isConnected()) {
			// create PingEvent
			timeval ping_time;
			gettimeofday(&ping_time, 0);

			EchoPingEvent ping;
			ping.seqnr     = 1;
			ping.ping_time = ping_time;

			// send the event
			itsClientPort->send(ping);
			LOG_INFO_STR("Client:PING sent (seqnr=" << ping.seqnr);
		}
		break;

	case F_DISCONNECTED:
		port.close();
	case F_CLOSED:
		if (itsFinishedTimer) {
			LOG_INFO ("Client:Lost connection with server");
			finishTest(1);
			TRAN(tClient::test2);
		}
		else {
			itsClientPort->setTimer(1.0); // try again after 1 second
		}
		break;

    case ECHO_ECHO: {
			EchoEchoEvent echo(event);
			LOG_INFO_STR("client:ECHO received, seqnr=" << echo.seqnr);
			itsFinishedTimer = itsClientPort->setTimer(0.5);
		}
		break;

	default:
		status = GCFEvent::NOT_HANDLED;
		break;
	}

	return status;
}

//
// TEST2
//
// The server opens a TCP port with name SERVER:test2 using the settings in
// the configuration file.
// The client tries to open the port "SERVER:test2" without the configfiles.
//
GCFEvent::TResult tServer::test2(GCFEvent& event, GCFPortInterface& /*port*/)
{
	GCFEvent::TResult status = GCFEvent::HANDLED;
	LOG_DEBUG_STR ("Server@test2: " << eventName(event.signal));

	switch (event.signal) {
	case F_INIT:
		break;

	case F_ENTRY:
		itsServerPort = new GCFPort(*this, "test2", GCFPortInterface::SPP,
									 ECHO_PROTOCOL);
		ASSERTSTR(itsServerPort, "Failed to open serverport in test2");
		LOG_INFO("Server:Starting test2");
		itsTimeoutTimer = itsServerPort->setTimer(30.0);	// max duration of test
		itsServerPort->open();
		break;

	case F_TIMER: {
			// Max testtime reached? Force transition to next test.
			GCFTimerEvent& timerEvent = static_cast<GCFTimerEvent&>(event);
			if (timerEvent.id == itsTimeoutTimer) {
				LOG_WARN("SERVER: TEST 2 FAILED ON TIMEOUT");
				gError |= 0x02;
				itsServerPort->close();	// results in disconnected event
				itsFinished = true;
				break;
			}
			// 'open' timer expired, retry to open the server port.
			if (!itsServerPort->isConnected()) {
				itsServerPort->open();
			}
		}
		break;

	case F_CONNECTED:
		if (itsServerPort->isConnected()) {
			// Alright, the server port is open wait for client or
			// max testtime timer.
		}
		break;

	case F_CLOSED:
	case F_DISCONNECTED:
		if (itsFinished) {	// timer exists when msg was received.
			finishTest(2);
			TRAN(tServer::test3);
		}
		else {
			itsServerPort->setTimer(1.0); // try again after 1 second
		}
		break;

    case ECHO_PING: {
		EchoPingEvent ping(event);
		LOG_INFO_STR ("Server:PING received, seqnr=" << ping.seqnr);

		// Construct echo message and send it.
		timeval echo_time;
		gettimeofday(&echo_time, 0);
		EchoEchoEvent echo;
		echo.seqnr = ping.seqnr;
		echo.ping_time = ping.ping_time;
		echo.echo_time = echo_time;

		itsServerPort->send(echo);
		itsFinished = true;

		break;
    }
	default:
		status = GCFEvent::NOT_HANDLED;
		break;
	}

	return status;
}

GCFEvent::TResult tClient::test2(GCFEvent& event, GCFPortInterface& port)
{
	GCFEvent::TResult status = GCFEvent::HANDLED;
	LOG_DEBUG_STR ("Client@test2: " << eventName(event.signal));

	switch (event.signal) {
	case F_INIT:
		break;

	case F_ENTRY:
		itsClientPort = new GCFPort(*this, "SERVER:test2", 
									GCFPortInterface::SAP, ECHO_PROTOCOL);
		ASSERTSTR(itsClientPort, "Failed to create a clientport in test2");
		LOG_INFO("Client:Starting test2");
		itsTimeoutTimer = itsClientPort->setTimer(30.0);	// max duration of test
		itsClientPort->open();
		break;

	case F_TIMER: {
			// Max testtime reached? Force transition to next test.
			GCFTimerEvent& timerEvent=static_cast<GCFTimerEvent&>(event);
			if ((timerEvent.id == itsTimeoutTimer) || 
				(timerEvent.id == itsFinishedTimer)) {

				if (timerEvent.id == itsTimeoutTimer) {
					LOG_WARN("CLIENT: TEST 2 FAILED ON TIMEOUT");
					gError |= 0x02;
				}
				itsClientPort->close();	// results in disconected event
				itsFinishedTimer = ~0;
				break;
			}
			// 'open' timer expired, retry to open the client port.
			if (!itsClientPort->isConnected()) {
				itsClientPort->open();
			}
		}
		break;

	case F_CONNECTED:
		if (itsClientPort->isConnected()) {
			// create PingEvent
			timeval ping_time;
			gettimeofday(&ping_time, 0);

			EchoPingEvent ping;
			ping.seqnr     = 2;
			ping.ping_time = ping_time;

			// send the event
			itsClientPort->send(ping);
			LOG_INFO_STR("Client:PING sent (seqnr=" << ping.seqnr);
		}
		break;

	case F_DISCONNECTED:
		port.close();
	case F_CLOSED:
		if (itsFinishedTimer) {
			LOG_INFO ("Client:Lost connection with server");
			finishTest(2);
			TRAN(tClient::test3);
		}
		else {
			itsClientPort->setTimer(1.0); // try again after 1 second
		}
		break;

    case ECHO_ECHO: {
			EchoEchoEvent echo(event);
			LOG_INFO_STR("client:ECHO received, seqnr=" << echo.seqnr);
			itsFinishedTimer = itsClientPort->setTimer(0.5);
		}
		break;

	default:
		status = GCFEvent::NOT_HANDLED;
		break;
	}

	return status;
}


//
// TEST3
//
// The server opens a TCP port with name "SERVER:test3" without using the 
// configuration files.
// The client tries to resolve the label CLIENT:3rdTest into the right
// service name of the server using the config files and serviceBroker.
//
//
GCFEvent::TResult tServer::test3(GCFEvent& event, GCFPortInterface& /*port*/)
{
	GCFEvent::TResult status = GCFEvent::HANDLED;
	LOG_DEBUG_STR ("Server@test3: " << eventName(event.signal));

	switch (event.signal) {
	case F_INIT:
		break;

	case F_ENTRY:
		itsServerPort = new GCFPort(*this, "SERVER:test3", 
									GCFPortInterface::SPP, ECHO_PROTOCOL);
		ASSERTSTR(itsServerPort, "Failed to open serverport in test3"); LOG_INFO("Server:Starting test3");
		itsTimeoutTimer = itsServerPort->setTimer(30.0);	// max duration of test
		itsServerPort->open();
		break;

	case F_TIMER: {
			// Max testtime reached? Force transition to next test.
			GCFTimerEvent& timerEvent = static_cast<GCFTimerEvent&>(event);
			if (timerEvent.id == itsTimeoutTimer) {
				LOG_WARN("SERVER: TEST 3 FAILED ON TIMEOUT");
				gError |= 0x04;
				itsServerPort->close();	// results in disconnected event
				itsFinished = true;
				break;
			}
			// 'open' timer expired, retry to open the server port.
			if (!itsServerPort->isConnected()) {
				itsServerPort->open();
			}
		}
		break;

	case F_CONNECTED:
		if (itsServerPort->isConnected()) {
			// Alright, the server port is open wait for client or
			// max testtime timer.
		}
		break;

	case F_CLOSED:
	case F_DISCONNECTED:
		if (itsFinished) {	// timer exists when msg was received.
			finishTest(3);
			TRAN(tServer::test4);
		}
		else {
			itsServerPort->setTimer(1.0); // try again after 1 second
		}
		break;

    case ECHO_PING: {
		EchoPingEvent ping(event);
		LOG_INFO_STR ("Server:PING received, seqnr=" << ping.seqnr);

		// Construct echo message and send it.
		timeval echo_time;
		gettimeofday(&echo_time, 0);
		EchoEchoEvent echo;
		echo.seqnr = ping.seqnr;
		echo.ping_time = ping.ping_time;
		echo.echo_time = echo_time;

		itsServerPort->send(echo);
		itsFinished = true;

		break;
    }
	default:
		status = GCFEvent::NOT_HANDLED;
		break;
	}

	return status;
}

GCFEvent::TResult tClient::test3(GCFEvent& event, GCFPortInterface& port)
{
	GCFEvent::TResult status = GCFEvent::HANDLED;
	LOG_DEBUG_STR ("Client@test3: " << eventName(event.signal));

	switch (event.signal) {
	case F_INIT:
		break;

	case F_ENTRY:
		itsClientPort = new GCFPort(*this, "3rdTest", 
									GCFPortInterface::SAP, ECHO_PROTOCOL);
		ASSERTSTR(itsClientPort, "Failed to create a clientport in test3");
		LOG_INFO("Client:Starting test3");
		itsTimeoutTimer = itsClientPort->setTimer(30.0);	// max duration of test
		itsClientPort->open();
		break;

	case F_TIMER: {
			// Max testtime reached? Force transition to next test.
			GCFTimerEvent& timerEvent=static_cast<GCFTimerEvent&>(event);
			if ((timerEvent.id == itsTimeoutTimer) || 
				(timerEvent.id == itsFinishedTimer)) {

				if (timerEvent.id == itsTimeoutTimer) {
					LOG_WARN("CLIENT: TEST 3 FAILED ON TIMEOUT");
					gError |= 0x04;
				}
				itsClientPort->close();	// results in disconnected event
				itsFinishedTimer = ~0;
				break;
			}
			// 'open' timer expired, retry to open the client port.
			if (!itsClientPort->isConnected()) {
				itsClientPort->open();
			}
		}
		break;

	case F_CONNECTED:
		if (itsClientPort->isConnected()) {
			// create PingEvent
			timeval ping_time;
			gettimeofday(&ping_time, 0);

			EchoPingEvent ping;
			ping.seqnr     = 3;
			ping.ping_time = ping_time;

			// send the event
			itsClientPort->send(ping);
			LOG_INFO_STR("Client:PING sent (seqnr=" << ping.seqnr);
		}
		break;

	case F_DISCONNECTED:
		port.close();
	case F_CLOSED:
		if (itsFinishedTimer) {
			LOG_INFO ("Client:Lost connection with server");
			finishTest(3);
			TRAN(tClient::test4);
		}
		else {
			itsClientPort->setTimer(1.0); // try again after 1 second
		}
		break;

    case ECHO_ECHO: {
			EchoEchoEvent echo(event);
			LOG_INFO_STR("client:ECHO received, seqnr=" << echo.seqnr);
			itsFinishedTimer = itsClientPort->setTimer(0.5);
		}
		break;

	default:
		status = GCFEvent::NOT_HANDLED;
		break;
	}

	return status;
}

//
// TEST 4
//
// The server opens a TCP port with name "SERVER:test4" without using the 
// configuration files, the client also.
//
GCFEvent::TResult tServer::test4(GCFEvent& event, GCFPortInterface& /*port*/)
{
	GCFEvent::TResult status = GCFEvent::HANDLED;
	LOG_DEBUG_STR ("Server@test4: " << eventName(event.signal));

	switch (event.signal) {
	case F_INIT:
		break;

	case F_ENTRY:
		itsServerPort = new GCFPort(*this, "SERVER:test4", 
									GCFPortInterface::SPP, ECHO_PROTOCOL);
		ASSERTSTR(itsServerPort, "Failed to open serverport in test4"); LOG_INFO("Server:Starting test4");
		itsTimeoutTimer = itsServerPort->setTimer(30.0);	// max duration of test
		itsServerPort->open();
		break;

	case F_TIMER: {
			// Max testtime reached? Force transition to next test.
			GCFTimerEvent& timerEvent = static_cast<GCFTimerEvent&>(event);
			if (timerEvent.id == itsTimeoutTimer) {
				LOG_WARN("SERVER: TEST 4 FAILED ON TIMEOUT");
				gError |= 0x08;
				itsServerPort->close();	// results in disconnected event
				itsFinished = true;
				break;
			}
			// 'open' timer expired, retry to open the server port.
			if (!itsServerPort->isConnected()) {
				itsServerPort->open();
			}
		}
		break;

	case F_CONNECTED:
		if (itsServerPort->isConnected()) {
			// Alright, the server port is open wait for client or
			// max testtime timer.
		}
		break;

	case F_CLOSED:
	case F_DISCONNECTED:
		if (itsFinished) {	// timer exists when msg was received.
			finishTest(4);
			TRAN(tServer::test5);
		}
		else {
			itsServerPort->setTimer(1.0); // try again after 1 second
		}
		break;

    case ECHO_PING: {
		EchoPingEvent ping(event);
		LOG_INFO_STR ("Server:PING received, seqnr=" << ping.seqnr);

		// Construct echo message and send it.
		timeval echo_time;
		gettimeofday(&echo_time, 0);
		EchoEchoEvent echo;
		echo.seqnr = ping.seqnr;
		echo.ping_time = ping.ping_time;
		echo.echo_time = echo_time;

		itsServerPort->send(echo);
		itsFinished = true;

		break;
    }
	default:
		status = GCFEvent::NOT_HANDLED;
		break;
	}

	return status;
}

GCFEvent::TResult tClient::test4(GCFEvent& event, GCFPortInterface& port)
{
	GCFEvent::TResult status = GCFEvent::HANDLED;
	LOG_DEBUG_STR ("Client@test4: " << eventName(event.signal));

	switch (event.signal) {
	case F_INIT:
		break;

	case F_ENTRY:
		itsClientPort = new GCFPort(*this, "SERVER:test4", 
									GCFPortInterface::SAP, ECHO_PROTOCOL);
		ASSERTSTR(itsClientPort, "Failed to create a clientport in test4");
		LOG_INFO("Client:Starting test4");
		itsTimeoutTimer = itsClientPort->setTimer(30.0);	// max duration of test
		itsClientPort->open();
		break;

	case F_TIMER: {
			// Max testtime reached? Force transition to next test.
			GCFTimerEvent& timerEvent=static_cast<GCFTimerEvent&>(event);
			if ((timerEvent.id == itsTimeoutTimer) || 
				(timerEvent.id == itsFinishedTimer)) {

				if (timerEvent.id == itsTimeoutTimer) {
					LOG_WARN("CLIENT: TEST 4 FAILED ON TIMEOUT");
					gError |= 0x08;
				}
				itsClientPort->close();	// results in disconnected event
				itsFinishedTimer = ~0;
				break;
			}
			// 'open' timer expired, retry to open the client port.
			if (!itsClientPort->isConnected()) {
				itsClientPort->open();
			}
		}
		break;

	case F_CONNECTED:
		if (itsClientPort->isConnected()) {
			// create PingEvent
			timeval ping_time;
			gettimeofday(&ping_time, 0);

			EchoPingEvent ping;
			ping.seqnr     = 4;
			ping.ping_time = ping_time;

			// send the event
			itsClientPort->send(ping);
			LOG_INFO_STR("Client:PING sent (seqnr=" << ping.seqnr);
		}
		break;

	case F_DISCONNECTED:
		port.close();
	case F_CLOSED:
		if (itsFinishedTimer) {
			LOG_INFO ("Client:Lost connection with server");
			finishTest(4);
			TRAN(tClient::test5);
		}
		else {
			itsClientPort->setTimer(1.0); // try again after 1 second
		}
		break;

    case ECHO_ECHO: {
			EchoEchoEvent echo(event);
			LOG_INFO_STR("client:ECHO received, seqnr=" << echo.seqnr);
			itsFinishedTimer = itsClientPort->setTimer(0.5);
		}
		break;

	default:
		status = GCFEvent::NOT_HANDLED;
		break;
	}

	return status;
}

//
// TEST5
//
// The server opens a TCP port with name "SERVER%s:test5" without using the 
// configuration files, using instancenr 5.
// The client does the same.
//
GCFEvent::TResult tServer::test5(GCFEvent& event, GCFPortInterface& /*port*/)
{
	GCFEvent::TResult status = GCFEvent::HANDLED;
	LOG_DEBUG_STR ("Server@test5: " << eventName(event.signal));

	switch (event.signal) {
	case F_INIT:
		break;

	case F_ENTRY:
		itsServerPort = new GCFPort(*this, "SERVER%s:test5", 
									GCFPortInterface::SPP, ECHO_PROTOCOL);
		ASSERTSTR(itsServerPort, "Failed to open serverport in test5"); LOG_INFO("Server:Starting test5");
		itsTimeoutTimer = itsServerPort->setTimer(30.0);	// max duration of test
		itsServerPort->setInstanceNr(5);
		itsServerPort->open();
		break;

	case F_TIMER: {
			// Max testtime reached? Force transition to next test.
			GCFTimerEvent& timerEvent = static_cast<GCFTimerEvent&>(event);
			if (timerEvent.id == itsTimeoutTimer) {
				LOG_WARN("SERVER: TEST 5 FAILED ON TIMEOUT");
				gError |= 0x10;
				itsServerPort->close();	// results in disconnected event
				itsFinished = true;
				break;
			}
			// 'open' timer expired, retry to open the server port.
			if (!itsServerPort->isConnected()) {
				itsServerPort->open();
			}
		}
		break;

	case F_CONNECTED:
		if (itsServerPort->isConnected()) {
			// Alright, the server port is open wait for client or
			// max testtime timer.
		}
		break;

	case F_CLOSED:
	case F_DISCONNECTED:
		if (itsFinished) {	// timer exists when msg was received.
			finishTest(5);
			GCFTask::stop();
//			TRAN(tServer::test3);
		}
		else {
			itsServerPort->setTimer(1.0); // try again after 1 second
		}
		break;

    case ECHO_PING: {
		EchoPingEvent ping(event);
		LOG_INFO_STR ("Server:PING received, seqnr=" << ping.seqnr);

		// Construct echo message and send it.
		timeval echo_time;
		gettimeofday(&echo_time, 0);
		EchoEchoEvent echo;
		echo.seqnr = ping.seqnr;
		echo.ping_time = ping.ping_time;
		echo.echo_time = echo_time;

		itsServerPort->send(echo);
		itsFinished = true;

		break;
    }
	default:
		status = GCFEvent::NOT_HANDLED;
		break;
	}

	return status;
}

GCFEvent::TResult tClient::test5(GCFEvent& event, GCFPortInterface& port)
{
	GCFEvent::TResult status = GCFEvent::HANDLED;
	LOG_DEBUG_STR ("Client@test5: " << eventName(event.signal));

	switch (event.signal) {
	case F_INIT:
		break;

	case F_ENTRY:
		itsClientPort = new GCFPort(*this, "SERVER%s:test5", 
									GCFPortInterface::SAP, ECHO_PROTOCOL);
		ASSERTSTR(itsClientPort, "Failed to create a clientport in test5");
		LOG_INFO("Client:Starting test5");
		itsTimeoutTimer = itsClientPort->setTimer(30.0);	// max duration of test
		itsClientPort->setInstanceNr(5);
		itsClientPort->open();
		break;

	case F_TIMER: {
			// Max testtime reached? Force transition to next test.
			GCFTimerEvent& timerEvent=static_cast<GCFTimerEvent&>(event);
			if ((timerEvent.id == itsTimeoutTimer) || 
				(timerEvent.id == itsFinishedTimer)) {

				if (timerEvent.id == itsTimeoutTimer) {
					LOG_WARN("CLIENT: TEST 5 FAILED ON TIMEOUT");
					gError |= 0x10;
				}
				itsClientPort->close();	// results in disconnected event
				itsFinishedTimer = ~0;
				break;
			}
			// 'open' timer expired, retry to open the client port.
			if (!itsClientPort->isConnected()) {
				itsClientPort->open();
			}
		}
		break;

	case F_CONNECTED:
		if (itsClientPort->isConnected()) {
			// create PingEvent
			timeval ping_time;
			gettimeofday(&ping_time, 0);

			EchoPingEvent ping;
			ping.seqnr     = 5;
			ping.ping_time = ping_time;

			// send the event
			itsClientPort->send(ping);
			LOG_INFO_STR("Client:PING sent (seqnr=" << ping.seqnr);
		}
		break;

	case F_DISCONNECTED:
		port.close();
	case F_CLOSED:
		if (itsFinishedTimer) {
			LOG_INFO ("Client:Lost connection with server");
			finishTest(5);
			GCFTask::stop();
//			TRAN(tClient::test3);
		}
		else {
			itsClientPort->setTimer(1.0); // try again after 1 second
		}
		break;

    case ECHO_ECHO: {
			EchoEchoEvent echo(event);
			LOG_INFO_STR("client:ECHO received, seqnr=" << echo.seqnr);
			itsFinishedTimer = itsClientPort->setTimer(0.5);
		}
		break;

	default:
		status = GCFEvent::NOT_HANDLED;
		break;
	}

	return status;
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
	GCFTask::init(argc, argv);

	tServer	serverTask("SERVER");
	tClient	clientTask("CLIENT");

	serverTask.start(); // make initial transition
	clientTask.start(); // make initial transition

	GCFTask::run();

	LOG_INFO_STR("Test result=" << gError);

	return (gError);
}
