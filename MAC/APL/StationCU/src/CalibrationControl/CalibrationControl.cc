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
#include <Common/Version.h>
#include <ApplCommon/StationConfig.h>
#include <ApplCommon/StationInfo.h>

#include <MACIO/MACServiceInfo.h>
#include <GCF/PVSS/GCF_PVTypes.h>
#include <GCF/RTDB/DP_Protocol.ph>
#include <APL/APLCommon/Controller_Protocol.ph>
#include <APL/CAL_Protocol/CAL_Protocol.ph>
#include <APL/RTDBCommon/RTDButilities.h>
#include <signal.h>

#include "CalibrationControl.h"
#include "PVSSDatapointDefs.h"
#include <StationCU/Package__Version.h>

using namespace LOFAR::GCF::TM;
using namespace LOFAR::GCF::PVSS;
using namespace LOFAR::GCF::RTDB;
using namespace LOFAR::APL::RTDBCommon;

namespace LOFAR {
	using namespace APLCommon;
	namespace StationCU {
	
// static pointer to this object for signal handler
static CalibrationControl*	thisCalibrationControl = 0;
static uint16				gResult = CT_RESULT_NO_ERROR;

//
// CalibrationControl()
//
CalibrationControl::CalibrationControl(const string&	cntlrName) :
	GCFTask 			((State)&CalibrationControl::initial_state,cntlrName),
	itsPropertySet		(0),
	itsPropertySetInitialized (false),
	itsParentControl	(0),
	itsParentPort		(0),
	itsTimerPort		(0),
	itsCalServer		(0),
	itsState			(CTState::NOSTATE)
{
	LOG_TRACE_OBJ_STR (cntlrName << " construction");
	LOG_INFO(Version::getInfo<StationCUVersion>("CalibrationControl"));

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
	registerProtocol (CONTROLLER_PROTOCOL, CONTROLLER_PROTOCOL_STRINGS);
	registerProtocol (DP_PROTOCOL, 		DP_PROTOCOL_STRINGS);
	registerProtocol (CAL_PROTOCOL,		CAL_PROTOCOL_STRINGS);

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
		itsPropertySet->setValue(PN_FSM_CURRENT_ACTION, GCFPVString(cts.name(newState)));
	}
}   


//
// convertFilterSelection(string) : uint8
//
int32 CalibrationControl::convertFilterSelection(const string&	filterselection, const string&	antennaSet) 
{
	// support old filters
	if (filterselection == "LBL_10_80")		{ return(1); }
	if (filterselection == "LBL_30_80")		{ return(2); }
	if (filterselection == "LBH_10_80")		{ return(3); }
	if (filterselection == "LBH_30_80")		{ return(4); }
	if (filterselection == "HB_100_190")	{ return(5); }
	if (filterselection == "HB_170_230")	{ return(6); }
	if (filterselection == "HB_210_240")	{ return(7); }

	// support new filternames
	if (antennaSet == "LBA_OUTER") {
		if (filterselection == "LBA_10_90")	{ return(1); }
		if (filterselection == "LBA_30_80")	{ return(2); }
	}
	if (antennaSet == "LBA_INNER") {
		if (filterselection == "LBA_10_90")	{ return(3); }
		if (filterselection == "LBA_30_80")	{ return(4); }
	}
	if (filterselection == "HBA_110_190")	{ return(5); }
	if (filterselection == "HBA_170_230")	{ return(6); }
	if (filterselection == "HBA_210_250")	{ return(7); }

	LOG_WARN_STR ("filterselection value '" << filterselection << 
									"' not recognized, using LBL_10_80");
	return (1);
}

//
// initial_state(event, port)
//
// Connect to PVSS and report state back to StartDaemon
//
GCFEvent::TResult CalibrationControl::initial_state(GCFEvent& event, 
													GCFPortInterface& port)
{
	LOG_DEBUG_STR ("initial:" << eventName(event) << "@" << port.getName());

	GCFEvent::TResult status = GCFEvent::HANDLED;
  
	switch (event.signal) {
    case F_INIT:
   		break;

	case F_ENTRY: {
		// Get access to my own propertyset.
		string	propSetName(createPropertySetName(PSN_CALIBRATION_CONTROL, getName(),
												  globalParameterSet()->getString("_DPname")));
		LOG_INFO_STR ("Activating PropertySet" << propSetName);
		itsPropertySet = new RTDBPropertySet(propSetName,
											 PST_CALIBRATION_CONTROL,
											 PSAT_RW | PSAT_TMP,
											 this);
		// Wait for timer that is set on DP_CREATED event

		// Instruct loggingProcessor
//		LOG_INFO_STR("MACProcessScope: LOFAR.ObsSW.Observation" << treeID << ".CalibratonControl");
		LOG_INFO_STR("MACProcessScope: " << propSetName);
		// NOTE: the SASgateway is not yet aware of claimMgr so the data will not be transferred to SAS.
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
		if (!itsPropertySetInitialized) {
			itsPropertySetInitialized = true;

			// first redirect signalHandler to our finishing state to leave PVSS
			// in the right state when we are going down
			thisCalibrationControl = this;
			signal (SIGINT,  CalibrationControl::sigintHandler);	// ctrl-c
			signal (SIGTERM, CalibrationControl::sigintHandler);	// kill

			// update PVSS.
			LOG_TRACE_FLOW ("Updateing state to PVSS");
			GCFPValueArray		beamNameArr;
			itsPropertySet->setValue(PN_FSM_CURRENT_ACTION,	GCFPVString ("initial"));
			itsPropertySet->setValue(PN_FSM_ERROR,			GCFPVString (""));
			itsPropertySet->setValue(PN_CC_CONNECTED,		GCFPVBool   (false));
			itsPropertySet->setValue(PN_CC_BEAM_NAMES,		GCFPVDynArr (LPT_DYNSTRING, beamNameArr));
			itsPropertySet->setValue(PN_CC_ANTENNA_ARRAY,	GCFPVString (""));
			itsPropertySet->setValue(PN_CC_FILTER,			GCFPVString (""));
			itsPropertySet->setValue(PN_CC_NYQUISTZONE,		GCFPVInteger(0));
			itsPropertySet->setValue(PN_CC_RCUS,			GCFPVString (""));
		  
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
	case F_EXIT:
		break;

	case CONTROL_CONNECT: {
		CONTROLConnectEvent		msg(event);
		LOG_DEBUG_STR("Received CONNECT(" << msg.cntlrName << ")");
		setState(CTState::CONNECTED);
		sendControlResult(port, CONTROL_CONNECTED, msg.cntlrName, CT_RESULT_NO_ERROR);
		
		// let ParentControl watch over the start and stop times for extra safety.
		ptime	startTime = time_from_string(globalParameterSet()->
													getString("Observation.startTime"));
		ptime	stopTime  = time_from_string(globalParameterSet()->
													getString("Observation.stopTime"));
		itsParentControl->activateObservationTimers(msg.cntlrName, startTime, stopTime);

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
	LOG_DEBUG_STR ("started:" << eventName(event) << "@" << port.getName());

	GCFEvent::TResult status = GCFEvent::HANDLED;

	switch (event.signal) {
	case F_INIT:
	case F_EXIT:
		break;

	case F_ENTRY:
		// update PVSS
		itsPropertySet->setValue(PN_FSM_ERROR,GCFPVString(""));
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
	LOG_DEBUG_STR ("claimed:" << eventName(event) << "@" << port.getName());

	GCFEvent::TResult status = GCFEvent::HANDLED;

	switch (event.signal) {
	case F_INIT:
	case F_EXIT:
		break;

	case F_ENTRY: {
		// update PVSS
		GCFPValueArray		beamNameArr;
		itsPropertySet->setValue(PN_FSM_ERROR, 			GCFPVString (""));
		itsPropertySet->setValue(PN_CC_BEAM_NAMES,	 	GCFPVDynArr (LPT_DYNSTRING, beamNameArr));
		itsPropertySet->setValue(PN_CC_ANTENNA_ARRAY,	GCFPVString (""));
		itsPropertySet->setValue(PN_CC_FILTER,		 	GCFPVString (""));
		itsPropertySet->setValue(PN_CC_NYQUISTZONE,	 	GCFPVInteger(0));
		itsPropertySet->setValue(PN_CC_RCUS,		 	GCFPVString (""));
		break;
	}

	case F_DISCONNECTED:
		port.close();
		ASSERTSTR (&port == itsCalServer, 
								"F_DISCONNECTED event from port " << port.getName());
		LOG_WARN("Connection with CalServer lost, going to reconnect state.");
		TRAN(CalibrationControl::started_state);
		break;

	// -------------------- EVENTS RECEIVED FROM PARENT CONTROL --------------------
	case CONTROL_PREPARE: {
		CONTROLPrepareEvent		msg(event);
		LOG_DEBUG_STR("Received PREPARE(" << msg.cntlrName << ")");
		setState(CTState::PREPARE);
		gResult = CT_RESULT_NO_ERROR;
		if (!startCalibration()) {	// will result in CAL_STARTACK event
			sendControlResult(port, CONTROL_PREPARED, msg.cntlrName, CT_RESULT_CALSTART_FAILED);
			setState(CTState::CLAIMED);
			setObjectState("No beams specified!", itsPropertySet->getFullScope(), RTDB_OBJ_STATE_SUSPICIOUS);
		}
		break;
	}

	case CONTROL_QUIT:
		TRAN(CalibrationControl::quiting_state);
		break;

	// -------------------- EVENTS RECEIVED FROM CALSERVER --------------------
	case CAL_STARTACK: {
		CALStartackEvent		ack(event);
		gResult |= ack.status;		// update overall result
		if (ack.status == CAL_SUCCESS || ack.status == ERR_ALREADY_REGISTERED) {
			LOG_INFO_STR ("Start of the calibration of beam " << ack.name << " was succesful");
			itsBeams[ack.name] = true;					// add to beammap
		}
		else {
			LOG_ERROR_STR("Start of calibration of beam " << ack.name << 
															" failed, staying in CLAIMED mode");
			itsBeams[ack.name] = false;					// add to beammap
			setObjectState("Cannot start beam", itsPropertySet->getFullScope(), RTDB_OBJ_STATE_BROKEN);
		}
		_showBeamAdmin();

		if (itsBeams.size() == itsNrBeams) {	// answer for all beams received? report state
			if (gResult == CAL_SUCCESS || gResult == ERR_ALREADY_REGISTERED) {
				LOG_INFO("Calibration of all beams started sucesfully, going to active-mode");
				sendControlResult(*itsParentPort, CONTROL_PREPARED, getName(), CT_RESULT_NO_ERROR);
				setState(CTState::PREPARED);
				TRAN(CalibrationControl::active_state);			// go to next state.
			}
			else {
				LOG_INFO("Not all beams were calibrated right, staying in claiming mode");
				setObjectState("Cannot start calibration", itsPropertySet->getFullScope(), RTDB_OBJ_STATE_BROKEN);
				sendControlResult(*itsParentPort, CONTROL_PREPARED, getName(), CT_RESULT_CALSTART_FAILED);
				setState(CTState::CLAIMED);
			}
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
	LOG_DEBUG_STR ("active:" << eventName(event) << "@" << port.getName());

	GCFEvent::TResult status = GCFEvent::HANDLED;

	switch (event.signal) {
	case F_INIT:
	case F_EXIT:
		break;

	case F_ENTRY: {
		// update PVSS
		itsPropertySet->setValue(PN_FSM_ERROR,GCFPVString(""));
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
		setState(CTState::RELEASE);
		gResult = CT_RESULT_NO_ERROR;
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
		gResult |= ack.status;
		if (ack.status == CAL_SUCCESS) {
			LOG_INFO_STR ("Calibration of beam " << ack.name << " successfully stopped");
		}
		else {
			LOG_WARN_STR ("Calibration of beam " << ack.name << " stopped with ERRORs");
		}

		// remove beam from the map
		map<string, bool>::iterator	iter = itsBeams.begin();
		map<string, bool>::iterator	end  = itsBeams.end ();
		while(iter != end) {
			if (iter->first == ack.name) {
				LOG_DEBUG_STR("Removing beam " << ack.name << " from the administration");
				itsBeams.erase(iter);
				break;
			}
			iter++;
		}
		_showBeamAdmin();

		if (itsBeams.empty()) {		// all beams stopped?
			if (gResult == CAL_SUCCESS) {
				LOG_INFO("All calibrations stopped, going to RELEASED state");
				sendControlResult(*itsParentPort, CONTROL_RELEASED, getName(), CT_RESULT_NO_ERROR);
				setState(CTState::RELEASED);
				TRAN(CalibrationControl::claimed_state);			// go to next state.
			}
			else {
				LOG_ERROR("Stop of some calibrations failed, staying in SUSPENDED mode");
				setObjectState("Cannot stop the calibration", itsPropertySet->getFullScope(), RTDB_OBJ_STATE_BROKEN);
				sendControlResult(*itsParentPort, CONTROL_RELEASED, getName(), 
																CT_RESULT_CALSTOP_FAILED);
				setState(CTState::SUSPENDED);
			}
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
	LOG_DEBUG_STR ("quiting:" << eventName(event) << "@" << port.getName());

	GCFEvent::TResult status = GCFEvent::HANDLED;

	switch (event.signal) {
	case F_INIT:
	case F_EXIT:
		break;

	case F_ENTRY: {
		// update PVSS
		setState(CTState::QUIT);
		// tell Parent task we like to go down.
		itsParentControl->nowInState(getName(), CTState::QUIT);

		itsPropertySet->setValue(PN_FSM_ERROR,GCFPVString(""));
		itsCalServer->close();

		LOG_DEBUG("Connection with CalServer closed, sending QUITED");
		CONTROLQuitedEvent		request;
		request.cntlrName = getName();
		request.result	  = CT_RESULT_NO_ERROR;
		itsParentPort->send(request);
		itsTimerPort->setTimer(1.0);		// let message go away
	}
	break;

	case F_TIMER:
		GCFScheduler::instance()->stop();
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
	bool	stereoBeams((itsObsPar.antennaSet == "HBA_BOTH") & (stationRingName() == "Core"));
	itsNrBeams = itsObsPar.beams.size() * (stereoBeams ? 2 : 1);
	LOG_DEBUG_STR("Calibrating " << itsNrBeams << " beams.");
	if (itsNrBeams == 0) {
		LOG_WARN("No beams specified");
		return (false);
	}

	GCFPValueArray		beamNameArr;
	for (uint32	i(0); i < itsObsPar.beams.size(); i++) {
		// construct and send CALStartEvent
		string		beamName(itsObsPar.getBeamName(i));

		CALStartEvent calStartEvent;
		calStartEvent.name   = beamName;
		StationConfig		config;
		// TODO: As long as the AntennaArray.conf uses different names as SAS we have to use this dirty hack.
		calStartEvent.parent = itsObsPar.getAntennaArrayName(config.hasSplitters);
		calStartEvent.rcumode().resize(1);
		calStartEvent.rcumode()(0).setMode((RSP_Protocol::RCUSettings::Control::RCUMode)
											convertFilterSelection(itsObsPar.filter, itsObsPar.antennaSet));
		calStartEvent.subset = itsObsPar.getRCUbitset(config.nrLBAs, config.nrHBAs, config.nrRSPs, config.hasSplitters);

		// Note: when HBA_BOTH is selected we should set up a calibration on both HBA_0 and HBA_1 field.
		if (!stereoBeams) {
			LOG_DEBUG(formatString("Sending CALSTART(%s,%s,%08X)", 
									calStartEvent.name.c_str(), calStartEvent.parent.c_str(),
									calStartEvent.rcumode()(0).getRaw()));
			itsCalServer->send(calStartEvent);
			beamNameArr.push_back(new GCFPVString(beamName));	// update array for PVSS
		}
		else {
			for (int rcu = config.nrHBAs; rcu < config.nrHBAs*2; rcu++) {	// clear second half of RCUs
				calStartEvent.subset.reset(rcu);
			}
			calStartEvent.parent = "HBA_0";
			calStartEvent.name   = beamName + "_0";
			LOG_DEBUG(formatString("Sending CALSTART(%s,%s,%08X)", 
									calStartEvent.name.c_str(), calStartEvent.parent.c_str(),
									calStartEvent.rcumode()(0).getRaw()));
			itsCalServer->send(calStartEvent);
			beamNameArr.push_back(new GCFPVString(calStartEvent.name));	// update array for PVSS

			calStartEvent.subset = itsObsPar.getRCUbitset(config.nrLBAs, config.nrHBAs, config.nrRSPs, config.hasSplitters);
			calStartEvent.parent = "HBA_1";
			calStartEvent.name   = beamName + "_1";
			for (int rcu = 0; rcu < config.nrHBAs; rcu++) {	// clear first half of RCUs
				calStartEvent.subset.reset(rcu);
			}
			LOG_DEBUG(formatString("Sending CALSTART(%s,%s,%08X)", 
									calStartEvent.name.c_str(), calStartEvent.parent.c_str(),
									calStartEvent.rcumode()(0).getRaw()));
			itsCalServer->send(calStartEvent);
			beamNameArr.push_back(new GCFPVString(calStartEvent.name));	// update array for PVSS
		}
	} // for all beams

	// inform operator about these values.
	itsPropertySet->setValue(PN_CC_BEAM_NAMES,	 GCFPVDynArr(LPT_DYNSTRING, beamNameArr));
	itsPropertySet->setValue(PN_CC_ANTENNA_ARRAY,GCFPVString(itsObsPar.antennaArray));
	itsPropertySet->setValue(PN_CC_FILTER,		 GCFPVString(itsObsPar.filter));
	itsPropertySet->setValue(PN_CC_NYQUISTZONE,	 GCFPVInteger(itsObsPar.nyquistZone));
	itsPropertySet->setValue(PN_CC_RCUS,		 GCFPVString(
										compactedArrayString(globalParameterSet()->
										getString("Observation.receiverList"))));
	return (true);
}


//
// stopCalibration()
//
bool	CalibrationControl::stopCalibration()
{
	LOG_DEBUG_STR("Stopping calibration of " << itsBeams.size() << " beams.");

	map<string, bool>::const_iterator	iter = itsBeams.begin();
	map<string, bool>::const_iterator	end  = itsBeams.end();
	while (iter != end) {
		LOG_DEBUG_STR ("Sending CALSTOP(" << iter->first << ") to CALserver");
		CALStopEvent calStopEvent;
		calStopEvent.name = iter->first;
		itsCalServer->send(calStopEvent);
		++iter;
	}

	return (true);
}

// _defaultEventHandler(event, port)
//
GCFEvent::TResult CalibrationControl::_defaultEventHandler(GCFEvent&		 event,
														   GCFPortInterface& port)
{
	CTState     cts;
	LOG_DEBUG_STR("Received " << eventName(event) << " in state " << cts.name(itsState)
					<< ". DEFAULT handling.");

	GCFEvent::TResult   result(GCFEvent::NOT_HANDLED);

	switch (event.signal) {
		case CONTROL_CONNECT:
		case CONTROL_RESYNC:
		case CONTROL_SCHEDULE:  // handled by parentControl task
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

		case DP_CHANGED:
		case DP_SET:
			result = GCFEvent::HANDLED;
			break;

	}

	if (result == GCFEvent::NOT_HANDLED) {
		LOG_WARN_STR("Event " << eventName(event) << " NOT handled in state " <<
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

//
// _showBeamAdmin()
//
void CalibrationControl::_showBeamAdmin()
{
	map<string, bool>::const_iterator	iter = itsBeams.begin();
	map<string, bool>::const_iterator	end  = itsBeams.end();
	while (iter != end) {
		LOG_DEBUG_STR("Beam " << iter->first << " is " << (iter->second ? "ON" : "OFF"));
		++iter;
	}
}

}; // StationCU
}; // LOFAR
