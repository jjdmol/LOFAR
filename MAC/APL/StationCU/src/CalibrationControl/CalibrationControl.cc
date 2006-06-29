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

#include "CalibrationControl.h"
#include "CalibrationControlDefines.h"

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

	// Readin some parameters from the ParameterSet.
	itsTreePrefix = globalParameterSet()->getString("prefix");
	itsInstanceNr = globalParameterSet()->getUint32(itsTreePrefix + ".instanceNr");
	itsStartTime  = globalParameterSet()->getTime  (itsTreePrefix + ".starttime");
	itsStopTime   = globalParameterSet()->getTime  (itsTreePrefix + ".stoptime");

	// CalibrationControl specific parameters
	// TODO: these values are delivered for every customer!
	itsNyquistZone   = globalParameterSet()->getInt16("nyquistZone");
	itsBandSelection = globalParameterSet()->getString("bandSelection");
	itsAntennaArray  = globalParameterSet()->getString("antennaArray");
	itsRCUvector	 = globalParameterSet()->getUint16Vector("rcus");

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

	// prepare TCP port to CalibrationServer.
	itsCalServer = new GCFTCPPort (*this, MAC_SVCMASK_CALSERVER,
											GCFPortInterface::SAP, CAL_PROTOCOL);
	ASSERTSTR(itsCalServer, "Cannot allocate TCPport to CalServer");

	// for debugging purposes
	registerProtocol (CONTROLLER_PROTOCOL, CONTROLLER_PROTOCOL_signalnames);
	registerProtocol (PA_PROTOCOL, 		   PA_PROTOCOL_signalnames);
	registerProtocol (CAL_PROTOCOL,		   CAL_PROTOCOL_signalnames);

	setState(CTState::CREATED);
}


//
// ~CalibrationControl()
//
CalibrationControl::~CalibrationControl()
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
int32 CalibrationControl::convertDirection(const string&	typeName)
{
	if (typeName == "J2000") 	{ return (1); }
	if (typeName == "AZEL") 	{ return (2); }
	if (typeName == "LMN")	 	{ return (3); }
	return (2);
}

//
// convertBandSelection(string) : uint8
//
uint8 CalibrationControl::convertBandSelection(const string&	bandselection) 
{
	if (bandselection == "LB_10_90") {
		return(0xB9);
	}
	if (bandselection == "HB_110_190") {
		return(0xC6);
	}
	if (bandselection == "HB_170_230") {
		return(0xCE);
	}
	if (bandselection == "HB_210_250") {
		return(0xD6);
	}

	LOG_WARN_STR ("bandselection value '" << bandselection << 
											"' not recognized, using LB_10_90");
	return (0xB9);
}

//
// propertySetsAvailable()
//
bool CalibrationControl::propertySetsAvailable()
{
	return (m_rcuFunctionalityMap.size() == m_n_rcus);
}

