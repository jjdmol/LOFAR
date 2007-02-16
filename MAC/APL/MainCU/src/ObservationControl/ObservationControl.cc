//#  ObservationControl.cc: Implementation of the MAC Scheduler task
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
#include <lofar_config.h>
#include <Common/LofarLogger.h>

#include <boost/shared_array.hpp>
#include <APS/ParameterSet.h>
#include <GCF/GCF_PVTypes.h>
#include <GCF/PAL/GCF_PVSSInfo.h>
#include <GCF/Utils.h>
#include <GCF/GCF_ServiceInfo.h>
#include <GCF/Protocols/PA_Protocol.ph>
#include <APL/APLCommon/APL_Defines.h>
#include <APL/APLCommon/APLUtilities.h>
#include <APL/APLCommon/APLCommonExceptions.h>
#include <APL/APLCommon/Controller_Protocol.ph>
#include <APL/APLCommon/StationInfo.h>
#include <signal.h>

#include "ObservationControl.h"
#include "ObservationControlDefines.h"

using namespace LOFAR::GCF::Common;
using namespace LOFAR::GCF::TM;
using namespace LOFAR::GCF::PAL;
using namespace std;

namespace LOFAR {
	using namespace APLCommon;
	using namespace ACC::APS;
	namespace MainCU {

// static pointer used for signal handler.
static ObservationControl*	thisObservationControl = 0;
	
//
// ObservationControl()
//
ObservationControl::ObservationControl(const string&	cntlrName) :
	GCFTask 			((State)&ObservationControl::initial_state,cntlrName),
	PropertySetAnswerHandlerInterface(),
	itsPropertySetAnswer(*this),
	itsPropertySet		(),
	itsChildControl		(0),
	itsChildPort		(0),
	itsParentControl	(0),
	itsParentPort		(0),
	itsTimerPort		(0),
	itsState			(CTState::NOSTATE),
	itsClaimTimer		(0),
	itsPrepareTimer		(0),
	itsStartTimer		(0),
	itsStopTimer		(0),
	itsHeartBeatTimer	(0),
	itsHeartBeatItv		(0)
{
	LOG_TRACE_OBJ_STR (cntlrName << " construction");

	// First readin our observation related config file.
	LOG_DEBUG_STR("Reading parset file:" << LOFAR_SHARE_LOCATION << "/" << cntlrName);
	globalParameterSet()->adoptFile(string(LOFAR_SHARE_LOCATION)+"/"+cntlrName);

	// Readin some parameters from the ParameterSet.
	itsStartTime     = time_from_string(globalParameterSet()->
											 getString("Observation.startTime"));
	itsStopTime      = time_from_string(globalParameterSet()->
											 getString("Observation.stopTime"));
	itsClaimPeriod   = globalParameterSet()->getTime  ("Observation.claimPeriod");
	itsPreparePeriod = globalParameterSet()->getTime  ("Observation.preparePeriod");

	// My own parameters
	itsTreePrefix   = globalParameterSet()->getString("prefix");
	itsInstanceNr   = globalParameterSet()->getUint32("_instanceNr");
	itsHeartBeatItv = globalParameterSet()->getUint32("heartbeatInterval");

	// Inform Logging manager who we are
	// TODO read this from the PARSET file!
	uint32	treeID(globalParameterSet()->getUint32("_treeID"));
	LOG_INFO(formatString("MACProcessScope: LOFAR.ObsSW.Observation%d.ObsCtrl", treeID));

	// attach to child control task
	itsChildControl = ChildControl::instance();
	itsChildPort = new GCFITCPort (*this, *itsChildControl, "childITCport", 
									GCFPortInterface::SAP, CONTROLLER_PROTOCOL);
	ASSERTSTR(itsChildPort, "Cannot allocate ITCport for childcontrol");
	itsChildPort->open();		// will result in F_CONNECTED

	// attach to parent control task
	itsParentControl = ParentControl::instance();
	itsParentPort = new GCFITCPort (*this, *itsParentControl, "ParentITCport", 
									GCFPortInterface::SAP, CONTROLLER_PROTOCOL);
	ASSERTSTR(itsChildPort, "Cannot allocate ITCport for Parentcontrol");
	itsParentPort->open();		// will result in F_CONNECTED

	// need port for timers.
	itsTimerPort = new GCFTimerPort(*this, "TimerPort");

	registerProtocol (CONTROLLER_PROTOCOL, CONTROLLER_PROTOCOL_signalnames);
	registerProtocol (PA_PROTOCOL, 		   PA_PROTOCOL_signalnames);
	registerProtocol (F_PML_PROTOCOL, 	   F_PML_PROTOCOL_signalnames);
 
	setState(CTState::CREATED);
}


//
// ~ObservationControl()
//
ObservationControl::~ObservationControl()
{
	LOG_TRACE_OBJ_STR (getName() << " destruction");

//	if (itsPropertySet) {
//		itsPropertySet->setValue(string(PVSSNAME_FSM_STATE),GCFPVString("down"));
//		itsPropertySet->disable();
//	}

}

//
// sigintHandler(signum)
//
void ObservationControl::sigintHandler(int signum)
{
	LOG_DEBUG (formatString("SIGINT signal detected (%d)",signum));

	// Note we can't call TRAN here because the siginthandler does not know our object.
	if (thisObservationControl) {
		thisObservationControl->finish();
	}
}

//
// finish
//
void ObservationControl::finish()
{
	TRAN(ObservationControl::finishing_state);
}

//
// setState(CTstateNr)
//
void	ObservationControl::setState(CTState::CTstateNr		newState)
{
	itsState = newState;

	CTState		cts;
	LOG_INFO_STR(getName() << " now in state " << cts.name(newState));

	if (itsPropertySet) {
		itsPropertySet->setValue(string(PVSSNAME_FSM_STATE),
									 GCFPVString(cts.name(newState)));
	}

	itsParentControl->nowInState(getName(), newState);
}

//
// handlePropertySetAnswer(answer)
//
void ObservationControl::handlePropertySetAnswer(GCFEvent& answer)
{
	LOG_TRACE_FLOW_STR ("handlePropertySetAnswer:" << evtstr(answer));

	switch(answer.signal) {
	case F_MYPS_ENABLED: {
		GCFPropSetAnswerEvent* pPropAnswer=static_cast<GCFPropSetAnswerEvent*>(&answer);
		if(pPropAnswer->result != GCF_NO_ERROR) {
			LOG_ERROR(formatString("%s : PropertySet %s NOT ENABLED",
										getName().c_str(), pPropAnswer->pScope));
		}
		// always let timer expire so main task will continue.
		itsTimerPort->setTimer(0.0);
		break;
	}

	case F_PS_CONFIGURED:
	{
		GCFConfAnswerEvent* pConfAnswer=static_cast<GCFConfAnswerEvent*>(&answer);
		if(pConfAnswer->result == GCF_NO_ERROR) {
			LOG_DEBUG(formatString("%s : apc %s Loaded",
										getName().c_str(), pConfAnswer->pApcName));
			//apcLoaded();
		}
		else {
			LOG_ERROR(formatString("%s : apc %s NOT LOADED",
										getName().c_str(), pConfAnswer->pApcName));
		}
		break;
	}

	case F_VGETRESP:
	case F_VCHANGEMSG: {
		// check which propertySet(!) changed
		GCFPropValueEvent* pPropAnswer=static_cast<GCFPropValueEvent*>(&answer);

		string PropSetName = createPropertySetName(PSN_OBS_CTRL, getName());
		if (strstr(pPropAnswer->pPropName, PropSetName.c_str()) == 0) {
			break;	// if not my own, exit
		}

		// don't watch state and error fields.
		if ((strstr(pPropAnswer->pPropName, PVSSNAME_FSM_STATE) != 0) || 
			(strstr(pPropAnswer->pPropName, PVSSNAME_FSM_ERROR) != 0) ||
			(strstr(pPropAnswer->pPropName, PVSSNAME_FSM_LOGMSG) != 0)) {
			return;
		}
 
		// periods are of type integer.
		if	(pPropAnswer->pValue->getType() == LPT_INTEGER) {
			uint32  newVal = (uint32) ((GCFPVInteger*)pPropAnswer->pValue)->getValue();

			if (strstr(pPropAnswer->pPropName, PN_OC_CLAIM_PERIOD) != 0) {
				LOG_INFO_STR ("Changing ClaimPeriod from " << itsClaimPeriod <<
							  " to " << newVal);
				itsClaimPeriod = newVal;
			}
			else if (strstr(pPropAnswer->pPropName, PN_OC_PREPARE_PERIOD) != 0) {
				LOG_INFO_STR ("Changing PreparePeriod from " << itsPreparePeriod <<
							  " to " << newVal);
				itsPreparePeriod = newVal;
			}
		}
		// times are of type string
		else if	(pPropAnswer->pValue->getType() == LPT_STRING) {
			string  newVal = (string) ((GCFPVString*)pPropAnswer->pValue)->getValue();
			ptime	newTime;
			try {
				newTime = time_from_string(newVal);
			}
			catch (exception&	e) {
				LOG_DEBUG_STR(newVal << " is not a legal time!!!");
				return;
			}
			if (strstr(pPropAnswer->pPropName, PN_OC_START_TIME) != 0) {
				LOG_INFO_STR ("Changing startTime from " << to_simple_string(itsStartTime)
							 << " to " << newVal);
				itsStartTime = newTime;
			}
			else if (strstr(pPropAnswer->pPropName, PN_OC_STOP_TIME) != 0) {
				LOG_INFO_STR ("Changing stopTime from " << to_simple_string(itsStopTime)
							 << " to " << newVal);
				itsStopTime = newTime;
			}
		}
		setObservationTimers();
		LOG_DEBUG("Sending all childs a RESCHEDULE event");
		itsChildControl->rescheduleChilds(to_time_t(itsStartTime), 
										  to_time_t(itsStopTime), "");
		break;
	}  

//  case F_SUBSCRIBED:      GCFPropAnswerEvent      pPropName
//  case F_UNSUBSCRIBED:    GCFPropAnswerEvent      pPropName
//  case F_PS_CONFIGURED:   GCFConfAnswerEvent      pApcName
//  case F_EXTPS_LOADED:    GCFPropSetAnswerEvent   pScope, result
//  case F_EXTPS_UNLOADED:  GCFPropSetAnswerEvent   pScope, result
//  case F_MYPS_ENABLED:    GCFPropSetAnswerEvent   pScope, result
//  case F_MYPS_DISABLED:   GCFPropSetAnswerEvent   pScope, result
//  case F_VGETRESP:        GCFPropValueEvent       pValue, pPropName
//  case F_VSETRESP:        GCFPropAnswerEvent      pPropName
//  case F_VCHANGEMSG:      GCFPropValueEvent       pValue, pPropName
//  case F_SERVER_GONE:     GCFPropSetAnswerEvent   pScope, result

	default:
		break;
	}  
}


//
// initial_state(event, port)
//
// Create top datapoint of this observation in PVSS.
//
GCFEvent::TResult ObservationControl::initial_state(GCFEvent& event, 
													GCFPortInterface& port)
{
	LOG_DEBUG_STR ("initial:" << evtstr(event) << "@" << port.getName());

	GCFEvent::TResult status = GCFEvent::HANDLED;
  
	switch (event.signal) {
    case F_INIT:
   		break;

	case F_ENTRY: {
		// Create 'Observationxxx' datapoint as top of the observation tree
		string	propSetName(createPropertySetName(PSN_OBSERVATION, getName()));
		LOG_DEBUG_STR ("Activating PropertySet: " << propSetName);
		itsBootPS = GCFMyPropertySetPtr(new GCFMyPropertySet(propSetName.c_str(),
															 PST_OBSERVATION,
															 PS_CAT_TEMP_AUTOLOAD,
															 &itsPropertySetAnswer));
		itsBootPS->enable();
		// Wait for timer that is set in PropertySetAnswer on ENABLED event
		}
		break;
	  
	case F_TIMER: {		// must be timer that PropSet has set.
		// update PVSS.
		LOG_TRACE_FLOW ("top DP of observation created, going to starting state");
		TRAN(ObservationControl::starting_state);				// go to next state.
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
// starting_state(event, port)
//
// Setup all connections.
//
GCFEvent::TResult ObservationControl::starting_state(GCFEvent& event, 
													GCFPortInterface& port)
{
	LOG_DEBUG_STR ("starting:" << evtstr(event) << "@" << port.getName());

	GCFEvent::TResult status = GCFEvent::HANDLED;
  
	switch (event.signal) {
    case F_INIT:
   		break;

	case F_ENTRY: {
		// Get access to my own propertyset.
		string	propSetName(createPropertySetName(PSN_OBS_CTRL, getName()));
		LOG_DEBUG_STR ("Activating PropertySet: " << propSetName);
		itsPropertySet = GCFMyPropertySetPtr(new GCFMyPropertySet(propSetName.c_str(),
																  PST_OBS_CTRL,
																  PS_CAT_TEMP_AUTOLOAD,
																  &itsPropertySetAnswer));
		itsPropertySet->enable();
		// Wait for timer that is set in PropertySetAnswer on ENABLED event
		}
		break;
	  
	case F_TIMER: {		// must be timer that PropSet has set.
		// first redirect signalhandler to finishing state to leave PVSS in the
		// right state when we are going down.
		thisObservationControl = this;
		signal (SIGINT,  ObservationControl::sigintHandler);	// ctrl-c
		signal (SIGTERM, ObservationControl::sigintHandler);	// kill

		// update PVSS.
		LOG_TRACE_FLOW ("Updateing state to PVSS");
		itsPropertySet->setValue(PVSSNAME_FSM_STATE,  GCFPVString("initial"));
		itsPropertySet->setValue(PVSSNAME_FSM_ERROR,  GCFPVString(""));
		itsPropertySet->setValue(PN_OC_CLAIM_PERIOD,  GCFPVInteger(itsClaimPeriod));
		itsPropertySet->setValue(PN_OC_PREPARE_PERIOD,GCFPVInteger(itsPreparePeriod));
		itsPropertySet->setValue(PN_OC_START_TIME,    GCFPVString(to_simple_string(itsStartTime)));
		itsPropertySet->setValue(PN_OC_STOP_TIME,     GCFPVString(to_simple_string(itsStopTime)));
		itsPropertySet->setValue(PN_OC_SUBBAND_LIST,  GCFPVString(
						APLUtilities::compactedArrayString(globalParameterSet()->
						getString("Observation.subbandList"))));
		itsPropertySet->setValue(PN_OC_BEAMLET_LIST, GCFPVString(
						APLUtilities::compactedArrayString(globalParameterSet()->
						getString("Observation.beamletList"))));
		itsPropertySet->setValue(PN_OC_BAND_FILTER, GCFPVString(
						globalParameterSet()->getString("Observation.bandFilter")));
		itsPropertySet->setValue(PN_OC_NYQUISTZONE, GCFPVInteger(
						globalParameterSet()->getInt32("Observation.nyquistZone")));
		itsPropertySet->setValue(PN_OC_ANTENNA_ARRAY, GCFPVString(
						globalParameterSet()->getString("Observation.antennaArray")));
		itsPropertySet->setValue(PN_OC_RECEIVER_LIST, GCFPVString(
						APLUtilities::compactedArrayString(globalParameterSet()->
						getString("Observation.receiverList"))));
		itsPropertySet->setValue(PN_OC_SAMPLE_CLOCK, GCFPVInteger(
						globalParameterSet()->getUint32("Observation.sampleClock")));
		itsPropertySet->setValue(PN_OC_MEASUREMENT_SET, GCFPVString(
						globalParameterSet()->getString("Observation.MSNameMask")));
		itsPropertySet->setValue(PN_OC_STATION_LIST, GCFPVString(
						APLUtilities::compactedArrayString(globalParameterSet()->
						getString("Observation.VirtualInstrument.stationList"))));
		itsPropertySet->setValue(PN_OC_INPUT_NODE_LIST, GCFPVString(
						APLUtilities::compactedArrayString(globalParameterSet()->
						getString("Observation.VirtualInstrument.inputNodeList"))));
		itsPropertySet->setValue(PN_OC_BGL_NODE_LIST, GCFPVString(
						APLUtilities::compactedArrayString(globalParameterSet()->
						getString("Observation.VirtualInstrument.BGLNodeList"))));
		itsPropertySet->setValue(PN_OC_STORAGE_NODE_LIST, GCFPVString(
						APLUtilities::compactedArrayString(globalParameterSet()->
						getString("Observation.VirtualInstrument.storageNodeList"))));

		// Start ChildControl task
		LOG_DEBUG ("Enabling ChildControl task");
		itsChildControl->openService(MAC_SVCMASK_OBSERVATIONCTRL, itsInstanceNr);
		itsChildControl->registerCompletionPort(itsChildPort);

		// Start ParentControl task
		LOG_DEBUG ("Enabling ParentControl task");
		itsParentPort = itsParentControl->registerTask(this);

		TRAN(ObservationControl::active_state);				// go to next state.
		}
		break;

	case F_CONNECTED:
		break;

	case F_DISCONNECTED:
		break;
	
	default:
		LOG_DEBUG_STR ("starting, default");
		status = GCFEvent::NOT_HANDLED;
		break;
	}    
	return (status);
}


//
// active_state(event, port)
//
// Normal operation state. 
//
GCFEvent::TResult ObservationControl::active_state(GCFEvent& event, GCFPortInterface& port)
{
	LOG_DEBUG_STR ("active:" << evtstr(event) << "@" << port.getName());

	GCFEvent::TResult status = GCFEvent::HANDLED;

	switch (event.signal) {
	case F_INIT:
		break;

	case F_ENTRY: {
		// convert times and periods to timersettings.
		itsChildControl->startChildControllers();
		setObservationTimers();
		itsHeartBeatTimer = itsTimerPort->setTimer(1.0 * itsHeartBeatItv);
		break;
	}

	case F_ACCEPT_REQ:
		break;

	case F_CONNECTED:	
		_connectedHandler(port);
		break;

	case F_DISCONNECTED:	
		_disconnectedHandler(port);
		break;

	case F_TIMER:  {
		GCFTimerEvent& timerEvent=static_cast<GCFTimerEvent&>(event);
		if (timerEvent.id == itsHeartBeatTimer) {
			doHeartBeatTask();
		}
		else if (timerEvent.id == itsClaimTimer) {
			setState(CTState::CLAIM);
			itsChildResult   = CT_RESULT_NO_ERROR;
			itsClaimTimer    = 0;
			LOG_DEBUG("Requesting all childs to execute the CLAIM state");
			itsChildControl->requestState(CTState::CLAIMED, "");
			itsBusyControllers = itsChildControl->countChilds(0, CNTLRTYPE_NO_TYPE);
		}
		else if (timerEvent.id == itsPrepareTimer) {
			setState(CTState::PREPARE);
			itsChildResult   = CT_RESULT_NO_ERROR;
			itsPrepareTimer  = 0;
			LOG_DEBUG("Requesting all childs to execute the PREPARE state");
			itsChildControl->requestState(CTState::PREPARED, "");
			itsBusyControllers = itsChildControl->countChilds(0, CNTLRTYPE_NO_TYPE);
		}
		else if (timerEvent.id == itsStartTimer) {
			setState(CTState::RESUME);
			itsChildResult   = CT_RESULT_NO_ERROR;
			itsStartTimer    = 0;
			LOG_DEBUG("Requesting all childs to go operation state");
			itsChildControl->requestState(CTState::RESUMED, "");
			itsBusyControllers = itsChildControl->countChilds(0, CNTLRTYPE_NO_TYPE);
		}
		else if (timerEvent.id == itsStopTimer) {
			setState(CTState::QUIT);
			itsChildResult   = CT_RESULT_NO_ERROR;
			itsStopTimer     = 0;
			LOG_DEBUG("Requesting all childs to quit");
			itsChildControl->requestState(CTState::QUITED, "");
			itsBusyControllers = itsChildControl->countChilds(0, CNTLRTYPE_NO_TYPE);
		}
		// some other timer?

		break;
	}

	// -------------------- EVENT RECEIVED FROM PARENT CONTROL --------------------
	case CONTROL_QUIT: {
		LOG_INFO("Received manual request for shutdown, accepting it.");
		itsTimerPort->cancelTimer(itsStopTimer);	// cancel old timer
		itsStopTimer = itsTimerPort->setTimer(0.0);	// expire immediately
		break;
	}

	// -------------------- EVENT RECEIVED FROM CHILD CONTROL --------------------
	case CONTROL_STARTED: {
		CONTROLStartedEvent	msg(event);
		if (msg.successful) {
			LOG_DEBUG_STR("Start of " << msg.cntlrName << " was successful");
		}
		else {
			LOG_WARN_STR("Start of " << msg.cntlrName << " was NOT successful");
		}
		// TODO: do something usefull with this information!
		break;
	}

	case CONTROL_CONNECT: {
		CONTROLConnectEvent		msg(event);
		LOG_DEBUG_STR("Received CONNECT(" << msg.cntlrName << ")");
		// TODO: do something usefull with this information!
		CONTROLConnectedEvent	answer;
		answer.cntlrName = msg.cntlrName;
		answer.result = CT_RESULT_NO_ERROR;
		itsParentPort->send(answer);
		break;
	}

	case CONTROL_SCHEDULED: {
		CONTROLScheduledEvent		msg(event);
		LOG_DEBUG_STR("Received SCHEDULED(" << msg.cntlrName << ")");
		// TODO: do something usefull with this information!
		break;
	}

	case CONTROL_CLAIMED: {
		CONTROLClaimedEvent		msg(event);
		LOG_DEBUG_STR("Received CLAIMED(" << msg.cntlrName << ")");
		itsChildResult |= msg.result;
		doHeartBeatTask();
		break;
	}

	case CONTROL_PREPARED: {
		CONTROLPreparedEvent		msg(event);
		LOG_DEBUG_STR("Received PREPARED(" << msg.cntlrName << ")");
		itsChildResult |= msg.result;
		doHeartBeatTask();
		break;
	}

	case CONTROL_RESUMED: {
		CONTROLResumedEvent		msg(event);
		LOG_DEBUG_STR("Received RESUMED(" << msg.cntlrName << ")");
		itsChildResult |= msg.result;
		doHeartBeatTask();
		break;
	}

	case CONTROL_SUSPENDED: {
		CONTROLSuspendedEvent		msg(event);
		LOG_DEBUG_STR("Received SUSPENDED(" << msg.cntlrName << ")");
		itsChildResult |= msg.result;
		doHeartBeatTask();
		break;
	}

	case CONTROL_RELEASED: {
		CONTROLReleasedEvent		msg(event);
		LOG_DEBUG_STR("Received RELEASED(" << msg.cntlrName << ")");
		itsChildResult |= msg.result;
		doHeartBeatTask();
		break;
	}

	case CONTROL_QUITED: {
		CONTROLQuitedEvent		msg(event);
		LOG_DEBUG_STR("Received QUITED(" << msg.cntlrName << ")");
		itsChildResult |= msg.result;
		doHeartBeatTask();
		break;
	}

	default:
		LOG_WARN("active_state, DEFAULT");
		status = GCFEvent::NOT_HANDLED;
		break;
	}

	return (status);
}

//
// finishing_state(event, port)
//
// Write controller state to PVSS, wait for 1 second (using a timer) to let GCF 
// handle the property change and close the controller
//
GCFEvent::TResult ObservationControl::finishing_state(GCFEvent& 		event, 
													  GCFPortInterface& port)
{
	LOG_DEBUG_STR ("finishing_state:" << evtstr(event) << "@" << port.getName());

	GCFEvent::TResult status = GCFEvent::HANDLED;

	switch (event.signal) {
	case F_INIT:
		break;

	case F_ENTRY: {
		// inform MACScheduler we are going down
		CONTROLQuitedEvent	msg;
		msg.cntlrName = getName();
		msg.result 	  = CT_RESULT_NO_ERROR;
		itsParentPort->send(msg);

		// update PVSS
		itsPropertySet->setValue(string(PVSSNAME_FSM_STATE),GCFPVString("finished"));
		itsPropertySet->setValue(string(PVSSNAME_FSM_ERROR),GCFPVString(""));

		itsTimerPort->setTimer(1L);
		break;
	}
  
    case F_TIMER:
      GCFTask::stop();
      break;
    
	default:
		LOG_DEBUG("finishing_state, default");
		status = GCFEvent::NOT_HANDLED;
		break;
	}    
	return (status);
}

//
// setObservationTimers()
//
void ObservationControl::setObservationTimers()
{
	//   |                  |  claim   |    prepare   |       observation       |
	//---+------------------+----------+--------------+-------------------------+-
	//  now               Tclaim    Tprepare       Tstart                    Tstop

	time_t	now   = to_time_t(second_clock::universal_time());
	time_t	start = to_time_t(itsStartTime);
	time_t	stop  = to_time_t(itsStopTime);

	// cancel current timers
	itsTimerPort->cancelTimer(itsClaimTimer);
	itsTimerPort->cancelTimer(itsPrepareTimer);
	itsTimerPort->cancelTimer(itsStartTimer);
	itsTimerPort->cancelTimer(itsStopTimer);
	itsClaimTimer = itsPrepareTimer = itsStartTimer = itsStopTimer = 0;

	// recalc new intervals
	int32	sec2claim   = start - now - itsPreparePeriod - itsClaimPeriod;
	int32	sec2prepare = start - now - itsPreparePeriod;
	int32	sec2start   = start - now;
	int32	sec2stop    = stop  - now;

	CTState::CTstateNr	assumedState(CTState::NOSTATE);

	// (re)set the claim timer
	if (itsState < CTState::CLAIM) { 				// claim state not done yet?
		if (sec2claim > 0) {
			itsClaimTimer = itsTimerPort->setTimer(1.0 * sec2claim);
			LOG_DEBUG_STR ("Claimperiod starts over " << sec2claim << " seconds");
		}
		else {
			assumedState = CTState::CLAIM;
			LOG_DEBUG_STR ("Claimperiod started " << -sec2claim << " seconds AGO!");
		}
	}
		
	// (re)set the prepare timer
	if (itsState < CTState::PREPARE) { 				// prepare state not done yet?
		if (sec2prepare > 0) {
			itsPrepareTimer = itsTimerPort->setTimer(1.0 * sec2prepare);
			LOG_DEBUG_STR ("PreparePeriod starts over " << sec2prepare << " seconds");
		}
		else {
			assumedState = CTState::PREPARE;
			LOG_DEBUG_STR ("PreparePeriod started " << -sec2prepare << " seconds AGO!");
		}
	}

	// (re)set the start timer
	if (itsState < CTState::RESUME) { 				// not yet active?
		if (sec2start > 0) {
			itsStartTimer = itsTimerPort->setTimer(1.0 * sec2start);
			LOG_DEBUG_STR ("Observation starts over " << sec2start << " seconds");
		}
		else {
			assumedState = CTState::RESUME;
			LOG_DEBUG_STR ("Observation started " << -sec2start << " seconds AGO!");
		}
	}

	// (re)set the stop timer
	if (itsState < CTState::RELEASE) { 				// not yet shutting down?
		if (sec2stop > 0) {
			itsStopTimer = itsTimerPort->setTimer(1.0 * sec2stop);
			LOG_DEBUG_STR ("Observation stops over " << sec2stop << " seconds");
		}
		else {
			assumedState = CTState::RELEASE;
			LOG_DEBUG_STR ("Observation should have been stopped " << -sec2start << 
							" seconds AGO!");
		}
	}

	// Timers of states in the future are set by now. Set the timer of the assumed
	// state to expired immediately. This may result in requesting a RESUME state
	// when not even the CLAIM state is reached yet. The parentControl task of the
	// child-controller will solve this by merging in the missing states.
	switch (assumedState) {
	case CTState::RELEASE: itsStopTimer    = itsTimerPort->setTimer(0.0); break;
	case CTState::RESUME:  itsStartTimer   = itsTimerPort->setTimer(0.0); break;
	case CTState::PREPARE: itsPrepareTimer = itsTimerPort->setTimer(0.0); break;
	case CTState::CLAIM:   itsClaimTimer   = itsTimerPort->setTimer(0.0); break;
	default:	break;	// satisfy compiler
	}

	LOG_DEBUG_STR ("Observation ends at " << to_simple_string(itsStopTime));
}

//
//doHeartBeatTask()
//
void  ObservationControl::doHeartBeatTask()
{
	// re-init heartbeat timer.
	itsTimerPort->cancelTimer(itsHeartBeatTimer);
	itsHeartBeatTimer = itsTimerPort->setTimer(1.0 * itsHeartBeatItv);
	
	// find out how many controllers are still busy.
	uint32	nrChilds = itsChildControl->countChilds(0, CNTLRTYPE_NO_TYPE);
	vector<ChildControl::StateInfo>		lateCntlrs = 
						itsChildControl->getPendingRequest("", 0, CNTLRTYPE_NO_TYPE);

	LOG_TRACE_FLOW_STR("itsBusyControllers=" << itsBusyControllers);

	// all controllers up to date?
	if (lateCntlrs.empty()) {
		LOG_DEBUG_STR("All (" << nrChilds << ") controllers are up to date");
		if (itsState == CTState::QUIT) {
			LOG_DEBUG_STR("Time for me to shutdown");
			TRAN(ObservationControl::finishing_state);
			return;
		}
 
		if (itsBusyControllers) {	// last time not all cntrls ready?
			CTState		cts;
			setState(cts.stateAck(itsState));
			itsBusyControllers = 0;
			// inform Parent (ignore funtion-result)
			sendControlResult(*itsParentPort, cts.signal(itsState), getName(), 
							  itsChildResult);
		}
		return;
	}

	itsBusyControllers = lateCntlrs.size();
	LOG_DEBUG_STR (itsBusyControllers << " controllers are still out of sync");
	CTState		cts;
	for (uint32 i = 0; i < itsBusyControllers; i++) {
		ChildControl::StateInfo*	si = &lateCntlrs[i];
		LOG_DEBUG_STR(si->name << ":" << cts.name(si->currentState) << " iso " 
									  << cts.name(si->requestedState));
	}

}
	
//
// _connectedHandler(port)
//
void ObservationControl::_connectedHandler(GCFPortInterface& /*port*/)
{
}


//
// _disconnectedHandler(port)
//
void ObservationControl::_disconnectedHandler(GCFPortInterface& port)
{
	port.close();
}


};
};
