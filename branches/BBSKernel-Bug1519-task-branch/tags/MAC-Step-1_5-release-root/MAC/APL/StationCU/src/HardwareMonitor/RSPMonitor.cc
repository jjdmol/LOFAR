//#  RSPMonitor.cc: Implementation of the MAC Scheduler task
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
//#  $Id: RSPMonitor.cc 10505 2007-09-07 17:14:57Z overeem $
#include <lofar_config.h>
#include <Common/LofarLogger.h>
#include <Common/LofarConstants.h>

#include <GCF/PVSS/GCF_PVTypes.h>
#include <MACIO/MACServiceInfo.h>
#include <APL/APLCommon/ControllerDefines.h>
#include <APL/RTDBCommon/RTDButilities.h>
#include <APL/RSP_Protocol/RSP_Protocol.ph>
#include <GCF/RTDB/DP_Protocol.ph>
//#include <APL/APLCommon/StationInfo.h>
#include <signal.h>

#include "RSPMonitor.h"
#include "RCUConstants.h"
#include "PVSSDatapointDefs.h"


namespace LOFAR {
	using namespace APLCommon;
	using namespace APL::RTDBCommon;
	using namespace GCF::TM;
	using namespace GCF::PVSS;
	using namespace GCF::RTDB;
	namespace StationCU {
	
//
// RSPMonitor()
//
RSPMonitor::RSPMonitor(const string&	cntlrName) :
	GCFTask 			((State)&RSPMonitor::initial_state,cntlrName),
	itsOwnPropertySet	(0),
	itsTimerPort		(0),
	itsRSPDriver		(0),
	itsPollInterval		(10),
	itsNrRCUs			(0),
	itsNrRSPboards		(0),
	itsNrSubracks		(0),
	itsNrCabinets		(0)
{
	LOG_TRACE_OBJ_STR (cntlrName << " construction");

	// need port for timers.
	itsTimerPort = new GCFTimerPort(*this, "TimerPort");

	// prepare TCP port to RSPDriver.
	itsRSPDriver = new GCFTCPPort (*this, MAC_SVCMASK_RSPDRIVER,
											GCFPortInterface::SAP, RSP_PROTOCOL);
	ASSERTSTR(itsRSPDriver, "Cannot allocate TCPport to RSPDriver");
	itsRSPDriver->setInstanceNr(0);

}


//
// ~RSPMonitor()
//
RSPMonitor::~RSPMonitor()
{
	LOG_TRACE_OBJ_STR (getName() << " destruction");

	if (itsRSPDriver) {
		itsRSPDriver->close();
	}

	// ...
}


//
// initial_state(event, port)
//
// Setup connection with PVSS
//
GCFEvent::TResult RSPMonitor::initial_state(GCFEvent& event, 
													GCFPortInterface& port)
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
		DPCreatedEvent		dpEvent(event);
		LOG_DEBUG_STR("Result of creating " << dpEvent.DPname << " = " << dpEvent.result);
		itsTimerPort->cancelAllTimers();
		itsTimerPort->setTimer(0.0);
		}
	break;

	case F_TIMER: {
		// update PVSS.
		LOG_TRACE_FLOW ("Updateing state to PVSS");
		itsOwnPropertySet->setValue(PN_FSM_CURRENT_ACTION, GCFPVString("RSP:initial"));
//		itsOwnPropertySet->setValue(PN_FSM_ERROR,  GCFPVString(""));
		
		LOG_DEBUG_STR("Going to connect to the RSPDriver.");
		TRAN (RSPMonitor::connect2RSP);
	}
	
	case DP_SET:
		break;

	case F_QUIT:
		TRAN (RSPMonitor::finish_state);
		break;

	default:
		LOG_DEBUG_STR ("initial, DEFAULT");
		break;
	}    

	return (status);
}


