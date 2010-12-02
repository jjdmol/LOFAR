//
//  tPAPerformance.cc: Test program to test performance of the PA stuff
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
#include <GCF/Protocols/PA_Protocol.ph>
#include "tPAPerformance.h"

namespace LOFAR {
  namespace GCF {
  using namespace Common;
  using namespace TM;
  namespace PAL {


tPAPerformance::tPAPerformance(const string& name) : 
	GCFTask((State)&tPAPerformance::initial, name), 
	GCFPropertySetAnswerHandlerInterface(),
	itsPSA(*this),
	itsTimerPort(0)
{
	TM::registerProtocol(F_PML_PROTOCOL, F_PML_PROTOCOL_STRINGS);
	TM::registerProtocol(PA_PROTOCOL,    PA_PROTOCOL_STRINGS);
	itsTimerPort = new GCFTimerPort(*this, "timerPort");
}

tPAPerformance::~tPAPerformance()
{
	cout << "Deleting tPAPerformance" << endl;
	if (itsTimerPort) {
		delete itsTimerPort;
	}
}

//
// handlePropertySetAnswer(answer)
//
void tPAPerformance::handlePropertySetAnswer(GCFEvent&	answer)
{
	switch (answer.signal) {
    case F_MYPS_ENABLED:
		gCreateCounter--;
		break;
    case F_VSETRESP:
		gSetCounter--;
		break;
    case F_VGETRESP:
		gSetCounter--;
		break;
    case F_MYPS_DISABLED:
		gDeleteCounter--;
		break;

//  case F_SUBSCRIBED:      GCFPropAnswerEvent      pPropName
//  case F_UNSUBSCRIBED:    GCFPropAnswerEvent      pPropName
//  case F_PS_CONFIGURED:   GCFConfAnswerEvent      pApcName
//  case F_EXTPS_LOADED:    GCFPropSetAnswerEvent   pScope, result
//  case F_EXTPS_UNLOADED:  GCFPropSetAnswerEvent   pScope, result
//  case F_VCHANGEMSG:      GCFPropValueEvent       pValue, pPropName
//  case F_SERVER_GONE:     GCFPropSetAnswerEvent   pScope, result
	default:
		break;
	}
}

//
// initial (event, port)
//
GCFEvent::TResult tPAPerformance::initial(GCFEvent& e, GCFPortInterface& /*p*/)
{
	LOG_DEBUG_STR("initial:" << eventName(e));
	GCFEvent::TResult status = GCFEvent::HANDLED;

	switch (e.signal) {
	case F_ENTRY:
		break;

	case F_INIT:  {
		LOG_INFO("Creating a Service Class");
		itsCoreService = new PerformanceService();

//		itsPropSetVector.resize(NR_OF_DPS, (GCFMyPropertySet*)0);
		itsPropSetVector.reserve(NR_OF_DPS);

		itsTimerPort->setTimer(1.0);
	}
	break;

	case F_TIMER:
		TRAN(tPAPerformance::test1cleanup);
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
GCFEvent::TResult tPAPerformance::final(GCFEvent& e, GCFPortInterface& /*p*/)
{
	LOG_DEBUG_STR("final:" << eventName(e));
	GCFEvent::TResult status = GCFEvent::HANDLED;

	switch (e.signal) {
	case F_ENTRY:
		if (itsCoreService) {
//			itsCoreService->dpDelete("testInt");
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
GCFEvent::TResult tPAPerformance::test1cleanup(GCFEvent& e, GCFPortInterface& p)
{
	static bool cleanupReady(false);
	LOG_DEBUG_STR("test1cleanup:" << eventName(e) << "@" << p.getName());

	GCFEvent::TResult status = GCFEvent::HANDLED;

	switch (e.signal) {
	case F_ENTRY: {
		// test PVSSInfo class
		if (gStructSize == 1) {
			bool	DBok (GCFPVSSInfo::typeExists("TestInt"));
			LOG_INFO_STR("typeExist(TestInt): " << DBok ? "Yes" : "no");
			ASSERTSTR(DBok, "type TestInt does not exist in PVSS");
		}
		else {
			bool	DBok (GCFPVSSInfo::typeExists("TestStruct5"));
			LOG_INFO_STR("typeExist(TestStruct5): " << DBok ? "Yes" : "no");
			ASSERTSTR(DBok, "type TestStruct5 does not exist in PVSS");	
		}

		LOG_INFO_STR("Cleaning up old datapoints if any");
		gDeleteCounter = 0;
		string	DPname;
		for (int i = 0; i < NR_OF_DPS; i++) {
			DPname = formatString ("Integer%04d", i);
			if (GCFPVSSInfo::propExists(DPname)) {
				itsCoreService->dpDelete(DPname);
				gDeleteCounter++;
			}
			DPname = formatString ("Integer%04d__enabled", i);
			if (GCFPVSSInfo::propExists(DPname)) {
				itsCoreService->dpDelete(DPname);
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
			if (!cleanupReady) {
				LOG_INFO_STR("Waiting 10 seconds");
				itsTimerPort->setTimer(10.0);
				cleanupReady = true;
			}
			else {
				TRAN(tPAPerformance::test1create);
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
// test1create (event, port)
//
GCFEvent::TResult tPAPerformance::test1create(GCFEvent& e, GCFPortInterface& p)
{
//	LOG_DEBUG_STR("test1create:" << evtstr(e) << "@" << p.getName());

	GCFEvent::TResult status = GCFEvent::HANDLED;
	static NSTimer		timer("Creating");
	static bool			createReady(false);

	switch (e.signal) {
	case F_ENTRY: {
		LOG_INFO_STR("Creating " << NR_OF_DPS << " integer variables");
		string				DPname;
		timer.start();
		for (int i = 0; i < NR_OF_DPS; i++) {
			DPname = formatString ("Integer%04d", i);
//			if (gStructSize == 1) {
				GCFMyPropertySetPtr	PropSetPtr(new GCFMyPropertySet(DPname.c_str(), 
										"TestInt",
//										PS_CAT_TEMPORARY, &itsPSA);
										PS_CAT_TEMP_AUTOLOAD, &itsPSA));
				ASSERTSTR(PropSetPtr, "Creation of variable " << i << " failed");
				itsPropSetVector[i] = PropSetPtr;
				PropSetPtr->enable();
//			}
//			else {
//				GCFMyPropertySetPtr	PropSetPtr(new GCFMyPropertySet(DPname.c_str(), 
//										"TestStruct5",
////										PS_CAT_TEMPORARY, &itsPSA);
//										PS_CAT_TEMP_AUTOLOAD, &itsPSA));
//				ASSERTSTR(PropSetPtr, "Creation of variable " << i << " failed");
//				itsPropSetVector[i] = PropSetPtr;
//				PropSetPtr->enable();
//			}
		}
		timer.stop();
		LOG_INFO_STR (timer);
		timer.start();
		gCreateCounter = NR_OF_DPS;
		itsTimerPort->setTimer(1.0);
	}
	break;

	case F_TIMER:
		if (gCreateCounter != 0) {
			itsTimerPort->setTimer(1.0);
			LOG_INFO_STR ("Waiting for " << gCreateCounter << " datapoints to be created");
		}
		else {
			if (!createReady) {
				timer.stop();
				LOG_INFO_STR (timer);
				LOG_INFO_STR("Waiting 30 seconds");
				itsTimerPort->setTimer(30.0);
				createReady = true;
			}
			else {
				TRAN(tPAPerformance::test1setvalue);
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
// test1setvalue (event, port)
//
GCFEvent::TResult tPAPerformance::test1setvalue(GCFEvent& e, GCFPortInterface& p)
{
	static bool		setReady(false);
	LOG_DEBUG_STR("test1setvalue:" << eventName(e) << "@" << p.getName());

	GCFEvent::TResult status = GCFEvent::HANDLED;

	switch (e.signal) {
	case F_ENTRY: {
		LOG_INFO_STR("Setting " << NR_OF_DPS << " integer variables");
		string	DPname;
		NSTimer		timer("Setvalue");
		timer.start();
		for (int i = 0; i < NR_OF_DPS; i++) {
			DPname = formatString ("Integer%04d.theValue", i);
			GCFPVInteger	newVal(123-i);
			itsPropSetVector[i]->setValue(DPname, newVal);
		}
		timer.stop();
		LOG_INFO_STR (timer);
		gSetCounter = 0;  // !!!!!!!
		itsTimerPort->setTimer(1.0);
	}
	break;

	case F_TIMER:
		if (gSetCounter != 0) {
			itsTimerPort->setTimer(1.0);
			LOG_INFO_STR ("Waiting for " << gSetCounter << " datapoints to be written");
		}
		else {
			if (!setReady) {
				LOG_INFO_STR("Waiting 30 seconds");
				itsTimerPort->setTimer(30.0);
				setReady = true;
			}
			else {
				TRAN(tPAPerformance::test1getvalue);
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
// test1getvalue (event, port)
//
GCFEvent::TResult tPAPerformance::test1getvalue(GCFEvent& e, GCFPortInterface& p)
{
	LOG_DEBUG_STR("test1getvalue:" << eventName(e) << "@" << p.getName());

	GCFEvent::TResult status = GCFEvent::HANDLED;
	static NSTimer		timer("Getting");
	static bool			getReady(false);

	switch (e.signal) {
	case F_ENTRY: {
		LOG_INFO_STR("Getting " << NR_OF_DPS << " integer variables");
		string	DPname;
		timer.start();
		for (int i = 0; i < NR_OF_DPS; i++) {
			DPname = formatString ("Integer%04d.theValue", i);
			itsPropSetVector[i]->getValue(DPname);
		}
		timer.stop();
		LOG_INFO_STR (timer);
		timer.start();
		gGetCounter = 0;  // !!!!!!!
		itsTimerPort->setTimer(0.1);
	}
	break;

	case F_TIMER:
		if (gGetCounter != 0) {
			itsTimerPort->setTimer(0.1);
			LOG_INFO_STR ("Waiting for " << gGetCounter << " datapoints to be read");
		}
		else {
			if (!getReady) {
				timer.stop();
				LOG_INFO_STR (timer);
				LOG_INFO_STR("Waiting 30 seconds");
				itsTimerPort->setTimer(30.0);
				getReady = true;
			}
			else {
				TRAN(tPAPerformance::test1delete);
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
// test1delete (event, port)
//
GCFEvent::TResult tPAPerformance::test1delete(GCFEvent& e, GCFPortInterface& p)
{
	LOG_DEBUG_STR("test1delete:" << eventName(e) << "@" << p.getName());

	GCFEvent::TResult status = GCFEvent::HANDLED;
	static NSTimer		timer("Deleting");
	static bool			deleteReady(false);

	switch (e.signal) {
	case F_ENTRY: {
		LOG_INFO_STR("Deleting " << NR_OF_DPS << " integer variables");
		string	DPname;
		timer.start();
		for (int i = NR_OF_DPS - 1; i >= 0; --i) {
			DPname = formatString ("Integer%04d", i);
//			delete itsPropSetVector[i];
			itsPropSetVector[i]->disable();
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
			itsTimerPort->setTimer(1.0);
			LOG_INFO_STR ("Waiting for " << gDeleteCounter << " datapoints to be deleted");
		}
		else {
			if (!deleteReady) {
				timer.stop();
				LOG_INFO_STR (timer);
				LOG_INFO_STR("Waiting 30 seconds");
				itsTimerPort->setTimer(30.0);
				deleteReady = true;
			}
			else {
				TRAN(tPAPerformance::final);
			}
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
using namespace std;

int main(int argc, char* argv[])
{
	gStructSize = 1;

	switch (argc) {
	case 3:		if (atoi(argv[2]) == 5) {
					gStructSize = 5;
				}
				// no break!
	case 2:		NR_OF_DPS = atoi(argv[1]);
				break;
	default:	cout << "Syntax: " << argv[0] << " number_of_datapoints [5]" << endl;
				cout << "    5   Used struct with 5 elements iso single integer" << endl;
				exit(1);
	}

	if (!NR_OF_DPS) {
		cout << "number_of_datapoints forced to 10" << endl;
		NR_OF_DPS = 10;
	}

	TM::GCFTask::init(argc, argv);

	PAL::tPAPerformance test_task("PASpeedTest");  
	test_task.start(); // make initial transition

	TM::GCFTask::run();

	return 0;
}
