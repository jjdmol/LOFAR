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
	
//
// StationControl()
//
StationControl::StationControl(const string&	cntlrName) :
	GCFTask 			((State)&StationControl::initial_state,cntlrName),
	PropertySetAnswerHandlerInterface(),
	itsPropertySetAnswer(*this),
	itsOwnPropertySet	(),
	itsExtPropertySet	(),
	itsOwnPSinitialized (false),
	itsExtPSinitialized (false),
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

	LOG_INFO_STR("MACProcessScope: " << itsTreePrefix + cntlrName);

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

	if (itsOwnPropertySet) {
		itsOwnPropertySet->setValue(string(PVSSNAME_FSM_STATE),GCFPVString("down"));
	}

	// ...
}


//
// handlePropertySetAnswer(answer)
//
void StationControl::handlePropertySetAnswer(GCFEvent& answer)
{
	LOG_DEBUG_STR ("handlePropertySetAnswer:" << eventstr(answer));

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
		string	myPropSetName(createPropertySetName(PSN_STATION_CLOCK, getName()));
		LOG_DEBUG_STR ("Activating PropertySet " << myPropSetName);
		itsOwnPropertySet = new GCFMyPropertySet(PSN_STATION_CLOCK,
													 PST_STATION_CLOCK,
													 PS_CAT_PERM_AUTOLOAD,
													 &itsPropertySetAnswer);
		itsOwnPropertySet->enable();		// will result in F_MYPS_ENABLED

		// When myPropSet is enabled we will connect to the external PropertySet
		// that dictates the systemClock setting.

		// Wait for timer that is set in PropertySetAnswer on ENABLED event
		}
		break;

	case F_TIMER:
		// first timer event is from own propertyset
		if (!itsOwnPSinitialized) {
			itsOwnPSinitialized = true;

			// update PVSS.
			LOG_TRACE_FLOW ("Updateing state to PVSS");
			itsOwnPropertySet->setValue(PVSSNAME_FSM_STATE,GCFPVString("initial"));
			itsOwnPropertySet->setValue(PVSSNAME_FSM_ERROR,GCFPVString(""));
		}

		LOG_DEBUG ("Attached to external propertySet, going to connection state");
		TRAN(StationControl::connect_state);			// go to next state.
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
		// start DigitalBoardController
		LOG_DEBUG_STR("Starting DigitalBaordController");
		itsChildControl->startChild(CNTLRTYPE_DIGITALBOARDCTRL,
							   		0,			// treeID, 
							   		0,			// instanceNr,
							   		myHostname(true));
		// will result in CONTROL_CONNECT
		}
		break;

	case CONTROL_CONNECT: {
		CONTROLConnectEvent		msg(event);
		ASSERTSTR(msg.cntlrName == controllerName(CNTLRTYPE_DIGITALBOARDCTRL, 0 ,0),
							"Connect event of unknown controller: " << msg.cntlrName);
		LOG_DEBUG ("Attached to DigitalBoardControl, going to operational state");
		TRAN(StationControl::operational_state);			// go to next state.
		}
		break;

	case F_CONNECTED:
		break;

	case F_DISCONNECTED:
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
		itsOwnPropertySet->setValue(PVSSNAME_FSM_STATE,GCFPVString("active"));
		itsOwnPropertySet->setValue(PVSSNAME_FSM_ERROR,GCFPVString(""));
	}
	break;

	case F_ACCEPT_REQ:
	case F_CONNECTED:
	case F_DISCONNECTED:
	case F_TIMER: 
		break;

	// -------------------- EVENTS RECEIVED FROM PARENT CONTROL --------------------
	case CONTROL_CONNECT: {
		CONTROLConnectEvent		msg(event);
		CONTROLConnectedEvent	answer;
		answer.cntlrName = msg.cntlrName;
		// add observation to the list of not already in the list
		answer.result = _addObservation(msg.cntlrName) ? 
									CT_RESULT_NO_ERROR : CT_RESULT_UNSPECIFIED;
		port.send(answer);
	}
	break;

	case CONTROL_SCHEDULE:
	case CONTROL_CLAIM:
	case CONTROL_PREPARE:
	case CONTROL_RESUME:
	case CONTROL_SUSPEND:
	case CONTROL_RELEASE:
	case CONTROL_QUIT: {
		CONTROLCommonEvent	ObsEvent(event);
		ObsIter		theObs = itsObsMap.find(ObsEvent.cntlrName);
		if (theObs == itsObsMap.end()) {
			LOG_WARN_STR("Event for unknown observation: " << ObsEvent.cntlrName);
			break;
		}
		theObs->second->dispatch(event, port);
		if (event.signal == CONTROL_QUIT && theObs->second->isReady()) {
			LOG_DEBUG_STR("Removing " <<ObsEvent.cntlrName<< " from the administration");
			delete theObs->second;
			itsObsMap.erase(theObs);
		}
	}
	break;

	default:
		LOG_DEBUG("active_state, default");
		status = GCFEvent::NOT_HANDLED;
		break;
	}

	return (status);
}

//
// addObservation(name)
//
bool StationControl::_addObservation(const string&	name)
{
	// Already in admin? Return error.
	if (itsObsMap.find(name) != itsObsMap.end()) {
		LOG_DEBUG_STR(name << " already in admin, returning error");
		return (false);
	}

	// find and read parameterset of this observation
	ParameterSet	theObsPS;
	LOG_TRACE_OBJ_STR("Trying to readfile " << LOFAR_SHARE_LOCATION << "/" << name);
	theObsPS.adoptFile(string(LOFAR_SHARE_LOCATION) + "/" + name);

	ActiveObs*	theNewObs = new ActiveObs(name, (State)&ActiveObs::initial, &theObsPS);
	LOG_FATAL_STR("Unable to create the Observation '" << name << "'");
	return (false);

	LOG_DEBUG_STR("Adding " << name << " to administration");
	itsObsMap[name] = theNewObs;
	theNewObs->start();				// call initial state.

	return (true);
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