//
// connect2RSP(event, port)
//
// Setup connection with RSPdriver
//
GCFEvent::TResult RSPMonitor::connect2RSP(GCFEvent& event, 
													GCFPortInterface& port)
{
	LOG_DEBUG_STR ("connect2RSP:" << eventName(event) << "@" << port.getName());

	GCFEvent::TResult status = GCFEvent::HANDLED;
  
	switch (event.signal) {
	case F_ENTRY:
		// update PVSS
		itsOwnPropertySet->setValue(PN_FSM_CURRENT_ACTION, GCFPVString("RSP:connecting"));
		itsOwnPropertySet->setValue(PN_HWM_RSP_CONNECTED,  GCFPVBool(false));
		itsRSPDriver->open();		// will result in F_CONN or F_DISCONN
		break;

	case F_CONNECTED:
		if (&port == itsRSPDriver) {
			LOG_DEBUG ("Connected with RSPDriver, going to get the configuration");
//			itsOwnPropertySet->setValue(PN_HWM_RSP_ERROR,  GCFPVString(""));
			itsOwnPropertySet->setValue(PN_HWM_RSP_CONNECTED,GCFPVBool(true));
			TRAN(RSPMonitor::askConfiguration);		// go to next state.
		}
		break;

	case F_DISCONNECTED:
		port.close();
		ASSERTSTR (&port == itsRSPDriver, 
								"F_DISCONNECTED event from port " << port.getName());
		LOG_WARN("RSP:Connection with RSPDriver failed, retry in 10 seconds");
		itsOwnPropertySet->setValue(PN_FSM_ERROR, GCFPVString("RSP:connection timeout"));
		itsTimerPort->setTimer(10.0);
		break;

	case F_TIMER:
		itsRSPDriver->open();	// results in F_CONN or F_DISCON
		break;

	case DP_SET:
		break;

	case F_QUIT:
		TRAN (RSPMonitor::finish_state);
		break;

	default:
		LOG_DEBUG_STR ("connect2RSP, DEFAULT");
		break;
	}    

	return (status);
}

//
// askConfiguration(event, port)
//
// Take subscription on clock modifications
//
GCFEvent::TResult RSPMonitor::askConfiguration(GCFEvent& event, 
													GCFPortInterface& port)
{
	LOG_DEBUG_STR ("askConfiguration:" << eventName(event) << "@" << port.getName());

	GCFEvent::TResult status = GCFEvent::HANDLED;
  
	switch (event.signal) {
	case F_ENTRY: {
		itsOwnPropertySet->setValue(PN_FSM_CURRENT_ACTION,GCFPVString("RSP:asking configuration"));
		RSPGetconfigEvent	getconfig;
		itsRSPDriver->send(getconfig);
	}
	break;

	case F_DISCONNECTED:
		_disconnectedHandler(port);		// might result in transition to connect2RSP
		break;

	case RSP_GETCONFIGACK: {
		RSPGetconfigackEvent	ack(event);
	
		// calc size of the propertyset vectors
		itsNrRCUs	   = ack.n_rcus;
		itsNrRSPboards = ack.n_rspboards;
		itsNrSubracks  = (itsNrRSPboards/NR_RSPBOARDS_PER_SUBRACK) + 
						 	((itsNrRSPboards%NR_RSPBOARDS_PER_SUBRACK) ? 1 : 0);
		itsNrCabinets  = (itsNrSubracks /NR_SUBRACKS_PER_CABINET)  + 
						 	((itsNrSubracks%NR_SUBRACKS_PER_CABINET) ? 1 : 0);

		// inform user
		LOG_DEBUG(formatString("nr RCUs      = %d",ack.n_rcus));
		LOG_DEBUG(formatString("nr RSPboards = %d",ack.max_rspboards));
		LOG_DEBUG(formatString("nr Subracks  = %d", itsNrSubracks));
		LOG_DEBUG(formatString("nr Cabinets  = %d", itsNrCabinets));
		LOG_DEBUG_STR("("<<itsNrRSPboards<<"/"<<NR_RSPBOARDS_PER_SUBRACK<<") + (("<<itsNrRSPboards<<"%"<<NR_RSPBOARDS_PER_SUBRACK<<") ? 1 : 0)");
		LOG_DEBUG_STR("("<<itsNrRSPboards<<"/"<<NR_RSPBOARDS_PER_SUBRACK<<") = " << itsNrRSPboards/NR_RSPBOARDS_PER_SUBRACK);
		LOG_DEBUG_STR("("<<itsNrRSPboards<<"%"<<NR_RSPBOARDS_PER_SUBRACK<<") = " << itsNrRSPboards%NR_RSPBOARDS_PER_SUBRACK);
	
		// do some checks
		if (itsNrRSPboards != (uint32)ack.max_rspboards) {
			LOG_WARN_STR("RSP:RSPdriver only controls " << itsNrRSPboards << " of " << ack.max_rspboards 
						<< " RSPboards, cannot monitor full station");
		}
		if (itsNrRSPboards * NR_RCUS_PER_RSPBOARD != itsNrRCUs) {
			LOG_INFO_STR("RSP:Station not fully equiped, only " << itsNrRCUs << " of " 
						<< itsNrRSPboards * NR_RCUS_PER_RSPBOARD << "RCUs");
		}

		LOG_DEBUG ("Going to allocate the property-sets");
//		itsOwnPropertySet->setValue(PN_HWM_RSP_ERROR,GCFPVString(""));
		TRAN(RSPMonitor::createPropertySets);				// go to next state.
		}
		break;

	case DP_SET:
		break;

	case F_QUIT:
		TRAN (RSPMonitor::finish_state);
		break;

	default:
		LOG_DEBUG_STR ("askConfiguration, DEFAULT");
		break;
	}    

	return (status);
}