//
// getRCUhardwareNr (propertyname)
//
int32 CalibrationControl::getRCUhardwareNr(const string&	propName)
{
	// strip property and systemname, propertyset name remains
	int    posBegin 		= propName.find_first_of(":") + 1;
	int    posEnd   		= propName.find_last_of(".");
	string propertySetName	= propName.substr(posBegin,posEnd-posBegin);

	// search in property map.
	TRCUMap::iterator it  = m_rcuMap.begin();
	TRCUMap::iterator end = m_rcuMap.end();
	while (it != end) {
		if (propertySetName == it->second->getScope()) {
			return (it->first);
		}
		++it;
	}

	return (-1);
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
		itsTimerPort->setTimer(0.0);
		break;
	}

	case F_VGETRESP: {
		// check which property changed
		GCFPropValueEvent* pPropAnswer=static_cast<GCFPropValueEvent*>(&answer);

		// check if it is a functionality
		if(strstr(pPropAnswer->pPropName, PROPNAME_FUNCTIONALITY) != 0) {
			// add the functionality state to the internal cache
			int rcu = getRCUHardwareNr(pPropAnswer->pPropName);
			if (rcu >= 0) {
				bool functional = ((GCFPVBool*)pPropAnswer->pValue)->getValue();
				m_rcuFunctionalityMap[static_cast<uint16>(rcu)] = functional;
			}
		}
		break;
	}  

	case F_VCHANGEMSG: {
		GCFPropValueEvent* pPropAnswer=static_cast<GCFPropValueEvent*>(&answer);

		// check if it is a functionality
		if (strstr(pPropAnswer->pPropName, PROPNAME_FUNCTIONALITY) != 0) {
			LOG_DEBUG("functionality property changed");
			// add the functionality to the internal cache
			int rcu = getRCUhardwareNr(pPropAnswer->pPropName);
			if (rcu >= 0) {
				bool functional = ((GCFPVBool*)pPropAnswer->pValue)->getValue();
				m_rcuFunctionalityMap[static_cast<uint16>(rcu)] = functional;
				LOG_DEBUG(formatString("RCU %d functionality: %s",
										rcu, (functional ? "true" : "false")));
			}
			// TODO
			if (getLogicalDeviceState() == LOGICALDEVICE_STATE_ACTIVE) {
				// check functionality state of RCU's
				if (!checkQuality()) {
					suspend(LD_RESULT_LOW_QUALITY);
				}
			}
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
		LOG_DEBUG ("Activating my PropertySet");
		string	propSetName = formatString(CS_PROPSET_NAME, itsInstanceNr);
		itsPropertySet = GCFMyPropertySetPtr(new GCFMyPropertySet(propSetName.c_str(),
																  CS_PROPSET_TYPE,
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

			// Open connection with CalServer
			LOG_DEBUG ("Trying to connect to CalibrationServer");
			itsCalServer->open();		// will result in F_CONN or F_DISCONN
		}
		else {
			// its the disconnected timer
			itsCalServer->open();		// will result in F_CONN or F_DISCONN
		}
		break;

	case F_CONNECTED:
		ASSERTSTR (&port == itsCalServer, 
									"F_CONNECTED event from port " << port.getName());
		itsTimerPort->cancelAllTimers();
		LOG_DEBUG ("Connected with CalServer, going to operational state");
		TRAN(CalibrationControl::active_state);				// go to next state.
		break;

	case F_DISCONNECTED:
		port.close();
		ASSERTSTR (&port == itsCalServer, 
								"F_DISCONNECTED event from port " << port.getName());
		LOG_DEBUG("Connection with CalServer failed, retry in 2 seconds");
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
GCFEvent::TResult CalibrationControl::active_state(GCFEvent& event, GCFPortInterface& port)
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

		loadPVSSpropertySets();							// of all RCU boards
														// this taes a while so
		itsPropAvailTimer = itsTimerPort.setTimer(3.0);	// check over 3 seconds
		break;
	}

	case F_ACCEPT_REQ:
		break;

	case F_CONNECTED:
		ASSERTSTR (&port == itsCalServer, 
									"F_CONNECTED event from port " << port.getNam());
		itsTimerPort->cancelAllTimers();
		LOG_DEBUG ("Reconnected with CalServer");
		// TODO: resend all claimes.
		break;

	case F_DISCONNECTED:
		port.close();
		ASSERTSTR (&port == itsCalServer, 
								"F_DISCONNECTED event from port " << port.getNam());
		LOG_DEBUG("Connection with CalServer failed, retry in 2 seconds");
		itsTimerPort.setTimer(2.0);
		break;

	case F_TIMER:  {
		GCFTimerEvent& timerEvent=static_cast<GCFTimerEvent&>(event);
		if (timerEvent.id == itsPropSetAvailTimer) {
			if (propertySetsAvailable()) {
				LOG_DEBUG("PVSS propertySet are active.");
				itsPropAvailTimer = 0;
			}
			else {
				itsPropAvailTimer = itsTimerPort.setTimer(1.0);	// check again later
			}
		}
		else {
			LOG_DEBUG ("Trying to reconnect to CalServer");
			itsCalServer->open();		// will result in F_CONN or F_DISCONN
		}
		break;
	}

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
		CONTROLClaimedEvent		answer;
		answer.cntlrName = msg.cntlrName;
		LOG_DEBUG_STR("Received CLAIM(" << msg.cntlrName << ")");
		setState(CTState::CLAIM);
		if (claimResources()) {
			LOG_DEBUG_STR (msg.cntlrName << ":resources claimed successful");
			setState(CTState::CLAIMED);
			answer.result = CT_RESULT_SUCCESS;
		}
		else {
			LOG_ERROR_STR (msg.cntlrName << 
							":failed to claim resources, staying in idle mode");
			setSTate(CTState::CONNECTED);
			answer.result = CT_RESULT_CLAIM_FAILED;
		}
		itsParentPort.send(answer);
		break;
	}

	case CONTROL_PREPARE: {
		CONTROLPrepareEvent		msg(event);
		LOG_DEBUG_STR("Received PREPARE(" << msg.cntlrName << ")");
		setState(CTState::PREPARE);
		startCalServer();			// will result in CAL_STARTACK event
		break;
	}

	case CONTROL_RESUME: {
		CONTROLResumeEvent		msg(event);
		LOG_DEBUG_STR("Received RESUME(" << msg.cntlrName << ")");
		setState(CTState::ACTIVE);
		// TODO: do something here?
		CONTROLResumedEvent		answer;
		answer.cntlrName = msg.cntlrName;
		port.send(answer);
		break;
	}

	case CONTROL_SUSPEND: {
		CONTROLSuspendEvent		msg(event);
		LOG_DEBUG_STR("Received SUSPEND(" << msg.cntlrName << ")");
		setState(CTState::SUSPENDED);
		// TODO: do something here?
		CONTROLSuspendedEvent		answer;
		answer.cntlrName = msg.cntlrName;
		port.send(answer);
		break;
	}

	case CONTROL_RELEASE: {
		CONTROLReleaseEvent		msg(event);
		LOG_DEBUG_STR("Received RELEASED(" << msg.cntlrName << ")");
		setState(CTState::RELEASE);
		stopCalServer(event);		// will result in CAL_STOPACK event
		break;
	}

	// -------------------- EVENTS RECEIVED FROM CALSERVER --------------------

	case CAL_STARTACK:
		CALStartEvent			ack(event);
		CONTROLPreparedEvent	answer;
		answer.cntlrName = ack.cntlrName;
		if (ack.status == SUCCESS) {
			LOG_DEBUG ("Start of CalServer was succesful");
			setState(CTState::PREPARED);
			answer.result = CT_RESULT_SUCCESS;
		}
		else {
			LOG_ERROR("Start of calibration failed, staying in CLAIMED mode");
			setState(CTState::CLAIMED);
			answer.result = CT_RESULT_START_FAILED;
		}
		itsParentPort->send();
		break;

	case CAL_STOPACK: {
		CALStopackEvent			ack(event);
		CONTROLReleasedEvent	answer;
		answer.cntlrName = msg.cntlrName;
		if (ack.status == SUCCESS) {
			LOG_DEBUG ("Calserver successfully stopped");
			setState(CTState::RELEASED);
			answer.result = CT_RESULT_SUCCESS;
		}
		else {
			LOG_ERROR("Stop of calibration failed, going to SUSPENDED mode");
			setState(CT::State::SUSPENDED);
			answer.result = CT_RESULT_CALSTOP_FAILED;
		}
		itsParentPort->send(answer);
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
// claimResources()
//
bool CalibrationControl::claimResources()
{
	// claim the resources:
	ResourceAllocator::TRcuSubset 	rcuSubset;
	for (TRCUMap::iterator it = m_rcuMap.begin(); it != m_rcuMap.end(); ++it) {
		rcuSubset.set(it->first);
	}
	uint8 rcuControl  = getRcuControlValue(itsBandSelection);

	if(!_getResourceAllocator()->claimSRG(shared_from_this(),
										  _getPriority(),
										  rcuSubset,
										  nyquistZone,
										  rcuControl)) {
		errorCode = LD_RESULT_LOW_PRIORITY;
		return (false);
	}

	_getResourceAllocator()->logSRGallocation();
	return (true);
}

//
// loadPVSSpropertySets
//
void	CalibrationControl::loadPVSSpropertySets()
{
	// load propertysets for the rcus
	try {
		for(vector<uint16>::iterator it = rcuVector.begin(); 
													it != rcuVector.end(); ++it) {
			char scopeString[300];
			int rackRelNr,subRackRelNr,boardRelNr,apRelNr,rcuRelNr;
			getRCURelNumbers((*it),rackRelNr,subRackRelNr,boardRelNr,apRelNr,rcuRelNr);
			sprintf(scopeString,SCOPE_PIC_RackN_SubRackN_BoardN_APN_RCUN_LFA,rackRelNr,subRackRelNr,boardRelNr,apRelNr,rcuRelNr);

			boost::shared_ptr<GCFExtPropertySet> propSetPtr(new GCFExtPropertySet(scopeString,TYPE_LCU_PIC_LFA,&m_propertySetAnswer));
			propSetPtr->load();
			propSetPtr->subscribeProp(string(PROPNAME_FUNCTIONALITY));
			propSetPtr->requestValue(string(PROPNAME_FUNCTIONALITY));
			m_rcuMap[(*it)]=propSetPtr;
		}
	}
	catch(Exception &e) {
		LOG_FATAL(formatString("Error claiming SRG: %s",e.message().c_str()));
	}
}

//
// startCalServer
//
void	CalibrationControl::startCalServer() 
{
	// send CALStartEvent
	LOG_DEBUG ("Sending CALSTART to CAL server");
	CALStartEvent calStartEvent;
	calStartEvent.name   		   = getName();
	calStartEvent.parent 		   = itsAntennaArray;
	calStartEvent.nyquist_zone     = itsNyquistZone;
	calStartEvent.rcucontrol.value = convertBandSelection(itsBandSelection);

	calStartEvent.subset.reset(); // reset every bit
	for (TRCUMap::iterator it = m_rcuMap.begin(); it != m_rcuMap.end(); ++it) {
		calStartEvent.subset.set(it->first);
	}

	itsCalServer->send(calStartEvent);
}


//
// stopCalServer()
//
void	CalibrationControl::stopCalServer()
{
	// send CALStopEvent
	LOG_DEBUG ("Sending CALSTOP to CAL server");
	CALStopEvent calStopEvent;
	calStopEvent.name = getName();
	itsCalServer->send(calStopEvent);

	itsResourceAllocator->releaseSRG(shared_from_this()); // TODO
	itsResourceAllocator->logSRGallocation();
}

//
// _connectedHandler(port)
//
void CalibrationControl::_connectedHandler(GCFPortInterface& port)
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
