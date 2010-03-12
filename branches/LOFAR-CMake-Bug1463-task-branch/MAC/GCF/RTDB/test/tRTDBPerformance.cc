//
//  tPerformance.cc: Test program to test the majority of the RTDBProperty class.
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
#include <Common/StringUtil.h>
#include <Common/Timer.h>
#include <GCF/PVSS/GCF_PVTypes.h>
#include <GCF/PVSS/PVSSinfo.h>
#include "DP_Protocol.ph"
#include "tRTDBPerformance.h"
#include "RTDBPerfResp.h"

namespace LOFAR {
  namespace GCF {
  using namespace TM;
  using namespace PVSS;
  namespace RTDB {


tPerformance::tPerformance(const string& name) : 
	GCFTask((State)&tPerformance::initial, name), 
	itsService(0),
	itsResponse(0),
	itsTimerPort(0)
{
	registerProtocol(F_FSM_PROTOCOL, F_FSM_PROTOCOL_STRINGS);
	registerProtocol(DP_PROTOCOL, DP_PROTOCOL_STRINGS);
	itsResponse  = new RTDBPerfResp;
	itsTimerPort = new GCFTimerPort(*this, "timerPort");
}

tPerformance::~tPerformance()
{
	LOG_DEBUG("Deleting tPerformance");
	if (itsService) {
		delete itsService;
	}
	if (itsResponse) {
		delete itsResponse;
	}
	if (itsTimerPort) {
		delete itsTimerPort;
	}
}

//
// initial (event, port)
//
GCFEvent::TResult tPerformance::initial(GCFEvent& e, GCFPortInterface& /*p*/)
{
	LOG_DEBUG_STR("initial:" << eventName(e));
	GCFEvent::TResult status = GCFEvent::HANDLED;

	switch (e.signal) {
	case F_ENTRY:
		break;

	case F_INIT:  {
		LOG_INFO("Creating a Service Class");
		itsService = new PVSSservice(itsResponse);

		itsTimerPort->setTimer(1.0);
	}
	break;

	case F_TIMER:
		TRAN(tPerformance::test1cleanup);
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
GCFEvent::TResult tPerformance::final(GCFEvent& e, GCFPortInterface& /*p*/)
{
	LOG_DEBUG_STR("final:" << eventName(e));
	GCFEvent::TResult status = GCFEvent::HANDLED;

	switch (e.signal) {
	case F_ENTRY:
		if (itsService) {
//			itsService->dpDelete("testInt");
			itsTimerPort->setTimer(1.0);
		}
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
// test1cleanup (event, port)
//
GCFEvent::TResult tPerformance::test1cleanup(GCFEvent& e, GCFPortInterface& p)
{
	LOG_DEBUG_STR("test1cleanup:" << eventName(e) << "@" << p.getName());

	GCFEvent::TResult status = GCFEvent::HANDLED;

	switch (e.signal) {
	case F_ENTRY: {
		// test PVSSInfo class
		bool	DBok (PVSSinfo::typeExists("ExampleDP_Int"));
		LOG_INFO_STR("typeExist(ExampleDP_Int): " << (DBok ? "Yes" : "no"));
		ASSERTSTR(DBok, "type ExampleDP_Int does not exist in PVSS");

		LOG_INFO_STR("Cleaning up old datapoints if any");
		gDeleteCounter = 0;
		string	DPname;
		for (int i = 0; i < NR_OF_DPS; i++) {
			DPname = formatString ("Integer%04d", i);
			if (PVSSinfo::propExists(DPname)) {
				itsService->dpDelete(DPname);
				gDeleteCounter++;
			}
		}
		// wait for gDeleteCounter to become 0 (decreased in PSA)
		itsTimerPort->setTimer(1.0);
	}
	break;

	case F_TIMER:
		if (gDeleteCounter != 0) {
			itsTimerPort->setTimer(1.0);
			LOG_INFO_STR ("Waiting for " << gDeleteCounter << " datapoints to disappear");
		}
		else {
			TRAN(tPerformance::test1create);
		}
		break;

	default:
		status = GCFEvent::NOT_HANDLED;
		break;
	}

	return status;
}

//
// test1create (event, port)
//
GCFEvent::TResult tPerformance::test1create(GCFEvent& e, GCFPortInterface& p)
{
	LOG_DEBUG_STR("test1create:" << eventName(e) << "@" << p.getName());

	GCFEvent::TResult status = GCFEvent::HANDLED;
	static NSTimer		timer("Creating");

	switch (e.signal) {
	case F_ENTRY: {
		LOG_INFO_STR("Creating " << NR_OF_DPS << " integer variables");
		string	DPname;
		gCreateCounter = NR_OF_DPS;
		timer.start();
		itsPSvector.resize(NR_OF_DPS);
		for (int i = 0; i < NR_OF_DPS; i++) {
			DPname = formatString ("Integer%04d", i);
			itsPSvector[i] = new RTDBPropertySet(DPname, "ExampleDP_Int", PSAT_WO | PSAT_TMP, this);
			ASSERTSTR(itsPSvector[i], "Creation variable " << i << " went wrong");
		}
		timer.stop();
		LOG_INFO_STR (timer);
		timer.start();
		itsTimerPort->setTimer(0.25);
	}
	break;

	case F_TIMER:
		if (gCreateCounter != 0) {
			itsTimerPort->setTimer(0.25);
			LOG_INFO_STR ("Waiting for " << gCreateCounter << " datapoints to be created");
		}
		else {
			timer.stop();
			LOG_INFO_STR (timer);
			TRAN(tPerformance::test1setvalue);
		}
		break;

	case DP_CREATED:
		// ignore result, its about speed not about correctness
		gCreateCounter--;
		if (gCreateCounter <= 0) {
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
// test1setvalue (event, port)
//
GCFEvent::TResult tPerformance::test1setvalue(GCFEvent& e, GCFPortInterface& p)
{
	LOG_DEBUG_STR("test1setvalue:" << eventName(e) << "@" << p.getName());

	GCFEvent::TResult status = GCFEvent::HANDLED;
	PVSSresult		  result;
	static NSTimer		timer("Setting");

	switch (e.signal) {
	case F_ENTRY: {
		LOG_INFO_STR("Setting " << NR_OF_DPS << " integer variables");
		gSetCounter = NR_OF_DPS;
		timer.start();
		for (int i = 0; i < NR_OF_DPS; i++) {
			GCFPVInteger	newVal(123-i);
			result = itsPSvector[i]->setValue("value", newVal);
			ASSERTSTR(result == SA_NO_ERROR, "Setting variable " << i << 
											" returned result: " << PVSSerrstr(result));
		}
		timer.stop();
		LOG_INFO_STR (timer);
		timer.start();
		itsTimerPort->setTimer(0.1);
	}
	break;

	case F_TIMER:
		if (gSetCounter != 0) {
			itsTimerPort->setTimer(0.1);
			LOG_INFO_STR ("Waiting for " << gSetCounter << " datapoints to be written");
		}
		else {
			timer.stop();
			LOG_INFO_STR (timer);
			TRAN(tPerformance::test1getvalue);
		}
		break;

	case DP_SET:
		gSetCounter--;
		if (gSetCounter <= 0) {
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
// test1getvalue (event, port)
//
GCFEvent::TResult tPerformance::test1getvalue(GCFEvent& e, GCFPortInterface& p)
{
	LOG_DEBUG_STR("test1getvalue:" << eventName(e) << "@" << p.getName());

	GCFEvent::TResult status = GCFEvent::HANDLED;
	PVSSresult		  result;
	static NSTimer		timer("Getting");

	switch (e.signal) {
	case F_ENTRY: {
		LOG_INFO_STR("Getting " << NR_OF_DPS << " integer variables");
		timer.start();
		for (int i = 0; i < NR_OF_DPS; i++) {
			GCFPVInteger	resultVar;
			result = itsPSvector[i]->getValue("value", resultVar);
			ASSERTSTR(result == SA_NO_ERROR, "Getting variable " << i << 
											" returned result: " << PVSSerrstr(result));
			if (resultVar.getValue() != (123-i)) {
				LOG_INFO_STR("Variable " << i << " has value " << resultVar.getValue());
			}
		}
		timer.stop();
		LOG_INFO_STR (timer);
		// note: getAction is synchronous, don't need to set the timer again
		itsTimerPort->setTimer(0.0);
	}
	break;

	case F_TIMER:
		TRAN(tPerformance::test1delete);
		break;

	default:
		status = GCFEvent::NOT_HANDLED;
		break;
	}

	return status;
}

//
// test1delete (event, port)
//
GCFEvent::TResult tPerformance::test1delete(GCFEvent& e, GCFPortInterface& p)
{
	LOG_DEBUG_STR("test1delete:" << eventName(e) << "@" << p.getName());

	GCFEvent::TResult status = GCFEvent::HANDLED;
	static NSTimer		timer("Deleting");

	switch (e.signal) {
	case F_ENTRY: {
		LOG_INFO_STR("Deleting " << NR_OF_DPS << " integer variables");
		timer.start();
		for (int i = NR_OF_DPS-1; i >= 0; i--) {
			delete itsPSvector[i];
		}
		timer.stop();
		LOG_INFO_STR (timer);
		// note: delete is sync action, don't restart timer.
		itsTimerPort->setTimer(0.0);
	}
	break;

	case F_TIMER:
			TRAN(tPerformance::final);
		break;

	case DP_DELETED:
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
	switch (argc) {
	case 2:		NR_OF_DPS = atoi(argv[1]);
				break;
	default:	cout << "Syntax: " << argv[0] << " number_of_datapoints" << endl;
				exit(1);
	}

	if (!NR_OF_DPS) {
		cout << "number_of_datapoints forced to 10" << endl;
		NR_OF_DPS = 10;
	}

	TM::GCFScheduler::instance()->init(argc, argv);

	RTDB::tPerformance test_task("RTDBSpeedTest");  
	test_task.start(); // make initial transition

	TM::GCFScheduler::instance()->run();

	return 0;
}
