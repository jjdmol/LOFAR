//#  ClockControl.cc: Implementation of ClockController of the station.
//#
//#  Copyright (C) 2006-2009
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, softwaresupport@astron.nl
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
#include <ApplCommon/StationInfo.h>
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
#include <StationCU/Package__Version.h>

using namespace LOFAR::GCF::TM;
using namespace LOFAR::GCF::PVSS;
using namespace LOFAR::GCF::RTDB;
using namespace std;

namespace LOFAR {
	using namespace APLCommon;
	using namespace Controller_Protocol;
	using namespace RSP_Protocol;
	using namespace DP_Protocol;
	using namespace Clock_Protocol;
	namespace StationCU {
	

static string bitmodeVersionString(uint16 version)
{
  switch(version) {
    case 0:  return "16";
    case 1:  return "16/8";
    case 2:  return "16/8/4";
    default: return "??";
  }
}

static bool bitmodeSupported(unsigned bitmode, uint16 version)
{
  if (bitmode == 16)
    return true;

  if (bitmode == 8)
    return version >= 1;

  if (bitmode == 4)
    return version >= 2;

  return false;
}

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
    itsLastCommandClient(0),
    itsNrRSPs           (0),

    // we need default values to push in case the boards are set to 0
	itsClock			(200),
	itsBitmode			(16),
	itsBitmodeVersion	(0)
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

	// open port for get/set clock/splitters
	itsCommandPort = new GCFTCPPort (*this, MAC_SVCMASK_CLOCKCTRL, 
										GCFPortInterface::MSPP, CLOCK_PROTOCOL);
	ASSERTSTR(itsCommandPort, "Cannot allocate listener for clock commands");

	// for debugging purposes
	registerProtocol (CONTROLLER_PROTOCOL,	CONTROLLER_PROTOCOL_STRINGS);
	registerProtocol (DP_PROTOCOL,			DP_PROTOCOL_STRINGS);
	registerProtocol (RSP_PROTOCOL,			RSP_PROTOCOL_STRINGS);
	registerProtocol (CLOCK_PROTOCOL,		CLOCK_PROTOCOL_STRINGS);

    StationConfig sc;

    itsNrRSPs = sc.nrRSPs;
}


//
// ~ClockControl()
//
ClockControl::~ClockControl()
{
	LOG_TRACE_OBJ_STR (getName() << " destruction");

	cancelClockSubscription();	// tell RSPdriver to stop sending updates.
	cancelBitmodeSubscription();	// tell RSPdriver to stop sending updates.
	cancelSplitterSubscription();	// tell RSPdriver to stop sending updates.

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
// Call ONLY from the active_state, since the code here triggers
// transactions that will always lead to active_state.
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
			uint32			newClock = clockObj->getValue();
			if (newClock != itsClock) {
				itsClock = newClock;
				LOG_DEBUG_STR("Received clock change from PVSS, clock is now " << itsClock);
				TRAN(ClockControl::setClock_state);
				// sendClockSetting();
			}
			break;
		}

