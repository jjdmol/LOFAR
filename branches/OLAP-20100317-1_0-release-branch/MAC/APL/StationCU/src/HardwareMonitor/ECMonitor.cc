//#  ECMonitor.cc: Implementation of the MAC Scheduler task
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
#include <Common/ParameterSet.h>
#include <Common/LofarLogger.h>
#include <Common/LofarConstants.h>
#include <Common/lofar_datetime.h>
#include <Common/StringUtil.h>
#include <Common/hexdump.h>

#include <GCF/PVSS/GCF_PVTypes.h>
#include <MACIO/MACServiceInfo.h>
#include <APL/APLCommon/ControllerDefines.h>
#include <APL/APLCommon/APLUtilities.h>
#include <ApplCommon/StationConfig.h>
#include <APL/RTDBCommon/RTDButilities.h>
#include <EC_Protocol.ph>
#include <GCF/RTDB/DP_Protocol.ph>
#include <GCF/TM/GCF_RawPort.h>
//#include <APL/APLCommon/StationInfo.h>
#include <signal.h>

#include "ECMonitor.h"
#include "RCUConstants.h"
#include "PVSSDatapointDefs.h"

// haal uit conf file hostname
// nog toevoegen aan PVVSDatapointDefs.h
// #define PN_HWM_EC_CONNECTED  "EC.connected"


