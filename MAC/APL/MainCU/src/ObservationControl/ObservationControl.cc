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

#include <APS/ParameterSet.h>
#include <GCF/GCF_PVTypes.h>
#include <GCF/Utils.h>
#include <GCF/GCF_ServiceInfo.h>
#include <APL/APLCommon/APL_Defines.h>
#include <APL/APLCommon/APLUtilities.h>
#include <APL/APLCommon/Controller_Protocol.ph>
#include <APL/APLCommon/StationInfo.h>
#include <GCF/RTDB/DP_Protocol.ph>
#include <signal.h>

#include "ObservationControl.h"
#include "ObservationControlDefines.h"

using namespace LOFAR::GCF::Common;
using namespace LOFAR::GCF::TM;
using namespace LOFAR::GCF::RTDB;
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
	itsPropertySet		(0),
	itsChildControl		(0),
	itsChildPort		(0),
	itsParentControl	(0),
	itsParentPort		(0),
	itsTimerPort		(0),
	itsState			(CTState::NOSTATE),
	itsNrControllers  	(0),
	itsBusyControllers  (0),
	itsQuitReason		(CT_RESULT_NO_ERROR),
	itsClaimTimer		(0),
	itsPrepareTimer		(0),
	itsStartTimer		(0),
	itsStopTimer		(0),
	itsForcedQuitTimer	(0),
	itsHeartBeatTimer	(0),
	itsHeartBeatItv		(0),
	itsForcedQuitDelay	(0)
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
	itsInstanceNr   = globalParameterSet()->getUint32("_treeID");	// !!!
	itsHeartBeatItv = globalParameterSet()->getUint32("heartbeatInterval");

	// The time I have to wait for the forced quit depends on the integration time of OLAP
	string	OLAPpos = globalParameterSet()->locateModule("OLAP");
	LOG_DEBUG(OLAPpos+"OLAP.IONProc.integrationSteps");
	itsForcedQuitDelay = 15 + globalParameterSet()->getUint32(OLAPpos+"OLAP.IONProc.integrationSteps",0);
	LOG_DEBUG_STR ("Timer for forcing quit is set to " << itsForcedQuitDelay);

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

	GCF::TM::registerProtocol (CONTROLLER_PROTOCOL, CONTROLLER_PROTOCOL_STRINGS);
	GCF::TM::registerProtocol (DP_PROTOCOL, 		DP_PROTOCOL_STRINGS);
 
	// we cannot use setState right now, wait for propertysets to come online
	//	setState(CTState::CREATED);
}


//
// ~ObservationControl()
//
ObservationControl::~ObservationControl()
{
	LOG_TRACE_OBJ_STR (getName() << " destruction");

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
		itsPropertySet->setValue(string(PVSSNAME_FSM_CURACT),
									 GCFPVString(cts.name(newState)));
	}

	itsParentControl->nowInState(getName(), newState);
}

