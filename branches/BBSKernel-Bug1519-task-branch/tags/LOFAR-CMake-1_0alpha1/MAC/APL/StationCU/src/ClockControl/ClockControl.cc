//#  ClockControl.cc: Implementation of ClockController of the station.
//#
//#  Copyright (C) 2006-2009
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
#include <ApplCommon/StationConfig.h>
#include <MACIO/MACServiceInfo.h>
#include <GCF/PVSS/GCF_PVTypes.h>
#include <APL/APLCommon/APL_Defines.h>
#include <APL/APLCommon/Controller_Protocol.ph>
#include <APL/RTCCommon/Timestamp.h>
#include <APL/RSP_Protocol/RSP_Protocol.ph>
#include <GCF/RTDB/DP_Protocol.ph>
#include <signal.h>

#include "ClockControl.h"
#include "PVSSDatapointDefs.h"
#include "Clock_Protocol.ph"
#include <APL/ClockProtocol/Package__Version.h>

using namespace LOFAR::GCF::TM;
using namespace LOFAR::GCF::PVSS;
using namespace LOFAR::GCF::RTDB;
using namespace std;

namespace LOFAR {
	using namespace APLCommon;
	namespace StationCU {
	
// static pointer to this object for signal handler
static ClockControl*	thisClockControl = 0;

//
// ClockControl()
//
ClockControl::ClockControl(const string&	cntlrName) :
	GCFTask 			((State)&ClockControl::initial_state,cntlrName),
	itsOwnPropertySet	(0),
	itsOwnPSinitialized (false),
	itsParentPort		(0),
	itsTimerPort		(0),
	itsRSPDriver		(0),
	itsCommandPort		(0),
	itsClock			(160)
{
	LOG_TRACE_OBJ_STR (cntlrName << " construction");
	LOG_INFO(Version::getInfo<StationCUVersion>("ClockControl"));

	// TODO
	LOG_INFO("MACProcessScope: LOFAR.PermSW.ClockControl");

	// need port for timers.
	itsTimerPort = new GCFTimerPort(*this, "TimerPort");

	// prepare TCP port to RSPDriver.
	itsRSPDriver = new GCFTCPPort (*this, MAC_SVCMASK_RSPDRIVER,
											GCFPortInterface::SAP, RSP_PROTOCOL);
	ASSERTSTR(itsRSPDriver, "Cannot allocate TCPport to RSPDriver");

	// attach to parent control task
	itsParentControl = ParentControl::instance();
	itsParentPort = new GCFITCPort (*this, *itsParentControl, "ParentITCport", 
									GCFPortInterface::SAP, CONTROLLER_PROTOCOL);
	ASSERTSTR(itsParentPort, "Cannot allocate ITCport for Parentcontrol");
	itsParentPort->open();		// will result in F_CONNECTED

	// open port for get/set clock/splitters
	itsCommandPort = new GCFTCPPort (*this, MAC_SVCMASK_CLOCKCTRL, 
										GCFPortInterface::MSPP, CLOCK_PROTOCOL);
	ASSERTSTR(itsCommandPort, "Cannot allocate listener for clock commands");

	// for debugging purposes
	registerProtocol (CONTROLLER_PROTOCOL,	CONTROLLER_PROTOCOL_STRINGS);
	registerProtocol (DP_PROTOCOL,			DP_PROTOCOL_STRINGS);
	registerProtocol (RSP_PROTOCOL,			RSP_PROTOCOL_STRINGS);
	registerProtocol (CLOCK_PROTOCOL,		CLOCK_PROTOCOL_STRINGS);
}


//
// ~ClockControl()
//
ClockControl::~ClockControl()
{
	LOG_TRACE_OBJ_STR (getName() << " destruction");

	cancelSubscription();	// tell RSPdriver to stop sending updates.

	if (itsCommandPort) {
		itsCommandPort->close();
	}

	if (itsRSPDriver) {
		itsRSPDriver->close();
	}

	// ...
}


//
// sigintHandler(signum)
//
void ClockControl::sigintHandler(int signum)
{
	LOG_DEBUG (formatString("SIGINT signal detected (%d)",signum));

	if (thisClockControl) {
		thisClockControl->finish();
	}
}

//
// finish
//
void ClockControl::finish()
{
	TRAN(ClockControl::finishing_state);
}


//
// _databaseEventHandler(event)
//
void ClockControl::_databaseEventHandler(GCFEvent& event)
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

