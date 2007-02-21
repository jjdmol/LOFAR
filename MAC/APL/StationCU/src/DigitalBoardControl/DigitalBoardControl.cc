//#  DigitalBoardControl.cc: Implementation of the MAC Scheduler task
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
#include <GCF/PAL/GCF_Answer.h>
#include <GCF/Utils.h>
#include <GCF/GCF_ServiceInfo.h>
#include <GCF/Protocols/PA_Protocol.ph>
#include <APL/APLCommon/APL_Defines.h>
#include <APL/APLCommon/APLCommonExceptions.h>
#include <APL/APLCommon/Controller_Protocol.ph>
#include <APL/RSP_Protocol/RSP_Protocol.ph>
#include <APL/APLCommon/StationInfo.h>
#include <signal.h>

#include "DigitalBoardControl.h"
#include "DigitalBoardControlDefines.h"

using namespace LOFAR::GCF::Common;
using namespace LOFAR::GCF::TM;
using namespace LOFAR::GCF::PAL;
using namespace std;

namespace LOFAR {
	using namespace APLCommon;
	using namespace ACC::APS;
	namespace StationCU {
	
// static pointer to this object for signal handler
static DigitalBoardControl*	thisDigitalBoardControl = 0;

//
// DigitalBoardControl()
//
DigitalBoardControl::DigitalBoardControl(const string&	cntlrName) :
	GCFTask 			((State)&DigitalBoardControl::initial_state,cntlrName),
	PropertySetAnswerHandlerInterface(),
	itsPropertySetAnswer(*this),
	itsOwnPropertySet	(),
	itsExtPropertySet	(),
	itsOwnPSinitialized (false),
	itsExtPSinitialized (false),
	itsParentPort		(0),
	itsTimerPort		(0),
	itsRSPDriver		(0)
{
	LOG_TRACE_OBJ_STR (cntlrName << " construction");

	// Readin some parameters from the ParameterSet.
	itsTreePrefix = globalParameterSet()->getString("prefix");
	itsInstanceNr = globalParameterSet()->getUint32(itsTreePrefix + "instanceNr");
// for now
itsClock = globalParameterSet()->getUint32(itsTreePrefix + "sampleClock");

	// TODO
	LOG_INFO("MACProcessScope: LOFAR.PermSW.DigBoardCtrl");

	// need port for timers.
	itsTimerPort = new GCFTimerPort(*this, "TimerPort");

	// prepare TCP port to RSPDriver.
	itsRSPDriver = new GCFTCPPort (*this, MAC_SVCMASK_RSPDRIVER,
											GCFPortInterface::SAP, RSP_PROTOCOL);
	ASSERTSTR(itsRSPDriver, "Cannot allocate TCPport to RSPDriver");
	itsRSPDriver->setInstanceNr(itsInstanceNr);

	// attach to parent control task
	itsParentControl = ParentControl::instance();
	itsParentPort = new GCFITCPort (*this, *itsParentControl, "ParentITCport", 
									GCFPortInterface::SAP, CONTROLLER_PROTOCOL);
	ASSERTSTR(itsParentPort, "Cannot allocate ITCport for Parentcontrol");
	itsParentPort->open();		// will result in F_CONNECTED

	// for debugging purposes
	registerProtocol (CONTROLLER_PROTOCOL, CONTROLLER_PROTOCOL_signalnames);
	registerProtocol (PA_PROTOCOL, 		   PA_PROTOCOL_signalnames);
	registerProtocol (RSP_PROTOCOL, 	   RSP_PROTOCOL_signalnames);
	registerProtocol (F_PML_PROTOCOL, 	   F_PML_PROTOCOL_signalnames);
}


//
// ~DigitalBoardControl()
//
DigitalBoardControl::~DigitalBoardControl()
{
	LOG_TRACE_OBJ_STR (getName() << " destruction");

	cancelSubscription();	// tell RSPdriver to stop sending updates.

	if (itsRSPDriver) {
		itsRSPDriver->close();
	}

	// ...
}


//
// sigintHandler(signum)
//
void DigitalBoardControl::sigintHandler(int signum)
{
	LOG_DEBUG (formatString("SIGINT signal detected (%d)",signum));

	if (thisDigitalBoardControl) {
		thisDigitalBoardControl->finish();
	}
}

//
// finish
//
void DigitalBoardControl::finish()
{
	TRAN(DigitalBoardControl::finishing_state);
}


//
// handlePropertySetAnswer(answer)
//
void DigitalBoardControl::handlePropertySetAnswer(GCFEvent& answer)
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
		else {
			LOG_DEBUG_STR("Property set " << pPropAnswer->pScope << 
													" enabled, continuing main task");
		}
		// always let timer expire so main task will continue.
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

