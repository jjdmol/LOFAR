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
#include <APL/APLCommon/APLCommonExceptions.h>
#include <APL/APLCommon/Controller_Protocol.ph>
#include <APL/APLCommon/StationInfo.h>

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
	LOG_INFO_STR("MACProcessScope: " << itsTreePrefix + cntlrName);

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

	if (itsPropertySet) {
		itsPropertySet->setValue(string(PVSSNAME_FSM_STATE),GCFPVString("down"));
		itsPropertySet->disable();
	}

	// ...
}

//
// setState(CTstateNr)
//
void	ObservationControl::setState(CTState::CTstateNr		newState)
{
	itsState = newState;

	CTState		cts;
	LOG_DEBUG_STR(getName() << " now in state " << cts.name(newState));

	if (itsPropertySet) {
		itsPropertySet->setValue(string(PVSSNAME_FSM_STATE),
									 GCFPVString(cts.name(newState)));
	}

}

//
// handlePropertySetAnswer(answer)
//
void ObservationControl::handlePropertySetAnswer(GCFEvent& answer)
{
	LOG_DEBUG_STR ("handlePropertySetAnswer:" << evtstr(answer));

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
		// check which property changed
		GCFPropValueEvent* pPropAnswer=static_cast<GCFPropValueEvent*>(&answer);

		string PropSetName = createPropertySetName(PSN_OBS_CTRL, getName());
		if (strstr(pPropAnswer->pPropName, PropSetName.c_str()) == 0) {
			break;
		}

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
		else if	(pPropAnswer->pValue->getType() == LPT_STRING) {
			string  newVal = (string) ((GCFPVString*)pPropAnswer->pValue)->getValue();
			ptime	newTime = time_from_string(newVal);
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
		break;
	}  

//	case F_SUBSCRIBED:
//	case F_UNSUBSCRIBED:
//	case F_PS_CONFIGURED:
//	case F_EXTPS_LOADED:
//	case F_EXTPS_UNLOADED:
//	case F_MYPS_ENABLED:
//	case F_MYPS_DISABLED:
//	case F_VGETRESP:
//	case F_VCHANGEMSG:
//	case F_SERVER_GONE:

	default:
		break;
	}  
}


//
// initial_state(event, port)
//
// Setup all connections.
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
		// Get access to my own propertyset.
		string	propSetName(createPropertySetName(PSN_OBS_CTRL, getName()));
		LOG_DEBUG_STR ("Activating PropertySet: " << propSetName);
		itsPropertySet = GCFMyPropertySetPtr(new GCFMyPropertySet(propSetName.c_str(),
																  PST_OBS_CTRL,
																  PS_CAT_TEMPORARY,
																  &itsPropertySetAnswer));
		itsPropertySet->enable();
		// Wait for timer that is set in PropertySetAnswer on ENABLED event
		}
		break;
	  
	case F_TIMER: {		// must be timer that PropSet has set.
		// update PVSS.
		LOG_TRACE_FLOW ("Updateing state to PVSS");
		itsPropertySet->setValue(string(PVSSNAME_FSM_STATE),GCFPVString  ("initial"));
		itsPropertySet->setValue(string(PVSSNAME_FSM_ERROR),GCFPVString  (""));
      
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
		LOG_DEBUG_STR ("initial, default");
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
			itsClaimTimer = 0;
			LOG_DEBUG("Requesting all childs to execute the CLAIM state");
			itsChildControl->requestState(CTState::CLAIMED, "");
		}
		else if (timerEvent.id == itsPrepareTimer) {
			setState(CTState::PREPARE);
			itsPrepareTimer = 0;
			LOG_DEBUG("Requesting all childs to execute the PREPARE state");
			itsChildControl->requestState(CTState::PREPARED, "");
		}
		else if (timerEvent.id == itsStartTimer) {
			setState(CTState::RESUME);
			itsStartTimer = 0;
			LOG_DEBUG("Requesting all childs to go operation state");
			itsChildControl->requestState(CTState::RESUMED, "");
		}
		else if (timerEvent.id == itsStopTimer) {
			setState(CTState::FINISH);
			itsStopTimer = 0;
			LOG_DEBUG("Requesting all childs to quit");
			itsChildControl->requestState(CTState::FINISHED, "");
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
		// TODO: do something usefull with this information!	--> OE 837
		break;
	}

	case CONTROL_PREPARED: {
		CONTROLPreparedEvent		msg(event);
		LOG_DEBUG_STR("Received PREPARED(" << msg.cntlrName << ")");
		// TODO: do something usefull with this information!	--> OE 837
		break;
	}

	case CONTROL_RESUMED: {
		CONTROLResumedEvent		msg(event);
		LOG_DEBUG_STR("Received RESUMED(" << msg.cntlrName << ")");
		// TODO: do something usefull with this information!	--> OE 837
		break;
	}

	case CONTROL_SUSPENDED: {
		CONTROLSuspendedEvent		msg(event);
		LOG_DEBUG_STR("Received SUSPENDED(" << msg.cntlrName << ")");
		// TODO: do something usefull with this information!	--> OE 837
		break;
	}

	case CONTROL_RELEASED: {
		CONTROLReleasedEvent		msg(event);
		LOG_DEBUG_STR("Received RELEASED(" << msg.cntlrName << ")");
		// TODO: do something usefull with this information!	--> OE 837
		break;
	}

	case CONTROL_FINISH: {
		CONTROLFinishEvent		msg(event);
		LOG_DEBUG_STR("Received FINISH(" << msg.cntlrName << ")");
		CONTROLFinishedEvent	answer;
		answer.cntlrName = msg.cntlrName;
		answer.result    = CT_RESULT_NO_ERROR;
		port.send(answer);
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

	LOG_DEBUG_STR ("Observation ends over " << (stop-now)/60 << " minutes");
}

//
//doHeartBeatTask()
//
void  ObservationControl::doHeartBeatTask()
{
	itsHeartBeatTimer = itsTimerPort->setTimer(1.0 * itsHeartBeatItv);
	
	uint32	nrChilds = itsChildControl->countChilds(0, CNTLRTYPE_NO_TYPE);
	vector<ChildControl::StateInfo>		lateCntlrs = 
						itsChildControl->getPendingRequest("", 0, CNTLRTYPE_NO_TYPE);
	if (lateCntlrs.empty()) {
		LOG_DEBUG_STR("All (" << nrChilds << ") controllers are up to date");
		return;
	}

	LOG_DEBUG_STR (lateCntlrs.size() << " controllers are still out of sync");
	CTState		cts;
	for (uint32 i = 0; i < lateCntlrs.size(); i++) {
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
