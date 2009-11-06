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
#include <Common/StringUtil.h>
#include <GCF/PVSS/GCF_PVTypes.h>
#include <GCF/PVSS/PVSSinfo.h>
#include "tPVSSservice.h"
#include "Response.h"

namespace LOFAR {
  namespace GCF {
  using namespace TM;
  namespace PVSS {


tGSAService::tGSAService(const string& name) : 
	GCFTask((State)&tGSAService::initial, name), 
	itsService(0),
	itsResponse(0),
	itsTimerPort(0)
{
	registerProtocol(F_FSM_PROTOCOL, F_FSM_PROTOCOL_STRINGS);
	itsResponse  = new Response;
	itsTimerPort = new GCFTimerPort(*this, "timerPort");
}

tGSAService::~tGSAService()
{
	LOG_DEBUG("Deleting tGSAService");
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
GCFEvent::TResult tGSAService::initial(GCFEvent& e, GCFPortInterface& /*p*/)
{
	LOG_DEBUG_STR("initial:" << eventName(e));
	GCFEvent::TResult status = GCFEvent::HANDLED;

	switch (e.signal) {
	case F_ENTRY:
		break;

	case F_INIT:  {
		LOG_DEBUG("===> Creating a Service Class");
		itsService = new PVSSservice(itsResponse);

		// test PVSSInfo class
		int8	sysID;
		string	sysName;
		LOG_DEBUG_STR("LocalSystemName: " << PVSSinfo::getLocalSystemName());
		LOG_DEBUG_STR("LocalSystemID  : " << (sysID = PVSSinfo::getLocalSystemId()));
		LOG_DEBUG_STR("ProjectName    : " << PVSSinfo::getProjectName());
		LOG_DEBUG_STR("SystemName(" << sysID << ") : " << (sysName = PVSSinfo::getSystemName(sysID)));
		LOG_DEBUG_STR("SystemID(" << sysName << ") : " << PVSSinfo::getSysId(sysName));
		LOG_DEBUG_STR("Own Man Num    : " << PVSSinfo::getOwnManNum());
		LOG_DEBUG_STR("Last Evt ManNum: " << PVSSinfo::getLastEventManNum());
		LOG_DEBUG_STR("typeExist(ExampleDP_Bit): " << (PVSSinfo::typeExists("ExampleDP_Bit") ? "Yes" : "no"));
		LOG_DEBUG_STR("typeExist(IsErNiet): " << (PVSSinfo::typeExists("IsErNiet") ? "Yes" : "no"));
		LOG_DEBUG_STR("propExist(testBit): " << (PVSSinfo::propExists("testBit") ? "Yes" : "no"));
		LOG_DEBUG_STR("propExist(IsErNiet): " << (PVSSinfo::propExists("IsErNiet") ? "Yes" : "no"));

		if (PVSSinfo::propExists("testBit")) {
			itsService->dpDelete("testBit");
		}
		if (PVSSinfo::propExists("testInt")) {
			itsService->dpDelete("testInt");
		}
		if (PVSSinfo::propExists("testDP")) {
			itsService->dpDelete("testDP");
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
		LOG_DEBUG("===> Creating a Service Class");
		if (itsService) {
			itsService->dpDelete("testBit");
			itsService->dpDelete("testInt");
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
	PVSSresult		  result;

	switch (e.signal) {
	case F_INIT: 
		break;

	case F_ENTRY:
		LOG_DEBUG("===> Creating a bit variable: testBit");
		result = itsService->dpCreate("testBit", "ExampleDP_Bit");
		ASSERTSTR(result == SA_NO_ERROR, "Creation of a bit variable returned result: " 
					<< PVSSerrstr(result));
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
	PVSSresult		  result;

	switch (e.signal) {
	case F_INIT: 
		break;

	case F_ENTRY:
		LOG_DEBUG("===> Creating a integer variable: testInt");
        result = itsService->dpCreate("testInt", "ExampleDP_Int");
		ASSERTSTR(result == SA_NO_ERROR, "Creation of a int variable returned result: "
					<< PVSSerrstr(result));
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
	PVSSresult		  result;

	switch (e.signal) {
	case F_ENTRY:
		LOG_DEBUG("===> Taking a subscription on testBit");
        result = itsService->dpeSubscribe("testBit");
		ASSERTSTR(result == SA_NO_ERROR, 
				"Taking a subscription on testBit returned result: " << PVSSerrstr(result));
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
	PVSSresult		  result;

	switch (e.signal) {
	case F_ENTRY:
		LOG_DEBUG("===> Taking a subscription on testInt");
        result = itsService->dpeSubscribe("testInt");
		ASSERTSTR(result == SA_NO_ERROR, 
				"Taking a subscription on testInt returned result: " << PVSSerrstr(result));
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
	PVSSresult		  result;

	switch (e.signal) {
	case F_ENTRY: {
		try {
			LOG_DEBUG("===> Writing a bool to the integer var");
			GCFPVBool wrongTestVal(true);
			result = itsService->dpeSet("testInt", wrongTestVal, 0.0, true);
		}
		catch (Exception& except) {
			LOG_INFO_STR ("Writing a bool to the testInt returned result: " 
							<< PVSSerrstr(result));
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
	PVSSresult		  result;

	switch (e.signal) {
	case F_ENTRY: {
		LOG_DEBUG("===> Writing 1000 to the integer var");
		GCFPVInteger goodTestVal(1000);
		result = itsService->dpeSet("testInt", goodTestVal, 0.0, true);
		ASSERTSTR(result == SA_NO_ERROR, "Writing an integer to the testInt returned result: " << PVSSerrstr(result));
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
	PVSSresult		  result;

	switch (e.signal) {
	case F_ENTRY: {
		LOG_DEBUG("===> Reading back the integer variable");
        result = itsService->dpeGet("testInt");
		ASSERTSTR(result == SA_NO_ERROR, "Reading the integer testInt returned result: " << PVSSerrstr(result));
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
	PVSSresult		  result;

	switch (e.signal) {
	case F_ENTRY: {
		LOG_DEBUG("===> Unsubscribing from the variables");
		result = itsService->dpeUnsubscribe("testInt");
		ASSERTSTR(result == SA_NO_ERROR, "Unsubscribing from testInt returned result: " << PVSSerrstr(result));
		result = itsService->dpeUnsubscribe("testBit");
		ASSERTSTR(result == SA_NO_ERROR, "Unsubscribing from testBit returned result: " << PVSSerrstr(result));
		try {
			result = itsService->dpeUnsubscribe("DoesNotExist");
		}
		catch (Exception&	except) {
			LOG_DEBUG_STR("Unsubscribing from DoesNotExist returned result: " << PVSSerrstr(result));
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
	PVSSresult		  result;

	switch (e.signal) {
	case F_ENTRY: {
		LOG_DEBUG("===> Subscribe, unsubscribe and dpSet on the testInt");
		result = itsService->dpeSubscribe("testInt");
		ASSERTSTR(result == SA_NO_ERROR, "Subscribing from testInt returned result: " << PVSSerrstr(result));
		result = itsService->dpeUnsubscribe("testInt");
		ASSERTSTR(result == SA_NO_ERROR, "Unsubscribing from testInt returned result: " << PVSSerrstr(result));
		GCFPVInteger goodTestVal(1000);
		result = itsService->dpeSet("testInt", goodTestVal, 0.0, true);
		ASSERTSTR(result == SA_NO_ERROR, "Writing an integer to the testInt returned result: " << PVSSerrstr(result));
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
	PVSSresult		  result;

	switch (e.signal) {
	case F_ENTRY: {
		LOG_DEBUG("===> Reading back a non existant variable");
		try {
			result = itsService->dpeGet("UnknownVariable");
			ASSERTSTR(result == SA_NO_ERROR, "Reading an unknown variable should return an error!");
		}
		catch (Exception& except) {
			LOG_INFO_STR ("Reading an unknown variable returned result: " 
							<< PVSSerrstr(result));
		}
		itsTimerPort->setTimer(1.0);
	}
	break;

	case F_TIMER:
		TRAN(tGSAService::testCreateMdpe);
		break;

	default:
		status = GCFEvent::NOT_HANDLED;
		break;
	}

	return status;
}

//
// testCreateMdpe (event, port)
//
GCFEvent::TResult tGSAService::testCreateMdpe(GCFEvent& e, GCFPortInterface& p)
{
	LOG_DEBUG_STR("testCreateMdpe:" << eventName(e) << "@" << p.getName());

	GCFEvent::TResult status = GCFEvent::HANDLED;
	PVSSresult		  result;

	switch (e.signal) {
	case F_INIT: 
		break;

	case F_ENTRY:
		LOG_DEBUG("===> Creating a complex variable: testDP");
		result = itsService->dpCreate("testDP", "TestPS");
		ASSERTSTR(result == SA_NO_ERROR, "Creation of a complex variable returned result: " 
					<< PVSSerrstr(result));
		itsTimerPort->setTimer(1.0);
		break;

	case F_TIMER:
		TRAN(tGSAService::testWriteMdpe);
		break;

	default:
		status = GCFEvent::NOT_HANDLED;
		break;
	}

	return status;
}

//
// testWriteMdpe (event, port)
//
GCFEvent::TResult tGSAService::testWriteMdpe(GCFEvent& e, GCFPortInterface& p)
{
	LOG_DEBUG_STR("testWriteMdpe:" << eventName(e) << "@" << p.getName());

	GCFEvent::TResult status = GCFEvent::HANDLED;
	PVSSresult		  result;

	switch (e.signal) {
	case F_ENTRY: {
		try {
			LOG_DEBUG("===> Setting intVal=4056,bool=false,string='boskabouter'");
			vector<string>	dpeNames;
			dpeNames.push_back("intVal");
			dpeNames.push_back("boolVal");
			dpeNames.push_back("stringVal");
			vector<GCFPValue*>	values;
			GCFPVInteger	theInt(4056);
			GCFPVBool		theBool(false);
			GCFPVString		theString("boskabouter");
			values.push_back(&theInt);
			values.push_back(&theBool);
			values.push_back(&theString);
			result = itsService->dpeSetMultiple("testDP", dpeNames, values, 0.0, true);
		}
		catch (Exception& except) {
			LOG_INFO_STR ("Writing multiple values at once returned result: " 
							<< PVSSerrstr(result));
		}
		itsTimerPort->setTimer(1.0);
	}
	break;

	case F_TIMER:
		TRAN(tGSAService::testWriteMdpeTimed);
		break;

	default:
		status = GCFEvent::NOT_HANDLED;
		break;
	}

	return status;
}


//
// testWriteMdpeTimed (event, port)
//
GCFEvent::TResult tGSAService::testWriteMdpeTimed(GCFEvent& e, GCFPortInterface& p)
{
	LOG_DEBUG_STR("testWriteMdpeTimed:" << eventName(e) << "@" << p.getName());

	GCFEvent::TResult status = GCFEvent::HANDLED;
	PVSSresult		  result;

	switch (e.signal) {
	case F_ENTRY: {
		try {
			LOG_DEBUG("===> Setting uintVal=30092,float=2.49473,time=1/1/2000 03:02:01.825");
			vector<string>	dpeNames;
			dpeNames.push_back("uintVal");
			dpeNames.push_back("floatVal");
			vector<GCFPValue*>	values;
			GCFPVUnsigned	theInt(30092);
			GCFPVDouble		theFloat(2.49473);
			values.push_back(&theInt);
			values.push_back(&theFloat);
			struct tm	theTM;
			theTM.tm_sec=1;
			theTM.tm_min=2;
			theTM.tm_hour=3;
			theTM.tm_mday=1;
			theTM.tm_mon=0;
			theTM.tm_year=100;
			theTM.tm_wday=0;
			theTM.tm_yday=0;
			theTM.tm_isdst=0;
			double	theTime = 1.0*mktime(&theTM) + 0.825;
			LOG_DEBUG(formatString("theTime = %15.4f", theTime));
			result = itsService->dpeSetMultiple("testDP", dpeNames, values, theTime, true);
		}
		catch (Exception& except) {
			LOG_INFO_STR ("Writing multiple values at once returned result: " 
							<< PVSSerrstr(result));
		}
		itsTimerPort->setTimer(1.0);
	}
	break;

	case F_TIMER:
		TRAN(tGSAService::testWriteDynArray);
		break;

	default:
		status = GCFEvent::NOT_HANDLED;
		break;
	}

	return status;
}


//
// testWriteDynArray (event, port)
//
GCFEvent::TResult tGSAService::testWriteDynArray(GCFEvent& e, GCFPortInterface& p)
{
	LOG_DEBUG_STR("testWriteDynArray:" << eventName(e) << "@" << p.getName());

	GCFEvent::TResult status = GCFEvent::HANDLED;
	PVSSresult		  result;

	switch (e.signal) {
	case F_ENTRY: {
		try {
			LOG_DEBUG("===> Setting stringArr='aap,noot,mies'");
			GCFPValueArray		dpeValues;
			dpeValues.push_back(new GCFPVString("aap"));
			dpeValues.push_back(new GCFPVString("noot"));
			dpeValues.push_back(new GCFPVString("mies"));
			result = itsService->dpeSet("testDP.stringArr", 
										GCFPVDynArr(LPT_DYNSTRING, dpeValues));
		}
		catch (Exception& except) {
			LOG_INFO_STR ("Writing multiple values at once returned result: " 
							<< PVSSerrstr(result));
		}
		itsTimerPort->setTimer(1.0);
	}
	break;

	case F_TIMER:
		TRAN(tGSAService::testQuerySingle);
		break;

	default:
		status = GCFEvent::NOT_HANDLED;
		break;
	}

	return status;
}

//
// testQuerySingle (event, port)
//
GCFEvent::TResult tGSAService::testQuerySingle(GCFEvent& e, GCFPortInterface& p)
{
	LOG_DEBUG_STR("testQuerySingle:" << eventName(e) << "@" << p.getName());

	GCFEvent::TResult status = GCFEvent::HANDLED;
	PVSSresult		  result;

	switch (e.signal) {
	case F_ENTRY: {
		try {
			LOG_DEBUG("===> Taking subscribtion: FROM 'state' WHERE _DP='LOFAR_PIC_*'");
			result = itsService->dpQuerySubscribeSingle("'LOFAR_PIC_*.state'", "_DPT=\"RCU\"");
		}
		catch (Exception& except) {
			LOG_INFO_STR ("Taking subscription went wrong:" 
							<< PVSSerrstr(result));
		}
		itsTimerPort->setTimer(1.0);
	}
	break;

	case F_TIMER:
		TRAN(tGSAService::testQueryUnsubscribe);
		break;

	default:
		status = GCFEvent::NOT_HANDLED;
		break;
	}

	return status;
}


//
// testQueryUnsubscribe (event, port)
//
GCFEvent::TResult tGSAService::testQueryUnsubscribe(GCFEvent& e, GCFPortInterface& p)
{
	LOG_DEBUG_STR("testQueryUnsubscribe:" << eventName(e) << "@" << p.getName());

	GCFEvent::TResult status = GCFEvent::HANDLED;
	PVSSresult		  result;

	switch (e.signal) {
	case F_ENTRY: {
		try {
			LOG_DEBUG("===> Releasing subscription");
			result = itsService->dpQueryUnsubscribe(gQueryID);
		}
		catch (Exception& except) {
			LOG_INFO_STR ("Unsubscribing from query went wrong:" << PVSSerrstr(result));
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


  } // namespace PVSS
 } // namespace GCF
} // namespace LOFAR


using namespace LOFAR::GCF;

int main(int argc, char* argv[])
{
	TM::GCFTask::init(argc, argv);

	PVSS::tGSAService test_task("SALtest");  
	test_task.start(); // make initial transition

	TM::GCFTask::run();

	return 0;
}