		// don't watch state and error fields.
		if ((strstr(pPropAnswer->pPropName, PVSSNAME_FSM_STATE) != 0) || 
			(strstr(pPropAnswer->pPropName, PVSSNAME_FSM_ERROR) != 0) ||
			(strstr(pPropAnswer->pPropName, PVSSNAME_FSM_LOGMSG) != 0)) {
			return;
		}

		if (strstr(pPropAnswer->pPropName, PN_SC_CLOCK) != 0) {
			int		newClock = (static_cast<const GCFPVInteger*>(pPropAnswer->pValue))->getValue();
			if (newClock != itsClock) {
				itsClock = newClock;
				LOG_DEBUG_STR("Received clock change from PVSS, clock is now " << itsClock);
				sendClockSetting();
			}
			break;
		}
		LOG_WARN_STR("Got VCHANGEMSG signal from unknown property " << 
															pPropAnswer->pPropName);
	}

	case F_SUBSCRIBED: {
		GCFPropAnswerEvent* pPropAnswer=static_cast<GCFPropAnswerEvent*>(&answer);
		LOG_DEBUG_STR("Got SUBSCRIBE signal from property " << pPropAnswer->pPropName);
		if (strstr(pPropAnswer->pPropName, PN_SC_CLOCK) != 0) {
			// signal main task we have a subscription
			itsTimerPort->setTimer(0.5);
		}
		break;
	}

//	case F_SUBSCRIBED: 		GCFPropAnswerEvent		pPropName
//	case F_UNSUBSCRIBED:	GCFPropAnswerEvent		pPropName
//	case F_PS_CONFIGURED:	GCFConfAnswerEvent		pApcName
//	case F_EXTPS_LOADED:	GCFPropSetAnswerEvent	pScope, result
//	case F_EXTPS_UNLOADED:	GCFPropSetAnswerEvent	pScope, result
//	case F_MYPS_ENABLED:	GCFPropSetAnswerEvent	pScope, result
//	case F_MYPS_DISABLED:	GCFPropSetAnswerEvent	pScope, result
//	case F_VGETRESP:		GCFPropValueEvent		pValue, pPropName
//	case F_VSETRESP:		GCFPropAnswerEvent		pPropName
//	case F_VCHANGEMSG:		GCFPropValueEvent		pValue, pPropName
//	case F_SERVER_GONE:		GCFPropSetAnswerEvent	pScope, result

	default:
		break;
	}  
}