//
// createPropertySets(event, port)
//
// Retrieve sampleclock from RSP driver
//
GCFEvent::TResult RSPMonitor::createPropertySets(GCFEvent& event, 
													GCFPortInterface& port)
{
	LOG_DEBUG_STR ("createPropertySets:" << eventName(event) << "@" << port.getName());

	GCFEvent::TResult status = GCFEvent::HANDLED;
  
	switch (event.signal) {

	case F_ENTRY: {
		itsOwnPropertySet->setValue(PN_FSM_CURRENT_ACTION,GCFPVString("RSP:create PropertySets"));
		// resize vectors.
		itsCabinets.resize(itsNrCabinets,  0);
		itsSubracks.resize(itsNrSubracks,  0);
		itsRSPs.resize	  (itsNrRSPboards, 0);
		itsRCUs.resize	  (itsNrRCUs, 	   0);

		int32	cabinet (-1);
		int32	subrack (-1);
		int32	RSP		(-1);
		string	cabinetNameMask (createPropertySetName(PSN_CABINET,   getName()));
		string	subrackNameMask (createPropertySetName(PSN_SUB_RACK,  getName()));
		string	rspboardNameMask(createPropertySetName(PSN_RSP_BOARD, getName()));
		string	rcuNameMask     (createPropertySetName(PSN_RCU, 	  getName()));
		for (uint32	rcu = 0; rcu < itsNrRCUs; rcu++) {
			// new cabinet?
			if (rcu % (NR_RCUS_PER_CABINET) == 0) {
				cabinet++;
				string	PSname(formatString(cabinetNameMask.c_str(), cabinet));
				itsCabinets[cabinet] = new RTDBPropertySet(PSname, PST_CABINET, PSAT_WO | PSAT_CW, this);
			}

			// new subrack?
			if (rcu % (NR_RCUS_PER_SUBRACK) == 0) {
				subrack++;
				string	PSname(formatString(subrackNameMask.c_str(), cabinet, subrack));
				itsSubracks[subrack] = new RTDBPropertySet(PSname, PST_SUB_RACK, PSAT_WO | PSAT_CW, this);
			}

			// new RSPboard?
			if (rcu % (NR_RCUS_PER_RSPBOARD) == 0) {
				RSP++;
				string	PSname(formatString(rspboardNameMask.c_str(), cabinet, subrack, RSP));
				itsRSPs[RSP] = new RTDBPropertySet(PSname, PST_RSP_BOARD, PSAT_WO | PSAT_CW, this);
			}

			// allocate RCU PS
			string	PSname(formatString(rcuNameMask.c_str(), cabinet, subrack, RSP, rcu));
			itsRCUs[rcu] = new RTDBPropertySet(PSname, PST_RCU, PSAT_WO | PSAT_CW, this);
		}
		itsTimerPort->setTimer(5.0);	// give database some time to finish the job
	}
	break;

	case F_TIMER: {
		// database should be ready by now, check if allocation was succesfull
		for (uint32	cabinet = 0; cabinet < itsNrCabinets; cabinet++) {
			ASSERTSTR(itsCabinets[cabinet], "Allocation of PS for cabinet " << cabinet << " failed.");
		}
		for (uint32	subrack = 0; subrack < itsNrSubracks; subrack++) {
			ASSERTSTR(itsSubracks[subrack], "Allocation of PS for subrack " << subrack << " failed.");
		}
		for (uint32	rsp = 0; rsp < itsNrRSPboards; rsp++) {
			ASSERTSTR(itsRSPs[rsp], "Allocation of PS for rsp " << rsp << " failed.");
		}
		for (uint32	rcu = 0; rcu < itsNrRCUs; rcu++) {
			ASSERTSTR(itsRCUs[rcu], "Allocation of PS for rcu " << rcu << " failed.");
		}
		LOG_DEBUG_STR("Allocation of all propertySets successfull, going to operational mode");
//		itsOwnPropertySet->setValue(PN_HWM_RSP_ERROR,GCFPVString(""));
		TRAN(RSPMonitor::askVersion);
	}
	break;

	case F_DISCONNECTED:
		_disconnectedHandler(port);		// might result in transition to connect2RSP
	break;

	case DP_SET:
		break;

	case F_QUIT:
		TRAN (RSPMonitor::finish_state);
		break;

	default:
		LOG_DEBUG_STR ("createPropertySets, DEFAULT");
		break;
	}    

	return (status);
}


