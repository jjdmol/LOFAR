//	StationControl.cc: Implementation of the StationControl task
//
//	Copyright (C) 2006-2008
//	ASTRON (Netherlands Foundation for Research in Astronomy)
//	P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//
//	This program is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation; either version 2 of the License, or
//	(at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program; if not, write to the Free Software
//	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
//	One of the main task of the station controller is the synchronisation between
//	the DigitalBoardController, the CalibrationController and the BeamController.
//
//	$Id$
//
#include <lofar_config.h>
#include <Common/LofarLogger.h>
#include <Common/LofarLocators.h>
#include <Common/LofarConstants.h>
#include <Common/SystemUtil.h>
#include <Common/Version.h>

#include <Common/ParameterSet.h>
#include <GCF/PVSS/GCF_PVTypes.h>
#include <MACIO/MACServiceInfo.h>
#include <APL/APLCommon/APL_Defines.h>
#include <APL/APLCommon/bitsetUtil.tcc>
#include <APL/APLCommon/ControllerDefines.h>
#include <APL/APLCommon/Controller_Protocol.ph>
#include <APL/RSP_Protocol/RSP_Protocol.ph>
#include <GCF/RTDB/DP_Protocol.ph>
#include <GCF/RTDB/DPservice.h>
#include <APL/APLCommon/StationInfo.h>
#include <APL/RTDBCommon/RTDButilities.h>
#include <signal.h>

#include "ActiveObs.h"
#include "StationControl.h"
#include "PVSSDatapointDefs.h"
#include "../Package__Version.h"

using namespace LOFAR::GCF::TM;
using namespace LOFAR::GCF::PVSS;
using namespace LOFAR::GCF::RTDB;
using namespace LOFAR::APL::RTDBCommon;
using namespace std;