//
// initial_state(event, port)
//
// Setup connection with PVSS
//
GCFEvent::TResult DigitalBoardControl::initial_state(GCFEvent& event, 
													GCFPortInterface& port)
{
	LOG_DEBUG_STR ("initial:" << eventstr(event) << "@" << port.getName());

	GCFEvent::TResult status = GCFEvent::HANDLED;
  
	switch (event.signal) {
    case F_INIT:
   		break;

	case F_ENTRY: {
		// Get access to my own propertyset.
		string	myPropSetName(createPropertySetName(PSN_DIG_BOARD_CTRL, getName()));
myPropSetName=myPropSetName.substr(8);
		LOG_DEBUG_STR ("Activating PropertySet " << myPropSetName);
		itsOwnPropertySet = GCFMyPropertySetPtr(
								new GCFMyPropertySet(myPropSetName.c_str(),
													 PST_DIG_BOARD_CTRL,
													 PS_CAT_PERM_AUTOLOAD,
													 &itsPropertySetAnswer));
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

			// first redirect signalHandler to our finishing state to leave PVSS
			// in the right state when we are going down
			thisDigitalBoardControl = this;
			signal (SIGINT,  DigitalBoardControl::sigintHandler);	// ctrl-c
			signal (SIGTERM, DigitalBoardControl::sigintHandler);	// kill

			// update PVSS.
			LOG_TRACE_FLOW ("Updateing state to PVSS");
			itsOwnPropertySet->setValue(PVSSNAME_FSM_STATE,GCFPVString("initial"));
			itsOwnPropertySet->setValue(PVSSNAME_FSM_ERROR,GCFPVString(""));
			itsOwnPropertySet->setValue(PN_DBC_CONNECTED, GCFPVBool(false));
			
			// Now connect to propertyset that dictates the clocksetting
			string	extPropSetName(createPropertySetName(PSN_STATION_CLOCK, getName()));
extPropSetName= extPropSetName.substr(8);
			LOG_DEBUG_STR ("Connecting to PropertySet " << extPropSetName);
			itsExtPropertySet = GCFExtPropertySetPtr(
									new GCFExtPropertySet(extPropSetName.c_str(),
														  PST_STATION_CLOCK,
														  &itsPropertySetAnswer));
			itsExtPropertySet->load();		// will result in F_EXTPS_LOADED
			break;
		}

		// second timer event is from external propertyset
		if (!itsExtPSinitialized) {
			itsExtPSinitialized = true;

			// take subscription
			LOG_DEBUG_STR("Taking subscription in property " <<  PN_SC_CLOCK);
			itsExtPropertySet->subscribeProp(PN_SC_CLOCK);
			break;
		}

		// third timer event is from subscription
		if (itsClock == 0) {
			// get current value
			LOG_DEBUG_STR("Getting current value of property " <<  PN_SC_CLOCK);
			GCFProperty*	requiredClockProp = 
									itsExtPropertySet->getProperty(PN_SC_CLOCK);
			ASSERTSTR(requiredClockProp, "Property " << PN_SC_CLOCK << 
									" not in propertyset " << PSN_STATION_CLOCK);
			requiredClockProp->requestValue();
			break;
		}

		// fourth timer event is from retrieving the required clock.
		if (itsClock) {
			LOG_DEBUG ("Attached to external propertySet, going to connect state");
			itsOwnPropertySet->setValue(PN_DBC_CLOCK,GCFPVInteger(itsClock));
			itsParentPort = itsParentControl->registerTask(this);
			TRAN(DigitalBoardControl::connect_state);			// go to next state.
		}
		break;

	case F_CONNECTED:
		break;

	case F_DISCONNECTED:
		break;
	
	default:
		LOG_DEBUG_STR ("initial, default");
		status = defaultMessageHandling(event, port);
		break;
	}    
	return (status);
}


//
// connect_state(event, port)
//
// Setup connection with RSPdriver
//
GCFEvent::TResult DigitalBoardControl::connect_state(GCFEvent& event, 
													GCFPortInterface& port)
{
	LOG_DEBUG_STR ("connect:" << eventstr(event) << "@" << port.getName());

	GCFEvent::TResult status = GCFEvent::HANDLED;
  
	switch (event.signal) {
    case F_INIT:
   		break;

	case F_ENTRY:
	case F_TIMER:
		itsOwnPropertySet->setValue(PVSSNAME_FSM_STATE, GCFPVString("connecting"));
		itsOwnPropertySet->setValue(PN_DBC_CONNECTED,  GCFPVBool(false));
		itsSubscription = 0;
		itsRSPDriver->open();		// will result in F_CONN or F_DISCONN
		break;

	case F_CONNECTED:
		if (&port == itsRSPDriver) {
			LOG_DEBUG ("Connected with RSPDriver, going to subscription state");
			itsOwnPropertySet->setValue(PVSSNAME_FSM_ERROR, GCFPVString(""));
			itsOwnPropertySet->setValue(PN_DBC_CONNECTED,  GCFPVBool(true));
			TRAN(DigitalBoardControl::subscribe_state);		// go to next state.
		}
		break;

	case F_DISCONNECTED:
		port.close();
		ASSERTSTR (&port == itsRSPDriver, 
								"F_DISCONNECTED event from port " << port.getName());
		LOG_DEBUG("Connection with RSPDriver failed, retry in 2 seconds");
		itsOwnPropertySet->setValue(PVSSNAME_FSM_ERROR, GCFPVString("connection timeout"));
		itsTimerPort->setTimer(2.0);
		break;

	default:
		LOG_DEBUG_STR ("connect, default");
		status = defaultMessageHandling(event, port);
		break;
	}    

	return (status);
}

