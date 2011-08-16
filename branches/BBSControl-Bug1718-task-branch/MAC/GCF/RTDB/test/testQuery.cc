//
//  testQuery.cc: Test program to test the majority of the DPservice class.
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
#include "DP_Protocol.ph"
#include "testQuery.h"
#include "DPresponse.h"

static string DPclause;
static string whereClause;

namespace LOFAR {
  namespace GCF {
  using namespace TM;
  using namespace PVSS;
  namespace RTDB {


//
// constructor
//
testQuery::testQuery(const string& name) : 
	GCFTask((State)&testQuery::waitForChanges, name), 
	itsDPservice(0),
	itsTimerPort(0)
{
	LOG_DEBUG_STR("testQuery(" << name << ")");

	registerProtocol(F_FSM_PROTOCOL, F_FSM_PROTOCOL_STRINGS);
	registerProtocol(DP_PROTOCOL, DP_PROTOCOL_STRINGS);

	itsTimerPort = new GCFTimerPort(*this, "timerPort");
	ASSERTSTR(itsTimerPort, "Can't allocate GCFTimerPort");

	itsDPservice	 = new DPservice(this);
	ASSERTSTR(itsDPservice, "Can't allocate DPservice");
}

//
// destructor
//
testQuery::~testQuery()
{
	LOG_DEBUG("Deleting testQuery");
	if (itsDPservice) {
		delete itsDPservice;
	}
	if (itsTimerPort) {
		delete itsTimerPort;
	}
}

//
// final (event, port)
//
GCFEvent::TResult testQuery::final(GCFEvent& e, GCFPortInterface& /*p*/)
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
// QueryTest (event, port)
//
GCFEvent::TResult testQuery::waitForChanges(GCFEvent& event, GCFPortInterface& /*p*/)
{
	LOG_DEBUG_STR("waitForChanges:" << eventName(event));
	GCFEvent::TResult status = GCFEvent::HANDLED;

	switch (event.signal) {
	case F_INIT:
		break;

	case F_ENTRY: {
		LOG_DEBUG_STR("Taking a subscription: " << DPclause << " WHERE " << whereClause);
		PVSSresult	result = itsDPservice->query(DPclause, whereClause);
		if (result != SA_NO_ERROR) {
			LOG_ERROR ("Taking subscription failed. Bailing out.");
			GCFScheduler::instance()->stop();
		}
	}
	break;

	case DP_QUERY_SUBSCRIBED: {
		DPQuerySubscribedEvent  answer(event);
		if (answer.result != SA_NO_ERROR) {
			LOG_ERROR_STR ("Taking subscription on PVSS-states failed (" << answer.result  <<
								"). Bailing out.");
			GCFScheduler::instance()->stop();
			break;
		}
		itsQueryID = answer.QryID;
		LOG_INFO_STR("Subscription at PVSS successful(" << itsQueryID  << "), waiting for changes.");
	}
    break;

	case DP_QUERY_CHANGED: {
		// log the stuff.
		DPQueryChangedEvent		DPevent(event);
		if (DPevent.result != SA_NO_ERROR) {
			LOG_ERROR_STR("PVSS reported error " << DPevent.result << " for query " << itsQueryID);
			break;
		}

		int     nrDPs = ((GCFPVDynArr*)(DPevent.DPnames._pValue))->getValue().size();
		GCFPVDynArr*    DPnames  = (GCFPVDynArr*)(DPevent.DPnames._pValue);
		GCFPVDynArr*    DPvalues = (GCFPVDynArr*)(DPevent.DPvalues._pValue);
		GCFPVDynArr*    DPtimes  = (GCFPVDynArr*)(DPevent.DPtimes._pValue);
		for (int    idx = 0; idx < nrDPs; ++idx) {
			// show operator what we are doing
			string  nameStr(DPnames->getValue() [idx]->getValueAsString());
			string  valStr (DPvalues->getValue()[idx]->getValueAsString());
			string  timeStr(DPtimes->getValue() [idx]->getValueAsString());
			LOG_INFO_STR(nameStr << " = " << valStr << " @ " << timeStr);
		} // for
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
using namespace std;

int main(int argc, char* argv[])
{
	if (argc != 3) {
		cerr << "Syntax: testQuery DPs where-clause" << endl;
		exit(1);
	}
	DPclause    = argv[1];
	whereClause = argv[2];

	TM::GCFScheduler::instance()->init(argc, argv);

	RTDB::testQuery test_task("DPStest");  
	test_task.start(); // make initial transition

	TM::GCFScheduler::instance()->run();

	return 0;
}
