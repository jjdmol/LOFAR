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
#include <Common/LofarLocators.h>

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
#include <APL/APLCommon/ControllerDefines.h>
#include <APL/APLCommon/StationInfo.h>
#include <APL/CAL_Protocol/CAL_Protocol.ph>

#include "CalibrationControl.h"
#include "CalibrationControlDefines.h"

using namespace LOFAR::GCF::Common;
using namespace LOFAR::GCF::TM;
using namespace LOFAR::GCF::PAL;

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
//	itsState			(CTState::NOSTATE)
	itsObsMap			(),
	itsPropSetAvailTimer(0)
{
	LOG_TRACE_OBJ_STR (cntlrName << " construction");

	// Readin some parameters from the ParameterSet.
	itsTreePrefix = globalParameterSet()->getString("prefix");
	itsInstanceNr = globalParameterSet()->getUint32(itsTreePrefix + ".instanceNr");
	itsStartTime  = globalParameterSet()->getTime  (itsTreePrefix + ".starttime");
	itsStopTime   = globalParameterSet()->getTime  (itsTreePrefix + ".stoptime");

	LOG_INFO_STR("MACProcessScope: " << itsTreePrefix + cntlrName);

	// CalibrationControl specific parameters
	// TODO: these values are delivered for every customer!
//	itsNyquistZone   = globalParameterSet()->getInt16("nyquistZone");
//	itsBandFilter	 = globalParameterSet()->getString("bandFilter");
//	itsAntennaArray  = globalParameterSet()->getString("antennaArray");
//	itsRCUvector	 = globalParameterSet()->getUint16Vector("receiverList");

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

	if (itsPropertySet) {
		itsPropertySet->setValue(string(PVSSNAME_FSM_STATE),GCFPVString("down"));
	}

	// ...
}

//
// setState(CTstateNr)
//
void    CalibrationControl::setState(CTState::CTstateNr     newState)
{
//	itsState = newState;

	if (itsPropertySet) {
		CTState		cts;
		itsPropertySet->setValue(PVSSNAME_FSM_STATE, GCFPVString(cts.name(newState)));
	}
}   


//
// convertFilterSelection(string) : uint8
//
uint8 CalibrationControl::convertFilterSelection(const string&	filterselection) 
{
	if (filterselection == "LB_10_90") 		{ return(0xB9); }
	if (filterselection == "HB_110_190") 	{ return(0xC6); }
	if (filterselection == "HB_170_230") 	{ return(0xCE); }
	if (filterselection == "HB_210_250") 	{ return(0xD6); }

	LOG_WARN_STR ("filterselection value '" << filterselection << 
											"' not recognized, using LB_10_90");
	return (0xB9);
}

//
// propertySetsAvailable()
//
// REO: does CC watch every rcu propertyset????
//
bool CalibrationControl::propertySetsAvailable()
{
	return (true);

	// TODO: calculate m_n_rcus: racks * subracks ...
	// uint32	m_n_rcus = 192;
	// return (m_rcuFunctionalityMap.size() == m_n_rcus);
	
}

//
// getRCUHardwareNr (propertyname)
//
int32 CalibrationControl::getRCUHardwareNr(const string&	propName)
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
		itsTimerPort->setTimer(0.5);
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
			int rcu = getRCUHardwareNr(pPropAnswer->pPropName);
			if (rcu >= 0) {
				bool functional = ((GCFPVBool*)pPropAnswer->pValue)->getValue();
				m_rcuFunctionalityMap[static_cast<uint16>(rcu)] = functional;
				LOG_DEBUG(formatString("RCU %d functionality: %s",
										rcu, (functional ? "true" : "false")));
			}
			// TODO
