//#  ActiveObs.cc: Implements a statemachine for an active Observation
//#
//#  Copyright (C) 2002-2004
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

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

//# Includes
#include <Common/LofarLogger.h>
#include <GCF/TM/GCF_Protocols.h>
#include <GCF/Utils.h>
#include <APL/APLCommon/ChildControl.h>
#include <APL/APLCommon/ControllerDefines.h>
#include <APL/APLCommon/Controller_Protocol.ph>
#include "ActiveObs.h"

namespace LOFAR {
	using ACC::APS::ParameterSet;
	using namespace APLCommon;
	using namespace GCF::TM;
	using namespace GCF::Common;
	namespace StationCU {

//
// ActiveObs(ParameterSet*	thePS, State	initial)
//
ActiveObs::ActiveObs(const string&		name,
					 State				initial,
					 ParameterSet*		thePS) :
	GCFFsm(initial),
	itsName(name),
	itsInstanceNr(getInstanceNr(name)),
	itsObsPar(APLCommon::Observation(thePS)),
	itsBeamCntlrReady(false),
	itsCalCntlrReady(false),
	itsBeamCntlrName(controllerName(CNTLRTYPE_BEAMCTRL, 
									itsInstanceNr, itsObsPar.treeID)),
	itsCalCntlrName(controllerName(CNTLRTYPE_CALIBRATIONCTRL, 
								   itsInstanceNr, itsObsPar.treeID)),
	itsReadyFlag(false)
{
}

//
// ~ActiveObs()
//
ActiveObs::~ActiveObs()
{
}

//
// initial(event, port)
// Connect to childControllers BeamControl and CalControl
//
GCFEvent::TResult	ActiveObs::initial(GCFEvent&	event, GCFPortInterface&	/*port*/)
{
	LOG_DEBUG(formatString("%s:initial - %04X", itsName.c_str(), event.signal));

	switch (event.signal) {
	case F_ENTRY: 
		LOG_DEBUG_STR("Started statemachine for observation " << itsName);
		break;

	case F_INIT: {
		LOG_DEBUG_STR("Starting controller " << itsCalCntlrName);
		ChildControl::instance()->startChild(CNTLRTYPE_CALIBRATIONCTRL,
											 itsObsPar.treeID,
											 itsInstanceNr,
											 myHostname(true));
		// Results in CONTROL_CONNECT in StationControl Task.
											
		LOG_DEBUG_STR("Starting controller " << itsBeamCntlrName);
		ChildControl::instance()->startChild(CNTLRTYPE_BEAMCTRL,
											 itsObsPar.treeID,
											 itsInstanceNr,
											 myHostname(true));
		// Results in CONTROL_CONNECT in StationControl Task.
	}
	break;

	case CONTROL_CONNECTED: {
		CONTROLConnectedEvent		msg(event);
		if (msg.cntlrName == itsCalCntlrName) {
			itsCalCntlrReady = true;
			LOG_DEBUG("Connected to CalibrationController");
		}
		else if (msg.cntlrName == itsBeamCntlrName) {
			itsBeamCntlrReady = true;
			LOG_DEBUG("Connected to BeamController");
		}
		else {
			ASSERTSTR(false, "Received Connect event for wrong controller: "
																	<< msg.cntlrName);
		}
		if (itsBeamCntlrReady && itsCalCntlrReady) {
			LOG_DEBUG_STR("Connected to both controllers, going to connected state");
			TRAN(ActiveObs::connected);
		}
	}
	break;

	case CONTROL_QUIT:
		TRAN(ActiveObs::stopping);
		break;

	default:
		LOG_DEBUG_STR(itsName << ":default(" << F_EVT_PROTOCOL(event) << "," <<
															F_EVT_SIGNAL(event) << ")");
		return(GCFEvent::NOT_HANDLED);
		break;
	}

	return (GCFEvent::HANDLED);
}

//
// connected(event, port)
// Wait for CLAIM event
//
GCFEvent::TResult	ActiveObs::connected(GCFEvent&	event, GCFPortInterface&	/*port*/)
{
	LOG_DEBUG_STR(itsName << ":connected");

	switch (event.signal) {
	case F_ENTRY: 
		itsBeamCntlrReady = false;
		itsCalCntlrReady  = false;
		LOG_DEBUG_STR(itsName << ": in 'connected-mode', waiting for CLAIM event");
		break;

	case CONTROL_CLAIM: {
		// The stationControllerTask should have set the stationclock by now.
		// Activate the Calibration Controller
		LOG_DEBUG_STR("Asking " << itsCalCntlrName << " to connect to CalServer");
		ChildControl::instance()->
				requestState(CTState::CLAIMED, itsCalCntlrName, 0, CNTLRTYPE_NO_TYPE);
		LOG_DEBUG_STR("Asking " << itsBeamCntlrName << " to connect to BeamServer");
		ChildControl::instance()->
				requestState(CTState::CLAIMED, itsBeamCntlrName, 0, CNTLRTYPE_NO_TYPE);
		// will result in CONTROL_CLAIMED
	}
	break;

	case CONTROL_CLAIMED: {
		CONTROLClaimedEvent		msg(event);
		if (msg.cntlrName == itsBeamCntlrName) {
			LOG_DEBUG_STR("BeamController is connected to BeamServer");
			itsBeamCntlrReady = true;
		}
		else if (msg.cntlrName == itsCalCntlrName) {
			LOG_DEBUG("CalController is connected to CalServer");
			itsCalCntlrReady = true;
		}
		else {
			ASSERTSTR(false, "Received claimed event of unknown controller: " << 
							 msg.cntlrName);
		}
		if (itsBeamCntlrReady && itsCalCntlrReady) {
			LOG_DEBUG("Both controllers are ready, going to standby mode");
			TRAN(ActiveObs::standby);
		}
	}
	break;

	case CONTROL_QUIT:
		TRAN(ActiveObs::stopping);
		break;

	case F_INIT: 
		break;

	default:
		LOG_DEBUG_STR(itsName << ":default(" << F_EVT_PROTOCOL(event) << "," <<
															F_EVT_SIGNAL(event) << ")");
		return(GCFEvent::NOT_HANDLED);
		break;
	}

	return (GCFEvent::HANDLED);
}

//
// standby(event, port)
// Wait for PREPARE(?) event to set the beam
//
GCFEvent::TResult	ActiveObs::standby(GCFEvent&	event, GCFPortInterface&	/*port*/)
{
	LOG_DEBUG_STR(itsName << ":standby");

	switch (event.signal) {
	case F_ENTRY: 
		itsBeamCntlrReady = false;
		itsCalCntlrReady  = false;
		LOG_DEBUG_STR(itsName << ": in 'standby-mode', wating for PREPARE event");
		break;

	case CONTROL_PREPARE: {
		// Activate the BeamController
		LOG_DEBUG_STR("Asking " << itsCalCntlrName << " to calibrate the subarray");
		ChildControl::instance()->
				requestState(CTState::PREPARED, itsCalCntlrName, 0, CNTLRTYPE_NO_TYPE);
		// will result in CONTROL_PREPARED
	}
	break;

	case CONTROL_PREPARED: {
		CONTROLPreparedEvent		msg(event);
		if (msg.cntlrName == itsCalCntlrName) {
			if (msg.result != CT_RESULT_NO_ERROR) {
				LOG_ERROR_STR("Calibration of subarray FAILED with error " << msg.result);
				break;
			}
			LOG_DEBUG("Subarray is calibrated, asking BeamCtlr to start the beam");
			itsCalCntlrReady = true;
			ChildControl::instance()->requestState(CTState::PREPARED, 
											itsBeamCntlrName, 0, CNTLRTYPE_NO_TYPE);
			// will result in another CONTROL_PREPARED
		}
		else if (msg.cntlrName == itsBeamCntlrName) {
			if (msg.result != CT_RESULT_NO_ERROR) {
				LOG_ERROR_STR("Start op beam failed with error " << msg.result);	
				break;
			}
			LOG_DEBUG_STR("BeamController has started the beam");
			itsBeamCntlrReady = true;
		}
		else {
			ASSERTSTR(false, "Received claimed event of unknown controller: " << 
							 msg.cntlrName);
		}

		if (itsBeamCntlrReady && itsCalCntlrReady) {
			LOG_DEBUG("Both controllers are ready, going to operational mode");
			TRAN(ActiveObs::operational);
		}
	}
	break;

	case CONTROL_QUIT:
		TRAN(ActiveObs::stopping);
		break;

	case F_INIT: 
		break;

	default:
		LOG_DEBUG_STR(itsName << ":default(" << F_EVT_PROTOCOL(event) << "," <<
															F_EVT_SIGNAL(event) << ")");
		return(GCFEvent::NOT_HANDLED);
		break;
	}

	return (GCFEvent::HANDLED);
}

//
// operational(event, port)
// Wait for RELEASE(?) event to stop the beam
//
GCFEvent::TResult	ActiveObs::operational(GCFEvent&	event, GCFPortInterface&	/*port*/)
{
	LOG_DEBUG_STR(itsName << ":operational");

	switch (event.signal) {
	case F_ENTRY: 
		itsBeamCntlrReady = false;
		itsCalCntlrReady  = false;
		LOG_DEBUG_STR(itsName << ": in 'operational-mode' until RELEASE event");
		break;

	case CONTROL_SUSPEND: {
		// pass event through to controllers
		LOG_DEBUG("Passing SUSPEND event to childs");
		itsBeamCntlrReady = false;
		itsCalCntlrReady  = false;
		ChildControl::instance()->
				requestState(CTState::SUSPENDED, itsBeamCntlrName, 0, CNTLRTYPE_NO_TYPE);
		ChildControl::instance()->
				requestState(CTState::SUSPENDED, itsCalCntlrName, 0, CNTLRTYPE_NO_TYPE);
		// will result in CONTROL_RELEASED
	}
	break;
	
	case CONTROL_SUSPENDED:
		break;

	case CONTROL_RELEASE: {
		// release beam at the BeamController
		LOG_DEBUG_STR("Asking " << itsBeamCntlrName << " to stop the beam");
		ChildControl::instance()->
				requestState(CTState::RELEASED, itsBeamCntlrName, 0, CNTLRTYPE_NO_TYPE);
		// will result in CONTROL_RELEASED
	}
	break;

	case CONTROL_RELEASED: {
		CONTROLReleaseEvent		msg(event);
		ASSERTSTR(msg.cntlrName == itsBeamCntlrName, 
					"Received released event of unknown controller: " << msg.cntlrName);
		LOG_DEBUG_STR("Beam is stopped, going to standby mode");
		TRAN(ActiveObs::standby);
	}
	break;

	case CONTROL_QUIT:
		TRAN(ActiveObs::stopping);
		break;

	case F_INIT: 
		break;

	default:
		LOG_DEBUG_STR(itsName << ":default(" << F_EVT_PROTOCOL(event) << "," <<
															F_EVT_SIGNAL(event) << ")");
		return(GCFEvent::NOT_HANDLED);
		break;
	}

	return (GCFEvent::HANDLED);
}

//
// stopping(event, port)
// Wait for RELEASE(?) event to stop the beam
//
GCFEvent::TResult	ActiveObs::stopping(GCFEvent&	event, GCFPortInterface&	/*port*/)
{
	LOG_DEBUG_STR(itsName << ":stopping");

	switch (event.signal) {
	case F_ENTRY: 
		itsBeamCntlrReady = false;
		itsCalCntlrReady  = false;
		LOG_DEBUG_STR(itsName << ": in 'stopping-mode' until controllers are down");
		break;

	case CONTROL_QUIT: {
		// release beam at the BeamController
		LOG_DEBUG_STR("Asking " << itsBeamCntlrName << " to quit");
		ChildControl::instance()->
				requestState(CTState::FINISHED, itsBeamCntlrName, 0, CNTLRTYPE_NO_TYPE);
		// will result in CONTROL_RELEASED

		LOG_DEBUG_STR("Asking " << itsCalCntlrName << " to quit");
		ChildControl::instance()->
				requestState(CTState::FINISHED, itsCalCntlrName, 0, CNTLRTYPE_NO_TYPE);
		// will result in CONTROL_RELEASED
	}
	break;

	case CONTROL_FINISHED: {
		CONTROLFinishedEvent		msg(event);
		if (msg.cntlrName == itsCalCntlrName) {
			itsCalCntlrReady = true;
			LOG_DEBUG_STR("CalibrationController " << itsCalCntlrName << " is down");
		}
		else if (msg.cntlrName == itsBeamCntlrName) {
			itsBeamCntlrReady = true;
			LOG_DEBUG_STR("BeamController " << itsBeamCntlrName << " is down");
		}
		else {
			ASSERTSTR(false, "Received finished event for wrong controller: "
																	<< msg.cntlrName);
		}
		if (itsBeamCntlrReady && itsCalCntlrReady) {
			LOG_DEBUG_STR("Both controllers are down, informing stationControl task");
			itsReadyFlag = true;
		}
	}
	break;

	case F_INIT: 
		break;

	default:
		LOG_DEBUG_STR(itsName << ":default(" << F_EVT_PROTOCOL(event) << "," <<
															F_EVT_SIGNAL(event) << ")");
		return(GCFEvent::NOT_HANDLED);
		break;
	}

	return (GCFEvent::HANDLED);
}

ostream& ActiveObs::print (ostream& os) const
{
	os << "ACTIVEOBS         : " << itsName << endl;
	os << "instancenr        : " << itsInstanceNr << endl;
	os << "treeid            : " << itsObsPar.treeID << endl;
	os << "nyquistZone       : " << itsObsPar.nyquistZone << endl;
	os << "sampleClock       : " << itsObsPar.sampleClock << endl;
	os << "filter            : " << itsObsPar.filter << endl;
	os << "antennaArray      : " << itsObsPar.antennaArray << endl;
	os << "BeamCntlr ready   : " << itsBeamCntlrReady << endl;
	os << "CalCntlr ready    : " << itsCalCntlrReady << endl;
	os << "BeamControllerName: " << itsBeamCntlrName << endl;
	os << "CalControllerName : " << itsCalCntlrName << endl;
	os << "Ready             : " << itsReadyFlag << endl;

	return (os);
}



  } // namespace StationCU
} // namespace LOFAR