//
// askVersion(event, port)
//
// Set sampleclock from RSP driver
//
GCFEvent::TResult RSPMonitor::askVersion(GCFEvent& event, 
													GCFPortInterface& port)
{
	LOG_DEBUG_STR ("askVersion:" << eventName(event) << "@" << port.getName());

	GCFEvent::TResult status = GCFEvent::HANDLED;
  
	switch (event.signal) {

	case F_ENTRY: {
		itsOwnPropertySet->setValue(PN_FSM_CURRENT_ACTION,GCFPVString("RSP:getting version info"));
//		itsOwnPropertySet->setValue(PN_HWM_RSP_ERROR,GCFPVString(""));
		RSPGetversionEvent	getVersion;
		getVersion.timestamp.setNow();
		getVersion.cache = true;
		itsRSPDriver->send(getVersion);
	}
	break;

	case RSP_GETVERSIONACK: {
		RSPGetversionackEvent		ack(event);
		if (ack.status != SUCCESS) {
			LOG_ERROR_STR ("RSP:Failed to get the version information, trying other information");
			itsOwnPropertySet->setValue(PN_FSM_ERROR,GCFPVString("RSP:getVersion error"));
			TRAN(RSPMonitor::askRSPinfo);				// go to next state.
			break;
		}

		// move the information to the database.
		string		versionStr;
		string		DPEname;
		for (uint32	rsp = 0; rsp < itsNrRSPboards; rsp++) {
			// RSP board version
			versionStr = formatString("%d.%d", ack.versions.bp()(rsp).rsp_version >> 4,
											   ack.versions.bp()(rsp).rsp_version & 0xF);
			itsRSPs[rsp]->setValue(PN_RSP_VERSION, GCFPVString(versionStr), double(ack.timestamp));
			setObjectState(getName(), itsRSPs[rsp]->getFullScope(), (ack.versions.bp()(rsp).rsp_version) ? 
															RTDB_OBJ_STATE_OPERATIONAL : RTDB_OBJ_STATE_OFF);
			
			// BP version
			versionStr = formatString("%d.%d", ack.versions.bp()(rsp).fpga_maj,
											   ack.versions.bp()(rsp).fpga_min);
			itsRSPs[rsp]->setValue(PN_RSP_BP_VERSION, GCFPVString(versionStr), double(ack.timestamp));
			setObjectState(getName(), itsRSPs[rsp]->getFullScope()+".BP", (ack.versions.bp()(rsp).rsp_version) ? 
															RTDB_OBJ_STATE_OPERATIONAL : RTDB_OBJ_STATE_OFF);
			
			// APx versions
			for (int ap = 0; ap < MEPHeader::N_AP; ap++) {
				versionStr = formatString("%d.%d", ack.versions.ap()(rsp * MEPHeader::N_AP + ap).fpga_maj,
												   ack.versions.ap()(rsp * MEPHeader::N_AP + ap).fpga_min);
				DPEname = formatString (PN_RSP_AP_VERSION_MASK, ap);
				itsRSPs[rsp]->setValue(DPEname, GCFPVString(versionStr), double(ack.timestamp));
				setObjectState(getName(), formatString("%s.AP%d", itsRSPs[rsp]->getFullScope().c_str(), ap),
										(ack.versions.ap()(rsp * MEPHeader::N_AP + ap).fpga_maj +
										 ack.versions.ap()(rsp * MEPHeader::N_AP + ap).fpga_min) ? 
										RTDB_OBJ_STATE_OPERATIONAL : RTDB_OBJ_STATE_OFF);
			}
		}

		LOG_DEBUG_STR ("Version information updated, going to status information");
//		itsOwnPropertySet->setValue(PN_HWM_RSP_ERROR,GCFPVString(""));
		TRAN(RSPMonitor::askRSPinfo);				// go to next state.
		break;
	}

	case F_DISCONNECTED:
		_disconnectedHandler(port);		// might result in transition to connect2RSP
		break;

	case DP_SET:
		break;

	case F_QUIT:
		TRAN (RSPMonitor::finish_state);
		break;

	default:
		LOG_DEBUG_STR ("askVersion, DEFAULT");
		break;
	}    

	return (status);
}


