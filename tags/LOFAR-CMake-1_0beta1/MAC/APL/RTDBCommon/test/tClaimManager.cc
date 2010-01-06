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
	GCFTask((State)&MgrTest::doTest, name),
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
// doTest (event, port)
//
GCFEvent::TResult MgrTest::doTest(GCFEvent& event, GCFPortInterface& port)
{
	LOG_DEBUG_STR ("=@=@= doTest:" << eventName(event) << "@" << port.getName());
	GCFEvent::TResult status = GCFEvent::HANDLED;

	switch (event.signal) {
	case F_INIT: 
		LOG_DEBUG_STR("=@=@= Creating a ClaimManager");
		itsClaimMgrTask = ClaimMgrTask::instance();
		ASSERTSTR(itsClaimMgrTask, "Can't construct a claimMgrTask");

		LOG_DEBUG_STR("=@=@= Calling claimManager for 'Observation7'");
		itsClaimMgrTask->claimObject("Observation", "LOFAR_ObsSW_Observation7", *itsMsgPort);
		itsTimerPort->setTimer(10.0);
		break;

	case F_ENTRY:
	break;

	case CM_CLAIM_RESULT: {
		CMClaimResultEvent	cmEvent(event);
		LOG_DEBUG_STR("=@=@= ObjectType: " << cmEvent.typeName);
		LOG_DEBUG_STR("=@=@= NameInAppl: " << cmEvent.nameInAppl);
		LOG_DEBUG_STR("=@=@= DBaseName : " << cmEvent.DPname);
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
