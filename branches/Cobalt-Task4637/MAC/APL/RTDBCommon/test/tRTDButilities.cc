//
//  tRTDButilities.cc: Test program to test the RTDB utilities
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
//  $Id: tDPservice.cc 10538 2007-10-03 15:04:43Z overeem $

#include <lofar_config.h>
#include <Common/LofarLogger.h>
#include <APL/RTDBCommon/RTDButilities.h>
#include "tRTDButilities.h"

namespace LOFAR {
  using namespace GCF;
  using namespace GCF::TM;
  namespace APL {
    namespace RTDBCommon {

int		gTestNr = 0;

tRTDButil::tRTDButil(const string& name) : 
	GCFTask((State)&tRTDButil::doTest, name),
	itsTimerPort(0)
{
	LOG_DEBUG_STR("tRTDButil(" << name << ")");

	itsTimerPort = new GCFTimerPort(*this, "timerPort");
	ASSERTSTR(itsTimerPort, "Can't allocate GCFTimerPort");

	registerProtocol(F_FSM_PROTOCOL, F_FSM_PROTOCOL_STRINGS);
}

//
// destructor
//
tRTDButil::~tRTDButil()
{
	LOG_DEBUG("Deleting tRTDButil");
}

//
// doTest (event, port)
//
GCFEvent::TResult tRTDButil::doTest(GCFEvent& e, GCFPortInterface& /*p*/)
{
	LOG_DEBUG_STR("doTest:" << eventName(e));
	GCFEvent::TResult status = GCFEvent::HANDLED;

	switch (e.signal) {
	case F_INIT: 
		break;

	case F_ENTRY: {
		// Good situation
		gTestNr = 1;
		setObjectState("tRTDButil", "testObject", RTDB_OBJ_STATE_TEST);
		itsTimerPort->setTimer(3.0); // max time for this test.
	}
	break;

	case F_TIMER:
		if (gTestNr == 1) {
			// wrong statenr
			setObjectState("tRTDButil", "testObject", 25);
			gTestNr++;
			itsTimerPort->setTimer(3.0);
		}
		else if (gTestNr == 2) {
			// wrong DP
			setObjectState("tRTDButil", "iserNiet", RTDB_OBJ_STATE_TEST);
			gTestNr++;
			itsTimerPort->setTimer(3.0);
		}
		else {
			GCFScheduler::instance()->stop();
		}
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

	tRTDButil test_task("UtilTest");  
	test_task.start(); // make initial transition

	TM::GCFScheduler::instance()->run();

	return 0;
}