//
// askRSPinfo(event, port)
//
// Set sampleclock from RSP driver
//
GCFEvent::TResult RSPMonitor::askRSPinfo(GCFEvent& event, 
													GCFPortInterface& port)
{
	LOG_DEBUG_STR ("askRSPinfo:" << eventName(event) << "@" << port.getName());

	GCFEvent::TResult status = GCFEvent::HANDLED;
  
	switch (event.signal) {

	case F_ENTRY: {
		itsOwnPropertySet->setValue(PN_FSM_CURRENT_ACTION,GCFPVString("RSP:updating RSP info"));
//		itsOwnPropertySet->setValue(PN_HWM_RSP_ERROR,GCFPVString(""));
		RSPGetstatusEvent	getStatus;
		getStatus.timestamp.setNow();
		getStatus.cache = true;
		getStatus.rspmask = bitset<MAX_N_RSPBOARDS>((1<<itsNrRSPboards)-1);
		itsRSPDriver->send(getStatus);
	}
	break;

	case RSP_GETSTATUSACK: {
		RSPGetstatusackEvent		ack(event);
		if (ack.status != SUCCESS) {
			LOG_ERROR_STR ("RSP:Failed to get the status information, trying other information");
			itsOwnPropertySet->setValue(PN_FSM_ERROR,GCFPVString("RSP:getStatus error"));
			TRAN(RSPMonitor::askRCUinfo);				// go to next state.
			break;
		}

		// move the information to the database.
		string		versionStr;
		string		DPEname;
		for (uint32	rsp = 0; rsp < itsNrRSPboards; rsp++) {
			BoardStatus		bStat = ack.sysstatus.board()(rsp);
			// board voltages
			itsRSPs[rsp]->setValue(PN_RSP_VOLTAGE12, GCFPVDouble(double(bStat.rsp.voltage_1_2)*(2.5/192.0)), 
									double(ack.timestamp));
			itsRSPs[rsp]->setValue(PN_RSP_VOLTAGE25, GCFPVDouble(double(bStat.rsp.voltage_2_5)*(3.3/192.0)), 
									double(ack.timestamp));
			itsRSPs[rsp]->setValue(PN_RSP_VOLTAGE33, GCFPVDouble(double(bStat.rsp.voltage_3_3)*(5.0/192.0)), 
									double(ack.timestamp));
			
			// Ethernet status
			itsRSPs[rsp]->setValue(PN_RSP_ETHERNET_PACKETS_RECEIVED, GCFPVUnsigned(bStat.eth.nof_frames), 
									double(ack.timestamp));
			itsRSPs[rsp]->setValue(PN_RSP_ETHERNET_PACKETS_ERROR,    GCFPVUnsigned(bStat.eth.nof_errors), 
									double(ack.timestamp));
			itsRSPs[rsp]->setValue(PN_RSP_ETHERNET_LAST_ERROR,       GCFPVUnsigned(bStat.eth.last_error), 
									double(ack.timestamp));
			setObjectState(getName(), itsRSPs[rsp]->getFullScope()+".Ethernet", (bStat.eth.nof_frames != 0) ? 
															RTDB_OBJ_STATE_OPERATIONAL : RTDB_OBJ_STATE_OFF);
			
			// MEP status
			itsRSPs[rsp]->setValue(PN_RSP_MEP_SEQNR, GCFPVUnsigned(bStat.mep.seqnr), 
									double(ack.timestamp));
			itsRSPs[rsp]->setValue(PN_RSP_MEP_ERROR, GCFPVUnsigned(bStat.mep.error), 
									double(ack.timestamp));

			// BP status
			itsRSPs[rsp]->setValue(PN_RSP_BP_TEMPERATURE, GCFPVDouble(double(bStat.rsp.bp_temp)), 
									double(ack.timestamp));
			// AP0
			itsRSPs[rsp]->setValue(PN_RSP_AP0_TEMPERATURE,		 GCFPVDouble(double(bStat.rsp.ap0_temp)), 
									double(ack.timestamp));
			itsRSPs[rsp]->setValue(PN_RSP_AP0_SYNC_SAMPLE_COUNT, GCFPVUnsigned(bStat.ap0_sync.sample_offset), 
									double(ack.timestamp));
			itsRSPs[rsp]->setValue(PN_RSP_AP0_SYNC_SYNC_COUNT,   GCFPVUnsigned(bStat.ap0_sync.sample_offset)), 
									double(ack.timestamp);
			itsRSPs[rsp]->setValue(PN_RSP_AP0_SYNC_ERROR_COUNT,  GCFPVUnsigned(bStat.ap0_sync.ext_count)), 
									double(ack.timestamp);

			// AP1
			itsRSPs[rsp]->setValue(PN_RSP_AP1_TEMPERATURE,		 GCFPVDouble(double(bStat.rsp.ap1_temp)), 
									double(ack.timestamp));
			itsRSPs[rsp]->setValue(PN_RSP_AP1_SYNC_SAMPLE_COUNT, GCFPVUnsigned(bStat.ap1_sync.sample_offset)), 
									double(ack.timestamp);
			itsRSPs[rsp]->setValue(PN_RSP_AP1_SYNC_SYNC_COUNT,   GCFPVUnsigned(bStat.ap1_sync.sample_offset)), 
									double(ack.timestamp);
			itsRSPs[rsp]->setValue(PN_RSP_AP1_SYNC_ERROR_COUNT,  GCFPVUnsigned(bStat.ap1_sync.ext_count)), 
									double(ack.timestamp);

			// AP2
			itsRSPs[rsp]->setValue(PN_RSP_AP2_TEMPERATURE,		 GCFPVDouble(double(bStat.rsp.ap2_temp)), 
									double(ack.timestamp));
			itsRSPs[rsp]->setValue(PN_RSP_AP2_SYNC_SAMPLE_COUNT, GCFPVUnsigned(bStat.ap2_sync.sample_offset)), 
									double(ack.timestamp);
			itsRSPs[rsp]->setValue(PN_RSP_AP2_SYNC_SYNC_COUNT,   GCFPVUnsigned(bStat.ap2_sync.sample_offset)), 
									double(ack.timestamp);
			itsRSPs[rsp]->setValue(PN_RSP_AP2_SYNC_ERROR_COUNT,  GCFPVUnsigned(bStat.ap2_sync.ext_count)), 
									double(ack.timestamp);

			// AP3
			itsRSPs[rsp]->setValue(PN_RSP_AP3_TEMPERATURE,		 GCFPVDouble(double(bStat.rsp.ap3_temp)), 
									double(ack.timestamp));
			itsRSPs[rsp]->setValue(PN_RSP_AP3_SYNC_SAMPLE_COUNT, GCFPVUnsigned(bStat.ap3_sync.sample_offset)), 
									double(ack.timestamp);
			itsRSPs[rsp]->setValue(PN_RSP_AP3_SYNC_SYNC_COUNT,   GCFPVUnsigned(bStat.ap3_sync.sample_offset)), 
									double(ack.timestamp);
			itsRSPs[rsp]->setValue(PN_RSP_AP3_SYNC_ERROR_COUNT,  GCFPVUnsigned(bStat.ap3_sync.ext_count)), 
									double(ack.timestamp);
		} // for all boards

		LOG_DEBUG_STR ("RSPboard information updated, going to RCU information");
//		itsOwnPropertySet->setValue(PN_HWM_RSP_ERROR,GCFPVString(""));
		TRAN(RSPMonitor::askRCUinfo);				// go to next state.
		break;
	}

	case F_DISCONNECTED:
		_disconnectedHandler(port);		// might result in transition to connect2RSP
		break;

	case DP_SET:
		break;

	case F_QUIT:
		TRAN (RSPMonitor::finish_state);
		break;

	default:
		LOG_DEBUG_STR ("askRSPinfo, DEFAULT");
		break;
	}    

	return (status);
}


