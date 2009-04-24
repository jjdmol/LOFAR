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
//	the ClockController, the CalibrationController and the BeamController.
//
//	$Id$
//
#include <lofar_config.h>
#include <Common/LofarLogger.h>
#include <Common/LofarLocators.h>
#include <Common/LofarConstants.h>
#include <Common/SystemUtil.h>
#include <Common/Version.h>
#include <ApplCommon/StationConfig.h>
#include <ApplCommon/StationInfo.h>

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
#include <APL/RTDBCommon/RTDButilities.h>
#include <signal.h>

#include "ActiveObs.h"
#include "StationControl.h"
#include "PVSSDatapointDefs.h"
#include "Clock_Protocol.ph"
#include <APL/ClockProtocol/Package__Version.h>

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
	itsParentInitialized(false),
	itsDPservice		(0),
	itsStateQryID		(0),
	itsSplitterQryID	(0),
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
	LOG_INFO("MACProcessScope: LOFAR.PermSW.StationControl");
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
	registerProtocol (CLOCK_PROTOCOL, 	CLOCK_PROTOCOL_STRINGS);

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
		string	myPropSetName(createPropertySetName(PSN_STATION_CONTROL, getName()));
		LOG_DEBUG_STR ("Activating PropertySet " << myPropSetName);
		itsOwnPropSet = new RTDBPropertySet(myPropSetName,
											PST_STATION_CONTROL,
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
			string	clkPropSetName(createPropertySetName(PSN_CLOCK_CONTROL, getName()));
			LOG_DEBUG_STR ("Activating PropertySet " << clkPropSetName);
			itsClockPropSet = new RTDBPropertySet(clkPropSetName,
												  PST_CLOCK_CONTROL,
												  PSAT_WO,
												  this);
		}
		else {
			itsClockPSinitialized = true;
			LOG_DEBUG ("Attached to external propertySets");

			GCFPVInteger	clockVal;
			itsClockPropSet->getValue(PN_CLC_REQUESTED_CLOCK, clockVal);
			if (clockVal.getValue() != 0) {
				itsClock = clockVal.getValue();
				LOG_DEBUG_STR("Clock in PVSS has value: " << itsClock);
			}
			else {
				// try actual clock
				itsClockPropSet->getValue(PN_CLC_ACTUAL_CLOCK, clockVal);
				if (clockVal.getValue() == 0) {
					// both DB values are 0, fall back to 160
					LOG_DEBUG("Clock settings in the database are all 0, setting 160 as default");
					itsClock = 160;
					itsClockPropSet->setValue(PN_CLC_REQUESTED_CLOCK, GCFPVInteger(itsClock));
				}
				else {
					itsClock = clockVal.getValue();
					LOG_DEBUG_STR("Actual clock in PVSS has value: " << itsClock << " applying that value");
					itsClockPropSet->setValue(PN_CLC_REQUESTED_CLOCK, clockVal);
				}
			}
			
			LOG_DEBUG ("Going to connect state to attach to ClockController");
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
// Start the ClockController, and wait for STARTED and CONNECTED event
// Finally open the command port.
//
GCFEvent::TResult StationControl::connect_state(GCFEvent& event, 
													GCFPortInterface& port)
{
	LOG_DEBUG_STR ("connect:" << eventName(event) << "@" << port.getName());

	GCFEvent::TResult status = GCFEvent::HANDLED;
  
	switch (event.signal) {

	case F_TIMER:
	case F_ENTRY: {
		itsOwnPropSet->setValue(PN_FSM_CURRENT_ACTION,GCFPVString("Starting ClockController"));

		// start ClockController
		LOG_DEBUG_STR("Starting ClockController");
		itsChildControl->startChild(CNTLRTYPE_CLOCKCTRL,
							   		0,			// treeID, force 'shared' by using 0
							   		0,			// instanceNr,
							   		myHostname(false));
		// will result in CONTROL_STARTED (and CONTROL_CONNECTED if no error).
		}
		break;

	case CONTROL_STARTED: {
		CONTROLStartedEvent		msg(event);
//		ASSERTSTR(msg.cntlrName == controllerName(CNTLRTYPE_CLOCKCTRL, 0 ,0),
//							"Started event of unknown controller: " << msg.cntlrName);
// note: need more complex test because the hostname is now in controllername.
			if (msg.successful) {
				LOG_INFO_STR("Startup of " << msg.cntlrName << 
											" succesful, waiting for connection");
			}
			else {
				LOG_WARN_STR("Startup of " << msg.cntlrName << "FAILED, retry in 5 seconds");
				// inform parent about the failure
//				CONTROLConnectedEvent	answer;
//				answer.cntlrName = getName();
//				answer.result    = CT_RESULT_LOST_CONNECTION;
//				itsParentPort->send(answer);
			}
			itsTimerPort->setTimer(5.0);	// in case we fail
		}
		break;

	case CONTROL_CONNECTED: {
		itsTimerPort->cancelAllTimers();
		CONTROLConnectedEvent		msg(event);
//		ASSERTSTR(msg.cntlrName == controllerName(CNTLRTYPE_CLOCKCTRL, 0 ,0),
//							"Connect event of unknown controller: " << msg.cntlrName);
// note: need more complex test because the hostname is now in controllername.

		// inform parent the chain is up
//		CONTROLConnectedEvent	answer;
//		answer.cntlrName = getName();
//		answer.result    = CT_RESULT_NO_ERROR;
//		itsParentPort->send(answer);

		itsClkCtrlPort = new GCFTCPPort(*this, MAC_SVCMASK_CLOCKCTRL, GCFPortInterface::SAP, CLOCK_PROTOCOL);
		ASSERTSTR(itsClkCtrlPort, "Cannot allocate port for communicating with the ClockController");
		itsClkCtrlPort->autoOpen(10,0,1);
	}
	break;

	case F_DISCONNECTED:
		port.close();
		LOG_FATAL("Cannot open the command-port to the ClockController, bailing out!");
		TRAN(StationControl::finishing_state);
	break;
	
	case F_CONNECTED: {
		LOG_DEBUG ("Attached to ClockController, taking a subscription on the hardware states");
		TRAN(StationControl::subscribe2HWstates);
	}
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

	case DP_QUERY_SUBSCRIBED: {			// NOTE: DIT EVENT KOMT EIGENLIJK NOOIT. WAAROM?
		itsTimerPort->cancelAllTimers();
		DPQuerySubscribedEvent  answer(event);
		if (answer.result != SA_NO_ERROR) {
			LOG_ERROR_STR ("Taking subscription on PVSS-states failed (" << answer.result  <<
							"), retry in 10 seconds");
			itsTimerPort->setTimer(10.0);
			break;
		}
		itsStateQryID = answer.QryID;
		LOG_INFO_STR("Subscription on state fields from PVSS successful(" << itsStateQryID  << ")");
		if (itsHasSplitters) {
			LOG_INFO("Going to setup a query for the splitter state");
			TRAN(StationControl::subscribe2Splitters);		// go to next state.
		}
		else {
			LOG_INFO("Going to operational mode");
			TRAN(StationControl::operational_state);		// go to next state.
		}
	}
	break;

	case DP_QUERY_CHANGED:
		// don't expect this event here right now, but you never know.
 		// in case DP_QUERY_SUBSCRIBED is skipped.
		LOG_WARN("STRANGE ORDER OF EVENTS IN subscribe2HWState");
		itsTimerPort->cancelAllTimers();
		_handleQueryEvent(event);
		if (itsHasSplitters) {
			TRAN(StationControl::subscribe2Splitters);		// go to next state.
		}
		else {
			TRAN(StationControl::operational_state);		// go to next state.
		}
	break;

	default:
		status = GCFEvent::NOT_HANDLED;
	break;
	}

	return (status);
}
//
// subscribe2Splitters(event, port)
//
// Take a subscribtion to the setting of the splitters
//
GCFEvent::TResult StationControl::subscribe2Splitters(GCFEvent& event, GCFPortInterface& port)
{
	LOG_DEBUG_STR("subscribe2splitters:" << eventName(event) << "@" << port.getName());

	GCFEvent::TResult status = GCFEvent::HANDLED;
	switch (event.signal) {
	case F_ENTRY:
	case F_TIMER: {
		// take subscribtion on *.state
		LOG_DEBUG("Taking subscription on all splitter settings");
		PVSSresult  result = itsDPservice->query("'LOFAR_PIC_*.splitterOn'", "");
		if (result != SA_NO_ERROR) {
			LOG_ERROR ("Taking subscription on splitter-settings failed, retry in 10 seconds");
			itsTimerPort->setTimer(10.0);
			break;
		}
		// wait for DP event
		LOG_DEBUG ("Waiting for subscription answer");
	}
	break;

	case DP_QUERY_SUBSCRIBED: {
		itsTimerPort->cancelAllTimers();
		DPQuerySubscribedEvent  answer(event);
		if (answer.result != SA_NO_ERROR) {
			LOG_ERROR_STR ("Taking subscription on splitter settings failed (" << answer.result  <<
							"), retry in 10 seconds");
			itsTimerPort->setTimer(10.0);
			break;
		}
		itsSplitterQryID = answer.QryID;
		LOG_INFO_STR("Subscription on state fields from PVSS successful(" << itsSplitterQryID  << ")");
		LOG_INFO("Going to operational mode");
		TRAN(StationControl::operational_state);		// go to next state.
	}
	break;

	case DP_QUERY_CHANGED:
		itsTimerPort->cancelAllTimers();
		_handleQueryEvent(event);
		LOG_INFO("Going to operational mode");
		TRAN(StationControl::operational_state);		// go to next state.
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
	case F_ENTRY: {
		// update PVSS
		if (!itsParentInitialized) {
			LOG_DEBUG ("All initialisation done, enabling ParentControl task");
			itsParentPort = itsParentControl->registerTask(this);

			itsOwnPropSet->setValue(PN_FSM_CURRENT_ACTION,GCFPVString("Active"));
			itsOwnPropSet->setValue(PN_FSM_ERROR,GCFPVString(""));
			itsParentInitialized = true;
		}
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
				LOG_WARN_STR("Timer event for unknown observation: " << port.getName());
				break;
			}
			// found on timerID. This means that the Observation should have died
			// five seconds ago. Send a QUIT command.
			LOG_WARN_STR("Observation " << theObs->second->getName() << 
						" should have died by now, sending an extra QUIT command.");
			CONTROLQuitEvent	quitevent;
			quitevent.cntlrName = theObs->second->getName();
			theObs->second->doEvent(quitevent, port);
		}

		// pass event to observation FSM
		LOG_TRACE_FLOW("Dispatch to observation FSM's");
		theObs->second->doEvent(event, port);
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
		if (answer.result != CT_RESULT_NO_ERROR) {	// problem?
			port.send(answer);						// tell parent we didn't start the obs.
		}
	}
	break;

	// ------------ EVENTS RECEIVED FROM CHILD AND PARENT CONTROL --------------

	// The events from the child-task may be of the ClockController or one of the
	// BeamCtlrs or CalCtlrs of the active observations.
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

		// In the claim state station-wide changes are activated.
		if (event.signal == CONTROL_CLAIM) {
			itsStartingObs = theObs;
			TRAN(StationControl::startObservation_state);
			queueTaskEvent(event, port);
			return (GCFEvent::HANDLED);
		}

		// TODO: CLEAN UP THE CODE BELOW

#if 0
		// before passing a new state request from the ObsController to the 
		// activeObs, make sure the last state is reached.
LOG_DEBUG_STR(formatString("event.signal = %04X", event.signal));
LOG_DEBUG_STR("F_INDIR = " << F_INDIR(event.signal));
LOG_DEBUG_STR("F_OUTDIR = " << F_OUTDIR(event.signal));
LOG_DEBUG_STR("inSync = " << (theObs->second->inSync() ? "true" : "false"));
		if (F_OUTDIR(event.signal) && !theObs->second->inSync()) {
			// TODO
			CTState		cts;
			LOG_FATAL_STR("Ignoring change to state " << cts.name(cts.signal2stateNr(event.signal)) << 
						" for observation " << treeID << " because obs is still in state " << 
						cts.name(theObs->second->curState()));
			sendControlResult(*itsParentPort, event.signal, cntlrName, 
																CT_RESULT_OUT_OF_SYNC);
			break;
			
		}
#endif
		// pass event to observation FSM
		LOG_TRACE_FLOW("Dispatch to observation FSM's");
		theObs->second->doEvent(event, port);

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

	case F_EXIT:
		break;

	default:
		LOG_DEBUG("active_state, default");
		status = GCFEvent::NOT_HANDLED;
	break;
	} // switch

	return (status);
}


