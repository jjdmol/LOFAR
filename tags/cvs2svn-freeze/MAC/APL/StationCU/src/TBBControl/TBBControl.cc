//#  TBBControl.cc: Implementation of the MAC Scheduler task
//#
//#  Copyright (C) 2007
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
#include <APL/APLCommon/Observation.h>
#include <APL/TBB_Protocol/TBB_Protocol.ph>
#include <GCF/RTDB/DP_Protocol.ph>
#include <signal.h>

#include "TBBControl.h"
#include "TBBControlDefines.h"

using namespace LOFAR::GCF::Common;
using namespace LOFAR::GCF::TM;
using namespace LOFAR::GCF::RTDB;
using namespace std;

namespace LOFAR {
	using namespace APLCommon;
	using namespace ACC::APS;
	namespace StationCU {
	
// static pointer to this object for signal handler
static TBBControl*	thisTBBControl = 0;

//
// TBBControl()
//
TBBControl::TBBControl(const string&	cntlrName) :
	GCFTask 			((State)&TBBControl::initial_state,cntlrName),
	itsPropertySet		(0),
	itsPropertySetInitialized (false),
	itsParentControl	(0),
	itsParentPort		(0),
	itsTimerPort		(0),
	itsTBBDriver		(0),
	itsState			(CTState::NOSTATE),
	itsTBBID			(0)
{
	LOG_TRACE_OBJ_STR (cntlrName << " construction");

	// First readin our observation related config file.
	LOG_DEBUG_STR("Reading parset file:" << LOFAR_SHARE_LOCATION << "/" << cntlrName);
	globalParameterSet()->adoptFile(string(LOFAR_SHARE_LOCATION)+"/"+cntlrName);

	// Readin some parameters from the ParameterSet.
	itsTreePrefix = globalParameterSet()->getString("prefix");
	itsInstanceNr = globalParameterSet()->getUint32("_instanceNr");

	// attach to parent control task
	itsParentControl = ParentControl::instance();
	itsParentPort = new GCFITCPort (*this, *itsParentControl, "ParentITCport", 
									GCFPortInterface::SAP, CONTROLLER_PROTOCOL);
	ASSERTSTR(itsParentPort, "Cannot allocate ITCport for Parentcontrol");
	itsParentPort->open();		// will result in F_CONNECTED

	// need port for timers.
	itsTimerPort = new GCFTimerPort(*this, "TimerPort");

	// prepare TCP port to TBBDriver.
	itsTBBDriver = new GCFTCPPort (*this, MAC_SVCMASK_TBBDRIVER,
											GCFPortInterface::SAP, TBB_PROTOCOL);
	ASSERTSTR(itsTBBDriver, "Cannot allocate TCPport to TBBDriver");

	// for debugging purposes
	GCF::TM::registerProtocol (CONTROLLER_PROTOCOL, CONTROLLER_PROTOCOL_STRINGS);
	GCF::TM::registerProtocol (DP_PROTOCOL, 		DP_PROTOCOL_STRINGS);
	GCF::TM::registerProtocol (TBB_PROTOCOL, 		TBB_PROTOCOL_STRINGS);

	setState(CTState::CREATED);
}


//
// ~TBBControl()
//
TBBControl::~TBBControl()
{
	LOG_TRACE_OBJ_STR (getName() << " destruction");

}

//
// sigintHandler(signum)
//
void TBBControl::sigintHandler(int signum)
{
	LOG_DEBUG (formatString("SIGINT signal detected (%d)",signum));

	if (thisTBBControl) {
		thisTBBControl->finish();
	}
}

//
// finish
//
void TBBControl::finish()
{
	TRAN(TBBControl::quiting_state);
}


//
// setState(CTstateNr)
//
void    TBBControl::setState(CTState::CTstateNr     newState)
{
	itsState = newState;

	if (itsPropertySet) {
		CTState		cts;
		itsPropertySet->setValue(PVSSNAME_FSM_CURACT, GCFPVString(cts.name(newState)));
	}
}   


//
// initial_state(event, port)
//
// Connect to PVSS and report state back to startdaemon
//
GCFEvent::TResult TBBControl::initial_state(GCFEvent& event, 
													GCFPortInterface& port)
{
	LOG_DEBUG_STR ("initial:" << eventName(event) << "@" << port.getName());

	GCFEvent::TResult status = GCFEvent::HANDLED;
  
	switch (event.signal) {
	case F_ENTRY:
   		break;

    case F_INIT: {
		// Get access to my own propertyset.
		string	propSetName(createPropertySetName(PSN_TBB_CTRL, getName()));
		LOG_INFO_STR ("Activating PropertySet" << propSetName);
		itsPropertySet = new RTDBPropertySet(propSetName,
											 PST_TBB_CTRL,
											 PSAT_RW,
											 this);
		// Wait for timer that is set on DP_CREATED event
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

			// Instruct codeloggingProcessor
			// TODO: get this from a .h file
			LOG_INFO_STR("MACProcessScope: LOFAR.PermSW.TBBCtrl");

			// first redirect signalHandler to our quiting state to leave PVSS
			// in the right state when we are going down
			thisTBBControl = this;
			signal (SIGINT,  TBBControl::sigintHandler);	// ctrl-c
			signal (SIGTERM, TBBControl::sigintHandler);	// kill

			// update PVSS.
			LOG_TRACE_FLOW ("Updateing state to PVSS");
// TODO TBB
			itsPropertySet->setValue(PVSSNAME_FSM_CURACT,GCFPVString("initial"));
			itsPropertySet->setValue(PVSSNAME_FSM_ERROR,GCFPVString(""));
			itsPropertySet->setValue(PN_BC_CONNECTED,	GCFPVBool  (false));
			itsPropertySet->setValue(PN_BC_SUBBANDLIST,	GCFPVString(""));
			itsPropertySet->setValue(PN_BC_BEAMLETLIST,	GCFPVString(""));
			itsPropertySet->setValue(PN_BC_ANGLE1,		GCFPVString(""));
			itsPropertySet->setValue(PN_BC_ANGLE2,		GCFPVString(""));
			itsPropertySet->setValue(PN_BC_ANGLETIMES,	GCFPVString(""));
			itsPropertySet->setValue(PN_BC_SUBARRAY,	GCFPVString(""));
			itsPropertySet->setValue(PN_BC_BEAMID,		GCFPVInteger(0));
		  
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

		// let ParentControl watch over the start and stop times for extra safety.
		ptime	startTime = time_from_string(globalParameterSet()->
													getString("Observation.startTime"));
		ptime	stopTime  = time_from_string(globalParameterSet()->
													getString("Observation.stopTime"));
		itsParentControl->activateObservationTimers(msg.cntlrName, startTime, stopTime);

		LOG_INFO ("Going to started state");
		TRAN(TBBControl::started_state);				// go to next state.
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
GCFEvent::TResult TBBControl::started_state(GCFEvent& event, GCFPortInterface& port)
{
	LOG_DEBUG_STR ("started:" << eventName(event) << "@" << port.getName());

	GCFEvent::TResult status = GCFEvent::HANDLED;

	switch (event.signal) {
	case F_ENTRY: {
		// update PVSS
//		itsPropertySet->setValue(string(PVSSNAME_FSM_CURACT),GCFPVString("started"));
		itsPropertySet->setValue(PVSSNAME_FSM_ERROR,GCFPVString(""));
		itsPropertySet->setValue(PN_BC_CONNECTED,	GCFPVBool  (false)); //[TBB]
		break;
	}

	case F_INIT:
	case F_EXIT:
		break;

	case F_CONNECTED: {
		ASSERTSTR (&port == itsTBBDriver, "F_CONNECTED event from port " 
																	<< port.getName());
		itsTimerPort->cancelAllTimers();
		LOG_INFO ("Connected with TBBDriver, going to claimed state");
		itsPropertySet->setValue(PN_BC_CONNECTED,	GCFPVBool(true));	// [TBB]
		setState(CTState::CLAIMED);
		sendControlResult(*itsParentPort, CONTROL_CLAIMED, getName(), CT_RESULT_NO_ERROR);
		TRAN(TBBControl::claimed_state);				// go to next state.
		break;
	}

	case F_DISCONNECTED: {
		port.close();
		ASSERTSTR (&port == itsTBBDriver, 
								"F_DISCONNECTED event from port " << port.getName());
		LOG_WARN ("Connection with TBBDriver failed, retry in 2 seconds");
		itsTimerPort->setTimer(2.0);
		break;
	}

	case F_CLOSED:
		break;

	case F_TIMER: 
//		GCFTimerEvent& timerEvent=static_cast<GCFTimerEvent&>(event);
		LOG_DEBUG ("Trying to reconnect to TBBDriver");
		itsTBBDriver->open();		// will result in F_CONN or F_DISCONN
		break;

	// -------------------- EVENTS RECEIVED FROM PARENT CONTROL --------------------
	case CONTROL_CLAIM: {
		CONTROLClaimEvent		msg(event);
		LOG_DEBUG_STR("Received CLAIM(" << msg.cntlrName << ")");
		setState(CTState::CLAIM);
		LOG_DEBUG ("Trying to connect to TBBDriver");
		itsTBBDriver->open();		// will result in F_CONN or F_DISCONN
		break;
	}

	case CONTROL_QUIT:
		TRAN(TBBControl::quiting_state);
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
// wait for PREPARE event.
//
GCFEvent::TResult TBBControl::claimed_state(GCFEvent& event, GCFPortInterface& port)
{
	LOG_DEBUG_STR ("claimed:" << eventName(event) << "@" << port.getName());

	GCFEvent::TResult status = GCFEvent::HANDLED;

	switch (event.signal) {
	case F_ENTRY: {
		// update PVSS
//		itsPropertySet->setValue(string(PVSSNAME_FSM_CURACT),GCFPVString("claimed"));
		itsPropertySet->setValue(PVSSNAME_FSM_ERROR, GCFPVString(""));
		break;
	}

	case F_INIT:
	case F_EXIT:
		break;

	case F_DISCONNECTED: {
		port.close();
		ASSERTSTR (&port == itsTBBDriver, 
								"F_DISCONNECTED event from port " << port.getName());
		LOG_WARN("Connection with TBBDriver lost, going to reconnect state.");
		TRAN(TBBControl::started_state);
		break;
	}

	case F_CLOSED:
		break;

	// -------------------- EVENTS RECEIVED FROM PARENT CONTROL --------------------
	case CONTROL_PREPARE: {
		CONTROLPrepareEvent		msg(event);
		LOG_DEBUG_STR("Received PREPARE(" << msg.cntlrName << ")");
		setState(CTState::PREPARE);
		// try to send a BEAMALLOC event to the TBBDriver.
		if (!doPrepare()) { // could not sent it?
			LOG_WARN_STR("TBBallocation error: nr subbands != nr beamlets" << 
						 ", staying in CLAIMED mode");	// [TBB]
			setState(CTState::CLAIMED);
			LOG_DEBUG_STR("Sending PREPARED(" << getName() << "," << 
												CT_RESULT_CONFLICTING_ARGS << ")");
			sendControlResult(*itsParentPort, CONTROL_PREPARED, getName(), 
															CT_RESULT_CONFLICTING_ARGS);
		}
		// else a BS_BEAMALLOCACK event will happen.
		break;
	}

	case CONTROL_QUIT:
		TRAN(TBBControl::quiting_state);
		break;

	// -------------------- EVENTS RECEIVED FROM BEAMSERVER --------------------
//TODO TBB
	case BS_BEAMALLOCACK: {
		BSTBBallocackEvent		msg(event);
		uint16 result  = handleTBBAllocAck(event);
		if (result == CT_RESULT_NO_ERROR) {
			setState(CTState::PREPARED);
			LOG_INFO("TBB allocated, going to active state");
			TRAN(TBBControl::active_state);
		}
		else {
			LOG_WARN_STR("TBBallocation failed with error " << result << 
						 ", staying in CLAIMED mode");
			setState(CTState::CLAIMED);
		}

		LOG_DEBUG_STR("Sending PREPARED(" << getName() << "," << result << ") event");
		sendControlResult(*itsParentPort, CONTROL_PREPARED, getName(), result);
		break;
	}

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
GCFEvent::TResult TBBControl::active_state(GCFEvent& event, GCFPortInterface& port)
{
	LOG_DEBUG_STR ("active:" << eventName(event) << "@" << port.getName());

	GCFEvent::TResult status = GCFEvent::HANDLED;

	switch (event.signal) {
	case F_ENTRY: {
		// update PVSS
//		itsPropertySet->setValue(string(PVSSNAME_FSM_CURACT),GCFPVString("active"));
		itsPropertySet->setValue(PVSSNAME_FSM_ERROR, GCFPVString(""));
		break;
	}

	case F_INIT:
	case F_EXIT:
		break;

	case F_DISCONNECTED: {
		port.close();
		ASSERTSTR (&port == itsTBBDriver, 
								"F_DISCONNECTED event from port " << port.getName());
		LOG_WARN("Connection with TBBDriver lost, going to reconnect");
		TRAN(TBBControl::started_state);
		break;
	}
	
	case F_CLOSED:
		break;

	// -------------------- EVENTS RECEIVED FROM PARENT CONTROL --------------------

	case CONTROL_SCHEDULE: {
		CONTROLScheduledEvent		msg(event);
		LOG_DEBUG_STR("Received SCHEDULE(" << msg.cntlrName << ")");
		// TODO: do something usefull with this information!
		break;
	}

	case CONTROL_RESUME: {		// nothing to do, just send answer
		CONTROLResumeEvent		msg(event);
		LOG_DEBUG_STR("Received RESUME(" << msg.cntlrName << ")");
		setState(CTState::RESUME);
		sendControlResult(port, CONTROL_RESUMED, msg.cntlrName, CT_RESULT_NO_ERROR);
		setState(CTState::RESUMED);
		break;
	}

	case CONTROL_SUSPEND: {		// nothing to do, just send answer
		CONTROLSuspendEvent		msg(event);
		LOG_DEBUG_STR("Received SUSPEND(" << msg.cntlrName << ")");
		setState(CTState::SUSPEND);
		sendControlResult(port, CONTROL_SUSPENDED, msg.cntlrName, CT_RESULT_NO_ERROR);
		setState(CTState::SUSPENDED);
		break;
	}

	case CONTROL_RELEASE: {
		CONTROLReleaseEvent		msg(event);
		LOG_DEBUG_STR("Received RELEASED(" << msg.cntlrName << ")");
		setState(CTState::RELEASE);
		if (!doRelease()) {
			LOG_WARN_STR("Cannot release a beam that was not allocated, continuing");
			setState(CTState::RELEASED);
			sendControlResult(*itsParentPort, CONTROL_RELEASED, getName(), 
															CT_RESULT_NO_ERROR);
			TRAN(TBBControl::claimed_state);
		}
		// else a BS_BEAMFREEACK event will be sent
		break;
	}

	case CONTROL_QUIT:
		TRAN(TBBControl::quiting_state);
		break;

	// -------------------- EVENTS RECEIVED FROM TBBDRIVER --------------------
// TODO TBB
	case BS_BEAMFREEACK: {
		if (!handleTBBFreeAck(event)) {
			LOG_WARN("Error in freeing beam, staying in active_state");
			sendControlResult(*itsParentPort, CONTROL_RELEASED, getName(), 
															CT_RESULT_BEAMFREE_FAILED);
		}
		else {
			LOG_INFO("Released beam going back to 'claimed' mode");
			setState(CTState::RELEASED);
			sendControlResult(*itsParentPort, CONTROL_RELEASED, getName(), 
															CT_RESULT_NO_ERROR);
			TRAN(TBBControl::claimed_state);
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
// quiting_state(event, port)
//
// Quiting: send QUITED, wait for answer max 5 seconds, stop
//
GCFEvent::TResult TBBControl::quiting_state(GCFEvent& event, GCFPortInterface& port)
{
	LOG_DEBUG_STR ("quiting:" << eventName(event) << "@" << port.getName());

	GCFEvent::TResult status = GCFEvent::HANDLED;

	switch (event.signal) {
	case F_ENTRY: {
		// update PVSS
		setState(CTState::QUIT);
		// tell Parent task we like to go down.
		itsParentControl->nowInState(getName(), CTState::QUIT);

//		itsPropertySet->setValue(string(PVSSNAME_FSM_CURACT),GCFPVString("quiting"));
		itsPropertySet->setValue(PVSSNAME_FSM_ERROR, GCFPVString(""));
		// disconnect from TBBDriver
		itsTBBDriver->close();
		break;
	}

	case F_INIT:
	case F_EXIT:
		break;

	case F_DISCONNECTED:		// propably from beamserver
		port.close();
		// fall through!!! 
	case F_CLOSED: {
		ASSERTSTR (&port == itsTBBDriver, 
								"F_DISCONNECTED event from port " << port.getName());
		LOG_INFO("Connection with TBBDriver down, sending QUITED to parent");
		CONTROLQuitedEvent		request;
		request.cntlrName = getName();
		request.result	  = CT_RESULT_NO_ERROR;
		itsParentPort->send(request);
		itsTimerPort->setTimer(1.0);		// wait 1 second to let message go away
		break;
	}
	
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
// doPrepare()
//
bool TBBControl::doPrepare()
{
// TODO TBB
	Observation		theObs(globalParameterSet());	// does all nasty conversions

	if (theObs.subbands.size() != theObs.beamlets.size()) {
		LOG_FATAL_STR("size of subbandList (" << theObs.subbands.size() << ") != " <<
						"size of beamletList (" << theObs.beamlets.size() << ")");
		return (false);
	}

	LOG_TRACE_VAR_STR("nr Subbands:" << theObs.subbands.size());
	LOG_TRACE_VAR_STR("nr TBBlets:" << theObs.beamlets.size());

	// contruct allocation event.
	BSTBBallocEvent beamAllocEvent;
	beamAllocEvent.name 		= getName();
	beamAllocEvent.subarrayname = formatString("observation[%d]{%d}", getInstanceNr(getName()),
																	getObservationNr(getName()));
	LOG_DEBUG_STR("subarrayName:" << beamAllocEvent.subarrayname);

	vector<int16>::iterator beamletIt = theObs.beamlets.begin();
	vector<int16>::iterator subbandIt = theObs.subbands.begin();
	while (beamletIt != theObs.beamlets.end() && subbandIt != theObs.subbands.end()) {
		LOG_TRACE_VAR_STR("alloc[" << *beamletIt << "]=" << *subbandIt);
		beamAllocEvent.allocation()[*beamletIt++] = *subbandIt++;
	}

	LOG_DEBUG_STR("Sending Alloc event to TBBDriver");
	itsTBBDriver->send(beamAllocEvent);		// will result in BS_BEAMALLOCACK;

	// store values in PVSS for operator
	itsPropertySet->setValue(PN_BC_SUBBANDLIST,	
				GCFPVString(APLUtilities::compactedArrayString(globalParameterSet()->
				getString("Observation.subbandList"))));
	itsPropertySet->setValue(PN_BC_BEAMLETLIST,	
				GCFPVString(APLUtilities::compactedArrayString(globalParameterSet()->
				getString("Observation.beamletList"))));
	itsPropertySet->setValue(PN_BC_ANGLE1,		
				GCFPVString(globalParameterSet()->getString("Observation.TBB.angle1")));
	itsPropertySet->setValue(PN_BC_ANGLE2,		
				GCFPVString(globalParameterSet()->getString("Observation.TBB.angle2")));
	itsPropertySet->setValue(PN_BC_ANGLETIMES,	
				GCFPVString(globalParameterSet()->getString("Observation.TBB.angleTimes")));
	itsPropertySet->setValue(PN_BC_SUBARRAY, GCFPVString(beamAllocEvent.subarrayname));

	return (true);
}

//
// handleTBBAllocAck(event);
//
uint16	TBBControl::handleTBBAllocAck(GCFEvent&	event)
{
// TODO TBB
	// check the beam ID and status of the ACK message
	BSTBBallocackEvent ackEvent(event);
	if (ackEvent.status != 0) {
		LOG_ERROR_STR("TBBlet allocation failed with errorcode: " << ackEvent.status);
		return (CT_RESULT_BEAMALLOC_FAILED);
	}
	itsTBBID = ackEvent.handle;
	itsPropertySet->setValue(PN_BC_BEAMID, GCFPVInteger(itsTBBID));

	// read new angles from parameterfile.
	vector<string>	sourceTimes;
	vector<double>	angles1;
	vector<double>	angles2;
	string			beam(globalParameterSet()->locateModule("TBB")+"TBB.");
	sourceTimes     = globalParameterSet()->getStringVector(beam+"angleTimes");
	angles1    = globalParameterSet()->getDoubleVector(beam+"angle1");
	angles2 = globalParameterSet()->getDoubleVector(beam+"angle2");

	// point the new beam
	BSTBBpointtoEvent beamPointToEvent;
	beamPointToEvent.handle = itsTBBID;
	beamPointToEvent.pointing.setType(static_cast<Pointing::Type>
				(convertDirection(globalParameterSet()->getString(beam+"directionTypes"))));

	// only 1 angle?
	if (sourceTimes.size() == 0 || sourceTimes.size() != angles1.size() || 
								  sourceTimes.size() != angles2.size()) {
		// key sourceTimes not found: use one fixed angle
		double	directionAngle1(0.0);
		double	directionAngle2(0.0);
		directionAngle1=globalParameterSet()->getDouble(beam+"angle1");
		directionAngle2=globalParameterSet()->getDouble(beam+"angle2");

		beamPointToEvent.pointing.setTime(RTC::Timestamp()); // asap
		beamPointToEvent.pointing.setDirection(directionAngle1,directionAngle2);
		itsTBBDriver->send(beamPointToEvent);
		// NB: will NOT result in an answer event of the beamserver.
		return (CT_RESULT_NO_ERROR);
	}

 	// its a vecor with angles.
	vector<double>::iterator angle1Iter    = angles1.begin();
	vector<double>::iterator angle2Iter = angles2.begin();
	for (vector<string>::iterator timesIt = sourceTimes.begin(); 
								  timesIt != sourceTimes.end(); ++timesIt) { 
		beamPointToEvent.pointing.setTime(RTC::Timestamp(
										APLUtilities::decodeTimeString(*timesIt),0));
		beamPointToEvent.pointing.setDirection(*angle1Iter++,*angle2Iter++);
		itsTBBDriver->send(beamPointToEvent);
		// NB: will NOT result in an answer event of the beamserver.
	}
	return (CT_RESULT_NO_ERROR);
}

//
// doRelease(event)
//
bool TBBControl::doRelease()
{
// TODO TBB
	if (!itsTBBID) {
		return (false);
	}

	BSTBBfreeEvent		beamFreeEvent;
	beamFreeEvent.handle = itsTBBID;
	itsTBBDriver->send(beamFreeEvent);	// will result in BS_BEAMFREEACK event
	return (true);
}


//
// handleTBBFreeAck(event)
//
bool TBBControl::handleTBBFreeAck(GCFEvent&		event)
{
// TODO TBB
	BSTBBfreeackEvent	ack(event);
	if (ack.status != 0 || ack.handle != itsTBBID) {
		LOG_ERROR_STR("TBBlet de-allocation failed with errorcode: " << ack.status);
		itsPropertySet->setValue(PVSSNAME_FSM_ERROR,GCFPVString("De-allocation of the beamlet failed."));
		return (false);	
	}

	// clear beam in PVSS
	itsPropertySet->setValue(PN_BC_SUBBANDLIST,	GCFPVString(""));
	itsPropertySet->setValue(PN_BC_BEAMLETLIST,	GCFPVString(""));
	itsPropertySet->setValue(PN_BC_ANGLE1,		GCFPVString(""));
	itsPropertySet->setValue(PN_BC_ANGLE2,		GCFPVString(""));
	itsPropertySet->setValue(PN_BC_ANGLETIMES,	GCFPVString(""));
	itsPropertySet->setValue(PN_BC_BEAMID,		GCFPVInteger(0));

	itsTBBID = 0;

	return (true);
}

// _defaultEventHandler(event, port)
//
GCFEvent::TResult TBBControl::_defaultEventHandler(GCFEvent&			event, 
													GCFPortInterface&	port)
{
	CTState		cts;
	LOG_DEBUG_STR("Received " << eventName(event) << " in state " << cts.name(itsState)
				  << ". DEFAULT handling.");

	GCFEvent::TResult	result(GCFEvent::NOT_HANDLED);

	switch (event.signal) {
		case CONTROL_CONNECT:
		case CONTROL_RESYNC:
		case CONTROL_SCHEDULE:	// TODO: we should do something with this
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

		case DP_CHANGED: {
			LOG_DEBUG_STR("DP " << DPChangedEvent(event).DPname << " was changed"); 
			result = GCFEvent::HANDLED;
		}
		break;
	}

	if (result == GCFEvent::NOT_HANDLED) {
		LOG_WARN_STR("Event " << eventName(event) << " NOT handled in state " << 
					 cts.name(itsState));
	}

	return (result);
}

// _connectedHandler(port)
//
void TBBControl::_connectedHandler(GCFPortInterface& /*port*/)
{
}


//
// _disconnectedHandler(port)
//
void TBBControl::_disconnectedHandler(GCFPortInterface& port)
{
	port.close();
}


}; // StationCU
}; // LOFAR