//
// askRCUinfo(event, port)
//
// Normal operation state. 
//
GCFEvent::TResult RSPMonitor::askRCUinfo(GCFEvent& event, GCFPortInterface& port)
{
	if (eventName(event) != "DP_SET") {
		LOG_DEBUG_STR ("askRCUinfo:" << eventName(event) << "@" << port.getName());
	}

	GCFEvent::TResult status = GCFEvent::HANDLED;

	switch (event.signal) {
	case F_INIT:
		break;

	case F_ENTRY: {
		// update PVSS
		itsOwnPropertySet->setValue(PN_FSM_CURRENT_ACTION,GCFPVString("RSP:updating RSP info"));
//		itsOwnPropertySet->setValue(PN_HWM_RSP_ERROR,GCFPVString(""));
		longlong	one(1);
		RSPGetrcuEvent	getStatus;
		getStatus.timestamp.setNow();
		getStatus.cache = true;
		getStatus.rcumask = bitset<MEPHeader::MAX_N_RCUS>((one<<itsNrRCUs)-1);
		itsRSPDriver->send(getStatus);
		LOG_DEBUG(formatString("MAX_N_RCUS=%d", MEPHeader::MAX_N_RCUS));
		LOG_DEBUG(formatString("itsRCUs=%d, rcumask=%08lX", itsNrRCUs, getStatus.rcumask.to_uint32()));
		break;
	}

	case RSP_GETRCUACK: {
		RSPGetrcuackEvent	ack(event);
		if (ack.status != SUCCESS) {
			LOG_ERROR_STR ("RSP:Failed to get the RCU information, trying other information");
			itsOwnPropertySet->setValue(PN_FSM_ERROR,GCFPVString("RSP:getRCU error"));
			TRAN(RSPMonitor::waitForNextCycle);			// go to next state.
			break;
		}

		// move the information to the database.
		string		versionStr;
		string		DPEname;
		for (uint32	rcu = 0; rcu < itsNrRCUs; rcu++) {
			uint32		rawValue = ack.settings()(rcu).getRaw();
			LOG_DEBUG(formatString("Updating rcu %d with %08lX", rcu, rawValue));
			// update all RCU variables
			itsRCUs[rcu]->setValue(PN_RCU_DELAY, 
						GCFPVUnsigned(uint32(rawValue & DELAY_MASK)),
						double(ack.timestamp), false);
			itsRCUs[rcu]->setValue(PN_RCU_INPUT_ENABLE, 
						GCFPVBool(rawValue & INPUT_ENABLE_MASK),
						double(ack.timestamp), false);
			itsRCUs[rcu]->setValue(PN_RCU_LBL_ENABLE, 
						GCFPVBool(rawValue & LBL_ANT_POWER_MASK), double(ack.timestamp), false);
			itsRCUs[rcu]->setValue(PN_RCU_LBH_ENABLE, 
						GCFPVBool(rawValue & LBH_ANT_POWER_MASK), double(ack.timestamp), false);
			itsRCUs[rcu]->setValue(PN_RCU_HBA_ENABLE, 
						GCFPVBool(rawValue & HBA_ANT_POWER_MASK), double(ack.timestamp), false);
			itsRCUs[rcu]->setValue(PN_RCU_BAND_SEL_LBA_HBA, 
						GCFPVBool(rawValue & USE_LB_MASK),
						double(ack.timestamp), false);
			itsRCUs[rcu]->setValue(PN_RCU_HBA_FILTER_SEL, 
						GCFPVUnsigned((rawValue & HB_FILTER_MASK) >> HB_FILTER_OFFSET),
						double(ack.timestamp), false);
			itsRCUs[rcu]->setValue(PN_RCU_VL_ENABLE, 
						GCFPVBool(rawValue & LB_POWER_MASK), double(ack.timestamp), false);
			itsRCUs[rcu]->setValue(PN_RCU_VH_ENABLE, 
						GCFPVBool(rawValue & HB_POWER_MASK), double(ack.timestamp), false);
			itsRCUs[rcu]->setValue(PN_RCU_VDD_VCC_ENABLE, 
						GCFPVBool(rawValue & ADC_POWER_MASK), double(ack.timestamp), false);
			itsRCUs[rcu]->setValue(PN_RCU_BAND_SEL_LBL_LBH, 
						GCFPVBool(rawValue & USE_LBH_MASK),
						double(ack.timestamp), false);
			itsRCUs[rcu]->setValue(PN_RCU_LBA_FILTER_SEL, 
						GCFPVUnsigned((rawValue & LB_FILTER_MASK) >> LB_FILTER_OFFSET),
						double(ack.timestamp), false);
			itsRCUs[rcu]->setValue(PN_RCU_ATTENUATION, 
						GCFPVUnsigned(uint32((rawValue & ATT_MASK) >> ATT_OFFSET)),
						double(ack.timestamp), false);
			itsRCUs[rcu]->flush();
			setObjectState(getName(), itsRCUs[rcu]->getFullScope(), (rawValue & ADC_POWER_MASK) ? 
															RTDB_OBJ_STATE_OPERATIONAL : RTDB_OBJ_STATE_OFF);
		} // for all boards

		LOG_DEBUG ("Updated all RCU information, waiting for next cycle");
//		itsOwnPropertySet->setValue(PN_HWM_RSP_ERROR,GCFPVString(""));
		TRAN(RSPMonitor::waitForNextCycle);			// go to next state.
	}
	break;

	case F_DISCONNECTED:
		_disconnectedHandler(port);		// might result in transition to connect2RSP
		break;

	case DP_SET:
		break;

	case F_QUIT:
		TRAN (RSPMonitor::finish_state);
		break;

	default:
		LOG_DEBUG("askRCUinfo, DEFAULT");
		break;
	}

	return (status);
}