		if (strstr(dpEvent.DPname.c_str(), PN_CLC_REQUESTED_BITMODE) != 0) {
			GCFPVInteger*	bitmodeObj = (GCFPVInteger*)dpEvent.value._pValue;
			int32			newBitmode = bitmodeObj->getValue();
			if (newBitmode != itsBitmode) {
				itsBitmode = newBitmode;
				LOG_DEBUG_STR("Received bitmode change from PVSS, bitmode is now " << itsBitmode);
				TRAN(ClockControl::setBitmode_state);
				// sendBitmodeSetting();
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
		LOG_INFO_STR ("Activating PropertySet " << myPropSetName);
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

		if (dpEvent.DPname.find(propSetName + "." PN_CLC_REQUESTED_CLOCK) != string::npos) {
			GCFPVInteger	clockVal;
			itsOwnPropertySet->getValue(PN_CLC_REQUESTED_CLOCK, clockVal);
			itsClock = clockVal.getValue();
			LOG_INFO_STR("Requested clock is " << itsClock);
        }

		if (dpEvent.DPname.find(propSetName + "." PN_CLC_REQUESTED_BITMODE) != string::npos) {
			GCFPVInteger	bitmodeVal;
			itsOwnPropertySet->getValue(PN_CLC_REQUESTED_BITMODE, bitmodeVal);
			itsBitmode = bitmodeVal.getValue();
			LOG_INFO_STR("Requested bitmode is " << itsBitmode);
		}

		LOG_DEBUG ("Going to connect2RSP state");
		TRAN(ClockControl::connect2RSP_state);			// go to next state.
        break;
	}
	break;

	case F_CONNECTED:
		break;

	case F_ACCEPT_REQ:
		_acceptRequestHandler(port);
	break;

	case DP_CHANGED:
		LOG_INFO_STR("Postponing event " << eventName(event) << " till next state");
		return (GCFEvent::NEXT_STATE);
	
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
		itsClockSubscription = 0;
		itsSplitterSubscription = 0;
        itsBitmodeSubscription = 0;
		itsRSPDriver->open();		// will result in F_CONN or F_DISCONN
		break;

	case F_CONNECTED:
		if (&port == itsRSPDriver) {
			itsTimerPort->cancelAllTimers();
			LOG_DEBUG ("Connected with RSPDriver, starting listener for commands");
			itsOwnPropertySet->setValue(PN_FSM_ERROR, GCFPVString(""));
			itsOwnPropertySet->setValue(PN_CLC_CONNECTED,  GCFPVBool(true));
			requestClockSetting();		// ask value of clock: will result in RSP_GETCLOCKACK
		}
		break;

	case RSP_GETCLOCKACK: {
		RSPGetclockackEvent		ack(event);
		if (ack.status != RSP_SUCCESS) {
			LOG_ERROR ("Clock could not be get. Ignoring that for now.");
		}
		else {
			itsClock = ack.clock;
			LOG_INFO_STR("RSP says clock is " << itsClock << "MHz. Adopting that value.");
			itsOwnPropertySet->setValue(PN_CLC_ACTUAL_CLOCK,GCFPVInteger(itsClock));
			// Note: only here I am allowed to change the value of the requested clock. Normally
			//       the stationController is the owner of this value.
			itsOwnPropertySet->setValue(PN_CLC_REQUESTED_CLOCK,GCFPVInteger(itsClock));
		}

		requestBitmodeSetting();		// ask value of bitmode: will result in RSP_GETBITMODEACK
	}
	break;

	case RSP_GETBITMODEACK: {
		RSPGetbitmodeackEvent		ack(event);
		if (ack.status != RSP_SUCCESS) {
			LOG_ERROR ("Bitmode could not be get. Ignoring that for now.");
		}
		else {
            bool success = true;

            for (unsigned i = 0; i < itsNrRSPs; i++) {
              if (ack.bits_per_sample[i] != ack.bits_per_sample[0]) {
                LOG_ERROR_STR("Mixed bit modes not supported: RSP board " << i << " is in " << ack.bits_per_sample[i] << " bit mode, but board 0 is in " << ack.bits_per_sample[0] << " bit mode");
                success = false;
                break;
              } 

              if (ack.bitmode_version[i] != ack.bitmode_version[0]) {
                LOG_ERROR_STR("Mixed bit mode support not supported: RSP board " << i << " supports modes " << bitmodeVersionString(ack.bitmode_version[i]) << ", but board 0 supports modes " << bitmodeVersionString(ack.bitmode_version[0]));
                success = false;
                break;
              } 
            }

            if (success) {
			    itsBitmode = ack.bits_per_sample[0];
				itsBitmodeVersion = ack.bitmode_version[0];

			    LOG_INFO_STR("RSP says bitmode is " << itsBitmode << " bits, and supports modes " << bitmodeVersionString(itsBitmodeVersion) << ". Adopting those values.");
			    itsOwnPropertySet->setValue(PN_CLC_ACTUAL_BITMODE,GCFPVInteger(itsBitmode));
			    // Note: only here I am allowed to change the value of the requested bitmode. Normally
			    //       the stationController is the owner of this value.
			    itsOwnPropertySet->setValue(PN_CLC_REQUESTED_BITMODE,GCFPVInteger(itsBitmode));
            }
		}

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
		LOG_INFO_STR("Postponing event " << eventName(event) << " till next state");
		return (GCFEvent::NEXT_STATE);
	
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
			TRAN(ClockControl::subscribeSplitter_state);        // go to next state.
		}
		itsOwnPropertySet->setValue(PN_FSM_CURRENT_ACTION, GCFPVString("Opening command-port"));
		itsCommandPort->autoOpen(0, 10, 2);		// will result in F_CONN or F_DISCONN
		break;

	case F_CONNECTED:
		if (&port == itsCommandPort) {
			LOG_DEBUG ("Command port opened, taking subscription on the splitter");
			itsOwnPropertySet->setValue(PN_FSM_ERROR, GCFPVString(""));

			// let Parent task register itself at the startDaemon. 
			// This will result in a CONTROL_CONNECT event from the parenttask (which we ignore) 
			// and probably CLKCTRL_XXX messages from the StationController 
			itsParentPort = itsParentControl->registerTask(this);	// PC reports itself at the startDaemon

			TRAN(ClockControl::subscribeSplitter_state);		// go to next state.
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
		LOG_INFO_STR("Postponing event " << eventName(event) << " till next state");
		return (GCFEvent::NEXT_STATE);
	
	default:
		LOG_DEBUG_STR ("startListener, default");
		status = defaultMessageHandling(event, port);
		break;
	}    

	return (status);
}

//
// subscribeSplitter_state(event, port)
//
// Take subscription on splitter modifications
//
GCFEvent::TResult ClockControl::subscribeSplitter_state(GCFEvent& event, 
													GCFPortInterface& port)
{
	LOG_DEBUG_STR ("subscribeSplitter:" << eventName(event) << "@" << port.getName());

	GCFEvent::TResult status = GCFEvent::HANDLED;
  
	switch (event.signal) {
    case F_EXIT:
   		break;

	case F_ENTRY:
	case F_TIMER:
		itsOwnPropertySet->setValue(PN_FSM_CURRENT_ACTION,GCFPVString("Subscribe to splitters"));
		requestSplitterSubscription();		// will result in RSP_SUBSPLITTERACK;
		break;

	case F_DISCONNECTED:
		_disconnectedHandler(port);		// might result in transition to connect_state
		break;

	case F_ACCEPT_REQ:
		_acceptRequestHandler(port);
	break;

	case RSP_SUBSPLITTERACK: {
		RSPSubsplitterackEvent	ack(event);
		if (ack.status != RSP_SUCCESS) {
			LOG_WARN ("Could not get subscribtion on splitter, retry in 2 seconds." );
			itsOwnPropertySet->setValue(PN_FSM_ERROR, GCFPVString("subscribe failed"));
			itsTimerPort->setTimer(2.0);
			break;
		}
		itsSplitterSubscription = ack.handle;
		LOG_DEBUG ("Subscription on splitters successful, waiting actual value");
	}
	break;

	case RSP_UPDSPLITTER: {
		RSPUpdsplitterEvent		update(event);
		itsOwnPropertySet->setValue(PN_FSM_ERROR,GCFPVString(""));
		itsSplitters = update.splitter;
		itsSplitterRequest = itsSplitters[0] ? true : false;
		LOG_INFO_STR("State of the splitters = " << update.splitter << ". Taking subscription on the clock");
		TRAN(ClockControl::subscribeClock_state);				// go to next state.
		}
		break;

	case DP_CHANGED:
	case CLKCTRL_GET_CLOCK:
	case CLKCTRL_SET_CLOCK:
	case CLKCTRL_GET_BITMODE:
	case CLKCTRL_SET_BITMODE:
	case CLKCTRL_GET_SPLITTERS:
	case CLKCTRL_SET_SPLITTERS:
	case RSP_UPDCLOCK:
    case RSP_UPDBITMODE:
		LOG_INFO_STR("Postponing event " << eventName(event) << " till next state");
		return (GCFEvent::NEXT_STATE);

	default:
		LOG_DEBUG_STR ("subscribeSplitter, default");
		status = defaultMessageHandling(event, port);
		break;
	}    

	return (status);
}

//
// subscribeClock_state(event, port)
//
// Take subscription on clock modifications
//
GCFEvent::TResult ClockControl::subscribeClock_state(GCFEvent& event, 
													GCFPortInterface& port)
{
	LOG_DEBUG_STR ("subscribeClock:" << eventName(event) << "@" << port.getName());

	GCFEvent::TResult status = GCFEvent::HANDLED;
  
	switch (event.signal) {
    case F_EXIT:
   		break;

	case F_ENTRY:
	case F_TIMER:
		itsOwnPropertySet->setValue(PN_FSM_CURRENT_ACTION,GCFPVString("Subscribe to clock"));
		requestClockSubscription();		// will result in RSP_SUBCLOCKACK;
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
			LOG_WARN ("Could not get subscription on clock, retry in 2 seconds.");
			itsOwnPropertySet->setValue(PN_FSM_ERROR, GCFPVString("subscribe failed"));
			itsTimerPort->setTimer(2.0);
			break;
		}
		itsClockSubscription = ack.handle;
		LOG_INFO("Subscription on the clock successful. Taking subscription on the clock.");
		itsOwnPropertySet->setValue(PN_CLC_ACTUAL_CLOCK,GCFPVInteger(itsClock));
		TRAN(ClockControl::subscribeBitmode_state);				// go to next state.
	}
	break;
	
