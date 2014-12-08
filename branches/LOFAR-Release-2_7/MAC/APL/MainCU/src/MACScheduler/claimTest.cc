//#  claimtest.cc: tool for manually testing the obsClaimer task.
//#
//#  Copyright (C) 2004-2012
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//#  This program is free software; you can redistribute it and/or modify
//#  it under the terms of the GNU General Public License as published by
//#  the Free Software Foundation; either version 2 of the License, or
//#  (at your option) any later version.
//#
//#  This program is distributed in the hope that it will be useful,
//#  but WITHOUT ANY WARRANTY; without even the implied warranty of
//#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//#  GNU General Public License for more details.
//#
//#  You should have received a copy of the GNU General Public License
//#  along with this program; if not, write to the Free Software
//#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#
//#  $Id: claimTest.cc 27513 2013-11-26 14:49:20Z overeem $
#include <lofar_config.h>
#include <Common/LofarLogger.h>
#include <Common/SystemUtil.h>
#include <Common/StringUtil.h>

#include <GCF/TM/GCF_Protocols.h>
#include <MACIO/MACServiceInfo.h>
#include <GCF/PVSS/GCF_PVTypes.h>
#include <APL/APLCommon/APL_Defines.h>
#include <APL/APLCommon/ControllerDefines.h>
#include <GCF/RTDB/DP_Protocol.ph>
#include <APL/RTDBCommon/CM_Protocol.ph>
#include <signal.h>

#include "claimTest.h"

using namespace LOFAR::GCF::PVSS;
using namespace LOFAR::GCF::TM;
using namespace LOFAR::GCF::RTDB;
using namespace std;

namespace LOFAR {
	using namespace APLCommon;
	namespace MainCU {

// Global vars
bool	gClaimIt;
string	gObsName;

//
// claimTest()
//
claimTest::claimTest() :
	GCFTask 			((State)&claimTest::initial_state,string("claimTest")),
	itsClaimerTask		(0),
	itsClaimerPort		(0),
	itsTimerPort		(0)
{
	// create an PVSSprepare Task
	itsClaimerTask = new ObsClaimer(this);
	ASSERTSTR(itsClaimerTask, "Cannot construct a ObsClaimerTask");
	itsClaimerPort = new GCFITCPort (*this, *itsClaimerTask, "ObsClaimerPort", GCFPortInterface::SAP, CM_PROTOCOL);

	// need port for timers
	itsTimerPort = new GCFTimerPort(*this, "Timerport");
}


//
// ~claimTest()
//
claimTest::~claimTest()
{
}

//
// initial_state(event, port)
//
// Setup all connections.
//
GCFEvent::TResult claimTest::initial_state(GCFEvent& event, GCFPortInterface& /*port*/)
{
	LOG_DEBUG_STR ("initial_state:" << eventName(event));

	GCFEvent::TResult status = GCFEvent::HANDLED;
  
	switch (event.signal) {
    case F_INIT:
   		break;

	case F_ENTRY: {
		itsTimerPort->setTimer(5.0);
		if (gClaimIt) {
			itsClaimerTask->prepareObservation(gObsName);
		}
		else {
			itsClaimerTask->freeObservation(gObsName);
		}
	} break;

	case CM_CLAIM_RESULT: {
		// some observation was claimed by the claimMgr. Update our prepare_list.
		CMClaimResultEvent	cmEvent(event);
		ltrim(cmEvent.nameInAppl, "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_");
		int		obsID = atoi(cmEvent.nameInAppl.c_str());
		// claim was successful, update admin
		cout << "Observation " << obsID << " is mapped to " << cmEvent.DPname << endl;
		GCFScheduler::instance()->stop();
	} break;

	case F_TIMER:
		cout << "TIMEOUT" << endl;
		GCFScheduler::instance()->stop();
		break;

	default:
		LOG_DEBUG("claimTest::active, default");
		status = GCFEvent::NOT_HANDLED;
		break;
	}

	return (status);
}

};   // namespace MainCU
}; // namespace LOFAR


using namespace LOFAR::GCF::TM;
using namespace LOFAR::MainCU;
using namespace LOFAR::APLCommon;
using namespace LOFAR;

int main(int argc, char* argv[])
{
	if (argc != 3) {
		cout << "Syntax: claimTest action name" << endl;
		cout << "  action: claim | free" << endl;
		cout << "  name:   observationname" << endl;
		return(-1);
	}
	cout << "######" << argv[1] << "#######" << endl;
	gClaimIt = (strcmp(argv[1],"claim")==0);
	gObsName = argv[2];

	GCFScheduler::instance()->init(argc, argv, "claimTest");

	claimTest	cT;
	cT.start(); // make initial transition
  
	GCFScheduler::instance()->run();

	return 0;
}

