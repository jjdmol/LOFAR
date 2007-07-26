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
#include <GCF/GCF_PVTypes.h>
#include <GCF/PVSS/PVSSinfo.h>
#include <GCF/RTDB/RTDB_PropertySet.h>
#include <GCF/RTDB/DP_Protocol.ph>
#include "tPropertySet.h"

namespace LOFAR {
  namespace GCF {
  using namespace Common;
  using namespace TM;
  using namespace PVSS;
  namespace RTDB {


tPropertySet::tPropertySet(const string& name) : 
	GCFTask((State)&tPropertySet::createPS, name), 
	itsPropSet  (0),
	itsTimerPort(0)
{
	LOG_DEBUG_STR("tPropertySet(" << name << ")");

	TM::registerProtocol(F_FSM_PROTOCOL, F_FSM_PROTOCOL_STRINGS);
	TM::registerProtocol(DP_PROTOCOL, DP_PROTOCOL_STRINGS);

	itsTimerPort = new GCFTimerPort(*this, "timerPort");
	ASSERTSTR(itsTimerPort, "Can't allocate GCFTimerPort");
}

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
		itsPropSet	 = new RTDBPropertySet("myPS", "TestPS", PS_AT_OWNED_TEMP, this);
		ASSERTSTR(itsPropSet, "Can't allocate PropertySet");
//		if (itsPropSet.created()) {
//			LOG_DEBUG("PropertySet is created, going to next state");
//			TRAN(tPropertySet::WriteTest);
//		}
//		else {
			LOG_DEBUG("Waiting for creation confirmation");
			itsTimerPort->setTimer(5.0);	// when things go wrong
//		}
	}
	break;

	case F_TIMER:
		LOG_DEBUG_STR("TIMEOUT ON CREATE");
		TRAN(tPropertySet::WriteTest);
	break;

	case DP_CREATED: {
		// NOTE: this function may be called DURING the construction of the PropertySet.
		DPCreatedEvent		dpEvent(e);
		LOG_DEBUG_STR("Result of creating " << dpEvent.DPname << " = " << dpEvent.result);
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
GCFEvent::TResult tPropertySet::WriteTest(GCFEvent& e, GCFPortInterface& /*p*/)
{
	LOG_DEBUG_STR("WriteTest:" << eventName(e));
	GCFEvent::TResult status = GCFEvent::HANDLED;

	switch (e.signal) {
	case F_ENTRY: {
		ASSERTSTR(itsPropSet, "Lost propertySet");
		itsPropSet->setValue("uintVal", "25");
//		if (itsPropSet->setValue("uintVal", "25") == SA_NO_ERROR) {
//			schedule_tran(tPropertySet::final);
//		}
//		else {
			itsTimerPort->setTimer(5.0);	// wait for DP_SET command.
//		}
	}
	break;

	case F_TIMER:
		LOG_DEBUG_STR("TIMEOUT ON WRITE");
		TRAN(tPropertySet::final);
	break;

	case DP_SET: {
		DPSetEvent		dpEvent(e);
		LOG_DEBUG_STR("Result of set " << dpEvent.DPname << " = " << dpEvent.result);
		itsTimerPort->cancelAllTimers();
		TRAN(tPropertySet::final);
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
	TM::GCFTask::init(argc, argv);

	RTDB::tPropertySet test_task("PStest");  
	test_task.start(); // make initial transition

	TM::GCFTask::run();

	return 0;
}