//
// initial_state(event, port)
//
// Create top datapoint of this observation in PVSS.
//
GCFEvent::TResult ObservationControl::initial_state(GCFEvent& event, 
													GCFPortInterface& port)
{
	LOG_DEBUG_STR ("initial:" << eventName(event) << "@" << port.getName());

	GCFEvent::TResult status = GCFEvent::HANDLED;
  
	switch (event.signal) {
    case F_INIT:
   		break;

	case F_ENTRY: {
		// Create 'Observationxxx' datapoint as top of the observation tree
		string	propSetName(createPropertySetName(PSN_OBSERVATION, getName()));
		LOG_DEBUG_STR ("Activating PropertySet: " << propSetName);
		itsBootPS = new RTDBPropertySet(propSetName.c_str(),
										PST_OBSERVATION,
										PSAT_RW,
										this);
		}
		break;
	  
	case DP_CREATED: {
			// NOTE: thsi function may be called DURING the construction of the PropertySet.
			// Always exit this event in a way that GCF can end the construction.
			DPCreatedEvent	dpEvent(event);
			LOG_DEBUG_STR("Result of creating " << dpEvent.DPname << " = " << dpEvent.result);
			itsTimerPort->cancelAllTimers();
			itsTimerPort->setTimer(0.0);
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
	LOG_DEBUG_STR ("starting:" << eventName(event) << "@" << port.getName());

	GCFEvent::TResult status = GCFEvent::HANDLED;
  
	switch (event.signal) {
    case F_INIT:
   		break;

	case F_ENTRY: {
		// Get access to my own propertyset.
		string	propSetName(createPropertySetName(PSN_OBS_CTRL, getName()));
		LOG_DEBUG_STR ("Activating PropertySet: " << propSetName);
		itsPropertySet = new RTDBPropertySet(propSetName.c_str(),
											 PST_OBS_CTRL,
											 PSAT_RW,
											 this);
		}
		break;
	  
	case DP_CREATED: {
			// NOTE: thsi function may be called DURING the construction of the PropertySet.
			// Always exit this event in a way that GCF can end the construction.
			DPCreatedEvent	dpEvent(event);
			LOG_DEBUG_STR("Result of creating " << dpEvent.DPname << " = " << dpEvent.result);
			itsTimerPort->cancelAllTimers();
			itsTimerPort->setTimer(0.0);
        }
		break;
	  
	case F_TIMER: {		// can be timer that PropSet has set or timer from
						// wait period for Child and Parent Control.
		if (thisObservationControl == this) {	// task timer
			TRAN(ObservationControl::active_state);				// go to next state.
			break;
		}

		// first redirect signalhandler to finishing state to leave PVSS in the
		// right state when we are going down.
		thisObservationControl = this;
		signal (SIGINT,  ObservationControl::sigintHandler);	// ctrl-c
		signal (SIGTERM, ObservationControl::sigintHandler);	// kill

		// register what we are doing
		setState(CTState::CONNECT);

		// update PVSS.
		LOG_TRACE_FLOW ("Updateing state to PVSS");
		itsPropertySet->setValue(PVSSNAME_FSM_CURACT, GCFPVString("Initial"));
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

		itsTimerPort->setTimer(2.0);	// wait 2 second for tasks to come up.

		}
		break;

	case F_CONNECTED:
		break;

	case F_DISCONNECTED:
		break;
	
	case DP_CHANGED:
		_databaseEventHandler(event);
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
	LOG_DEBUG_STR ("active:" << eventName(event) << "@" << port.getName());

	GCFEvent::TResult status = GCFEvent::HANDLED;

	switch (event.signal) {
	case F_INIT:
		break;

	case F_ENTRY: {
		// convert times and periods to timersettings.
		itsChildControl->startChildControllers();
		itsNrControllers = itsChildControl->countChilds(0, CNTLRTYPE_NO_TYPE);
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

	case DP_CHANGED:
		_databaseEventHandler(event);
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
			// reschedule forced-quit timer for safety.
			itsTimerPort->cancelTimer(itsForcedQuitTimer);
			itsForcedQuitTimer = itsTimerPort->setTimer(1.0 * itsForcedQuitDelay);
		}
		else if (timerEvent.id == itsForcedQuitTimer) {
			LOG_WARN("QUITING BEFORE ALL CHILDREN DIED.");
			TRAN(ObservationControl::finishing_state);
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
	LOG_DEBUG_STR ("finishing_state:" << eventName(event) << "@" << port.getName());

	GCFEvent::TResult status = GCFEvent::HANDLED;

	switch (event.signal) {
	case F_INIT:
		break;

	case F_ENTRY: {
		// first turn off 'old' timers
		itsTimerPort->cancelTimer(itsForcedQuitTimer);
		itsTimerPort->cancelTimer(itsStopTimer);

		// tell Parent task we like to go down.
		itsParentControl->nowInState(getName(), CTState::QUIT);

		// inform MACScheduler we are going down
		CONTROLQuitedEvent	msg;
		msg.cntlrName = getName();
		msg.result 	  = itsQuitReason;;
		itsParentPort->send(msg);

		// update PVSS
		itsPropertySet->setValue(string(PVSSNAME_FSM_CURACT),GCFPVString("Finished"));
		itsPropertySet->setValue(string(PVSSNAME_FSM_ERROR),GCFPVString(""));

		itsTimerPort->setTimer(1L);	// give PVSS task some time to update the DB.
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
	itsTimerPort->cancelTimer(itsForcedQuitTimer);
	itsClaimTimer = itsPrepareTimer = itsStartTimer = 
					itsStopTimer = itsForcedQuitTimer = 0;

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
			// make sure we go down 30 seconds after quit was requested.
			itsForcedQuitTimer = itsTimerPort->setTimer(sec2stop + (1.0 * itsForcedQuitDelay));
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

	// check if enough stations are left too do our job.
	// TODO: add criteria to SAS database and test those iso this foolish criteria.
	if (nrChilds != itsNrControllers) {
		LOG_WARN_STR("Only " << nrChilds << " out of " << itsNrControllers << " stations still available.");
		// if no more children left while we are not in the quit-phase (stoptimer still running)
		if (!nrChilds && itsStopTimer) {
			LOG_FATAL("Too less stations left, FORCING QUIT OF OBSERVATION");
			itsQuitReason = CT_RESULT_LOST_CONNECTION;
			itsTimerPort->cancelTimer(itsStopTimer);
			itsStopTimer = itsTimerPort->setTimer(0.0);
			return;
		}
	}

	LOG_TRACE_FLOW_STR("itsBusyControllers=" << itsBusyControllers);

	// all controllers up to date?
	if (lateCntlrs.empty()) {
		LOG_DEBUG_STR("All (" << nrChilds << ") controllers are up to date");
		if (itsState == CTState::QUIT) {
			LOG_DEBUG_STR("Time for me to shutdown");
			TRAN(ObservationControl::finishing_state);
			return;
		}
 
		if (itsBusyControllers) {	// last time NOT all cntrls ready?
			CTState		cts;		// report that state is reached.
			setState(cts.stateAck(itsState));
			itsBusyControllers = 0;
			// inform Parent (ignore function-result)
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

//
// _databaseEventHandler(answer)
//
void ObservationControl::_databaseEventHandler(GCFEvent& event)
{
	LOG_TRACE_FLOW_STR ("_databaseEventHandler:" << eventName(event));

	switch(event.signal) {
	case DP_CHANGED: {
		DPChangedEvent	dpEvent(event);

		// don't watch state and error fields.
		if ((strstr(dpEvent.DPname.c_str(), PVSSNAME_FSM_STATE) != 0) || 
			(strstr(dpEvent.DPname.c_str(), PVSSNAME_FSM_ERROR) != 0) ||
			(strstr(dpEvent.DPname.c_str(), PVSSNAME_FSM_CURACT) != 0) ||
			(strstr(dpEvent.DPname.c_str(), PVSSNAME_FSM_LOGMSG) != 0)) {
			return;
		}
 
		// Change of claim_period?
		if (strstr(dpEvent.DPname.c_str(), PN_OC_CLAIM_PERIOD) != 0) {
			uint32  newVal = ((GCFPVInteger*) (dpEvent.value._pValue))->getValue();
			LOG_INFO_STR ("Changing ClaimPeriod from " << itsClaimPeriod << " to " << newVal);
			itsClaimPeriod = newVal;
		}
		// Change of prepare_period?
		else if (strstr(dpEvent.DPname.c_str(), PN_OC_PREPARE_PERIOD) != 0) {
			uint32  newVal = ((GCFPVInteger*) (dpEvent.value._pValue))->getValue();
			LOG_INFO_STR ("Changing PreparePeriod from " << itsPreparePeriod << " to " << newVal);
			itsPreparePeriod = newVal;
		}

		// Change of start or stop time?
		if ((strstr(dpEvent.DPname.c_str(), PN_OC_START_TIME) != 0)  || 
		    (strstr(dpEvent.DPname.c_str(), PN_OC_STOP_TIME) != 0)) {
			string  newVal = ((GCFPVString*) (dpEvent.value._pValue))->getValue();
			ptime	newTime;
			try {
				newTime = time_from_string(newVal);
			}
			catch (exception&	e) {
				LOG_DEBUG_STR(newVal << " is not a legal time!!!");
				return;
			}
			if (strstr(dpEvent.DPname.c_str(), PN_OC_START_TIME) != 0) { 
				LOG_INFO_STR ("Changing startTime from " << to_simple_string(itsStartTime) << " to " << newVal);
				itsStartTime = newTime;
			}
			else {
				LOG_INFO_STR ("Changing stopTime from " << to_simple_string(itsStopTime) << " to " << newVal);
				itsStopTime = newTime;
			}
			setObservationTimers();
			LOG_DEBUG("NOT  YET  Sending all childs a RESCHEDULE event");
//			itsChildControl->rescheduleChilds(to_time_t(itsStartTime), 
//											  to_time_t(itsStopTime), "");
		}
	}  
	break;

	default:
		break;
	}  
}



};
};
