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

//
// Note: the ActiveObs does all the administration of 1 observation. The admin
//		 of all observations is done by the StationController itself.
//

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

//# Includes
#include <Common/LofarLogger.h>
#include <Common/SystemUtil.h>
#include <ApplCommon/StationConfig.h>
#include <GCF/PVSS/GCF_PVTypes.h>
#include <GCF/TM/GCF_Protocols.h>
#include <APL/APLCommon/ChildControl.h>
#include <APL/APLCommon/ControllerDefines.h>
#include <APL/APLCommon/Controller_Protocol.ph>
#include <GCF/RTDB/DP_Protocol.ph>
#include "PVSSDatapointDefs.h"
#include "ActiveObs.h"

namespace LOFAR {
	using namespace APLCommon;
	using namespace MACIO;
	using namespace GCF::TM;
	using namespace GCF::PVSS;
	using namespace GCF::RTDB;
	namespace StationCU {

//
// ActiveObs(ParameterSet*	thePS, State	initial)
//
ActiveObs::ActiveObs(const string&		name,
					 State				initial,
					 ParameterSet*		thePS,
					 GCFTask&			task) :
	GCFTask				(initial, string("ActiveObs:") + name),
	itsStopTimerID		(0),
	itsPropSetTimer		(new GCFTimerPort(task, name)),
	itsName				(name),
	itsTask				(&task),
	itsInstanceNr		(getInstanceNr(name)),
	itsObsPar			(Observation(thePS)),
	itsUsesTBB			(false),
	itsBeamCntlrReady	(false),
	itsCalCntlrReady	(false),
	itsTBBCntlrReady	(false),
	itsBeamCntlrName	(controllerName(CNTLRTYPE_BEAMCTRL, 
						 itsInstanceNr, itsObsPar.obsID)),
	itsCalCntlrName		(controllerName(CNTLRTYPE_CALIBRATIONCTRL, 
						 itsInstanceNr, itsObsPar.obsID)),
	itsTBBCntlrName		(controllerName(CNTLRTYPE_TBBCTRL, 
						 itsInstanceNr, itsObsPar.obsID)),
	itsReadyFlag		(false),
	itsReqState			(CTState::NOSTATE),
	itsCurState			(CTState::NOSTATE)
{
	if (thePS->isDefined("Observation.TBB.TBBsetting[1].C0")) {
		LOG_INFO("Observation also uses the TB boards");
		itsUsesTBB = true;
	}
	else {
		LOG_INFO("Observation does not use the TB boards");
	}
}

//
// ~ActiveObs()
//
ActiveObs::~ActiveObs()
{
	if (itsPropertySet) {
		delete itsPropertySet;
	}
}

//
// initial(event, port)
//
// Create top datapoint of this observation in PVSS.
//
GCFEvent::TResult ActiveObs::initial(GCFEvent& event, 
								     GCFPortInterface& /*port*/)
{
	LOG_DEBUG(formatString("%s:initial - %04X", itsName.c_str(), event.signal));

	GCFEvent::TResult status = GCFEvent::HANDLED;
  
	switch (event.signal) {
    case F_ENTRY:
		itsReqState = CTState::CREATED;
		LOG_DEBUG_STR("Starting statemachine for observation " << itsName);
   		break;

	case F_INIT: {
		// Get access to my own propertyset.
		LOG_DEBUG_STR ("Activating PropertySet: " << itsObsPar.realPVSSdatapoint);
		itsPropertySet = new RTDBPropertySet(itsObsPar.realPVSSdatapoint,
											 PST_STN_OBSERVATION,
											 PSAT_WO,
											 itsTask);
#if 0
		}
		break;
	  
	case F_TIMER: {		// must be timer that PropSet has set.
#endif
		// update PVSS.
		LOG_TRACE_FLOW ("top DP of observation created, going to starting state");
		itsCurState = CTState::CREATED;
		TRAN(ActiveObs::starting);				// go to next state.
		}
		break;

	case DP_CREATED: {
			// NOTE: this function may be called DURING the construction of the PropertySet.
			// Always exit this event in a way that GCF can end the construction.
			DPCreatedEvent	dpEvent(event);
			LOG_DEBUG_STR("Result of creating " << dpEvent.DPname << " = " << dpEvent.result);
//			itsPropSetTimer->setTimer(0.1);
// TODO: Find out why this timer is not running.
        }
		break;
	  
	case F_CONNECTED:
		break;

	case F_DISCONNECTED:
		break;
	
	default:
		LOG_DEBUG_STR ("initial, default");
		status = GCFEvent::NOT_HANDLED;
		break;
	}    
	return (status);
}


//
// starting(event, port)
// Connect to childControllers BeamControl and CalControl
//
GCFEvent::TResult	ActiveObs::starting(GCFEvent&	event, GCFPortInterface&	port)
{
	LOG_DEBUG(formatString("%s:starting - %04X", itsName.c_str(), event.signal));

	switch (event.signal) {
	case F_ENTRY:  {
		// first make a mapping of the receivers that are used.
		// the StationController already modified the set to reflect the available receivers
		// So askfor this 'core' set by passed zeros.
		// the receiver bitmap can be derived from the RCUset.
		StationConfig		config;
		bitset<MAX_RCUS>	theRCUs(itsObsPar.getRCUbitset(0, 0, 0, config.hasSplitters));
		string	rbm;
		rbm.resize(MAX_RCUS, '0');
		for (int i = 0; i < MAX_RCUS; i++) {
			if (theRCUs[i]) {
				rbm[i] = '1';
			}
		}
		LOG_INFO_STR("Setting receiverBitMap of DP:" << itsObsPar.realPVSSdatapoint << "." << PN_OBS_RECEIVER_BITMAP);
		itsPropertySet->setValue(PN_OBS_RECEIVER_BITMAP,GCFPVString (rbm));
		itsPropertySet->setValue(PN_OBS_CLAIM_NAME, 
								 GCFPVString(formatString("LOFAR_ObsSW_Observation%d", itsObsPar.obsID)));

		// Startup the right controllers.
		itsReqState = CTState::CREATED;
		LOG_DEBUG_STR("Starting controller " << itsCalCntlrName);
		ChildControl::instance()->startChild(CNTLRTYPE_CALIBRATIONCTRL,
											 itsObsPar.obsID,
											 itsInstanceNr,
											 myHostname(false));
		// Results in CONTROL_CONNECT in StationControl Task.
											
		LOG_DEBUG_STR("Starting controller " << itsBeamCntlrName);
		ChildControl::instance()->startChild(CNTLRTYPE_BEAMCTRL,
											 itsObsPar.obsID,
											 itsInstanceNr,
											 myHostname(false));
		// Results in CONTROL_CONNECT in StationControl Task.

		if (itsUsesTBB) {
			LOG_DEBUG_STR("Starting controller " << itsTBBCntlrName);
			ChildControl::instance()->startChild(CNTLRTYPE_TBBCTRL,
												 itsObsPar.obsID,
												 itsInstanceNr,
												 myHostname(false));
			// Results in CONTROL_CONNECT in StationControl Task.
		}

		itsCurState = CTState::CONNECT;
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
		else if (msg.cntlrName == itsTBBCntlrName) {
			itsTBBCntlrReady = true;
			LOG_DEBUG("Connected to TBBController");
		}
		else {
			ASSERTSTR(false, "Received Connect event for wrong controller: "
																	<< msg.cntlrName);
		}
		if (itsBeamCntlrReady && itsCalCntlrReady && (itsUsesTBB == itsTBBCntlrReady)) {
			LOG_DEBUG_STR("Connected to all controllers, going to connected state");
			itsCurState = CTState::CONNECTED;
			TRAN(ActiveObs::connected);
		}
	}
	break;

	case CONTROL_QUIT:
		TRAN(ActiveObs::stopping);
		break;

	case CONTROL_QUITED: {		// one of the controller died unexpected. Force quit.
		CONTROLQuitedEvent		msg(event);
		LOG_FATAL_STR("Controller " << msg.cntlrName << " died unexpectedly. Aborting observation");
		TRAN(ActiveObs::stopping);
		queueTaskEvent(event, port);
	}
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
GCFEvent::TResult	ActiveObs::connected(GCFEvent&	event, GCFPortInterface&	port)
{
	LOG_DEBUG_STR(itsName << ":connected");

	switch (event.signal) {
	case F_ENTRY: 
		LOG_DEBUG_STR(itsName << ": in 'connected-mode', waiting for CLAIM event");
		break;

	case CONTROL_CLAIM: {
		// The stationControllerTask should have set the stationclock by now.
#if 0
		// Wait a second to give the RSPDriver time to register the clocksetting
		itsPropSetTimer->setTimer(1.5);
	}
	break;

	case F_TIMER: {
#endif
		// Activate the Calibration Controller
		itsReqState = CTState::CLAIMED;
		itsBeamCntlrReady = false;
		itsCalCntlrReady  = false;
		itsTBBCntlrReady  = false;

		LOG_DEBUG_STR("Asking " << itsCalCntlrName << " to connect to CalServer");
		ChildControl::instance()-> requestState(CTState::CLAIMED, itsCalCntlrName, 0, CNTLRTYPE_NO_TYPE);

		LOG_DEBUG_STR("Asking " << itsBeamCntlrName << " to connect to BeamServer");
		ChildControl::instance()-> requestState(CTState::CLAIMED, itsBeamCntlrName, 0, CNTLRTYPE_NO_TYPE);

		if (itsUsesTBB) {
			LOG_DEBUG_STR("Asking " << itsTBBCntlrName << " to connect to TBBDriver");
			ChildControl::instance()-> requestState(CTState::CLAIMED, itsTBBCntlrName, 0, CNTLRTYPE_NO_TYPE);
		}
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
		else if (msg.cntlrName == itsTBBCntlrName) {
			LOG_DEBUG("TBBController is connected to TBBDriver");
			itsTBBCntlrReady = true;
		}
		else {
			ASSERTSTR(false, "Received claimed event of unknown controller: " << 
							 msg.cntlrName);
		}
		if (itsBeamCntlrReady && itsCalCntlrReady && (itsUsesTBB == itsTBBCntlrReady)) {
			LOG_DEBUG("All controllers are ready, going to standby mode");
			itsCurState = CTState::CLAIMED;
			TRAN(ActiveObs::standby);
		}
	}
	break;

	case CONTROL_QUIT:
		TRAN(ActiveObs::stopping);
		break;

	case CONTROL_QUITED: {		// one of the controller died unexpected. Force quit.
		CONTROLQuitedEvent		msg(event);
		LOG_FATAL_STR("Controller " << msg.cntlrName << " died unexpectedly. Aborting observation");
		TRAN(ActiveObs::stopping);
		queueTaskEvent(event, port);
	}
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
GCFEvent::TResult	ActiveObs::standby(GCFEvent&	event, GCFPortInterface&	port)
{
	LOG_DEBUG_STR(itsName << ":standby");

	switch (event.signal) {
	case F_ENTRY: 
		LOG_DEBUG_STR(itsName << ": in 'standby-mode', waiting for PREPARE event");
		break;

	case CONTROL_PREPARE: {
		itsReqState 	  = CTState::PREPARED;
		itsBeamCntlrReady = false;
		itsCalCntlrReady  = false;
		itsTBBCntlrReady  = false;
		// Activate the CalController
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
				LOG_ERROR_STR("Start of beam failed with error " << msg.result);	
				break;
			}
			LOG_DEBUG_STR("BeamController has started the beam, asking TBBCtlr to prepare TBBs");
			itsBeamCntlrReady = true;
			ChildControl::instance()->requestState(CTState::PREPARED, 
											itsTBBCntlrName, 0, CNTLRTYPE_NO_TYPE);
		}
		else if (msg.cntlrName == itsTBBCntlrName) {
			if (msg.result != CT_RESULT_NO_ERROR) {
				LOG_ERROR_STR("Start of TBB failed with error, " << msg.result << 
								". CONTINUING OBSERVATION WITHOUT TBB.");	
				itsUsesTBB = false;
				break;
			}
			LOG_DEBUG_STR("TBBController has started the beam");
			itsTBBCntlrReady = true;
		}
		else {
			ASSERTSTR(false, "Received claimed event of unknown controller: " << 
							 msg.cntlrName);
		}

		if (itsBeamCntlrReady && itsCalCntlrReady && (itsUsesTBB == itsTBBCntlrReady)) {
			LOG_DEBUG("Both controllers are ready, going to operational mode");
			itsCurState = CTState::PREPARED;
			TRAN(ActiveObs::operational);
		}
	}
	break;

	case CONTROL_QUIT:
		TRAN(ActiveObs::stopping);
		break;

	case CONTROL_QUITED: {		// one of the controller died unexpected. Force quit.
		CONTROLQuitedEvent		msg(event);
		LOG_FATAL_STR("Controller " << msg.cntlrName << " died unexpectedly. Aborting observation");
		TRAN(ActiveObs::stopping);
		queueTaskEvent(event, port);
	}
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
GCFEvent::TResult	ActiveObs::operational(GCFEvent&	event, GCFPortInterface&	port)
{
	LOG_DEBUG_STR(itsName << ":operational");

	switch (event.signal) {
	case F_ENTRY: 
		LOG_DEBUG_STR(itsName << ": in 'operational-mode' until RELEASE event");
		break;

	case CONTROL_SUSPEND: {
		// pass event through to controllers
		LOG_DEBUG("Passing SUSPEND event to childs");
		itsReqState 	  = CTState::SUSPENDED;
		itsBeamCntlrReady = false;
		itsCalCntlrReady  = false;
		itsTBBCntlrReady  = false;
		ChildControl::instance()->
				requestState(CTState::SUSPENDED, itsBeamCntlrName, 0, CNTLRTYPE_NO_TYPE);
		ChildControl::instance()->
				requestState(CTState::SUSPENDED, itsCalCntlrName, 0, CNTLRTYPE_NO_TYPE);
		ChildControl::instance()->
				requestState(CTState::SUSPENDED, itsTBBCntlrName, 0, CNTLRTYPE_NO_TYPE);
		// will result in CONTROL_RELEASED
	}
	break;
	
	case CONTROL_RESUME: {
		// pass event through to controllers
		LOG_DEBUG("Passing RESUME event to childs");
		itsReqState 	  = CTState::RESUMED;
		itsBeamCntlrReady = false;
		itsCalCntlrReady  = false;
		itsTBBCntlrReady  = false;
		ChildControl::instance()->
				requestState(CTState::RESUMED, itsBeamCntlrName, 0, CNTLRTYPE_NO_TYPE);
		ChildControl::instance()->
				requestState(CTState::RESUMED, itsCalCntlrName, 0, CNTLRTYPE_NO_TYPE);
		ChildControl::instance()->
				requestState(CTState::RESUMED, itsTBBCntlrName, 0, CNTLRTYPE_NO_TYPE);
		// will result in CONTROL_RESUMED
	}
	break;

	case CONTROL_SUSPENDED:
	case CONTROL_RESUMED: {
		// TODO: test for the results of the actions
		CONTROLCommonEvent		msg(event);			// we only need the name for now
		if (msg.cntlrName == itsBeamCntlrName) {
			itsBeamCntlrReady = true;
		}
		if (msg.cntlrName == itsCalCntlrName) {
			itsCalCntlrReady = true;
		}
		if (msg.cntlrName == itsTBBCntlrName) {
			itsTBBCntlrReady = true;
		}
		if (itsBeamCntlrReady && itsCalCntlrReady && (itsUsesTBB == itsTBBCntlrReady)) {
			CTState		cts;
			itsCurState == cts.signal2stateNr(event.signal);
		}
	}
	break;
	
	case CONTROL_RELEASE: {
		// release beam at the BeamController
		LOG_DEBUG_STR("Asking " << itsBeamCntlrName << " to stop the beam");
		itsReqState = CTState::RELEASED;
		itsBeamCntlrReady = false;
		itsCalCntlrReady  = false;
		ChildControl::instance()->
				requestState(CTState::RELEASED, itsBeamCntlrName, 0, CNTLRTYPE_NO_TYPE);
		// will result in CONTROL_RELEASED
	}
	break;

	case CONTROL_RELEASED: {
		CONTROLReleaseEvent		msg(event);
		if (msg.cntlrName == itsBeamCntlrName) {
			itsBeamCntlrReady = true;
			LOG_DEBUG_STR("Beam is stopped, stopping calibration");
			ChildControl::instance()->
				requestState(CTState::RELEASED, itsCalCntlrName, 0, CNTLRTYPE_NO_TYPE);
			break;
		}
		if (msg.cntlrName == itsCalCntlrName) {
			itsCalCntlrReady  = true;
			if (itsUsesTBB) {
				LOG_DEBUG_STR("Calibration is stopped, Stopping TBB");
				ChildControl::instance()->
					requestState(CTState::RELEASED, itsTBBCntlrName, 0, CNTLRTYPE_NO_TYPE);
			}
			else {
				LOG_DEBUG_STR("Calibration is stopped, going to standby mode");
				itsCurState = CTState::RESUMED;
				TRAN(ActiveObs::standby);
			}
			break;
		}
		if (msg.cntlrName == itsTBBCntlrName) {
			itsTBBCntlrReady  = true;
			LOG_DEBUG_STR("TBB is stopped, going to standby mode");
			itsCurState = CTState::RESUMED;
			TRAN(ActiveObs::standby);
			break;
		}
		ASSERTSTR(false, "RELEASE event received from unknown controller:" 
																	<< msg.cntlrName);
	}
	break;

	case CONTROL_QUIT:
		TRAN(ActiveObs::stopping);
		break;

	case CONTROL_QUITED: {		// one of the controller died unexpected. Force quit.
		CONTROLQuitedEvent		msg(event);
		LOG_FATAL_STR("Controller " << msg.cntlrName << " died unexpectedly. Aborting observation");
		TRAN(ActiveObs::stopping);
		queueTaskEvent(event, port);
	}
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
	case F_ENTRY:  {
		itsReqState 	  = CTState::QUITED;
		itsBeamCntlrReady = false;
		itsCalCntlrReady  = false;
		itsTBBCntlrReady  = false;

		// release beam at the BeamController
		LOG_DEBUG_STR("Asking " << itsBeamCntlrName << " to quit");
		ChildControl::instance()->
				requestState(CTState::QUITED, itsBeamCntlrName, 0, CNTLRTYPE_NO_TYPE);
		// will result in CONTROL_QUITED

		LOG_DEBUG_STR("Asking " << itsCalCntlrName << " to quit");
		ChildControl::instance()->
				requestState(CTState::QUITED, itsCalCntlrName, 0, CNTLRTYPE_NO_TYPE);
		// will result in CONTROL_QUITED

		if (itsUsesTBB) {
			LOG_DEBUG_STR("Asking " << itsTBBCntlrName << " to quit");
			ChildControl::instance()->
					requestState(CTState::QUITED, itsTBBCntlrName, 0, CNTLRTYPE_NO_TYPE);
			// will result in CONTROL_QUITED
		}

		LOG_DEBUG_STR(itsName << ": in 'stopping-mode' until controllers are down");
	}
	break;

	case CONTROL_QUITED: {
		CONTROLQuitedEvent		msg(event);
		if (msg.cntlrName == itsCalCntlrName) {
			itsCalCntlrReady = true;
			LOG_DEBUG_STR("CalibrationController " << itsCalCntlrName << " is down");
		}
		else if (msg.cntlrName == itsBeamCntlrName) {
			itsBeamCntlrReady = true;
			LOG_DEBUG_STR("BeamController " << itsBeamCntlrName << " is down");
		}
		else if (msg.cntlrName == itsTBBCntlrName) {
			itsTBBCntlrReady = true;
			LOG_DEBUG_STR("TBBController " << itsTBBCntlrName << " is down");
		}
		else {
			ASSERTSTR(false, "Received finished event for wrong controller: "
																	<< msg.cntlrName);
		}
		if (itsBeamCntlrReady && itsCalCntlrReady && (itsUsesTBB == itsTBBCntlrReady)) {
			LOG_DEBUG_STR("All controllers are down, informing stationControl task");
			itsCurState  = CTState::QUITED;
			itsReadyFlag = true;
		}
	}
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
// print(os)
//
ostream& ActiveObs::print (ostream& os) const
{
	os << "ACTIVEOBS         : " << itsName << endl;
	os << "instancenr        : " << itsInstanceNr << endl;
	os << "treeid            : " << itsObsPar.obsID << endl;
	os << "nyquistZone       : " << itsObsPar.nyquistZone << endl;
	os << "sampleClock       : " << itsObsPar.sampleClock << endl;
	os << "filter            : " << itsObsPar.filter << endl;
	os << "antennaArray      : " << itsObsPar.antennaArray << endl;
	os << "receiverList      : " << itsObsPar.receiverList << endl;
	os << "BeamCntlr ready   : " << itsBeamCntlrReady << endl;
	os << "CalCntlr ready    : " << itsCalCntlrReady << endl;
	if (itsUsesTBB) {
		os << "TBBCntlr ready    : " << itsTBBCntlrReady << endl;
	}
	os << "BeamControllerName: " << itsBeamCntlrName << endl;
	os << "CalControllerName : " << itsCalCntlrName << endl;
	if (itsUsesTBB) {
		os << "TBBControllerName : " << itsTBBCntlrName << endl;
	}
	os << "Ready             : " << itsReadyFlag << endl;

	return (os);
}



  } // namespace StationCU
} // namespace LOFAR
