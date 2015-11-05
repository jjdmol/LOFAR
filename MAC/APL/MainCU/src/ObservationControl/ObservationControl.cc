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
#include <Common/StreamUtil.h>
#include <Common/Version.h>
#include <Common/ParameterSet.h>
#include <Common/SystemUtil.h>
#include <ApplCommon/StationInfo.h>
#include <ApplCommon/LofarDirs.h>
#include <ApplCommon/PosixTime.h>

#include <MACIO/MACServiceInfo.h>
#include <APL/APLCommon/APL_Defines.h>
#include <APL/APLCommon/APLUtilities.h>
#include <APL/APLCommon/Controller_Protocol.ph>
#include <GCF/PVSS/GCF_PVTypes.h>
#include <GCF/RTDB/DP_Protocol.ph>
#include <APL/RTDBCommon/RTDButilities.h>
#include <APL/RTDBCommon/CM_Protocol.ph>
#include <signal.h>

#include "ObservationControl.h"
#include "PVSSDatapointDefs.h"
#include <MainCU/Package__Version.h>

#include <boost/date_time/posix_time/posix_time.hpp>

using namespace boost::posix_time;

namespace LOFAR {
	using namespace APLCommon;
	using namespace GCF::TM;
	using namespace GCF::PVSS;
	using namespace GCF::RTDB;
	using namespace APL::RTDBCommon;
	namespace MainCU {

// static pointer used for signal handler.
static ObservationControl*	thisObservationControl = 0;
	
//
// ObservationControl()
//
ObservationControl::ObservationControl(const string&	cntlrName) :
	GCFTask 			((State)&ObservationControl::starting_state,cntlrName),
	itsPropertySet		(0),
	itsDPservice		(0),
	itsClaimMgrTask		(0),
	itsClaimMgrPort		(0),
	itsChildControl		(0),
	itsChildPort		(0),
	itsParentControl	(0),
	itsParentPort		(0),
	itsTimerPort		(0),
	itsFullReport		(false),
	itsChangeReport		(false),
	itsState			(CTState::NOSTATE),
	itsLastReportedState(CTState::NOSTATE),
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
	LOG_INFO(Version::getInfo<MainCUVersion>("ObservationControl"));

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
	itsProcessType   = globalParameterSet()->getString("Observation.processType", "Observation");

	// Values from my conf file
	itsLateLimit     = globalParameterSet()->getTime   ("ObservationControl.lateLimit", 15);
	itsFailedLimit   = globalParameterSet()->getTime   ("ObservationControl.failedLimit", 30);
	itsHeartBeatItv	 = globalParameterSet()->getTime   ("ObservationControl.heartbeatInterval", 10);
	string reportType = globalParameterSet()->getString("ObservationControl.reportType", "Full");
	if 		(reportType == "Full")		itsFullReport = true;
	else if (reportType == "Changes")	itsChangeReport = true;

	// My own parameters
	itsTreePrefix   	 = globalParameterSet()->getString("prefix");
	itsTreeID			 = globalParameterSet()->getUint32("_treeID");	// !!!
	itsObsDPname		 = globalParameterSet()->getString("_DPname");

	// The time I have to wait for the forced quit depends on the integration time of OLAP
	string	OLAPpos = globalParameterSet()->locateModule("OLAP");
	LOG_DEBUG(OLAPpos+"OLAP.IONProc.integrationSteps");
	itsForcedQuitDelay = 15 + globalParameterSet()->getUint32(OLAPpos+"OLAP.IONProc.integrationSteps",0);
	LOG_INFO_STR ("Timer for forcing quit is " << itsForcedQuitDelay << " seconds");

	// Inform Logging manager who we are
	LOG_INFO_STR("MACProcessScope: " << createPropertySetName(PSN_OBSERVATION_CONTROL, getName(), itsObsDPname));
	// TODO: SAS gateway is not yet aware of claimMgr so the data will not be transferred to SAS.

	// attach to child control task
	itsChildControl = ChildControl::instance();
	itsChildPort = new GCFITCPort (*this, *itsChildControl, "childITCport", GCFPortInterface::SAP, CONTROLLER_PROTOCOL);
	ASSERTSTR(itsChildPort, "Cannot allocate ITCport for childcontrol");
	itsChildPort->open();		// will result in F_CONNECTED

