//#  CalibrationControl.cc: Implementation of the MAC Scheduler task
//#
//#  Copyright (C) 2006
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
#include <Common/lofar_datetime.h>

#include <GCF/GCF_PVTypes.h>
#include <GCF/GCF_ServiceInfo.h>
#include <GCF/Protocols/PA_Protocol.ph>
#include <APL/APLCommon/APLUtilities.h>
#include <APL/APLCommon/Controller_Protocol.ph>
#include <APL/APLCommon/StationInfo.h>
#include <APL/CAL_Protocol/CAL_Protocol.ph>
#include <signal.h>

#include "CalibrationControl.h"
#include "CalibrationControlDefines.h"

using namespace LOFAR::GCF::Common;
using namespace LOFAR::GCF::TM;
using namespace LOFAR::GCF::PAL;

namespace LOFAR {
	using namespace APLCommon;
	using namespace ACC::APS;
	namespace StationCU {
	
// static pointer to this object for signal handler
static CalibrationControl*	thisCalibrationControl = 0;

//
// CalibrationControl()
//
CalibrationControl::CalibrationControl(const string&	cntlrName) :
	GCFTask 			((State)&CalibrationControl::initial_state,cntlrName),
	PropertySetAnswerHandlerInterface(),
	itsPropertySetAnswer(*this),
	itsPropertySet		(),
	itsPropertySetInitialized (false),
	itsParentControl	(0),
	itsParentPort		(0),
	itsTimerPort		(0),
	itsCalServer		(0),
	itsState			(CTState::NOSTATE)
{
	LOG_TRACE_OBJ_STR (cntlrName << " construction");

	// First readin our observation related config file.
	LOG_DEBUG_STR("Reading parset file:" << LOFAR_SHARE_LOCATION << "/" << cntlrName);
	globalParameterSet()->adoptFile(string(LOFAR_SHARE_LOCATION)+"/"+cntlrName);

	// Readin some parameters from the ParameterSet.
	itsTreePrefix = globalParameterSet()->getString("prefix");
	itsInstanceNr = globalParameterSet()->getUint32("_instanceNr");
	itsObsPar	  = Observation(globalParameterSet());

	// attach to parent control task
	itsParentControl = ParentControl::instance();
	itsParentPort = new GCFITCPort (*this, *itsParentControl, "ParentITCport", 
									GCFPortInterface::SAP, CONTROLLER_PROTOCOL);
	ASSERTSTR(itsParentPort, "Cannot allocate ITCport for Parentcontrol");
	itsParentPort->open();		// will result in F_CONNECTED

	// need port for timers.
	itsTimerPort = new GCFTimerPort(*this, "TimerPort");

	// prepare TCP port to CalibrationServer.
	itsCalServer = new GCFTCPPort (*this, MAC_SVCMASK_CALSERVER,
											GCFPortInterface::SAP, CAL_PROTOCOL);
	ASSERTSTR(itsCalServer, "Cannot allocate TCPport to CalServer");

	// for debugging purposes
	registerProtocol (CONTROLLER_PROTOCOL, CONTROLLER_PROTOCOL_signalnames);
	registerProtocol (PA_PROTOCOL, 		   PA_PROTOCOL_signalnames);
	registerProtocol (CAL_PROTOCOL,		   CAL_PROTOCOL_signalnames);
	registerProtocol (F_PML_PROTOCOL,	   F_PML_PROTOCOL_signalnames);

	setState(CTState::CREATED);
}


//
// ~CalibrationControl()
//
CalibrationControl::~CalibrationControl()
{
	LOG_TRACE_OBJ_STR (getName() << " destruction");
}

//
// sigintHandler(signum)
//
void CalibrationControl::sigintHandler(int signum)
{
	LOG_DEBUG (formatString("SIGINT signal detected (%d)",signum));

	if (thisCalibrationControl) {
		thisCalibrationControl->finish();
	}
}

//
// finish
//
void CalibrationControl::finish()
{
	TRAN(CalibrationControl::quiting_state);
}


//
// setState(CTstateNr)
//
void    CalibrationControl::setState(CTState::CTstateNr     newState)
{
	itsState = newState;

	if (itsPropertySet) {
		CTState		cts;
		itsPropertySet->setValue(PVSSNAME_FSM_STATE, GCFPVString(cts.name(newState)));
	}
}   


//
// convertFilterSelection(string) : uint8
//
int32 CalibrationControl::convertFilterSelection(const string&	filterselection) 
{
	if (filterselection == "LBL_10_90")		{ return(1); }
	if (filterselection == "LBL_30_80")		{ return(2); }
	if (filterselection == "LBH_10_90")		{ return(3); }
	if (filterselection == "LBH_30_80")		{ return(4); }
	if (filterselection == "HB_110_190") 	{ return(5); }
	if (filterselection == "HB_170_230") 	{ return(6); }
	if (filterselection == "HB_210_250") 	{ return(7); }

	LOG_WARN_STR ("filterselection value '" << filterselection << 
											"' not recognized, using LBL_10_90");
	return (1);
}

//
// handlePropertySetAnswer(answer)
//
void CalibrationControl::handlePropertySetAnswer(GCFEvent& answer)
{
	LOG_DEBUG_STR ("handlePropertySetAnswer:" << evtstr(answer));

	switch(answer.signal) {
    case F_MYPS_ENABLED: {
		GCFPropSetAnswerEvent* pPropAnswer=static_cast<GCFPropSetAnswerEvent*>(&answer);
		if (pPropAnswer->result != GCF_NO_ERROR) {
			LOG_ERROR(formatString("%s : PropertySet %s NOT ENABLED",
										getName().c_str(), pPropAnswer->pScope));
		}
		// always let timer expire so main task will continue.
		itsTimerPort->setTimer(0.5);
		break;
	}

	case F_VGETRESP: 
	case F_VCHANGEMSG: 
		// check which property changed
		// GCFPropValueEvent* pPropAnswer=static_cast<GCFPropValueEvent*>(&answer);
		// TODO: implement something usefull.
		break;

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
// Connect to PVSS and report state back to StartDaemon
//
GCFEvent::TResult CalibrationControl::initial_state(GCFEvent& event, 
													GCFPortInterface& port)
{
	LOG_DEBUG_STR ("initial:" << evtstr(event) << "@" << port.getName());

	GCFEvent::TResult status = GCFEvent::HANDLED;
  
	switch (event.signal) {
    case F_INIT:
   		break;

	case F_ENTRY: {
		// Get access to my own propertyset.
		string	propSetName(createPropertySetName(PSN_CAL_CTRL, getName()));
		LOG_INFO_STR ("Activating PropertySet" << propSetName);
		itsPropertySet = GCFMyPropertySetPtr(new GCFMyPropertySet(propSetName.c_str(),
																  PST_CAL_CTRL,
																  PS_CAT_TEMP_AUTOLOAD,
																  &itsPropertySetAnswer));
		itsPropertySet->enable();
		// Wait for timer that is set in PropertySetAnswer on ENABLED event
		}
		break;

	case F_TIMER:
		if (!itsPropertySetInitialized) {
			itsPropertySetInitialized = true;

			// Instruct CodeLoggingProcessor
			// TODO
			uint32	treeID = getObservationNr(getName());
			LOG_INFO_STR("MACProcessScope: LOFAR.ObsSW.Observation" << treeID << ".CalCtrl");

			// first redirect signalHandler to our finishing state to leave PVSS
			// in the right state when we are going down
			thisCalibrationControl = this;
			signal (SIGINT,  CalibrationControl::sigintHandler);	// ctrl-c
			signal (SIGTERM, CalibrationControl::sigintHandler);	// kill

			// update PVSS.
			LOG_TRACE_FLOW ("Updateing state to PVSS");
			itsPropertySet->setValue(PVSSNAME_FSM_STATE,GCFPVString ("initial"));
			itsPropertySet->setValue(PVSSNAME_FSM_ERROR,GCFPVString (""));
			itsPropertySet->setValue(PN_CC_CONNECTED,	GCFPVBool   (false));
			itsPropertySet->setValue(PN_CC_OBSNAME,		GCFPVString (""));
			itsPropertySet->setValue(PN_CC_ANTENNAARRAY,GCFPVString (""));
			itsPropertySet->setValue(PN_CC_FILTER,		GCFPVString (""));
			itsPropertySet->setValue(PN_CC_NYQUISTZONE,	GCFPVInteger(0));
			itsPropertySet->setValue(PN_CC_RCUS,		GCFPVString (""));
		  
			// Start ParentControl task
			LOG_DEBUG ("Enabling ParentControl task and wait for my name");
			itsParentPort = itsParentControl->registerTask(this);
			// results in CONTROL_CONNECT
		}
		break;

	case F_CONNECTED:
		ASSERTSTR (&port == itsParentPort,
							"F_CONNECTED event from port " << port.getName());
		break;

	case F_DISCONNECTED:
	case F_CLOSED:
	case F_EXIT:
		break;

	case CONTROL_CONNECT: {
		CONTROLConnectEvent		msg(event);
		LOG_DEBUG_STR("Received CONNECT(" << msg.cntlrName << ")");
		setState(CTState::CONNECTED);
		sendControlResult(port, CONTROL_CONNECTED, msg.cntlrName, CT_RESULT_NO_ERROR);
		
		LOG_INFO ("Going to started state");
		TRAN(CalibrationControl::started_state);
		break;
	}

	default:
		status = _defaultEventHandler(event, port);
		break;
	}    

	return (status);
}

//
// started_state(event, port)
//
// wait for CLAIM event
//
GCFEvent::TResult CalibrationControl::started_state(GCFEvent& 		  event, 
													GCFPortInterface& port)
{
	LOG_DEBUG_STR ("started:" << evtstr(event) << "@" << port.getName());

	GCFEvent::TResult status = GCFEvent::HANDLED;

	switch (event.signal) {
	case F_INIT:
	case F_EXIT:
		break;

	case F_ENTRY:
		// update PVSS
		itsPropertySet->setValue(PVSSNAME_FSM_ERROR,GCFPVString(""));
		itsPropertySet->setValue(PN_CC_CONNECTED,GCFPVBool(false));
		break;

	case F_CONNECTED: // CONNECT must be from Calserver.
		ASSERTSTR (&port == itsCalServer, 
									"F_CONNECTED event from port " << port.getName());
		itsTimerPort->cancelAllTimers();
		itsPropertySet->setValue(PN_CC_CONNECTED,GCFPVBool(true));
		LOG_INFO ("Connected with CalServer, going to claimed state");
		setState(CTState::CLAIMED);
		sendControlResult(*itsParentPort, CONTROL_CLAIMED, getName(), CT_RESULT_NO_ERROR);
		TRAN(CalibrationControl::claimed_state);			// go to next state.
		break;

	case F_DISCONNECTED:
		port.close();
		ASSERTSTR (&port == itsCalServer, 
								"F_DISCONNECTED event from port " << port.getName());
		itsPropertySet->setValue(PN_CC_CONNECTED,GCFPVBool(false));
		LOG_WARN("Connection with CalServer failed, retry in 2 seconds");
		itsTimerPort->setTimer(2.0);
		break;

	case F_CLOSED:
		break;

	case F_TIMER:
		LOG_DEBUG ("Trying to reconnect to CalServer");
		itsCalServer->open();		// will result in F_CONN or F_DISCONN
		break;

	// -------------------- EVENTS RECEIVED FROM PARENT CONTROL --------------------
	case CONTROL_CLAIM: {
		CONTROLClaimEvent	msg(event);
		LOG_DEBUG_STR ("Received CLAIM(" << msg.cntlrName << ")");
		setState(CTState::CLAIM);
		LOG_DEBUG ("Trying to connect to CalServer");
		itsCalServer->open();
		break;
	}

	case CONTROL_QUIT:
		TRAN(CalibrationControl::quiting_state);
		break;

	default:
		status = _defaultEventHandler(event, port);
		break;
	}

	return (status);
}
	

//
// claimed_state(event, port)
//
// wait for PREPARE event
//
GCFEvent::TResult CalibrationControl::claimed_state(GCFEvent& 		  event, 
													GCFPortInterface& port)
{
	LOG_DEBUG_STR ("claimed:" << evtstr(event) << "@" << port.getName());

	GCFEvent::TResult status = GCFEvent::HANDLED;

	switch (event.signal) {
	case F_INIT:
	case F_EXIT:
		break;

	case F_ENTRY: {
		// update PVSS
		itsPropertySet->setValue(PVSSNAME_FSM_ERROR, GCFPVString (""));
		itsPropertySet->setValue(PN_CC_OBSNAME,		 GCFPVString (""));
		itsPropertySet->setValue(PN_CC_ANTENNAARRAY, GCFPVString (""));
		itsPropertySet->setValue(PN_CC_FILTER,		 GCFPVString (""));
		itsPropertySet->setValue(PN_CC_NYQUISTZONE,	 GCFPVInteger(0));
		itsPropertySet->setValue(PN_CC_RCUS,		 GCFPVString (""));
		break;
	}

	case F_DISCONNECTED:
		port.close();
		ASSERTSTR (&port == itsCalServer, 
								"F_DISCONNECTED event from port " << port.getName());
		LOG_WARN("Connection with CalServer lost, going to reconnect state.");
		TRAN(CalibrationControl::started_state);
		break;

	case F_CLOSED:
		break;

	// -------------------- EVENTS RECEIVED FROM PARENT CONTROL --------------------
	case CONTROL_PREPARE: {
		CONTROLPrepareEvent		msg(event);
		LOG_DEBUG_STR("Received PREPARE(" << msg.cntlrName << ")");
		setState(CTState::PREPARE);
		if (!startCalibration()) {	// will result in CAL_STARTACK event
			sendControlResult(port, CONTROL_PREPARED, msg.cntlrName, 
														CT_RESULT_CALSTART_FAILED);
			setState(CTState::CLAIMED);
		}
		break;
	}

	case CONTROL_QUIT:
		TRAN(CalibrationControl::quiting_state);
		break;

	// -------------------- EVENTS RECEIVED FROM CALSERVER --------------------
	case CAL_STARTACK: {
		CALStartackEvent		ack(event);
		if (ack.status == SUCCESS || ack.status == ERR_ALREADY_REGISTERED) {
			LOG_INFO ("Start of calibration was succesful");
			sendControlResult(*itsParentPort, CONTROL_PREPARED, getName(),
																CT_RESULT_NO_ERROR);
			setState(CTState::PREPARED);
		}
		else {
			LOG_ERROR("Start of calibration failed, staying in CLAIMED mode");
			sendControlResult(*itsParentPort, CONTROL_PREPARED, getName(),
															CT_RESULT_CALSTART_FAILED);
			setState(CTState::CLAIMED);
		}
	}
	break;

	default:
		status = _defaultEventHandler(event, port);
		break;
	}

	return (status);
}
	

//
// active_state(event, port)
//
// Normal operation state. 
//
GCFEvent::TResult CalibrationControl::active_state(GCFEvent& event, GCFPortInterface& port)
{
	LOG_DEBUG_STR ("active:" << evtstr(event) << "@" << port.getName());

	GCFEvent::TResult status = GCFEvent::HANDLED;

	switch (event.signal) {
	case F_INIT:
	case F_EXIT:
	case F_CLOSED:
		break;

	case F_ENTRY: {
		// update PVSS
		itsPropertySet->setValue(PVSSNAME_FSM_ERROR,GCFPVString(""));
		break;
	}

	case F_DISCONNECTED:
		port.close();
		ASSERTSTR (&port == itsCalServer, 
								"F_DISCONNECTED event from port " << port.getName());
		LOG_DEBUG("Connection with CalServer lost, going to reconnect");
		TRAN(CalibrationControl::started_state);
		break;

	// -------------------- EVENTS RECEIVED FROM PARENT CONTROL --------------------

	case CONTROL_RESUME: {
		CONTROLResumeEvent		msg(event);
		LOG_DEBUG_STR("Received RESUME(" << msg.cntlrName << ")");
		setState(CTState::RESUME);
		sendControlResult(port, CONTROL_RESUMED, getName(), CT_RESULT_NO_ERROR);
		setState(CTState::RESUMED);
		break;
	}

	case CONTROL_SUSPEND: {
		CONTROLSuspendEvent		msg(event);
		LOG_DEBUG_STR("Received SUSPEND(" << msg.cntlrName << ")");
		setState(CTState::SUSPEND);
		sendControlResult(port, CONTROL_SUSPENDED, getName(), CT_RESULT_NO_ERROR);
		setState(CTState::SUSPENDED);
		break;
	}

	case CONTROL_RELEASE: {
		CONTROLReleaseEvent		msg(event);
		LOG_DEBUG_STR("Received RELEASED(" << msg.cntlrName << ")");
		if (!stopCalibration()) {	// will result in CAL_STOPACK event
			sendControlResult(port,CONTROL_RELEASED, getName(), CT_RESULT_CALSTOP_FAILED);
		}
		// wait for CAL_STOPACK
		break;
	}

	case CONTROL_QUIT:
		TRAN(CalibrationControl::quiting_state);
		break;

	// -------------------- EVENTS RECEIVED FROM CALSERVER --------------------

	case CAL_STOPACK: {
		CALStopackEvent			ack(event);
		if (ack.status == SUCCESS) {
			LOG_INFO ("Calibration successfully stopped");
			sendControlResult(*itsParentPort, CONTROL_RELEASED, getName(), 
															CT_RESULT_NO_ERROR);
			setState(CTState::RELEASED);
			TRAN(CalibrationControl::claimed_state);			// go to next state.
		}
		else {
			LOG_ERROR("Stop of calibration failed, going to SUSPENDED mode");
			sendControlResult(*itsParentPort, CONTROL_RELEASED, getName(), 
															CT_RESULT_CALSTOP_FAILED);
			setState(CTState::SUSPENDED);
		}
		break;
	}

	default:
		LOG_DEBUG("active_state, default");
		status = GCFEvent::NOT_HANDLED;
		break;
	}

	return (status);
}


//
// quiting_state(event, port)
//
// Quiting: send QUITED, wait 1 second and stop
//
GCFEvent::TResult CalibrationControl::quiting_state(GCFEvent& 		  event, 
													GCFPortInterface& port)
{
	LOG_DEBUG_STR ("quiting:" << evtstr(event) << "@" << port.getName());

	GCFEvent::TResult status = GCFEvent::HANDLED;

	switch (event.signal) {
	case F_INIT:
	case F_EXIT:
		break;

	case F_ENTRY: {
		// update PVSS
		itsPropertySet->setValue(PVSSNAME_FSM_ERROR,GCFPVString(""));
		itsCalServer->close();
		break;
	}

	case F_DISCONNECTED:
		port.close();
		// fall through!!!
	case F_CLOSED:  {
		ASSERTSTR (&port == itsCalServer,
						"F_DISCONNECTED event from port " << port.getName());
		LOG_DEBUG("Connection with CalServer down, sending FINISH");
		CONTROLQuitedEvent		request;
		request.cntlrName = getName();
		request.result	  = CT_RESULT_NO_ERROR;
		itsParentPort->send(request);
		itsTimerPort->setTimer(1.0);		// let message go away
	}
	break;

	case F_TIMER:
		GCFTask::stop();
		break;

	default:
		status = _defaultEventHandler(event, port);
		break;
	}

	return (status);
}


//
// startCalibration()
//
bool	CalibrationControl::startCalibration() 
{
	// construct and send CALStartEvent
	string		obsName(formatString("observation[%d]{%d}", getInstanceNr(getName()), 
															getObservationNr(getName())));
	CALStartEvent calStartEvent;
	calStartEvent.name   	   = obsName;
	calStartEvent.parent 	   = itsObsPar.antennaArray;
	calStartEvent.rcumode().resize(1);
	calStartEvent.rcumode()(0).setMode((RSP_Protocol::RCUSettings::Control::RCUMode)
										convertFilterSelection(itsObsPar.filter));
//	calStartEvent.nyquist_zone = itsObsPar.nyquistZone;
	calStartEvent.subset 	   = itsObsPar.RCUset;
	LOG_DEBUG(formatString("Sending CALSTART(%s,%s,08X)", 
							obsName.c_str(), calStartEvent.parent.c_str(),
							calStartEvent.rcumode()(0).getRaw()));

	itsCalServer->send(calStartEvent);

	// inform operator about these values.
	itsPropertySet->setValue(PN_CC_OBSNAME,		GCFPVString(obsName));
	itsPropertySet->setValue(PN_CC_ANTENNAARRAY,GCFPVString(itsObsPar.antennaArray));
	itsPropertySet->setValue(PN_CC_FILTER,		GCFPVString(itsObsPar.filter));
	itsPropertySet->setValue(PN_CC_NYQUISTZONE,	GCFPVInteger(itsObsPar.nyquistZone));
	itsPropertySet->setValue(PN_CC_RCUS,		GCFPVString(
		APLUtilities::compactedArrayString(globalParameterSet()->
		getString("Observation.receiverList"))));

	return (true);
}


//
// stopCalibration()
//
bool	CalibrationControl::stopCalibration()
{
	// send CALStopEvent
	string		obsName(formatString("observation[%d]{%d}", getInstanceNr(getName()), 
															getObservationNr(getName())));
	LOG_DEBUG_STR ("Sending CALSTOP(" << obsName << ") to CALserver");
	CALStopEvent calStopEvent;
	calStopEvent.name = obsName;
	itsCalServer->send(calStopEvent);

	return (true);
}

// _defaultEventHandler(event, port)
//
GCFEvent::TResult CalibrationControl::_defaultEventHandler(GCFEvent&		 event,
														   GCFPortInterface& port)
{
	CTState     cts;
	LOG_DEBUG_STR("Received " << evtstr(event) << " in state " << cts.name(itsState)
					<< ". DEFAULT handling.");

	GCFEvent::TResult   result(GCFEvent::NOT_HANDLED);

	switch (event.signal) {
		case CONTROL_CONNECT:
		case CONTROL_RESYNC:
		case CONTROL_SCHEDULE:  // we should do something with this
		case CONTROL_CLAIM:
		case CONTROL_PREPARE:
		case CONTROL_RESUME:
		case CONTROL_SUSPEND:
		case CONTROL_RELEASE:
		case CONTROL_QUIT:
			if (sendControlResult(port, event.signal, getName(), CT_RESULT_NO_ERROR)) {
				result = GCFEvent::HANDLED;
			}
			break;

		case CONTROL_CONNECTED:
		case CONTROL_RESYNCED:
		case CONTROL_SCHEDULED:
		case CONTROL_CLAIMED:
		case CONTROL_PREPARED:
		case CONTROL_RESUMED:
		case CONTROL_SUSPENDED:
		case CONTROL_RELEASED:
		case CONTROL_QUITED:
			result = GCFEvent::HANDLED;
			break;
	}

	if (result == GCFEvent::NOT_HANDLED) {
		LOG_WARN_STR("Event " << evtstr(event) << " NOT handled in state " <<
					cts.name(itsState));
	}

	return (result);
}

//
// _connectedHandler(port)
//
void CalibrationControl::_connectedHandler(GCFPortInterface& /*port*/)
{
}

//
// _disconnectedHandler(port)
//
void CalibrationControl::_disconnectedHandler(GCFPortInterface& port)
{
	port.close();
}


}; // StationCU
}; // LOFAR
