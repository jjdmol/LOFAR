//#  BeamControl.cc: Implementation of the MAC Scheduler task
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
#include <APL/BS_Protocol/BS_Protocol.ph>

#include "BeamControl.h"
#include "BeamControlDefines.h"

using namespace LOFAR::GCF::Common;
using namespace LOFAR::GCF::TM;
using namespace LOFAR::GCF::PAL;
using namespace LOFAR::OTDB;
using namespace std;

namespace LOFAR {
	using namespace APLCommon;
	using namespace ACC::APS;
	namespace StationCU {
	
//
// BeamControl()
//
BeamControl::BeamControl(const string&	cntlrName) :
	GCFTask 			((State)&BeamControl::initial_state,cntlrName),
	PropertySetAnswerHandlerInterface(),
	itsPropertySetAnswer(*this),
	itsPropertySet		(),
	itsPropertySetInitialized (false),
	itsParentControl	(0),
	itsParentPort		(0),
	itsTimerPort		(0),
	itsBeamServer		(0),
	itsState			(CTState::NOSTATE)
{
	LOG_TRACE_OBJ_STR (cntlrName << " construction");

	// Readin some parameters from the ParameterSet.
	itsTreePrefix = globalParameterSet()->getString("prefix");
	itsInstanceNr = globalParameterSet()->getUint32(itsTreePrefix + ".instanceNr");
	itsStartTime  = globalParameterSet()->getTime  (itsTreePrefix + ".starttime");
	itsStopTime   = globalParameterSet()->getTime  (itsTreePrefix + ".stoptime");

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

	// prepare TCP port to BeamServer.
	itsBeamServer = new GCFTCPPort (*this, MAC_SVCMASK_BEAMSERVER,
											GCFPortInterface::SAP, BS_PROTOCOL);
	ASSERTSTR(itsBeamServer, "Cannot allocate TCPport to BeamServer");

	// for debugging purposes
	registerProtocol (CONTROLLER_PROTOCOL, CONTROLLER_PROTOCOL_signalnames);
	registerProtocol (PA_PROTOCOL, 		   PA_PROTOCOL_signalnames);
	registerProtocol (BS_PROTOCOL, 		   BS_PROTOCOL_signalnames);

	setState(CTState::CREATED);
}


//
// ~BeamControl()
//
BeamControl::~BeamControl()
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
void    ObservationControl::setState(CTState::CTstateNr     newState)
{
	itsState = newState;

	if (itsPropertySet) {
		CTState		cts;
		itsPropertySet->setValue(string(PVSSNAME_FSM_STATE),
								 GCFPVString(cts.name(newState)));
	}
}   


//
// convertDirection(string) : int32
//
int32 BeamControl::convertDirection(const string&	typeName)
{
	if (typeName == "J2000") 	{ return (1); }
	if (typeName == "AZEL") 	{ return (2); }
	if (typeName == "LMN")	 	{ return (3); }
	return (2);
}

//
// handlePropertySetAnswer(answer)
//
void BeamControl::handlePropertySetAnswer(GCFEvent& answer)
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
		// GCFPropValueEvent* pPropAnswer=static_cast<GCFPropValueEvent*>(&answer);

