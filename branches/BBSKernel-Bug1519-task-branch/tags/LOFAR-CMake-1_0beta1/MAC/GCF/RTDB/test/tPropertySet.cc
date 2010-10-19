//
//  tPropertySet.cc: Test program to test the majority of RTDB propertySet class.
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
#include <GCF/RTDB/RTDB_PropertySet.h>
#include "DP_Protocol.ph"
#include "tPropertySet.h"

namespace LOFAR {
  namespace GCF {
  using namespace TM;
  using namespace PVSS;
  namespace RTDB {

static bool	gTestPassed;

//
// constructor
//
tPropertySet::tPropertySet(const string& name) : 
	GCFTask((State)&tPropertySet::createPS, name), 
	itsPropSet  (0),
	itsTimerPort(0)
{
	LOG_DEBUG_STR("tPropertySet(" << name << ")");

	registerProtocol(F_FSM_PROTOCOL, F_FSM_PROTOCOL_STRINGS);
	registerProtocol(DP_PROTOCOL, DP_PROTOCOL_STRINGS);

	itsTimerPort = new GCFTimerPort(*this, "timerPort");
	ASSERTSTR(itsTimerPort, "Can't allocate GCFTimerPort");
}

//
// destructor
//
tPropertySet::~tPropertySet()
{
	LOG_DEBUG("Deleting tPropertySet");
	if (itsPropSet) {
		delete itsPropSet;
	}
	if (itsTimerPort) {
		delete itsTimerPort;
	}
}

//
// createPS (event, port)
//
GCFEvent::TResult tPropertySet::createPS(GCFEvent& e, GCFPortInterface& /*p*/)
{
	LOG_DEBUG_STR("createPS:" << eventName(e));
	GCFEvent::TResult status = GCFEvent::HANDLED;

	switch (e.signal) {
	case F_INIT: 
		break;

	case F_ENTRY: {
		gTestPassed = false;
//		itsPropSet	 = new RTDBPropertySet("myPS", "TestPS", PSAT_RO | PSAT_TMP, this);
//		itsPropSet	 = new RTDBPropertySet("myPS", "TestPS", PSAT_WO | PSAT_TMP, this);
		itsPropSet	 = new RTDBPropertySet("myPS", "TestPS", PSAT_RW, this);
		ASSERTSTR(itsPropSet, "Can't allocate PropertySet");
		itsTimerPort->setTimer(1.0); // max time for this test.
	}
	break;

	case F_TIMER:
		LOG_DEBUG_STR("Creation of DP " << (gTestPassed ? "was successful" : "FAILED"));
		TRAN(tPropertySet::WriteTest);
	break;

	case DP_CREATED: {
		// NOTE: this function may be called DURING the construction of the PropertySet.
		DPCreatedEvent		dpEvent(e);
		LOG_DEBUG_STR("Result of creating " << dpEvent.DPname << " = " << dpEvent.result);
		gTestPassed = true;
	}
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
GCFEvent::TResult tPropertySet::final(GCFEvent& e, GCFPortInterface& /*p*/)
{
	LOG_DEBUG_STR("final:" << eventName(e));
	GCFEvent::TResult status = GCFEvent::HANDLED;

	switch (e.signal) {
	case F_ENTRY:
		itsTimerPort->setTimer(1.0);
		break;
	
	case F_TIMER:
		GCFScheduler::instance()->stop();
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
GCFEvent::TResult tPropertySet::WriteTest(GCFEvent& e, GCFPortInterface& /*p*/)
{
	LOG_DEBUG_STR("WriteTest:" << eventName(e));
	GCFEvent::TResult status = GCFEvent::HANDLED;
	static	int		nrWriteTests;

	switch (e.signal) {
	case F_ENTRY: {
		gTestPassed = false;
		itsPropSet->setValue("uintVal",   "25");
		itsPropSet->setValue("intVal",    "-36");
		itsPropSet->setValue("floatVal",  "3.1415926");
		itsPropSet->setValue("boolVal",   "true");
		itsPropSet->setValue("stringVal", "test Stringetje");
		nrWriteTests = 5;
		itsTimerPort->setTimer(1.0);
	}
	break;

	case F_TIMER:
		LOG_DEBUG_STR("Writetest " << (gTestPassed ? "was successful" : "FAILED"));
		TRAN(tPropertySet::ReadTest);
	break;

	case DP_SET: {
		DPSetEvent		dpEvent(e);
		LOG_DEBUG_STR("Result of setting " << dpEvent.DPname << " = " << dpEvent.result);
		if (--nrWriteTests == 0) {
			gTestPassed = true;
		}
	}
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
GCFEvent::TResult tPropertySet::ReadTest(GCFEvent& e, GCFPortInterface& /*p*/)
{
	LOG_DEBUG_STR("ReadTest:" << eventName(e));
	GCFEvent::TResult status = GCFEvent::HANDLED;

	switch (e.signal) {
	case F_ENTRY: {
		gTestPassed = true;

		{
			GCFPVUnsigned	theVar;
			PVSSresult	result = itsPropSet->getValue("uintVal", theVar);
			if (result != SA_NO_ERROR) {
				LOG_DEBUG_STR("getValue(uintVal) returned errno: " << result);
				gTestPassed = false;
			}
			else {
				LOG_DEBUG_STR ("Value of 'uintVal' element = " << theVar.getValue());
			}
		}

		{
			GCFPVInteger	theVar;
			PVSSresult	result = itsPropSet->getValue("intVal", theVar);
			if (result != SA_NO_ERROR) {
				LOG_DEBUG_STR("getValue(intVal) returned errno: " << result);
				gTestPassed = false;
			}
			else {
				LOG_DEBUG_STR ("Value of 'intVal' element = " << theVar.getValue());
			}
		}

		{
			GCFPVDouble	theVar;
			PVSSresult	result = itsPropSet->getValue("floatVal", theVar);
			if (result != SA_NO_ERROR) {
				LOG_DEBUG_STR("getValue(floatVal) returned errno: " << result);
				gTestPassed = false;
			}
			else {
				LOG_DEBUG_STR ("Value of 'floatVal' element = " << theVar.getValue());
			}
		}

		{
			GCFPVBool	theVar;
			PVSSresult	result = itsPropSet->getValue("boolVal", theVar);
			if (result != SA_NO_ERROR) {
				LOG_DEBUG_STR("getValue(boolVal) returned errno: " << result);
				gTestPassed = false;
			}
			else {
				LOG_DEBUG_STR ("Value of 'boolVal' element = " << theVar.getValue());
			}
		}

		{
			GCFPVString	theVar;
			PVSSresult	result = itsPropSet->getValue("stringVal", theVar);
			if (result != SA_NO_ERROR) {
				LOG_DEBUG_STR("getValue(stringVal) returned errno: " << result);
				gTestPassed = false;
			}
			else {
				LOG_DEBUG_STR ("Value of 'stringVal' element = " << theVar.getValue());
			}
		}

		itsTimerPort->setTimer(0.0);
	}
	break;

	case F_TIMER:
		LOG_DEBUG_STR("Readtest " << (gTestPassed ? "was successful" : "FAILED"));
		TRAN(tPropertySet::Level1Test);
	break;

	default:
		status = GCFEvent::NOT_HANDLED;
		break;
	}

	return status;
}


//
// Level1Test (event, port)
//
GCFEvent::TResult tPropertySet::Level1Test(GCFEvent& e, GCFPortInterface& /*p*/)
{
	LOG_DEBUG_STR("Level1Test:" << eventName(e));
	GCFEvent::TResult status = GCFEvent::HANDLED;

	switch (e.signal) {
	case F_ENTRY: {
		LOG_DEBUG_STR("Writing to myPS.level1.intVal");
		gTestPassed = false;
		// try it this time via a variable iso a valueString
		GCFPVInteger	anIntVar;
		anIntVar.setValue(9372);
		itsPropSet->setValue("level1.intVal", anIntVar);
		itsTimerPort->setTimer(2.0);
	}
	break;

	case F_TIMER:
		LOG_DEBUG_STR("Level1Test " << (gTestPassed ? "was successful" : "FAILED"));
		TRAN(tPropertySet::Level2Test);
	break;

	case DP_SET: {
		DPSetEvent		dpEvent(e);
		LOG_DEBUG_STR("Result of setting " << dpEvent.DPname << " = " << dpEvent.result);
		GCFPVInteger	resultVar;
		itsPropSet->getValue("level1.intVal", resultVar);
		ASSERTSTR (resultVar.getValue() == 9372, "Readback of variable returned value " <<
					resultVar.getValue() << " iso 9372");
		gTestPassed = true;
		itsTimerPort->cancelAllTimers();
		itsTimerPort->setTimer(0.0);
		
	}
	break;

	default:
		status = GCFEvent::NOT_HANDLED;
		break;
	}

	return status;
}

//
// Level2Test (event, port)
//
GCFEvent::TResult tPropertySet::Level2Test(GCFEvent& e, GCFPortInterface& /*p*/)
{
	LOG_DEBUG_STR("Level2Test:" << eventName(e));
	GCFEvent::TResult status = GCFEvent::HANDLED;

	switch (e.signal) {
	case F_ENTRY: {
		LOG_DEBUG_STR("Writing to myPS.level1.level2.stringVal");
		gTestPassed = false;
		itsPropSet->setValue("level1.level2.stringVal", "Level2 testje");
		itsTimerPort->setTimer(2.0);
	}
	break;

	case F_TIMER:
		LOG_DEBUG_STR("Level2Test " << (gTestPassed ? "was successful" : "FAILED"));
		TRAN(tPropertySet::WriteErrorTest);
	break;

	case DP_SET: {
		DPSetEvent		dpEvent(e);
		LOG_DEBUG_STR("Result of setting " << dpEvent.DPname << " = " << dpEvent.result);
		GCFPVString	resultVar;
		itsPropSet->getValue("level1.level2.stringVal", resultVar);
		ASSERTSTR (resultVar.getValue() == "Level2 testje", "Readback of variable returned value " <<
					resultVar.getValue() << " iso 'Level2 testje'");
		gTestPassed = true;
		itsTimerPort->cancelAllTimers();
		itsTimerPort->setTimer(0.0);
	}
	break;

	default:
		status = GCFEvent::NOT_HANDLED;
		break;
	}

	return status;
}

//
// WriteError:Test (event, port)
//
GCFEvent::TResult tPropertySet::WriteErrorTest(GCFEvent& e, GCFPortInterface& /*p*/)
{
	LOG_DEBUG_STR("WriteErrorTest:" << eventName(e));
	GCFEvent::TResult status = GCFEvent::HANDLED;

	switch (e.signal) {
	case F_ENTRY: {
		gTestPassed = true;
		itsTimerPort->setTimer(2.0);
		itsPropSet->setValue("uintVal", "-25");
		try {
			itsPropSet->setValue("iserniet", "-36");
		}
		catch (...) {
			LOG_DEBUG_STR("Caught exception");
			return (GCFEvent::HANDLED);
		}
		gTestPassed = false;
	}
	break;

	case F_TIMER:
		LOG_DEBUG_STR("WriteErrortest " << (gTestPassed ? "was successful" : "FAILED"));
		TRAN(tPropertySet::WriteDelayTest);
	break;

	case DP_SET: {
		DPSetEvent		dpEvent(e);
		LOG_DEBUG_STR("Result of setting " << dpEvent.DPname << " = " << dpEvent.result);
		gTestPassed = (dpEvent.result != SA_NO_ERROR);
	}
	break;

	default:
		status = GCFEvent::NOT_HANDLED;
		break;
	}

	return status;
}

//
// WriteDelayTest (event, port)
//
GCFEvent::TResult tPropertySet::WriteDelayTest(GCFEvent& e, GCFPortInterface& /*p*/)
{
	LOG_DEBUG_STR("WriteDelayTest:" << eventName(e));
	GCFEvent::TResult status = GCFEvent::HANDLED;
	static	int		nrWriteTests;

	switch (e.signal) {
	case F_ENTRY: {
		PVSSresult 	result1, result2;
		result1 = itsPropSet->setValue("uintVal",   "6903", 0.0, false);
		result2 = itsPropSet->setValue("stringVal", "Delayed write", 0.0, false);
		gTestPassed = (result1 == SA_NO_ERROR) && (result2 == SA_NO_ERROR);
		nrWriteTests = 2;
		itsTimerPort->setTimer(2.0);
	}
	break;

	case F_TIMER:
		LOG_DEBUG_STR("First part of WriteDelayTest " << (gTestPassed ? "was successful" : "FAILED"));
		if (gTestPassed) {
			LOG_DEBUG_STR("Calling flush");
			itsPropSet->flush();
		}
		itsTimerPort->setTimer(2.0);
		TRAN(tPropertySet::final);
	break;

	case DP_SET: {
		DPSetEvent		dpEvent(e);
		LOG_DEBUG_STR("Result of setting " << dpEvent.DPname << " = " << dpEvent.result);
		if (--nrWriteTests == 0) {
			gTestPassed = true;
		}
	}
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
	TM::GCFScheduler::instance()->init(argc, argv);

	RTDB::tPropertySet test_task("PStest");  
	test_task.start(); // make initial transition

	TM::GCFScheduler::instance()->run();

	return 0;
}
