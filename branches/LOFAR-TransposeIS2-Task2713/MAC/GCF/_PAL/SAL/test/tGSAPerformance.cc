//
//  tGSAPerformance.cc: Test program to test the majority of the GSA Service class.
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
#include <Common/Timer.h>
#include <GCF/GCF_PVTypes.h>
#include <GCF/PAL/GCF_PVSSInfo.h>
#include "tGSAPerformance.h"

using std::cout;
using std::endl;

namespace LOFAR {
  namespace GCF {
  using namespace Common;
  using namespace TM;
  namespace PAL {


tGSAPerformance::tGSAPerformance(const string& name) : 
	GCFTask((State)&tGSAPerformance::initial, name), 
	_pService(0),
	itsTimerPort(0)
{
	TM::registerProtocol(F_FSM_PROTOCOL, F_FSM_PROTOCOL_STRINGS);
	itsTimerPort = new GCFTimerPort(*this, "timerPort");
}

tGSAPerformance::~tGSAPerformance()
{
	cout << "Deleting tGSAPerformance" << endl;
	if (_pService) {
		delete _pService;
	}
	if (itsTimerPort) {
		delete itsTimerPort;
	}
}

//
// initial (event, port)
//
GCFEvent::TResult tGSAPerformance::initial(GCFEvent& e, GCFPortInterface& /*p*/)
{
	LOG_DEBUG_STR("initial:" << eventName(e));
	GCFEvent::TResult status = GCFEvent::HANDLED;

	switch (e.signal) {
	case F_ENTRY:
		break;

	case F_INIT:  {
		LOG_INFO("Creating a Service Class");
		_pService = new PerformanceService();

		itsTimerPort->setTimer(1.0);
	}
	break;

	case F_TIMER:
		TRAN(tGSAPerformance::test1cleanup);
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
GCFEvent::TResult tGSAPerformance::final(GCFEvent& e, GCFPortInterface& /*p*/)
{
	LOG_DEBUG_STR("final:" << eventName(e));
	GCFEvent::TResult status = GCFEvent::HANDLED;

	switch (e.signal) {
	case F_ENTRY:
		if (_pService) {
//			_pService->dpDelete("testInt");
			itsTimerPort->setTimer(1.0);
		}
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
// test1cleanup (event, port)
//
GCFEvent::TResult tGSAPerformance::test1cleanup(GCFEvent& e, GCFPortInterface& p)
{
	LOG_DEBUG_STR("test1cleanup:" << eventName(e) << "@" << p.getName());

	GCFEvent::TResult status = GCFEvent::HANDLED;

	switch (e.signal) {
	case F_ENTRY: {
		// test PVSSInfo class
		bool	DBok (GCFPVSSInfo::typeExists("ExampleDP_Int"));
		LOG_INFO_STR("typeExist(ExampleDP_Int): " << DBok ? "Yes" : "no");
		ASSERTSTR(DBok, "type ExampleDP_Int does not exist in PVSS");

		LOG_INFO_STR("Cleaning up old datapoints if any");
		gDeleteCounter = 0;
		string	DPname;
		for (int i = 0; i < NR_OF_DPS; i++) {
			DPname = formatString ("Integer%04d", i);
			if (GCFPVSSInfo::propExists(DPname)) {
				_pService->dpDelete(DPname);
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
			TRAN(tGSAPerformance::test1create);
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
GCFEvent::TResult tGSAPerformance::test1create(GCFEvent& e, GCFPortInterface& p)
{
	LOG_DEBUG_STR("test1create:" << eventName(e) << "@" << p.getName());

	GCFEvent::TResult status = GCFEvent::HANDLED;
	TSAResult		  result;
	static NSTimer		timer("Creating");

	switch (e.signal) {
	case F_ENTRY: {
		LOG_INFO_STR("Creating " << NR_OF_DPS << " integer variables");
		string	DPname;
		timer.start();
		for (int i = 0; i < NR_OF_DPS; i++) {
			DPname = formatString ("Integer%04d", i);
			result = _pService->dpCreate(DPname, "ExampleDP_Int");
			ASSERTSTR(result == SA_NO_ERROR, "Creation variable " << i << 
											" returned result: " << GSAerror(result));
		}
		timer.stop();
		LOG_INFO_STR (timer);
		timer.start();
		gCreateCounter = NR_OF_DPS;
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
			TRAN(tGSAPerformance::test1setvalue);
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
GCFEvent::TResult tGSAPerformance::test1setvalue(GCFEvent& e, GCFPortInterface& p)
{
	LOG_DEBUG_STR("test1setvalue:" << eventName(e) << "@" << p.getName());

	GCFEvent::TResult status = GCFEvent::HANDLED;
	TSAResult		  result;
	static NSTimer			  timer("Setting");

	switch (e.signal) {
	case F_ENTRY: {
		LOG_INFO_STR("Setting " << NR_OF_DPS << " integer variables");
		string	DPname;
		timer.start();
		for (int i = 0; i < NR_OF_DPS; i++) {
			DPname = formatString ("Integer%04d", i);
			GCFPVInteger	newVal(123-i);
			result = _pService->dpeSet(DPname, newVal, 0.0, true);
			ASSERTSTR(result == SA_NO_ERROR, "Setting variable " << i << 
											" returned result: " << GSAerror(result));
		}
		timer.stop();
		LOG_INFO_STR (timer);
		timer.start();
		gSetCounter = NR_OF_DPS;
		itsTimerPort->setTimer(0.01);
	}
	break;

	case F_TIMER:
		if (gSetCounter != 0) {
			itsTimerPort->setTimer(0.01);
			LOG_INFO_STR ("Waiting for " << gSetCounter << " datapoints to be written");
		}
		else {
			timer.stop();
			LOG_INFO_STR(timer);
			TRAN(tGSAPerformance::test1getvalue);
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
GCFEvent::TResult tGSAPerformance::test1getvalue(GCFEvent& e, GCFPortInterface& p)
{
	LOG_DEBUG_STR("test1getvalue:" << eventName(e) << "@" << p.getName());

	GCFEvent::TResult status = GCFEvent::HANDLED;
	TSAResult		  result;
	static NSTimer		timer("Getting");

	switch (e.signal) {
	case F_ENTRY: {
		LOG_INFO_STR("Getting " << NR_OF_DPS << " integer variables");
		string	DPname;
		timer.start();
		for (int i = 0; i < NR_OF_DPS; i++) {
			DPname = formatString ("Integer%04d", i);
			result = _pService->dpeGet(DPname);
			ASSERTSTR(result == SA_NO_ERROR, "Getting variable " << i << 
											" returned result: " << GSAerror(result));
		}
		timer.stop();
		LOG_INFO_STR (timer);
		timer.start();
		gGetCounter = NR_OF_DPS;
		itsTimerPort->setTimer(0.1);
	}
	break;

	case F_TIMER:
		if (gGetCounter != 0) {
			itsTimerPort->setTimer(0.1);
			LOG_INFO_STR ("Waiting for " << gGetCounter << " datapoints to be read");
		}
		else {
			timer.stop();
			LOG_INFO_STR (timer);
			TRAN(tGSAPerformance::test1delete);
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
GCFEvent::TResult tGSAPerformance::test1delete(GCFEvent& e, GCFPortInterface& p)
{
	LOG_DEBUG_STR("test1delete:" << eventName(e) << "@" << p.getName());

	GCFEvent::TResult status = GCFEvent::HANDLED;
	TSAResult		  result;
	static NSTimer		timer("Deleting");

	switch (e.signal) {
	case F_ENTRY: {
		LOG_INFO_STR("Deleting " << NR_OF_DPS << " integer variables");
		string	DPname;
		timer.start();
		for (int i = 0; i < NR_OF_DPS; i++) {
			DPname = formatString ("Integer%04d", i);
			result = _pService->dpDelete(DPname);
			ASSERTSTR(result == SA_NO_ERROR, "Deleting variable " << i << 
											" returned result: " << GSAerror(result));
		}
		timer.stop();
		LOG_INFO_STR (timer);
		timer.start();
		gDeleteCounter = NR_OF_DPS;
		itsTimerPort->setTimer(1.0);
	}
	break;

	case F_TIMER:
		if (gDeleteCounter != 0) {
			itsTimerPort->setTimer(0.25);
			LOG_INFO_STR ("Waiting for " << gDeleteCounter << " datapoints to be deleted");
		}
		else {
			timer.stop();
			LOG_INFO_STR (timer);
			TRAN(tGSAPerformance::final);
		}
		break;

	default:
		status = GCFEvent::NOT_HANDLED;
		break;
	}

	return status;
}



  } // namespace PAL
 } // namespace GCF
} // namespace LOFAR


using namespace LOFAR::GCF;

int main(int argc, char* argv[])
{
	switch (argc) {
	case 3:		gValidate = true;
				// no break!
	case 2:		NR_OF_DPS = atoi(argv[1]);
				break;
	default:	cout << "Syntax: " << argv[0] << " number_of_datapoints [-v]" << endl;
				cout << "    -v   Validate values after get." << endl;
				exit(1);
	}

	if (!NR_OF_DPS) {
		cout << "number_of_datapoints forced to 10" << endl;
		NR_OF_DPS = 10;
	}

	TM::GCFTask::init(argc, argv);

	PAL::tGSAPerformance test_task("SALSpeedTest");  
	test_task.start(); // make initial transition

	TM::GCFTask::run();

	return 0;
}
