//
//  tRTDBPort.cc: Test program to test the majority of the RTDBPort class.
//
//  Copyright (C) 2007
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
//  $Id: tRTDBPort.cc 13145 2009-04-22 08:47:05Z overeem $

#include <lofar_config.h>
#include <Common/LofarLogger.h>
#include <Common/Exception.h>
#include <MACIO/KVT_Protocol.ph>	// just as a test protocol, could have been any protocol
#include <GCF/PVSS/GCF_PVTypes.h>
#include <GCF/PVSS/PVSSinfo.h>
#include <GCF/PVSS/PVSSservice.h>
#include "tRTDBPort.h"

namespace LOFAR {
  namespace GCF {
  using namespace TM;
  using namespace PVSS;
  namespace RTDB {

//
// constructor
//
tRTDBPort::tRTDBPort(const string& name) : 
	GCFTask((State)&tRTDBPort::openPort, name), 
	itsRTDBPort(0),
	itsTimerPort(0)
{
	LOG_DEBUG_STR("tRTDBPort(" << name << ")");

	itsRTDBPort	 = new GCFRTDBPort(*this, "RTDBPort", GCFPortInterface::SAP, 0, "DP_from_ruud");
	ASSERTSTR(itsRTDBPort, "Can't allocate RTDBPort");

	itsTimerPort = new GCFTimerPort(*this, "timerPort");
	ASSERTSTR(itsTimerPort, "Can't allocate GCFTimerPort");

	registerProtocol(KVT_PROTOCOL, KVT_PROTOCOL_STRINGS);

}

//
// destructor
//
tRTDBPort::~tRTDBPort()
{
	LOG_DEBUG("Deleting tRTDBPort");
	if (itsRTDBPort) {
		delete itsRTDBPort;
	}
	if (itsTimerPort) {
		delete itsTimerPort;
	}
}

//
// openPort (event, port)
//
GCFEvent::TResult tRTDBPort::openPort(GCFEvent& e, GCFPortInterface& /*p*/)
{
	LOG_DEBUG_STR("openPort:" << eventName(e));

	switch (e.signal) {
	case F_INIT: 
		break;

	case F_ENTRY: {
		if (!itsRTDBPort->open()) {
			LOG_FATAL("Calling open failed!");
			TRAN (tRTDBPort::final);
			return (GCFEvent::HANDLED);
		}
		// wait for F_CONNECT or F_DISCONNECT
		itsTimerPort->setTimer(5.0);
	}
	break;

	case F_TIMER:
		LOG_FATAL("'open' of RTDBPort did not result in a F_(DIS)CONNECT");
		TRAN(tRTDBPort::final);
		break;

	case F_CONNECTED:
		LOG_INFO("Calling 'open' was successful, continue with write test");
		TRAN(tRTDBPort::writeTest);
	break;

	case F_DISCONNECTED:
		LOG_FATAL("'open' of RTDBPort resulted in a F_DISCONNECT");
		TRAN(tRTDBPort::final);
		break;

	case F_EXIT:
		itsTimerPort->cancelAllTimers();
		break;

	default:
		return(GCFEvent::NOT_HANDLED);
	}

	return (GCFEvent::HANDLED);
}

//
// final (event, port)
//
GCFEvent::TResult tRTDBPort::final(GCFEvent& e, GCFPortInterface& /*p*/)
{
	LOG_DEBUG_STR("final:" << eventName(e));

	switch (e.signal) {
	case F_ENTRY:
		itsTimerPort->setTimer(1.0);
		break;
	
	case F_TIMER:
		GCFScheduler::instance()->stop();
		break;

	default:
		return(GCFEvent::NOT_HANDLED);
	}

	return (GCFEvent::HANDLED);
}

//
// writeTest (event, port)
//
GCFEvent::TResult tRTDBPort::writeTest(GCFEvent& e, GCFPortInterface& /*p*/)
{
	LOG_DEBUG_STR("writeTest:" << eventName(e));

	switch (e.signal) {
	case F_ENTRY: {
		KVTRegisterEvent	request;
		request.obsID = 25002;
		request.name  = "This is a test string to test the RTDBPort interface";
		ssize_t		btsSent = itsRTDBPort->send(request);
		if (btsSent <= 0) {
			LOG_FATAL_STR("Sending a message resulted in " << btsSent << " bytes being send");
			TRAN(tRTDBPort::final);
			break;
		}
		// assume the event is in the database
		LOG_INFO_STR(btsSent << " bytes were stored in the database");
		TRAN(tRTDBPort::readTest);
	}
	break;

	default:
		return(GCFEvent::NOT_HANDLED);
	}

	return (GCFEvent::HANDLED);
}

//
// readTest (event, port)
//
GCFEvent::TResult tRTDBPort::readTest(GCFEvent& e, GCFPortInterface& /*p*/)
{
	LOG_DEBUG_STR("readTest:" << eventName(e));

	switch (e.signal) {
	case F_ENTRY:
		itsTimerPort->setTimer(5.0);
	break;

	case F_TIMER:
		LOG_FATAL("Readtest FAILED");
		TRAN(tRTDBPort::final);
	break;

	case F_EXIT:
		itsRTDBPort->cancelAllTimers();
		break;

	default:
		return(GCFEvent::NOT_HANDLED);
		break;
	}

	return (GCFEvent::HANDLED);
}

//
// closeTest (event, port)
//
GCFEvent::TResult tRTDBPort::closeTest(GCFEvent& e, GCFPortInterface& /*p*/)
{
	LOG_DEBUG_STR("closeTest:" << eventName(e));

	switch (e.signal) {
	case F_ENTRY: {
		itsRTDBPort->close();
		itsTimerPort->setTimer(2.0);
	}
	break;

	case F_TIMER:
		LOG_FATAL("closeTest FAILED");
		TRAN(tRTDBPort::final); 
	break;

	case F_DISCONNECTED:
		LOG_INFO("closeTest was also successful. ALL TEST PASSED!");
		TRAN(tRTDBPort::final);
		break;

	default:
		return(GCFEvent::NOT_HANDLED);
	}

	return (GCFEvent::HANDLED);
}

  } // namespace RTDB
 } // namespace GCF
} // namespace LOFAR


using namespace LOFAR::GCF;

int main(int argc, char* argv[])
{
	TM::GCFScheduler::instance()->init(argc, argv);

	RTDB::tRTDBPort test_task("RTDBPortTest");  
	test_task.start(); // make initial transition

	TM::GCFScheduler::instance()->run();

	return 0;
}
