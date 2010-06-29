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
#include <Common/LofarLocators.h>
#include <Common/StringUtil.h>
#include <Common/ParameterSet.h>

#include <GCF/PVSS/GCF_PVTypes.h>
#include <MACIO/MACServiceInfo.h>
#include <APL/APLCommon/ControllerDefines.h>
#include <APL/APLCommon/AntennaMapper.h>
#include <APL/RTDBCommon/RTDButilities.h>
#include <APL/RSP_Protocol/RSP_Protocol.ph>
#include <GCF/RTDB/DP_Protocol.ph>
//#include <APL/APLCommon/StationInfo.h>
#include <signal.h>
#include <unistd.h>	// usleep

#include "RSPMonitor.h"
#include "RCUConstants.h"
#include "PVSSDatapointDefs.h"

#define MAX2(a,b)	((a) > (b)) ? (a) : (b)

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
	itsDPservice		(0),
	itsPollInterval		(15),
	itsNrRCUs			(0),
	itsNrRSPboards		(0),
	itsNrSubracks		(0),
	itsNrCabinets		(0),
	itsRCUquery			(0),
	itsAntMapper		(0)
{
	LOG_TRACE_OBJ_STR (cntlrName << " construction");

	// need port for timers.
	itsTimerPort = new GCFTimerPort(*this, "TimerPort");

	// prepare TCP port to RSPDriver.
	itsRSPDriver = new GCFTCPPort (*this, MAC_SVCMASK_RSPDRIVER,
											GCFPortInterface::SAP, RSP_PROTOCOL);
	ASSERTSTR(itsRSPDriver, "Cannot allocate TCPport to RSPDriver");
	itsRSPDriver->setInstanceNr(0);

	itsDPservice = new DPservice(this);
	ASSERTSTR(itsDPservice, "Can't allocate DPservice");

	itsSplitters.reset();
}


