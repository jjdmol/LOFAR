//
//  tEventQueue.cc: Test program to test all kind of usage of the GCF ports.
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
#include <GCF/TM/GCF_Scheduler.h>
#include "Echo_Protocol.ph"
#include "tEventQueue.h"
#include "tClient.h"

static	int	gTest = 0;
static	int	gTimerID;

namespace LOFAR {
 namespace GCF {
  namespace TM {


// Constructors of both classes
tEventQueue::tEventQueue(string name) : 
	GCFTask         ((State)&tEventQueue::timerTest, name),
	itsTimerPort    (0)
{ 
	registerProtocol (ECHO_PROTOCOL, ECHO_PROTOCOL_STRINGS);
}

tEventQueue::~tEventQueue()
{
	delete itsTimerPort;
}

//
// timerTest
//
// We simply set one timer in different ways and wait for the timer to expire.
//
GCFEvent::TResult tEventQueue::timerTest(GCFEvent& event, GCFPortInterface& /*port*/)
{
	GCFEvent::TResult status = GCFEvent::HANDLED;
	LOG_DEBUG_STR ("tEventQueue::timerTest: " << eventName(event.signal));

	switch (event.signal) {
	case F_ENTRY:
		itsTimerPort = new GCFTimerPort(*this, "timerPort");
		ASSERTSTR(itsTimerPort, "Failed to open timerport");
		break;

	case F_INIT:
		gTest=1;
		gTimerID = itsTimerPort->setTimer(1.0);
		LOG_DEBUG_STR("setTimer(1.0) = " << gTimerID);
		break;

	case F_TIMER: {
		switch (gTest) {
			case 1: {	// wait for setTimer(1.0)
				GCFTimerEvent& timerEvent = static_cast<GCFTimerEvent&>(event);
				LOG_DEBUG_STR("ID = " << timerEvent.id << ", arg = " << timerEvent.arg);

				gTimerID = itsTimerPort->setTimer(1.0, 2.0);
				LOG_DEBUG_STR("setTimer(1.0, 2.0) = " << gTimerID);
				gTest++;
			}
			break;

			case 2: { // wait for first expire of setTimer(1.0, 2.0)
				GCFTimerEvent& timerEvent = static_cast<GCFTimerEvent&>(event);
				LOG_DEBUG_STR("ID = " << timerEvent.id << ", arg = " << timerEvent.arg);
				gTest++;
			}
			break;

			case 3: {	// wait for second expire of setTimer(1.0, 2.0)
				GCFTimerEvent& timerEvent = static_cast<GCFTimerEvent&>(event);
				LOG_DEBUG_STR("ID = " << timerEvent.id << ", arg = " << timerEvent.arg);
				itsTimerPort->cancelTimer(gTimerID);

				gTimerID = itsTimerPort->setTimer(1.0, 1.0, (char*)"pietje puk");
				LOG_DEBUG_STR("setTimer(1.0, 0.0, 'pietje puk') = " << gTimerID);
				gTest++;
			}
			break;
			case 4: {
				GCFTimerEvent& timerEvent = static_cast<GCFTimerEvent&>(event);
				LOG_DEBUG_STR("ID = " << timerEvent.id << ", arg = " << timerEvent.arg);
				LOG_DEBUG_STR("ID = " << timerEvent.id << ", *arg = " << (char*)timerEvent.arg);
				gTest++;
			}
			break;

			default: {
				GCFTimerEvent& timerEvent = static_cast<GCFTimerEvent&>(event);
				LOG_DEBUG_STR("ID = " << timerEvent.id << ", *arg = " << (char*)timerEvent.arg);
				if (gTest++ > 8) {
					itsTimerPort->cancelTimer(gTimerID);
					LOG_INFO("Timertest passed, Going to the next statemachine\n\n\n");
					TRAN(tEventQueue::tranTest);
					LOG_INFO("Just after 'TRAN(tEventQueue::tranTest)', still in timerTest");
				}
			}
			break;
		}
		break;
	}
	break;

	case F_EXIT:
		LOG_INFO("F_EXIT event in timerTest");
		break;

	default:
		LOG_WARN_STR("DEFAULT in timerTest: " << eventName(event));
		status = GCFEvent::NOT_HANDLED;
		break;
	}

	return status;
}

//
// tranTest
//
// We simply set one timer in different ways and wait for the timer to expire.
//
GCFEvent::TResult tEventQueue::tranTest(GCFEvent& event, GCFPortInterface& /*port*/)
{
	GCFEvent::TResult status = GCFEvent::HANDLED;
	LOG_DEBUG_STR ("tEventQueue::tranTest: " << eventName(event.signal));

	switch (event.signal) {
	case F_ENTRY:
		LOG_INFO("Just setting a timer to 5 seconds");
		itsTimerPort->setTimer(5.0);
		break;

	case F_TIMER:
		LOG_DEBUG("Timer expired, going to listener test\n\n\n");
		TRAN(tEventQueue::listenerTest);
		break;

	case F_EXIT:
		break;

	default:
		LOG_WARN_STR("DEFAULT in tranTest: " << eventName(event));
		status = GCFEvent::NOT_HANDLED;
		break;
	}

	return status;
}

//
// listenerTest 3
//
// Opening a listener port AND tries to start a task in a running scheduler.
//
GCFEvent::TResult tEventQueue::listenerTest(GCFEvent& event, GCFPortInterface& port)
{
	GCFEvent::TResult status = GCFEvent::HANDLED;
	LOG_DEBUG_STR ("tEventQueue::listenerTest: " << eventName(event.signal));

	switch (event.signal) {
	case F_ENTRY: {
			LOG_INFO("Going to open a listener port");
			itsListener = new GCFTCPPort(*this, "EchoServer:v1.0", GCFPortInterface::MSPP, ECHO_PROTOCOL);
			ASSERTSTR(itsListener, "Failed to allocate listenerport");
		}
		// NO BREAK
	case F_TIMER:
		itsListener->open();		// results in F_DISCON or F_CONN
		break;

	case F_DISCONNECTED:
		LOG_INFO("Opening listener failed, waiting for close event");
		port.close();
		break;

	case F_CLOSED:
		LOG_INFO("Listener closed, setting timer to 2 seconds for retry");
		itsTimerPort->setTimer(2.0);
		break;

	case F_CONNECTED: {
		itsTimerPort->cancelAllTimers();
		LOG_INFO("Listener opened, creating a new task now that will connect to me\n\n\n");
		tClient*	client = new tClient("client");
		client->start();
		TRAN(tEventQueue::wait4client2connect);
		break;
	}

	case F_EXIT:
		break;

	default:
		LOG_WARN_STR("DEFAULT in listenerTest: " << eventName(event));
		status = GCFEvent::NEXT_STATE;
		break;
	}

	return status;
}
//
// wait4client2connect
//
// Opening a listener port
//
GCFEvent::TResult tEventQueue::wait4client2connect(GCFEvent& event, GCFPortInterface& /*port*/)
{
	GCFEvent::TResult status = GCFEvent::HANDLED;
	LOG_DEBUG_STR ("tEventQueue::wait4client2connect: " << eventName(event.signal));

	switch (event.signal) {
	case F_ENTRY:
		itsTimerPort->setTimer(30.0);		// in case it does not work
		break;

	case F_TIMER:
		LOG_INFO("Client did NOT connect, bailing out!");
		GCFScheduler::instance()->stop();
		break;

	case F_ACCEPT_REQ:
		itsClient = new GCFTCPPort();
		itsClient->init(*this, "client", GCFPortInterface::SPP, ECHO_PROTOCOL);
		ASSERTSTR(itsListener->accept(*itsClient), "ACCEPT FAILED");
		break;

	case F_CONNECTED:
		LOG_INFO("Client is connected, going to do my Server state\n\n\n");
		itsTimerPort->cancelAllTimers();
		TRAN(tEventQueue::serverState);
		break;

	case F_EXIT:
		break;

	default:
		LOG_WARN_STR("DEFAULT in wait4client2connect: " << eventName(event));
		status = GCFEvent::NEXT_STATE;
		break;
	}

	return status;
}

//
// serverState
//
// wait for Ping messages and reply to them
//
GCFEvent::TResult tEventQueue::serverState(GCFEvent& event, GCFPortInterface& port)
{
	GCFEvent::TResult status = GCFEvent::HANDLED;
	LOG_DEBUG_STR ("tEventQueue::serverState: " << eventName(event.signal));

	switch (event.signal) {
	case F_ENTRY:
		itsTimerPort->setTimer(30.0);		// in case it does not work
		break;

	case F_TIMER:
		LOG_INFO("Client did NOT send a PING, bailing out!");
		GCFScheduler::instance()->stop();
		break;

	case ECHO_PING: {
		EchoPingEvent	ping(event);
		EchoEchoEvent	echo;
		echo.seqnr     = ping.seqnr;
		port.send(echo);
		itsTimerPort->cancelAllTimers();
		LOG_DEBUG_STR("YES, ping was received, echo was sent, I'm ready!");
		break;
	}

	case F_DISCONNECTED:
		LOG_DEBUG_STR("Closing port because i received a disconnect event");
		port.close();

	case F_CLOSED:
		LOG_DEBUG_STR("Succesfully terminated the test, Stopping program");
		GCFScheduler::instance()->stop();
		break;

	case F_EXIT:
		break;

	default:
		LOG_WARN_STR("DEFAULT in serverState: " << eventName(event));
		status = GCFEvent::HANDLED;
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

	tEventQueue	eqTask("eqTask");
	eqTask.start(); // make initial transition

	theScheduler->run();

	return (0);
}
