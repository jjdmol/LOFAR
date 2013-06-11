//
//  tEventBinding.cc: Test program to test binding of events
//
//  Copyright (C) 2013
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
//  $Id: tEventBinding.cc 15471 2010-04-19 09:03:48Z overeem $

#include <lofar_config.h>
#include <Common/LofarLogger.h>
#include <GCF/TM/GCF_Scheduler.h>
#include "Echo_Protocol.ph"
#include "tEventBinding.h"
#include "tServer.h"

namespace LOFAR {
 namespace GCF {
  namespace TM {

static int	gCounter1;
static int	gCounter2;
static int	gCounter3;
static bool	gRoutingActive;

// Constructors of both classes
tEventBinding::tEventBinding(const string& name) : 
	GCFTask         ((State)&tEventBinding::sanityCheck, name),
	itsTimerPort    (0),
	itsDataPort		(0)
{ 
	// create timerport for guarding our tests
	itsTimerPort = new GCFTimerPort(*this, "timerPort");
	ASSERTSTR(itsTimerPort, "Failed to open timerport");

	registerProtocol (ECHO_PROTOCOL, ECHO_PROTOCOL_STRINGS);
}

tEventBinding::~tEventBinding()
{
	delete itsTimerPort;
}

void tEventBinding::handleEchoEvent(GCFEvent& event, GCFPortInterface& /*port*/) 
{
	EchoEchoEvent	echo(event);
	cout << "### ECHO event received in handleEchoEvent: " << echo << endl;
	gCounter1++;
}

void tEventBinding::handleDiscoEvent(GCFEvent& /*event*/, GCFPortInterface& port) 
{
	cout << "### DISCO event received in handleDiscoEvent:" << endl;
	port.close();
	gCounter1++;
}

//
// sanityCheck
//
// Test if everything works without bindings.
//
GCFEvent::TResult tEventBinding::sanityCheck(GCFEvent& event, GCFPortInterface& port)
{
	GCFEvent::TResult status = GCFEvent::HANDLED;
	LOG_DEBUG_STR ("sanityCheck: " << eventName(event.signal) << "@" << port.getName() << " : " << &port);

	switch (event.signal) {
	case F_ENTRY:
		cout << "Testing is everything works without binding events..." << endl;
		itsTimerPort->setTimer(5.0);
		itsDataPort = new GCFTCPPort(*this, "Echo:Server", GCFPortInterface::SAP, ECHO_PROTOCOL, false);
		ASSERTSTR(itsDataPort, "Unable to allocate IO port");
		itsDataPort->autoOpen(4,0,1); 		//nrRetry, timeout, retryItv
		break;

	case F_CONNECTED: {
		// looks good, we are connected
		EchoPingEvent	ping;
		ping.seqnr = 100;
		itsDataPort->send(ping);
	} break;
		
	case ECHO_ECHO: {
		EchoEchoEvent	echo(event);
		ASSERTSTR(echo.seqnr == 100, "Wrong echo returned");
		cout << "==> SanityCheck OK!" << endl;
		TRAN(tEventBinding::bindFixedOnSignal);
		break;
	} break;

	case F_TIMER: {
		cout << "EMERGENCY TIMER EXPIRED!" << endl;
		GCFScheduler::instance()->stop();
	} break;

	case F_EXIT:
		itsTimerPort->cancelAllTimers();
		break;

	default:
		LOG_WARN_STR("DEFAULT in timerTest: " << eventName(event));
		status = GCFEvent::NOT_HANDLED;
		break;
	}

	return status;
}

//
// bindFixedOnSignal
//
// Test is binding on signals work.
//
GCFEvent::TResult tEventBinding::bindFixedOnSignal(GCFEvent& event, GCFPortInterface& port)
{
	GCFEvent::TResult status = GCFEvent::HANDLED;
	LOG_DEBUG_STR ("bindFixedOnSignal: " << eventName(event.signal) << "@" << port.getName() << " : " << &port);

	switch (event.signal) {
	case F_ENTRY: {
		cout << "Test if we can bind an event fixed on a signal..." << endl;
		GCFScheduler::instance()->bindEvent(ECHO_ECHO, (GCFPortInterface*) 0, this, (GCFFunction)&tEventBinding::handleEchoEvent, true);
		gRoutingActive = true;
		EchoPingEvent	ping;
		ping.seqnr = 201;
		itsDataPort->send(ping);
		gCounter1 = 0;
		itsTimerPort->setTimer(1.0);
	} break;
		
	case ECHO_ECHO: {
		ASSERTSTR(gRoutingActive == false, "FIXED EVENT BINDING ON SIGNAL DOES NOT WORK");
		cout << "Fixed binding on signal works." << endl;
		TRAN(tEventBinding::bindFixedOnPort);
	} break;

	case F_TIMER: {
		ASSERTSTR((gRoutingActive && gCounter1 == 1) || (!gRoutingActive && gCounter1 == 0), "FIXED EVENT BINDING ON SIGNAL DOESN'T WORK");
		if (gRoutingActive) {
			GCFScheduler::instance()->deleteBinding(ECHO_ECHO, (GCFPortInterface*) 0, this);
			gRoutingActive = false;
			EchoPingEvent	ping;
			ping.seqnr = 202;
			itsDataPort->send(ping);
			gCounter1 = 0;
			itsTimerPort->setTimer(1.0);
		}
		else {
			cout << "DELETING A FIXED BINDING ON SIGNAL DOES NOT WORK." << endl;
			GCFScheduler::instance()->stop();
		}
	} break;

	case F_EXIT:
		itsTimerPort->cancelAllTimers();
		LOG_INFO("F_EXIT event in bindFixedOnSignal");
		break;

	default:
		LOG_WARN_STR("DEFAULT in bindFixedOnSignal: " << eventName(event));
		status = GCFEvent::NOT_HANDLED;
		break;
	}

	return status;
}

//
// bindFixedOnPort
//
// Test is binding on signals work.
//
GCFEvent::TResult tEventBinding::bindFixedOnPort(GCFEvent& event, GCFPortInterface& port)
{
	GCFEvent::TResult status = GCFEvent::HANDLED;
	LOG_DEBUG_STR ("bindFixedOnPort: " << eventName(event.signal) << "@" << port.getName() << " : " << &port);

	switch (event.signal) {
	case F_ENTRY: {
		cout << "Test is fixed binding on port works..." << endl;
		GCFScheduler::instance()->bindEvent(0, itsDataPort, this, (GCFFunction)&tEventBinding::handleEchoEvent, true);
		gRoutingActive = true;
		EchoPingEvent	ping;
		ping.seqnr = 301;
		itsDataPort->send(ping);
		gCounter1 = 0;
		itsTimerPort->setTimer(1.0);
	} break;
		
	case ECHO_ECHO: {
		ASSERTSTR(gRoutingActive == false, "FIXED EVENT BINDING ON PORT DOES NOT WORK");
		cout << "Fixed binding on port works." << endl;
		TRAN(tEventBinding::bindFixedOnBoth);
	} break;

	case F_TIMER: {
		ASSERTSTR((gRoutingActive && gCounter1 == 1) || (!gRoutingActive && gCounter1 == 0), "FIXED EVENT BINDING ON PORT DOESN'T WORK");
		if (gRoutingActive) {
			GCFScheduler::instance()->deleteBinding(0, itsDataPort, this);
			gRoutingActive = false;
			EchoPingEvent	ping;
			ping.seqnr = 302;
			itsDataPort->send(ping);
			gCounter1 = 0;
			itsTimerPort->setTimer(1.0);
		}
		else {
			cout << "DELETING A FIXED BINDING ON PORT DOES NOT WORK." << endl;
			cout << "gRoutineActive=" << (gRoutingActive ? "Yes" : "No") << ", gCounter1=" << gCounter1 << endl;
			GCFScheduler::instance()->stop();
		}
	} break;

	case F_EXIT:
		itsTimerPort->cancelAllTimers();
		LOG_INFO("F_EXIT event in bindFixedOnPort");
		break;

	default:
		LOG_WARN_STR("DEFAULT in bindFixedOnPort: " << eventName(event));
		status = GCFEvent::NOT_HANDLED;
		break;
	}

	return status;
}
//
// bindFixedOnBoth
//
// Test is binding on signals work.
//
GCFEvent::TResult tEventBinding::bindFixedOnBoth(GCFEvent& event, GCFPortInterface& port)
{
	GCFEvent::TResult status = GCFEvent::HANDLED;
	LOG_DEBUG_STR ("bindFixedOnBoth: " << eventName(event.signal) << "@" << port.getName() << " : " << &port);

	switch (event.signal) {
	case F_ENTRY: {
		cout << "Testing if fixed binding on signal and port works..." << endl;
		GCFScheduler::instance()->bindEvent(ECHO_ECHO, itsDataPort, this, (GCFFunction)&tEventBinding::handleEchoEvent, true);
		gRoutingActive = true;
		EchoPingEvent	ping;
		ping.seqnr = 401;
		itsDataPort->send(ping);
		gCounter1 = 0;
		itsTimerPort->setTimer(1.0);
	} break;
		
	case ECHO_ECHO: {
		ASSERTSTR(gRoutingActive == false, "FIXED EVENT BINDING ON PORT AND SIGNAL DOES NOT WORK");
		cout << "Fixed binding on signal and port works." << endl;
		TRAN(tEventBinding::bindDefaultOnSignal);
	} break;

	case F_TIMER: {
		ASSERTSTR((gRoutingActive && gCounter1 == 1) || (!gRoutingActive && gCounter1 == 0), "FIXED EVENT BINDING ON PORT AND SIGNAL DOESN'T WORK");
		if (gRoutingActive) {
			GCFScheduler::instance()->deleteBinding( ECHO_ECHO, itsDataPort, this);
			gRoutingActive = false;
			EchoPingEvent	ping;
			ping.seqnr = 302;
			itsDataPort->send(ping);
			gCounter1 = 0;
			itsTimerPort->setTimer(1.0);
		}
		else {
			cout << "DELETING A FIXED BINDING ON PORT AND SIGNAL DOES NOT WORK." << endl;
			cout << "gRoutineActive=" << (gRoutingActive ? "Yes" : "No") << ", gCounter1=" << gCounter1 << endl;
			GCFScheduler::instance()->stop();
		}
	} break;

	case F_EXIT:
		itsTimerPort->cancelAllTimers();
		LOG_INFO("F_EXIT event in bindFixedOnPort");
		break;

	default:
		LOG_WARN_STR("DEFAULT in bindFixedOnPort: " << eventName(event));
		status = GCFEvent::NOT_HANDLED;
		break;
	}

	return status;
}

//
// bindDefaultOnSignal
//
// Test is binding on signals work.
//
GCFEvent::TResult tEventBinding::bindDefaultOnSignal(GCFEvent& event, GCFPortInterface& port)
{
	GCFEvent::TResult status = GCFEvent::HANDLED;
	LOG_DEBUG_STR ("bindDefaultOnSignal: " << eventName(event.signal) << "@" << port.getName() << " : " << &port);

	switch (event.signal) {
	case F_ENTRY: {
		cout << "Test if default binding on signal works..." << endl;
		GCFScheduler::instance()->bindEvent(ECHO_ECHO, (GCFPortInterface*) 0, this, (GCFFunction)&tEventBinding::handleEchoEvent, false);
		EchoPingEvent	ping;
		ping.seqnr = 401;
		itsDataPort->send(ping);
		gCounter1 = 0;
		itsTimerPort->setTimer(2.0);
	} break;
		
	case ECHO_ECHO: {
		EchoEchoEvent	echo(event);
		ASSERTSTR(echo.seqnr == 401 && gCounter1 == 0 || echo.seqnr == 402 && gCounter1 == 1, "DEFAULT BINDING ON SIGNAL DOES NOT WORK.");
		switch (echo.seqnr) {
		case 401: {
			EchoPingEvent	ping;
			ping.seqnr = 402;
			itsDataPort->send(ping);
			return (GCFEvent::NOT_HANDLED);		// reroute 401 to default handling --> gCounter1++
		} break;
		case 402:
			return (GCFEvent::HANDLED);
		}
	} break;

	case F_TIMER: {
		ASSERTSTR(gCounter1 == 1, "DEFAULT EVENT BINDING ON SIGNAL DOESN'T WORK");
		GCFScheduler::instance()->deleteBinding(ECHO_ECHO, (GCFPortInterface*) 0, this);
		cout << "Default binding on signal works." << endl;
		TRAN(tEventBinding::bindPortEvent);
	} break;

	case F_EXIT:
		itsTimerPort->cancelAllTimers();
		LOG_INFO("F_EXIT event in bindDefaultOnSignal");
		break;

	default:
		LOG_WARN_STR("DEFAULT in bindDefaultOnSignal: " << eventName(event));
		status = GCFEvent::NOT_HANDLED;
		break;
	}

	return status;
}

//
// bindPortEvent
//
// Test is binding on signals work.
//
GCFEvent::TResult tEventBinding::bindPortEvent(GCFEvent& event, GCFPortInterface& port)
{
	GCFEvent::TResult status = GCFEvent::HANDLED;
	LOG_DEBUG_STR ("bindPortEvent: " << eventName(event.signal) << "@" << port.getName() << " : " << &port);

	switch (event.signal) {
	case F_ENTRY: {
		cout << "Testing if binding port-events works..." << endl;
		GCFScheduler::instance()->bindEvent(F_DISCONNECTED, itsDataPort, this, (GCFFunction)&tEventBinding::handleDiscoEvent, true);
		EchoPingEvent	ping;
		ping.seqnr = 192837;		// use magic 'disconnect' seqnr
		itsDataPort->send(ping);
		gCounter1 = 0;
		itsTimerPort->setTimer(1.0);
	} break;
		
	case F_DISCONNECTED:
		ASSERTSTR(false, "DISCONNECT EVENT WAS NOT REROUTED");
		break;

	case F_TIMER: {
		ASSERTSTR(gCounter1 == 1, "BINDING A PORT EVENT DOESN'T WORK");
		GCFScheduler::instance()->deleteBinding(F_DISCONNECTED, itsDataPort, this);
		cout << "ALL SINGLE TESTS OK" << endl;
		itsTimerPort->cancelAllTimers();
		GCFScheduler::instance()->stop();
	} break;

	case ECHO_ECHO: {
		EchoEchoEvent	echo(event);
		LOG_INFO("Received ECHO as expected.");
	} break;

	case F_EXIT:
		itsTimerPort->cancelAllTimers();
		LOG_INFO("F_EXIT event in bindPortEvent");
		break;

	default:
		LOG_WARN_STR("DEFAULT in bindPortEvent: " << eventName(event));
		status = GCFEvent::NOT_HANDLED;
		break;
	}

	return status;
}

// Constructors of both classes
tMultiTask::tMultiTask(const string& name, uint taskNr) : 
	GCFTask         ((State)&tMultiTask::connect, name),
	itsTimerPort    (0),
	itsDataPort		(0),
	itsTaskNr		(taskNr)
{ 
	// create timerport for guarding our tests
	itsTimerPort = new GCFTimerPort(*this, "timerPort");
	ASSERTSTR(itsTimerPort, "Failed to open timerport");

	registerProtocol (ECHO_PROTOCOL, ECHO_PROTOCOL_STRINGS);
}

tMultiTask::~tMultiTask()
{
	delete itsTimerPort;
}

void tMultiTask::handleEchoEvent1(GCFEvent& event, GCFPortInterface& /*port*/) 
{
	EchoEchoEvent	echo(event);
	cout << "### ECHO event received in handleEchoEvent: " << echo << endl;
	gCounter1++;
}

void tMultiTask::handleEchoEvent2(GCFEvent& event, GCFPortInterface& /*port*/) 
{
	EchoEchoEvent	echo(event);
	cout << "### ECHO event received in handleEchoEvent: " << echo << endl;
	gCounter2++;
}

//
// connect
//
GCFEvent::TResult tMultiTask::connect(GCFEvent& event, GCFPortInterface& port)
{
	GCFEvent::TResult status = GCFEvent::HANDLED;
	LOG_DEBUG_STR (getName() << "-connect: " << eventName(event.signal) << "@" << port.getName() << " : " << &port);

	switch (event.signal) {
	case F_ENTRY:
		cout << getName() << ": Testing if everything works without binding events..." << endl;
		itsTimerPort->setTimer(5.0);
		itsDataPort = new GCFTCPPort(*this, "Echo:Server", GCFPortInterface::SAP, ECHO_PROTOCOL, false);
		ASSERTSTR(itsDataPort, "Unable to allocate IO port");
		itsDataPort->autoOpen(4,0,1); 		//nrRetry, timeout, retryItv
		break;

	case F_CONNECTED: {
		// looks good, we are connected
		EchoPingEvent	ping;
		ping.seqnr = 100 + itsTaskNr;
		itsDataPort->send(ping);
	} break;
		
	case ECHO_ECHO: {
		EchoEchoEvent	echo(event);
		ASSERTSTR(echo.seqnr == 100 + itsTaskNr, "Wrong echo returned");
		cout << "==> connect OK!" << endl;
		TRAN(tMultiTask::doTest);
		break;
	} break;

	case F_TIMER: {
		cout << "EMERGENCY TIMER EXPIRED!" << endl;
		GCFScheduler::instance()->stop();
	} break;

	case F_EXIT:
		itsTimerPort->cancelAllTimers();
		break;

	default:
		LOG_WARN_STR("DEFAULT in timerTest: " << eventName(event));
		status = GCFEvent::NOT_HANDLED;
		break;
	}

	return status;
}

//
// doTest
//
// Test is binding on signals work.
//
GCFEvent::TResult tMultiTask::doTest(GCFEvent& event, GCFPortInterface& port)
{
	GCFEvent::TResult status = GCFEvent::HANDLED;
	LOG_DEBUG_STR (getName() << "-doTest: " << eventName(event.signal) << "@" << port.getName() << " : " << &port);

	switch (event.signal) {
	case F_ENTRY: {
		switch(itsTaskNr) {
		case 1:
			cout << getName() << "-Binding Echo Event..." << endl;
			GCFScheduler::instance()->bindEvent(ECHO_ECHO, itsDataPort, this, (GCFFunction)&tMultiTask::handleEchoEvent1, true);
			itsTimerPort->setTimer(0.1,0.1);	// for sending multiple messages
			break;
		case 2:
			cout << getName() << "-Binding Echo Event..." << endl;
			GCFScheduler::instance()->bindEvent(ECHO_ECHO, itsDataPort, this, (GCFFunction)&tMultiTask::handleEchoEvent2, true);
			itsTimerPort->setTimer(0.1,0.1);	// for sending multiple messages
			break;
		break;
		case 3: {
			cout << getName() << "-Not binding the Echo event..." << endl;
			EchoPingEvent	ping;
			ping.seqnr = 500 + itsTaskNr;
			itsDataPort->send(ping);
			itsTimerPort->setTimer(3.0);	// to end of test
		} break;
		}

		switch (itsTaskNr) {
		case 1:	gCounter1 = 0;	break;
		case 2:	gCounter2 = 0;	break;
		case 3:	gCounter3 = 0;	break;
		}

	} break;
		
	case ECHO_ECHO: {
		ASSERTSTR(itsTaskNr == 3, "BINDING SIMULTANEOUS TASKS DOES NOT WORK");
		gCounter3++;
		if (gCounter3 < 10) {
			EchoPingEvent	ping;
			ping.seqnr = 500 + 10*gCounter3 + itsTaskNr;
			itsDataPort->send(ping);
		}
	} break;

	case F_TIMER: {
		switch (itsTaskNr) {
		case 1: {
			if (gCounter1 < 10) {
				EchoPingEvent	ping;
				ping.seqnr = 500 + 10*gCounter1 + itsTaskNr;
				itsDataPort->send(ping);
				LOG_DEBUG_STR("SENDING SEQNR: " << ping.seqnr);
			}
			else {
				itsTimerPort->cancelAllTimers();
			}
		} break;
		case 2: {
			if (gCounter2 < 10) {
				EchoPingEvent	ping;
				ping.seqnr = 500 + 10*gCounter2 + itsTaskNr;
				itsDataPort->send(ping);
			}
			else {
				itsTimerPort->cancelAllTimers();
			}
		} break;
		case 3: {
			ASSERTSTR(gCounter1 == 10, "Expected 10 echo's for task 1");
			ASSERTSTR(gCounter2 == 10, "Expected 10 echo's for task 2");
			ASSERTSTR(gCounter3 == 10, "Expected 10 echo's for task 3");
			cout << "ALL SIMULTANEOUS TESTS PASSED" << endl;
			GCFScheduler::instance()->stop();
		} break;
		}
	} break;

	case F_EXIT:
		itsTimerPort->cancelAllTimers();
		LOG_INFO("F_EXIT event in bindFixedOnPort");
		break;

	default:
		LOG_WARN_STR("DEFAULT in bindFixedOnPort: " << eventName(event));
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

	tEventBinding	testTask("testTask");
	testTask.start(); // make initial transition

	// start server task
	tServer			server("Server", 0);
	server.start();

	theScheduler->setDelayedQuit(true);
	theScheduler->run();

	// setup next test type with 3 tasks.
	tMultiTask		mt1("Task 1", 1);
	tMultiTask		mt2("Task 2", 2);
	tMultiTask		mt3("Task 3", 3);
	mt1.start();
	mt2.start();
	mt3.start();

	theScheduler->run();
	return (0);
}