		// TODO: implement something usefull.
		}
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
GCFEvent::TResult BeamControl::initial_state(GCFEvent& event, 
													GCFPortInterface& port)
{
	LOG_DEBUG_STR ("initial:" << evtstr(event) << "@" << port.getName());

	GCFEvent::TResult status = GCFEvent::HANDLED;
  
	switch (event.signal) {
    case F_INIT:
   		break;

	case F_ENTRY: {
		// Get access to my own propertyset.
		LOG_DEBUG ("Activating PropertySet");
		string	propSetName = formatString(BS_PROPSET_NAME, itsInstanceNr);
		itsPropertySet = GCFMyPropertySetPtr(new GCFMyPropertySet(propSetName.c_str(),
																  BS_PROPSET_TYPE,
																  PS_CAT_TEMPORARY,
																  &itsPropertySetAnswer));
		itsPropertySet->enable();
		// Wait for timer that is set in PropertySetAnswer on ENABLED event
		}
		break;

	case F_TIMER:
		if (!itsPropertySetInitialized) {
			itsPropertySetInitialized = true;

			// update PVSS.
			LOG_TRACE_FLOW ("Updateing state to PVSS");
			itsPropertySet->setValue(string(PVSSNAME_FSM_STATE),GCFPVString("initial"));
			itsPropertySet->setValue(string(PVSSNAME_FSM_ERROR),GCFPVString(""));
		  
			// Start ParentControl task
			LOG_DEBUG ("Enabling ParentControl task");
			itsParentPort = itsParentControl->registerTask(this);

			LOG_DEBUG ("Going to operational state");
			TRAN(BeamControl::active_state);				// go to next state.
		}
		break;

	case F_CONNECTED:
		ASSERTSTR (&port == itsBeamServer, 
									"F_CONNECTED event from port " << port.getNam());
		break;

	case F_DISCONNECTED:
		port.close();
		ASSERTSTR (&port == itsBeamServer, 
								"F_DISCONNECTED event from port " << port.getNam());
		LOG_DEBUG("Connection with BeamServer failed, retry in 2 seconds");
		itsTimerPort.setTimer(2.0);
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
GCFEvent::TResult BeamControl::active_state(GCFEvent& event, GCFPortInterface& port)
{
	LOG_DEBUG_STR ("active:" << evtstr(event) << "@" << port.getName());

	GCFEvent::TResult status = GCFEvent::HANDLED;

	switch (event.signal) {
	case F_INIT:
		break;

	case F_ENTRY: {
		// update PVSS
		itsPropertySet->setValue(string(PVSSNAME_FSM_STATE),GCFPVString("active"));
		itsPropertySet->setValue(string(PVSSNAME_FSM_ERROR),GCFPVString(""));
		break;
	}

	case F_ACCEPT_REQ:
		break;

	case F_CONNECTED:
		ASSERTSTR (&port == itsBeamServer, 
									"F_CONNECTED event from port " << port.getNam());
		itsTimerPort->cancelAllTimers();
		LOG_DEBUG ("Connected with BeamServer");
		setState(CTState::CLAIMED);
		CONTROLClaimedEvent		answer;
		answer.cntlrName = msg.cntlrName;
		port.send(answer);
		break;

	case F_DISCONNECTED:
		port.close();
		ASSERTSTR (&port == itsBeamServer, 
								"F_DISCONNECTED event from port " << port.getNam());
		LOG_DEBUG("Connection with BeamServer failed, retry in 2 seconds");
		itsTimerPort.setTimer(2.0);
		break;

	case F_TIMER: 
//		GCFTimerEvent& timerEvent=static_cast<GCFTimerEvent&>(event);
		LOG_DEBUG ("Trying to reconnect to BeamServer");
		itsBeamServer->open();		// will result in F_CONN or F_DISCONN
		break;

	// -------------------- EVENTS RECEIVED FROM PARENT CONTROL --------------------
	case CONTROL_CONNECT: {
		CONTROLConnectEvent		msg(event);
		LOG_DEBUG_STR("Received CONNECT(" << msg.cntlrName << ")");
		setState(CTState::CONNECTED);
		CONTROLConnectedEvent	answer;
		answer.cntlrName = msg.cntlrName;
		port.send(answer);
		break;
	}

	case CONTROL_SCHEDULED: {
		CONTROLScheduledEvent		msg(event);
		LOG_DEBUG_STR("Received SCHEDULED(" << msg.cntlrName << ")");
		// TODO: do something usefull with this information!
		break;
	}

	case CONTROL_CLAIM: {
		CONTROLClaimEvent		msg(event);
		LOG_DEBUG_STR("Received CLAIM(" << msg.cntlrName << ")");
		setState(CTState::CLAIM);
		LOG_DEBUG ("Trying to connect to BeamServer");
		itsBeamServer->open();		// will result in F_CONN or F_DISCONN
		break;
	}

	case CONTROL_PREPARE: {
		CONTROLPrepareEvent		msg(event);
		LOG_DEBUG_STR("Received PREPARE(" << msg.cntlrName << ")");
		setState(CTState::PREPARE);
		setState(doPrepare (msg.cntlrName) ? CTState::PREPARED : CTState::CLAIMED);
		CONTROLPreparedEvent	answer;
		answer.cntlrName = msg.cntlrName;
		port.send();
		break;
	}

	case CONTROL_RESUME: {
		CONTROLResumeEvent		msg(event);
		LOG_DEBUG_STR("Received RESUME(" << msg.cntlrName << ")");
		setState(CTState::ACTIVE);
		CONTROLResumedEvent		answer;
		answer.cntlrName = msg.cntlrName;
		port.send(answer);
		break;
	}

	case CONTROL_SUSPEND: {
		CONTROLSuspendEvent		msg(event);
		LOG_DEBUG_STR("Received SUSPEND(" << msg.cntlrName << ")");
		setState(CTState::SUSPENDED);
		CONTROLSuspendedEvent		answer;
		answer.cntlrName = msg.cntlrName;
		port.send(answer);
		break;
	}

	case CONTROL_RELEASE: {
		CONTROLReleaseEvent		msg(event);
		LOG_DEBUG_STR("Received RELEASED(" << msg.cntlrName << ")");
		setState(CTState::RELEASE);
		doRelease(event);
		setState(CTState::RELEASED);
		CONTROLReleasedEvent	answer;
		answer.cntlrName = msg.cntlrName;
		port.send(answer);
		break;
	}

	// -------------------- EVENTS RECEIVED FROM BEAMSERVER --------------------
	case BS_BEAMALLOCACK:
		handleBeamAllocAck(event);
		break;

	case BS_BEAMFREEACK:
		handleBeamFreeAck(event);
		break;

	default:
		LOG_DEBUG("active_state, default");
		status = GCFEvent::NOT_HANDLED;
		break;
	}

	return (status);
}


//
// doPrepare(cntlrName)
//
void BeamControl::doPrepare(const string&	cntlrName)
{
	try {
		vector<int> subbandsVector;
		vector<int> beamletsVector;
		// TODO
		APLUtilities::string2Vector(m_parameterSet.getString(string("subbands")),subbandsVector);
		APLUtilities::string2Vector(m_parameterSet.getString(string("beamlets")),beamletsVector);

		BSBeamallocEvent beamAllocEvent;
		beamAllocEvent.name 		= getName();
		beamAllocEvent.subarrayname = m_parameterSet.getString(string("subarrayName"));
		vector<int>::iterator beamletIt = beamletsVector.begin();
		vector<int>::iterator subbandIt = subbandsVector.begin();
		while (beamletIt != beamletsVector.end() && subbandIt != subbandsVector.end()) {
			beamAllocEvent.allocation()[*beamletIt++] = *subbandIt++;
		}
		itsBeamServer->send(beamAllocEvent);		// will result in BS_BEAMALLOCACK;
	}
	catch(Exception& e) {
		LOG_ERROR(formatString("Error preparing BeamServer: %s",e.message().c_str()));
	}
}

//
// handleBeamAllocAck(event);
//
bool	BeamControl::handleBeamAllocAck(GCFEvent&	event)
{
	// check the beam ID and status of the ACK message
	BSBeamallocackEvent ackEvent(event);
	if (ackEvent.status != 0) {
		errorCode = CT_RESULT_BEAMALLOC_ERROR;
		LOG_ERROR_STR("Beamlet allocation failed with errorcode: ",ackEvent.status);
		return (false);
	}

	// read new angles from parameterfile.
	m_beamID=ackEvent.handle;

	double directionAngle1(0.0);
	double directionAngle2(0.0);
	vector<string> angleTimes;
	vector<double> angles1;
	vector<double> angles2;

	try {
		angleTimes = m_parameterSet.getStringVector(string("angleTimes"));
		angles1 = m_parameterSet.getDoubleVector(string("angle1"));
		angles2 = m_parameterSet.getDoubleVector(string("angle2"));
	}
	catch(Exception &e) {
	}

	// point the new beam
	BSBeampointtoEvent beamPointToEvent;
	beamPointToEvent.handle = m_beamID;
	beamPointToEvent.pointing.setType(static_cast<Pointing::Type>
				(convertDirection(m_parameterSet.getString(string("directionType")))));

	if (angleTimes.size() == 0 || angleTimes.size() != angles1.size() || 
								  angleTimes.size() != angles2.size()) {
		// key angleTimes not found: use one fixed angle
		directionAngle1=m_parameterSet.getDouble(string("angle1"));
		directionAngle2=m_parameterSet.getDouble(string("angle2"));

		beamPointToEvent.pointing.setTime(RTC::Timestamp()); // asap
		beamPointToEvent.pointing.setDirection(directionAngle1,directionAngle2);
		m_beamServer.send(beamPointToEvent);
	}
	else { 	// its a vecor with angles.
		vector<double>::iterator angle1It = angles1.begin();
		vector<double>::iterator angle2It = angles2.begin();
		for (vector<string>::iterator timesIt = angleTimes.begin(); 
									  timesIt != angleTimes.end(); ++timesIt) { 
			beamPointToEvent.pointing.setTime(RTC::Timestamp(
											APLUtilities::decodeTimeString(*timesIt),0));
			beamPointToEvent.pointing.setDirection(*angle1It++,*angle2It++);
			m_beamServer.send(beamPointToEvent);
		}
	}

	return (true);
}

//
// doRelease(event)
//
void BeamControl::doRelease(GCFEvent&	event)
{
	BSBeamfreeEvent		beamFreeEvent;
	beamFreeEvent.handle = ...beamID ...
	itsBeamServer->send(beamFreeEvent);
}


//
// handleBeamFreeAck(event)
//
bool BeamControlBeamFreeAck(GCFEvent&		event)
{
	BSBeamfreeackEvent	ack(event);
	if (ack.status != 0 || ack.handle != ...beamID...) {
		errorCode = CT_RESULT_BEAMFREE_ERROR;
		LOG_ERROR_STR("Beamlet de-allocation failed with errorcode: ", ack.status);
	}
}

// _connectedHandler(port)
//
void BeamControl::_connectedHandler(GCFPortInterface& port)
{
}

//
// _disconnectedHandler(port)
//
void BeamControl::_disconnectedHandler(GCFPortInterface& port)
{
	port.close();
}


}; // StationCU
}; // LOFAR
