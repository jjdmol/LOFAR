//#  StationControl.cc: Implementation of the StationControl task
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
//#
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

#include "StationControl.h"
#include "StationControlDefines.h"

using namespace LOFAR::GCF::Common;
using namespace LOFAR::GCF::TM;
using namespace LOFAR::GCF::PAL;
using namespace std;

namespace LOFAR {
	using namespace APLCommon;
	using namespace ACC::APS;
	namespace StationCU {
	
//
// StationControl()
//
StationControl::StationControl(const string&	cntlrName) :
	GCFTask 			((State)&StationControl::initial_state,cntlrName),
	PropertySetAnswerHandlerInterface(),
	itsPropertySetAnswer(*this),
	itsOwnPropertySet	(),
	itsExtPropertySet	(),
	itsOwnPSinitialized (false),
	itsExtPSinitialized (false),
	itsTimerPort		(0)
{
	LOG_TRACE_OBJ_STR (cntlrName << " construction");

	// Readin some parameters from the ParameterSet.
	itsTreePrefix = globalParameterSet()->getString("prefix");
	itsInstanceNr = globalParameterSet()->getUint32(itsTreePrefix + "instanceNr");

	LOG_INFO_STR("MACProcessScope: " << itsTreePrefix + cntlrName);

	// need port for timers.
	itsTimerPort = new GCFTimerPort(*this, "TimerPort");

	// for debugging purposes
	registerProtocol (CONTROLLER_PROTOCOL, CONTROLLER_PROTOCOL_signalnames);
	registerProtocol (PA_PROTOCOL, 		   PA_PROTOCOL_signalnames);
	registerProtocol (F_PML_PROTOCOL, 	   F_PML_PROTOCOL_signalnames);
}


//
// ~StationControl()
//
StationControl::~StationControl()
{
	LOG_TRACE_OBJ_STR (getName() << " destruction");

	if (itsOwnPropertySet) {
		itsOwnPropertySet->setValue(string(PVSSNAME_FSM_STATE),GCFPVString("down"));
	}

	// ...
}


//
// handlePropertySetAnswer(answer)
//
void StationControl::handlePropertySetAnswer(GCFEvent& answer)
{
	LOG_DEBUG_STR ("handlePropertySetAnswer:" << eventstr(answer));

	switch(answer.signal) {
	case F_MYPS_ENABLED: 
	case F_EXTPS_LOADED: {
		GCFPropSetAnswerEvent* pPropAnswer=static_cast<GCFPropSetAnswerEvent*>(&answer);
		if(pPropAnswer->result != GCF_NO_ERROR) {
			LOG_ERROR(formatString("%s : PropertySet %s NOT ENABLED",
										getName().c_str(), pPropAnswer->pScope));
		}
		// always let timer expire so main task will continue.
		LOG_DEBUG_STR("Property set " << pPropAnswer->pScope << 
													" enabled, continuing main task");
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
		if (strstr(pPropAnswer->pPropName, PN_SC_CLOCK) != 0) {
			itsClock =(static_cast<const GCFPVInteger*>(pPropAnswer->pValue))->getValue();
			LOG_DEBUG_STR("Received clock change from PVSS, clock is now " << itsClock);
			break;
		}
		LOG_WARN_STR("Got VCHANGEMSG signal from unknown property " << 
															pPropAnswer->pPropName);
	}

//	case F_SUBSCRIBED:
//	case F_UNSUBSCRIBED:
//	case F_PS_CONFIGURED:
//	case F_EXTPS_LOADED:
//	case F_EXTPS_UNLOADED:
//	case F_MYPS_ENABLED:
//	case F_MYPS_DISABLED:
//	case F_VGETRESP:
//	case F_VSETRESP:
//	case F_VCHANGEMSG:
//	case F_SERVER_GONE:

	default:
		break;
	}  
}


//
// initial_state(event, port)
//
// Setup connection with PVSS
//
GCFEvent::TResult StationControl::initial_state(GCFEvent& event, 
													GCFPortInterface& port)
{
	LOG_DEBUG_STR ("initial:" << eventstr(event) << "@" << port.getName());

	GCFEvent::TResult status = GCFEvent::HANDLED;
  
	switch (event.signal) {
    case F_INIT:
   		break;

	case F_ENTRY: {
		// Get access to my own propertyset.
//		string	myPropSetName(createPropertySetName(PSN_STATION_CLOCK, getName()));
//myPropSetName=myPropSetName.substr(8);
								//new GCFMyPropertySet(myPropSetName.c_str(),
//		LOG_DEBUG_STR ("Activating PropertySet " << myPropSetName);
		LOG_DEBUG_STR ("Activating PropertySet " << PSN_STATION_CLOCK);
		itsOwnPropertySet = GCFMyPropertySetPtr(
								new GCFMyPropertySet(PSN_STATION_CLOCK,
													 PST_STATION_CLOCK,
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

			// update PVSS.
			LOG_TRACE_FLOW ("Updateing state to PVSS");
			itsOwnPropertySet->setValue(PVSSNAME_FSM_STATE,GCFPVString("initial"));
			itsOwnPropertySet->setValue(PVSSNAME_FSM_ERROR,GCFPVString(""));
		}

		LOG_DEBUG ("Attached to external propertySet, going to operational state");
		TRAN(StationControl::operational_state);			// go to next state.
		break;

	case F_CONNECTED:
		break;

	case F_DISCONNECTED:
		break;
	
	default:
		LOG_DEBUG_STR ("initial, default");
		status = GCFEvent::NOT_HANDLED;
		break;
	}    
	return (status);
}


//
// operational_state(event, port)
//
// Normal operation state. 
//
GCFEvent::TResult StationControl::operational_state(GCFEvent& event, GCFPortInterface& port)
{
	LOG_DEBUG_STR ("operational:" << eventstr(event) << "@" << port.getName());

	GCFEvent::TResult status = GCFEvent::HANDLED;

	switch (event.signal) {
	case F_INIT:
		break;

	case F_ENTRY: {
		// update PVSS
		itsOwnPropertySet->setValue(PVSSNAME_FSM_STATE,GCFPVString("active"));
		itsOwnPropertySet->setValue(PVSSNAME_FSM_ERROR,GCFPVString(""));
		LOG_DEBUG ("Changing clock over 20 seconds...");
		itsTimerPort->setTimer(20.0);
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
		if (itsClock == 200) {
			itsClock = 160;
		}
		else {
			itsClock = 200;
		}
		LOG_DEBUG_STR ("Changing clock to " << itsClock);
		itsOwnPropertySet->setValue(PN_SC_CLOCK,GCFPVInteger(itsClock));
		LOG_DEBUG ("Changing clock back over 60 seconds...");
		itsTimerPort->setTimer(60.0);
		break;

	default:
		LOG_DEBUG("active_state, default");
		status = GCFEvent::NOT_HANDLED;
		break;
	}

	return (status);
}


//
// _disconnectedHandler(port)
//
void StationControl::_disconnectedHandler(GCFPortInterface& port)
{
	port.close();
}


}; // StationCU
}; // LOFAR
