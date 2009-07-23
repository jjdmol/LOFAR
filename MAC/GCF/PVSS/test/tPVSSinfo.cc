//
//  tPVSSinfo.cc: Test prog for manual testing queries
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
#include <GCF/PVSS/GCF_PVTypes.h>
#include <GCF/PVSS/PVSSinfo.h>
#include "tPVSSservice.h"
#include "tPVSSinfo.h"
#include "Response.h"

#define TEST_PROP_NAME(propname) \
	cout << "name (" propname ") = " << (PVSSinfo::isValidPropName(propname) ? "OK" : "INVALID") << endl;

namespace LOFAR {
  namespace GCF {
  using namespace TM;
  namespace PVSS {


tPVSSinfo::tPVSSinfo(const string& name) : 
	GCFTask((State)&tPVSSinfo::initial, name), 
	itsService(0),
	itsResponse(0),
	itsTimerPort(0)
{
	registerProtocol(F_FSM_PROTOCOL, F_FSM_PROTOCOL_STRINGS);
	itsResponse  = new Response;
	itsTimerPort = new GCFTimerPort(*this, "timerPort");
}

tPVSSinfo::~tPVSSinfo()
{
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
GCFEvent::TResult tPVSSinfo::initial(GCFEvent& e, GCFPortInterface& /*p*/)
{
	LOG_DEBUG_STR("initial:" << eventName(e));
	GCFEvent::TResult status = GCFEvent::HANDLED;

	switch (e.signal) {
	case F_ENTRY:
		break;

	case F_INIT:  {
		itsService = new PVSSservice(itsResponse);
		PVSSresult		  result;

		try {
			cout << "getLocalSystemId  : " << (int)PVSSinfo::getLocalSystemId() << endl;
			cout << "getLocalSystemName: " << PVSSinfo::getLocalSystemName() << endl;
			cout << "getSystemName(Id) : " << PVSSinfo::getSystemName(PVSSinfo::getLocalSystemId()) << endl;
			cout << "getSystemId(Name) : " << (int)PVSSinfo::getSysId(PVSSinfo::getLocalSystemName()) << endl;
			cout << "getProjectName    : " << PVSSinfo::getProjectName() << endl;
			cout << "getOwnManNum      : " << (int)PVSSinfo::getOwnManNum() << endl << endl;
			TEST_PROP_NAME("MCU001:LOFAR_XYZ.abc");
			TEST_PROP_NAME("LOFAR_XYZ.abc");
			TEST_PROP_NAME("LOFAR_XYZ.__abc");
			TEST_PROP_NAME(".LOFAR_XYZ.abc");
			TEST_PROP_NAME("LOFAR_XYZ.abc.");
			TEST_PROP_NAME("LOFAR__XYZ.abc");
			TEST_PROP_NAME("LOFAR_XYZ!abc");
		}
		catch (Exception& except) {
			cout << "One of the PVSS-calls went wrong" << endl;
		}
		GCFScheduler::instance()->stop();
	}
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
	TM::GCFScheduler::instance()->init(argc, argv);

	PVSS::tPVSSinfo test_task("infoTest");  
	test_task.start(); // make initial transition

	TM::GCFScheduler::instance()->run();

	return 0;
}