	// attach to parent control task
	itsParentControl = ParentControl::instance();

	// need port for timers.
	itsTimerPort = new GCFTimerPort(*this, "ObservationControlTimer");

	// create a datapoint service for setting runstates and so on
	itsDPservice = new DPservice(this);

	registerProtocol (CONTROLLER_PROTOCOL, CONTROLLER_PROTOCOL_STRINGS);
	registerProtocol (DP_PROTOCOL, 		   DP_PROTOCOL_STRINGS);
	registerProtocol (CM_PROTOCOL, 		   CM_PROTOCOL_STRINGS);
 
	// we cannot use setState right now, wait for propertysets to come online
	//	setState(CTState::CREATED);
}


//
// ~ObservationControl()
//
ObservationControl::~ObservationControl()
{
	LOG_TRACE_OBJ_STR (getName() << " destruction");

	if (itsDPservice) {
		delete itsDPservice;
	}

}

//
// sigintHandler(signum)
//
void ObservationControl::sigintHandler(int signum)
{
	LOG_WARN (formatString("SIGINT signal detected (%d)",signum));

	// Note we can't call TRAN here because the siginthandler does not know our object.
	if (thisObservationControl) {
//		if (signum == SIGABRT) {
			thisObservationControl->abortObservation();
//		}
//		else {
//			thisObservationControl->finish();
//		}
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
// abortObservation
//
void ObservationControl::abortObservation()
{
	LOG_WARN("Received manual interrupt to ABORT the observation");
	itsQuitReason = (itsState < CTState::RESUME) ? CT_RESULT_MANUAL_REMOVED : CT_RESULT_MANUAL_ABORT;

	itsTimerPort->cancelTimer(itsStopTimer);	// cancel old timer
	itsStopTimer = itsTimerPort->setTimer(0.0);	// expire immediately
	// will result in F_TIMER in ::active_state
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
		itsPropertySet->setValue(string(PN_FSM_CURRENT_ACTION), GCFPVString(cts.name(newState)));

		// update the runstate field of the observation also
		// note: itsObsDPname = LOFAR_ObsSW_TempObs9999
		if (itsDPservice) {
			PVSSresult	result = itsDPservice->setValue(itsObsDPname+"."+PN_OBS_RUN_STATE, GCFPVString(cts.name(newState)));
			LOG_DEBUG_STR("Setting PVSSDP " << itsObsDPname+"."+PN_OBS_RUN_STATE << " to " << cts.name(newState));
			if (result != SA_NO_ERROR) {
				LOG_WARN_STR("Could not update runstate in PVSS of observation " << itsTreeID);
			}
		}

		string message(cts.name(newState));
		int    reportState(RTDB_OBJ_STATE_OFF);
		if (newState == CTState::QUITED && itsQuitReason != CT_RESULT_NO_ERROR) {
			switch (itsQuitReason) {
			case CT_RESULT_MANUAL_REMOVED: 	
				message = "Aborted by operator before observation started";
				break;
			case CT_RESULT_MANUAL_ABORT: 	
				message = "Aborted by operator during the observation";
				reportState = RTDB_OBJ_STATE_SUSPICIOUS;
				break;
			case CT_RESULT_LOST_CONNECTION:	
				message = "Lost connection(s)";
				reportState = RTDB_OBJ_STATE_BROKEN;
				break;
			default:
				message = "Unknown reason";
				reportState = RTDB_OBJ_STATE_BROKEN;
			}
		}
		else if (newState > CTState::CONNECT) {
			reportState = RTDB_OBJ_STATE_OPERATIONAL;
		}
		string reporterID (formatString("ObservationControl: %s: %s", getName().c_str(), message.c_str()));
		setObjectState(reporterID, itsObsDPname, reportState);
	}

	if (itsParentControl) {		// allow calling this function before parentControl is online
		itsParentControl->nowInState(getName(), newState);
	}
}


//
// registerResultMessage(...)
//
void ObservationControl::registerResultMessage(const string& cntlrName, int	result, CTState::CTstateNr	state)
{
	// always handle a quited-msg from a controller.
	if (state == CTState::QUITED) {
		_updateChildInfo(cntlrName, state);
		if (result != CT_RESULT_NO_ERROR) {
			itsQuitReason = result;
		}
	}

	// does the message belong to the current state?
	CTState		cts;
	CTState::CTstateNr	expectedState(cts.stateAck(itsState));
	if (state != expectedState) {
		if (state < expectedState) {
			LOG_INFO_STR("Controller " << cntlrName << " sent a late " << cts.name(state) << " message iso a " 
					<< cts.name(cts.stateAck(itsState)) << " message, ignored.");
		}
		else {
			LOG_WARN_STR("Controller " << cntlrName << " sent a " << cts.name(state) << " message iso a " 
					<< cts.name(cts.stateAck(itsState)) << " message.");
			itsBusyControllers--;	// [15122010] see note in doHeartBeatTask!
		}
		return;
	}

	LOG_DEBUG_STR("Received " << cts.name(state) << "(" << cntlrName << ",error=" << errorName(result) << ")");
	itsChildResult |= result;
	itsChildsInError += (result == CT_RESULT_NO_ERROR) ? 0 : 1;
	itsBusyControllers--;	// [15122010] see note in doHeartBeatTask!
	doHeartBeatTask();
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
		string	propSetName(createPropertySetName(PSN_OBSERVATION_CONTROL, getName(), itsObsDPname));
		LOG_DEBUG_STR ("Activating PropertySet: " << propSetName);
		itsPropertySet = new RTDBPropertySet(propSetName.c_str(),
											 PST_OBSERVATION_CONTROL,
											 PSAT_RW,
											 this);
		}
		break;
	  
	case DP_CREATED: {
			// NOTE: this function may be called DURING the construction of the PropertySet.
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
		signal (SIGABRT, ObservationControl::sigintHandler);	// kill -6

		// update PVSS.
		LOG_TRACE_FLOW ("Updateing state to PVSS");
		itsPropertySet->setValue(PN_FSM_CURRENT_ACTION, GCFPVString("Initial"));
		itsPropertySet->setValue(PN_FSM_ERROR,  GCFPVString(""));

		// Start ChildControl task
		LOG_DEBUG ("Enabling ChildControl task");
		itsChildControl->openService(MAC_SVCMASK_OBSERVATIONCTRL, itsTreeID);
		itsChildControl->registerCompletionPort(itsChildPort);

		// Start ParentControl task
		LOG_DEBUG ("Enabling ParentControl task");
		itsParentPort = itsParentControl->registerTask(this);

		// register what we are doing
		setState(CTState::CONNECT);

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
		// do some bookkeeping
		itsChildControl->startChildControllers();
		itsNrControllers = itsChildControl->countChilds(0, CNTLRTYPE_NO_TYPE);
		itsNrStations    = itsChildControl->countChilds(0, CNTLRTYPE_STATIONCTRL);
		itsNrOnlineCtrls = itsChildControl->countChilds(0, CNTLRTYPE_ONLINECTRL);
		itsNrOfflineCtrls= itsChildControl->countChilds(0, CNTLRTYPE_OFFLINECTRL);
		LOG_INFO(formatString ("Controlling: %d stations, %d onlinectrl, %d offlinectrl, %d other ctrls", 
			itsNrStations, itsNrOnlineCtrls, itsNrOfflineCtrls, 
			itsNrControllers-itsNrStations-itsNrOnlineCtrls-itsNrOfflineCtrls));
		_updateChildInfo();
		_showChildInfo();
		// convert times and periods to timersettings.
		setObservationTimers(1.0 * MAC_SCP_TIMEOUT);
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
			itsChildsInError = 0;
			itsClaimTimer    = 0;
			LOG_INFO("Requesting all childs to execute the CLAIM state");
			itsChildControl->requestState(CTState::CLAIMED, "");
			itsBusyControllers = itsChildControl->countChilds(0, CNTLRTYPE_NO_TYPE);
		}
		else if (timerEvent.id == itsPrepareTimer) {
			setState(CTState::PREPARE);
			itsChildResult   = CT_RESULT_NO_ERROR;
			itsChildsInError = 0;
			itsPrepareTimer  = 0;
			LOG_INFO("Requesting all childs to execute the PREPARE state");
			itsChildControl->requestState(CTState::PREPARED, "");
			itsBusyControllers = itsChildControl->countChilds(0, CNTLRTYPE_NO_TYPE);
		}
		else if (timerEvent.id == itsStartTimer) {
			setState(CTState::RESUME);
			itsChildResult   = CT_RESULT_NO_ERROR;
			itsChildsInError = 0;
			itsStartTimer    = 0;
			LOG_INFO("Requesting all childs to go operation state");
			itsChildControl->requestState(CTState::RESUMED, "");
			itsBusyControllers = itsChildControl->countChilds(0, CNTLRTYPE_NO_TYPE);
		}
		else if (timerEvent.id == itsStopTimer) {
			if (itsState == CTState::QUIT) {
				LOG_INFO("Re-entry of quit-phase, ignored.");
				break;
			}
			setState(CTState::QUIT);
			itsChildResult   = itsQuitReason;
			itsChildsInError = 0;
			itsStopTimer     = 0;
			LOG_INFO("Requesting all childs to quit");
			itsChildControl->requestState(CTState::QUITED, "");
			itsBusyControllers = itsChildControl->countChilds(0, CNTLRTYPE_NO_TYPE);
			// reschedule forced-quit timer for safety.
			itsTimerPort->cancelTimer(itsForcedQuitTimer);
			itsForcedQuitTimer = itsTimerPort->setTimer(1.0 * itsForcedQuitDelay);
			// cancel all other timers in case premature quit was requested.
			itsTimerPort->cancelTimer(itsStartTimer);
			itsTimerPort->cancelTimer(itsPrepareTimer);
			itsTimerPort->cancelTimer(itsClaimTimer);
		}
		else if (timerEvent.id == itsForcedQuitTimer) {
			LOG_WARN("QUITING BEFORE ALL CHILDREN DIED.");
			TRAN(ObservationControl::finishing_state);
		}
		// some other timer?

		break;
	}