		if (strstr(dpEvent.DPname.c_str(), PN_CLC_REQUESTED_CLOCK) != 0) {
			GCFPVInteger*	clockObj = (GCFPVInteger*)dpEvent.value._pValue;
			int32			newClock = clockObj->getValue();
			if (newClock != itsClock) {
				itsClock = newClock;
				LOG_DEBUG_STR("Received clock change from PVSS, clock is now " << itsClock);
				TRAN(ClockControl::setClock_state);
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
GCFEvent::TResult ClockControl::initial_state(GCFEvent& event, 
													GCFPortInterface& port)
{
	LOG_DEBUG_STR ("initial:" << eventName(event) << "@" << port.getName());

	GCFEvent::TResult status = GCFEvent::HANDLED;
  
	switch (event.signal) {
	case F_ENTRY: {
		// Get access to my own propertyset.
		string	myPropSetName(createPropertySetName(PSN_CLOCK_CONTROL, getName()));
		LOG_DEBUG_STR ("Activating PropertySet " << myPropSetName);
		itsOwnPropertySet = new RTDBPropertySet(myPropSetName,
												PST_CLOCK_CONTROL,
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
			thisClockControl = this;
			signal (SIGINT,  ClockControl::sigintHandler);	// ctrl-c
			signal (SIGTERM, ClockControl::sigintHandler);	// kill

			// update PVSS.
			LOG_TRACE_FLOW ("Updateing state to PVSS");
			itsOwnPropertySet->setValue(PN_FSM_CURRENT_ACTION, GCFPVString("Initial"));
			itsOwnPropertySet->setValue(PN_FSM_ERROR, 		   GCFPVString(""));
			itsOwnPropertySet->setValue(PN_CLC_CONNECTED,	   GCFPVBool(false));
			itsOwnPropertySet->setSubscription(true);
			break;
		}
	}
	break;

	case DP_SUBSCRIBED: {
		DPSubscribedEvent	dpEvent(event);
		string	propSetName(createPropertySetName(PSN_CLOCK_CONTROL, getName()));
		propSetName += "." PN_CLC_REQUESTED_CLOCK;
		if (dpEvent.DPname.find(propSetName) != string::npos) {
			GCFPVInteger	clockVal;
			itsOwnPropertySet->getValue(PN_CLC_REQUESTED_CLOCK, clockVal);
			itsClock = clockVal.getValue();
			LOG_DEBUG_STR("ClockSetting is " << itsClock);

			LOG_DEBUG ("Going to connect2RSP state");
			TRAN(ClockControl::connect2RSP_state);			// go to next state.
		}
	}
	break;

	case F_CONNECTED:
		ASSERTSTR(&port == itsParentPort, "Received unexpected F_CONNECTED at port " << port.getName());
		LOG_INFO_STR("Connected to Parent Control Task");
		break;

	case F_ACCEPT_REQ:
		_acceptRequestHandler(port);
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
// connect2RSP_state(event, port)
//
// Setup connection with RSPdriver
//
GCFEvent::TResult ClockControl::connect2RSP_state(GCFEvent& event, 
													GCFPortInterface& port)
{
	LOG_DEBUG_STR ("connect2RSP:" << eventName(event) << "@" << port.getName());

	GCFEvent::TResult status = GCFEvent::HANDLED;
  
	switch (event.signal) {
    case F_EXIT:
   		break;

	case F_ENTRY:
		if (itsRSPDriver->getState() == GCFPortInterface::S_CONNECTED) {
			TRAN(ClockControl::startListener_state);        // go to next state.
		}
		// NO BREAK !!!!
	case F_TIMER:
		itsOwnPropertySet->setValue(PN_FSM_CURRENT_ACTION, GCFPVString("Connecting"));
		itsOwnPropertySet->setValue(PN_CLC_CONNECTED,  GCFPVBool(false));
		itsSubscription = 0;
		itsRSPDriver->open();		// will result in F_CONN or F_DISCONN
		break;

	case F_CONNECTED:
		if (&port == itsRSPDriver) {
			itsTimerPort->cancelAllTimers();
			LOG_DEBUG ("Connected with RSPDriver, starting listener for commands");
			itsOwnPropertySet->setValue(PN_FSM_ERROR, GCFPVString(""));
			itsOwnPropertySet->setValue(PN_CLC_CONNECTED,  GCFPVBool(true));
			TRAN(ClockControl::startListener_state);		// go to next state.
		}
		break;

	case F_DISCONNECTED:
		ASSERTSTR (&port == itsRSPDriver, 
								"F_DISCONNECTED event from port " << port.getName());
		LOG_DEBUG("Connection with RSPDriver failed, retry in 2 seconds");
		itsOwnPropertySet->setValue(PN_FSM_ERROR, GCFPVString("connection timeout"));
		port.close();
		itsTimerPort->setTimer(2.0);
		break;
		
	case F_ACCEPT_REQ:
		_acceptRequestHandler(port);
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
// startListener_state(event, port)
//
// Start listener for clock and splitter commands
//
GCFEvent::TResult ClockControl::startListener_state(GCFEvent& event, 
													GCFPortInterface& port)
{
	LOG_DEBUG_STR ("startListener:" << eventName(event) << "@" << port.getName());

	GCFEvent::TResult status = GCFEvent::HANDLED;
  
	switch (event.signal) {
	case F_ENTRY:
		if (itsCommandPort->getState() == GCFPortInterface::S_CONNECTED) {
			TRAN(ClockControl::subscribe_state);        // go to next state.
		}
		itsOwnPropertySet->setValue(PN_FSM_CURRENT_ACTION, GCFPVString("Opening command-port"));
		itsCommandPort->autoOpen(0, 10, 2);		// will result in F_CONN or F_DISCONN
		break;

	case F_CONNECTED:
		if (&port == itsCommandPort) {
			LOG_DEBUG ("Command port opened, taking subscription on the clock");
			itsOwnPropertySet->setValue(PN_FSM_ERROR, GCFPVString(""));

			// let Parent task register itself at the startDaemon. This will in a CONTROL_CONNECT event from the
			// Parenttask (which we ignore) and probably CLKCTRL_XXX messages from the StationController 
			itsParentPort = itsParentControl->registerTask(this);	// PC reports itself at the startDaemon

			TRAN(ClockControl::subscribe_state);		// go to next state.
		}
		break;

	case F_DISCONNECTED:
		if (&port == itsCommandPort) { 
			LOG_FATAL("Opening of commandport failed, no communication with stationController!!");
			itsOwnPropertySet->setValue(PN_FSM_ERROR, GCFPVString("commandport error"));
			TRAN(ClockControl::finishing_state);
		}
		else {
			_disconnectedHandler(port);		// might result in transition to connect_state
		}
		break;

	case F_ACCEPT_REQ:
		_acceptRequestHandler(port);
	break;

	case DP_CHANGED:
		_databaseEventHandler(event);
		break;
	
	default:
		LOG_DEBUG_STR ("startListener, default");
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
GCFEvent::TResult ClockControl::subscribe_state(GCFEvent& event, 
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

	case F_ACCEPT_REQ:
		_acceptRequestHandler(port);
	break;

	case RSP_SUBCLOCKACK: {
		RSPSubclockackEvent	ack(event);
		if (ack.status != RSP_SUCCESS) {
			LOG_WARN ("Could not get subscribtion on clock, retry in 2 seconds");
			itsOwnPropertySet->setValue(PN_FSM_ERROR, GCFPVString("subscribe failed"));
			itsTimerPort->setTimer(2.0);
			break;
		}

		itsSubscription = ack.handle;
	
		LOG_DEBUG ("Subscription successful, going to retrieve splitter state");
		itsOwnPropertySet->setValue(PN_FSM_ERROR,GCFPVString(""));
		TRAN(ClockControl::retrieveSplitters_state);				// go to next state.
		}
		break;

	case DP_CHANGED:
		_databaseEventHandler(event);
		break;
	
	case CLKCTRL_GET_CLOCK:
	case CLKCTRL_SET_CLOCK:
	case CLKCTRL_GET_SPLITTERS:
	case CLKCTRL_SET_SPLITTERS:
		LOG_INFO_STR("Postponing event " << eventName(event) << " till next state");
		return (GCFEvent::NEXT_STATE);

	default:
		LOG_DEBUG_STR ("subscribe, default");
		status = defaultMessageHandling(event, port);
		break;
	}    

	return (status);
}


//
// retrieveSplitters_state(event, port)
//
// Retrieve splitters-state from RSP driver
//
GCFEvent::TResult ClockControl::retrieveSplitters_state(GCFEvent& event, 
													GCFPortInterface& port)
{
	LOG_DEBUG_STR ("retrieveSplitters:" << eventName(event) << "@" << port.getName());

	GCFEvent::TResult status = GCFEvent::HANDLED;
  
	switch (event.signal) {
    case F_EXIT:
   		break;

	case F_ENTRY:
	case F_TIMER:
		itsOwnPropertySet->setValue(PN_FSM_CURRENT_ACTION,GCFPVString("Retrieve clock"));
		requestSplitterSetting();		// will result in RSP_GETSPLITTERACK;
		break;

	case F_DISCONNECTED:
		_disconnectedHandler(port);		// might result in transition to connect_state
		break;

	case F_ACCEPT_REQ:
		_acceptRequestHandler(port);
	break;

	case RSP_GETSPLITTERACK: {
		RSPGetsplitterackEvent	ack(event);
		if (ack.status != RSP_SUCCESS) {
			LOG_WARN ("Could not retrieve splittersetting of RSPDriver, retry in 2 seconds");
			itsOwnPropertySet->setValue(PN_FSM_ERROR, GCFPVString("getsplitter failed"));
			itsTimerPort->setTimer(2.0);
			break;
		}
		itsOwnPropertySet->setValue(PN_FSM_ERROR,GCFPVString(""));

		itsSplitters = ack.splitter;
		LOG_DEBUG_STR ("rspmask="  << ack.rspmask);
		LOG_DEBUG_STR ("Splitters="  << itsSplitters << ", going to retrieve-clock state");
		TRAN(ClockControl::retrieveClock_state);				// go to next state.
	}
	break;

	case DP_CHANGED:
		_databaseEventHandler(event);
		break;
	
	case CLKCTRL_GET_CLOCK:
	case CLKCTRL_SET_CLOCK:
	case CLKCTRL_GET_SPLITTERS:
	case CLKCTRL_SET_SPLITTERS:
		LOG_INFO_STR("Postponing event " << eventName(event) << " till next state");
		return (GCFEvent::NEXT_STATE);

	default:
		LOG_DEBUG_STR ("retrieveSplitters, default");
		status = defaultMessageHandling(event, port);
		break;
	}    

	return (status);
}


//
// retrieveClock_state(event, port)
//
// Retrieve sampleclock from RSP driver
//
GCFEvent::TResult ClockControl::retrieveClock_state(GCFEvent& event, 
													GCFPortInterface& port)
{
	LOG_DEBUG_STR ("retrieveClock:" << eventName(event) << "@" << port.getName());

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

	case F_ACCEPT_REQ:
		_acceptRequestHandler(port);
	break;

	case RSP_GETCLOCKACK: {
		RSPGetclockackEvent	ack(event);
		if (ack.status != RSP_SUCCESS) {
			LOG_WARN ("Could not retrieve clocksetting of RSPDriver, retry in 2 seconds");
			itsOwnPropertySet->setValue(PN_FSM_ERROR, GCFPVString("getclock failed"));
			itsTimerPort->setTimer(2.0);
			break;
		}
		itsOwnPropertySet->setValue(PN_FSM_ERROR,GCFPVString(""));

		// my clock still uninitialized?
		if (itsClock == 0) {
			LOG_INFO_STR("My clock is still not initialized. StationClock is " << ack.clock << " adopting this value");
			itsClock=ack.clock;
			LOG_DEBUG ("Going to operational state");
			TRAN(ClockControl::active_state);				// go to next state.
			break;
		}

		// is station clock different from my setting?
		if ((int32)ack.clock != itsClock) {
			LOG_INFO_STR ("StationClock is " << ack.clock << ", required clock is " << itsClock << ", changing StationClock");
			LOG_DEBUG ("Going to setClock state");
			TRAN(ClockControl::setClock_state);
		}
		else {
			LOG_INFO_STR ("StationClock is " << ack.clock << ", required clock is " << itsClock << ", no action required");

			LOG_DEBUG ("Going to operational state");
			TRAN(ClockControl::active_state);				// go to next state.
		}
		break;
	}

	case DP_CHANGED:
		_databaseEventHandler(event);
		break;
	
	case CLKCTRL_GET_CLOCK:
	case CLKCTRL_SET_CLOCK:
	case CLKCTRL_GET_SPLITTERS:
	case CLKCTRL_SET_SPLITTERS:
		LOG_INFO_STR("Postponing event " << eventName(event) << " till next state");
		return (GCFEvent::NEXT_STATE);

	default:
		LOG_DEBUG_STR ("retrieveClock, default");
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
GCFEvent::TResult ClockControl::setClock_state(GCFEvent& event, 
													GCFPortInterface& port)
{
	LOG_DEBUG_STR ("setClock:" << eventName(event) << "@" << port.getName());

	GCFEvent::TResult status = GCFEvent::HANDLED;
  
	switch (event.signal) {
	case F_ENTRY:
	case F_TIMER:
		itsOwnPropertySet->setValue(PN_FSM_CURRENT_ACTION,GCFPVString("Set clock"));
		sendClockSetting();				// will result in RSP_SETCLOCKACK;
		break;

	case F_DISCONNECTED:
		_disconnectedHandler(port);		// might result in transition to connect_state
		break;

	case F_ACCEPT_REQ:
		_acceptRequestHandler(port);
	break;

	case RSP_SETCLOCKACK: {
		RSPSetclockackEvent		ack(event);
		if (ack.status != RSP_SUCCESS) {
			LOG_ERROR_STR ("Clock could not be set to " << itsClock << 
															", retry in 5 seconds.");
			itsOwnPropertySet->setValue(PN_FSM_ERROR,GCFPVString("clockset error"));
			itsTimerPort->setTimer(5.0);
			break;
		}
		LOG_INFO_STR ("StationClock is set to " << itsClock << ", going to operational state");
		itsOwnPropertySet->setValue(PN_FSM_ERROR,GCFPVString(""));
		itsOwnPropertySet->setValue(PN_CLC_ACTUAL_CLOCK,GCFPVInteger(itsClock));
		TRAN(ClockControl::active_state);				// go to next state.
		break;
	}

	case DP_CHANGED:
		_databaseEventHandler(event);
		break;
	
	case CLKCTRL_GET_CLOCK:
	case CLKCTRL_SET_CLOCK:
	case CLKCTRL_GET_SPLITTERS:
	case CLKCTRL_SET_SPLITTERS:
		LOG_INFO_STR("Postponing event " << eventName(event) << " till next state");
		return (GCFEvent::NEXT_STATE);

	default:
		LOG_DEBUG_STR ("setClock, default");
		status = defaultMessageHandling(event, port);
		break;
	}    

	return (status);
}


//
// setSplitters_state(event, port)
//
// Set station splitters on or off
//
GCFEvent::TResult ClockControl::setSplitters_state(GCFEvent& event, 
													GCFPortInterface& port)
{
	LOG_DEBUG_STR ("setSplitters:" << eventName(event) << "@" << port.getName());

	GCFEvent::TResult status = GCFEvent::HANDLED;
  
	switch (event.signal) {
    case F_INIT:
   		break;

	case F_ENTRY:
	case F_TIMER:
		itsOwnPropertySet->setValue(PN_FSM_CURRENT_ACTION,GCFPVString("Set splitters"));
		sendSplitterSetting();				// will result in RSP_SETSPLITTERACK;
		break;

	case F_DISCONNECTED:
		_disconnectedHandler(port);		// might result in transition to connect_state
		break;

	case F_ACCEPT_REQ:
		_acceptRequestHandler(port);
	break;

	case RSP_SETSPLITTERACK: {
		RSPSetsplitterackEvent		ack(event);
		if (ack.status != RSP_SUCCESS) {
			LOG_ERROR_STR ("Splitters could not be set to " << (itsSplitterRequest ? "ON" : "OFF") << 
															", retry in 5 seconds.");
			itsOwnPropertySet->setValue(PN_FSM_ERROR,GCFPVString("splitter set error"));
			itsTimerPort->setTimer(5.0);
			break;
		}
		LOG_INFO_STR ("Splitter are set to " << (itsSplitterRequest ? "ON" : "OFF") << ", going to operational state");
		itsOwnPropertySet->setValue(PN_FSM_ERROR,GCFPVString(""));
		// update our admin
		itsSplitters.reset();
		if (itsSplitterRequest) {
			StationConfig			sc;
			for (int i = 0; i < sc.nrRSPs; i++) {
				itsSplitters.set(i);
			}
		}

		TRAN(ClockControl::active_state);				// go to next state.
		break;
	}

	case DP_CHANGED:
		_databaseEventHandler(event);
		break;
	
	case CLKCTRL_GET_CLOCK:
	case CLKCTRL_SET_CLOCK:
	case CLKCTRL_GET_SPLITTERS:
	case CLKCTRL_SET_SPLITTERS:
		LOG_INFO_STR("Postponing event " << eventName(event) << " till next state");
		return (GCFEvent::NEXT_STATE);

	default:
		LOG_DEBUG_STR ("setSplitter, default");
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
GCFEvent::TResult ClockControl::active_state(GCFEvent& event, GCFPortInterface& port)
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
	}
	break;

	case F_ACCEPT_REQ:
		_acceptRequestHandler(port);
	break;

	case F_DISCONNECTED:
		_disconnectedHandler(port);		// might result in transition to connect_state
	break;

	case RSP_UPDCLOCK: {
		RSPUpdclockEvent	updateEvent(event);
		if (updateEvent.status != RSP_SUCCESS || updateEvent.clock == 0) {
			LOG_ERROR_STR ("StationClock has stopped! Going to setClock state to try to solve the problem");
			itsOwnPropertySet->setValue(PN_FSM_ERROR,GCFPVString("Clock stopped"));
			TRAN(ClockControl::setClock_state);
			break;
		}

		if ((int32) updateEvent.clock != itsClock) {
			LOG_ERROR_STR ("CLOCK WAS CHANGED TO " << updateEvent.clock << 
						   " BY SOMEONE WHILE CLOCK SHOULD BE " << itsClock << ". CHANGING CLOCK BACK.");
			itsOwnPropertySet->setValue(PN_FSM_ERROR,GCFPVString("Clock unallowed changed"));
			TRAN (ClockControl::setClock_state);
		}

		// when update.clock==itsClock ignore it, we probable caused it ourselves.
		LOG_DEBUG_STR("Event.clock = " << updateEvent.clock << ", myClock = " << itsClock);
	}
	break;

	case DP_CHANGED:
		_databaseEventHandler(event);
		break;

	case CLKCTRL_GET_CLOCK: {
		CLKCTRLGetClockAckEvent		answer;
		answer.clock = itsClock;
		port.send(answer);
	}
	break;

	case CLKCTRL_SET_CLOCK:	{
		CLKCTRLSetClockEvent		request(event);
		CLKCTRLSetClockAckEvent		response;
		if (request.clock != 160 && request.clock != 200) {
			LOG_DEBUG_STR("Received request to change the clock to invalid value " << request.clock);
			response.status = CLKCTRL_CLOCKFREQ_ERR;
			port.send(response);
			break;
		}
		response.status = CLKCTRL_NO_ERR;
		LOG_INFO_STR("Received request to change the clock to " << request.clock << " MHz.");
		itsClock = request.clock;
		TRAN(ClockControl::setClock_state);
		port.send(response);
	}
	break;

	case CLKCTRL_GET_SPLITTERS: {
		CLKCTRLGetSplittersAckEvent	answer;
		answer.splitters = itsSplitters;
		port.send(answer);
	}
	break;

	case CLKCTRL_SET_SPLITTERS: {
		CLKCTRLSetSplittersEvent		request(event);
		LOG_INFO_STR("Received request to switch the splitters " << (request.splittersOn ? "ON" : "OFF"));
		itsSplitterRequest = request.splittersOn;
		TRAN (ClockControl::setSplitters_state);
		LOG_INFO("@@@@@@@@@@@@@@@");
		CLKCTRLSetSplittersAckEvent		response;
		response.status = CLKCTRL_NO_ERR;
		port.send(response);
	}
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
void ClockControl::_disconnectedHandler(GCFPortInterface& port)
{
	port.close();
	if (&port == itsRSPDriver) {
		LOG_DEBUG("Connection with RSPDriver failed, going to reconnect state");
		itsOwnPropertySet->setValue(PN_FSM_ERROR,GCFPVString("connection lost"));
		TRAN (ClockControl::connect2RSP_state);
	}
	else {
		itsClientList.remove(&port);
	}
}

//
// _acceptRequestHandler(port)
//
void ClockControl::_acceptRequestHandler(GCFPortInterface& /*port*/)
{
	GCFTCPPort*	client = new GCFTCPPort();
	client->init(*this, "client", GCFPortInterface::SPP, CLOCK_PROTOCOL);
	if (!itsCommandPort->accept(*client)) {
		delete client;
	}
	else {
		itsClientList.push_back(client);
		LOG_INFO("New client connected");
	}
}

//
// requestSubscription()
//
void ClockControl::requestSubscription()
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
void ClockControl::cancelSubscription()
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
void ClockControl::requestClockSetting()
{
	LOG_INFO ("Asking RSPdriver current clock setting");

	RSPGetclockEvent		msg;
	msg.timestamp = RTC::Timestamp(0,0);
	msg.cache = 1;
	itsRSPDriver->send(msg);
}


//
// sendClockSetting()
//
void ClockControl::sendClockSetting()
{
	LOG_INFO_STR ("Setting stationClock to " << itsClock << " MHz");

	RSPSetclockEvent		msg;
	msg.timestamp = RTC::Timestamp(0,0);
	msg.clock = itsClock;
	itsRSPDriver->send(msg);
}

//
// requestSplitterSetting()
//
void ClockControl::requestSplitterSetting()
{
	LOG_INFO ("Asking RSPdriver current splitter setting");

	StationConfig			sc;
	RSPGetsplitterEvent		msg;
	msg.timestamp = RTC::Timestamp(0,0);
	msg.cache = 1;
	msg.rspmask.reset();
	for (int i = 0; i < sc.nrRSPs; i++) {
		msg.rspmask.set(i);
	}
	itsRSPDriver->send(msg);
}


//
// sendSplitterSetting()
//
void ClockControl::sendSplitterSetting()
{
	LOG_INFO_STR ("Setting stationSplitters to " << (itsSplitterRequest ? "ON" : "OFF"));

	StationConfig			sc;
	RSPSetsplitterEvent		msg;
	msg.timestamp = RTC::Timestamp(0,0);
	msg.switch_on = itsSplitterRequest;
	msg.rspmask.reset();
	for (int i = 0; i < sc.nrRSPs; i++) {
		msg.rspmask.set(i);
	}
	itsRSPDriver->send(msg);
}
//
// defaultMessageHandling
//
GCFEvent::TResult ClockControl::defaultMessageHandling(GCFEvent& 		event, 
													  GCFPortInterface& /*port*/)
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
		case F_INIT:
		case F_EXIT:
		case F_ENTRY:
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
GCFEvent::TResult ClockControl::finishing_state(GCFEvent& event, GCFPortInterface& port)
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
		itsOwnPropertySet->setValue(PN_CLC_CONNECTED,   GCFPVBool  (false));

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


}; // StationCU
}; // LOFAR