	case DP_CHANGED:
	case CLKCTRL_GET_CLOCK:
	case CLKCTRL_SET_CLOCK:
	case CLKCTRL_GET_BITMODE:
	case CLKCTRL_SET_BITMODE:
	case CLKCTRL_GET_SPLITTERS:
	case CLKCTRL_SET_SPLITTERS:
	case RSP_UPDSPLITTER:
	case RSP_UPDCLOCK:
    case RSP_UPDBITMODE:
		LOG_INFO_STR("Postponing event " << eventName(event) << " till next state");
		return (GCFEvent::NEXT_STATE);

	default:
		LOG_DEBUG_STR ("subscribeClock, default");
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
            if (ack.status == RSP_BUSY) {
                LOG_WARN_STR ("Clock could not be set to " << itsClock << ", busy retry in 5 seconds.");
            }
			else {
                LOG_ERROR_STR ("Clock could not be set to " << itsClock << ", retry in 5 seconds.");
            }
            itsOwnPropertySet->setValue(PN_FSM_ERROR,GCFPVString("clockset error"));
			itsTimerPort->setTimer(5.0);
			break;
		}
		LOG_INFO_STR ("StationClock is set to " << itsClock << ", wait for update");
        itsOwnPropertySet->setValue(PN_FSM_ERROR,GCFPVString(""));
		itsOwnPropertySet->setValue(PN_CLC_ACTUAL_CLOCK,GCFPVInteger(itsClock));
		break;
	}

    case RSP_UPDCLOCK: {
	    if (itsLastCommandClient) {
		    CLKCTRLSetClockAckEvent response;
		    response.status = CLKCTRL_NO_ERR;
		    itsLastCommandClient->send(response);
            itsLastCommandClient = 0;
        	LOG_DEBUG("Informed client of clock update");
	    }

        LOG_INFO_STR ("Received clock update, going to operational state");
        TRAN(ClockControl::active_state);				// go to next state.
        return (GCFEvent::NEXT_STATE);
    }

	case DP_CHANGED:
	case CLKCTRL_GET_CLOCK:
	case CLKCTRL_SET_CLOCK:
	case CLKCTRL_GET_BITMODE:
	case CLKCTRL_SET_BITMODE:
	case CLKCTRL_GET_SPLITTERS:
	case CLKCTRL_SET_SPLITTERS:
	case RSP_UPDSPLITTER:
    case RSP_UPDBITMODE:
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
// subscribeBitmode_state(event, port)
//
// Take subscription on bitmode modifications
//
GCFEvent::TResult ClockControl::subscribeBitmode_state(GCFEvent& event, 
													GCFPortInterface& port)
{
	LOG_DEBUG_STR ("subscribeBitmode:" << eventName(event) << "@" << port.getName());

	GCFEvent::TResult status = GCFEvent::HANDLED;
  
	switch (event.signal) {
    case F_EXIT:
   		break;

	case F_ENTRY:
	case F_TIMER:
		itsOwnPropertySet->setValue(PN_FSM_CURRENT_ACTION,GCFPVString("Subscribe to bitmode"));
		requestBitmodeSubscription();		// will result in RSP_SUBBITMODEACK;
		break;

	case F_DISCONNECTED:
		_disconnectedHandler(port);		// might result in transition to connect_state
		break;

	case F_ACCEPT_REQ:
		_acceptRequestHandler(port);
	break;

	case RSP_SUBBITMODEACK: {
		RSPSubbitmodeackEvent	ack(event);
		if (ack.status != RSP_SUCCESS) {
			LOG_WARN ("Could not get subscription on bitmode, retry in 2 seconds.");
			itsOwnPropertySet->setValue(PN_FSM_ERROR, GCFPVString("subscribe failed"));
			itsTimerPort->setTimer(2.0);
			break;
		}
		itsBitmodeSubscription = ack.handle;
		LOG_INFO("Subscription on the bitmode successful. going to operational mode");
		itsOwnPropertySet->setValue(PN_CLC_ACTUAL_BITMODE,GCFPVInteger(itsBitmode));
		TRAN(ClockControl::active_state);				// go to next state.
	}
	break;
	
	case DP_CHANGED:
	case CLKCTRL_GET_CLOCK:
	case CLKCTRL_SET_CLOCK:
	case CLKCTRL_GET_BITMODE:
	case CLKCTRL_SET_BITMODE:
	case CLKCTRL_GET_SPLITTERS:
	case CLKCTRL_SET_SPLITTERS:
	case RSP_UPDCLOCK:
    case RSP_UPDSPLITTER:
	case RSP_UPDBITMODE:
		LOG_INFO_STR("Postponing event " << eventName(event) << " till next state");
		return (GCFEvent::NEXT_STATE);

	default:
		LOG_DEBUG_STR ("subscribeBitmode, default");
		status = defaultMessageHandling(event, port);
		break;
	}    

	return (status);
}