	// -------------------- EVENT RECEIVED FROM PARENT CONTROL --------------------
	case CONTROL_CONNECT:
		LOG_INFO("Opening connection with parent controller");
		break;
	case CONTROL_QUIT: {
		LOG_INFO("Received manual request for shutdown, accepting it.");
		itsTimerPort->cancelTimer(itsStopTimer);	// cancel old timer
		itsStopTimer = itsTimerPort->setTimer(0.0);	// expire immediately
		break;
	}
	// ----- The next events from parent control are implemented for ControllerMenu ----
	case CONTROL_CLAIM:
		itsTimerPort->cancelTimer(itsClaimTimer);
		itsClaimTimer = itsTimerPort->setTimer(0.0);
		break;
	case CONTROL_PREPARE:
		itsTimerPort->cancelTimer(itsPrepareTimer);
		itsPrepareTimer = itsTimerPort->setTimer(0.0);
		break;
	case CONTROL_RESUME:
		itsTimerPort->cancelTimer(itsStartTimer);
		itsStartTimer = itsTimerPort->setTimer(0.0);
		break;
	case CONTROL_SUSPEND:	// Note: SUSPEND, RELEASE and QUIT will result in QUIT.
	case CONTROL_RELEASE:
		itsTimerPort->cancelTimer(itsStopTimer);
		itsStopTimer = itsTimerPort->setTimer(0.0);
		break;

