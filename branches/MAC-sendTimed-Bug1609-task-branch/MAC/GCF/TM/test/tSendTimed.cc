//
//  tSendTimed.cc: Test program to test the GCFTimeout event mechanism.
//
//  Copyright (C) 2010
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
//  $Id: tSendTimed.cc 13130 2009-04-20 14:18:58Z overeem $

#include <lofar_config.h>
#include <Common/LofarLogger.h>
#include <GCF/TM/GCF_Scheduler.h>
#include "Echo_Protocol.ph"
#include "tSendTimed.h"

namespace LOFAR {
 namespace GCF {
  namespace TM {

// ----------- tServer ----------
class tServer : public GCFTask
{
public:
	tServer (string name);

	GCFEvent::TResult initial  (GCFEvent& e, GCFPortInterface& p);
	GCFEvent::TResult connected(GCFEvent& e, GCFPortInterface& p);

private:
	GCFTCPPort* 	itsListener;
	GCFTimerPort*	itsTimerPort;
	EchoEchoEvent*	itsEchoEvent;
};

tServer::tServer(string name) : 
	GCFTask((State)&tServer::initial, name),
	itsListener(0),
	itsTimerPort(0),
	itsEchoEvent(0)
{
	registerProtocol(ECHO_PROTOCOL, ECHO_PROTOCOL_STRINGS);

	itsListener = new GCFTCPPort(*this, "Echo:Server", GCFPortInterface::SPP, ECHO_PROTOCOL);
	ASSERTSTR(itsListener, "failed to alloc listener");

	itsTimerPort = new GCFTimerPort(*this, "timerPort");
	ASSERTSTR(itsTimerPort, "failed to alloc timer for server task");
}

GCFEvent::TResult tServer::initial(GCFEvent& event, GCFPortInterface& /*port*/)
{
	GCFEvent::TResult status = GCFEvent::HANDLED;

	switch (event.signal) {
	case F_INIT: {
		itsTimerPort->setTimer(1.0);
	}
	break;

    case F_TIMER:
		LOG_DEBUG("STARTING server");
		itsListener->open();
		break;

    case F_CONNECTED:
		if (itsListener->isConnected()) {
			TRAN(tServer::connected);
		}
		break;

    case F_DISCONNECTED:
		ASSERTSTR(false, "Bailing out because server could not be started");
		GCFScheduler::instance()->stop();
		break;

    default:
		status = GCFEvent::NOT_HANDLED;
		break;
	}

	return (status);
}

GCFEvent::TResult tServer::connected(GCFEvent& event, GCFPortInterface& port)
{
	GCFEvent::TResult status = GCFEvent::HANDLED;

	LOG_DEBUG_STR("SERVER: " << eventName(event.signal));

	switch (event.signal) {
	case ECHO_PING: {
		EchoPingEvent	ping(event);
		if (ping.someValue < 0) {	// return respons immediately without a seqnr.
			EchoEchoEvent	echo;	// create from scratch (no copy of seqnr)
			echo.seqnr     = ping.seqnr;
			echo.someName  = "immediate,seqnr=0";
			echo.ping_time = ping.ping_time;
			port.send(echo);
		}
		else if (ping.someValue == 0) {		// return respons immediately with a seqnr.
			EchoEchoEvent	echo(event);	// create from original (seqnr is copied)
			echo.seqnr     = ping.seqnr;
			echo.someName  = "immediate,seqnr=...";
			echo.ping_time = ping.ping_time;
			port.send(echo);
		}
		else {								// return response after a while.
			itsEchoEvent = new EchoEchoEvent(event);
			itsEchoEvent->seqnr     = ping.seqnr;
			itsEchoEvent->someName  = formatString("delay=%dms,seqnr=...",ping.someValue);
			itsEchoEvent->ping_time = ping.ping_time;
			port.setTimer(ping.someValue / 1000.0);
		}
		break;
	}

	case F_TIMER:
		port.send(*itsEchoEvent);
		delete itsEchoEvent;
		itsEchoEvent = 0;
		break;

	default:
		status = GCFEvent::NOT_HANDLED;
		break;
	}

	return (status);
}

// ---------- tSendTimed ----------

// Constructors of both classes
tSendTimed::tSendTimed(string name) : 
	GCFTask        ((State)&tSendTimed::connect, name),
	itsTimerPort   (0),
	itsConn 	   (0)
{ 
	registerProtocol (ECHO_PROTOCOL, ECHO_PROTOCOL_STRINGS);

	itsTimerPort = new GCFTimerPort(*this, "timerPort");
	ASSERTSTR(itsTimerPort, "failed to alloc timer for client task");
}

tSendTimed::~tSendTimed()
{
	delete itsTimerPort;
}

//
// connect
//
// set up the connection with the echo server.
//
GCFEvent::TResult tSendTimed::connect(GCFEvent& event, GCFPortInterface& /*port*/)
{
	GCFEvent::TResult status = GCFEvent::HANDLED;
	LOG_DEBUG_STR ("tSendTimed::connect: " << eventName(event.signal));

	switch (event.signal) {
	case F_ENTRY:
		break;

	case F_INIT: {
		itsConn = new GCFTCPPort(*this, "Echo:Server", GCFPortInterface::SAP, ECHO_PROTOCOL);
		ASSERTSTR(itsConn, "Can't allocate a TCPport");
		LOG_DEBUG("Calling autoOpen(5, 0, 4)");
		itsConn->autoOpen(5, 0, 4); // nrRetry, timeout, retryItv
		break;
	}

	case F_CONNECTED:
		LOG_DEBUG_STR("Connected to the echoServer, going to test 1");
		TRAN(tSendTimed::sendTest1);
		break;

	case F_DISCONNECTED:
		LOG_DEBUG_STR("NO CONNECTION WITH THE ECHOSERVER POSSIBLE, QUITING!");
		GCFScheduler::instance()->stop();
		break;

	case F_EXIT:
		break;

	default:
		LOG_WARN_STR("DEFAULT in ::connect: " << eventName(event));
		status = GCFEvent::NOT_HANDLED;
		break;
	}

	return status;
}

//
// sendTest1
//
// First test to old 'send' function without timeout timers.
//
GCFEvent::TResult tSendTimed::sendTest1(GCFEvent& event, GCFPortInterface& /*port*/)
{
	GCFEvent::TResult status = GCFEvent::HANDLED;
	LOG_DEBUG_STR ("tSendTimed::sendTest1: " << eventName(event.signal));

	switch (event.signal) {
	case F_ENTRY: {
		LOG_DEBUG("\n\n\n\nTest conventional communication without seqnrs and timers");
		EchoPingEvent	ping;
		ping.seqnr=300563;
		ping.someValue=-1;		// server will send response without a seqnr.
		itsConn->send(ping);
		break;
	}

	case F_DISCONNECTED:
		LOG_DEBUG_STR("LOST CONNECTION WITH THE SERVER, QUITING!");
		GCFScheduler::instance()->stop();
		break;

	case F_TIMEOUT: {
		GCFTimeoutEvent& 	ti = static_cast<GCFTimeoutEvent&>(event);
		LOG_DEBUG_STR("UNEXPECTED TIMEOUT EVENT OF MESSAGE " << eventName(ti.orgSignal()) << ", seqnr=" << ti.orgSeqnr());
		LOG_DEBUG_STR(ti);
		GCFScheduler::instance()->stop();
		break;
	}

	case ECHO_ECHO: {
		EchoEchoEvent	echo(event);
		if (echo.seqnr != 300563) {
			LOG_ERROR("WRONG DATAPACKET WAS RETURNED, QUITING");
			GCFScheduler::instance()->stop();
			break;
		}
		LOG_DEBUG("Got the right answer from the server, going to test 2");
		TRAN(tSendTimed::sendTest2);
		break;
	}

	case F_EXIT:
		break;

	default:
		LOG_WARN_STR("DEFAULT in sendTest1: " << eventName(event));
		LOG_WARN_STR(event);
		status = GCFEvent::NOT_HANDLED;
		break;
	}

	return status;
}

//
// sendTest2
//
// First test to old 'send' function without timeout timers.
//
GCFEvent::TResult tSendTimed::sendTest2(GCFEvent& event, GCFPortInterface& /*port*/)
{
	GCFEvent::TResult status = GCFEvent::HANDLED;
	LOG_DEBUG_STR ("tSendTimed::sendTest2: " << eventName(event.signal));

	switch (event.signal) {
	case F_ENTRY: {
		LOG_DEBUG("\n\n\n\nTest conventional communication with seqnrs but without timers");
		EchoPingEvent	ping;
		ping.seqnr=300563;
		ping.someValue=0;		// server will send response with a seqnr immediate.
		itsConn->send(ping);
		break;
	}

	case F_DISCONNECTED:
		LOG_DEBUG_STR("LOST CONNECTION WITH THE SERVER, QUITING!");
		GCFScheduler::instance()->stop();
		break;

	case F_TIMEOUT: {
		GCFTimeoutEvent& 	ti = static_cast<GCFTimeoutEvent&>(event);
		LOG_DEBUG_STR("UNEXPECTED TIMEOUT EVENT OF MESSAGE " << eventName(ti.orgSignal()) << ", seqnr=" << ti.orgSeqnr());
		LOG_DEBUG_STR(ti);
		GCFScheduler::instance()->stop();
		break;
	}

	case ECHO_ECHO: {
		EchoEchoEvent	echo(event);
		if (echo.seqnr != 300563) {
			LOG_ERROR("WRONG DATAPACKET WAS RETURNED, QUITING");
			GCFScheduler::instance()->stop();
			break;
		}
		LOG_DEBUG("Got the right answer from the server, going to test 3");
		TRAN(tSendTimed::sendTest3);
		break;
	}

	case F_EXIT:
		break;

	default:
		LOG_WARN_STR("DEFAULT in sendTest2: " << eventName(event));
		LOG_WARN_STR(event);
		status = GCFEvent::NOT_HANDLED;
		break;
	}

	return status;
}
//
// sendTest3
//
// First test to old 'send' function without timeout timers.
//
GCFEvent::TResult tSendTimed::sendTest3(GCFEvent& event, GCFPortInterface& /*port*/)
{
	GCFEvent::TResult status = GCFEvent::HANDLED;
	LOG_DEBUG_STR ("tSendTimed::sendTest3: " << eventName(event.signal));

	switch (event.signal) {
	case F_ENTRY: {
		LOG_DEBUG("\n\n\n\nTest conventional communication with seqnrs and timers");
		EchoPingEvent	ping;
		ping.seqnr=300563;
		ping.someValue=200;		// server will send response with a seqnr after 200 ms.
		itsConn->send(ping);
		break;
	}

	case F_DISCONNECTED:
		LOG_DEBUG_STR("LOST CONNECTION WITH THE SERVER, QUITING!");
		GCFScheduler::instance()->stop();
		break;

	case F_TIMEOUT: {
		GCFTimeoutEvent& 	ti = static_cast<GCFTimeoutEvent&>(event);
		LOG_DEBUG_STR("UNEXPECTED TIMEOUT EVENT OF MESSAGE " << eventName(ti.orgSignal()) << ", seqnr=" << ti.orgSeqnr());
		LOG_DEBUG_STR(ti);
		GCFScheduler::instance()->stop();
		break;
	}

	case ECHO_ECHO: {
		EchoEchoEvent	echo(event);
		if (echo.seqnr != 300563) {
			LOG_ERROR("WRONG DATAPACKET WAS RETURNED, QUITING");
			GCFScheduler::instance()->stop();
			break;
		}
		LOG_DEBUG("Got the right answer from the server, going to test 4");
		TRAN(tSendTimed::sendTest4);
		break;
	}

	case F_EXIT:
		break;

	default:
		LOG_WARN_STR("DEFAULT in sendTest4: " << eventName(event));
		LOG_WARN_STR(event);
		status = GCFEvent::NOT_HANDLED;
		break;
	}

	return status;
}

//
// sendTest4
//
// test the new sendTimed function
//
GCFEvent::TResult tSendTimed::sendTest4(GCFEvent& event, GCFPortInterface& /*port*/)
{
	GCFEvent::TResult status = GCFEvent::HANDLED;
	LOG_DEBUG_STR ("tSendTimed::sendTest4: " << eventName(event.signal));

	switch (event.signal) {
	case F_ENTRY: {
		LOG_DEBUG("\n\n\n\nTest timed communication with seqnrs and immediate response");
		EchoPingEvent	ping;
		ping.seqnr=300563;
		ping.someName ="test4";
		ping.someValue=0;		// server will send response with a seqnr immediate.
		itsConn->sendTimed(ping, 500);
		break;
	}

	case F_DISCONNECTED:
		LOG_DEBUG_STR("LOST CONNECTION WITH THE SERVER, QUITING!");
		GCFScheduler::instance()->stop();
		break;

	case F_TIMEOUT: {
		GCFTimeoutEvent& 	ti = static_cast<GCFTimeoutEvent&>(event);
		LOG_DEBUG_STR("UNEXPECTED TIMEOUT EVENT OF MESSAGE " << eventName(ti.orgSignal()) << ", seqnr=" << ti.orgSeqnr());
		LOG_DEBUG_STR(ti);
		GCFScheduler::instance()->stop();
		break;
	}

	case ECHO_ECHO: {
		EchoEchoEvent	echo(event);
		if (echo.seqnr != 300563) {
			LOG_ERROR("WRONG DATAPACKET WAS RETURNED, QUITING");
			GCFScheduler::instance()->stop();
			break;
		}
		LOG_DEBUG("Got the right answer from the server, going to test 5");
		sleep (1);
		TRAN(tSendTimed::sendTest5);
		break;
	}

	case F_EXIT:
		break;

	default:
		LOG_WARN_STR("DEFAULT in sendTest4: " << eventName(event));
		LOG_WARN_STR(event);
		status = GCFEvent::NOT_HANDLED;
		break;
	}

	return status;
}

//
// sendTest5
//
// test the new sendTimed function
//
GCFEvent::TResult tSendTimed::sendTest5(GCFEvent& event, GCFPortInterface& /*port*/)
{
	GCFEvent::TResult status = GCFEvent::HANDLED;
	LOG_DEBUG_STR ("tSendTimed::sendTest5: " << eventName(event.signal));

	switch (event.signal) {
	case F_ENTRY: {
		LOG_DEBUG("\n\n\n\nTest timed communication with seqnrs and delayed (but in time) response");
		EchoPingEvent	ping;
		ping.seqnr=300563;
		ping.someName ="test5";
		ping.someValue=300;		// server will send response with a seqnr after 300ms.
		itsConn->sendTimed(ping, 500);	// wait max 500 ms
		break;
	}

	case F_DISCONNECTED:
		LOG_DEBUG_STR("LOST CONNECTION WITH THE SERVER, QUITING!");
		GCFScheduler::instance()->stop();
		break;

	case F_TIMEOUT: {
		GCFTimeoutEvent& 	ti = static_cast<GCFTimeoutEvent&>(event);
		LOG_DEBUG_STR("UNEXPECTED TIMEOUT EVENT OF MESSAGE " << eventName(ti.orgSignal()) << ", seqnr=" << ti.orgSeqnr());
		LOG_DEBUG_STR(ti);
		GCFScheduler::instance()->stop();
		break;
	}

	case ECHO_ECHO: {
		EchoEchoEvent	echo(event);
		if (echo.seqnr != 300563) {
			LOG_ERROR("WRONG DATAPACKET WAS RETURNED, QUITING");
			GCFScheduler::instance()->stop();
			break;
		}
		LOG_DEBUG("Got the right answer from the server, going to test 6");
		sleep (1);
		TRAN(tSendTimed::sendTest6);
		break;
	}

	case F_EXIT:
		break;

	default:
		LOG_WARN_STR("DEFAULT in sendTest5: " << eventName(event));
		LOG_WARN_STR(event);
		status = GCFEvent::NOT_HANDLED;
		break;
	}

	return status;
}

//
// sendTest6
//
// test the new sendTimed function
//
GCFEvent::TResult tSendTimed::sendTest6(GCFEvent& event, GCFPortInterface& /*port*/)
{
	GCFEvent::TResult status = GCFEvent::HANDLED;
	LOG_DEBUG_STR ("tSendTimed::sendTest6: " << eventName(event.signal));

	switch (event.signal) {
	case F_ENTRY: {
		LOG_DEBUG("\n\n\n\nTest timed communication with seqnrs and delayed (too late) response");
		EchoPingEvent	ping;
		ping.seqnr=300563;
		ping.someName ="test6";
		ping.someValue=700;		// server will send response with a seqnr after 700ms.
		itsConn->sendTimed(ping, 500);	// wait max 500 ms
		break;
	}

	case F_DISCONNECTED:
		LOG_DEBUG_STR("LOST CONNECTION WITH THE SERVER, QUITING!");
		GCFScheduler::instance()->stop();
		break;

	case F_TIMEOUT: {
		GCFTimeoutEvent& 	ti = static_cast<GCFTimeoutEvent&>(event);
		LOG_DEBUG_STR("Received the expected timeout event of message " << eventName(ti.orgSignal()) << ", seqnr=" << ti.orgSeqnr());
		LOG_DEBUG_STR(ti);
		itsTimerPort->setTimer(2.0);	// will response still arrive?
		break;
	}

	case ECHO_ECHO: {
		EchoEchoEvent	echo(event);
		if (echo.seqnr != 300563) {
			LOG_ERROR("WRONG DATAPACKET WAS RETURNED, QUITING");
			GCFScheduler::instance()->stop();
			break;
		}
		LOG_DEBUG("ALTHOUGH MSGS WAS TIMED OUT THE ANSWER WAS STILL RECEIVED!!!");
		GCFScheduler::instance()->stop();
		break;
	}

	case F_TIMER:
		LOG_DEBUG_STR("ALL TEST PASSED SUCCESSFUL!!");
		GCFScheduler::instance()->stop();
		break;

	case F_EXIT:
		break;

	default:
		LOG_WARN_STR("DEFAULT in sendTest6: " << eventName(event));
		LOG_WARN_STR(event);
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
	GCFScheduler*	theScheduler(GCFScheduler::instance());
	theScheduler->init(argc, argv);

	tServer	serverTask("serverTask");
	serverTask.start(); // make initial transition

	tSendTimed	clientTask("clientTask");
	clientTask.start(); // make initial transition

	theScheduler->run();

	return (0);
}