namespace LOFAR {
	using namespace APLCommon;
	namespace StationCU {

// static pointer to this object for signalhandler
static StationControl*	thisStationControl = 0;
	
//
// StationControl()
//
StationControl::StationControl(const string&	cntlrName) :
	GCFTask 			((State)&StationControl::initial_state,cntlrName),
	itsClockPropSet		(0),
	itsOwnPropSet		(0),
	itsClockPSinitialized(false),
	itsOwnPSinitialized (false),
	itsDPservice		(0),
	itsQueryID			(0),
	itsChildControl		(0),
	itsChildPort		(0),
	itsParentControl	(0),
	itsParentPort		(0),
	itsTimerPort		(0)
{
	LOG_TRACE_OBJ_STR (cntlrName << " construction");
	LOG_INFO(Version::getInfo<StationCUVersion>("StationControl"));

	// Readin some parameters from the ParameterSet.
	itsInstanceNr = globalParameterSet()->getUint32("instanceNr", 0);
	itsUseHWinfo  = globalParameterSet()->getBool("useHardwareStates", true);

	// TODO
	LOG_INFO("MACProcessScope: LOFAR.PermSW.StationCtrl");
	LOG_INFO_STR((itsUseHWinfo ? "Using" : "Ignoring") << " the hardware states in PVSS");

	// attach to child control task
	itsChildControl = ChildControl::instance();
	itsChildPort = new GCFITCPort (*this, *itsChildControl, "ChildITCport", 
									GCFPortInterface::SAP, CONTROLLER_PROTOCOL);
	ASSERTSTR(itsChildPort, "Cannot allocate ITCport for childcontrol");
	itsChildPort->open();		// will result in F_CONNECTED

	// attach to parent control task
	itsParentControl = ParentControl::instance();
	itsParentPort = new GCFITCPort (*this, *itsParentControl, "ParentITCport", 
									GCFPortInterface::SAP, CONTROLLER_PROTOCOL);
	ASSERTSTR(itsParentPort, "Cannot allocate ITCport for Parentcontrol");
	itsParentPort->open();		// will result in F_CONNECTED

	// need port for timers.
	itsTimerPort = new GCFTimerPort(*this, "TimerPort");

	// for doing PVSS queries
	itsDPservice = new DPservice(this);
	ASSERTSTR(itsDPservice, "Can't allocate DataPoint Service for PVSS");

	// for debugging purposes
	registerProtocol (CONTROLLER_PROTOCOL, CONTROLLER_PROTOCOL_STRINGS);
	registerProtocol (DP_PROTOCOL, 		DP_PROTOCOL_STRINGS);

	// reset some mappings.
	itsRCUmask.reset();
	itsTBmask.reset();

	LOG_DEBUG_STR("sizeof itsLBAmask: " << itsLBAmask.size());
	LOG_DEBUG_STR("sizeof itsHBAmask: " << itsHBAmask.size());
	LOG_DEBUG_STR("sizeof itsRCUmask: " << itsRCUmask.size());
	LOG_DEBUG_STR("sizeof itsTBmask: "  << itsTBmask.size());
}


//
// ~StationControl()
//
StationControl::~StationControl()
{
	LOG_TRACE_OBJ_STR (getName() << " destruction");

	if (itsDPservice) {
		if  (itsQueryID) {
			itsDPservice->cancelQuery(itsQueryID);
			itsQueryID = 0;
		}
		delete itsDPservice;
	}

	// ...
}

//
// sigintHandler(signum)
//
void StationControl::sigintHandler(int signum)
{
	LOG_DEBUG (formatString("SIGINT signal detected (%d)",signum));

	if (thisStationControl) {
		thisStationControl->finish();
	}
}

//
// finish
//
void StationControl::finish()
{
	TRAN(StationControl::finishing_state);
}



//
// initial_state(event, port)
//
// Setup connection with PVSS and load Property sets.
//
GCFEvent::TResult StationControl::initial_state(GCFEvent& event, 
													GCFPortInterface& port)
{
	LOG_DEBUG_STR ("initial:" << eventName(event) << "@" << port.getName());

	GCFEvent::TResult status = GCFEvent::HANDLED;
  
	switch (event.signal) {
    case F_INIT:
   		break;

	case F_ENTRY: {
		// Get access to my own propertyset.
		string	myPropSetName(createPropertySetName(PSN_STATION_CTRL, getName()));
		LOG_DEBUG_STR ("Activating PropertySet " << myPropSetName);
		itsOwnPropSet = new RTDBPropertySet(myPropSetName,
											PST_STATION_CTRL,
											PSAT_RW,
											this);
		// Wait for timer that is set in PropertySetAnswer on ENABLED event
		}
		break;

	case DP_CREATED: {
			// NOTE: thsi function may be called DURING the construction of the PropertySet.
			// Always exit this event in a way that GCF can end the construction.
			DPCreatedEvent	dpEvent(event);
			LOG_DEBUG_STR("Result of creating " << dpEvent.DPname << " = " << dpEvent.result);
			itsTimerPort->cancelAllTimers();
			itsTimerPort->setTimer(0.5);	// give RTDB time to get original value.
        }
		break;
	  
	case F_TIMER:
		// first timer event is from own propertyset
		if (!itsOwnPSinitialized) {
			itsOwnPSinitialized = true;

			// Enable Child task on time so we have some time to resolve the name
			LOG_DEBUG ("Enabling ChildControl task");
			itsChildControl->openService(MAC_SVCMASK_STATIONCTRL, itsInstanceNr);
			itsChildControl->registerCompletionPort(itsChildPort);

			// first redirect signalHandler to our finishing state to leave PVSS
			// in the right state when we are going down
			thisStationControl = this;
			signal (SIGINT,  StationControl::sigintHandler);	// ctrl-c
			signal (SIGTERM, StationControl::sigintHandler);	// kill

			// update PVSS.
			LOG_TRACE_FLOW ("Updateing state to PVSS");
			itsOwnPropSet->setValue(PN_FSM_CURRENT_ACTION,GCFPVString("Initial"));
			itsOwnPropSet->setValue(PN_FSM_ERROR,GCFPVString(""));

			// enable clock propertyset.
			string	clkPropSetName(createPropertySetName(PSN_STATION_CLOCK, getName()));
			LOG_DEBUG_STR ("Activating PropertySet " << clkPropSetName);
			itsClockPropSet = new RTDBPropertySet(clkPropSetName,
												  PST_STATION_CLOCK,
												  PSAT_RW,
												  this);
		}
		else {
			itsClockPSinitialized = true;
			LOG_DEBUG ("Attached to external propertySets");

			GCFPVInteger	clockVal;
			itsClockPropSet->getValue(PN_SCK_CLOCK, clockVal);
			itsClock = clockVal.getValue();
			LOG_DEBUG_STR("Clock in PVSS has value: " << itsClock);

			LOG_DEBUG ("Going to connect state to attach to DigitalBoardController");
			TRAN(StationControl::connect_state);			// go to next state.
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
		LOG_DEBUG_STR ("initial, default");
		status = GCFEvent::NOT_HANDLED;
		break;
	}    
	return (status);
}

//
// connect_state(event, port)
//
// Setup connection with DigitalBoardControl
//
GCFEvent::TResult StationControl::connect_state(GCFEvent& event, 
													GCFPortInterface& port)
{
	LOG_DEBUG_STR ("connect:" << eventName(event) << "@" << port.getName());

	GCFEvent::TResult status = GCFEvent::HANDLED;
  
	switch (event.signal) {
    case F_INIT:
   		break;

	case F_ENTRY: {
		itsOwnPropSet->setValue(PN_FSM_CURRENT_ACTION,GCFPVString("Connected"));

		// start DigitalBoardController
		LOG_DEBUG_STR("Starting DigitalBoardController");
		itsChildControl->startChild(CNTLRTYPE_DIGITALBOARDCTRL,
							   		0,			// treeID, force 'shared' by using 0
							   		0,			// instanceNr,
							   		myHostname(false));
		// will result in CONTROL_STARTED (and CONTROL_CONNECTED if no error).
		}
		break;

	case CONTROL_STARTED: {
		CONTROLStartedEvent		msg(event);
//		ASSERTSTR(msg.cntlrName == controllerName(CNTLRTYPE_DIGITALBOARDCTRL, 0 ,0),
//							"Started event of unknown controller: " << msg.cntlrName);
// note: need more complex test because the hostname is now in controllername.
			if (msg.successful) {
				LOG_INFO_STR("Startup of " << msg.cntlrName << 
											" succesful, waiting for connection");
			}
			else {
				LOG_WARN_STR("Startup of " << msg.cntlrName << "FAILED");
				// inform parent about the failure
				CONTROLConnectedEvent	answer;
				answer.cntlrName = getName();
				answer.result    = CT_RESULT_LOST_CONNECTION;
				itsParentPort->send(answer);
			}
		}
		break;

	case CONTROL_CONNECTED: {
		CONTROLConnectedEvent		msg(event);
//		ASSERTSTR(msg.cntlrName == controllerName(CNTLRTYPE_DIGITALBOARDCTRL, 0 ,0),
//							"Connect event of unknown controller: " << msg.cntlrName);
// note: need more complex test because the hostname is now in controllername.

		// inform parent the chain is up
		CONTROLConnectedEvent	answer;
		answer.cntlrName = getName();
		answer.result    = CT_RESULT_NO_ERROR;
		itsParentPort->send(answer);

		LOG_DEBUG ("Attached to DigitalBoardControl, taking a subscription on the hardware states");
		TRAN(StationControl::subscribe2HWstates);
		}
		break;

	case F_CONNECTED:
		break;

	case F_DISCONNECTED:
		port.close();
		break;
	
	case DP_CHANGED:
		_databaseEventHandler(event);
		break;

	default:
		LOG_DEBUG_STR ("connect, default");
		status = GCFEvent::NOT_HANDLED;
		break;
	}    
	return (status);
}

//
// subscribe2HWstates(event, port)
//
// Take a subscribtion to the states of the hardware so we can construct correct
// masks that reflect the availability of the LBA and HBA antenna's.
//
GCFEvent::TResult StationControl::subscribe2HWstates(GCFEvent& event, GCFPortInterface& port)
{
	LOG_DEBUG_STR("subscribe2HWstate:" << eventName(event) << "@" << port.getName());

	GCFEvent::TResult status = GCFEvent::HANDLED;
	switch (event.signal) {
	case F_ENTRY:
		// setup initial masks for the LBA and HBA antenna's
		_initAntennaMasks();
		// !!! NO BREAK !!! 

	case F_TIMER: {
		// take subscribtion on *.state
		LOG_DEBUG("Taking subscription on all state fields");
		PVSSresult  result = itsDPservice->query("'LOFAR_PIC_*.status.state'", "");
		if (result != SA_NO_ERROR) {
			LOG_ERROR ("Taking subscription on PVSS-states failed, retry in 10 seconds");
			itsTimerPort->setTimer(10.0);
			break;
		}
		// wait for DP event
		LOG_DEBUG ("Waiting for subscription answer");
	}
	break;

	case DP_QUERY_SUBSCRIBED: {
		DPQuerySubscribedEvent  answer(event);
		if (answer.result != SA_NO_ERROR) {
			LOG_ERROR_STR ("Taking subscription on PVSS-states failed (" << answer.result  <<
							"), retry in 10 seconds");
			itsTimerPort->setTimer(10.0);
			break;
		}
		itsQueryID = answer.QryID;
		LOG_INFO_STR("Subscription on state fields from PVSS successful(" << itsQueryID  <<
					"), going to operational mode");

		itsTimerPort->cancelAllTimers();
		TRAN(StationControl::operational_state);			// go to next state.
	}
	break;

	case DP_QUERY_CHANGED:
		// don't expect this event here right now, but you never know.
		_handleQueryEvent(event);
		itsTimerPort->cancelAllTimers();					// in case DP_QUERY_SUBSCRIBED is skipped.
		TRAN(StationControl::operational_state);			// go to next state.
	break;

	default:
		status = GCFEvent::NOT_HANDLED;
	break;
	}

	return (status);
}


//
// operational_state(event, port)
//
// Normal operation state, wait for events from parent task. 
//
GCFEvent::TResult StationControl::operational_state(GCFEvent& event, GCFPortInterface& port)
{
	LOG_DEBUG_STR ("operational:" << eventName(event) << "@" << port.getName());

	GCFEvent::TResult status = GCFEvent::HANDLED;

	switch (event.signal) {
	case F_INIT:
		break;

	case F_ENTRY: {
		// update PVSS
		LOG_DEBUG ("All initialisation done, enabling ParentControl task");
		itsParentPort = itsParentControl->registerTask(this);

		itsOwnPropSet->setValue(PN_FSM_CURRENT_ACTION,GCFPVString("Active"));
		itsOwnPropSet->setValue(PN_FSM_ERROR,GCFPVString(""));
	}
	break;

	case F_ACCEPT_REQ:
	case F_CONNECTED:
	case F_DISCONNECTED:
		break;

	case DP_CHANGED:
		_databaseEventHandler(event);
		break;

	case DP_QUERY_CHANGED:
		_handleQueryEvent(event);
	break;

	case F_TIMER:  {
		// try to map the timer on portname to an Observation
		ObsIter		theObs = itsObsMap.find(port.getName());
		if (theObs == itsObsMap.end()) {		// not found, try timerID
			GCFTimerEvent	timerEvt = static_cast<GCFTimerEvent&>(event);
			theObs = _searchObsByTimerID(timerEvt.id);
			if (theObs == itsObsMap.end()) {	// still not found?
				LOG_WARN_STR("Event for unknown observation: " << port.getName());
				break;
			}
			// found on timerID. This means that the Observation should have died
			// five seconds ago. Send a QUIT command.
			LOG_WARN_STR("Observation " << theObs->second->getName() << 
						" should have died by now, sending an extra QUIT command.");
			CONTROLQuitEvent	quitevent;
			quitevent.cntlrName = theObs->second->getName();
			theObs->second->dispatch(quitevent, port);
		}

		// pass event to observation FSM
		LOG_TRACE_FLOW("Dispatch to observation FSM's");
		theObs->second->dispatch(event, port);
		LOG_TRACE_FLOW("Back from dispatch");
	}
	break;

	// -------------------- EVENTS RECEIVED FROM PARENT CONTROL ----------------

	// A new observationcontroller has connected, create a new ActiveObs to
	// handle the admin of this observation.
	case CONTROL_CONNECT: {
		CONTROLConnectEvent		msg(event);
		CONTROLConnectedEvent	answer;
		answer.cntlrName = msg.cntlrName;
		// add observation to the list if not already in the list
		answer.result = _addObservation(msg.cntlrName);
		if (answer.result != CT_RESULT_NO_ERROR) {
			port.send(answer);
		}
	}
	break;

	// ------------ EVENTS RECEIVED FROM CHILD AND PARENT CONTROL --------------

	// The events from the child-task may be of the DigBoardCtlr or one of the
	// BeamCtlrs or CalCtrls of the active observations.
	case CONTROL_CONNECTED:		// from ChildITCport
	case CONTROL_SCHEDULED:
	case CONTROL_CLAIMED:
	case CONTROL_PREPARED:
	case CONTROL_RESUMED:
	case CONTROL_SUSPENDED:
	case CONTROL_RELEASED:
	case CONTROL_QUITED:
	// The next events are from one of the ObservationControllers.
	case CONTROL_CLAIM:			// from ParentITCport
	case CONTROL_SCHEDULE:
	case CONTROL_PREPARE:
	case CONTROL_RESUME:
	case CONTROL_SUSPEND:
	case CONTROL_RELEASE:
	case CONTROL_QUIT: {
		// All the events have the problem that the controller can be StationControl,
		// CalibrationControl or BeamControl. But what they all have in common is
		// the instancenumber and treeID. So we can substract instanceNr and treeID 
		// from the name and contruct the StationControl-name which we use for the
		// registration of the Active Obs.
		CONTROLCommonEvent	ObsEvent(event);		// we just need the name
		uint16			 instanceNr = getInstanceNr(ObsEvent.cntlrName);
		OTDBtreeIDType	 treeID	    = getObservationNr(ObsEvent.cntlrName);
		string			 cntlrName  = controllerName(CNTLRTYPE_STATIONCTRL, 
															instanceNr, treeID);
		ObsIter			 theObs     = itsObsMap.find(cntlrName);
		if (theObs == itsObsMap.end()) {
			LOG_WARN_STR("Event for unknown observation: " << ObsEvent.cntlrName);
			break;
		}

		// Clock changes are done in the claim state and require an extra action
		// TODO: CHECK ALL OTHERS OBSs ARE DOWN OTHERWISE WE MAY NOT SWITCH THE CLOCK
		if (event.signal == CONTROL_CLAIM && 
								itsClock != theObs->second->obsPar()->sampleClock) {
			itsClock = theObs->second->obsPar()->sampleClock;
			LOG_DEBUG_STR ("Changing clock to " << itsClock);
			itsClockPropSet->setValue(PN_SCK_CLOCK,GCFPVInteger(itsClock));
			// TODO: give clock 5 seconds to stabelize
		}

		// before passing a new state request from the ObsController to the 
		// activeObs, make sure the last state is reached.
LOG_DEBUG_STR(formatString("event.signal = %04X", event.signal));
LOG_DEBUG_STR("F_INDIR = " << F_INDIR(event.signal));
LOG_DEBUG_STR("F_OUTDIR = " << F_OUTDIR(event.signal));
LOG_DEBUG_STR("inSync = " << (theObs->second->inSync() ? "true" : "false"));
#if 0
		if (F_OUTDIR(event.signal) && !theObs->second->inSync()) {
			// TODO
			CTState		cts;
			LOG_FATAL_STR("Ignoring change to state " << 
						cts.name(cts.signal2stateNr(event.signal)) << 
						" for observation " << treeID << 
						" because obs is still in state " << 
						cts.name(theObs->second->curState()));
			sendControlResult(*itsParentPort, event.signal, cntlrName, 
																CT_RESULT_OUT_OF_SYNC);
			break;
			
		}
#endif
		// pass event to observation FSM
		LOG_TRACE_FLOW("Dispatch to observation FSM's");
		theObs->second->dispatch(event, port);

		// end of FSM?
		if (event.signal == CONTROL_QUITED && theObs->second->isReady()) {
			LOG_DEBUG_STR("Removing " <<ObsEvent.cntlrName<< " from the administration");
			itsTimerPort->cancelTimer(theObs->second->itsStopTimerID);
			delete theObs->second;
			itsObsMap.erase(theObs);
		}

		// check if all actions for this event are finished.
		vector<ChildControl::StateInfo>	cntlrStates = 
									itsChildControl->getPendingRequest("", treeID);
LOG_TRACE_FLOW_STR("There are " << cntlrStates.size() << " busy controllers");
		if (cntlrStates.empty()) {	// no pending requests? Ready.
			if (event.signal != CONTROL_QUITED) {
				sendControlResult(*itsParentPort, event.signal, cntlrName, 
																CT_RESULT_NO_ERROR);
			}
			else {
				// we are done, pass finish request to parent
				CONTROLQuitedEvent		request;
				request.cntlrName = cntlrName;
				request.result	  = CT_RESULT_NO_ERROR;
				itsParentPort->send(request);
			}
			break;
		}
		// Show where we are waiting for. When error occured, report it back and stop
		for (uint i = 0; i < cntlrStates.size(); i++) {
			if (cntlrStates[i].result != CT_RESULT_NO_ERROR) {
				LOG_ERROR_STR("Controller " << cntlrStates[i].name << 
							  " failed with error " << cntlrStates[i].result);
				sendControlResult(*itsParentPort, event.signal, cntlrName, 
														cntlrStates[i].result);
				break;
			}
			LOG_TRACE_COND_STR ("Still waiting for " << cntlrStates[i].name);
		} // for
	}
	break;

	default:
		LOG_DEBUG("active_state, default");
		status = GCFEvent::NOT_HANDLED;
	break;
	} // switch

	return (status);
}

//
// finishing_state(event, port)
//
// Write controller state to PVSS, wait for 1 second (using a timer) to let 
// GCF handle the property change and close the controller
//
GCFEvent::TResult StationControl::finishing_state(GCFEvent& event, GCFPortInterface& port)
{
	LOG_DEBUG_STR ("finishing_state:" << eventName(event) << "@" << port.getName());

	GCFEvent::TResult status = GCFEvent::HANDLED;

	switch (event.signal) {
	case F_ENTRY: {
		// tell Parent task we like to go down.
		itsParentControl->nowInState(getName(), CTState::QUIT);

		// Inform parent process we are going down.
		CONTROLQuitedEvent		msg;
		msg.cntlrName = getName();
		msg.result	  = CT_RESULT_NO_ERROR;
		msg.treeID    = 0;
		msg.errorMsg  = "";
		itsParentPort->send(msg);

		// update PVSS
		itsOwnPropSet->setValue(PN_FSM_CURRENT_ACTION, GCFPVString("Finished"));
		itsOwnPropSet->setValue(PN_FSM_ERROR,    GCFPVString(""));

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
// _databaseEventHandler(event)
//
void StationControl::_databaseEventHandler(GCFEvent& event)
{
	LOG_TRACE_FLOW_STR ("_databaseEventHandler:" << eventName(event));

	switch(event.signal) {
	case DP_CHANGED:  {
		DPChangedEvent		dpEvent(event);
		if (strstr(dpEvent.DPname.c_str(), PN_SCK_CLOCK) != 0) {
			itsClock = ((GCFPVInteger*)(dpEvent.value._pValue))->getValue();
			LOG_DEBUG_STR("Received clock change from PVSS, clock is now " << itsClock);
			break;
		}

		// don't watch state and error fields.
		if ((strstr(dpEvent.DPname.c_str(), PN_OBJ_STATE) != 0) || 
			(strstr(dpEvent.DPname.c_str(), PN_FSM_ERROR) != 0) ||
			(strstr(dpEvent.DPname.c_str(), PN_FSM_CURRENT_ACTION) != 0) ||
			(strstr(dpEvent.DPname.c_str(), PN_FSM_LOG_MSG) != 0)) {
			return;
		}
 
		LOG_WARN_STR("Got VCHANGEMSG signal from unknown property " << dpEvent.DPname);
	}
	break;

	default:
		break;
	}  
}

//
// _handleQueryEvent(event)
//
void StationControl::_handleQueryEvent(GCFEvent& event)
{
	LOG_TRACE_FLOW_STR ("_handleQueryEvent:" << eventName(event));
	
	DPQueryChangedEvent		DPevent(event);
	if (DPevent.result != SA_NO_ERROR) {
		LOG_ERROR_STR("PVSS reported error " << DPevent.result << " for query " << itsQueryID << 
				", cannot determine actual state of the hardware! Assuming everything is available.");
		return;
	}

	// remember QueryID if not yet set
	if (!itsQueryID) {
		itsQueryID = DPevent.QryID;
	}

	// The selected datapoints are delivered with full PVSS names, like:
	// CS001:LOFAR_PIC_Cabinet0_Subrack0_RSPBoard0_RCU5.status.state
	// Each event may contain more than one DP.
	int     nrDPs = ((GCFPVDynArr*)(DPevent.DPnames._pValue))->getValue().size();
	GCFPVDynArr*    DPnames  = (GCFPVDynArr*)(DPevent.DPnames._pValue);
	GCFPVDynArr*    DPvalues = (GCFPVDynArr*)(DPevent.DPvalues._pValue);
	// GCFPVDynArr*    DPtimes  = (GCFPVDynArr*)(DPevent.DPtimes._pValue);

	for (int    idx = 0; idx < nrDPs; ++idx) {
		// Get the name and figure out what circuitboard we are talking about
		string  nameStr(DPnames->getValue()[idx]->getValueAsString());				// DP name
		uint32	newState(((GCFPVInteger*)(DPvalues->getValue()[idx]))->getValue());	// value
		size_t	pos;

		uint32	modeOff(objectStateIndex2Value(RTDB_OBJ_STATE_OFF));
		uint32	modeOperational(objectStateIndex2Value(RTDB_OBJ_STATE_OPERATIONAL));
		// test for RCU
		if ((pos = nameStr.find("_RCU")) != string::npos) {
			int		rcu;
			if (sscanf(nameStr.substr(pos).c_str(), "_RCU%d.status.state", &rcu) != 1) {
				LOG_ERROR_STR("Cannot determine address of " << nameStr << 
								". AVAILABILITY OF ANTENNA'S MIGHT NOT BE UP TO DATE ANYMORE");
				continue;
			}
			LOG_INFO_STR("New state of RCU " << rcu << " is " << newState);
			// RCU's in de mode OFF and OPERATIONAL may be used in observations.
			if (newState == modeOff || newState == modeOperational) {
				itsRCUmask.set(rcu);
			}
			else {	// all other modes
				itsRCUmask.reset(rcu);
			}
		}

		// test for RSPBoard
		else if ((pos = nameStr.find("_RSPBoard")) != string::npos) {
			int		rsp;
			if (sscanf(nameStr.substr(pos).c_str(), "_RSPBoard%d.status.state", &rsp) != 1) {
				LOG_ERROR_STR("Cannot determine address of " << nameStr << 
								". AVAILABILITY OF ANTENNA'S MIGHT NOT BE UP TO DATE ANYMORE");
				continue;
			}
			LOG_INFO_STR("New state of RSPBoard " << rsp << " is " << newState);
			int rcubase = rsp * NR_RCUS_PER_RSPBOARD;
			for (int i = 0; i < NR_RCUS_PER_RSPBOARD; i++) {
				if (newState != RTDB_OBJ_STATE_OPERATIONAL) {
					itsRCUmask.reset(rcubase + i);
				}
				else {
					itsRCUmask.set(rcubase + i);
				}
			}
		}

		// test for TBBoard
		else if ((pos = nameStr.find("_TBBoard")) != string::npos) {
			int		tbb;
			if (sscanf(nameStr.substr(pos).c_str(), "_TBBoard%d.status.state", &tbb) != 1) {
				LOG_ERROR_STR("Cannot determine address of " << nameStr << 
								". AVAILABILITY OF TBBOARD'S MIGHT NOT BE UP TO DATE ANYMORE");
				continue;
			}
			LOG_INFO_STR("New state of TBBoard " << tbb << " is " << newState);
			if (newState != RTDB_OBJ_STATE_OPERATIONAL) {
				itsTBmask.reset(tbb);
			}
			else {
				itsTBmask.set(tbb);
			}
		}

		else {
			LOG_DEBUG_STR("State of unknown component received: " << nameStr);
		}
	} // for

	_updateAntennaMasks();	// translate new RCU mask to the LBA and HBA masks.
}


//
// _addObservation(name)
//
uint16 StationControl::_addObservation(const string&	name)
{
	// Already in admin? Return error.
	if (itsObsMap.find(name) != itsObsMap.end()) {
		LOG_WARN_STR(name << " already in admin, returning error");
		return (CT_RESULT_ALREADY_REGISTERED);
	}

	// find and read parameterset of this observation
	ParameterSet	theObsPS;
	string			filename(string(LOFAR_SHARE_LOCATION) + "/" + name);
	LOG_DEBUG_STR("Trying to readfile " << filename);
	theObsPS.adoptFile(filename);
	Observation		theObs(&theObsPS);
LOG_DEBUG_STR("theOBS=" << theObs);

	// Create a bitset containing the available receivers for this oberservation.
	Observation::RCUset_t	realReceivers = Observation(&theObsPS).RCUset;
	// apply the current state of the hardware to the desired selection when user likes that.
	if (itsUseHWinfo) {
		realReceivers &= itsRCUmask;
		// Write the corrected set back into the ParameterSetfile.
		string prefix = theObsPS.locateModule("Observation") + "Observation.";
		// save original under different name (using 'replace' is 'add' w. simplified testing)
		theObsPS.replace(prefix+"originalReceiverList", theObs.receiverList);
		// replace orignal
		theObsPS.replace(prefix+"receiverList", bitset2CompactedArrayString(realReceivers));
		// modify base parsetfile and my own parsetfile too, to prevent confusion
		theObsPS.writeFile(observationParset(theObs.obsID));
		theObsPS.writeFile(filename);
	}
	LOG_INFO_STR("Available receivers for observation " << theObs.obsID << ":" << 
				string(realReceivers.to_string<char,char_traits<char>,allocator<char> >()));

	// create an activeObservation object that will manage the child controllers.
	ActiveObs*	theNewObs = new ActiveObs(name, (State)&ActiveObs::initial, &theObsPS, *this);
	if (!theNewObs) {
		LOG_FATAL_STR("Unable to create the Observation '" << name << "'");
		return (CT_RESULT_UNSPECIFIED);
	}

	LOG_DEBUG_STR("Adding " << name << " to administration");
	itsObsMap[name] = theNewObs;
	LOG_DEBUG_STR(*theNewObs);
	theNewObs->start();				// call initial state.

	// Start a timer that while expire 5 seconds after stoptime.
	time_t	stopTime = to_time_t(time_from_string(theObsPS.getString("Observation.stopTime")));
	time_t	now		 = to_time_t(second_clock::universal_time());
	theNewObs->itsStopTimerID = itsTimerPort->setTimer(now-stopTime+5);
	
	return (CT_RESULT_NO_ERROR);
}

//      
// _initAntennaMasks
//      
// setup masks that contain the available antennas.
//      
void StationControl::_initAntennaMasks()
{
	// reset all variables
	itsLBAmask.reset();
	itsHBAmask.reset();

	// Try to find the configurationfile
	ConfigLocator   CL;
	string          fileName(CL.locate("RemoteStation.conf"));
	if (fileName.empty()) {
		LOG_ERROR("Cannot find file 'RemoteStation.conf', assuming I have 96 antennas");
		itsNrLBAs = 96;
		itsNrHBAs = 96;
	}   
	else {  // read info from configfile
		ParameterSet    StationInfo(fileName);
		int defaultValue = 4 * StationInfo.getUint32("RS.N_RSPBOARDS", 24);   // key must be present for RSPDriver.
		itsNrLBAs = StationInfo.getUint32("RS.N_LBAS", defaultValue);          // step5 key
		itsNrHBAs = StationInfo.getUint32("RS.N_HBAS", defaultValue);          // step5 key
	}

	LOG_INFO(formatString("Stations has %d LBA and %d HBA antennas", itsNrLBAs, itsNrHBAs));
	ASSERTSTR (itsNrLBAs <= itsLBAmask.size() && 
			   itsNrHBAs <= itsHBAmask.size(), "Number of antennas exceed expected count");

	// set the right bits.
	for (uint i = 0; i < itsNrLBAs; i++) {
		itsLBAmask.set(i);
	}   
	for (uint i = 0; i < itsNrHBAs; i++) {
		itsHBAmask.set(i);
	}

	// The masks are now initialized with the static information. The _handleQueryEvent routine
	// corrects the sets with the life information from PVSS. We assume that PVSS always has the 
	// correct and latest state.
}       

//
// _updateAntennaMasks()
//
// Translates the RCU mask to the LBA and HBA masks.
// This routine is familiar with mapping of the antennas on the RCU's.
//
void StationControl::_updateAntennaMasks()
{
	// Note: the definition in StationControl.h and the ASSERT in _initAntennaMasks assure
	//		 that we never exceed the boundaries of the bitmaps here.
	for (int rcu = 0; rcu < MAX_RCUS ; rcu+=2) {
		if (itsRCUmask[rcu] && itsRCUmask[rcu+1]) {		// X and Y
			itsLBAmask.set(rcu/2);
			itsLBAmask.set(48+(rcu/2));
			itsHBAmask.set(rcu/2);
		}
		else {
			itsLBAmask.reset(rcu/2);
			itsLBAmask.reset(48+(rcu/2));
			itsHBAmask.reset(rcu/2);
		}
	}
	LOG_DEBUG_STR("itsRCU:" << string(itsRCUmask.to_string<char,char_traits<char>,allocator<char> >()));
	LOG_DEBUG_STR("itsLBA:" << string(itsLBAmask.to_string<char,char_traits<char>,allocator<char> >()));
	LOG_DEBUG_STR("itsHBA:" << string(itsHBAmask.to_string<char,char_traits<char>,allocator<char> >()));
}

//
// _searchObsByTimerID(timerID)
//
StationControl::ObsIter StationControl::_searchObsByTimerID(uint32	timerID)
{
	ObsIter		iter(itsObsMap.begin());
	ObsIter		end (itsObsMap.end());

	while (iter != end) {
		if (iter->second->itsStopTimerID == timerID)
			return (iter);
		++iter;
	}

	return (iter);
}


//
// _disconnectedHandler(port)
//
void StationControl::_disconnectedHandler(GCFPortInterface& port)
{
	port.close();
}


}; // StationCU
}; // LOFAR
