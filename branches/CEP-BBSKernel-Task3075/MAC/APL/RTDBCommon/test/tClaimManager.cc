//
//  tClaimManager.cc: Test program to test the resolve Observation mechanism
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
#include <APL/RTDBCommon/ClaimMgrTask.h>
#include "CM_Protocol.ph"
#include "tClaimManager.h"

namespace LOFAR {
  using namespace GCF;
  using namespace GCF::TM;
  namespace APL {
    namespace RTDBCommon {

int		gTestNr = 0;

MgrTest::MgrTest(const string& name) : 
	GCFTask((State)&MgrTest::doSingleTest, name),
	itsTimerPort(0)
{
	LOG_DEBUG_STR("=@=@= MgrTest(" << name << ")");

	itsTimerPort = new GCFTimerPort(*this, "timerPort");
	ASSERTSTR(itsTimerPort, "=@=@= Can't allocate GCFTimerPort");
	itsMsgPort   = new GCFITCPort  (*this, *this, "ITCPort", GCFPortInterface::SAP, CM_PROTOCOL);
	ASSERTSTR(itsMsgPort, "=@=@= Can't allocate GCFITCPort");

	registerProtocol(F_FSM_PROTOCOL, F_FSM_PROTOCOL_STRINGS);
	registerProtocol(CM_PROTOCOL, 	 CM_PROTOCOL_STRINGS);
}

//
// destructor
//
MgrTest::~MgrTest()
{
	LOG_DEBUG("=@=@= Deleting MgrTest");
}

//
// doSingleTest (event, port)
//
GCFEvent::TResult MgrTest::doSingleTest(GCFEvent& event, GCFPortInterface& port)
{
	LOG_DEBUG_STR ("=@=@= doSingleTest:" << eventName(event) << "@" << port.getName());
	GCFEvent::TResult status = GCFEvent::HANDLED;

	switch (event.signal) {
	case F_INIT: 
		LOG_INFO_STR("=@=@= Creating a ClaimManager");
		itsClaimMgrTask = ClaimMgrTask::instance();
		ASSERTSTR(itsClaimMgrTask, "Can't construct a claimMgrTask");

		LOG_INFO_STR("=@=@= Calling claimManager for 'Observation7'");
		itsClaimMgrTask->claimObject("Observation", "LOFAR_ObsSW_Observation7", *itsMsgPort);
		itsTimerPort->setTimer(10.0);
		break;

	case F_ENTRY:
	break;

	case CM_CLAIM_RESULT: {
		CMClaimResultEvent	cmEvent(event);
		LOG_INFO_STR("=@=@= ObjectType: " << cmEvent.typeName);
		LOG_INFO_STR("=@=@= NameInAppl: " << cmEvent.nameInAppl);
		LOG_INFO_STR("=@=@= DBaseName : " << cmEvent.DPname);
		itsTimerPort->cancelAllTimers();
		TRAN(MgrTest::doMultipleTest);
	}
	break;

	case F_TIMER:
		LOG_FATAL("=@=@= Did not receive an answer from the claimManager, aborting program!");
		GCFScheduler::instance()->stop();
		break;

	default:
		status = GCFEvent::NOT_HANDLED;
		break;
	}

	return status;
}
//
// doMultipleTest (event, port)
//
GCFEvent::TResult MgrTest::doMultipleTest(GCFEvent& event, GCFPortInterface& port)
{
	LOG_DEBUG_STR ("=@=@= doMultipleTest:" << eventName(event) << "@" << port.getName());
	GCFEvent::TResult status = GCFEvent::HANDLED;

	switch (event.signal) {
	case F_ENTRY:
		LOG_INFO_STR("=@=@= Calling claimManager for 'Observation8'");
		itsClaimMgrTask->claimObject("Observation", "LOFAR_ObsSW_Observation8", *itsMsgPort);
		LOG_INFO_STR("=@=@= Calling claimManager for 'Observation9'");
		itsClaimMgrTask->claimObject("Observation", "LOFAR_ObsSW_Observation9", *itsMsgPort);
		LOG_INFO_STR("=@=@= Calling claimManager for 'Observation10'");
		itsClaimMgrTask->claimObject("Observation", "LOFAR_ObsSW_Observation10", *itsMsgPort);
		LOG_INFO_STR("=@=@= Calling claimManager for 'Observation11'");
		itsClaimMgrTask->claimObject("Observation", "LOFAR_ObsSW_Observation11", *itsMsgPort);
		LOG_INFO_STR("=@=@= Calling claimManager for 'Observation12'");
		itsClaimMgrTask->claimObject("Observation", "LOFAR_ObsSW_Observation12", *itsMsgPort);
		itsAnswers2Xpect=5;
		itsTimerPort->setTimer(10.0);
	break;

	case CM_CLAIM_RESULT: {
		CMClaimResultEvent	cmEvent(event);
		LOG_INFO_STR("=@=@= ObjectType: " << cmEvent.typeName);
		LOG_INFO_STR("=@=@= NameInAppl: " << cmEvent.nameInAppl);
		LOG_INFO_STR("=@=@= DBaseName : " << cmEvent.DPname);
		if (--itsAnswers2Xpect == 0) {
			LOG_INFO("Received all 5 answers, successful end of program");
			GCFScheduler::instance()->stop();
		}
	}
	break;

	case F_TIMER:
		LOG_FATAL_STR("Still waiting for " << itsAnswers2Xpect << " answers, aborting program");
		GCFScheduler::instance()->stop();
		break;

	default:
		status = GCFEvent::NOT_HANDLED;
		break;
	}

	return status;
}


    } // namepsace RTDBCommon
  } // namspace APL
} // namespace LOFAR

using namespace LOFAR;
using namespace LOFAR::GCF;
using namespace LOFAR::APL::RTDBCommon;

int main(int argc, char* argv[])
{
	TM::GCFScheduler::instance()->init(argc, argv);

	MgrTest test_task("UtilTest");  
	test_task.start(); // make initial transition

	TM::GCFScheduler::instance()->run();

	return 0;
}