//
// subscribe_state(event, port)
//
// Take subscription on clock modifications
//
GCFEvent::TResult DigitalBoardControl::subscribe_state(GCFEvent& event, 
													GCFPortInterface& port)
{
	LOG_DEBUG_STR ("subscribe:" << eventstr(event) << "@" << port.getName());

	GCFEvent::TResult status = GCFEvent::HANDLED;
  
	switch (event.signal) {
    case F_INIT:
   		break;

	case F_ENTRY:
	case F_TIMER:
		itsOwnPropertySet->setValue(PVSSNAME_FSM_STATE,GCFPVString("subscribe on clock"));
		requestSubscription();		// will result in RSP_SUBCLOCKACK;
		break;

	case F_DISCONNECTED:
		_disconnectedHandler(port);		// might result in transition to connect_state
		break;

	case RSP_SUBCLOCKACK: {
		RSPSubclockackEvent	ack(event);
		if (ack.status != SUCCESS) {
			LOG_WARN ("Could not get subscribtion on clock, retry in 2 seconds");
			itsOwnPropertySet->setValue(PVSSNAME_FSM_ERROR, GCFPVString("subscribe failed"));
			itsTimerPort->setTimer(2.0);
			break;
		}

		itsSubscription = ack.handle;
	
		LOG_DEBUG ("Subscription successful, going to retrieve state");
		itsOwnPropertySet->setValue(PVSSNAME_FSM_ERROR,GCFPVString(""));
		TRAN(DigitalBoardControl::retrieve_state);				// go to next state.
		}
		break;

	default:
		LOG_DEBUG_STR ("subscribe, default");
		status = defaultMessageHandling(event, port);
		break;
	}    

	return (status);
}


//
// retrieve_state(event, port)
//
// Retrieve sampleclock from RSP driver
//
GCFEvent::TResult DigitalBoardControl::retrieve_state(GCFEvent& event, 
													GCFPortInterface& port)
{
	LOG_DEBUG_STR ("retrieve:" << eventstr(event) << "@" << port.getName());

	GCFEvent::TResult status = GCFEvent::HANDLED;
  
	switch (event.signal) {
    case F_INIT:
   		break;

	case F_ENTRY:
	case F_TIMER:
		itsOwnPropertySet->setValue(PVSSNAME_FSM_STATE,GCFPVString("retrieve clock"));
		requestClockSetting();		// will result in RSP_GETCLOCKACK;
		break;

	case F_DISCONNECTED:
		_disconnectedHandler(port);		// might result in transition to connect_state
		break;

	case RSP_GETCLOCKACK: {
		RSPGetclockackEvent	ack(event);
		if (ack.status != SUCCESS) {
			LOG_WARN ("Could not retrieve clocksetting of RSPDriver, retry in 2 seconds");
			itsOwnPropertySet->setValue(PVSSNAME_FSM_ERROR, GCFPVString("getclock failed"));
			itsTimerPort->setTimer(2.0);
			break;
		}
		itsOwnPropertySet->setValue(PVSSNAME_FSM_ERROR,GCFPVString(""));

		if (ack.clock != itsClock) {
			LOG_INFO_STR ("StationClock is " << ack.clock << ", required clock is " << itsClock << ", changing StationClock");
			LOG_DEBUG ("Going to setClock state");
			TRAN(DigitalBoardControl::setClock_state);
		}
		else {
			LOG_INFO_STR ("StationClock is " << ack.clock << ", required clock is " << itsClock << ", no action required");

			LOG_DEBUG ("Going to operational state");
			TRAN(DigitalBoardControl::active_state);				// go to next state.
		}
		break;
	}

	default:
		LOG_DEBUG_STR ("retrieve, default");
		status = defaultMessageHandling(event, port);
		break;
	}    

	return (status);
}