//
// ~RSPMonitor()
//
RSPMonitor::~RSPMonitor()
{
	LOG_TRACE_OBJ_STR (getName() << " destruction");

	// database should be ready by now, check if allocation was succesfull
	for (int	cabinet = itsNrCabinets - 1; cabinet >= 0; cabinet--) {
		delete itsCabinets[cabinet];
	}
	for (int	subrack = itsNrSubracks - 1; subrack >= 0; subrack--) {
		delete itsSubracks[subrack];
	}
	for (int	rsp = itsNrRSPboards - 1; rsp >= 0; rsp--) {
		delete itsRSPs[rsp];
	}
	for (int	rcu = itsNrRCUs - 1; rcu >= 0; rcu--) {
		delete itsRCUs[rcu];
	}

	if (itsRSPDriver)	itsRSPDriver->close();

	if (itsDPservice)	delete itsDPservice;

	if (itsTimerPort)	delete itsTimerPort;

	if (itsRSPDriver)	delete itsRSPDriver;

	if (itsAntMapper)	delete itsAntMapper;
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
			itsOwnPropertySet->setValue(PN_FSM_ERROR,  GCFPVString(""));
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
// Ask the configuration of the station
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
		// TODO cheack status?
	
		// calc size of the propertyset vectors
		itsNrRCUs	   = ack.n_rcus;
		itsNrRSPboards = ack.n_rspboards;
		itsNrSubracks  = (itsNrRSPboards/NR_RSPBOARDS_PER_SUBRACK) + 
						 	((itsNrRSPboards%NR_RSPBOARDS_PER_SUBRACK) ? 1 : 0);
		itsNrCabinets  = (itsNrSubracks /NR_SUBRACKS_PER_CABINET)  + 
						 	((itsNrSubracks%NR_SUBRACKS_PER_CABINET) ? 1 : 0);
		// construct a mask containing all available RSPboards.
		itsRSPmask.reset();
		for(uint32	rsp = 0; rsp < itsNrRSPboards; rsp++) {
			itsRSPmask.set(rsp);
		}
		// construct a mask containing all available RCUs.
		itsRCUmask.reset();
		for(uint32	rcu = 0; rcu < itsNrRCUs; rcu++) {
			itsRCUmask.set(rcu);
		}
		
		// Read number of Antenna's from RemoteStation.conf file.
		ConfigLocator	CL;
		ParameterSet	RSconf(CL.locate("RemoteStation.conf"));
		itsNrHBAs = RSconf.getInt("RS.N_HBAS", 0);
		itsNrLBAs = RSconf.getInt("RS.N_LBAS", 0);
		itsHasSplitters = RSconf.getBool("RS.HBA_SPLIT", false);

		// inform user
		LOG_INFO(formatString("nr RCUs      = %d",ack.n_rcus));
		LOG_INFO(formatString("nr RSPboards = %d",ack.max_rspboards));
		LOG_INFO(formatString("nr Subracks  = %d", itsNrSubracks));
		LOG_INFO(formatString("nr Cabinets  = %d", itsNrCabinets));
		LOG_INFO(formatString("nr LBAs      = %d", itsNrLBAs));
		LOG_INFO(formatString("nr HBAs      = %d", itsNrHBAs));
		LOG_INFO_STR(         "RSPmask      = " << itsRSPmask);
		LOG_INFO(formatString("has splitters= %s", (itsHasSplitters ? "yes" : "no")));
	
		// do some checks
		if (itsNrRSPboards != (uint32)ack.max_rspboards) {
			LOG_WARN_STR("RSP:RSPdriver only controls " << itsNrRSPboards << " of " << ack.max_rspboards 
						<< " RSPboards, cannot monitor full station");
		}
		if (itsNrRSPboards * NR_RCUS_PER_RSPBOARD != itsNrRCUs) {
			LOG_INFO_STR("RSP:Station not fully equiped, only " << itsNrRCUs << " of " 
						<< itsNrRSPboards * NR_RCUS_PER_RSPBOARD << "RCUs");
		}
		if (itsNrLBAs > (itsNrRCUs / 2)) {
			LOG_INFO_STR("LBA antennas are connected to LBL and LBH inputs of the RCUs");
		}
		else {
			LOG_INFO_STR("LBL inputs of the RCUs are not used.");
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
// Create PropertySets for all hardware.
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
		// Note: when we are REconnected to the RSPdriver we already have the 
		// propertySet allocated. So only create a PS when we don't have one yet.
		for (uint32	rcu = 0; rcu < itsNrRCUs; rcu++) {
			// new cabinet?
			if (rcu % (NR_RCUS_PER_CABINET) == 0) {
				cabinet++;
				string	PSname(formatString(cabinetNameMask.c_str(), cabinet));
				if (!itsCabinets[cabinet]) {
					itsCabinets[cabinet] = new RTDBPropertySet(PSname, PST_CABINET, PSAT_WO | PSAT_CW, this);
				}
			}

			// new subrack?
			if (rcu % (NR_RCUS_PER_SUBRACK) == 0) {
				subrack++;
				string	PSname(formatString(subrackNameMask.c_str(), cabinet, subrack));
				if (!itsSubracks[subrack]) {
					itsSubracks[subrack] = new RTDBPropertySet(PSname, PST_SUB_RACK, PSAT_WO | PSAT_CW, this);
				}
			}

			// new RSPboard?
			if (rcu % (NR_RCUS_PER_RSPBOARD) == 0) {
				RSP++;
				string	PSname(formatString(rspboardNameMask.c_str(), cabinet, subrack, RSP));
				if (!itsRSPs[RSP]) {
					itsRSPs[RSP] = new RTDBPropertySet(PSname, PST_RSP_BOARD, PSAT_WO | PSAT_CW, this);
				}
			}

			// allocate RCU PS
			string	PSname(formatString(rcuNameMask.c_str(), cabinet, subrack, RSP, rcu));
			if (!itsRCUs[rcu]) {
				itsRCUs[rcu] = new RTDBPropertySet(PSname, PST_RCU, PSAT_WO | PSAT_CW, this);
			}
			usleep (2000); // wait 2 ms in order not to overload the system  
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
		LOG_INFO_STR("Allocation of all propertySets successfull, going to subscribe to RCU states");
//		itsOwnPropertySet->setValue(PN_HWM_RSP_ERROR,GCFPVString(""));
		TRAN(RSPMonitor::subscribeToRCUs);
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
// subscribeToRCUs(event, port)
//
// Get a subscription to the state of the RCU's
//
GCFEvent::TResult RSPMonitor::subscribeToRCUs(GCFEvent& event, 
											  GCFPortInterface& port)
{
	LOG_DEBUG_STR ("subscribeToRCUs:" << eventName(event) << "@" << port.getName());

	GCFEvent::TResult status = GCFEvent::HANDLED;
  
	switch (event.signal) {

	case F_ENTRY: {
		if (itsRCUquery) {
			// still have the query connected (in case of a REconnect of the RSPDriver)
			TRAN(RSPMonitor::askVersion);
			return (status);
		}

		itsOwnPropertySet->setValue(PN_FSM_CURRENT_ACTION,GCFPVString("RSP:Subscribe to RCUstates"));
		// time to init our RCU admin
		itsAntMapper = new AntennaMapper(itsNrRCUs, itsNrLBAs, itsNrHBAs);
		itsRCUstates.resize(itsNrRCUs);
		itsRCUstates = RTDB_OBJ_STATE_OFF;
		itsRCUInputStates.resize(itsNrRCUs, AntennaMapper::RI_MAX_INPUTS);
		itsRCUInputStates = false;

		itsDPservice->query("'LOFAR_PIC_*.status.state'", "_DPT=\"RCU\"");
		itsTimerPort->setTimer(5.0);	// give database some time to finish the job
	}
	break;

	case F_TIMER: {
		ASSERTSTR(itsRCUquery, "No response on the query for the RCU states");
		LOG_DEBUG_STR("No initial values received for RCU states, going to ask version info");
		TRAN(RSPMonitor::askVersion);
	}
	break;

	case F_DISCONNECTED:
		_disconnectedHandler(port);		// might result in transition to connect2RSP
	break;

	case DP_QUERY_SUBSCRIBED: {
		DPQuerySubscribedEvent		DPevent(event);
		ASSERTSTR(DPevent.result == SA_NO_ERROR, "Query on RCU states returned error code " << DPevent.result);
		itsRCUquery = DPevent.QryID;
		LOG_DEBUG_STR("RCU queryID = " << itsRCUquery << ", waiting for first response");
	}
	break;

	case DP_QUERY_CHANGED: {
		_doQueryChanged(event);
		itsTimerPort->cancelAllTimers();
		LOG_DEBUG_STR("States of RCU's received, going to ask version info");
		TRAN(RSPMonitor::askVersion);
	}
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
// Ask the firmware version of the boards
//
GCFEvent::TResult RSPMonitor::askVersion(GCFEvent& event, 
													GCFPortInterface& port)
{
	LOG_DEBUG_STR ("askVersion:" << eventName(event) << "@" << port.getName());

	GCFEvent::TResult status = GCFEvent::HANDLED;
  
	switch (event.signal) {

	case F_ENTRY: 
	case F_TIMER: {
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
		if ((ack.status != RSP_SUCCESS) || (ack.versions.bp()(0).rsp_version == 0)) {
			LOG_ERROR_STR ("RSP:Failed to get the version information, retry in 5 seconds.");
			itsOwnPropertySet->setValue(PN_FSM_ERROR,GCFPVString("RSP:getVersion error"));
			itsTimerPort->setTimer(5.0);
			break;
		}

		// move the information to the database.
		string		versionStr;
		string		DPEname;
		for (uint32	rsp = 0; rsp < itsNrRSPboards; rsp++) {
			// RSP board version
			versionStr = formatString("%d", ack.versions.bp()(rsp).rsp_version);
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
		TRAN(RSPMonitor::askSplitterInfo);				// go to next state.
		break;
	}

	case F_DISCONNECTED:
		_disconnectedHandler(port);		// might result in transition to connect2RSP
		break;

	case DP_SET:
		break;

	case DP_QUERY_CHANGED:
		_doQueryChanged(event);
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
// askSplitterInfo(event, port)
//
// Ask the information of the RSP boards
//
GCFEvent::TResult RSPMonitor::askSplitterInfo(GCFEvent& event, 
											  GCFPortInterface& port)
{
	LOG_DEBUG_STR ("askSplitterInfo:" << eventName(event) << "@" << port.getName());

	GCFEvent::TResult status = GCFEvent::HANDLED;
  
	switch (event.signal) {

	case F_ENTRY: {
		if (!itsHasSplitters) {
			TRAN(RSPMonitor::askRSPinfo);
			return (status);
		}

		itsOwnPropertySet->setValue(PN_FSM_CURRENT_ACTION,GCFPVString("RSP:updating splitter info"));

		RSPGetsplitterEvent		getSplitter;
		getSplitter.timestamp.setNow();
		itsRSPDriver->send(getSplitter);
		itsTimerPort->setTimer(5.0);		// in case the answer never comes.
	}
	break;

	case RSP_GETSPLITTERACK: {
		itsTimerPort->cancelAllTimers();
		RSPGetsplitterackEvent		ack(event);
		if (ack.status != RSP_SUCCESS) {
			LOG_ERROR ("RSP:Failed to get the splitter information. Trying status information");
			itsOwnPropertySet->setValue(PN_FSM_ERROR,GCFPVString("RSP:getSplitter error"));
		}
		else {
			itsSplitters = ack.splitter;		// save for later
		}
		TRAN (RSPMonitor::askRSPinfo);
	}
	break;

	case F_TIMER: 
		LOG_ERROR_STR("RSP:Timeout on getting the splitter information, trying status information");
		itsOwnPropertySet->setValue(PN_FSM_ERROR,GCFPVString("RSP:getSplitter timeout"));
		TRAN (RSPMonitor::askRSPinfo);
		break;

	case F_DISCONNECTED:
		_disconnectedHandler(port);		// might result in transition to connect2RSP
		break;

	case DP_SET:
		break;

	case DP_QUERY_CHANGED:
		_doQueryChanged(event);
		break;

	case F_QUIT:
		TRAN (RSPMonitor::finish_state);
		break;

	default:
		LOG_DEBUG_STR ("askSplitterInfo, DEFAULT");
		break;
	}    

	return (status);
}

//
// askRSPinfo(event, port)
//
// Ask the information of the RSP boards
//
GCFEvent::TResult RSPMonitor::askRSPinfo(GCFEvent& event, 
										 GCFPortInterface& port)
{
	LOG_DEBUG_STR ("askRSPinfo:" << eventName(event) << "@" << port.getName());

	GCFEvent::TResult status = GCFEvent::HANDLED;
  
	switch (event.signal) {

	case F_ENTRY: {
		itsOwnPropertySet->setValue(PN_FSM_CURRENT_ACTION,GCFPVString("RSP:updating RSP info"));

		RSPGetstatusEvent	getStatus;
		getStatus.timestamp.setNow();
		getStatus.cache = true;
		getStatus.rspmask = itsRSPmask;
		itsRSPDriver->send(getStatus);
		itsTimerPort->setTimer(5.0);		// in case the answer never comes
	}
	break;

	case F_TIMER:
		LOG_ERROR_STR ("RSP:Timeout on getting the status information, trying other information");
		itsOwnPropertySet->setValue(PN_FSM_ERROR,GCFPVString("RSP:getStatus timeout"));
		TRAN(RSPMonitor::askRCUinfo);				// go to next state.
		break;

	case RSP_GETSTATUSACK: {
		itsTimerPort->cancelAllTimers();
		RSPGetstatusackEvent		ack(event);
		if (ack.status != RSP_SUCCESS) {
			LOG_ERROR ("RSP:Failed to get the status information. Trying other information");
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
									double(ack.timestamp), false);
			itsRSPs[rsp]->setValue(PN_RSP_VOLTAGE25, GCFPVDouble(double(bStat.rsp.voltage_2_5)*(3.3/192.0)), 
									double(ack.timestamp), false);
			itsRSPs[rsp]->setValue(PN_RSP_VOLTAGE33, GCFPVDouble(double(bStat.rsp.voltage_3_3)*(5.0/192.0)), 
									double(ack.timestamp), false);
			
			// Ethernet status
			itsRSPs[rsp]->setValue(PN_RSP_ETHERNET_PACKETS_RECEIVED, GCFPVUnsigned(bStat.eth.nof_frames), 
									double(ack.timestamp), false);
			itsRSPs[rsp]->setValue(PN_RSP_ETHERNET_PACKETS_ERROR,    GCFPVUnsigned(bStat.eth.nof_errors), 
									double(ack.timestamp), false);
			itsRSPs[rsp]->setValue(PN_RSP_ETHERNET_LAST_ERROR,       GCFPVUnsigned(bStat.eth.last_error), 
									double(ack.timestamp), false);
			setObjectState(getName(), itsRSPs[rsp]->getFullScope()+".Ethernet", (bStat.eth.nof_frames != 0) ? 
															RTDB_OBJ_STATE_OPERATIONAL : RTDB_OBJ_STATE_OFF);
			
			// MEP status
			itsRSPs[rsp]->setValue(PN_RSP_MEP_SEQNR, GCFPVUnsigned(bStat.mep.seqnr), 
									double(ack.timestamp), false);
			itsRSPs[rsp]->setValue(PN_RSP_MEP_ERROR, GCFPVUnsigned(bStat.mep.error), 
									double(ack.timestamp), false);

			// BP status
			itsRSPs[rsp]->setValue(PN_RSP_BP_TEMPERATURE, GCFPVDouble(double(bStat.rsp.bp_temp)), 
									double(ack.timestamp), false);
			// AP0
			itsRSPs[rsp]->setValue(PN_RSP_AP0_TEMPERATURE,		 GCFPVDouble(double(bStat.rsp.ap0_temp)), 
									double(ack.timestamp), false);
			itsRSPs[rsp]->setValue(PN_RSP_AP0_SYNC_SAMPLE_COUNT, GCFPVUnsigned(bStat.ap0_sync.sample_offset), 
									double(ack.timestamp), false);
			itsRSPs[rsp]->setValue(PN_RSP_AP0_SYNC_SYNC_COUNT,   GCFPVUnsigned(bStat.ap0_sync.sample_offset), 
									double(ack.timestamp), false);
			itsRSPs[rsp]->setValue(PN_RSP_AP0_SYNC_ERROR_COUNT,  GCFPVUnsigned(bStat.ap0_sync.ext_count), 
									double(ack.timestamp), false);

			// AP1
			itsRSPs[rsp]->setValue(PN_RSP_AP1_TEMPERATURE,		 GCFPVDouble(double(bStat.rsp.ap1_temp)), 
									double(ack.timestamp), false);
			itsRSPs[rsp]->setValue(PN_RSP_AP1_SYNC_SAMPLE_COUNT, GCFPVUnsigned(bStat.ap1_sync.sample_offset), 
									double(ack.timestamp), false);
			itsRSPs[rsp]->setValue(PN_RSP_AP1_SYNC_SYNC_COUNT,   GCFPVUnsigned(bStat.ap1_sync.sample_offset), 
									double(ack.timestamp), false);
			itsRSPs[rsp]->setValue(PN_RSP_AP1_SYNC_ERROR_COUNT,  GCFPVUnsigned(bStat.ap1_sync.ext_count), 
									double(ack.timestamp), false);

			// AP2
			itsRSPs[rsp]->setValue(PN_RSP_AP2_TEMPERATURE,		 GCFPVDouble(double(bStat.rsp.ap2_temp)), 
									double(ack.timestamp), false);
			itsRSPs[rsp]->setValue(PN_RSP_AP2_SYNC_SAMPLE_COUNT, GCFPVUnsigned(bStat.ap2_sync.sample_offset), 
									double(ack.timestamp), false);
			itsRSPs[rsp]->setValue(PN_RSP_AP2_SYNC_SYNC_COUNT,   GCFPVUnsigned(bStat.ap2_sync.sample_offset), 
									double(ack.timestamp), false);
			itsRSPs[rsp]->setValue(PN_RSP_AP2_SYNC_ERROR_COUNT,  GCFPVUnsigned(bStat.ap2_sync.ext_count), 
									double(ack.timestamp), false);

			// AP3
			itsRSPs[rsp]->setValue(PN_RSP_AP3_TEMPERATURE,		 GCFPVDouble(double(bStat.rsp.ap3_temp)), 
									double(ack.timestamp), false);
			itsRSPs[rsp]->setValue(PN_RSP_AP3_SYNC_SAMPLE_COUNT, GCFPVUnsigned(bStat.ap3_sync.sample_offset), 
									double(ack.timestamp), false);
			itsRSPs[rsp]->setValue(PN_RSP_AP3_SYNC_SYNC_COUNT,   GCFPVUnsigned(bStat.ap3_sync.sample_offset), 
									double(ack.timestamp), false);
			itsRSPs[rsp]->setValue(PN_RSP_AP3_SYNC_ERROR_COUNT,  GCFPVUnsigned(bStat.ap3_sync.ext_count), 
									double(ack.timestamp), false);

			// finally set the splitter
			itsRSPs[rsp]->setValue(PN_RSP_SPLITTER_ON,			 GCFPVUnsigned(itsSplitters.test(rsp)),
									double(ack.timestamp), false);

			itsRSPs[rsp]->flush();
			usleep(1000); // wait 1 ms
		} // for all boards

		LOG_DEBUG_STR ("RSPboard information updated, going to RCU information");
//		itsOwnPropertySet->setValue(PN_HWM_RSP_ERROR,GCFPVString(""));
		TRAN(RSPMonitor::askTDstatus);				// go to next state.
		break;
	}

	case F_DISCONNECTED:
		_disconnectedHandler(port);		// might result in transition to connect2RSP
		break;

	case DP_SET:
		break;

	case DP_QUERY_CHANGED:
		_doQueryChanged(event);
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
// askTDstatus(event, port)
//
// Ask the settings of the clock board.
//
GCFEvent::TResult RSPMonitor::askTDstatus(GCFEvent& event, GCFPortInterface& port)
{
	LOG_DEBUG_STR ("askTDstatus:" << eventName(event) << "@" << port.getName());

	GCFEvent::TResult status = GCFEvent::HANDLED;
  
	switch (event.signal) {

	case F_ENTRY:  {
		itsOwnPropertySet->setValue(PN_FSM_CURRENT_ACTION,GCFPVString("RSP:getting clockboard info"));
//		itsOwnPropertySet->setValue(PN_HWM_RSP_ERROR,GCFPVString(""));
		RSPGettdstatusEvent	getTDstatus;
		getTDstatus.timestamp.setNow();
		getTDstatus.cache = true;
		getTDstatus.rspmask = itsRSPmask;
		itsRSPDriver->send(getTDstatus);
		itsTimerPort->setTimer(5.0);		// in case the answer never comes
	}
	break;
	
	case F_TIMER:
		LOG_ERROR_STR ("RSP:Timeout on getting information of the TD board, trying again in next run");
		itsOwnPropertySet->setValue(PN_FSM_ERROR,GCFPVString("RSP:getClockboard timeout"));
		TRAN(RSPMonitor::askSPUstatus);				// go to next state.
		break;

	case RSP_GETTDSTATUSACK: {
		itsTimerPort->cancelAllTimers();
		RSPGettdstatusackEvent		ack(event);
		if (ack.status != RSP_SUCCESS) {
			LOG_ERROR_STR ("RSP:Failed to get information of the TD board. Trying again in next run");
			itsOwnPropertySet->setValue(PN_FSM_ERROR,GCFPVString("RSP:getClockboard error"));
			// mark the boards as offline
			for (uint32	subrack = 0; subrack < itsNrSubracks; subrack++) {
				setObjectState(getName(), itsSubracks[subrack]->getFullScope()+".clockBoard", RTDB_OBJ_STATE_OFF);
			}
			TRAN(RSPMonitor::askSPUstatus);				// go to next state.
			break;
		}

		// move the information to the database.
		// Note: there is only one RSPboard in each subrack that controls the clockboard.
		// The answer contains vales for all RSPBoards.
		string		versionStr;
		string		DPEname;
		for (uint32	rsp = 0; rsp < itsNrRSPboards; rsp++) {
			TDBoardStatus&	boardStatus = ack.tdstatus.board()(rsp);
			if (boardStatus.invalid) {		// does this board control the clockboard?
				continue;					// no, try next
			}

			// This board controls the clockboard.
			int	subrack = rsp/4;
			// set state first
			setObjectState(getName(), itsSubracks[subrack]->getFullScope()+".clockBoard", 
							(boardStatus.unknown) ?  RTDB_OBJ_STATE_OFF : RTDB_OBJ_STATE_OPERATIONAL);

			if (boardStatus.unknown) {
				LOG_WARN_STR("ClockBoard information of subrack " << subrack << " not available");
				continue;
			}

			itsSubracks[subrack]->setValue(PN_SRCK_CLOCK_BOARD_TEMPERATURE, 
									GCFPVDouble(boardStatus.temperature), double(ack.timestamp), false);
			itsSubracks[subrack]->setValue(PN_SRCK_CLOCK_BOARD_LOCK160, 
									GCFPVBool(boardStatus.pll_160MHz_locked), double(ack.timestamp), false);
			itsSubracks[subrack]->setValue(PN_SRCK_CLOCK_BOARD_LOCK200, 
									GCFPVBool(boardStatus.pll_200MHz_locked), double(ack.timestamp), false);
			itsSubracks[subrack]->setValue(PN_SRCK_CLOCK_BOARD_FREQ, 
									GCFPVInteger(boardStatus.output_clock ? 200 : 160), double(ack.timestamp), false);
			itsSubracks[subrack]->setValue(PN_SRCK_CLOCK_BOARD__VFSP, 
									GCFPVDouble(boardStatus.v2_5 * 3.3 / 192.0), double(ack.timestamp), false);
			itsSubracks[subrack]->setValue(PN_SRCK_CLOCK_BOARD__VCLOCK, 
									GCFPVDouble(boardStatus.v3_3 * 5.0 / 192.0), double(ack.timestamp), false);
			itsSubracks[subrack]->flush();
			usleep(1000); // wait 1 ms

			// copy clock settings for later.
			itsClock = boardStatus.output_clock ? 200 : 160;
		}

		LOG_DEBUG_STR ("Clockboard information updated, going to RCU status information");
//		itsOwnPropertySet->setValue(PN_HWM_RSP_ERROR,GCFPVString(""));
		TRAN(RSPMonitor::askSPUstatus);				// go to next state.
		break;
	}

	case F_DISCONNECTED:
		_disconnectedHandler(port);		// might result in transition to connect2RSP
		break;

	case DP_SET:
		break;

	case DP_QUERY_CHANGED:
		_doQueryChanged(event);
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
// askSPUstatus(event, port)
//
// Read the settings from the Station Power Unit
//
GCFEvent::TResult RSPMonitor::askSPUstatus(GCFEvent& event, GCFPortInterface& port)
{
	LOG_DEBUG_STR ("askSPUstatus:" << eventName(event) << "@" << port.getName());

	GCFEvent::TResult status = GCFEvent::HANDLED;
  
	switch (event.signal) {

	case F_ENTRY:  {
		itsOwnPropertySet->setValue(PN_FSM_CURRENT_ACTION,GCFPVString("RSP:getting power info"));
//		itsOwnPropertySet->setValue(PN_HWM_RSP_ERROR,GCFPVString(""));
		RSPGetspustatusEvent	getSPUstatus;
		getSPUstatus.timestamp.setNow();
		getSPUstatus.cache = true;
		itsRSPDriver->send(getSPUstatus);
		itsTimerPort->setTimer(5.0);		// in case the answer never comes.
	}
	break;

	case F_TIMER:
		LOG_ERROR_STR ("RSP:Failed to get information of the power board, trying again in next run");
		itsOwnPropertySet->setValue(PN_FSM_ERROR,GCFPVString("RSP:get powerboard timeout"));
		TRAN(RSPMonitor::askRCUinfo);				// go to next state.
		break;

	case RSP_GETSPUSTATUSACK: {
		itsTimerPort->cancelAllTimers();
		RSPGetspustatusackEvent		ack(event);
		if (ack.status != RSP_SUCCESS) {
			LOG_ERROR ("RSP:Failed to get information of the power board. Trying again in next run");
			itsOwnPropertySet->setValue(PN_FSM_ERROR,GCFPVString("RSP:get powerboard error"));
			// mark the boards as off-line
			for (uint32	subrack = 0; subrack < itsNrSubracks; subrack++) {
				setObjectState(getName(), itsSubracks[subrack]->getFullScope()+".SPU", RTDB_OBJ_STATE_OFF);
			}
			TRAN(RSPMonitor::askRCUinfo);				// go to next state.
			break;
		}

		// move the information to the database.
		for (uint32	subrack = 0; subrack < itsNrSubracks; subrack++) {
			SPUBoardStatus&	boardStatus = ack.spustatus.subrack()(subrack);

			itsSubracks[subrack]->setValue(PN_SRCK_SPU_TEMPERATURE, 
									GCFPVDouble(boardStatus.temperature), double(ack.timestamp), false);
			itsSubracks[subrack]->setValue(PN_SRCK_SPU__VDIG,  // RCU
									GCFPVDouble(boardStatus.v2_5 * 2.5 / 192.0 * 2.0), double(ack.timestamp), false);
			itsSubracks[subrack]->setValue(PN_SRCK_SPU__VLBA,  // LBA
									GCFPVDouble(boardStatus.v3_3 * 3.3 / 192.0 * 3.0), double(ack.timestamp), false);
			itsSubracks[subrack]->setValue(PN_SRCK_SPU__VHBA,  // HBA
									GCFPVDouble(boardStatus.v12 * 12.0 / 192.0 * 4.01), double(ack.timestamp), false);
			itsSubracks[subrack]->flush();
			usleep(1000); // wait 1 ms

			setObjectState(getName(), itsSubracks[subrack]->getFullScope()+".SPU", RTDB_OBJ_STATE_OPERATIONAL);
		}

		LOG_DEBUG_STR ("Powerboard information updated, going to RCU status information");
		TRAN(RSPMonitor::askRCUinfo);				// go to next state.
		break;
	}

	case F_DISCONNECTED:
		_disconnectedHandler(port);		// might result in transition to connect2RSP
		break;

	case DP_SET:
		break;

	case DP_QUERY_CHANGED:
		_doQueryChanged(event);
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
// askRCUinfo(event, port)
//
// Get the info of the RCU's
//
GCFEvent::TResult RSPMonitor::askRCUinfo(GCFEvent& event, GCFPortInterface& port)
{
	if (eventName(event) != "DP_SET") {
		LOG_DEBUG_STR ("askRCUinfo:" << eventName(event) << "@" << port.getName());
	}

	GCFEvent::TResult status = GCFEvent::HANDLED;

	switch (event.signal) {
	case F_ENTRY: {
		itsOwnPropertySet->setValue(PN_FSM_CURRENT_ACTION,GCFPVString("RSP:updating RSP info"));

		RSPGetrcuEvent	getStatus;
		getStatus.timestamp.setNow();
		getStatus.cache   = true;
		getStatus.rcumask = itsRCUmask;
		itsRSPDriver->send(getStatus);
		itsTimerPort->setTimer(5.0);		// in case the answer never comes
		break;
	}

	case F_TIMER:
		LOG_ERROR_STR ("RSP:Timeout on getting the RCU information, trying other information");
		itsOwnPropertySet->setValue(PN_FSM_ERROR,GCFPVString("RSP:getRCU timeout"));
		TRAN(RSPMonitor::waitForNextCycle);			// go to next state.
		break;

	case RSP_GETRCUACK: {
		itsTimerPort->cancelAllTimers();
		RSPGetrcuackEvent	ack(event);
		if (ack.status != RSP_SUCCESS) {
			LOG_ERROR ("RSP:Failed to get the RCU information. Trying other information");
			itsOwnPropertySet->setValue(PN_FSM_ERROR,GCFPVString("RSP:getRCU error"));
			TRAN(RSPMonitor::waitForNextCycle);			// go to next state.
			break;
		}

		// move the information to the database.
		string		versionStr;
		string		DPEname;
		for (int	rcu = 0; rcu < (int)itsNrRCUs; rcu++) {
			uint32		rawValue = ack.settings()(rcu).getRaw();
			LOG_DEBUG(formatString("Updating rcu %d with %08lX", rcu, rawValue));
			// update all RCU variables
			itsRCUs[rcu]->setValue(PN_RCU_DELAY, 
						GCFPVDouble((1000.0 / itsClock) * uint32(rawValue & DELAY_MASK)),
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
						GCFPVDouble(0.25 * uint32((rawValue & ATT_MASK) >> ATT_OFFSET)),
						double(ack.timestamp), false);
			itsRCUs[rcu]->flush();
			usleep(1000); // wait 1 ms
#if 0
			if (rcu == 0) {
				LOG_DEBUG_STR("PN_RCU_DELAY        (0x00007F):" << uint32(rawValue & DELAY_MASK));
                LOG_DEBUG_STR("PN_RCU_INPUT_ENABLE (0x000080):" << (rawValue & INPUT_ENABLE_MASK));
                LOG_DEBUG_STR("PN_RCU_LBL_ENABLE   (0x000100):" << (rawValue & LBL_ANT_POWER_MASK));
                LOG_DEBUG_STR("PN_RCU_LBH_ENABLE   (0x000200):" << (rawValue & LBH_ANT_POWER_MASK));
                LOG_DEBUG_STR("PN_RCU_HBA_ENABLE   (0x000400):" << (rawValue & HBA_ANT_POWER_MASK));
                LOG_DEBUG_STR("PN_RCU_SEL_LBA_HBA  (0x000800):" << (rawValue & USE_LB_MASK));
                LOG_DEBUG_STR("PN_RCU_HBAFILTERSEL (0x003000):" << ((rawValue & HB_FILTER_MASK) >> HB_FILTER_OFFSET));
                LOG_DEBUG_STR("PN_RCU_VL_ENABLE    (0x004000):" << (rawValue & LB_POWER_MASK));
                LOG_DEBUG_STR("PN_RCU_VH_ENABLE    (0x008000):" << (rawValue & HB_POWER_MASK));
                LOG_DEBUG_STR("PN_RCU_VDD_VCC_EN   (0x010000):" << (rawValue & ADC_POWER_MASK));
                LOG_DEBUG_STR("PN_RCU_SEL_LBL_LBH  (0x020000):" << (rawValue & USE_LBH_MASK));
                LOG_DEBUG_STR("PN_RCU_LBAFILTERSEL (0x040000):" << ((rawValue & LB_FILTER_MASK) >> LB_FILTER_OFFSET));
                LOG_DEBUG_STR("PN_RCU_ATTENUATION  (0xF80000):" << (uint32((rawValue & ATT_MASK) >> ATT_OFFSET)));
			}
#endif
			setObjectState(getName(), itsRCUs[rcu]->getFullScope(), (rawValue & ADC_POWER_MASK) ? 
															RTDB_OBJ_STATE_OPERATIONAL : RTDB_OBJ_STATE_OFF);

			// update own RCU admin also
			itsRCUInputStates(rcu, AntennaMapper::RI_LBL) = rawValue & LBL_ANT_POWER_MASK;
			itsRCUInputStates(rcu, AntennaMapper::RI_LBH) = rawValue & LBH_ANT_POWER_MASK;
			itsRCUInputStates(rcu, AntennaMapper::RI_HBA) = rawValue & HBA_ANT_POWER_MASK;
			itsRCUstates(rcu) = (rawValue & ADC_POWER_MASK) ? RTDB_OBJ_STATE_OPERATIONAL : RTDB_OBJ_STATE_OFF;

		} // for all boards

		LOG_DEBUG ("Updated all RCU information, updating antenna states");
		for (uint ant = 0; ant < itsNrLBAs; ant++) {
			if (itsRCUInputStates(itsAntMapper->XRCU(ant), itsAntMapper->RCUinput(ant, AntennaMapper::AT_LBA)) ||
				itsRCUInputStates(itsAntMapper->YRCU(ant), itsAntMapper->RCUinput(ant, AntennaMapper::AT_LBA))) {
				setObjectState(getName(), formatString("LBA%d", ant), 
								MAX2(itsRCUstates(itsAntMapper->XRCU(ant)), itsRCUstates(itsAntMapper->YRCU(ant)) ) );
			}
			else {
				setObjectState(getName(), formatString("LBA%d", ant), RTDB_OBJ_STATE_OFF);
			}
		}
		for (uint ant = 0; ant < itsNrHBAs; ant++) {
			if (itsRCUInputStates(itsAntMapper->XRCU(ant), itsAntMapper->RCUinput(ant, AntennaMapper::AT_HBA)) ||
				itsRCUInputStates(itsAntMapper->YRCU(ant), itsAntMapper->RCUinput(ant, AntennaMapper::AT_HBA))) {
				setObjectState(getName(), formatString("HBA%d", ant), 
								MAX2(itsRCUstates(itsAntMapper->XRCU(ant)), itsRCUstates(itsAntMapper->YRCU(ant)) ) );
			}
			else {
				setObjectState(getName(), formatString("HBA%d", ant), RTDB_OBJ_STATE_OFF);
			}
		}

		LOG_DEBUG ("Updated all station information, waiting for next cycle");
//		itsOwnPropertySet->setValue(PN_HWM_RSP_ERROR,GCFPVString(""));
		TRAN(RSPMonitor::waitForNextCycle);			// go to next state.
	}
	break;

	case F_DISCONNECTED:
		_disconnectedHandler(port);		// might result in transition to connect2RSP
		break;

	case DP_SET:
		break;

	case DP_QUERY_CHANGED:
		_doQueryChanged(event);
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
// Wait for our next cycle.
//
GCFEvent::TResult RSPMonitor::waitForNextCycle(GCFEvent& event, 
													GCFPortInterface& port)
{
	if (eventName(event) != "DP_SET") {
		LOG_DEBUG_STR ("waitForNextCycle:" << eventName(event) << "@" << port.getName());	
	}

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
		itsOwnPropertySet->setValue(string(PN_FSM_ERROR),GCFPVString(""));
		TRAN(RSPMonitor::askSplitterInfo);
	}
	break;

	case DP_SET:
		break;

	case DP_QUERY_CHANGED:
		_doQueryChanged(event);
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
// _doQueryChanged(event)
//
void RSPMonitor::_doQueryChanged(GCFEvent&		event)
{
	DPQueryChangedEvent		DPevent(event);
	if (DPevent.result != SA_NO_ERROR) {
		LOG_ERROR_STR("PVSS reported error " << DPevent.result << " for query " << DPevent.QryID);
		return;
	}

	int				nrDPs    = ((GCFPVDynArr*)(DPevent.DPnames._pValue))->getValue().size();
	GCFPVDynArr*	DPnames  = (GCFPVDynArr*)(DPevent.DPnames._pValue);
	GCFPVDynArr*	DPvalues = (GCFPVDynArr*)(DPevent.DPvalues._pValue);
	// register all states
	for (int idx = 0; idx < nrDPs; ++idx) {
		string 				nameStr = DPnames->getValue() [idx]->getValueAsString();
		int  				status  = ((GCFPVInteger *)(DPvalues->getValue()[idx]))->getValue();
		uint				rcuNr;
		// get rcuNr from name
		// note: name is like: RS002:LOFAR_PIC_Cabinet0_Subrack0_RSPBoard0_RCU4.status.state
		string::size_type	rcuPos = nameStr.find("_RCU");
		if (rcuPos == string::npos) {
			LOG_WARN_STR("Unrecognized datapointname ignored: " << nameStr);
			continue;
		}
		if (sscanf(nameStr.substr(rcuPos).c_str(), "_RCU%d.%*s", &rcuNr) != 1) {
			LOG_WARN_STR("Unrecognized datapointname ignored: " << nameStr);
			continue;
		}

		// check rcuNr
		if (rcuNr >= itsNrRCUs) {
			LOG_WARN_STR("Illegal RCUnumber " << rcuNr << " in " << nameStr << ", ignoring status change!");
			continue;
		}

		// store for later
		itsRCUstates(rcuNr) = status;
		LOG_INFO(formatString("RCU %d = %d", rcuNr, status));
	} // for
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
