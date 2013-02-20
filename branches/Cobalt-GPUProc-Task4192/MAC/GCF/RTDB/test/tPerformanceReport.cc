//
//  tPerformance.cc: Test for supporting the 'PVSS performance' report
//
//  Copyright (C) 2008
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
#include "tPerformanceReport.h"
#include "RTDBPerfResp.h"

int		NR_OF_ACTIONS(1000);
bool	gReadMode;

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
		TRAN(tPerformance::test1create);
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
		itsPSvector.resize(NR_OF_DPS);
		for (int i = 0; i < NR_OF_DPS; i++) {
			DPname = formatString ("Integer%04d", i);
			LOG_INFO_STR("Creating a PS in '" << (gReadMode ? "Read" : "Write") << "Only' mode");
			itsPSvector[i] = new RTDBPropertySet(DPname, "ExampleDP_Int", (gReadMode ? PSAT_RO : PSAT_WO), this);
			ASSERTSTR(itsPSvector[i], "Creation variable " << i << " went wrong");
		}
		itsTimerPort->setTimer(0.25);
	}
	break;

	case F_TIMER:
		if (gCreateCounter != 0) {
			itsTimerPort->setTimer(0.25);
			LOG_INFO_STR ("Waiting for " << gCreateCounter << " datapoints to be created");
		}
		else {
			if (gReadMode) {
				LOG_INFO("Moving to state test1getvalue");
				TRAN(tPerformance::test1getvalue);
			}
			else {
				LOG_INFO("Moving to state test1setvalue");
				TRAN(tPerformance::test1setvalue);
			}
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
		LOG_INFO_STR("Writing " << NR_OF_ACTIONS << " integer values");
		gSetCounter = NR_OF_ACTIONS;
		timer.start();
		for (int i = 0; i < NR_OF_ACTIONS; i++) {
			GCFPVInteger	newVal(5123-i);
			result = itsPSvector[0]->setValue("value", newVal);
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
			TRAN(tPerformance::final);
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
	static NSTimer		timer("Getting");

	switch (e.signal) {
	case F_ENTRY: {
		LOG_INFO_STR("Waiting for " << NR_OF_ACTIONS << " value changes");
		gGetCounter = NR_OF_ACTIONS;
	}
	break;

	case DP_CHANGED:
		if (gGetCounter == NR_OF_ACTIONS) {
			timer.start();
		}
		gGetCounter--;
		if (gGetCounter <= 0) {
			timer.stop();
			LOG_INFO_STR(timer);
			TRAN(tPerformance::final);
		}
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
	bool syntax_error(false);

	switch (argc) {
	case 2:		if (argv[1][0]=='r' || argv[1][0] =='R') {
					gReadMode = true;
				}
				else if (argv[1][0]!='w' && argv[1][0] !='W') {
					gReadMode = false;
					syntax_error = true;
				}
				break;
	default:	syntax_error = true;
	}

	if (syntax_error) {
		cout << "This test is meant to measure the reaction time between the C++ I/F of PVSS" << endl;
		cout << "and the database itself. In the database a read- or write-script must be run" << endl;
		cout << "simultaneously to get the responsetimes from the database. " << endl;
		cout << "There is a read-test and a write test. In the read-test the C-program waits for" << endl;
		cout << "1000 changes of a datapoint and measures the time between the first and the last trigger." << endl;
		cout << "The write test goes the otherway around." << endl << endl;
		cout << "Syntax: " << argv[0] << " w|r" << endl;
		cout << "         w: write data to datapoint" << endl;
		cout << "         r: read data to datapoint" << endl;
		exit(1);
	}

	cout << "Starting in mode " << (gReadMode ? "'read'" : "'write'") << endl;

	NR_OF_DPS = 1;
	NR_OF_ACTIONS = 1000;

	TM::GCFScheduler::instance()->init(argc, argv);

	RTDB::tPerformance test_task("PVSSperformanceTest");  
	test_task.start(); // make initial transition

	TM::GCFScheduler::instance()->run();

	return 0;
}