//
// setClock_state(event, port)
//
// Set sampleclock from RSP driver
//
GCFEvent::TResult DigitalBoardControl::setClock_state(GCFEvent& event, 
													GCFPortInterface& port)
{
	LOG_DEBUG_STR ("setClock:" << eventstr(event) << "@" << port.getName());

	GCFEvent::TResult status = GCFEvent::HANDLED;
  
	switch (event.signal) {
    case F_INIT:
   		break;

	case F_ENTRY:
	case F_TIMER:
		itsOwnPropertySet->setValue(PVSSNAME_FSM_STATE,GCFPVString("set clock"));
		sendClockSetting();		// will result in RSP_SETCLOCKACK;
		break;

	case F_DISCONNECTED:
		_disconnectedHandler(port);		// might result in transition to connect_state
		break;

	case RSP_SETCLOCKACK: {
		RSPSetclockackEvent		ack(event);
		if (ack.status != SUCCESS) {
			LOG_ERROR_STR ("Clock could not be set to " << itsClock << 
															", retry in 5 seconds.");
			itsOwnPropertySet->setValue(PVSSNAME_FSM_ERROR,GCFPVString("clockset error"));
			itsTimerPort->setTimer(5.0);
			break;
		}
		LOG_INFO_STR ("StationClock is set to " << itsClock << ", going to operational state");
		itsOwnPropertySet->setValue(PVSSNAME_FSM_ERROR,GCFPVString(""));
		itsOwnPropertySet->setValue(PN_DBC_CLOCK,GCFPVInteger(itsClock));
		TRAN(DigitalBoardControl::active_state);				// go to next state.
		break;
	}

	default:
		LOG_DEBUG_STR ("setClock, default");
		status = defaultMessageHandling(event, port);
		break;
	}    

	return (status);
}


//
// active_state(event, port)
//
// Normal operation state. 
//
GCFEvent::TResult DigitalBoardControl::active_state(GCFEvent& event, GCFPortInterface& port)
{
	LOG_DEBUG_STR ("active:" << eventstr(event) << "@" << port.getName());

	GCFEvent::TResult status = GCFEvent::HANDLED;

	switch (event.signal) {
	case F_INIT:
		break;

	case F_ENTRY: {
		// update PVSS
		itsOwnPropertySet->setValue(PVSSNAME_FSM_STATE,GCFPVString("active"));
		itsOwnPropertySet->setValue(PVSSNAME_FSM_ERROR,GCFPVString(""));
		break;
	}

	case F_ACCEPT_REQ:
		break;

	case F_CONNECTED:
		break;

	case F_DISCONNECTED:
		_disconnectedHandler(port);		// might result in transition to connect_state
		break;

	case F_TIMER: 
		break;

	case RSP_UPDCLOCK: {
		RSPUpdclockEvent	updateEvent(event);
		if (updateEvent.status != SUCCESS || updateEvent.clock == 0) {
			LOG_ERROR_STR ("StationClock has stopped! Going to setClock state to try to solve the problem");
			itsOwnPropertySet->setValue(PVSSNAME_FSM_ERROR,GCFPVString("Clock stopped"));
			TRAN(DigitalBoardControl::setClock_state);
			break;
		}

		if (updateEvent.clock != itsClock) {
			LOG_ERROR_STR ("CLOCK WAS CHANGED TO " << updateEvent.clock << 
						   " BY SOMEONE WHILE CLOCK SHOULD BE " << itsClock << ". CHANGING CLOCK BACK.");
			itsOwnPropertySet->setValue(PVSSNAME_FSM_ERROR,GCFPVString("Clock unallowed changed"));
			TRAN (DigitalBoardControl::setClock_state);
		}

		// when update.clock==itsClock ignore it, we probable caused it ourselves.
		break;
	}

	default:
		LOG_DEBUG("active_state, default");
		status = defaultMessageHandling(event, port);
		break;
	}

	return (status);
}


//
// _disconnectedHandler(port)
//
void DigitalBoardControl::_disconnectedHandler(GCFPortInterface& port)
{
	port.close();
	if (&port == itsRSPDriver) {
		LOG_DEBUG("Connection with RSPDriver failed, going to reconnect state");
		itsOwnPropertySet->setValue(PVSSNAME_FSM_ERROR,GCFPVString("connection lost"));
		TRAN (DigitalBoardControl::connect_state);
	}
}

//
// requestSubscription()
//
void DigitalBoardControl::requestSubscription()
{
	LOG_INFO ("Taking subscription on clock settings");

	RSPSubclockEvent		msg;
	
	msg.timestamp.setNow();
	msg.period = 1;				// let RSPdriver check every second

	itsRSPDriver->send(msg);
}

