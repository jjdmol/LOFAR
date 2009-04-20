//
//  tAutoOpen.cc: Test program to test all kind of usage of the GCF ports.
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
#include <GCF/TM/GCF_Scheduler.h>
#include "Echo_Protocol.ph"
#include "tAutoOpen.h"
#include "tServer.h"

namespace LOFAR {
 namespace GCF {
  namespace TM {


// Constructors of both classes
tAutoOpen::tAutoOpen(string name) : 
	GCFTask        ((State)&tAutoOpen::retryTest, name),
	itsTimerPort   (0),
	itsConn 	   (0)
{ 
	registerProtocol (ECHO_PROTOCOL, ECHO_PROTOCOL_STRINGS);
}

tAutoOpen::~tAutoOpen()
{
	delete itsTimerPort;
}

//
// retryTest
//
// Try to autoOpen a connection to a non existing server specifying several retries but no timeout.
//
GCFEvent::TResult tAutoOpen::retryTest(GCFEvent& event, GCFPortInterface& /*port*/)
{
	GCFEvent::TResult status = GCFEvent::HANDLED;
	LOG_DEBUG_STR ("tAutoOpen::retryTest: " << eventName(event.signal));

	switch (event.signal) {
	case F_ENTRY:
		break;

	case F_INIT: {
		itsConn = new GCFTCPPort(*this, "Echo:Server", GCFPortInterface::SAP, ECHO_PROTOCOL, false);
		ASSERTSTR(itsConn, "Can't allocate a TCPport");
		LOG_DEBUG("Calling autoOpen(5, 0, 4)");
		itsConn->autoOpen(5, 0, 4); // nrRetry, timeout, retryItv
		break;
	}

	case F_CONNECTED:
		LOG_DEBUG_STR("THIS IS REALY STRANGE!!!");
		GCFScheduler::instance()->stop();
		break;

	case F_DISCONNECTED:
		LOG_DEBUG_STR("Seems that retries are working, going to try the timeout.");
		TRAN(tAutoOpen::timeoutTest);
		break;

	case F_EXIT:
		LOG_INFO("F_EXIT event in retryTest");
		break;

	default:
		LOG_WARN_STR("DEFAULT in retryTest: " << eventName(event));
		status = GCFEvent::NOT_HANDLED;
		break;
	}

	return status;
}

//
// timeoutTest
//
// Try to autoOpen a connection to a non existing server specifying a timeout
//
GCFEvent::TResult tAutoOpen::timeoutTest(GCFEvent& event, GCFPortInterface& /*port*/)
{
	GCFEvent::TResult status = GCFEvent::HANDLED;
	LOG_DEBUG_STR ("tAutoOpen::timeoutTest: " << eventName(event.signal));

	switch (event.signal) {
	case F_ENTRY:
		LOG_DEBUG("\n\n\n\nCalling autoOpen(0, 18, 4)");
		itsConn->autoOpen(0, 18, 4); // nrRetry, timeout, retryItv
		break;

	case F_CONNECTED:
		LOG_DEBUG_STR("THIS IS REALY STRANGE!!!");
		GCFScheduler::instance()->stop();
		break;

	case F_DISCONNECTED:
		LOG_DEBUG_STR("timeTest seems to be working, trying next mode");
		TRAN(tAutoOpen::zeroTest);
		break;

	case F_EXIT:
		LOG_INFO("F_EXIT event in timeoutTest");
		break;

	default:
		LOG_WARN_STR("DEFAULT in timeoutTest: " << eventName(event));
		status = GCFEvent::NOT_HANDLED;
		break;
	}

	return status;
}


//
// zeroTest
//
// Try to autoOpen a connection to a non existing server not setting the number of retries
// nor setting the timeout timer. This should behave like a normal open() call.
//
GCFEvent::TResult tAutoOpen::zeroTest(GCFEvent& event, GCFPortInterface& /*port*/)
{
	GCFEvent::TResult status = GCFEvent::HANDLED;
	LOG_DEBUG_STR ("tAutoOpen::zeroTest: " << eventName(event.signal));

	switch (event.signal) {
	case F_ENTRY:
		LOG_DEBUG("\n\n\n\nCalling autoOpen(0, 0, 4)");
		itsConn->autoOpen(0, 0, 4); // nrRetry, timeout, retryItv
		break;

	case F_CONNECTED:
		LOG_DEBUG_STR("THIS IS REALY STRANGE!!!");
		GCFScheduler::instance()->stop();
		break;

	case F_DISCONNECTED:
		LOG_DEBUG_STR("Seems to be working, going to the next test");
		TRAN(tAutoOpen::double1Test);
		break;

	case F_EXIT:
		LOG_INFO("F_EXIT event in zeroTest");
		break;

	default:
		LOG_WARN_STR("DEFAULT in zeroTest: " << eventName(event));
		status = GCFEvent::NOT_HANDLED;
		break;
	}

	return status;
}

//
// double1Test
//
// Try to autoOpen a connection to a non existing server setting the number of retries
// AND setting the timeout timer. The nr of retries is smaller than the maxTimeout.
//
GCFEvent::TResult tAutoOpen::double1Test(GCFEvent& event, GCFPortInterface& /*port*/)
{
	GCFEvent::TResult status = GCFEvent::HANDLED;
	LOG_DEBUG_STR ("tAutoOpen::double1Test: " << eventName(event.signal));

	switch (event.signal) {
	case F_ENTRY:
		LOG_DEBUG("\n\n\n\nCalling autoOpen(3, 60, 4)");
		itsConn->autoOpen(3, 60, 4); // nrRetry, timeout, retryItv
		break;

	case F_CONNECTED:
		LOG_DEBUG_STR("THIS IS REALY STRANGE!!!");
		GCFScheduler::instance()->stop();
		break;

	case F_DISCONNECTED:
		LOG_DEBUG_STR("Seems to be working, going to the next test");
		TRAN(tAutoOpen::double2Test);
		break;

	case F_EXIT:
		LOG_INFO("F_EXIT event in double1Test");
		break;

	default:
		LOG_WARN_STR("DEFAULT in double1Test: " << eventName(event));
		status = GCFEvent::NOT_HANDLED;
		break;
	}

	return status;
}

//
// double2Test
//
// Try to autoOpen a connection to a non existing server setting the number of retries
// AND setting the timeout timer. The nr of retries is larger than the maxTimeout.
//
GCFEvent::TResult tAutoOpen::double2Test(GCFEvent& event, GCFPortInterface& /*port*/)
{
	GCFEvent::TResult status = GCFEvent::HANDLED;
	LOG_DEBUG_STR ("tAutoOpen::double2Test: " << eventName(event.signal));

	switch (event.signal) {
	case F_ENTRY:
		LOG_DEBUG("\n\n\n\nCalling autoOpen(6, 10, 4)");
		itsConn->autoOpen(6, 10, 4); // nrRetry, timeout, retryItv
		break;

	case F_CONNECTED:
		LOG_DEBUG_STR("THIS IS REALY STRANGE!!!");
		GCFScheduler::instance()->stop();
		break;

	case F_DISCONNECTED:
		LOG_DEBUG_STR("Seems to be working, going to the next test");
		TRAN(tAutoOpen::openTest);
		break;

	case F_EXIT:
		LOG_INFO("F_EXIT event in double2Test");
		break;

	default:
		LOG_WARN_STR("DEFAULT in double2Test: " << eventName(event));
		status = GCFEvent::NOT_HANDLED;
		break;
	}

	return status;
}

//
// openTest
//
// Test is the old open() call is still working.
//
GCFEvent::TResult tAutoOpen::openTest(GCFEvent& event, GCFPortInterface& /*port*/)
{
	GCFEvent::TResult status = GCFEvent::HANDLED;
	LOG_DEBUG_STR ("tAutoOpen::openTest: " << eventName(event.signal));

	switch (event.signal) {
	case F_ENTRY:
		LOG_DEBUG("\n\n\n\nCalling open()");
		itsConn->open();
		break;

	case F_CONNECTED:
		LOG_DEBUG_STR("THIS IS REALY STRANGE!!!");
		GCFScheduler::instance()->stop();
		break;

	case F_DISCONNECTED:
		LOG_DEBUG_STR("Seems to be working, going to last test");
		TRAN(tAutoOpen::relayedOpenTest);
		break;

	case F_EXIT:
		LOG_INFO("F_EXIT event in openTest");
		break;

	default:
		LOG_WARN_STR("DEFAULT in openTest: " << eventName(event));
		status = GCFEvent::NOT_HANDLED;
		break;
	}

	return status;
}

//
// relayedOpenTest
//
// Start a server task that will open the listener after 25 second. Start an autoOpen call
// in this taks and wait till the connection is established.
//
GCFEvent::TResult tAutoOpen::relayedOpenTest(GCFEvent& event, GCFPortInterface& port)
{
	GCFEvent::TResult status = GCFEvent::HANDLED;
	LOG_DEBUG_STR ("tAutoOpen::relayedOpenTest: " << eventName(event.signal));

	switch (event.signal) {
	case F_ENTRY: {
		LOG_DEBUG("\n\n\n\nStarting the server task");
		tServer*	server = new tServer("Server", 25);
		server->start();
		itsConn->autoOpen(10, 0, 10);	// connect within 100 seconds.
		break;
	}

	case F_CONNECTED: {
		LOG_DEBUG("COOL its is realy working");
		EchoPingEvent	ping;
		ping.seqnr=300563;
		port.send(ping);
		break;
	}

	case ECHO_ECHO: {
		EchoEchoEvent	echo(event);
		if (echo.seqnr != 300563) {
			LOG_ERROR("Wrong sequence number");
		}
		else {
			LOG_DEBUG("Got the right answer from the server, FINISHED the tests");
		}
		GCFScheduler::instance()->stop();
		break;
	}

	case F_DISCONNECTED:
		LOG_DEBUG_STR("WE STILL HAVE A PROBLEM");
		break;

	case F_EXIT:
		LOG_INFO("F_EXIT event in relayedOpenTest");
		break;

	default:
		LOG_WARN_STR("DEFAULT in relayedOpenTest: " << eventName(event));
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

	tAutoOpen	clientTask("clientTask");
	clientTask.start(); // make initial transition

	theScheduler->run();

	return (0);
}