//
// startObservation_state(event,port)
//
// Substate where we optionally set the clock and the splitters before passing the CLAIM event to the ActiveObs.
//
GCFEvent::TResult	StationControl::startObservation_state(GCFEvent&	event, GCFPortInterface&	port)
{
	LOG_DEBUG_STR("startObservation: " << eventName(event) << "@" << port.getName());

	switch (event.signal) {
	case CONTROL_CLAIM: {
		// Clock changes are done in the claim state and require an extra action
		if (itsClock != itsStartingObs->second->obsPar()->sampleClock) {
			// Check if all others obs are down otherwise we may not switch the clock
			CONTROLCommonEvent	ObsEvent(event);		// we just need the name
			uint16			 instanceNr = getInstanceNr(ObsEvent.cntlrName);
			OTDBtreeIDType	 treeID	    = getObservationNr(ObsEvent.cntlrName);
			string			 cntlrName  = controllerName(CNTLRTYPE_STATIONCTRL, instanceNr, treeID);
			if (itsObsMap.size() != 1) {
				LOG_FATAL_STR("Need to switch the clock to " <<  itsStartingObs->second->obsPar()->sampleClock << 
						" for observation " << treeID << " but there are still " << itsObsMap.size()-1 << 
						" other observations running at clockspeed" << itsClock << ".");
				_abortObservation(itsStartingObs);
				itsStartingObs = 0;
				TRAN(StationControl::operational_state);
				break;
			}
			// its OK to switch te clock
			itsClock = itsStartingObs->second->obsPar()->sampleClock;
			LOG_DEBUG_STR ("Changing clock to " << itsClock);
			CLKCTRLSetClockEvent	setClock;
			setClock.clock = itsClock;
			itsClkCtrlPort->send(setClock);		// results in CLKCTRL_SET_CLOCK_ACK
			itsClockPropSet->setValue(PN_CLC_REQUESTED_CLOCK,GCFPVInteger(itsClock));
		}
		else {
			LOG_INFO_STR("new observation also uses clock " << itsClock);
			itsTimerPort->setTimer(0.0);	// goto set splitter section
		}
	}
	break;

	case CLKCTRL_SET_CLOCK_ACK: {
		CLKCTRLSetClockAckEvent		ack(event);
		if (ack.status != CLKCTRL_NO_ERR) {
			LOG_FATAL_STR("Unable to set the clock to " << itsClock << ".");
			_abortObservation(itsStartingObs);
			itsStartingObs = 0;
			TRAN(StationControl::operational_state);
			break;
		}
		// clock was set succesfully, give clock 5 seconds to stabelize
		LOG_INFO("Stationclock is changed, waiting 5 seconds to let the clock stabelize");
		itsTimerPort->setTimer(5.0);
	}
	break;

	case F_TIMER: {
		bool	splitterState = itsStartingObs->second->obsPar()->splitter;
		LOG_DEBUG_STR ("Setting the splitters to " << (splitterState ? "ON" : "OFF"));
		CLKCTRLSetSplittersEvent	setEvent;
		setEvent.splittersOn = splitterState;
		itsClkCtrlPort->send(setEvent);		// will result in CLKCTRL_SET_SPLITTERS_ACK
	}
	break;

	case CLKCTRL_SET_SPLITTERS_ACK: {
		CLKCTRLSetSplittersAckEvent		ack(event);
		bool	splitterState = itsStartingObs->second->obsPar()->splitter;
		if (ack.status != CLKCTRL_NO_ERR) {
			LOG_FATAL_STR("Unable to set the splittters to " << (splitterState ? "ON" : "OFF"));
			_abortObservation(itsStartingObs);
			itsStartingObs = 0;
			TRAN(StationControl::operational_state);
			break;
		}
		
		itsSplitters = splitterState;

		// finally send a CLAIM event to the observation
		LOG_TRACE_FLOW("Dispatch CLAIM event to observation FSM's");
		CONTROLClaimEvent		claimEvent;
		itsStartingObs->second->doEvent(claimEvent, port);

		LOG_INFO("Going back to operational state");
		itsStartingObs = 0;
		TRAN(StationControl::operational_state);
	}
	break;

	case F_EXIT:
		break;

	default:
		LOG_DEBUG_STR("Postponing event " << eventName(event) << " till operational state");
		return (GCFEvent::NEXT_STATE);
	}

	LOG_DEBUG("startObservation_state: just before exit");

	return (GCFEvent::HANDLED);
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

		// cancel active queries
		if  (itsStateQryID) {
			itsDPservice->cancelQuery(itsStateQryID);
			itsStateQryID = 0;
		}
		if  (itsSplitterQryID) {
			itsDPservice->cancelQuery(itsSplitterQryID);
			itsSplitterQryID = 0;
		}

		itsTimerPort->setTimer(1L);
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
// _databaseEventHandler(event)
//
void StationControl::_databaseEventHandler(GCFEvent& event)
{
	LOG_TRACE_FLOW_STR ("_databaseEventHandler:" << eventName(event));

	switch(event.signal) {
	case DP_CHANGED:  {
		DPChangedEvent		dpEvent(event);
		if (strstr(dpEvent.DPname.c_str(), PN_CLC_REQUESTED_CLOCK) != 0) {
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
		LOG_ERROR_STR("PVSS reported error " << DPevent.result << " for a query " << 
				", cannot determine actual state of the hardware! Assuming everything is available.");
		return;
	}

	if (!itsStateQryID) {
		itsStateQryID = DPevent.QryID;
	}

	// The selected datapoints are delivered with full PVSS names, like:
	// CS001:LOFAR_PIC_Cabinet0_Subrack0_RSPBoard0_RCU5.status.state
	// CS001:LOFAR_PIC_Cabinet0_Subrack0_RSPBoard0.splitterOn
	// Each event may contain more than one DP.
	int     nrDPs = ((GCFPVDynArr*)(DPevent.DPnames._pValue))->getValue().size();
	GCFPVDynArr*    DPnames  = (GCFPVDynArr*)(DPevent.DPnames._pValue);
	GCFPVDynArr*    DPvalues = (GCFPVDynArr*)(DPevent.DPvalues._pValue);
	// GCFPVDynArr*    DPtimes  = (GCFPVDynArr*)(DPevent.DPtimes._pValue);

	uint32	modeOff(objectStateIndex2Value(RTDB_OBJ_STATE_OFF));
	uint32	modeOperational(objectStateIndex2Value(RTDB_OBJ_STATE_OPERATIONAL));
	for (int    idx = 0; idx < nrDPs; ++idx) {
		// Get the name and figure out what circuitboard we are talking about
		string  nameStr(DPnames->getValue()[idx]->getValueAsString());				// DP name
		uint32	newState(((GCFPVInteger*)(DPvalues->getValue()[idx]))->getValue());	// value
		size_t	pos;

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
			if (nameStr.find(".status.state") != string::npos) {
				if (sscanf(nameStr.substr(pos).c_str(), "_RSPBoard%d.status.state", &rsp) == 1) {
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
			}
			else if (nameStr.find(".splitterOn") != string::npos) {
				// remember QueryID if not yet set
				if (!itsSplitterQryID) {
					itsSplitterQryID = DPevent.QryID;
				}

				if (sscanf(nameStr.substr(pos).c_str(), "_RSPBoard%d.splitterOn", &rsp) == 1) {
					LOG_INFO_STR("New setting of splitter " << rsp << " is " << (newState ? "on" : "off"));
					itsSplitters.set(rsp);
				}
			}
			else {
				LOG_ERROR_STR("Unrecognized datapoint " << nameStr << 
								". STATE OF ANTENNA'S OR SPLITTERS MIGHT NOT BE UP TO DATE ANYMORE");
				continue;
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

	// check if there will be a conflict.
	ObsIter		iter = itsObsMap.begin();
	ObsIter		end  = itsObsMap.end();
	while (iter != end) {
		if (iter->second->obsPar()->conflicts(theObs)) {
			return (CT_RESULT_OBS_CONFLICT);
		}
		++iter;
	}
	LOG_INFO_STR("Observation " << theObs.obsID << 
				 " has no conflicts with other running observations");

	// Create a bitset containing the available receivers for this oberservation.
	StationConfig			config;
	Observation::RCUset_t	realReceivers = 
			Observation(&theObsPS).getRCUbitset(config.nrLBAs, config.nrHBAs, config.nrRSPs, config.hasSplitters);

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
// _abortObservation(theObsIter)
//
void StationControl::_abortObservation(ObsIter	theObs)
{
	LOG_ERROR_STR("Aborting observation " << getObservationNr(theObs->second->getName()));

	CONTROLQuitEvent		quitEvent;
	quitEvent.cntlrName = theObs->second->getName();
	GCFDummyPort	DP(this, "abortObsPort", 0);
	theObs->second->doEvent(quitEvent, DP);

	// tell Parent task we initiated a quit
	itsParentControl->nowInState(theObs->second->getName(), CTState::QUIT);
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

	// Adopt values from RemoteStation.conf
	StationConfig	SC;
	itsNrRSPboards = SC.nrRSPs;
	itsNrLBAs 	   = SC.nrLBAs;
	itsNrHBAs 	   = SC.nrHBAs;
	itsHasSplitters= SC.hasSplitters;

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

	if (itsHasSplitters && itsSplitters.count() != 0 && itsSplitters.count() != itsNrRSPboards) {
		LOG_WARN_STR("Not all splitters have the same state! " << itsSplitters);
		// TODO: ring some bells in the Navigator?
	}
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
// _updateObsListInPVSS()
//
// Refresh the contents of the activeObservations datafield in PVSS.
//
void StationControl::_updateObsListInPVSS()
{
	ObsIter		iter(itsObsMap.begin());
	ObsIter		end (itsObsMap.end());

	GCFPValueArray		obsArr;
	while (iter != end) {
		obsArr.push_back(new GCFPVString(iter->first));
		++iter;
	}
	itsOwnPropSet->setValue(PN_SC_ACTIVE_OBSERVATIONS, GCFPVDynArr(LPT_DYNSTRING, obsArr));
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
