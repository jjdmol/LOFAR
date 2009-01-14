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
#include <Common/SystemUtil.h>
#include <Common/Version.h>

#include <Common/ParameterSet.h>
#include <MACIO/MACServiceInfo.h>
#include <GCF/PVSS/GCF_PVTypes.h>
#include <APL/APLCommon/APL_Defines.h>
#include <APL/APLCommon/Controller_Protocol.ph>
#include <APL/APLCommon/StationInfo.h>
#include <APL/RSP_Protocol/RSP_Protocol.ph>
#include <GCF/RTDB/DP_Protocol.ph>
#include <signal.h>

#include "DigitalBoardControl.h"
#include "PVSSDatapointDefs.h"
#include "../Package__Version.h"

using namespace LOFAR::GCF::TM;
using namespace LOFAR::GCF::PVSS;
using namespace LOFAR::GCF::RTDB;
using namespace std;

namespace LOFAR {
	using namespace APLCommon;
	namespace StationCU {
	
// static pointer to this object for signal handler
static DigitalBoardControl*	thisDigitalBoardControl = 0;

//
// DigitalBoardControl()
//
DigitalBoardControl::DigitalBoardControl(const string&	cntlrName) :
	GCFTask 			((State)&DigitalBoardControl::initial_state,cntlrName),
	itsOwnPropertySet	(0),
	itsExtPropertySet	(0),
	itsOwnPSinitialized (false),
	itsExtPSinitialized (false),
	itsParentPort		(0),
	itsTimerPort		(0),
	itsRSPDriver		(0)
{
	LOG_TRACE_OBJ_STR (cntlrName << " construction");
	LOG_INFO(Version::getInfo<StationCUVersion>("DigitalBoardControl"));

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
	registerProtocol (CONTROLLER_PROTOCOL, CONTROLLER_PROTOCOL_STRINGS);
	registerProtocol (DP_PROTOCOL, 		DP_PROTOCOL_STRINGS);
	registerProtocol (RSP_PROTOCOL, 	    RSP_PROTOCOL_STRINGS);
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
// _databaseEventHandler(event)
//
void DigitalBoardControl::_databaseEventHandler(GCFEvent& event)
{
	LOG_DEBUG_STR ("_databaseEventHandler:" << eventName(event));

	switch(event.signal) {

	case DP_CHANGED: {
		DPChangedEvent	dpEvent(event);
		LOG_DEBUG_STR("_databaseEventHandler:DP_CHANGED(" << dpEvent.DPname << ")");

		// don't watch state and error fields.
		if ((strstr(dpEvent.DPname.c_str(), PN_OBJ_STATE) != 0) || 
			(strstr(dpEvent.DPname.c_str(), PN_FSM_ERROR) != 0) ||
			(strstr(dpEvent.DPname.c_str(), PN_FSM_CURRENT_ACTION) != 0) ||
			(strstr(dpEvent.DPname.c_str(), PN_FSM_LOG_MSG) != 0)) {
			return;
		}

		if (strstr(dpEvent.DPname.c_str(), PN_SCK_CLOCK) != 0) {
			GCFPVInteger*	clockObj = (GCFPVInteger*)dpEvent.value._pValue;
			int32			newClock = clockObj->getValue();
			if (newClock != itsClock) {
				itsClock = newClock;
				LOG_DEBUG_STR("Received clock change from PVSS, clock is now " << itsClock);
				TRAN(DigitalBoardControl::setClock_state);
				// sendClockSetting();
			}
			break;
		}
		LOG_WARN_STR("Got VCHANGEMSG signal from unknown property " << 
															dpEvent.DPname);
	}

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
	LOG_DEBUG_STR ("initial:" << eventName(event) << "@" << port.getName());

	GCFEvent::TResult status = GCFEvent::HANDLED;
  
	switch (event.signal) {
    case F_INIT:
   		break;

	case F_ENTRY: {
		// Get access to my own propertyset.
		string	myPropSetName(createPropertySetName(PSN_DIG_BOARD_CTRL, getName()));
		LOG_DEBUG_STR ("Activating PropertySet " << myPropSetName);
		itsOwnPropertySet = new RTDBPropertySet(myPropSetName,
												PST_DIG_BOARD_CTRL,
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
	  
	case F_TIMER: {
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
			itsOwnPropertySet->setValue(PN_FSM_CURRENT_ACTION,GCFPVString("Initial"));
			itsOwnPropertySet->setValue(PN_FSM_ERROR, GCFPVString(""));
			itsOwnPropertySet->setValue(PN_DBC_CONNECTED,   GCFPVBool(false));
			
			// Now connect to propertyset that dictates the clocksetting
			string	extPropSetName(createPropertySetName(PSN_STATION_CLOCK, getName()));
//extPropSetName= extPropSetName.substr(8);
			LOG_DEBUG_STR ("Connecting to PropertySet " << extPropSetName);
			itsExtPropertySet = new RTDBPropertySet(extPropSetName,
													PST_STATION_CLOCK,
													PSAT_RO,
													this);
			break;
		}

		// second timer event is from external propertyset
		if (!itsExtPSinitialized) {
			itsExtPSinitialized = true;

			// take subscription
			LOG_DEBUG("Taking subscription on clock datapoint");
			itsExtPropertySet->setSubscription(true);
			break;
		}
	}
	break;

	case DP_SUBSCRIBED: {
		DPSubscribedEvent	dpEvent(event);
		string	propSetName(createPropertySetName(PSN_STATION_CLOCK, getName()));
		propSetName += "." PN_SCK_CLOCK;
		if (dpEvent.DPname.find(propSetName) != string::npos) {
			GCFPVInteger	clockVal;
			itsExtPropertySet->getValue(PN_SCK_CLOCK, clockVal);
			itsClock = clockVal.getValue();
			LOG_DEBUG_STR("ClockSetting is " << itsClock);

			itsOwnPropertySet->setValue(PN_DBC_CLOCK,GCFPVInteger(itsClock));
			itsParentPort = itsParentControl->registerTask(this);
			LOG_DEBUG ("Going to connect state");
			TRAN(DigitalBoardControl::connect_state);			// go to next state.
		}
	}
	break;

	case F_CONNECTED:
	case F_DISCONNECTED:
	case F_EXIT:
		break;

	case DP_CHANGED:
		_databaseEventHandler(event);
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
	LOG_DEBUG_STR ("connect:" << eventName(event) << "@" << port.getName());

	GCFEvent::TResult status = GCFEvent::HANDLED;
  
	switch (event.signal) {
    case F_EXIT:
   		break;

	case F_ENTRY:
	case F_TIMER:
		itsOwnPropertySet->setValue(PN_FSM_CURRENT_ACTION, GCFPVString("Connecting"));
		itsOwnPropertySet->setValue(PN_DBC_CONNECTED,  GCFPVBool(false));
		itsSubscription = 0;
		itsRSPDriver->open();		// will result in F_CONN or F_DISCONN
		break;

	case F_CONNECTED:
		if (&port == itsRSPDriver) {
			LOG_DEBUG ("Connected with RSPDriver, going to subscription state");
			itsOwnPropertySet->setValue(PN_FSM_ERROR, GCFPVString(""));
			itsOwnPropertySet->setValue(PN_DBC_CONNECTED,  GCFPVBool(true));
			TRAN(DigitalBoardControl::subscribe_state);		// go to next state.
		}
		break;

	case F_DISCONNECTED:
		port.close();
		ASSERTSTR (&port == itsRSPDriver, 
								"F_DISCONNECTED event from port " << port.getName());
		LOG_DEBUG("Connection with RSPDriver failed, retry in 2 seconds");
		itsOwnPropertySet->setValue(PN_FSM_ERROR, GCFPVString("connection timeout"));
		itsTimerPort->setTimer(2.0);
		break;

	case DP_CHANGED:
		_databaseEventHandler(event);
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
	LOG_DEBUG_STR ("subscribe:" << eventName(event) << "@" << port.getName());

	GCFEvent::TResult status = GCFEvent::HANDLED;
  
	switch (event.signal) {
    case F_EXIT:
   		break;

	case F_ENTRY:
	case F_TIMER:
		itsOwnPropertySet->setValue(PN_FSM_CURRENT_ACTION,GCFPVString("Subscribe to clock"));
		requestSubscription();		// will result in RSP_SUBCLOCKACK;
		break;

	case F_DISCONNECTED:
		_disconnectedHandler(port);		// might result in transition to connect_state
		break;

	case RSP_SUBCLOCKACK: {
		RSPSubclockackEvent	ack(event);
		if (ack.status != SUCCESS) {
			LOG_WARN ("Could not get subscribtion on clock, retry in 2 seconds");
			itsOwnPropertySet->setValue(PN_FSM_ERROR, GCFPVString("subscribe failed"));
			itsTimerPort->setTimer(2.0);
			break;
		}

		itsSubscription = ack.handle;
	
		LOG_DEBUG ("Subscription successful, going to retrieve state");
		itsOwnPropertySet->setValue(PN_FSM_ERROR,GCFPVString(""));
		TRAN(DigitalBoardControl::retrieve_state);				// go to next state.
		}
		break;

	case DP_CHANGED:
		_databaseEventHandler(event);
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
	LOG_DEBUG_STR ("retrieve:" << eventName(event) << "@" << port.getName());

	GCFEvent::TResult status = GCFEvent::HANDLED;
  
	switch (event.signal) {
    case F_EXIT:
   		break;

	case F_ENTRY:
	case F_TIMER:
		itsOwnPropertySet->setValue(PN_FSM_CURRENT_ACTION,GCFPVString("Retrieve clock"));
		requestClockSetting();		// will result in RSP_GETCLOCKACK;
		break;

	case F_DISCONNECTED:
		_disconnectedHandler(port);		// might result in transition to connect_state
		break;

	case RSP_GETCLOCKACK: {
		RSPGetclockackEvent	ack(event);
		if (ack.status != SUCCESS) {
			LOG_WARN ("Could not retrieve clocksetting of RSPDriver, retry in 2 seconds");
			itsOwnPropertySet->setValue(PN_FSM_ERROR, GCFPVString("getclock failed"));
			itsTimerPort->setTimer(2.0);
			break;
		}
		itsOwnPropertySet->setValue(PN_FSM_ERROR,GCFPVString(""));

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

	case DP_CHANGED:
		_databaseEventHandler(event);
		break;
	
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
	LOG_DEBUG_STR ("setClock:" << eventName(event) << "@" << port.getName());

	GCFEvent::TResult status = GCFEvent::HANDLED;
  
	switch (event.signal) {
    case F_INIT:
   		break;

	case F_ENTRY:
	case F_TIMER:
		itsOwnPropertySet->setValue(PN_FSM_CURRENT_ACTION,GCFPVString("Set clock"));
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
			itsOwnPropertySet->setValue(PN_FSM_ERROR,GCFPVString("clockset error"));
			itsTimerPort->setTimer(5.0);
			break;
		}
		LOG_INFO_STR ("StationClock is set to " << itsClock << ", going to operational state");
		itsOwnPropertySet->setValue(PN_FSM_ERROR,GCFPVString(""));
		itsOwnPropertySet->setValue(PN_DBC_CLOCK,GCFPVInteger(itsClock));
		TRAN(DigitalBoardControl::active_state);				// go to next state.
		break;
	}

	case DP_CHANGED:
		_databaseEventHandler(event);
		break;
	
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
	LOG_DEBUG_STR ("active:" << eventName(event) << "@" << port.getName());

	GCFEvent::TResult status = GCFEvent::HANDLED;

	switch (event.signal) {
	case F_INIT:
		break;

	case F_ENTRY: {
		// update PVSS
		itsOwnPropertySet->setValue(PN_FSM_CURRENT_ACTION,GCFPVString("Active"));
		itsOwnPropertySet->setValue(PN_FSM_ERROR, GCFPVString(""));
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
			itsOwnPropertySet->setValue(PN_FSM_ERROR,GCFPVString("Clock stopped"));
			TRAN(DigitalBoardControl::setClock_state);
			break;
		}

		if (updateEvent.clock != itsClock) {
			LOG_ERROR_STR ("CLOCK WAS CHANGED TO " << updateEvent.clock << 
						   " BY SOMEONE WHILE CLOCK SHOULD BE " << itsClock << ". CHANGING CLOCK BACK.");
			itsOwnPropertySet->setValue(PN_FSM_ERROR,GCFPVString("Clock unallowed changed"));
			TRAN (DigitalBoardControl::setClock_state);
		}

		// when update.clock==itsClock ignore it, we probable caused it ourselves.
		LOG_DEBUG_STR("Event.clock = " << updateEvent.clock << ", myClock = " << itsClock);
	}
	break;

	case DP_CHANGED:
		_databaseEventHandler(event);
		break;
	
	default:
//		LOG_DEBUG("active_state, default");
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
		itsOwnPropertySet->setValue(PN_FSM_ERROR,GCFPVString("connection lost"));
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
	
//	msg.timestamp = 0;
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
	
//	msg.timestamp = 0;
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
	
//	msg.timestamp = 0;
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

		case DP_CHANGED: {
			DPChangedEvent	dpEvent(event);
			LOG_DEBUG_STR("DP " << dpEvent.DPname << " changed");
		}
		break;

		case DP_SET:
		break;

		default: {
			LOG_WARN_STR ("no action defined for event:" << eventName(event));
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
	LOG_DEBUG_STR ("finishing_state:" << eventName(event) << "@" << port.getName());

	GCFEvent::TResult status = GCFEvent::HANDLED;

	switch (event.signal) {
	case F_INIT:
		break;

	case F_ENTRY: {
		// tell Parent task we like to go down.
		itsParentControl->nowInState(getName(), CTState::QUIT);

		// update PVSS
		itsOwnPropertySet->setValue(PN_FSM_CURRENT_ACTION,GCFPVString("Finished"));
		itsOwnPropertySet->setValue(PN_FSM_ERROR, GCFPVString(""));
		itsOwnPropertySet->setValue(PN_DBC_CONNECTED,   GCFPVBool  (false));

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
