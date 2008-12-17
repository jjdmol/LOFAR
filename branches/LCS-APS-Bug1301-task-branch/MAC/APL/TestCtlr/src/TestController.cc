//#  TestController.cc: Dummy task for debugging parentControl
//#
//#  Copyright (C) 2006
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
//#  $Id$
//#
#include <lofar_config.h>
#include <Common/LofarLogger.h>
#include <Common/LofarLocators.h>

#include <Common/ParameterSet.h>
#include <MACIO/MACServiceInfo.h>
#include <APL/APLCommon/APL_Defines.h>
#include <APL/APLCommon/Controller_Protocol.ph>
#include <APL/APLCommon/ControllerDefines.h>
#include <APL/APLCommon/StationInfo.h>

#include "TestController.h"

using namespace LOFAR::GCF::TM;

namespace LOFAR {
	using namespace APLCommon;
	namespace Test {
	
//
// TestController()
//
TestController::TestController(const string&	cntlrName) :
	GCFTask 			((State)&TestController::initial_state,cntlrName),
	itsParentControl	(0),
	itsParentPort		(0),
	itsTimerPort		(0)
{
	LOG_TRACE_OBJ_STR (cntlrName << " construction");

	// attach to parent control task
	itsParentControl = ParentControl::instance();
	itsParentPort = new GCFITCPort (*this, *itsParentControl, "ParentITCport", 
									GCFPortInterface::SAP, CONTROLLER_PROTOCOL);
	ASSERTSTR(itsParentPort, "Cannot allocate ITCport for Parentcontrol");
	itsParentPort->open();		// will result in F_CONNECTED

	// need port for timers.
	itsTimerPort = new GCFTimerPort(*this, "TimerPort");

	// for debugging purposes
	registerProtocol (CONTROLLER_PROTOCOL, CONTROLLER_PROTOCOL_STRINGS);
}


//
// ~TestController()
//
TestController::~TestController()
{
	LOG_TRACE_OBJ_STR (getName() << " destruction");
}


//
// initial_state(event, port)
//
// Setup all connections.
//
GCFEvent::TResult TestController::initial_state(GCFEvent& event, 
													GCFPortInterface& port)
{
	LOG_DEBUG_STR ("initial:" << eventName(event) << "@" << port.getName());

	switch (event.signal) {
	case F_ENTRY: {
		// Start ParentControl task
		LOG_DEBUG ("Enabling ParentControl task");
		itsParentPort = itsParentControl->registerTask(this);
	}
	break;

	case F_CONNECTED:
		TRAN(TestController::active_state);				// go to next state.
		break;

	default:
		LOG_DEBUG_STR ("initial, default");
		break;
	}    

	return (GCFEvent::HANDLED);
}


//
// active_state(event, port)
//
// Normal operation state. 
//
GCFEvent::TResult TestController::active_state(GCFEvent& event, GCFPortInterface& port)
{
	LOG_DEBUG_STR ("active:" << eventName(event) << "@" << port.getName());

	switch (event.signal) {
	case F_TIMER:  {
		switch (itsState) {
		case CTState::CONNECT: {
			CONTROLConnectedEvent	answer;
			answer.cntlrName = itsController;
			answer.result = true;
			LOG_DEBUG_STR("Sending CONNECTED event");
			itsParentPort->send(answer);
		}
		break;

		case CTState::RESYNC: {
		}
		break;

		case CTState::SCHEDULE: {
			CONTROLScheduledEvent	answer;
			answer.cntlrName = itsController;
			answer.result = 0;
			LOG_DEBUG_STR("Sending SCHEDULED event");
			itsParentPort->send(answer);
		}
		break;

		case CTState::CLAIM: {
			CONTROLClaimedEvent	answer;
			answer.cntlrName = itsController;
			answer.result = 0;
			LOG_DEBUG_STR("Sending CLAIMED event");
			itsParentPort->send(answer);
		}
		break;

		case CTState::PREPARE: {
			CONTROLPreparedEvent	answer;
			answer.cntlrName = itsController;
			answer.result = 0;
			LOG_DEBUG_STR("Sending PREPARED event");
			itsParentPort->send(answer);
		}
		break;

		case CTState::RESUME: {
			CONTROLResumedEvent	answer;
			answer.cntlrName = itsController;
			answer.result = 0;
			LOG_DEBUG_STR("Sending RESUMED event");
			itsParentPort->send(answer);
		}
		break;

		case CTState::SUSPEND: {
			CONTROLSuspendedEvent	answer;
			answer.cntlrName = itsController;
			answer.result = 0;
			LOG_DEBUG_STR("Sending SUSPENDED event");
			itsParentPort->send(answer);
		}
		break;

		case CTState::RELEASE: {
			CONTROLReleasedEvent	answer;
			answer.cntlrName = itsController;
			answer.result = 0;
			LOG_DEBUG_STR("Sending RELEASED event");
			itsParentPort->send(answer);
			LOG_DEBUG_STR("Will send QUITED event over 6 seconds");
			itsState      = CTState::QUIT;
			itsTimerPort->setTimer(6.0);
		}
		break;

		case CTState::QUIT: {
			CONTROLQuitedEvent	msg;
			msg.cntlrName = itsController;
			msg.treeID    = 7;
			msg.errorMsg  = "Normal Termination";
			msg.result    = 0;
			LOG_DEBUG_STR("Sending QUITED event");
			itsParentPort->send(msg);
		}
		break;

		} // switch state
	}
	break;

	// -------------------- EVENTS RECEIVED FROM PARENT CONTROL --------------------
	case CONTROL_CONNECT: {
		CONTROLConnectEvent		msg(event);
		itsController = msg.cntlrName;
		itsState      = CTState::CONNECT;
		itsTimerPort->setTimer(5.0);
	}
	break;

	case CONTROL_RESYNC: {

	}
	break;

	case CONTROL_SCHEDULE: {
		CONTROLScheduleEvent		msg(event);
		itsController = msg.cntlrName;
		itsState      = CTState::SCHEDULE;
		itsTimerPort->setTimer(5.0);
	}
	break;

	case CONTROL_CLAIM: {
		CONTROLClaimEvent		msg(event);
		itsController = msg.cntlrName;
		itsState      = CTState::CLAIM;
		itsTimerPort->setTimer(5.0);
	}
	break;

	case CONTROL_PREPARE: {
		CONTROLPrepareEvent		msg(event);
		itsController = msg.cntlrName;
		itsState      = CTState::PREPARE;
		itsTimerPort->setTimer(5.0);
	}
	break;

	case CONTROL_RESUME: {
		CONTROLResumeEvent		msg(event);
		itsController = msg.cntlrName;
		itsState      = CTState::RESUME;
		itsTimerPort->setTimer(5.0);
	}
	break;

	case CONTROL_SUSPEND: {
		CONTROLSuspendEvent		msg(event);
		itsController = msg.cntlrName;
		itsState      = CTState::SUSPEND;
		itsTimerPort->setTimer(5.0);
	}
	break;

	case CONTROL_RELEASE: {
		CONTROLReleaseEvent		msg(event);
		itsController = msg.cntlrName;
		itsState      = CTState::RELEASE;
		itsTimerPort->setTimer(5.0);
	}
	break;

	case CONTROL_QUIT: {
		CONTROLQuitEvent		msg(event);
		itsController = msg.cntlrName;
		itsState      = CTState::QUIT;
		itsTimerPort->setTimer(5.0);
	}
	break;
		
	default:
		LOG_DEBUG("active_state, default");
		break;
	}

	return (GCFEvent::HANDLED);
}


}; // Test
}; // LOFAR
