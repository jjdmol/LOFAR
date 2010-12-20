//
//  tGCFPerformance.cc: Test program to test the performance of the send-related routines.
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
//  $Id: tPerformance.cc 13130 2009-04-20 14:18:58Z overeem $

#include <lofar_config.h>
#include <Common/LofarLogger.h>
#include <GCF/TM/GCF_Scheduler.h>
#include "Echo_Protocol.ph"
#include "tGCFPerformance.h"

namespace LOFAR {
 namespace GCF {
  namespace TM {

int	gNrOfTestMessages = 100000;
NSTimer		gTimerTimed;
NSTimer		gTimerUntimed;
double		gTonQon, gToffQon, gTonQoff, gToffQoff;

// ----------- tServer ----------
class tServer : public GCFTask
{
public:
	tServer (string name);

	GCFEvent::TResult initial  (GCFEvent& e, GCFPortInterface& p);
	GCFEvent::TResult connected(GCFEvent& e, GCFPortInterface& p);

private:
	GCFTCPPort* 	itsListener;
	int				itsCounter;
};

tServer::tServer(string name) : 
	GCFTask((State)&tServer::initial, name),
	itsListener(0),
	itsCounter (0)
{
	registerProtocol(ECHO_PROTOCOL, ECHO_PROTOCOL_STRINGS);

	itsListener = new GCFTCPPort(*this, "Echo:Server", GCFPortInterface::SPP, ECHO_PROTOCOL);
	ASSERTSTR(itsListener, "failed to alloc listener");
}

GCFEvent::TResult tServer::initial(GCFEvent& event, GCFPortInterface& /*port*/)
{
	GCFEvent::TResult status = GCFEvent::HANDLED;

	switch (event.signal) {
	case F_INIT: 
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
		if (ping.someValue==0) {	// return respons without same seqnr.
			gTimerUntimed.stop();
			EchoEchoEvent	echo;	// create from scratch (no copy of seqnr)
			echo.ping_time = ping.ping_time;
			port.send(echo);
		}
		else {								// return respons with same seqnr.
			gTimerTimed.stop();
			EchoEchoEvent	echo(event);	// create from original (seqnr is copied)
			echo.ping_time = ping.ping_time;
			port.send(echo);
		}
		break;
	}

	default:
		status = GCFEvent::NOT_HANDLED;
		break;
	}

