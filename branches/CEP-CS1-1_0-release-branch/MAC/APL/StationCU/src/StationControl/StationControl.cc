//	StationControl.cc: Implementation of the StationControl task
//
//	Copyright (C) 2006
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

#include <APS/ParameterSet.h>
#include <GCF/GCF_PVTypes.h>
#include <GCF/Utils.h>
#include <GCF/GCF_ServiceInfo.h>
#include <APL/APLCommon/APL_Defines.h>
#include <APL/APLCommon/ControllerDefines.h>
#include <APL/APLCommon/Controller_Protocol.ph>
#include <APL/RSP_Protocol/RSP_Protocol.ph>
#include <GCF/RTDB/DP_Protocol.ph>
#include <APL/APLCommon/StationInfo.h>
#include <signal.h>

#include "ActiveObs.h"
#include "StationControl.h"
#include "StationControlDefines.h"

using namespace LOFAR::GCF::Common;
using namespace LOFAR::GCF::TM;
using namespace LOFAR::GCF::RTDB;
using namespace std;

namespace LOFAR {
	using namespace APLCommon;
	using namespace ACC::APS;
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
	itsChildControl		(0),
	itsChildPort		(0),
	itsParentControl	(0),
	itsParentPort		(0),
	itsTimerPort		(0)
{
	LOG_TRACE_OBJ_STR (cntlrName << " construction");

	// Readin some parameters from the ParameterSet.
	itsTreePrefix = globalParameterSet()->getString("prefix");
	itsInstanceNr = globalParameterSet()->getUint32(itsTreePrefix + "instanceNr");

	// TODO
	LOG_INFO("MACProcessScope: LOFAR.PermSW.StationCtrl");

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

	// for debugging purposes
	GCF::TM::registerProtocol (CONTROLLER_PROTOCOL, CONTROLLER_PROTOCOL_STRINGS);
	GCF::TM::registerProtocol (DP_PROTOCOL, 		DP_PROTOCOL_STRINGS);
}


//
// ~StationControl()
//
StationControl::~StationControl()
{
	LOG_TRACE_OBJ_STR (getName() << " destruction");

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
// _databaseEventHandler(event)
//
void StationControl::_databaseEventHandler(GCFEvent& event)
{
	LOG_TRACE_FLOW_STR ("_databaseEventHandler:" << eventName(event));

	switch(event.signal) {
	case DP_CHANGED:  {
		DPChangedEvent		dpEvent(event);
		if (strstr(dpEvent.DPname.c_str(), PN_SC_CLOCK) != 0) {
			itsClock = ((GCFPVInteger*)(dpEvent.value._pValue))->getValue();
			LOG_DEBUG_STR("Received clock change from PVSS, clock is now " << itsClock);
			break;
		}

		// don't watch state and error fields.
		if ((strstr(dpEvent.DPname.c_str(), PVSSNAME_FSM_STATE) != 0) || 
			(strstr(dpEvent.DPname.c_str(), PVSSNAME_FSM_ERROR) != 0) ||
			(strstr(dpEvent.DPname.c_str(), PVSSNAME_FSM_CURACT) != 0) ||
			(strstr(dpEvent.DPname.c_str(), PVSSNAME_FSM_LOGMSG) != 0)) {
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
			itsTimerPort->setTimer(0.0);
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
			itsOwnPropSet->setValue(PVSSNAME_FSM_CURACT,GCFPVString("Initial"));
			itsOwnPropSet->setValue(PVSSNAME_FSM_ERROR,GCFPVString(""));

			// enable clock propertyset.
			string	clkPropSetName(createPropertySetName(PSN_STATION_CLOCK, getName()));
			LOG_DEBUG_STR ("Activating PropertySet " << clkPropSetName);
			itsClockPropSet = new RTDBPropertySet(clkPropSetName,
												  PST_STATION_CLOCK,
												  PSAT_WO,
												  this);
		}
		else {
			itsClockPSinitialized = true;
			LOG_DEBUG ("Attached to external propertySets");

			GCFPVInteger	clockVal;
			itsClockPropSet->getValue(PN_SC_CLOCK, clockVal);
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
		itsOwnPropSet->setValue(PVSSNAME_FSM_CURACT,GCFPVString("Connected"));

		// start DigitalBoardController
		LOG_DEBUG_STR("Starting DigitalBoardController");
		itsChildControl->startChild(CNTLRTYPE_DIGITALBOARDCTRL,
							   		0,			// treeID, 
							   		0,			// instanceNr,
							   		myHostname(false));
		// will result in CONTROL_STARTED (and CONTROL_CONNECTED if no error).
		}
		break;

	case CONTROL_STARTED: {
		CONTROLStartedEvent		msg(event);
//		ASSERTSTR(msg.cntlrName == controllerName(CNTLRTYPE_DIGITALBOARDCTRL, 0 ,0),
//							"Started event of unknown controller: " << msg.cntlrName);
// note: hostname is now in controller.
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
// note: hostname is now in controller.

		// inform parent the chain is up
		CONTROLConnectedEvent	answer;
		answer.cntlrName = getName();
		answer.result    = CT_RESULT_NO_ERROR;
		itsParentPort->send(answer);

		LOG_DEBUG ("Attached to DigitalBoardControl, enabling ParentControl task");
		itsParentPort = itsParentControl->registerTask(this);

		LOG_DEBUG ("All initialisation done, going to operational state");
		TRAN(StationControl::operational_state);			// go to next state.
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
		itsOwnPropSet->setValue(PVSSNAME_FSM_CURACT,GCFPVString("Active"));
		itsOwnPropSet->setValue(PVSSNAME_FSM_ERROR,GCFPVString(""));
	}
	break;

	case F_ACCEPT_REQ:
	case F_CONNECTED:
	case F_DISCONNECTED:
		break;

	case DP_CHANGED:
		_databaseEventHandler(event);
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
	// BeamCtlrs or CalCntrls of the active observations.
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
		// form the name and contruct the StationControl-name which we use for the
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
		if (event.signal == CONTROL_CLAIM && 
								itsClock != theObs->second->obsPar()->sampleClock) {
			itsClock = theObs->second->obsPar()->sampleClock;
			LOG_DEBUG_STR ("Changing clock to " << itsClock);
			itsClockPropSet->setValue(PN_SC_CLOCK,GCFPVInteger(itsClock));
			// TODO: give clock 5 seconds to stabelize
		}

		// before passing a new state request from the ObsController to the 
		// activeObs, make sure the last state is reached.
LOG_DEBUG_STR(formatString("event.signal = %04X", event.signal));
LOG_DEBUG_STR("F_INDIR = " << F_INDIR(event.signal));
LOG_DEBUG_STR("F_OUTDIR = " << F_OUTDIR(event.signal));
LOG_DEBUG_STR("inSync = " << theObs->second->inSync() ? "true" : "false");
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
		itsOwnPropSet->setValue(string(PVSSNAME_FSM_CURACT),GCFPVString("Finished"));
		itsOwnPropSet->setValue(string(PVSSNAME_FSM_ERROR),GCFPVString(""));

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
// addObservation(name)
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
	LOG_TRACE_OBJ_STR("Trying to readfile " << LOFAR_SHARE_LOCATION << "/" << name);
	theObsPS.adoptFile(string(LOFAR_SHARE_LOCATION) + "/" + name);

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
