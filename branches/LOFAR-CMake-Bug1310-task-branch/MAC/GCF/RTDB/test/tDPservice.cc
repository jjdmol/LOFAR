//
//  tDPservice.cc: Test program to test the majority of the DPservice class.
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
//  $Id$

#include <lofar_config.h>
#include <Common/LofarLogger.h>
#include <Common/Exception.h>
#include <GCF/PVSS/GCF_PVTypes.h>
#include <GCF/PVSS/PVSSinfo.h>
#include <GCF/PVSS/PVSSservice.h>
#include <GCF/RTDB/DPservice.h>
#include <GCF/RTDB/DP_Protocol.ph>
#include "tDPservice.h"
#include "DPresponse.h"

namespace LOFAR {
  namespace GCF {
  using namespace TM;
  using namespace PVSS;
  namespace RTDB {

static bool	gTestPassed;

//
// constructor
//
tDPservice::tDPservice(const string& name) : 
	GCFTask((State)&tDPservice::createDPS, name), 
	itsPVSSservice(0),
	itsDPservice(0),
	itsTimerPort(0)
{
	LOG_DEBUG_STR("tDPservice(" << name << ")");

	registerProtocol(F_FSM_PROTOCOL, F_FSM_PROTOCOL_STRINGS);
	registerProtocol(DP_PROTOCOL, DP_PROTOCOL_STRINGS);

	itsTimerPort = new GCFTimerPort(*this, "timerPort");
	ASSERTSTR(itsTimerPort, "Can't allocate GCFTimerPort");

	itsDPservice	 = new DPservice(this);
	ASSERTSTR(itsDPservice, "Can't allocate DPservice");

	itsPVSSservice	 = new PVSSservice(new DPresponse);
	ASSERTSTR(itsDPservice, "Can't allocate DPservice");
}

//
// destructor
//
tDPservice::~tDPservice()
{
	LOG_DEBUG("Deleting tDPservice");
	if (itsDPservice) {
		delete itsDPservice;
	}
	if (itsTimerPort) {
		delete itsTimerPort;
	}
}

//
// createDPS (event, port)
//
GCFEvent::TResult tDPservice::createDPS(GCFEvent& e, GCFPortInterface& /*p*/)
{
	LOG_DEBUG_STR("createDPS:" << eventName(e));
	GCFEvent::TResult status = GCFEvent::HANDLED;

	switch (e.signal) {
	case F_INIT: 
		break;

	case F_ENTRY: {
		gTestPassed = false;
		gCreateCounter = 2;
		itsPVSSservice->dpCreate("test_int", "TestInt");	// NOTE: direct with PVSSservice!
		itsPVSSservice->dpCreate("test_PS",  "TestPS");
		itsTimerPort->setTimer(3.0); // max time for this test.
	}
	break;

	case F_TIMER:
		if (gCreateCounter == 0) {
			LOG_DEBUG("Allocation of DPservice was successful");
		}
		else {
			ASSERTSTR(false, "Not all DP were created.");
		}
		TRAN(tDPservice::WriteTest);
	break;

	default:
		status = GCFEvent::NOT_HANDLED;
		break;
	}

	return status;
}

//
// final (event, port)
//
GCFEvent::TResult tDPservice::final(GCFEvent& e, GCFPortInterface& /*p*/)
{
	LOG_DEBUG_STR("final:" << eventName(e));
	GCFEvent::TResult status = GCFEvent::HANDLED;

	switch (e.signal) {
	case F_ENTRY:
		itsTimerPort->setTimer(1.0);
		break;
	
	case F_TIMER:
		stop();
		break;

	default:
		status = GCFEvent::NOT_HANDLED;
		break;
	}

	return status;
}

//
// WriteTest (event, port)
//
GCFEvent::TResult tDPservice::WriteTest(GCFEvent& e, GCFPortInterface& /*p*/)
{
	LOG_DEBUG_STR("WriteTest:" << eventName(e));
	GCFEvent::TResult status = GCFEvent::HANDLED;

	switch (e.signal) {
	case F_ENTRY: {
		gSetCounter = 2;
		itsDPservice->setValue("test_int.theValue", "3125", LPT_INTEGER);
		itsDPservice->setValue("test_PS.floatVal",   "-36.6125", LPT_DOUBLE);
		itsTimerPort->setTimer(1.0);
	}
	break;

	case F_TIMER:
		LOG_DEBUG_STR("Writetest " << ((gSetCounter == 0) ? "was successful" : "FAILED"));
		TRAN(tDPservice::ReadTest);
	break;

	default:
		status = GCFEvent::NOT_HANDLED;
		break;
	}

	return status;
}

//
// ReadTest (event, port)
//
GCFEvent::TResult tDPservice::ReadTest(GCFEvent& e, GCFPortInterface& /*p*/)
{
	LOG_DEBUG_STR("ReadTest:" << eventName(e));
	GCFEvent::TResult status = GCFEvent::HANDLED;

	switch (e.signal) {
	case F_ENTRY: {
		gGetCounter = 2;

		PVSSresult	result = itsDPservice->getValue("test_int.theValue");
		if (result != SA_NO_ERROR) {
			LOG_DEBUG_STR("getValue(test_int.theValue) returned errno: " << result);
		}
		result = itsDPservice->getValue("test_PS.floatVal");
		if (result != SA_NO_ERROR) {
			LOG_DEBUG_STR("getValue(test_PS.floatVal) returned errno: " << result);
		}

		itsTimerPort->setTimer(5.0);
	}
	break;

	case F_TIMER:
		LOG_DEBUG_STR("Readtest " << ((gGetCounter == 0) ? "was successful" : "FAILED"));
		TRAN(tDPservice::WriteTest2);
	break;

	default:
		status = GCFEvent::NOT_HANDLED;
		break;
	}

	return status;
}

//
// WriteTest2 (event, port)
//
GCFEvent::TResult tDPservice::WriteTest2(GCFEvent& e, GCFPortInterface& /*p*/)
{
	LOG_DEBUG_STR("WriteTest2:" << eventName(e));
	GCFEvent::TResult status = GCFEvent::HANDLED;

	switch (e.signal) {
	case F_ENTRY: {
		LOG_DEBUG_STR("Writing to test_int.theValue via a GCFValue object");
		gSetCounter = 1;
		GCFPVInteger	anIntVar;
		anIntVar.setValue(50394);
		itsDPservice->setValue("test_int.theValue", anIntVar);
		itsTimerPort->setTimer(2.0);
	}
	break;

	case F_TIMER:
		LOG_DEBUG_STR("WriteTest2 " << ((gSetCounter == 0) ? "was successful" : "FAILED"));
		TRAN(tDPservice::WriteErrorTest); 
	break;

	default:
		status = GCFEvent::NOT_HANDLED;
		break;
	}

	return status;
}

//
// WriteErrorTest (event, port)
//
GCFEvent::TResult tDPservice::WriteErrorTest(GCFEvent& e, GCFPortInterface& /*p*/)
{
	LOG_DEBUG_STR("WriteErrorTest:" << eventName(e));
	GCFEvent::TResult status = GCFEvent::HANDLED;

	switch (e.signal) {
	case F_ENTRY: {
		itsTimerPort->setTimer(2.0);
		try {
			if (itsDPservice->setValue("iserniet", "432", LPT_INTEGER) == SA_NO_ERROR) {
				LOG_ERROR("Setting value of 'iserniet' should have failed");
			}
		}
		catch (...) {
			LOG_DEBUG_STR("Caught exception");
			return (GCFEvent::HANDLED);
		}
	}
	break;

	case F_TIMER:
		TRAN(tDPservice::ReadErrorTest);
	break;

	default:
		status = GCFEvent::NOT_HANDLED;
		break;
	}

	return status;
}

//
// ReadErrorTest (event, port)
//
GCFEvent::TResult tDPservice::ReadErrorTest(GCFEvent& e, GCFPortInterface& /*p*/)
{
	LOG_DEBUG_STR("ReadErrorTest:" << eventName(e));
	GCFEvent::TResult status = GCFEvent::HANDLED;

	switch (e.signal) {
	case F_ENTRY: {
		itsTimerPort->setTimer(2.0);
		try {
			if (itsDPservice->getValue("iserniet") == SA_NO_ERROR) {
				LOG_ERROR("Getting value of 'iserniet' should have failed");
			}
		}
		catch (...) {
			LOG_DEBUG_STR("Caught exception");
			return (GCFEvent::HANDLED);
		}
	}
	break;

	case F_TIMER:
		TRAN(tDPservice::QueryTest);
	break;

	default:
		status = GCFEvent::NOT_HANDLED;
		break;
	}

	return status;
}

//
// QueryTest (event, port)
//
GCFEvent::TResult tDPservice::QueryTest(GCFEvent& e, GCFPortInterface& /*p*/)
{
	LOG_DEBUG_STR("QueryTest:" << eventName(e));
	GCFEvent::TResult status = GCFEvent::HANDLED;

	switch (e.signal) {
	case F_ENTRY: {
		LOG_DEBUG_STR("Taking a subscription on the state-field of all RCU's");
		gQryCounter = 1;
		itsDPservice->query("'LOFAR_PIC_*.state'", "_DPT=\"RCU\"");
		itsTimerPort->setTimer(2.0);
	}
	break;

	case F_TIMER:
		LOG_DEBUG_STR("QueryTest " << ((gQryCounter == 0) ? "was successful" : "FAILED"));
		TRAN(tDPservice::CancelQueryTest);
	break;

	default:
		status = GCFEvent::NOT_HANDLED;
		break;
	}

	return status;
}

//
// CancelQueryTest (event, port)
//
GCFEvent::TResult tDPservice::CancelQueryTest(GCFEvent& e, GCFPortInterface& /*p*/)
{
	LOG_DEBUG_STR("CancelQueryTest:" << eventName(e));
	GCFEvent::TResult status = GCFEvent::HANDLED;

	switch (e.signal) {
	case F_ENTRY: {
		LOG_DEBUG_STR("Stopping subscription " << gQueryID << " on the state-field of all RCU's");
		gQryCounter = 1;
		itsDPservice->cancelQuery(gQueryID);
		itsTimerPort->setTimer(2.0);
	}
	break;

	case F_TIMER:
		LOG_DEBUG_STR("CancelQueryTest " << ((gQryCounter == 0) ? "was successful" : "FAILED"));
		TRAN(tDPservice::final);
	break;

	default:
		status = GCFEvent::NOT_HANDLED;
		break;
	}

	return status;
}

  } // namespace RTDB
 } // namespace GCF
} // namespace LOFAR


using namespace LOFAR::GCF;

int main(int argc, char* argv[])
{
	TM::GCFTask::init(argc, argv);

	RTDB::tDPservice test_task("DPStest");  
	test_task.start(); // make initial transition

	TM::GCFTask::run();

	return 0;
}
