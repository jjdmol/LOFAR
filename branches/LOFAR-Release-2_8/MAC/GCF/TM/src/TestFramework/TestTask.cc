//#  TestTask.cc: ABC for testing GCF tasks more easier.
//#
//#  Copyright (C) 2011
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
//#  $Id: TestTask.cc 13113 2009-04-16 12:30:06Z overeem $
//#
#include <lofar_config.h>
#include <Common/LofarLogger.h>
#include <GCF/TM/TestTask.h>

namespace LOFAR {
  namespace GCF {
	namespace TM {

//
// TestTask(taskname)
//
TestTask::TestTask(State	initial, const string&	taskname) :
	GCFTask(initial, taskname),
	itsTestTimer(0)
{
	LOG_TRACE_OBJ ("TestTask construction");

	// need port for pause timers.
	itsTestTimer = new GCFTimerPort(*this, "TestTimer");
}


//
// ~TestTask()
//
TestTask::~TestTask()
{
	LOG_TRACE_OBJ_STR (getName() << " destruction");
}


//
// TestFrame(event, port)
//
// Wait for connection from ParentTask, followed by an Announcement event and send newParent message
//
GCFEvent::TResult TestTask::TestFrame(GCFEvent& event, GCFPortInterface& port)
{
	LOG_DEBUG_STR ("TestFrame:" << eventName(event) << "@" << port.getName());

	switch (event.signal) {
	case F_ENTRY:
		LOG_INFO("-------------------- Tests start here --------------------");
		startTesten();
		break;

	default:
		_checkTest(event, port);
	}

	return (GCFEvent::HANDLED);
}

//
// _checkTest(event, port)
//
// Called when a message/event was received
//
void TestTask::_checkTest(GCFEvent& event, GCFPortInterface& port)
{
	// action must be a TP_RECV action;
	testAction	action = itsActionQ.front();
	switch (action.type) {
	case TP_PAUSE:
		ASSERTSTR(event.signal == F_TIMER && &port == itsTestTimer,
				"Received event " << eventName(event) << "@" << port.getName() << 
				" while nothing may have happend during " << action.period << " seconds");
		itsTestTimer->cancelAllTimers();
		break;

	case TP_OBSERVE: 
		itsActionQ.pop();
		ASSERTSTR(!itsActionQ.empty(), "Programming error! ActionQ is empty after 'observe' command");
		if (event.signal == F_TIMER && &port == itsTestTimer) {
			itsTestTimer->cancelAllTimers();
			action = itsActionQ.front();
		}
		// !!! fall through !!!
		
	case TP_RECV:
		ASSERTSTR(&port==action.port && event.signal==action.signal, 
				"Received event " << eventName(event) << "@" << port.getName() << 
				", but expected to receive message " << eventName(action.signal) << 
				"@" << action.port->getName());
		LOG_INFO_STR("TEST:Received event " << eventName(event) << "@" << port.getName());
//		LOG_INFO_STR("TEST:Received event " << eventName(event) << "@" << port.getName() << ":" << event);
		break;

	default:
		ASSERTSTR(false, "Programming error! Action on queue is " << action.type << 
				", remaining queuesize = " << itsActionQ.size());
	}

	itsActionQ.pop();
	doTestSuite();
}

//
// doTestSuite()
void TestTask::doTestSuite()
{
	// do next (send)action(s) in Q
	while (!itsActionQ.empty()) {
		testAction	action = itsActionQ.front();
		switch (action.type) {
		case TP_MSG:
			LOG_INFO_STR("   ### " << action.message << " ###");
			itsActionQ.pop();
			break;

		case TP_SEND:
			LOG_INFO_STR("TEST:Sending event " << eventName(*(action.event)) << "@" << action.port->getName());
			action.port->send(*(action.event));
			delete	action.event;
			itsActionQ.pop();
			break;

		case TP_RECV:
			LOG_INFO_STR("TEST:Waiting for signal " << eventName(action.signal));
			return;		// let task under test do the rest

		case TP_PAUSE:		// must result in a F_TIMER
		case TP_OBSERVE:	// may result in a F_TIMER or the next action on the Q
			LOG_INFO_STR("TEST:Timer running for " << action.period << " seconds");
			itsTestTimer->setTimer(action.period);
			return;

		case TP_DONE:
			LOG_INFO("@@@ ALL TEST SUCCESFULL @@@");
			GCFScheduler::instance()->stop();
			break;
		}
	}

	// queue is empty, ready with this testSuite
	LOG_INFO_STR("TEST:Finished testSuite successful!");
	GCFScheduler::instance()->stop();
}

//
// addSendTest(event, port)
void TestTask::addSendTest(GCFEvent*	event, GCFPortInterface*	port)
{
	itsActionQ.push(testAction(event->clone(), port));
}

//
// addRecvTest(signal, port)
void TestTask::addRecvTest(int	signal, GCFPortInterface*	port)
{
	itsActionQ.push(testAction(port , signal));
}

//
// addPause(seconds)
void TestTask::addPause(double	period)
{
	itsActionQ.push(testAction(TP_PAUSE, period));
}

//
// newTest(title)
void TestTask::newTest(const string&	title)
{
	itsActionQ.push(testAction(title));
}

	}; // TM
  }; // GCF
}; // LOFAR