//
// waitForNextCycle(event, port)
//
// Take subscription on clock modifications
//
GCFEvent::TResult RSPMonitor::waitForNextCycle(GCFEvent& event, 
													GCFPortInterface& port)
{
	if (eventName(event) != "DP_SET") {
		LOG_DEBUG_STR ("waitForNextCycle:" << eventName(event) << "@" << port.getName());	}

	GCFEvent::TResult status = GCFEvent::HANDLED;
  
	switch (event.signal) {
	case F_ENTRY: {
		itsOwnPropertySet->setValue(PN_FSM_CURRENT_ACTION,GCFPVString("RSP:wait for next cycle"));
		int		waitTime = itsPollInterval - (time(0) % itsPollInterval);
		if (waitTime == 0) {
			waitTime = itsPollInterval;
		}
		itsTimerPort->cancelAllTimers();
		itsTimerPort->setTimer(double(waitTime));
		LOG_INFO_STR("RSP:Waiting " << waitTime << " seconds for next cycle");
	}
	break;

	case F_DISCONNECTED:
		_disconnectedHandler(port);		// might result in transition to connect2RSP
	break;

	case F_TIMER: {
		TRAN(RSPMonitor::askRSPinfo);
	}
	break;

	case DP_SET:
		break;

	case F_QUIT:
		TRAN (RSPMonitor::finish_state);
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
void RSPMonitor::_disconnectedHandler(GCFPortInterface& port)
{
	port.close();
	if (&port == itsRSPDriver) {
		LOG_WARN("RSP:Connection with RSPDriver failed, going to reconnect state");
		itsOwnPropertySet->setValue(PN_FSM_ERROR,GCFPVString("RSP:connection lost"));
		TRAN (RSPMonitor::connect2RSP);
	}
}

//
// finish_state(event, port)
//
// Write controller state to PVSS
//
GCFEvent::TResult RSPMonitor::finish_state(GCFEvent& event, GCFPortInterface& port)
{
	LOG_DEBUG_STR ("finish_state:" << eventName(event) << "@" << port.getName());

	GCFEvent::TResult status = GCFEvent::HANDLED;

	switch (event.signal) {
	case F_INIT:
		break;

	case F_ENTRY: {
		// update PVSS
		itsOwnPropertySet->setValue(string(PN_FSM_CURRENT_ACTION),GCFPVString("RSP:finished"));
//		itsOwnPropertySet->setValue(string(PN_HWM_RSP_ERROR),GCFPVString(""));
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


}; // StationCU
}; // LOFAR