	return (status);
}

// ---------- tPerformance ----------

// Constructors of both classes
tPerformance::tPerformance(string name) : 
	GCFTask        ((State)&tPerformance::connect, name),
	itsConn 	   (0)
{ 
	registerProtocol (ECHO_PROTOCOL, ECHO_PROTOCOL_STRINGS);
}

tPerformance::~tPerformance()
{
}

//
// connect
//
// set up the connection with the echo server.
//
GCFEvent::TResult tPerformance::connect(GCFEvent& event, GCFPortInterface& /*port*/)
{
	GCFEvent::TResult status = GCFEvent::HANDLED;
	LOG_DEBUG_STR ("tPerformance::connect: " << eventName(event.signal));

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
		TRAN(tPerformance::timedQueued);
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
// timedQueued
//
// First test the worst case scenario with timers and queues on
//
GCFEvent::TResult tPerformance::timedQueued(GCFEvent& event, GCFPortInterface& /*port*/)
{
	switch (event.signal) {
	case F_ENTRY: {
		LOG_INFO("Test timed messages including the queuemechanism");
		itsTimer.start();
		EchoPingEvent	ping;
		ping.someValue=1;		// server will send response with a seqnr.
		gTimerTimed.start();
		itsConn->sendTimed(ping, 100);
		itsCounter=1;
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
		if (itsCounter != gNrOfTestMessages) {
			EchoPingEvent	ping;
			ping.someValue=1;		// server will send response with a seqnr.
			gTimerTimed.start();
			itsConn->sendTimed(ping, 100);
			++itsCounter;
		}
		else {
			itsTimer.stop();
			gTonQon=1000000.0*gTimerTimed.getElapsed()/gTimerTimed.getCount();
			gTimerTimed.reset();
			TRAN(tPerformance::queued);
		}
		break;
	}

	case F_EXIT:
		break;

	default:
		LOG_WARN_STR("DEFAULT in timedQueued: " << eventName(event));
		LOG_WARN_STR(event);
		break;
	}

	return(GCFEvent::HANDLED);
}

//
// queued
//
// Second test the plain send but with queues on
//
GCFEvent::TResult tPerformance::queued(GCFEvent& event, GCFPortInterface& /*port*/)
{
	switch (event.signal) {
	case F_ENTRY: {
		LOG_INFO("Test non-timed messages including the queuemechanism");
		itsTimer.reset();
		itsTimer.start();
		EchoPingEvent	ping;
		ping.someValue=0;		// server will send response without a seqnr.
		gTimerUntimed.start();
		itsConn->send(ping);
		itsCounter=1;
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
		if (itsCounter != gNrOfTestMessages) {
			EchoPingEvent	ping;
			ping.someValue=0;		// server will send response without a seqnr.
			gTimerUntimed.start();
			itsConn->send(ping);
			++itsCounter;
		}
		else {
			itsTimer.stop();
			gToffQon=1000000.0*gTimerUntimed.getElapsed()/gTimerUntimed.getCount();
			gTimerUntimed.reset();
			TRAN(tPerformance::timed);
		}
		break;
	}

	case F_EXIT:
		break;

	default:
		LOG_WARN_STR("DEFAULT in timedQueued: " << eventName(event));
		LOG_WARN_STR(event);
		break;
	}

	return(GCFEvent::HANDLED);
}
//
// timed
//
// Third test timed messages but disable queuing mechanism
//
GCFEvent::TResult tPerformance::timed(GCFEvent& event, GCFPortInterface& /*port*/)
{
	switch (event.signal) {
	case F_ENTRY: {
		LOG_INFO("Test timed messages without the queuemechanism");
		GCFScheduler::instance()->disableQueue();
		itsTimer.reset();
		itsTimer.start();
		EchoPingEvent	ping;
		ping.someValue=1;		// server will send response with a seqnr.
		gTimerTimed.start();
		itsConn->sendTimed(ping, 100);
		itsCounter=1;
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
		if (itsCounter != gNrOfTestMessages) {
			EchoPingEvent	ping;
			ping.someValue=1;		// server will send response without a seqnr.
			gTimerTimed.start();
			itsConn->sendTimed(ping, 100);
			++itsCounter;
		}
		else {
			itsTimer.stop();
			gTonQoff=1000000.0*gTimerTimed.getElapsed()/gTimerTimed.getCount();
			gTimerTimed.reset();
			TRAN(tPerformance::plain);
		}
		break;
	}

	case F_EXIT:
		break;

	default:
		LOG_WARN_STR("DEFAULT in timed: " << eventName(event));
		LOG_WARN_STR(event);
		break;
	}

	return(GCFEvent::HANDLED);
}
//
// plain
//
// Finally test messages without queues and timers
//
GCFEvent::TResult tPerformance::plain(GCFEvent& event, GCFPortInterface& /*port*/)
{
	switch (event.signal) {
	case F_ENTRY: {
		LOG_INFO("Test non-timed messages without the queuemechanism");
		itsTimer.reset();
		itsTimer.start();
		EchoPingEvent	ping;
		ping.someValue=0;		// server will send response without a seqnr.
		gTimerUntimed.start();
		itsConn->send(ping);
		itsCounter=1;
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
		if (itsCounter != gNrOfTestMessages) {
			EchoPingEvent	ping;
			ping.someValue=0;		// server will send response without a seqnr.
			gTimerUntimed.start();
			itsConn->send(ping);
			++itsCounter;
		}
		else {
			itsTimer.stop();
			LOG_INFO(formatString("Timers ON  queues ON : %5.1f us", gTonQon));
			LOG_INFO(formatString("Timers OFF queues ON : %5.1f us", gToffQon));
			LOG_INFO(formatString("Timers ON  queues OFF: %5.1f us", gTonQoff));
			gToffQoff=1000000.0*gTimerUntimed.getElapsed()/gTimerUntimed.getCount();
			LOG_INFO(formatString("Timers OFF queues OFF: %5.1f us", gToffQoff));
			LOG_INFO(formatString("Average cost for timers : %5.1f us", ((gTonQon-gToffQon + gTonQoff-gToffQoff)/2.0)));
			LOG_INFO(formatString("Average cost for queues : %5.1f us", ((gTonQon-gTonQoff + gToffQon-gToffQoff)/2.0)));
			GCFScheduler::instance()->stop();
		}
		break;
	}

	case F_EXIT:
		break;

	default:
		LOG_WARN_STR("DEFAULT in timedQueued: " << eventName(event));
		LOG_WARN_STR(event);
		break;
	}

	return(GCFEvent::HANDLED);
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

	tPerformance	clientTask("clientTask");
	clientTask.start(); // make initial transition

	theScheduler->run();

	return (0);
}
