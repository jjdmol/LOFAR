//
//  tGSAService.cc: Test program to test the majority of the GSA Service class.
//
//  Copyright (C) 2003
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
#include <GCF/PAL/GCF_PVSSInfo.h>
#include "tGSAService.h"

using std::cout;
using std::endl;

namespace LOFAR {
  namespace GCF {
  using namespace Common;
  using namespace TM;
  namespace PAL {


tGSAService::tGSAService(const string& name) : 
	GCFTask((State)&tGSAService::initial, name), 
	_pService(0),
	itsTimerPort(0)
{
	TM::registerProtocol(F_FSM_PROTOCOL, F_FSM_PROTOCOL_STRINGS);
	itsTimerPort = new GCFTimerPort(*this, "timerPort");
}

tGSAService::~tGSAService()
{
	cout << "Deleting tGSAService" << endl;
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
GCFEvent::TResult tGSAService::initial(GCFEvent& e, GCFPortInterface& /*p*/)
{
	LOG_DEBUG_STR("initial:" << eventName(e));
	GCFEvent::TResult status = GCFEvent::HANDLED;

	switch (e.signal) {
	case F_ENTRY:
		break;

	case F_INIT:  {
		LOG_DEBUG("Creating a Service Class");
		_pService = new Service();

		// test PVSSInfo class
		int8	sysID;
		string	sysName;
		LOG_DEBUG_STR("LocalSystemName: " << GCFPVSSInfo::getLocalSystemName());
		LOG_DEBUG_STR("LocalSystemID  : " << (sysID = GCFPVSSInfo::getLocalSystemId()));
		LOG_DEBUG_STR("ProjectName    : " << GCFPVSSInfo::getProjectName());
		LOG_DEBUG_STR("SystemName(" << sysID << ") : " << (sysName = GCFPVSSInfo::getSystemName(sysID)));
		LOG_DEBUG_STR("SystemID(" << sysName << ") : " << GCFPVSSInfo::getSysId(sysName));
		LOG_DEBUG_STR("Own Man Num    : " << GCFPVSSInfo::getOwnManNum());
		LOG_DEBUG_STR("Last Evt ManNum: " << GCFPVSSInfo::getLastEventManNum());
		LOG_DEBUG_STR("typeExist(ExampleDP_Bit): " << (GCFPVSSInfo::typeExists("ExampleDP_Bit") ? "Yes" : "no"));
		LOG_DEBUG_STR("typeExist(IsErNiet): " << (GCFPVSSInfo::typeExists("IsErNiet") ? "Yes" : "no"));
		LOG_DEBUG_STR("propExist(testBit): " << (GCFPVSSInfo::propExists("testBit") ? "Yes" : "no"));
		LOG_DEBUG_STR("propExist(IsErNiet): " << (GCFPVSSInfo::propExists("IsErNiet") ? "Yes" : "no"));

		if (GCFPVSSInfo::propExists("testBit")) {
			_pService->dpDelete("testBit");
		}
		if (GCFPVSSInfo::propExists("testInt")) {
			_pService->dpDelete("testInt");
		}
		itsTimerPort->setTimer(1.0);
	}
	break;

	case F_TIMER:
		TRAN(tGSAService::test1);
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
GCFEvent::TResult tGSAService::final(GCFEvent& e, GCFPortInterface& /*p*/)
{
	LOG_DEBUG_STR("final:" << eventName(e));
	GCFEvent::TResult status = GCFEvent::HANDLED;

	switch (e.signal) {
	case F_ENTRY:
		LOG_DEBUG("Creating a Service Class");
		if (_pService) {
			_pService->dpDelete("testBit");
			_pService->dpDelete("testInt");
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
// test1 (event, port)
//
GCFEvent::TResult tGSAService::test1(GCFEvent& e, GCFPortInterface& p)
{
	LOG_DEBUG_STR("test1:" << eventName(e) << "@" << p.getName());

	GCFEvent::TResult status = GCFEvent::HANDLED;
	TSAResult		  result;

	switch (e.signal) {
	case F_INIT: 
		break;

	case F_ENTRY:
		LOG_DEBUG("Creating a bit variable: testBit");
		result = _pService->dpCreate("testBit", "ExampleDP_Bit");
		ASSERTSTR(result == SA_NO_ERROR, "Creation of a bit variable returned result: " 
					<< GSAerror(result));
		itsTimerPort->setTimer(1.0);
		break;

	case F_TIMER:
		TRAN(tGSAService::test2);
		break;

	default:
		status = GCFEvent::NOT_HANDLED;
		break;
	}

	return status;
}

//
// test2 (event, port)
//
GCFEvent::TResult tGSAService::test2(GCFEvent& e, GCFPortInterface& p)
{
	LOG_DEBUG_STR("test2:" << eventName(e) << "@" << p.getName());

	GCFEvent::TResult status = GCFEvent::HANDLED;
	TSAResult		  result;

	switch (e.signal) {
	case F_INIT: 
		break;

	case F_ENTRY:
		LOG_DEBUG("Creating a integer variable: testInt");
        result = _pService->dpCreate("testInt", "ExampleDP_Int");
		ASSERTSTR(result == SA_NO_ERROR, "Creation of a int variable returned result: "
					<< GSAerror(result));
		itsTimerPort->setTimer(1.0);
		break;

	case F_TIMER:
		TRAN(tGSAService::test3);
		break;

	default:
		status = GCFEvent::NOT_HANDLED;
		break;
	}

	return status;
}

//
// test3 (event, port)
//
GCFEvent::TResult tGSAService::test3(GCFEvent& e, GCFPortInterface& p)
{
	LOG_DEBUG_STR("test3:" << eventName(e) << "@" << p.getName());

	GCFEvent::TResult status = GCFEvent::HANDLED;
	TSAResult		  result;

	switch (e.signal) {
	case F_ENTRY:
		LOG_DEBUG("Taking a subscription on testBit");
        result = _pService->dpeSubscribe("testBit");
		ASSERTSTR(result == SA_NO_ERROR, 
				"Taking a subscription on testBit returned result: " << GSAerror(result));
		itsTimerPort->setTimer(1.0);
		break;

	case F_TIMER:
		TRAN(tGSAService::test4);
		break;

	default:
		status = GCFEvent::NOT_HANDLED;
		break;
	}

	return status;
}

//
// test4 (event, port)
//
GCFEvent::TResult tGSAService::test4(GCFEvent& e, GCFPortInterface& p)
{
	LOG_DEBUG_STR("test4:" << eventName(e) << "@" << p.getName());

	GCFEvent::TResult status = GCFEvent::HANDLED;
	TSAResult		  result;

	switch (e.signal) {
	case F_ENTRY:
		LOG_DEBUG("Taking a subscription on testInt");
        result = _pService->dpeSubscribe("testInt");
		ASSERTSTR(result == SA_NO_ERROR, 
				"Taking a subscription on testInt returned result: " << GSAerror(result));
		itsTimerPort->setTimer(1.0);
		break;

	case F_TIMER:
		TRAN(tGSAService::test5);
		break;

	default:
		status = GCFEvent::NOT_HANDLED;
		break;
	}

	return status;
}

//
// test5 (event, port)
//
GCFEvent::TResult tGSAService::test5(GCFEvent& e, GCFPortInterface& p)
{
	LOG_DEBUG_STR("test5:" << eventName(e) << "@" << p.getName());

	GCFEvent::TResult status = GCFEvent::HANDLED;
	TSAResult		  result;

	switch (e.signal) {
	case F_ENTRY: {
		try {
			LOG_DEBUG("Writing a bool to the integer var");
			GCFPVBool wrongTestVal(true);
			result = _pService->dpeSet("testInt", wrongTestVal, 0.0);
		}
		catch (Exception& except) {
			LOG_INFO_STR ("Writing a bool to the testInt returned result: " 
							<< GSAerror(result));
		}
		itsTimerPort->setTimer(1.0);
	}
	break;

	case F_TIMER:
		TRAN(tGSAService::test6);
		break;

	default:
		status = GCFEvent::NOT_HANDLED;
		break;
	}

	return status;
}

//
// test6 (event, port)
//
GCFEvent::TResult tGSAService::test6(GCFEvent& e, GCFPortInterface& p)
{
	LOG_DEBUG_STR("test6:" << eventName(e) << "@" << p.getName());

	GCFEvent::TResult status = GCFEvent::HANDLED;
	TSAResult		  result;

	switch (e.signal) {
	case F_ENTRY: {
		LOG_DEBUG("Writing a integer to the integer var");
		GCFPVInteger goodTestVal(1000);
		result = _pService->dpeSet("testInt", goodTestVal, 0.0);
		ASSERTSTR(result == SA_NO_ERROR, "Writing an integer to the testInt returned result: " << GSAerror(result));
		itsTimerPort->setTimer(1.0);
	}
	break;

	case F_TIMER:
		TRAN(tGSAService::test7);
		break;

	default:
		status = GCFEvent::NOT_HANDLED;
		break;
	}

	return status;
}

//
// test7 (event, port)
//
GCFEvent::TResult tGSAService::test7(GCFEvent& e, GCFPortInterface& p)
{
	LOG_DEBUG_STR("test7:" << eventName(e) << "@" << p.getName());

	GCFEvent::TResult status = GCFEvent::HANDLED;
	TSAResult		  result;

	switch (e.signal) {
	case F_ENTRY: {
		LOG_DEBUG("Reading back the integer variable");
        result = _pService->dpeGet("testInt");
		ASSERTSTR(result == SA_NO_ERROR, "Reading the integer testInt returned result: " << GSAerror(result));
		itsTimerPort->setTimer(1.0);
	}
	break;

	case F_TIMER:
		TRAN(tGSAService::test8);
		break;

	default:
		status = GCFEvent::NOT_HANDLED;
		break;
	}

	return status;
}

//
// test8 (event, port)
//
GCFEvent::TResult tGSAService::test8(GCFEvent& e, GCFPortInterface& p)
{
	LOG_DEBUG_STR("test8:" << eventName(e) << "@" << p.getName());

	GCFEvent::TResult status = GCFEvent::HANDLED;
	TSAResult		  result;

	switch (e.signal) {
	case F_ENTRY: {
		LOG_DEBUG("Unsubscribing from the variables");
		result = _pService->dpeUnsubscribe("testInt");
		ASSERTSTR(result == SA_NO_ERROR, "Unsubscribing from testInt returned result: " << GSAerror(result));
		result = _pService->dpeUnsubscribe("testBit");
		ASSERTSTR(result == SA_NO_ERROR, "Unsubscribing from testBit returned result: " << GSAerror(result));
		try {
			result = _pService->dpeUnsubscribe("DoesNotExist");
		}
		catch (Exception&	except) {
			LOG_DEBUG_STR("Unsubscribing from DoesNotExist returned result: " << GSAerror(result));
		}
		itsTimerPort->setTimer(1.0);
	}
	break;

	case F_TIMER:
		TRAN(tGSAService::test9);
		break;

	default:
		status = GCFEvent::NOT_HANDLED;
		break;
	}

	return status;
}

//
// test9 (event, port)
//
GCFEvent::TResult tGSAService::test9(GCFEvent& e, GCFPortInterface& p)
{
	LOG_DEBUG_STR("test9:" << eventName(e) << "@" << p.getName());

	GCFEvent::TResult status = GCFEvent::HANDLED;
	TSAResult		  result;

	switch (e.signal) {
	case F_ENTRY: {
		LOG_DEBUG("Subscribe, unsubscribe and dpSet on the testInt");
		result = _pService->dpeSubscribe("testInt");
		ASSERTSTR(result == SA_NO_ERROR, "Subscribing from testInt returned result: " << GSAerror(result));
		result = _pService->dpeUnsubscribe("testInt");
		ASSERTSTR(result == SA_NO_ERROR, "Unsubscribing from testInt returned result: " << GSAerror(result));
		GCFPVInteger goodTestVal(1000);
		result = _pService->dpeSet("testInt", goodTestVal, 0.0);
		ASSERTSTR(result == SA_NO_ERROR, "Writing an integer to the testInt returned result: " << GSAerror(result));
		itsTimerPort->setTimer(1.0);
	}
	break;

	case F_TIMER:
		TRAN(tGSAService::test10);
		break;

	default:
		status = GCFEvent::NOT_HANDLED;
		break;
	}

	return status;
}

//
// test10 (event, port)
//
GCFEvent::TResult tGSAService::test10(GCFEvent& e, GCFPortInterface& p)
{
	LOG_DEBUG_STR("test10:" << eventName(e) << "@" << p.getName());

	GCFEvent::TResult status = GCFEvent::HANDLED;
	TSAResult		  result;

	switch (e.signal) {
	case F_ENTRY: {
		LOG_DEBUG("Reading back a non existant variable");
		try {
			result = _pService->dpeGet("UnknownVariable");
			ASSERTSTR(result == SA_NO_ERROR, "Reading an unknown variable should return an error!");
		}
		catch (Exception& except) {
			LOG_INFO_STR ("Reading an unknown variable returned result: " 
							<< GSAerror(result));
		}
		itsTimerPort->setTimer(1.0);
	}
	break;

	case F_TIMER:
		TRAN(tGSAService::final);
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
	TM::GCFTask::init(argc, argv);

	PAL::tGSAService test_task("SALtest");  
	test_task.start(); // make initial transition

	TM::GCFTask::run();

	return 0;
}