//
// setBitmode_state(event, port)
//
// Set samplebitmode from RSP driver
//
GCFEvent::TResult ClockControl::setBitmode_state(GCFEvent& event, 
													GCFPortInterface& port)
{
	LOG_DEBUG_STR ("setBitmode:" << eventName(event) << "@" << port.getName());

	GCFEvent::TResult status = GCFEvent::HANDLED;
  
	switch (event.signal) {
	case F_ENTRY:
        itsTimerPort->setTimer(1.0);
        break;
        
	case F_TIMER:
		itsOwnPropertySet->setValue(PN_FSM_CURRENT_ACTION,GCFPVString("Set bitmode"));
		sendBitmodeSetting();				// will result in RSP_SETBITMODEACK;
		break;

	case F_DISCONNECTED:
		_disconnectedHandler(port);		// might result in transition to connect_state
		break;

	case F_ACCEPT_REQ:
		_acceptRequestHandler(port);
	break;

	case RSP_SETBITMODEACK: {
		RSPSetbitmodeackEvent		ack(event);
		if (ack.status != RSP_SUCCESS) {
			LOG_ERROR_STR ("Bitmode could not be set to " << itsBitmode << ", retry in 2 seconds.");
			itsOwnPropertySet->setValue(PN_FSM_ERROR,GCFPVString("bitmodeset error"));
			itsTimerPort->setTimer(2.0);
			break;
		}

		LOG_INFO_STR ("StationBitmode is set to " << itsBitmode << ", going to operational state");
		itsOwnPropertySet->setValue(PN_FSM_ERROR,GCFPVString(""));
		itsOwnPropertySet->setValue(PN_CLC_ACTUAL_BITMODE,GCFPVInteger(itsBitmode));

		// Do NOT wait for RSP_UPDBITMODE: Updating the bit mode is instantaneous,
        // and does not trigger RSP_UPDBITMODE if the bit mode happened to be in the
        // right state already.

	    if (itsLastCommandClient) {
		    CLKCTRLSetBitmodeAckEvent	response;
		    response.status = CLKCTRL_NO_ERR;
		    itsLastCommandClient->send(response);
            itsLastCommandClient = 0;
        	LOG_DEBUG("Informed client of bitmode update");
	    }

        LOG_INFO_STR ("Received bitmode update, going to operational state");

		TRAN(ClockControl::active_state);				// go to next state.
		return (GCFEvent::NEXT_STATE);
    }

	case DP_CHANGED:
	case CLKCTRL_GET_CLOCK:
	case CLKCTRL_SET_CLOCK:
	case CLKCTRL_GET_BITMODE:
	case CLKCTRL_SET_BITMODE:
	case CLKCTRL_GET_SPLITTERS:
	case CLKCTRL_SET_SPLITTERS:
    case RSP_UPDBITMODE:
	case RSP_UPDCLOCK:
	case RSP_UPDSPLITTER:
		LOG_INFO_STR("Postponing event " << eventName(event) << " till next state");
		return (GCFEvent::NEXT_STATE);

	default:
		LOG_DEBUG_STR ("setBitmode, default");
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
		sendSplitterSetting();				// will result in RSP_SETSPLITTERACK and RSP_UPDSPLITTER
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
						", retry in 2 seconds.");
			itsOwnPropertySet->setValue(PN_FSM_ERROR,GCFPVString("splitter set error"));
			itsTimerPort->setTimer(2.0);
			break;
		}

		LOG_INFO_STR ("Splitter are set to " << (itsSplitterRequest ? "ON" : "OFF"));
		itsOwnPropertySet->setValue(PN_FSM_ERROR,GCFPVString(""));
		// update our admin
		itsSplitters.reset();
		if (itsSplitterRequest) {
			for (unsigned i = 0; i < itsNrRSPs; i++) {
				itsSplitters.set(i);
			}
		}

		// Do NOT wait for RSP_UDPSPLITTER: Updating the splitter is instantaneous,
        // and does not trigger RSP_UDPSPLITTER if the splitters happened to be in the
        // right state already.

	    if (itsLastCommandClient) {
            CLKCTRLSetSplittersAckEvent	response;
            response.status = CLKCTRL_NO_ERR;
            itsLastCommandClient->send(response);
            itsLastCommandClient = 0;
        	LOG_DEBUG("Informed client of splitter update");
        }

        LOG_INFO_STR ("Received splitter update, going to operational state");

		TRAN(ClockControl::active_state);
		break;
	}

	case DP_CHANGED:
	case CLKCTRL_GET_CLOCK:
	case CLKCTRL_SET_CLOCK:
	case CLKCTRL_GET_BITMODE:
	case CLKCTRL_SET_BITMODE:
	case CLKCTRL_GET_SPLITTERS:
	case CLKCTRL_SET_SPLITTERS:
	case RSP_UPDCLOCK:
    case RSP_UPDBITMODE:
	case RSP_UPDSPLITTER:
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
		if (updateEvent.status != RSP_SUCCESS) {
			LOG_WARN ("Received and INVALID clock update, WHAT IS THE CLOCK?");
			itsOwnPropertySet->setValue(PN_FSM_ERROR, GCFPVString("getclock failed"));
			break;
		}

		if (updateEvent.clock == 0) {
			LOG_ERROR_STR ("StationClock has stopped! Going to setClock state to try to solve the problem");
			itsOwnPropertySet->setValue(PN_FSM_ERROR,GCFPVString("Clock stopped"));
			TRAN(ClockControl::setClock_state);
			break;
		}

		if (itsClock == 0) { // my clock still uninitialized?
			LOG_INFO_STR("My clock is still not initialized. StationClock is " << updateEvent.clock << " adopting this value");
			itsClock=updateEvent.clock;
			break;
		}

		if (updateEvent.clock != itsClock) {
			LOG_ERROR_STR ("CLOCK WAS CHANGED TO " << updateEvent.clock << 
						   " BY SOMEONE WHILE CLOCK SHOULD BE " << itsClock << ". CHANGING CLOCK BACK.");
			itsOwnPropertySet->setValue(PN_FSM_ERROR,GCFPVString("Clock unallowed changed"));
			TRAN (ClockControl::setClock_state);
			break;
		}

		// when update.clock==itsClock ignore it, we probable caused it ourselves.
		LOG_DEBUG_STR("Event.clock = " << updateEvent.clock << ", myClock = " << itsClock);
	}
	break;

	case RSP_UPDBITMODE: {
		RSPUpdbitmodeEvent	updateEvent(event);

        // was the update even succesful?
		if (updateEvent.status != RSP_SUCCESS) {
			LOG_WARN ("Received an INVALID bitmode update, WHAT IS THE BITMODE?");
			itsOwnPropertySet->setValue(PN_FSM_ERROR, GCFPVString("getbitmode failed"));
			break;
		}
  
        bool retry = false;
        for (unsigned i = 0; i < itsNrRSPs; i++) {
            // 0 bits indicates the bit mode could not be set
            if (updateEvent.bits_per_sample[i] == 0) {
			    LOG_ERROR_STR ("StationBitmode has stopped on board " << i << " (and possibly others)! Going to setBitmode state to try to solve the problem");
			    itsOwnPropertySet->setValue(PN_FSM_ERROR,GCFPVString("Bitmode stopped"));

                retry = true;
                break;
            }

            // we don't allow a mix of bit modes from the boards
            if (updateEvent.bits_per_sample[i] != updateEvent.bits_per_sample[0]) {
                LOG_ERROR_STR("Mixed bit modes not supported: RSP board " << i << " is in " << updateEvent.bits_per_sample[i] << " bit mode, but board 0 is in " << updateEvent.bits_per_sample[0] << " bit mode, going to setBitmode state to try to solve the problem");
			    itsOwnPropertySet->setValue(PN_FSM_ERROR,GCFPVString("boards report mixed bit modes"));

                retry = true;
                break;
            } 
        }

        if (retry) {
			TRAN(ClockControl::setBitmode_state);
            break;
        }

        // because we don't allow mixed bit modes, we can simply use the first one
        uint16 bitmode = updateEvent.bits_per_sample[0];

		if (itsBitmode == 0) { // my bitmode still uninitialized?
			LOG_INFO_STR("My bitmode is still not initialized. StationBitmode is " << bitmode << " adopting this value");
			itsBitmode = bitmode;
			break;
		} else if (bitmode != itsBitmode) {
			LOG_ERROR_STR ("BITMODE WAS CHANGED TO " << bitmode <<
						   " BY SOMEONE WHILE BITMODE SHOULD BE " << itsBitmode << ". CHANGING BITMODE BACK.");
			itsOwnPropertySet->setValue(PN_FSM_ERROR,GCFPVString("Bitmode unallowed changed"));

			TRAN (ClockControl::setBitmode_state);
			break;
		} else {
		    // when update.bits_per_sample==itsBitmode ignore it, we probable caused it ourselves.
		    LOG_DEBUG_STR("Event.bits_per_sample[0..n] = " << bitmode << ", myBitmode = " << itsBitmode);
        }    
	}
	break;

	case RSP_UPDSPLITTER: {
		RSPUpdsplitterEvent		update(event);
		itsOwnPropertySet->setValue(PN_FSM_ERROR,GCFPVString(""));
		if (update.splitter[0] != itsSplitters[0]) {
			LOG_ERROR_STR ("SPLITTER WAS CHANGED TO " << (itsSplitters[0] ? "OFF" : "ON") << 
						   " BY SOMEONE WHILE SPLITTER SHOULD BE " <<
							(itsSplitters[0] ? "ON" : "OFF") << ". CHANGING SPLITTER BACK.");
			itsOwnPropertySet->setValue(PN_FSM_ERROR,GCFPVString("Splitter unallowed changed"));
			TRAN (ClockControl::setSplitters_state);
			break;
		}
		itsSplitters = update.splitter;
		LOG_DEBUG_STR("State of the splitters = " << itsSplitters);
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
			LOG_ERROR_STR("Received request to change the clock to invalid value " << request.clock);

			response.status = CLKCTRL_CLOCKFREQ_ERR;
            port.send(response);
		} else {
		    LOG_INFO_STR("Received request to change the clock to " << request.clock << " MHz.");
		    itsOwnPropertySet->setValue(PN_CLC_REQUESTED_CLOCK,GCFPVInteger(request.clock));

            if (itsClock == request.clock) {
		        LOG_INFO_STR("Clock was already set to " << itsClock << ".");

		        response.status = CLKCTRL_NO_ERR;
                port.send(response);
            } else {
		        itsClock = request.clock;
		        TRAN(ClockControl::setClock_state);
                itsLastCommandClient = &port;
            }
        }
		// port.send(response);
	}
	break;

	case CLKCTRL_GET_BITMODE: {
		CLKCTRLGetBitmodeAckEvent		answer;
		answer.bits_per_sample = itsBitmode;
		port.send(answer);
	}
	break;

	case CLKCTRL_SET_BITMODE:	{
		CLKCTRLSetBitmodeEvent		request(event);
	    CLKCTRLSetBitmodeAckEvent	response;

		if (request.bits_per_sample != 16 && request.bits_per_sample != 8 && request.bits_per_sample != 4) {
			LOG_ERROR_STR("Received request to change the bitmode to invalid value " << request.bits_per_sample);

			response.status = CLKCTRL_INVALIDBITMODE_ERR;
		    port.send(response);
		} else if (!bitmodeSupported(request.bits_per_sample, itsBitmodeVersion)) {
			LOG_ERROR_STR("Received request to change the bitmode to unsupported value " << request.bits_per_sample << " (supported is " << bitmodeVersionString(itsBitmodeVersion) << ")");
			response.status = CLKCTRL_INVALIDBITMODE_ERR;
		    port.send(response);
        } else {
		    LOG_INFO_STR("Received request to change the bitmode to " << request.bits_per_sample << " bit.");
		    itsOwnPropertySet->setValue(PN_CLC_REQUESTED_BITMODE,GCFPVInteger(request.bits_per_sample));

            if (itsBitmode == request.bits_per_sample) {
		        LOG_INFO_STR("Bitmode was already set to " << itsBitmode << ".");

                response.status = CLKCTRL_NO_ERR;
                port.send(response);
            } else {
		        itsBitmode = request.bits_per_sample;
		        TRAN(ClockControl::setBitmode_state);
                itsLastCommandClient = &port;
            }
        }
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
        itsLastCommandClient = &port;
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
		if (&port == itsLastCommandClient) {
			itsLastCommandClient = 0;
		}

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
// requestClockSubscription()
//
void ClockControl::requestClockSubscription()
{
	LOG_INFO ("Taking subscription on clock settings");

	RSPSubclockEvent		msg;
//	msg.timestamp = 0;
	msg.period = 1;				// let RSPdriver check every second
	itsRSPDriver->send(msg);
}

//
// cancelClockSubscription()
//
void ClockControl::cancelClockSubscription()
{
	LOG_INFO ("Canceling subscription on clock settings");

	RSPUnsubclockEvent		msg;
	msg.handle = itsClockSubscription;
	itsClockSubscription = 0;
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
// requestBitmodeSubscription()
//
void ClockControl::requestBitmodeSubscription()
{
	LOG_INFO ("Taking subscription on bitmode settings");

	RSPSubbitmodeEvent		msg;
//	msg.timestamp = 0;
	msg.period = 1;				// let RSPdriver check every second
	itsRSPDriver->send(msg);
}

//
// cancelBitmodeSubscription()
//
void ClockControl::cancelBitmodeSubscription()
{
	LOG_INFO ("Canceling subscription on bitmode settings");

	RSPUnsubbitmodeEvent		msg;
	msg.handle = itsBitmodeSubscription;
	itsBitmodeSubscription = 0;
	itsRSPDriver->send(msg);
}

//
// requestBitmodeSetting()
//
void ClockControl::requestBitmodeSetting()
{
	LOG_INFO ("Asking RSPdriver current bitmode setting");

	RSPGetbitmodeEvent		msg;
	msg.timestamp = RTC::Timestamp(0,0);
	msg.cache = 1;
	itsRSPDriver->send(msg);
}


//
// sendBitmodeSetting()
//
void ClockControl::sendBitmodeSetting()
{
	LOG_INFO_STR ("Setting stationBitmode to " << itsBitmode << " bit");

	RSPSetbitmodeEvent		msg;
    bitset<MAX_N_RSPBOARDS> mask;

    // select all RSP boards
    for (unsigned i = 0; i < itsNrRSPs; i++)
      mask.set(i);

	msg.timestamp = RTC::Timestamp(0,0);
    msg.rspmask   = mask;
	msg.bits_per_sample = itsBitmode;
	itsRSPDriver->send(msg);
}

//
// requestSplitterSubscription()
//
void ClockControl::requestSplitterSubscription()
{
	LOG_INFO ("Taking subscription on splitter settings");

	RSPSubsplitterEvent		msg;
	msg.period = 1;				// let RSPdriver check every second
	itsRSPDriver->send(msg);
}

//
// cancelSplitterSubscription()
//
void ClockControl::cancelSplitterSubscription()
{
	LOG_INFO ("Canceling subscription on splitter settings");

	RSPUnsubsplitterEvent		msg;
	msg.handle = itsSplitterSubscription;
	itsSplitterSubscription = 0;
	itsRSPDriver->send(msg);
}

//
// requestSplitterSetting()
//
void ClockControl::requestSplitterSetting()
{
	LOG_INFO ("Asking RSPdriver current splitter setting");

	RSPGetsplitterEvent		msg;
	msg.timestamp = RTC::Timestamp(0,0);
	itsRSPDriver->send(msg);
}


//
// sendSplitterSetting()
//
void ClockControl::sendSplitterSetting()
{
	LOG_INFO_STR ("Setting stationSplitters to " << (itsSplitterRequest ? "ON" : "OFF"));

	RSPSetsplitterEvent		msg;
	msg.timestamp = RTC::Timestamp(0,0);
	msg.switch_on = itsSplitterRequest;
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
			answer.result = true;	// !!!! ??? TODO
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
		case F_CONNECTED:
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