	// -------------------- EVENT RECEIVED FROM CHILD CONTROL --------------------
	case CONTROL_STARTED: {
		CONTROLStartedEvent	msg(event);
		if (msg.successful) {
			LOG_INFO_STR("Start of " << msg.cntlrName << " was successful");
		}
		else {
			LOG_WARN_STR("Start of " << msg.cntlrName << " was NOT successful");
		}
		// TODO: do something usefull with this information!
		break;
	}

	case CONTROL_CONNECTED: {
		CONTROLConnectedEvent		msg(event);
		LOG_DEBUG_STR("Received CONNECTED(" << msg.cntlrName << ")");
		// TODO: do something usefull with this information!
//		CONTROLConnectedEvent	answer;
//		answer.cntlrName = msg.cntlrName;
//		answer.result = CT_RESULT_NO_ERROR;
//		itsParentPort->send(answer);
		msg.cntlrName = getName();
		itsParentPort->send(msg);
		break;
	}

	case CONTROL_SCHEDULED: {
		CONTROLScheduledEvent		msg(event);
		LOG_DEBUG_STR("Received SCHEDULED(" << msg.cntlrName << "),error=" << errorName(msg.result));
		// TODO: do something usefull with this information!
		break;
	}

	case CONTROL_CLAIMED:
	case CONTROL_PREPARED:
	case CONTROL_RESUMED:
	case CONTROL_SUSPENDED:
	case CONTROL_RELEASED:
	case CONTROL_QUITED: {
		CONTROLCommonAnswerEvent		msg(event);
		CTState		cts;
		registerResultMessage(msg.cntlrName, msg.result, cts.signal2stateNr(event.signal));
		break;
	}