//
// cancelSubscription()
//
void DigitalBoardControl::cancelSubscription()
{
	LOG_INFO ("Canceling subscription on clock settings");

	RSPUnsubclockEvent		msg;
	
	msg.handle = itsSubscription;
	itsSubscription = 0;

	itsRSPDriver->send(msg);
}

//
// requestClockSetting()
//
void DigitalBoardControl::requestClockSetting()
{
	LOG_INFO ("Asking RSPdriver current clock setting");

	RSPGetclockEvent		msg;
	
	msg.timestamp.setNow();
	msg.cache = 1;

	itsRSPDriver->send(msg);
}


//
// sendClockSetting()
//
void DigitalBoardControl::sendClockSetting()
{
	LOG_INFO_STR ("Setting stationClock to " << itsClock << " MHz");

	RSPSetclockEvent		msg;
	
	msg.timestamp.setNow();
	msg.clock = itsClock;

	itsRSPDriver->send(msg);
}

//
// defaultMessageHandling
//
GCFEvent::TResult DigitalBoardControl::defaultMessageHandling(GCFEvent& 		event, 
															  GCFPortInterface& port)
{
	switch (event.signal) {
		case CONTROL_CONNECT: {
			CONTROLConnectEvent		msg(event);
			CONTROLConnectedEvent	answer;
			answer.cntlrName = msg.cntlrName;
			answer.result = true;
			itsParentPort->send(answer);
		}
		break;

		case CONTROL_SCHEDULE: {
			CONTROLScheduleEvent		msg(event);
			CONTROLScheduledEvent	answer;
			answer.cntlrName = msg.cntlrName;
			answer.result = 0;
			itsParentPort->send(answer);
		}
		break;

		case CONTROL_CLAIM: {
			CONTROLClaimEvent		msg(event);
			CONTROLClaimedEvent	answer;
			answer.cntlrName = msg.cntlrName;
			answer.result = 0;
			itsParentPort->send(answer);
		}
		break;

		case CONTROL_PREPARE: {
			CONTROLPrepareEvent		msg(event);
			CONTROLPreparedEvent	answer;
			answer.cntlrName = msg.cntlrName;
			answer.result = 0;
			itsParentPort->send(answer);
		}
		break;

		case CONTROL_RESUME: {
			CONTROLResumeEvent		msg(event);
			CONTROLResumedEvent	answer;
			answer.cntlrName = msg.cntlrName;
			answer.result = 0;
			itsParentPort->send(answer);
		}
		break;

		case CONTROL_SUSPEND: {
			CONTROLSuspendEvent		msg(event);
			CONTROLSuspendedEvent	answer;
			answer.cntlrName = msg.cntlrName;
			answer.result = 0;
			itsParentPort->send(answer);
		}
		break;

		case CONTROL_RELEASE: {
			CONTROLReleaseEvent		msg(event);
			CONTROLReleasedEvent	answer;
			answer.cntlrName = msg.cntlrName;
			answer.result = 0;
			itsParentPort->send(answer);

			CONTROLQuitedEvent		qEvent;
			qEvent.cntlrName = msg.cntlrName;
			qEvent.treeID    = getObservationNr(msg.cntlrName);
			qEvent.errorMsg  = "Normal Termination";
			qEvent.result    = 0;
			LOG_DEBUG_STR("Sending QUITED event");
			itsParentPort->send(qEvent);
		}
		break;

		case CONTROL_RESYNC: {

		}
		break;

		default: {
			LOG_WARN_STR ("no action defined for event:" << eventstr(event));
		}
		break;
	}

	return (GCFEvent::HANDLED);
}

//
// finishing_state(event, port)
//
// Write controller state to PVSS, wait for 1 second (using a timer) to let 
// GCF handle the property change and close the controller
//
GCFEvent::TResult DigitalBoardControl::finishing_state(GCFEvent& event, GCFPortInterface& port)
{
	LOG_DEBUG_STR ("finishing_state:" << evtstr(event) << "@" << port.getName());

	GCFEvent::TResult status = GCFEvent::HANDLED;

	switch (event.signal) {
	case F_INIT:
		break;

	case F_ENTRY: {
		// update PVSS
		itsOwnPropertySet->setValue(string(PVSSNAME_FSM_STATE),GCFPVString("finished"));
		itsOwnPropertySet->setValue(string(PVSSNAME_FSM_ERROR),GCFPVString(""));

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


}; // StationCU
}; // LOFAR
