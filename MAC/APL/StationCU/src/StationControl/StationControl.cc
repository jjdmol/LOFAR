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

#include <boost/shared_array.hpp>
#include <APS/ParameterSet.h>
#include <GCF/GCF_PVTypes.h>
#include <GCF/PAL/GCF_PVSSInfo.h>
#include <GCF/PAL/GCF_Answer.h>
#include <GCF/Utils.h>
#include <GCF/GCF_ServiceInfo.h>
#include <GCF/Protocols/PA_Protocol.ph>
#include <APL/APLCommon/APL_Defines.h>
#include <APL/APLCommon/APLCommonExceptions.h>
#include <APL/APLCommon/ControllerDefines.h>
#include <APL/APLCommon/Controller_Protocol.ph>
#include <APL/RSP_Protocol/RSP_Protocol.ph>
#include <APL/APLCommon/StationInfo.h>
#include <signal.h>

#include "ActiveObs.h"
#include "StationControl.h"
#include "StationControlDefines.h"

using namespace LOFAR::GCF::Common;
using namespace LOFAR::GCF::TM;
using namespace LOFAR::GCF::PAL;
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
	PropertySetAnswerHandlerInterface(),
	itsPropertySetAnswer(*this),
	itsClockPropSet		(),
	itsOwnPropSet		(),
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
	registerProtocol (CONTROLLER_PROTOCOL, CONTROLLER_PROTOCOL_signalnames);
	registerProtocol (PA_PROTOCOL, 		   PA_PROTOCOL_signalnames);
	registerProtocol (F_PML_PROTOCOL, 	   F_PML_PROTOCOL_signalnames);
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
// handlePropertySetAnswer(answer)
//
void StationControl::handlePropertySetAnswer(GCFEvent& answer)
{
	LOG_TRACE_FLOW_STR ("handlePropertySetAnswer:" << eventstr(answer));

	switch(answer.signal) {
	case F_MYPS_ENABLED: 
	case F_EXTPS_LOADED: {
		GCFPropSetAnswerEvent* pPropAnswer=static_cast<GCFPropSetAnswerEvent*>(&answer);
		if(pPropAnswer->result != GCF_NO_ERROR) {
			LOG_ERROR(formatString("%s : PropertySet %s NOT ENABLED",
										getName().c_str(), pPropAnswer->pScope));
		}
		// always let timer expire so main task will continue.
		LOG_DEBUG_STR("Property set " << pPropAnswer->pScope << 
													" enabled, continuing main task");
		itsTimerPort->setTimer(0.5);
		break;
	}
	
	case F_VGETRESP: {	// initial get of required clock
		GCFPropValueEvent* pPropAnswer=static_cast<GCFPropValueEvent*>(&answer);
		if (strstr(pPropAnswer->pPropName, PN_SC_CLOCK) != 0) {
			itsClock =(static_cast<const GCFPVInteger*>(pPropAnswer->pValue))->getValue();

			// signal main task we have the value.
			LOG_DEBUG_STR("Clock in PVSS has value: " << itsClock);
			itsTimerPort->setTimer(0.5);
			break;
		}
		LOG_WARN_STR("Got VGETRESP signal from unknown property " << 
															pPropAnswer->pPropName);
	}

	case F_VCHANGEMSG: {
		// check which property changed
		GCFPropValueEvent* pPropAnswer=static_cast<GCFPropValueEvent*>(&answer);

		if (strstr(pPropAnswer->pPropName, PN_SC_CLOCK) != 0) {
			itsClock =(static_cast<const GCFPVInteger*>(pPropAnswer->pValue))->getValue();
			LOG_DEBUG_STR("Received clock change from PVSS, clock is now " << itsClock);
			break;
		}

		// don't watch state and error fields.
		if ((strstr(pPropAnswer->pPropName, PVSSNAME_FSM_STATE) != 0) || 
			(strstr(pPropAnswer->pPropName, PVSSNAME_FSM_ERROR) != 0) ||
			(strstr(pPropAnswer->pPropName, PVSSNAME_FSM_LOGMSG) != 0)) {
			return;
		}
 
		LOG_WARN_STR("Got VCHANGEMSG signal from unknown property " << 
															pPropAnswer->pPropName);
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
// Setup connection with PVSS and load Property sets.
//
GCFEvent::TResult StationControl::initial_state(GCFEvent& event, 
													GCFPortInterface& port)
{
	LOG_DEBUG_STR ("initial:" << eventstr(event) << "@" << port.getName());

	GCFEvent::TResult status = GCFEvent::HANDLED;
  
	switch (event.signal) {
    case F_INIT:
   		break;

	case F_ENTRY: {
		// Get access to my own propertyset.
		string	myPropSetName(createPropertySetName(PSN_STATION_CTRL, getName()));
		LOG_DEBUG_STR ("Activating PropertySet " << myPropSetName);
		itsOwnPropSet = new GCFMyPropertySet(PSN_STATION_CTRL,
													 PST_STATION_CTRL,
													 PS_CAT_PERM_AUTOLOAD,
													 &itsPropertySetAnswer);
		itsOwnPropSet->enable();		// will result in F_MYPS_ENABLED

		// When myPropSet is enabled we will connect to the external PropertySet
		// that dictates the systemClock setting.

		// Wait for timer that is set in PropertySetAnswer on ENABLED event
		}
		break;

	case F_TIMER:
		// first timer event is from own propertyset
		if (!itsOwnPSinitialized) {
			itsOwnPSinitialized = true;

			// first redirect signalHandler to our finishing state to leave PVSS
			// in the right state when we are going down
			thisStationControl = this;
			signal (SIGINT,  StationControl::sigintHandler);	// ctrl-c
			signal (SIGTERM, StationControl::sigintHandler);	// kill

			// update PVSS.
			LOG_TRACE_FLOW ("Updateing state to PVSS");
			itsOwnPropSet->setValue(PVSSNAME_FSM_STATE,GCFPVString("initial"));
			itsOwnPropSet->setValue(PVSSNAME_FSM_ERROR,GCFPVString(""));

			// enable clock propertyset.
			string	clkPropSetName(createPropertySetName(PSN_STATION_CLOCK, getName()));
			LOG_DEBUG_STR ("Activating PropertySet " << clkPropSetName);
			itsClockPropSet = new GCFMyPropertySet(PSN_STATION_CLOCK,
														 PST_STATION_CLOCK,
														 PS_CAT_PERM_AUTOLOAD,
														 &itsPropertySetAnswer);
			itsClockPropSet->enable();		// will result in F_MYPS_ENABLED
		}
		else {
			itsClockPSinitialized = true;

			LOG_DEBUG ("Attached to external propertySets");

			LOG_DEBUG ("Enabling ChildControl task");
			itsChildControl->openService(MAC_SVCMASK_STATIONCTRL, itsInstanceNr);
			itsChildControl->registerCompletionPort(itsChildPort);

			LOG_DEBUG ("Going to connect state to attach to DigitalBoardController");
			TRAN(StationControl::connect_state);			// go to next state.
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
// connect_state(event, port)
//
// Setup connection with DigitalBoardControl
//
GCFEvent::TResult StationControl::connect_state(GCFEvent& event, 
													GCFPortInterface& port)
{
	LOG_DEBUG_STR ("connect:" << eventstr(event) << "@" << port.getName());

	GCFEvent::TResult status = GCFEvent::HANDLED;
  
	switch (event.signal) {
    case F_INIT:
   		break;

	case F_ENTRY: {
		itsOwnPropSet->setValue(PVSSNAME_FSM_STATE,GCFPVString("connected"));

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
	LOG_DEBUG_STR ("operational:" << eventstr(event) << "@" << port.getName());

	GCFEvent::TResult status = GCFEvent::HANDLED;

	switch (event.signal) {
	case F_INIT:
		break;

	case F_ENTRY: {
		// update PVSS
		itsOwnPropSet->setValue(PVSSNAME_FSM_STATE,GCFPVString("active"));
		itsOwnPropSet->setValue(PVSSNAME_FSM_ERROR,GCFPVString(""));
	}
	break;

	case F_ACCEPT_REQ:
	case F_CONNECTED:
	case F_DISCONNECTED:
		break;

	case F_TIMER:  {
		ObsIter			 theObs     = itsObsMap.find(port.getName());
		if (theObs == itsObsMap.end()) {
			LOG_WARN_STR("Event for unknown observation: " << port.getName());
			break;
		}

		// pass event to observation FSM
		LOG_TRACE_FLOW("Dispatch to observation FSM's");
		theObs->second->dispatch(event, port);
		LOG_TRACE_FLOW("Back from dispatch");
	}
	break;

	// -------------------- EVENTS RECEIVED FROM PARENT CONTROL --------------------
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

	case CONTROL_CONNECTED:		// from ChildITCport
	case CONTROL_SCHEDULED:
	case CONTROL_CLAIMED:
	case CONTROL_PREPARED:
	case CONTROL_RESUMED:
	case CONTROL_SUSPENDED:
	case CONTROL_RELEASED:
	case CONTROL_FINISH:
	case CONTROL_CLAIM:			// from ParentITCport
	case CONTROL_SCHEDULE:
	case CONTROL_PREPARE:
	case CONTROL_RESUME:
	case CONTROL_SUSPEND:
	case CONTROL_RELEASE:
	case CONTROL_QUIT: {
		// All the events have the problem that the controller can be StationControl,
		// CalibrationControl or BeamControl. But what they all have in common is
		// the instancenumber and treeID.
		// Substract instanceNr and treeID and contruct the StationControl-variant.
		CONTROLCommonEvent	ObsEvent(event);
		uint16			 instanceNr = getInstanceNr(ObsEvent.cntlrName);
		OTDBtreeIDType	 treeID	    = getObservationNr(ObsEvent.cntlrName);
		string			 cntlrName  = controllerName(CNTLRTYPE_STATIONCTRL, 
															instanceNr, treeID);
		ObsIter			 theObs     = itsObsMap.find(cntlrName);
		if (theObs == itsObsMap.end()) {
			LOG_WARN_STR("Event for unknown observation: " << ObsEvent.cntlrName);
			break;
		}

		if (event.signal == CONTROL_CLAIM && 
								itsClock != theObs->second->obsPar()->sampleClock) {
			itsClock = theObs->second->obsPar()->sampleClock;
			LOG_DEBUG_STR ("Changing clock to " << itsClock);
			itsClockPropSet->setValue(PN_SC_CLOCK,GCFPVInteger(itsClock));
			// TODO: give clock 5 seconds to stabelize
		}

		// pass event to observation FSM
		LOG_TRACE_FLOW("Dispatch to observation FSM's");
		theObs->second->dispatch(event, port);

		// end of FSM?
		if (event.signal == CONTROL_FINISH && theObs->second->isReady()) {
			LOG_DEBUG_STR("Removing " <<ObsEvent.cntlrName<< " from the administration");
			delete theObs->second;
			itsObsMap.erase(theObs);
		}

		// check if all actions for this event are finished.
LOG_TRACE_FLOW("Counting busy controllers...");
		vector<ChildControl::StateInfo>	cntlrStates = 
									itsChildControl->getPendingRequest("", treeID);
LOG_TRACE_FLOW_STR("There are " << cntlrStates.size() << " busy controllers");
		if (cntlrStates.empty()) {	// no pending requests? Ready.
			if (event.signal != CONTROL_FINISH) {
				sendControlResult(*itsParentPort, event.signal, cntlrName, 
																CT_RESULT_NO_ERROR);
			}
			else {
				// we are done, pass finish request to parent
				CONTROLFinishEvent		request;
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
	LOG_DEBUG_STR ("finishing_state:" << evtstr(event) << "@" << port.getName());

	GCFEvent::TResult status = GCFEvent::HANDLED;

	switch (event.signal) {
	case F_INIT:
		break;

	case F_ENTRY: {
		// update PVSS
		itsOwnPropSet->setValue(string(PVSSNAME_FSM_STATE),GCFPVString("finished"));
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

	return (CT_RESULT_NO_ERROR);
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