namespace LOFAR {
	using namespace GCF::TM;
	using namespace GCF::PVSS;
	using namespace GCF::RTDB;
	using namespace APLCommon;
	using namespace APL::RTDBCommon;
	namespace StationCU {

//
// ECMonitor()
//
ECMonitor::ECMonitor(const string&  cntlrName) :
	GCFTask            ((State)&ECMonitor::initial_state,cntlrName),
	itsOwnPropertySet  (0),
	itsTimerPort       (0),
	itsECPort          (0),
	itsPollInterval    (10),
	itsNrCabs          (4)
{
	LOG_TRACE_OBJ_STR (cntlrName << " construction");

	// need port for timers.
	itsTimerPort = new GCFTimerPort(*this, "TimerPort");

	// prepare TCP port to EC controller.
	itsECPort = new GCFTCPPort (*this, toString(MAC_EC_PORT),
												GCFPortInterface::SAP, EC_PROTOCOL, true /*raw*/);
	// IP adres
	string hostName = globalParameterSet()->getString("EnvCntrl_IP","0.0.0.0");
	LOG_DEBUG_STR("IP of Environmental Controller = " << hostName);
	itsECPort->setHostName(hostName);
	itsECPort->setPortNumber(10000);

	ASSERTSTR(itsECPort, "Cannot allocate TCPport to EnvironmentController");
	itsECPort->setInstanceNr(0);

	// for debugging purposes
	registerProtocol (EC_PROTOCOL, EC_PROTOCOL_STRINGS);

	StationConfig sc;

	int itsNrSubracks = (sc.nrRSPs / NR_RSPBOARDS_PER_SUBRACK) +
						((sc.nrRSPs % NR_RSPBOARDS_PER_SUBRACK) ? 1 : 0);
	itsNrSystemCabs   = (itsNrSubracks / NR_SUBRACKS_PER_CABINET)  +
						((itsNrSubracks % NR_SUBRACKS_PER_CABINET) ? 1 : 0);
}


//
// ~ECMonitor()
//
ECMonitor::~ECMonitor()
{
	LOG_TRACE_OBJ_STR (getName() << " destruction");

	if (itsECPort) {
		itsECPort->close();
		delete itsECPort;
	}

	if (itsTimerPort) {
		delete itsTimerPort;
	}

	// ...
}


//
// initial_state(event, port)
//
// Setup connection with PVSS
//
GCFEvent::TResult ECMonitor::initial_state(GCFEvent& event, GCFPortInterface& port)
{
	LOG_DEBUG_STR ("initial:" << eventName(event) << "@" << port.getName());

	GCFEvent::TResult status = GCFEvent::HANDLED;

	switch (event.signal) {
	case F_INIT:
		break;

	case F_ENTRY: {
		// Get access to my own propertyset.
		LOG_DEBUG_STR ("Activating PropertySet " << PSN_HARDWARE_MONITOR);
		itsTimerPort->setTimer(2.0);
		itsOwnPropertySet = new RTDBPropertySet(PSN_HARDWARE_MONITOR,
													PST_HARDWARE_MONITOR,
													PSAT_WO,
													this);

		}
		break;

	case DP_CREATED: {
		// NOTE: this function may be called DURING the construction of the PropertySet.
		// Always exit this event in a way that GCF can end the construction.
		DPCreatedEvent      dpEvent(event);
		LOG_DEBUG_STR("Result of creating " << dpEvent.DPname << " = " << dpEvent.result);
		itsTimerPort->cancelAllTimers();
		itsTimerPort->setTimer(0.0);
		}
	break;

	case F_TIMER: {
		// update PVSS.
		LOG_TRACE_FLOW ("Updating state to PVSS");
		itsOwnPropertySet->setValue(PN_FSM_CURRENT_ACTION, GCFPVString("EC:initial"));
		itsOwnPropertySet->setValue(PN_HWM_EC_CONNECTED,GCFPVBool(false));
//      itsOwnPropertySet->setValue(PN_FSM_ERROR,  GCFPVString(""));

		LOG_DEBUG_STR("Going to connect to the ECPort.");
		TRAN (ECMonitor::connect2EC);
	}

	case DP_SET:
		break;

	case F_QUIT:
		TRAN (ECMonitor::finish_state);
		break;

	default:
		LOG_DEBUG_STR ("initial, DEFAULT");
		break;
	}

	return (status);
}


//
// connect2EC(event, port)
//
// Setup connection with ECdriver
//
GCFEvent::TResult ECMonitor::connect2EC(GCFEvent& event, GCFPortInterface& port)
{
	LOG_DEBUG_STR ("connect2EC:" << eventName(event) << "@" << port.getName());

	GCFEvent::TResult status = GCFEvent::HANDLED;

	switch (event.signal) {
	case F_ENTRY:
		// update PVSS
		itsOwnPropertySet->setValue(PN_FSM_CURRENT_ACTION, GCFPVString("EC:connecting"));
		itsTimerPort->setTimer(2.0);    // give database some time
		break;

	case F_CONNECTED:
		if (&port == itsECPort) {
		  LOG_DEBUG ("Connected with ECPort, going to get the status");
//          itsOwnPropertySet->setValue(PN_FSM_ERROR,  GCFPVString(""));
		  itsOwnPropertySet->setValue(PN_HWM_EC_CONNECTED,GCFPVBool(true));
		  TRAN(ECMonitor::createPropertySets);        // go to next state.
		}
		break;

	case F_DISCONNECTED:
		port.close();
		ASSERTSTR (&port == itsECPort, "F_DISCONNECTED event from port " << port.getName());
		LOG_WARN("Connection with ECPort failed, retry in 10 seconds");
		itsOwnPropertySet->setValue(PN_FSM_ERROR, GCFPVString("EC:connection timeout"));
		itsTimerPort->setTimer(10.0);
		break;

	case F_TIMER:
		itsECPort->open();      // results in F_CONN or F_DISCON
		break;

	case DP_SET:
		break;

	case F_QUIT:
		TRAN (ECMonitor::finish_state);
		break;

	default:
		LOG_DEBUG_STR ("connect2EC, DEFAULT");
		break;
	}

	return (status);
}


//
// createPropertySets(event, port)
//
// Retrieve sampleclock from EC driver
//
GCFEvent::TResult ECMonitor::createPropertySets(GCFEvent& event, GCFPortInterface& port)
{
	LOG_DEBUG_STR ("createPropertySets:" << eventName(event) << "@" << port.getName());

	GCFEvent::TResult status = GCFEvent::HANDLED;

	switch (event.signal) {

	case F_ENTRY: {
		itsOwnPropertySet->setValue(PN_FSM_CURRENT_ACTION,GCFPVString("EC:create PropertySets"));
		// resize vectors.
		itsCabs.resize (itsNrCabs, 0);
		
		string  stationNameMask("MCU001:"+createPropertySetName(PSN_STATION, getName()));
//		LOG_DEBUG_STR("stationNameMask=" << stationNameMask);
		string  PSname(formatString(stationNameMask.c_str(), 0));
//		LOG_DEBUG_STR("PSname=" << PSname);
		itsStation = new RTDBPropertySet(PSname, PST_STATION, PSAT_WO | PSAT_CW, this);
		itsStation->setConfirmation(false);

		string  cabNameMask(createPropertySetName(PSN_CABINET, getName()));
		for (int cab = 0; cab < itsNrCabs; cab++) {
			if ((cab > (itsNrSystemCabs - 1)) && (cab != (itsNrCabs - 1))) {
				itsCabs[cab] = 0;
			}
			else {
				// allocate RCU PS
				string  PSname(formatString(cabNameMask.c_str(), cab));
				itsCabs[cab] = new RTDBPropertySet(PSname, PST_CABINET, PSAT_WO | PSAT_CW, this);
				itsCabs[cab]->setConfirmation(false);
			}
		}
		itsTimerPort->setTimer(5.0);    // give database some time to finish the job
	}
	break;

	case F_TIMER: {
		// database should be ready by now, check if allocation was succesfull
		ASSERTSTR(itsStation, "Allocation of PS for station " << 0 << " failed.");
		for (int cab = 0; cab < itsNrCabs; cab++) {
			if ((cab > (itsNrSystemCabs - 1)) && (cab != (itsNrCabs - 1))) {
				continue;
			}
			ASSERTSTR(itsCabs[cab], "Allocation of PS for ec " << cab << " failed.");
		}
		LOG_INFO_STR("Allocation of all propertySets successfull, going to operational mode");
//      itsOwnPropertySet->setValue(PN_FSM_ERROR,GCFPVString(""));
		TRAN(ECMonitor::askSettings);
	}
	break;

	case F_DISCONNECTED:
		_disconnectedHandler(port);     // might result in transition to connect2EC
	break;

	case DP_SET:
		break;

	case F_QUIT:
		TRAN (ECMonitor::finish_state);
		break;

	default:
		LOG_DEBUG_STR ("createPropertySets, DEFAULT");
		break;
	}

	return (status);
}

//
// askSettings(event, port)
//
// Ask the settings of the EC controller.
//
GCFEvent::TResult ECMonitor::askSettings(GCFEvent& event, GCFPortInterface& port)
{
	LOG_DEBUG_STR ("askSettings:" << eventName(event) << "@" << port.getName());

	GCFEvent::TResult status = GCFEvent::HANDLED;

	switch (event.signal) {

	case F_ENTRY: {
		itsOwnPropertySet->setValue(PN_FSM_CURRENT_ACTION,GCFPVString("EC:getting settings info"));
//      itsOwnPropertySet->setValue(PN_FSM_ERROR,GCFPVString(""));
		itsTimerPort->setTimer(0.1);
	}
	break;

	case F_TIMER: {
		ECCmdEvent  cmd;
		cmd.cmdId = EC_SETTINGS;
		cmd.cabNr = 0;
		cmd.value = 0;
		string hd;
		hexdump(hd,&cmd,6);
		LOG_DEBUG_STR(hd);
		itsECPort->send(cmd);
	}
	break;

	case F_DATAIN: {
		status = RawEvent::dispatch(*this, port);
	}
	break;

	case EC_CMD_ACK: {
		itsTimerPort->cancelAllTimers();
		ECCmdAckEvent   ack(event);
		LOG_DEBUG_STR("EC_CMD_ACK: " << ack);
		sts_settings settings;
		memcpy(&settings, &ack.payload, ack.payloadSize);

		// move the information to the database.
		string  infoStr;
		double   value;
		for (int cab = 0; cab < itsNrCabs; cab++) {
			if (itsCabs[cab] == 0) { continue; }
			value = static_cast<double>(settings.cab[cab].temp_min) / 10.0;
			itsCabs[cab]->setValue(PN_CAB_TEMP_MIN, GCFPVDouble(value), 0.0, false);

			//value = static_cast<double>(settings.cab[cab].temp_min_min) / 10.0;
			//itsCabs[cab]->setValue(PN_CAB_TEMP_MIN_MIN, GCFPVDouble(value), 0.0, false);

			value = static_cast<double>(settings.cab[cab].temp_max) / 10.0;
			itsCabs[cab]->setValue(PN_CAB_TEMP_MAX, GCFPVDouble(value), 0.0, false);

			value = static_cast<double>(settings.cab[cab].temp_max_max) / 10.0;
			itsCabs[cab]->setValue(PN_CAB_TEMP_MAX_MAX, GCFPVDouble(value), 0.0, false);

			value = static_cast<double>(settings.cab[cab].humidity_max) / 10.0;
			itsCabs[cab]->setValue(PN_CAB_HUMIDITY_MAX, GCFPVDouble(value), 0.0, false);

			value = static_cast<double>(settings.cab[cab].humidity_max_max) / 10.0;
			itsCabs[cab]->setValue(PN_CAB_HUMIDITY_MAX_MAX, GCFPVDouble(value), 0.0, false);

			itsCabs[cab]->flush();
		}

		LOG_DEBUG_STR ("Settings information updated, going to status information");
//      itsOwnPropertySet->setValue(PN_FSM_ERROR,GCFPVString(""));
		TRAN(ECMonitor::askStatus);             // go to next state.
		break;
	}

	case F_DISCONNECTED:
		_disconnectedHandler(port);     // might result in transition to connect2EC
		break;

	case DP_SET:
		break;

	case F_QUIT:
		TRAN (ECMonitor::finish_state);
		break;

	default:
		LOG_DEBUG_STR ("askVersion, DEFAULT");
		break;
	}

	LOG_DEBUG("EO AskSettings");
	return (status);
}




//
// askStatus(event, port)
//
// Ask the status of the EC controller.
//
GCFEvent::TResult ECMonitor::askStatus(GCFEvent& event, GCFPortInterface& port)
{
	LOG_DEBUG_STR ("askStatus:" << eventName(event) << "@" << port.getName());

	GCFEvent::TResult status = GCFEvent::HANDLED;

	switch (event.signal) {

	case F_ENTRY: {
		itsOwnPropertySet->setValue(PN_FSM_CURRENT_ACTION,GCFPVString("EC:getting status info"));
//      itsOwnPropertySet->setValue(PN_FSM_ERROR,GCFPVString(""));
		itsTimerPort->setTimer(0.1);
	}
	break;

	case F_TIMER: {
		ECCmdEvent  cmd;
		cmd.cmdId = EC_STATUS;
		cmd.cabNr = 0;
		cmd.value = 0;
		string hd;
		hexdump(hd,&cmd.cmdId,6);
		LOG_DEBUG_STR(hd);
		itsECPort->send(cmd);
	}
	break;

	case F_DATAIN: {
		status = RawEvent::dispatch(*this, port);
	}
	break;

	case EC_CMD_ACK: {
		itsTimerPort->cancelAllTimers();
		ECCmdAckEvent   ack(event);
		sts_status sts_stat;
		memcpy(&sts_stat, &ack.payload, ack.payloadSize);

// Cabinet
//#define PN_CAB_TEMP_ALARM   "tempAlarm"
//#define PN_CAB_HUMIDITY_ALARM   "humidityAlarm"

		// move the information to the database.
		string  infoStr;
		bool     bState;
		int      iState;
		double   value;
		for (int cab = 0; cab < itsNrCabs; cab++) {
			if (itsCabs[cab] == 0) { continue; }
			itsCabs[cab]->setValue(PN_CAB_CONTROL_MODE, GCFPVString(ctrlMode(sts_stat.cab[cab].mode)), 0.0, false);

			if (sts_stat.cab[cab].state & CAB_TEMP_MIN_MIN) { iState = -2; }
			else if (sts_stat.cab[cab].state & CAB_TEMP_MAX_MAX) { iState = 2; }
			else if (sts_stat.cab[cab].state & CAB_TEMP_MIN) { iState = -1; }
			else if (sts_stat.cab[cab].state & CAB_TEMP_MAX) { iState = 1; }
			else { iState = 0; }
			itsCabs[cab]->setValue(PN_CAB_TEMP_ALARM, GCFPVBool(iState), 0.0, false);

			if (sts_stat.cab[cab].state & CAB_HUMIDITY_MAX_MAX) { iState = 2; }
			else if (sts_stat.cab[cab].state & CAB_HUMIDITY_MAX) { iState = 1; }
			else { iState = 0; }
			itsCabs[cab]->setValue(PN_CAB_HUMIDITY_ALARM, GCFPVBool(iState), 0.0, false);

			bState = (sts_stat.cab[cab].state & CAB_TEMPERATURE_SENSOR)?false:true;
			itsCabs[cab]->setValue(PN_CAB_TEMPERATURE_SENSOR, GCFPVBool(bState), 0.0, false);

			bState = (sts_stat.cab[cab].state & CAB_HUMIDITY_CONTROL)?false:true;
			itsCabs[cab]->setValue(PN_CAB_HUMIDITY_CONTROL, GCFPVBool(bState), 0.0, false);

			bState = (sts_stat.cab[cab].state & CAB_DOOR_CONTROL)?false:true;
			itsCabs[cab]->setValue(PN_CAB_DOOR_CONTROL, GCFPVBool(bState), 0.0, false);

			value = static_cast<double>(sts_stat.cab[cab].temperature / 100.);
			itsCabs[cab]->setValue(PN_CAB_TEMPERATURE, GCFPVDouble(value), 0.0, false);

			value = static_cast<double>(sts_stat.cab[cab].humidity / 100.);
			itsCabs[cab]->setValue(PN_CAB_HUMIDITY, GCFPVDouble(value), 0.0, false);

			bState = (sts_stat.cab[cab].control & CAB_FRONT_FAN_INNER);
			itsCabs[cab]->setValue(PN_CAB_FRONT_FAN_INNER, GCFPVBool(bState), 0.0, false);

			bState = (sts_stat.cab[cab].control & CAB_FRONT_FAN_OUTER);
			itsCabs[cab]->setValue(PN_CAB_FRONT_FAN_OUTER, GCFPVBool(bState), 0.0, false);

			bState = (sts_stat.cab[cab].control & CAB_BACK_FAN_INNER);
			itsCabs[cab]->setValue(PN_CAB_BACK_FAN_INNER, GCFPVBool(bState), 0.0, false);

			bState = (sts_stat.cab[cab].control & CAB_BACK_FAN_OUTER);
			itsCabs[cab]->setValue(PN_CAB_BACK_FAN_OUTER, GCFPVBool(bState), 0.0, false);

			bState = (sts_stat.cab[cab].control & CAB_FRONT_AIRFLOW);
			itsCabs[cab]->setValue(PN_CAB_FRONT_AIRFLOW, GCFPVBool(bState), 0.0, false);

			bState = (sts_stat.cab[cab].control & CAB_BACK_AIRFLOW);
			itsCabs[cab]->setValue(PN_CAB_BACK_AIRFLOW, GCFPVBool(bState), 0.0, false);

			//bState = (sts_stat.cab[cab].control & CAB_HEATER);
			//itsCabs[cab]->setValue(PN_CAB_HEATER, GCFPVBool(bState), 0.0, false);

			bState = (sts_stat.cab[cab].doors & CAB_FRONT_DOOR_OPEN);
			itsCabs[cab]->setValue(PN_CAB_FRONT_DOOR_OPEN, GCFPVBool(bState), 0.0, false);

			bState = (sts_stat.cab[cab].doors & CAB_BACK_DOOR_OPEN);
			itsCabs[cab]->setValue(PN_CAB_BACK_DOOR_OPEN, GCFPVBool(bState), 0.0, false);

			itsCabs[cab]->flush();
		}

		bState = (sts_stat.power & STS_POWER48_ON);
		itsStation->setValue(PN_STS_POWER48_ON, GCFPVBool(bState), 0.0, false);

		bState = (sts_stat.power & STS_POWER220_ON);
		itsStation->setValue(PN_STS_POWER220_ON, GCFPVBool(bState), 0.0, false);

		//bState = (sts_stat.lightning & STS_LIGHTNING);
		//itsStation->setValue(PN_STS_LIGHTNING, GCFPVBool(bState), 0.0, false);

		itsStation->flush();


		LOG_DEBUG_STR ("Status information updated, going to waitForNextCycle");
//      itsOwnPropertySet->setValue(PN_FSM_ERROR,GCFPVString(""));
		TRAN(ECMonitor::waitForNextCycle);              // go to next state.
		break;
	}

	case F_DISCONNECTED:
		_disconnectedHandler(port);     // might result in transition to connect2EC
		break;

	case DP_SET:
		break;

	case F_QUIT:
		TRAN (ECMonitor::finish_state);
		break;

	default:
		LOG_DEBUG_STR ("askVersion, DEFAULT");
		break;
	}

	return (status);
}


//
// waitForNextCycle(event, port)
//
// Take subscription on clock modifications
//
GCFEvent::TResult ECMonitor::waitForNextCycle(GCFEvent& event,
														GCFPortInterface& port)
{
	if (eventName(event) != "DP_SET") {
		LOG_DEBUG_STR ("waitForNextCycle:" << eventName(event) << "@" << port.getName());   }

	GCFEvent::TResult status = GCFEvent::HANDLED;

	switch (event.signal) {
	case F_ENTRY: {
		itsOwnPropertySet->setValue(PN_FSM_CURRENT_ACTION,GCFPVString("EC:wait for next cycle"));
		int     waitTime = itsPollInterval - (time(0) % itsPollInterval);
		if (waitTime == 0) {
			 waitTime = itsPollInterval;
		}
		itsTimerPort->cancelAllTimers();
		itsTimerPort->setTimer(double(waitTime));
		LOG_INFO_STR("Waiting " << waitTime << " seconds for next cycle");
	}
	break;

	case F_DISCONNECTED:
		_disconnectedHandler(port);     // might result in transition to connect2EC
	break;

	case F_TIMER: {
		TRAN(ECMonitor::askStatus);
	}
	break;

	case DP_SET:
		break;

	case F_QUIT:
		TRAN (ECMonitor::finish_state);
		break;

	default:
		LOG_DEBUG_STR ("waitForNextCycle, DEFAULT");
		break;
	}

	return (status);
}


//
// _disconnectedHandler(port)
//
void ECMonitor::_disconnectedHandler(GCFPortInterface& port)
{
	port.close();
	if (&port == itsECPort) {
		LOG_ERROR("Connection with ECPort failed, going to reconnect state");
		itsOwnPropertySet->setValue(PN_FSM_ERROR,GCFPVString("EC:connection lost"));
		TRAN (ECMonitor::connect2EC);
	}
}

//
// finish_state(event, port)
//
// Write controller state to PVSS
//
GCFEvent::TResult ECMonitor::finish_state(GCFEvent& event, GCFPortInterface& port)
{
	LOG_DEBUG_STR ("finish_state:" << eventName(event) << "@" << port.getName());

	GCFEvent::TResult status = GCFEvent::HANDLED;

	switch (event.signal) {
	case F_INIT:
		break;

	case F_ENTRY: {
		// update PVSS
		itsOwnPropertySet->setValue(PN_FSM_CURRENT_ACTION,GCFPVString("EC:finished"));
		itsOwnPropertySet->setValue(PN_HWM_EC_CONNECTED,GCFPVBool(false));
//      itsOwnPropertySet->setValue(PN_FSM_ERROR,GCFPVString(""));
		break;
	}

	case DP_SET:
		break;

	default:
		LOG_DEBUG("finishing_state, DEFAULT");
		status = GCFEvent::NOT_HANDLED;
		break;
	}
	return (status);
}

string ECMonitor::ctrlMode(int16 mode)
{
	switch (mode) {
		case 0: return ("Off");
		case 1: return ("On");
		case 2: return ("Auto");
		case 3: return ("Manual");
		case 4: return ("StartUp");
		case 5: return ("Absent");
		default: return ("Unknown");
	}
}

// next code for raw event handling
typedef struct {
	GCFEvent event;
	int16    cmdId;
	int16    status;
	int16    payloadLen; // number of bytes in payload
	int16    payload[64];
} ECFrame;


GCFEvent::TResult RawEvent::dispatch(GCFTask& task, GCFPortInterface& port)
{
  static ECFrame buf;
  string hd;
  ssize_t size;
  LOG_DEBUG_STR("received raw event");
  GCFEvent::TResult status = GCFEvent::NOT_HANDLED;

  // Receive a raw packet
  size = port.recv(&buf.cmdId, (3 * sizeof(int16)));
  hexdump(hd,&buf.cmdId,(3*sizeof(int16)));
  LOG_DEBUG_STR("raw buf header=" << hd);

  // at least 6 bytes
  if (size < 6) return(GCFEvent::NOT_HANDLED);

  if (buf.payloadLen > 0) {
	 size = port.recv(buf.payload, buf.payloadLen);
	 hexdump(hd,&buf.payload[0], buf.payloadLen);
	 LOG_DEBUG_STR("raw buf payload=" << hd);
  }

  buf.event.signal = EC_CMD_ACK;
  buf.event.length = (3*sizeof(int16)) + buf.payloadLen;
  hexdump(hd,&buf,sizeof(buf));
  LOG_DEBUG_STR("raw buf all=" << hd);
  // dispatch the EC message as a GCFEvent (which it now is)
  buf.event._buffer = (char*)(&buf.cmdId) - GCFEvent::sizePackedGCFEvent;
  status = task.doEvent(buf.event, port);
  buf.event._buffer = 0;

  return(status);
}


}; // StationCU
}; // LOFAR