	case DP_SET:
		break;

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
		itsStopTimer = 0;
		itsForcedQuitTimer = 0;

		// tell Parent task we like to go down.
		itsParentControl->nowInState(getName(), CTState::QUIT);
		setState(CTState::QUITED);

		// inform MACScheduler we are going down
		CONTROLQuitedEvent	msg;
		msg.cntlrName = getName();
		msg.result 	  = itsQuitReason;
		itsParentPort->send(msg);

		// update PVSS
		itsPropertySet->setValue(string(PN_FSM_CURRENT_ACTION),
						GCFPVString((itsQuitReason == CT_RESULT_NO_ERROR) ? "Finished" : "Aborted"));
		itsPropertySet->setValue(string(PN_FSM_ERROR),GCFPVString(""));

		itsTimerPort->setTimer(1L);	// give PVSS task some time to update the DB.
		break;
	}
  
    case F_TIMER:
      GCFScheduler::instance()->stop();
      break;
    
	default:
		LOG_DEBUG("finishing_state, default");
		status = GCFEvent::NOT_HANDLED;
		break;
	}    
	return (status);
}

//
// setObservationTimers(minimalDelay)
//
void ObservationControl::setObservationTimers(double	minimalDelay)
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
	itsClaimTimer = itsPrepareTimer = itsStartTimer = itsStopTimer = itsForcedQuitTimer = 0;

	// recalc new intervals
	int32	sec2claim   = start - now - itsPreparePeriod - itsClaimPeriod;
	int32	sec2prepare = start - now - itsPreparePeriod;
	int32	sec2start   = start - now;
	int32	sec2stop    = stop  - now;

	CTState::CTstateNr	assumedState(CTState::NOSTATE);

	// (re)set the claim timer
	if (itsState < CTState::CLAIM) { 				// claim state not done yet?
		if (sec2claim > 0) {
			itsClaimTimer = itsTimerPort->setTimer((sec2claim < minimalDelay) ? minimalDelay : 1.0 * sec2claim);
			LOG_INFO_STR ("Claimperiod starts over " << sec2claim << " seconds");
		}
		else {
			assumedState = CTState::CLAIM;
			LOG_INFO_STR ("Claimperiod started " << -sec2claim << " seconds AGO!");
		}
	}
		
	// (re)set the prepare timer
	if (itsState < CTState::PREPARE) { 				// prepare state not done yet?
		if (sec2prepare > 0) {
			itsPrepareTimer = itsTimerPort->setTimer((sec2prepare < minimalDelay) ? minimalDelay : 1.0 * sec2prepare);
			LOG_INFO_STR ("PreparePeriod starts over " << sec2prepare << " seconds");
		}
		else {
			assumedState = CTState::PREPARE;
			LOG_INFO_STR ("PreparePeriod started " << -sec2prepare << " seconds AGO!");
		}
	}

	// (re)set the start timer
	if (itsState < CTState::RESUME) { 				// not yet active?
		if (sec2start > 0) {
			itsStartTimer = itsTimerPort->setTimer((sec2start < minimalDelay) ? minimalDelay : 1.0 * sec2start);
			LOG_INFO_STR ("Observation starts over " << sec2start << " seconds");
		}
		else {
			assumedState = CTState::RESUME;
			LOG_INFO_STR ("Observation started " << -sec2start << " seconds AGO!");
		}
	}

	// (re)set the stop timer
	if (itsProcessType == "Pipeline") {		// QUICK FIX #3633
		LOG_INFO("NOT SETTING STOP_TIMERS BECAUSE WE ARE RUNNING A PIPELINE!");
	}
	else {
		if (itsState < CTState::RELEASE) { 				// not yet shutting down?
			if (sec2stop > 0) {
				itsStopTimer = itsTimerPort->setTimer((sec2stop < minimalDelay) ? minimalDelay : 1.0 * sec2stop);
				// make sure we go down 30 seconds after quit was requested.
				itsForcedQuitTimer = itsTimerPort->setTimer(sec2stop + (1.0 * itsForcedQuitDelay));
				LOG_INFO_STR ("Observation stops over " << sec2stop << " seconds");
			}
			else {
				assumedState = CTState::RELEASE;
				LOG_INFO_STR ("Observation should have been stopped " << -sec2start << 
								" seconds AGO!");
			}
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

	LOG_INFO_STR ("Observation ends at " << to_simple_string(itsStopTime));
}

//
//doHeartBeatTask()
//
void  ObservationControl::doHeartBeatTask()
{
	// re-init heartbeat timer.
	itsTimerPort->cancelTimer(itsHeartBeatTimer);
	itsHeartBeatTimer = itsTimerPort->setTimer(1.0 * itsHeartBeatItv);
	
	_updateChildInfo();
	_showChildInfo();

	// find out how many controllers are still busy.
	uint32	nrChilds 						   = itsChildControl->countChilds(0, CNTLRTYPE_NO_TYPE);
	vector<ChildControl::StateInfo>	lateCntlrs = itsChildControl->getPendingRequest("", 0, CNTLRTYPE_NO_TYPE);

	// check if enough stations are left too do our job.
	// TODO: add criteria to SAS database and test those iso this foolish criteria.
	if (nrChilds != itsNrControllers) {
		LOG_WARN_STR("Only " << nrChilds << " out of " << itsNrControllers << " controllers still available.");
		// if no more children left while we are not in the quit-phase
		uint32	nrStations = itsChildControl->countChilds(0, CNTLRTYPE_STATIONCTRL);
		time_t	now   = to_time_t(second_clock::universal_time());
		time_t	stop  = to_time_t(itsStopTime);
		if (!nrChilds || (now < stop && itsProcessType == "Observation" && !nrStations)) {
			if (itsProcessType == "Observation") {
				LOG_FATAL("Too less stations left, FORCING QUIT OF OBSERVATION");
				if (itsState < CTState::RESUME) {
					itsQuitReason = CT_RESULT_LOST_CONNECTION;
				}
			}
			else {
				LOG_INFO("Lost connection with last childcontroller, quiting...");
			}
			itsTimerPort->cancelTimer(itsStopTimer);
			itsStopTimer = itsTimerPort->setTimer(0.0);
			return;
		}
	}

#if 1
	// NOTE: [15122010] Sending respons when first child reached required state.
	// NOTE: [15122010] WHEN nrChilds = 1 EACH TIME WE COME HERE A REPLY IS SENT!!!!!
	if ((itsBusyControllers == nrChilds-1) && (itsLastReportedState != itsState)) {	// first reply received?
		CTState		cts;					// report that state is reached.
		LOG_INFO_STR("First controller reached required state " << cts.name(cts.stateAck(itsState)) << 
					 ", informing SAS although it is too early!");
		sendControlResult(*itsParentPort, cts.signal(cts.stateAck(itsState)), getName(), CT_RESULT_NO_ERROR);
		setState(cts.stateAck(itsState));
		itsLastReportedState = itsState;
	}
#endif

	LOG_TRACE_FLOW_STR("itsBusyControllers=" << itsBusyControllers);

	// all controllers up to date?
	if (lateCntlrs.empty()) {
		LOG_DEBUG_STR("All (" << nrChilds << ") controllers are up to date");
		if (itsState >= CTState::QUIT) {
			LOG_DEBUG_STR("Time for me to shutdown");
			TRAN(ObservationControl::finishing_state);
			return;
		}
#if 0 
		// NOTE: [15122010] When one (or more) stations failed to reach the new state the state is not
		//                  reported back to the MACScheduler, hence SAS is not updated...
		//                  For now we send the acknowledge as soon as the first child reaches the desired state.
		//                  See related code-changes in statemachine active_state.
		if (itsBusyControllers) {	// last time NOT all cntrls ready?
			CTState		cts;		// report that state is reached.
			setState(cts.stateAck(itsState));
			itsBusyControllers = 0;
			// inform Parent (ignore function-result)
			sendControlResult(*itsParentPort, cts.signal(itsState), getName(), itsChildResult);
		}
#endif
		return;
	}

	itsBusyControllers = lateCntlrs.size();
	LOG_DEBUG_STR (itsBusyControllers << " controllers are still out of sync");
}

//
// _updateChildInfo([name], [state])
// When name and state are filled the function acts as a setChildInfo function otherwise the
// information is adopted from the ChildControl task.
//
void ObservationControl::_updateChildInfo(const string& name, CTState::CTstateNr	state)
{
	// make sure that quited (and by ChildControl already removed) controllers are updatd also.
	if (!name.empty() && state != CTState::NOSTATE && itsChildInfo.find(name) != itsChildInfo.end()) {
		itsChildInfo[name].currentState = state;
		CTState	CTS; 
		LOG_DEBUG_STR("_updateChildInfo: " << name << " says it is in state " << CTS.name(state));
		return;
	}

	// get latest status info
	vector<ChildControl::StateInfo> childs = itsChildControl->getChildInfo(name, 0, CNTLRTYPE_NO_TYPE);
	int	nrChilds = childs.size();
	for (int i = 0; i < nrChilds; i++) {
		if (itsChildInfo.find(childs[i].name) == itsChildInfo.end()) {	// not in map already?
			itsChildInfo[childs[i].name] = 
				ChildProc(childs[i].cntlrType, childs[i].currentState, childs[i].requestedState, childs[i].requestTime);
		}
		else { // its in the map, update the info.
			itsChildInfo[childs[i].name].currentState   = (state != CTState::NOSTATE) ? state : childs[i].currentState;
			itsChildInfo[childs[i].name].requestedState = childs[i].requestedState;
			itsChildInfo[childs[i].name].requestTime    = childs[i].requestTime;
		}
	}

	// we might still have controllers that are already removed by the childcontrol because they closed
	// the connection. Update those also.
	map<string, ChildProc>::iterator	iter = itsChildInfo.begin();			// own admin
	map<string, ChildProc>::iterator	end  = itsChildInfo.end();
	vector<ChildControl::StateInfo>::const_iterator	vFirst = childs.begin();	// childcontrol admin
	vector<ChildControl::StateInfo>::const_iterator	vLast  = childs.end();
	while (iter != end) {
		// not in childcontrol info anymore?
		if (iter->second.currentState != CTState::QUITED) {
			vector<ChildControl::StateInfo>::const_iterator vIter = vFirst;
			while ((vIter != vLast) && (vIter->name != iter->first)) {
				vIter++;
			}
			if (vIter == vLast) {		// not found?
				LOG_INFO_STR(iter->first << " not in the ChildControl admin anymore, assuming it quited");
				iter->second.currentState = CTState::QUITED;
			}
		}
		iter++;
	}
}

//
// _showChildInfo()
//
void ObservationControl::_showChildInfo()
{
	if (!itsFullReport && !itsChangeReport) {
		return;
	}

	CTState		CTS;
	map<string, ChildProc>::iterator	iter = itsChildInfo.begin();
	map<string, ChildProc>::iterator	end  = itsChildInfo.end();
	time_t		now(time(0));
	while (iter != end) {
		ChildProc*	cp = &(iter->second);
		LOG_DEBUG_STR(iter->first<<":cur="<<cp->currentState<<",req="<<cp->requestedState<<",rep="
														<<cp->reportedState<<",late="<<now-cp->requestTime);
		//    always      OR     child not in requested state       OR     current state not yet reported
		if (itsFullReport || cp->currentState != cp->requestedState || cp->reportedState != cp->currentState) {
			string	stateName = CTS.name(cp->currentState);
			
			// Selection of loglineType is based on 'late', while starting up correct late for scp timeout
			// to prevent fault 'not responding' messages.
			int32	late      = now - cp->requestTime - (cp->requestedState <= CTState::CLAIMED ? MAC_SCP_TIMEOUT : 0);
			if (late > itsLateLimit && late < itsFailedLimit && cp->currentState < cp->requestedState) {
				LOG_WARN(formatString("%-35.35s: IS LATE, state= %-10.10s", iter->first.c_str(), stateName.c_str()));
			}
			else if (late > itsFailedLimit && cp->reportedState != cp->requestedState && cp->currentState < cp->requestedState) {
				LOG_FATAL(formatString("%-35.35s: IS NOT RESPONDING, state= %-10.10s", iter->first.c_str(), stateName.c_str()));
				cp->reportedState = cp->currentState;
			} 
			else {
				LOG_INFO(formatString("%-35.35s: %-10.10s", iter->first.c_str(), stateName.c_str()));
				if (cp->currentState == cp->requestedState) {
					cp->reportedState = cp->currentState;
				}
			}
		}
		iter++;
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
		if ((strstr(dpEvent.DPname.c_str(), PN_FSM_ERROR) != 0) ||
			(strstr(dpEvent.DPname.c_str(), PN_FSM_CURRENT_ACTION) != 0) ||
			(strstr(dpEvent.DPname.c_str(), PN_FSM_LOG_MSG) != 0)) {
			return;
		}
 
		// abort request?
		if (strstr(dpEvent.DPname.c_str(), PN_OBSCTRL_COMMAND) != 0) {
			string  command = ((GCFPVString*) (dpEvent.value._pValue))->getValue();
			if (command == "ABORT") {
				LOG_INFO("Received manual request for abort, accepting it.");
				if (itsState < CTState::RESUME) {
					itsQuitReason = CT_RESULT_MANUAL_ABORT;
				}
				itsTimerPort->cancelTimer(itsStopTimer);	// cancel old timer
				itsStopTimer = itsTimerPort->setTimer(0.0);	// expire immediately
			}
			return;
			LOG_INFO_STR ("Received unknown command " << command << ". Ignoring it.");
		}

		// When datapoint does not concern the observation itself, where are done
		string	observationDPname(itsObsDPname+".");
		if (!strstr(dpEvent.DPname.c_str(), observationDPname.c_str())) {
			return;
		}

		// Change of claim_period?
		if (strstr(dpEvent.DPname.c_str(), PN_OBS_CLAIM_PERIOD) != 0) {
			uint32  newVal = ((GCFPVInteger*) (dpEvent.value._pValue))->getValue();
			LOG_INFO_STR ("Changing ClaimPeriod from " << itsClaimPeriod << " to " << newVal);
			itsClaimPeriod = newVal;
			return;
		}

		// Change of prepare_period?
		else if (strstr(dpEvent.DPname.c_str(), PN_OBS_PREPARE_PERIOD) != 0) {
			uint32  newVal = ((GCFPVInteger*) (dpEvent.value._pValue))->getValue();
			LOG_INFO_STR ("Changing PreparePeriod from " << itsPreparePeriod << " to " << newVal);
			itsPreparePeriod = newVal;
			return;
		}

		// Change of start or stop time?
		if ((strstr(dpEvent.DPname.c_str(), PN_OBS_START_TIME) != 0)  || 
		    (strstr(dpEvent.DPname.c_str(), PN_OBS_STOP_TIME) != 0)) {
			string  newVal = ((GCFPVString*) (dpEvent.value._pValue))->getValue();
			ptime	newTime;
			try {
				newTime = time_from_string(newVal);
			}
			catch (std::exception&	e) {
				LOG_ERROR_STR(newVal << " is not a legal time!!!");
				return;
			}
			if (strstr(dpEvent.DPname.c_str(), PN_OBS_START_TIME) != 0) { 
				LOG_INFO_STR ("Changing startTime from " << to_simple_string(itsStartTime) << " to " << newVal);
				itsStartTime = newTime;
			}
			else {
				LOG_INFO_STR ("Changing stopTime from " << to_simple_string(itsStopTime) << " to " << newVal);
				itsStopTime = newTime;
			}
			setObservationTimers(0.0);
			LOG_DEBUG("NOT  YET  Sending all childs a RESCHEDULE event");
//			itsChildControl->rescheduleChilds(to_time_t(itsStartTime), 
//											  to_time_t(itsStopTime), "");
		}
	}  
	break;

	default:
		break;
	}   // switch(signal)
}

  }; // namespace MainCU
}; // namepsace LOFAR
