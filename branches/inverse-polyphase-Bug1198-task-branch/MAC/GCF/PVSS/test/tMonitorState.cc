//
//  tMonitorState.cc: Test prog for manual testing queries
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
#include "tMonitorState.h"
#include "Response.h"

namespace LOFAR {
  namespace GCF {
  using namespace TM;
  namespace PVSS {


tMonitorState::tMonitorState(const string& name) : 
	GCFTask((State)&tMonitorState::initial, name), 
	itsService(0),
	itsResponse(0),
	itsTimerPort(0)
{
	registerProtocol(F_FSM_PROTOCOL, F_FSM_PROTOCOL_STRINGS);
	itsResponse  = new Response;
	itsTimerPort = new GCFTimerPort(*this, "timerPort");
}

tMonitorState::~tMonitorState()
{
	LOG_DEBUG("Deleting tMonitorState");
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
GCFEvent::TResult tMonitorState::initial(GCFEvent& e, GCFPortInterface& /*p*/)
{
	LOG_DEBUG_STR("initial:" << eventName(e));
	GCFEvent::TResult status = GCFEvent::HANDLED;

	switch (e.signal) {
	case F_ENTRY:
		break;

	case F_INIT:  {
		LOG_DEBUG("Creating a Service Class");
		itsService = new PVSSservice(itsResponse);
		PVSSresult		  result;

		try {
			LOG_DEBUG("Taking subscribtion: FROM 'state' WHERE _DP='LOFAR_PIC_*'");
//			result = itsService->dpQuerySubscribeAll("'LOFAR_PIC_*.state' WHERE _DPT=\"RCU\" AND '_online.._online_bad' = \"FALSE\" ","");
			result = itsService->dpQuerySubscribeSingle("'LOFAR_PIC_*.status.state'", "_DPT=\"RCU\"");
		}
		catch (Exception& except) {
			LOG_INFO_STR ("Taking subscription went wrong:" << PVSSerrstr(result));
		}
		itsTimerPort->setTimer(1.0);
	}
	break;

	case F_TIMER:
		TRAN(tMonitorState::watching);
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
GCFEvent::TResult tMonitorState::final(GCFEvent& e, GCFPortInterface& /*p*/)
{
	LOG_DEBUG_STR("final:" << eventName(e));
	GCFEvent::TResult status = GCFEvent::HANDLED;

	switch (e.signal) {
	case F_ENTRY:
		GCFScheduler::instance()->stop();
		break;

	default:
		status = GCFEvent::NOT_HANDLED;
		break;
	}

	return status;
}

//
// watching (event, port)
//
GCFEvent::TResult tMonitorState::watching(GCFEvent& e, GCFPortInterface& p)
{
	LOG_DEBUG_STR("watching:" << eventName(e) << "@" << p.getName());

	GCFEvent::TResult status = GCFEvent::HANDLED;

	switch (e.signal) {
	case F_ENTRY: {
	}
	break;

	case F_TIMER:
		TRAN(tMonitorState::final);
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

	PVSS::tMonitorState test_task("QueryTest");  
	test_task.start(); // make initial transition

	TM::GCFScheduler::instance()->run();

	return 0;
}