//TODO			if (getLogicalDeviceState() == LOGICALDEVICE_STATE_ACTIVE) {
//TODO				// check functionality state of RCU's
//TODO				if (!checkQuality()) {
//TODO					suspend(LD_RESULT_LOW_QUALITY);
//TODO				}
//TODO			}
		}
		break;
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
		string	propSetName(createPropertySetName(PSN_CAL_CTRL, getName()));
		LOG_DEBUG_STR ("Activating PropertySet" << propSetName);
		itsPropertySet = GCFMyPropertySetPtr(new GCFMyPropertySet(propSetName.c_str(),
																  PST_CAL_CTRL,
																  PS_CAT_PERM_AUTOLOAD,
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
		if (&port == itsParentPort) {
			break;
		}

		// CONNECT must be from Calserver.
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
		itsTimerPort->setTimer(2.0);
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

//		loadPVSSpropertySets();							// of all RCU boards
														// this takes a while so
		itsPropSetAvailTimer = itsTimerPort->setTimer(3.0);	// check over 3 seconds
		break;
	}

	case F_ACCEPT_REQ:
		break;

	case F_CONNECTED:
		ASSERTSTR (&port == itsCalServer, 
									"F_CONNECTED event from port " << port.getName());
		itsTimerPort->cancelAllTimers();
		LOG_DEBUG ("Reconnected with CalServer");
		// TODO: resend all claimes.
		break;

	case F_DISCONNECTED:
		port.close();
		ASSERTSTR (&port == itsCalServer, 
								"F_DISCONNECTED event from port " << port.getName());
		LOG_DEBUG("Connection with CalServer failed, retry in 2 seconds");
		itsTimerPort->setTimer(2.0);
		break;

	case F_TIMER:  {
		GCFTimerEvent& timerEvent=static_cast<GCFTimerEvent&>(event);
		if (timerEvent.id == itsPropSetAvailTimer) {
			if (propertySetsAvailable()) {
				LOG_DEBUG("PVSS propertySet are active.");
				itsPropSetAvailTimer = 0;
			}
			else {
				itsPropSetAvailTimer = itsTimerPort->setTimer(1.0);	// check again later
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
		CONTROLConnectedEvent	answer;
		answer.cntlrName = msg.cntlrName;
		LOG_DEBUG_STR("Received CONNECT(" << msg.cntlrName << ")");
		// add observation to list if not already in list
		answer.result = addObservation(msg.cntlrName) ?
									CT_RESULT_NO_ERROR : CT_RESULT_UNSPECIFIED;
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
		answer.result = handleClaimEvent(msg.cntlrName);
		port.send(answer);
		break;
	}

	case CONTROL_PREPARE: {
		CONTROLPrepareEvent		msg(event);
		LOG_DEBUG_STR("Received PREPARE(" << msg.cntlrName << ")");
		if (!startCalibration(msg.cntlrName)) {	// will result in CAL_STARTACK event
			CONTROLPreparedEvent	answer;		// when command was sent.
			answer.cntlrName = msg.cntlrName;
			answer.result = CT_RESULT_CALSTART_FAILED;
			port.send(answer);
		}
		break;
	}

	case CONTROL_RESUME: {
		CONTROLResumeEvent		msg(event);
		LOG_DEBUG_STR("Received RESUME(" << msg.cntlrName << ")");
		// TODO: do something here?
		CONTROLResumedEvent		answer;
		answer.cntlrName = msg.cntlrName;
		answer.result = CT_RESULT_NO_ERROR;
		port.send(answer);
		break;
	}

	case CONTROL_SUSPEND: {
		CONTROLSuspendEvent		msg(event);
		LOG_DEBUG_STR("Received SUSPEND(" << msg.cntlrName << ")");
		// TODO: do something here?
		CONTROLSuspendedEvent		answer;
		answer.cntlrName = msg.cntlrName;
		answer.result = CT_RESULT_NO_ERROR;
		port.send(answer);
		break;
	}

	case CONTROL_RELEASE: {
		CONTROLReleaseEvent		msg(event);
		LOG_DEBUG_STR("Received RELEASED(" << msg.cntlrName << ")");
		if (!stopCalibration(msg.cntlrName)) {	// will result in CAL_STOPACK event
			CONTROLPreparedEvent	answer;		// when command was sent.
			answer.cntlrName = msg.cntlrName;
			answer.result = CT_RESULT_CALSTOP_FAILED;
			port.send(answer);
		}
		break;
	}

	// -------------------- EVENTS RECEIVED FROM CALSERVER --------------------

	case CAL_STARTACK: {
		CALStartackEvent		ack(event);
		CONTROLPreparedEvent	answer;
		answer.cntlrName = ack.name;
		if (ack.status == SUCCESS) {
			LOG_DEBUG ("Start of calibration was succesful");
			answer.result = CT_RESULT_NO_ERROR;
			setObservationState(ack.name, CTState::PREPARED);
		}
		else {
			LOG_ERROR("Start of calibration failed, staying in CLAIMED mode");
			answer.result = CT_RESULT_CALSTART_FAILED;
			setObservationState(ack.name, CTState::CLAIMED);
		}
		itsParentPort->send(answer);
	}
	break;

	case CAL_STOPACK: {
		CALStopackEvent			ack(event);
		CONTROLReleasedEvent	answer;
		answer.cntlrName = ack.name;
		if (ack.status == SUCCESS) {
			LOG_DEBUG ("Calibration successfully stopped");
			setObservationState(ack.name, CTState::RELEASED);
			answer.result = CT_RESULT_NO_ERROR;
		}
		else {
			LOG_ERROR("Stop of calibration failed, going to SUSPENDED mode");
			setObservationState(ack.name, CTState::SUSPENDED);
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
// claimResources(name)
//
bool CalibrationControl::claimResources(const string& name)
{
#if 0
	// claim the resources:
	ResourceAllocator::TRcuSubset 	rcuSubset;
	for (TRCUMap::iterator it = m_rcuMap.begin(); it != m_rcuMap.end(); ++it) {
		rcuSubset.set(it->first);
	}
	uint8 rcuControl  = getRcuControlValue(itsBandFilter);

	if(!_getResourceAllocator()->claimSRG(shared_from_this(),
										  _getPriority(),
										  rcuSubset,
										  nyquistZone,
										  rcuControl)) {
		errorCode = LD_RESULT_LOW_PRIORITY;
		return (false);
	}

	_getResourceAllocator()->logSRGallocation();
#endif
	return (true);
}


//
// loadPVSSpropertySets
//
void	CalibrationControl::loadPVSSpropertySets()
{
#if 0
	// load propertysets for the rcus
	try {
		for(vector<uint16>::iterator it = rcuVector.begin(); 
													it != rcuVector.end(); ++it) {
			char scopeString[300];
			int  boardRelNr, apRelNr, rcuRelNr;
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
#endif
}


//
// startCalibration(name)
//
bool	CalibrationControl::startCalibration(const string& name) 
{
	// search observation
	OIter		observation = itsObsMap.find(name);
	if (observation == itsObsMap.end()) {
		LOG_DEBUG_STR(name << " is unknown, ignoring event");
		return(false);
	}

	// send CALStartEvent
	CALStartEvent calStartEvent;
	calStartEvent.name   	   = name;
	calStartEvent.parent 	   = observation->second.antennaArray;
	calStartEvent.nyquist_zone = observation->second.nyquistZone;
	calStartEvent.rcumode().resize(1);
	calStartEvent.rcumode()(0).setRaw(convertFilterSelection(observation->second.bandSelection));
	calStartEvent.subset 	   = observation->second.RCUset;
	LOG_DEBUG_STR("Sending CALSTART(" << name <<","<< calStartEvent.parent <<","<<
				   calStartEvent.nyquist_zone <<","<< 
				   convertFilterSelection(observation->second.bandSelection) <<")");

	itsCalServer->send(calStartEvent);
	return (true);
}


//
// stopCalibration(name)
//
bool	CalibrationControl::stopCalibration(const string&	name)
{
	// search observation
	OIter		observation = itsObsMap.find(name);
	if (observation == itsObsMap.end()) {
		LOG_DEBUG_STR(name << " is unknown, ignoring event");
		return(false);
	}

	// send CALStopEvent
	LOG_DEBUG_STR ("Sending CALSTOP(" << name << ") to CALserver");
	CALStopEvent calStopEvent;
	calStopEvent.name = name;
	itsCalServer->send(calStopEvent);

#if 0
	itsResourceAllocator->releaseSRG(shared_from_this()); // TODO
	itsResourceAllocator->logSRGallocation();
#endif

	return (true);
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


//
// addObservation(name)
//
bool CalibrationControl::addObservation(const string&	name)
{
	// Already in admin? return error.
	if (itsObsMap.find(name) != itsObsMap.end()) {
		LOG_DEBUG_STR(name << " already in admin, returning error");
		return (false);
	}

	// find and read parameterset of this observation.
	ParameterSet	theObsPS;
	LOG_TRACE_OBJ_STR("Trying to readfile " << LOFAR_SHARE_LOCATION << "/" << name);
	theObsPS.adoptFile(string(LOFAR_SHARE_LOCATION) + "/" + name);

	ObsInfo_t		theNewObs;
	theNewObs.state			= CTState::CREATED;
	theNewObs.nyquistZone	= theObsPS.getInt16("Observation.nyquistZone");
	theNewObs.sampleFreq	= theObsPS.getUint32("Observation.sampleClock");
	theNewObs.bandSelection = theObsPS.getString("Observation.bandFilter");
	theNewObs.antennaArray	= theObsPS.getString("Observation.antennaArray");
	vector<uint16>	RCUnumbers(theObsPS.getUint16Vector("Observation.receiverList"));
	if (RCUnumbers.empty()) {							// No receivers in list?
		theNewObs.RCUset.set();							// assume all receivers
	}
	else {
		theNewObs.RCUset.reset();						// clear set.
		for (uint i = 0; i < RCUnumbers.size(); i++) {
			theNewObs.RCUset.set(RCUnumbers[i]);		// set mentioned receivers
		}
	}

	LOG_DEBUG_STR("Adding " << name << " to administration");
	itsObsMap[name] = theNewObs;

	return (true);
}

//
// handleClaimEvent(observation)
//
uint16 CalibrationControl::handleClaimEvent(const string&	name)
{
	// search observation
	OIter		observation = itsObsMap.find(name);
	if (observation == itsObsMap.end()) {
		LOG_DEBUG_STR(name << " is unknown, ignoring event");
		return(CT_RESULT_CLAIM_FAILED);
	}

	// check if receivers are available
	if (!claimResources(name)) {
		LOG_ERROR_STR (name << ":failed to claim resources, staying in idle mode");
		return (CT_RESULT_CLAIM_FAILED);
	}

	LOG_DEBUG_STR (name << ":resources claimed successful");
	observation->second.state = CTState::CLAIMED;
	return (CT_RESULT_NO_ERROR);
}

//
// setObservationState(name, state)
//
void CalibrationControl::setObservationState(const string& 		name, 
											 CTState::CTstateNr	newState)
{
	// search observation
	OIter		observation = itsObsMap.find(name);
	if (observation == itsObsMap.end()) {
		return;
	}

	observation->second.state = newState;
}

}; // StationCU
}; // LOFAR
