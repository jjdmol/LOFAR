//#  TBBMonitor.cc: Implementation of the MAC Scheduler task
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
//#  $Id: TBBMonitor.cc 10505 2007-09-07 17:14:57Z overeem $
#include <lofar_config.h>
#include <Common/LofarLogger.h>
#include <Common/LofarConstants.h>
#include <Common/lofar_datetime.h>
#include <Common/StringUtil.h>

#include <GCF/PVSS/GCF_PVTypes.h>
#include <MACIO/MACServiceInfo.h>
#include <APL/APLCommon/ControllerDefines.h>
#include <APL/APLCommon/APLUtilities.h>
#include <APL/RTDBCommon/RTDButilities.h>
#include <APL/TBB_Protocol/TBB_Protocol.ph>
#include <GCF/RTDB/DP_Protocol.ph>
//#include <APL/APLCommon/StationInfo.h>
#include <signal.h>

#include "TBBMonitor.h"
#include "RCUConstants.h"
#include "PVSSDatapointDefs.h"


namespace LOFAR {
	using namespace GCF::TM;
	using namespace GCF::PVSS;
	using namespace GCF::RTDB;
	using namespace APLCommon;
	using namespace APL::RTDBCommon;
	namespace StationCU {
	
//
// TBBMonitor()
//
TBBMonitor::TBBMonitor(const string&	cntlrName) :
	GCFTask 			((State)&TBBMonitor::initial_state,cntlrName),
	itsOwnPropertySet	(0),
	itsTimerPort		(0),
	itsTBBDriver		(0),
	itsPollInterval		(10),
	itsNrRCUs			(0),
	itsNrTBboards		(0)
{
	LOG_TRACE_OBJ_STR (cntlrName << " construction");

	// need port for timers.
	itsTimerPort = new GCFTimerPort(*this, "TimerPort");

	// prepare TCP port to TBBDriver.
	itsTBBDriver = new GCFTCPPort (*this, MAC_SVCMASK_TBBDRIVER,
											GCFPortInterface::SAP, TBB_PROTOCOL);
	ASSERTSTR(itsTBBDriver, "Cannot allocate TCPport to TBBDriver");
	itsTBBDriver->setInstanceNr(0);

}


//
// ~TBBMonitor()
//
TBBMonitor::~TBBMonitor()
{
	LOG_TRACE_OBJ_STR (getName() << " destruction");

	if (itsTBBDriver) {
		itsTBBDriver->close();
		delete itsTBBDriver;
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
GCFEvent::TResult TBBMonitor::initial_state(GCFEvent& event, GCFPortInterface& port)
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
		LOG_TRACE_FLOW ("Updating state to PVSS");
		itsOwnPropertySet->setValue(PN_FSM_CURRENT_ACTION, GCFPVString("TBB:initial"));
		itsOwnPropertySet->setValue(PN_HWM_TBB_CONNECTED,GCFPVBool(false));
//		itsOwnPropertySet->setValue(PN_FSM_ERROR,  GCFPVString(""));
		
		LOG_DEBUG_STR("Going to connect to the TBBDriver.");
		TRAN (TBBMonitor::connect2TBB);
	}
	
	case DP_SET:
		break;

	case F_QUIT:
		TRAN (TBBMonitor::finish_state);
		break;

	default:
		LOG_DEBUG_STR ("initial, DEFAULT");
		break;
	}    

	return (status);
}


//
// connect2TBB(event, port)
//
// Setup connection with TBBdriver
//
GCFEvent::TResult TBBMonitor::connect2TBB(GCFEvent& event, GCFPortInterface& port)
{
	LOG_DEBUG_STR ("connect2TBB:" << eventName(event) << "@" << port.getName());

	GCFEvent::TResult status = GCFEvent::HANDLED;
  
	switch (event.signal) {
	case F_ENTRY:
		// update PVSS
		itsOwnPropertySet->setValue(PN_FSM_CURRENT_ACTION, GCFPVString("TBB:connecting"));
		itsTimerPort->setTimer(2.0);	// give database some time
		break;

	case F_CONNECTED:
		if (&port == itsTBBDriver) {
			LOG_DEBUG ("Connected with TBBDriver, going to get the configuration");
//			itsOwnPropertySet->setValue(PN_FSM_ERROR,  GCFPVString(""));
			itsOwnPropertySet->setValue(PN_HWM_TBB_CONNECTED,GCFPVBool(true));
			TRAN(TBBMonitor::askConfiguration);		// go to next state.
		}
		break;

	case F_DISCONNECTED:
		port.close();
		ASSERTSTR (&port == itsTBBDriver, 
								"F_DISCONNECTED event from port " << port.getName());
		LOG_WARN("Connection with TBBDriver failed, retry in 10 seconds");
		itsOwnPropertySet->setValue(PN_FSM_ERROR, GCFPVString("TBB:connection timeout"));
		itsTimerPort->setTimer(10.0);
		break;

	case F_TIMER:
		itsTBBDriver->open();		// results in F_CONN or F_DISCON
		break;

	case DP_SET:
		break;

	case F_QUIT:
		TRAN (TBBMonitor::finish_state);
		break;

	default:
		LOG_DEBUG_STR ("connect2TBB, DEFAULT");
		break;
	}    

	return (status);
}

//
// askConfiguration(event, port)
//
// Take subscription on clock modifications
//
GCFEvent::TResult TBBMonitor::askConfiguration(GCFEvent& event, GCFPortInterface& port)
{
	LOG_DEBUG_STR ("askConfiguration:" << eventName(event) << "@" << port.getName());

	GCFEvent::TResult status = GCFEvent::HANDLED;
  
	switch (event.signal) {
	case F_ENTRY: {
		itsOwnPropertySet->setValue(PN_FSM_CURRENT_ACTION,GCFPVString("TBB:asking configuration"));
		itsTimerPort->setTimer(2.0);		// give database some time
	}
	break;

	case F_DISCONNECTED:
		_disconnectedHandler(port);		// might result in transition to connect2TBB
		break;

	case TBB_DRIVER_BUSY_ACK:
		LOG_DEBUG("TBBDriver is busy, retry in 5 seconds");
		itsTimerPort->setTimer(5.0);
		break;

	case F_TIMER: {
		TBBGetConfigEvent	getconfig;
		itsTBBDriver->send(getconfig);
	}
	break;
		
	case TBB_GET_CONFIG_ACK: {
		TBBGetConfigAckEvent	ack(event);
	
		// calc size of the propertyset vectors
		itsBoardMask   = bitset<MAX_N_TBBOARDS>(ack.active_boards_mask);
		itsNrTBboards  = ack.max_boards;
		itsNrRCUs	   = itsNrTBboards * NR_RCUS_PER_TBBOARD;

		// inform user
		LOG_INFO_STR("Active boards = " << itsBoardMask);
		LOG_INFO(formatString("nr TBboards = %d", itsNrTBboards));
	
		// do some checks
		if (itsNrTBboards != itsBoardMask.count()) {
			LOG_WARN_STR("TBB:Only " << itsBoardMask.count() << " of " << itsNrTBboards 
						<< " TBboards are available.");
		}

		LOG_DEBUG ("Going to allocate the property-sets");
//		itsOwnPropertySet->setValue(PN_FSM_ERROR,GCFPVString(""));
		TRAN(TBBMonitor::createPropertySets);				// go to next state.
		}
		break;

	case DP_SET:
		break;

	case F_QUIT:
		TRAN (TBBMonitor::finish_state);
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
// Retrieve sampleclock from TBB driver
//
GCFEvent::TResult TBBMonitor::createPropertySets(GCFEvent& event, GCFPortInterface& port)
{
	LOG_DEBUG_STR ("createPropertySets:" << eventName(event) << "@" << port.getName());

	GCFEvent::TResult status = GCFEvent::HANDLED;
  
	switch (event.signal) {

	case F_ENTRY: {
		itsOwnPropertySet->setValue(PN_FSM_CURRENT_ACTION,GCFPVString("TBB:create PropertySets"));
		// resize vectors.
		itsTBBs.resize	  (itsNrTBboards, 0);
		itsRCUs.resize	  (itsNrRCUs, 	   0);

		int32	cabinet (-1);
		int32	subrack (-1);
		int32	RSP		(-1);
		int32	TBB		(-1);
		string	tbboardNameMask(createPropertySetName(PSN_TB_BOARD, getName()));
		string	rcuNameMask    (createPropertySetName(PSN_RCU, 	  getName()));
		for (uint32	rcu = 0; rcu < itsNrRCUs; rcu++) {
			// new cabinet?
			if (rcu % (NR_RCUS_PER_CABINET) == 0) {
				cabinet++;
			}

			// new subrack?
			if (rcu % (NR_RCUS_PER_SUBRACK) == 0) {
				subrack++;
			}

			// new TBboard?
			if (rcu % (NR_RCUS_PER_TBBOARD) == 0) {
				TBB++;
				string	PSname(formatString(tbboardNameMask.c_str(), cabinet, subrack, TBB));
//				itsTBBs[TBB] = new RTDBPropertySet(PSname, PST_TB_BOARD, PSAT_WO | PSAT_CW, this);
				itsTBBs[TBB] = new RTDBPropertySet(PSname, PST_TB_BOARD, PSAT_WO, this);
				itsTBBs[TBB]->setConfirmation(false);
			}

			// new RSPboard?
			if (rcu % (NR_RCUS_PER_RSPBOARD) == 0) {
				RSP++;
			}

			// allocate RCU PS
			string	PSname(formatString(rcuNameMask.c_str(), cabinet, subrack, RSP, rcu));
//			itsRCUs[rcu] = new RTDBPropertySet(PSname, PST_RCU, PSAT_WO | PSAT_CW, this);
			itsRCUs[rcu] = new RTDBPropertySet(PSname, PST_RCU, PSAT_WO, this);
			itsRCUs[rcu]->setConfirmation(false);
		}
		itsTimerPort->setTimer(5.0);	// give database some time to finish the job
	}
	break;

	case F_TIMER: {
		// database should be ready by now, check if allocation was succesfull
		for (uint32	tbb = 0; tbb < itsNrTBboards; tbb++) {
			ASSERTSTR(itsTBBs[tbb], "Allocation of PS for tbb " << tbb << " failed.");
		}
		for (uint32	rcu = 0; rcu < itsNrRCUs; rcu++) {
			ASSERTSTR(itsRCUs[rcu], "Allocation of PS for rcu " << rcu << " failed.");
		}
		LOG_INFO_STR("Allocation of all propertySets successfull, going to operational mode");
//		itsOwnPropertySet->setValue(PN_FSM_ERROR,GCFPVString(""));
		TRAN(TBBMonitor::askVersion);
	}
	break;

	case F_DISCONNECTED:
		_disconnectedHandler(port);		// might result in transition to connect2TBB
	break;

	case DP_SET:
		break;

	case F_QUIT:
		TRAN (TBBMonitor::finish_state);
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
// Ask the version of the firmware and the hardware.
//
GCFEvent::TResult TBBMonitor::askVersion(GCFEvent& event, GCFPortInterface& port)
{
	LOG_DEBUG_STR ("askVersion:" << eventName(event) << "@" << port.getName());

	GCFEvent::TResult status = GCFEvent::HANDLED;
  
	switch (event.signal) {

	case F_ENTRY: {
		itsOwnPropertySet->setValue(PN_FSM_CURRENT_ACTION,GCFPVString("TBB:getting version info"));
//		itsOwnPropertySet->setValue(PN_FSM_ERROR,GCFPVString(""));
		itsTimerPort->setTimer(0.1);
	}
	break;

	case TBB_DRIVER_BUSY_ACK:
		LOG_INFO("TBBDriver is busy, retry in 3 seconds");
		itsTimerPort->setTimer(3.0);
		break;

	case F_TIMER: {
		TBBVersionEvent	getVersion;
		getVersion.boardmask = itsBoardMask.to_uint32();
		itsTBBDriver->send(getVersion);
	}
	break;
		
	case TBB_VERSION_ACK: {
		itsTimerPort->cancelAllTimers();
		TBBVersionAckEvent		ack(event);
		// move the information to the database.
		string		versionStr;
		string		DPEname;
		for (uint32	tbb = 0; tbb < itsNrTBboards; tbb++) {
			if (ack.status_mask[tbb] == TBB_SUCCESS) {
				// TBB board version
				versionStr = formatString("%d.%d", ack.tpswversion[tbb] / 10, ack.tpswversion[tbb] % 10);
				itsTBBs[tbb]->setValue(PN_TBB_SW_VERSION, GCFPVString(versionStr), 0.0, false);
				versionStr = formatString("%d.%d", ack.boardversion[tbb] / 10, ack.boardversion[tbb] % 10);
				itsTBBs[tbb]->setValue(PN_TBB_BOARD_VERSION, GCFPVString(versionStr), 0.0, false);
				itsTBBs[tbb]->setValue(PN_TBB_BOARDID, GCFPVUnsigned(ack.boardid[tbb]), 0.0, false);
			
				// BP version
				versionStr = formatString("%d.%d", ack.tphwversion[tbb] / 10, ack.tphwversion[tbb] % 10);
				itsTBBs[tbb]->setValue(PN_TBB_TP_VERSION, GCFPVString(versionStr), 0.0, false);
			
				// MPx versions
				versionStr = formatString("%d.%d", ack.mp0version[tbb] / 10, ack.mp0version[tbb] % 10);
				itsTBBs[tbb]->setValue(PN_TBB_MP0_VERSION, GCFPVString(versionStr), 0.0, false);
				versionStr = formatString("%d.%d", ack.mp1version[tbb] / 10, ack.mp1version[tbb] % 10);
				itsTBBs[tbb]->setValue(PN_TBB_MP1_VERSION, GCFPVString(versionStr), 0.0, false);
				versionStr = formatString("%d.%d", ack.mp2version[tbb] / 10, ack.mp2version[tbb] % 10);
				itsTBBs[tbb]->setValue(PN_TBB_MP2_VERSION, GCFPVString(versionStr), 0.0, false);
				versionStr = formatString("%d.%d", ack.mp3version[tbb] / 10, ack.mp3version[tbb] % 10);
				itsTBBs[tbb]->setValue(PN_TBB_MP3_VERSION, GCFPVString(versionStr), 0.0, false);
			}
			else {	// board in error set ?.?
				itsTBBs[tbb]->setValue(PN_TBB_BOARDID,		 GCFPVUnsigned(0), 0.0, false);
				itsTBBs[tbb]->setValue(PN_TBB_BOARD_VERSION, GCFPVString("?.?"), 0.0, false);
				itsTBBs[tbb]->setValue(PN_TBB_SW_VERSION,    GCFPVString("?.?"), 0.0, false);
				itsTBBs[tbb]->setValue(PN_TBB_TP_VERSION,	 GCFPVString("?.?"), 0.0, false);
				itsTBBs[tbb]->setValue(PN_TBB_MP0_VERSION,	 GCFPVString("?.?"), 0.0, false);
				itsTBBs[tbb]->setValue(PN_TBB_MP1_VERSION,	 GCFPVString("?.?"), 0.0, false);
				itsTBBs[tbb]->setValue(PN_TBB_MP2_VERSION,	 GCFPVString("?.?"), 0.0, false);
				itsTBBs[tbb]->setValue(PN_TBB_MP3_VERSION,	 GCFPVString("?.?"), 0.0, false);
			}
			itsTBBs[tbb]->flush();
			
			// set right color
			setObjectState(getName(), itsTBBs[tbb]->getFullScope(), (ack.status_mask[tbb] == TBB_SUCCESS) ? 
							RTDB_OBJ_STATE_OPERATIONAL : RTDB_OBJ_STATE_OFF);
		}

		LOG_DEBUG_STR ("Version information updated, going to status information");
//		itsOwnPropertySet->setValue(PN_FSM_ERROR,GCFPVString(""));
		TRAN(TBBMonitor::askSizeInfo);				// go to next state.
		break;
	}

	case F_DISCONNECTED:
		_disconnectedHandler(port);		// might result in transition to connect2TBB
		break;

	case DP_SET:
		break;

	case F_QUIT:
		TRAN (TBBMonitor::finish_state);
		break;

	default:
		LOG_DEBUG_STR ("askVersion, DEFAULT");
		break;
	}    

	return (status);
}

//
// askSizeInfo(event, port)
//
// Ask how much RAM is installed on the TB boards
//
GCFEvent::TResult TBBMonitor::askSizeInfo(GCFEvent& event, GCFPortInterface& port)
{
	LOG_DEBUG_STR ("askSizeInfo:" << eventName(event) << "@" << port.getName());

	GCFEvent::TResult status = GCFEvent::HANDLED;
  
	switch (event.signal) {

	case F_ENTRY: {
		itsOwnPropertySet->setValue(PN_FSM_CURRENT_ACTION,GCFPVString("TBB:getting size info"));
//		itsOwnPropertySet->setValue(PN_FSM_ERROR,GCFPVString(""));
		itsTimerPort->setTimer(0.1);
	}
	break;

	case TBB_DRIVER_BUSY_ACK:
		LOG_INFO("TBBDriver is busy, retry in 3 seconds");
		itsTimerPort->setTimer(3.0);
		break;

	case F_TIMER: {
		LOG_DEBUG_STR("Asking size, boardmask=" << itsBoardMask.to_uint32());
		TBBSizeEvent	getSize;
		getSize.boardmask = itsBoardMask.to_uint32();
		itsTBBDriver->send(getSize);
	}
	break;

	case TBB_SIZE_ACK: {
		itsTimerPort->cancelAllTimers();
		TBBSizeAckEvent		ack(event);
		// move the information to the database.
		for (uint32	tbb = 0; tbb < itsNrTBboards; tbb++) {
			if (ack.status_mask[tbb] == TBB_SUCCESS) {
				LOG_INFO_STR("RAMSIZE board " << tbb << ": " << ack.npages[tbb] << " pages = " << 2048.0*(double)ack.npages[tbb] << " bytes");
				itsTBBs[tbb]->setValue(PN_TBB_RAM_SIZE, GCFPVString(byteSize(2048.0*(double)ack.npages[tbb])));
			}
			else {	// board in error set ?.?
				itsTBBs[tbb]->setValue(PN_TBB_RAM_SIZE, GCFPVString(""));
			}
		}

		LOG_DEBUG_STR ("Size information updated, going to status information");
//		itsOwnPropertySet->setValue(PN_FSM_ERROR,GCFPVString(""));
		TRAN(TBBMonitor::askFlashInfo);				// go to next state.
		break;
	}

	case F_DISCONNECTED:
		_disconnectedHandler(port);		// might result in transition to connect2TBB
		break;

	case DP_SET:
		break;

	case F_QUIT:
		TRAN (TBBMonitor::finish_state);
		break;

	default:
		LOG_DEBUG_STR ("askSizeInfo, DEFAULT");
		break;
	}    

	return (status);
}

//
// askFlashInfo(event, port)
//
// Ask which flash images are available
//
GCFEvent::TResult TBBMonitor::askFlashInfo(GCFEvent& event, GCFPortInterface& port)
{
	static uint32	nrOfRequests = 0;

	LOG_DEBUG_STR ("askFlashInfo:" << eventName(event) << "@" << port.getName());

	GCFEvent::TResult status = GCFEvent::HANDLED;
  
	switch (event.signal) {

	case F_ENTRY: {
		itsOwnPropertySet->setValue(PN_FSM_CURRENT_ACTION,GCFPVString("getting flash info"));
//		itsOwnPropertySet->setValue(PN_FSM_ERROR,GCFPVString(""));
		TBBImageInfoEvent	getFlash;
		for (uint32 tbb = 0; tbb < itsNrTBboards; tbb++) {
			if (itsBoardMask.test(tbb)) {
				getFlash.board = tbb;
				itsTBBDriver->send(getFlash);
				nrOfRequests++;
			}
		}
		LOG_DEBUG_STR("Waiting for " << nrOfRequests << " answer messages");
	}
	break;

	case TBB_IMAGE_INFO_ACK: {
		TBBImageInfoAckEvent		ack(event);
		nrOfRequests--;
		// move the information to the database.
		LOG_DEBUG_STR("ack.status_mask=" << ack.status_mask << ", board = " << ack.board);
		
		if (ack.status_mask == TBB_SUCCESS) {
			vector<GCFPValue*>		imageVersions;
			vector<GCFPValue*>		writeDates;
			vector<GCFPValue*>		TPfilenames;
			vector<GCFPValue*>		MPfilenames;
			for (int32	image = 0; image < MAX_N_IMAGES; image++) {
				if (ack.write_date[image] != -1L) {
LOG_DEBUG(formatString("%d:%d:%d:%16.16s", image, ack.image_version[image], ack.write_date[image], ack.tp_file_name[image]));
					imageVersions.push_back(new GCFPVString(formatString("%d.%d", ack.image_version[image]/10, ack.image_version[image]%10)));
					ptime		theTime(from_time_t(ack.write_date[image]));
					writeDates.push_back(new GCFPVString(to_simple_string(theTime)));
					TPfilenames.push_back  (new GCFPVString(ack.tp_file_name[image]));
					MPfilenames.push_back  (new GCFPVString(ack.mp_file_name[image]));
				}
				else {
					imageVersions.push_back(new GCFPVString("free"));
					writeDates.push_back   (new GCFPVString("---"));
					TPfilenames.push_back  (new GCFPVString(""));
					MPfilenames.push_back  (new GCFPVString(""));
				}
			}
			itsTBBs[ack.board]->setValue(PN_TBB_IMAGE_INFO_VERSION,    GCFPVDynArr(LPT_DYNSTRING, imageVersions));
			itsTBBs[ack.board]->setValue(PN_TBB_IMAGE_INFO_WRITE_DATE, GCFPVDynArr(LPT_DYNSTRING, writeDates));
			itsTBBs[ack.board]->setValue(PN_TBB_IMAGE_INFO_TP_FILE,    GCFPVDynArr(LPT_DYNSTRING, TPfilenames));
			itsTBBs[ack.board]->setValue(PN_TBB_IMAGE_INFO_MP_FILE,    GCFPVDynArr(LPT_DYNSTRING, MPfilenames));
		}
		else {
			LOG_WARN_STR("TBB:Flashinfo of boardnr " << ack.board << " contains errors, no update of Flash");
		}

		if (nrOfRequests == 0) {
			LOG_INFO_STR ("Flash information updated, going to status information");
//			itsOwnPropertySet->setValue(PN_FSM_ERROR,GCFPVString(""));
			TRAN(TBBMonitor::askTBBinfo);				// go to next state.
		}
	}
	break;

	case F_DISCONNECTED:
		_disconnectedHandler(port);		// might result in transition to connect2TBB
		break;

	case DP_SET:
		break;

	case F_QUIT:
		TRAN (TBBMonitor::finish_state);
		break;

	default:
		LOG_DEBUG_STR ("askFlashInfo, DEFAULT");
		break;
	}    

	return (status);
}


//
// askTBBinfo(event, port)
//
// Ask info about the TBB boards
//
GCFEvent::TResult TBBMonitor::askTBBinfo(GCFEvent& event, GCFPortInterface& port)
{
	LOG_DEBUG_STR ("askTBBinfo:" << eventName(event) << "@" << port.getName());

	GCFEvent::TResult status = GCFEvent::HANDLED;
  
	switch (event.signal) {

	case F_ENTRY: {
		itsOwnPropertySet->setValue(PN_FSM_CURRENT_ACTION,GCFPVString("TBB:updating TBB info"));
//		itsOwnPropertySet->setValue(PN_FSM_ERROR,GCFPVString(""));
		TBBStatusEvent	getStatus;
		getStatus.boardmask = itsBoardMask.to_uint32();
		itsTBBDriver->send(getStatus);
		itsTimerPort->setTimer(2.0);
	}
	break;

	case TBB_STATUS_ACK: {
		itsTimerPort->cancelAllTimers();
		TBBStatusAckEvent		ack(event);
		// move the information to the database.
		string		versionStr;
		string		DPEname;
		for (uint32	tbb = 0; tbb < itsNrTBboards; tbb++) {
			if (ack.status_mask[tbb] == TBB_SUCCESS) {
				// board voltages
				itsTBBs[tbb]->setValue(PN_TBB_VOLTAGE12, GCFPVDouble(double(ack.V12[tbb])*(2.5/192.0)), 0.0, false);
				itsTBBs[tbb]->setValue(PN_TBB_VOLTAGE25, GCFPVDouble(double(ack.V25[tbb])*(3.3/192.0)), 0.0, false);
				itsTBBs[tbb]->setValue(PN_TBB_VOLTAGE33, GCFPVDouble(double(ack.V33[tbb])*(5.0/192.0)), 0.0, false);
				// all temperatures
				itsTBBs[tbb]->setValue(PN_TBB_TEMPPCB, GCFPVDouble(double(ack.Tpcb[tbb])), 0.0, false);
				itsTBBs[tbb]->setValue(PN_TBB_TEMPTP,  GCFPVDouble(double(ack.Ttp [tbb])), 0.0, false);
				itsTBBs[tbb]->setValue(PN_TBB_TEMPMP0, GCFPVDouble(double(ack.Tmp0[tbb])), 0.0, false);
				itsTBBs[tbb]->setValue(PN_TBB_TEMPMP1, GCFPVDouble(double(ack.Tmp1[tbb])), 0.0, false);
				itsTBBs[tbb]->setValue(PN_TBB_TEMPMP2, GCFPVDouble(double(ack.Tmp2[tbb])), 0.0, false);
				itsTBBs[tbb]->setValue(PN_TBB_TEMPMP3, GCFPVDouble(double(ack.Tmp3[tbb])), 0.0, false);
				// flush to database
				itsTBBs[tbb]->flush();
			}
			
		} // for all boards

		LOG_DEBUG_STR ("TBboard information updated, going to RCU information");
//		itsOwnPropertySet->setValue(PN_FSM_ERROR,GCFPVString(""));
		TRAN(TBBMonitor::askRCUinfo);				// go to next state.
		break;
	}

	case F_TIMER: {
		LOG_WARN_STR ("TBBDriver is not responding, TBboard information is not available");
		itsOwnPropertySet->setValue(PN_FSM_ERROR,GCFPVString("TBB:Driver is not responding, TBboard information is not available"));
		TRAN(TBBMonitor::askRCUinfo);				// go to next state.
		break;
	}

	case F_DISCONNECTED:
		_disconnectedHandler(port);		// might result in transition to connect2TBB
		break;

	case DP_SET:
		break;

	case F_QUIT:
		TRAN (TBBMonitor::finish_state);
		break;

	default:
		LOG_DEBUG_STR ("askTBBinfo, DEFAULT");
		break;
	}    

	return (status);
}


//
// askRCUinfo(event, port)
//
// Normal operation state. 
//
GCFEvent::TResult TBBMonitor::askRCUinfo(GCFEvent& event, GCFPortInterface& port)
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
		itsOwnPropertySet->setValue(PN_FSM_CURRENT_ACTION,GCFPVString("TBB:updating RCU TBB info"));
//		itsOwnPropertySet->setValue(PN_FSM_ERROR,GCFPVString(""));
		TBBRcuInfoEvent	getStatus;
		itsTBBDriver->send(getStatus);
		itsTimerPort->setTimer(2.0);
		break;
	}

	case TBB_RCU_INFO_ACK: {
		itsTimerPort->cancelAllTimers();
		TBBRcuInfoAckEvent	ack(event);

		// move the information to the database.
		for (uint32	rcu = 0; rcu < itsNrRCUs; rcu++) {
			LOG_TRACE_FLOW_STR("Updating rcu " << rcu << ", status = " << ack.rcu_status[rcu]);
			if (ack.rcu_status[rcu] == 0) {
				// update all RCU variables
				itsRCUs[rcu]->setValue(PN_RCU_TBB_ERROR, 	  GCFPVInteger(ack.rcu_status[rcu])),
				itsRCUs[rcu]->setValue(PN_RCU_TBB_MODE, 	  GCFPVString(TBBRCUstate(ack.rcu_state[rcu])));
				itsRCUs[rcu]->setValue(PN_RCU_TBB_START_ADDR, GCFPVString(formatString("0x%08d",ack.rcu_start_addr[rcu])));
				itsRCUs[rcu]->setValue(PN_RCU_TBB_BUF_SIZE,   GCFPVString(byteSize(2048.0*(double)ack.rcu_pages[rcu])));
			}
		} // for all boards

		LOG_DEBUG ("Updated all RCU information, waiting for next cycle");
//		itsOwnPropertySet->setValue(PN_FSM_ERROR,GCFPVString(""));
		TRAN(TBBMonitor::askRCUSettings);			// go to next state.
	}
	break;

	case F_TIMER: {
		LOG_INFO ("TBBDriver is not responding, RCU information is not available");
		itsOwnPropertySet->setValue(PN_FSM_ERROR,GCFPVString("TBB:Driver is not responding, RCU information is not available"));
		TRAN(TBBMonitor::askRCUSettings);			// go to next state.
	}
	break;

	case F_DISCONNECTED:
		_disconnectedHandler(port);		// might result in transition to connect2TBB
		break;

	case DP_SET:
		break;

	case F_QUIT:
		TRAN (TBBMonitor::finish_state);
		break;

	default:
		LOG_DEBUG("askRCUinfo, DEFAULT");
		break;
	}

	return (status);
}

//
// askRCUSettings(event, port)
//
// Ask TBB settings of all RCU's
//
GCFEvent::TResult TBBMonitor::askRCUSettings(GCFEvent& event, GCFPortInterface& port)
{
	if (eventName(event) != "DP_SET") {
		LOG_DEBUG_STR ("askRCUSettings:" << eventName(event) << "@" << port.getName());
	}

	GCFEvent::TResult status = GCFEvent::HANDLED;

	switch (event.signal) {
	case F_INIT:
		break;

	case F_ENTRY: {
		// update PVSS
		itsOwnPropertySet->setValue(PN_FSM_CURRENT_ACTION,GCFPVString("TBB:updating RCU trigger info"));
//		itsOwnPropertySet->setValue(PN_FSM_ERROR,GCFPVString(""));
		TBBTrigSettingsEvent	getSettings;
		itsTBBDriver->send(getSettings);
		itsTimerPort->setTimer(2.0);
		break;
	}

	case TBB_TRIG_SETTINGS_ACK: {
		itsTimerPort->cancelAllTimers();
		TBBTrigSettingsAckEvent	ack(event);

		// move the information to the database.
		for (uint32	rcu = 0; rcu < itsNrRCUs; rcu++) {
			LOG_TRACE_FLOW_STR("Updating rcu " << rcu);
			// update all RCU variables
			itsRCUs[rcu]->setValue(PN_RCU_TRIGGER_STARTLEVEL, GCFPVInteger(ack.setup[rcu].start_mode), 0.0, false),
			itsRCUs[rcu]->setValue(PN_RCU_TRIGGER_BASELEVEL,  GCFPVInteger(ack.setup[rcu].level), 0.0, false),
			itsRCUs[rcu]->setValue(PN_RCU_TRIGGER_STOPLEVEL,  GCFPVInteger(ack.setup[rcu].stop_mode), 0.0, false),
			itsRCUs[rcu]->setValue(PN_RCU_TRIGGER_FILTER,     GCFPVInteger(ack.setup[rcu].filter_select), 0.0, false),
			itsRCUs[rcu]->setValue(PN_RCU_TRIGGER_WINDOW,     GCFPVInteger(ack.setup[rcu].window), 0.0, false),
			//itsRCUs[rcu]->setValue(PN_RCU_TRIGGER_MODE,       GCFPVInteger(ack.setup[rcu].trigger_mode), 0.0, false),
			itsRCUs[rcu]->setValue(PN_RCU_TRIGGER_OPERATING_MODE, 
													GCFPVInteger(ack.setup[rcu].operating_mode), 0.0, false);
			itsRCUs[rcu]->setValue(PN_RCU_TRIGGER_COEFF0, 	  GCFPVInteger(ack.coefficients[rcu].c0), 0.0, false);
			itsRCUs[rcu]->setValue(PN_RCU_TRIGGER_COEFF1, 	  GCFPVInteger(ack.coefficients[rcu].c1), 0.0, false);
			itsRCUs[rcu]->setValue(PN_RCU_TRIGGER_COEFF2, 	  GCFPVInteger(ack.coefficients[rcu].c2), 0.0, false);
			itsRCUs[rcu]->setValue(PN_RCU_TRIGGER_COEFF3, 	  GCFPVInteger(ack.coefficients[rcu].c3), 0.0, false);
			itsRCUs[rcu]->flush();
		} // for all boards

		LOG_DEBUG ("Updated all TriggerSetting information, waiting for next cycle");
//		itsOwnPropertySet->setValue(PN_FSM_ERROR,GCFPVString(""));
		TRAN(TBBMonitor::waitForNextCycle);			// go to next state.
	}
	break;

	case F_TIMER: {
		LOG_INFO ("TBBDriver is not responding, TriggerSetting are not available");
		itsOwnPropertySet->setValue(PN_FSM_ERROR,GCFPVString("TBB:Driver is not responding, TriggerSetting are not available"));
		TRAN(TBBMonitor::waitForNextCycle);			// go to next state.
	}
	break;

	case F_DISCONNECTED:
		_disconnectedHandler(port);		// might result in transition to connect2TBB
		break;

	case DP_SET:
		break;

	case F_QUIT:
		TRAN (TBBMonitor::finish_state);
		break;

	default:
		LOG_DEBUG("askRCUSettings, DEFAULT");
		break;
	}

	return (status);
}

//
// waitForNextCycle(event, port)
//
// Take subscription on clock modifications
//
GCFEvent::TResult TBBMonitor::waitForNextCycle(GCFEvent& event, 
													GCFPortInterface& port)
{
	if (eventName(event) != "DP_SET") {
		LOG_DEBUG_STR ("waitForNextCycle:" << eventName(event) << "@" << port.getName());	}

	GCFEvent::TResult status = GCFEvent::HANDLED;
  
	switch (event.signal) {
	case F_ENTRY: {
		itsOwnPropertySet->setValue(PN_FSM_CURRENT_ACTION,GCFPVString("TBB:wait for next cycle"));
		int		waitTime = itsPollInterval - (time(0) % itsPollInterval);
		if (waitTime == 0) {
			waitTime = itsPollInterval;
		}
		itsTimerPort->cancelAllTimers();
		itsTimerPort->setTimer(double(waitTime));
		LOG_INFO_STR("Waiting " << waitTime << " seconds for next cycle");
	}
	break;

	case F_DISCONNECTED:
		_disconnectedHandler(port);		// might result in transition to connect2TBB
	break;

	case F_TIMER: {
		TRAN(TBBMonitor::askTBBinfo);
	}
	break;

	case DP_SET:
		break;

	case F_QUIT:
		TRAN (TBBMonitor::finish_state);
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
void TBBMonitor::_disconnectedHandler(GCFPortInterface& port)
{
	port.close();
	if (&port == itsTBBDriver) {
		LOG_ERROR("Connection with TBBDriver failed, going to reconnect state");
		itsOwnPropertySet->setValue(PN_FSM_ERROR,GCFPVString("TBB:connection lost"));
		TRAN (TBBMonitor::connect2TBB);
	}
}

//
// finish_state(event, port)
//
// Write controller state to PVSS
//
GCFEvent::TResult TBBMonitor::finish_state(GCFEvent& event, GCFPortInterface& port)
{
	LOG_DEBUG_STR ("finish_state:" << eventName(event) << "@" << port.getName());

	GCFEvent::TResult status = GCFEvent::HANDLED;

	switch (event.signal) {
	case F_INIT:
		break;

	case F_ENTRY: {
		// update PVSS
		itsOwnPropertySet->setValue(PN_FSM_CURRENT_ACTION,GCFPVString("TBB:finished"));
		itsOwnPropertySet->setValue(PN_HWM_TBB_CONNECTED,GCFPVBool(false));
//		itsOwnPropertySet->setValue(PN_FSM_ERROR,GCFPVString(""));
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

//
// TBBRCUstate(char)
//
string	TBBMonitor::TBBRCUstate(char	stateCode) 
{
	switch (stateCode) {
	case 'A':	return ("Allocated");
	case 'E':	return ("Error");
	case 'F':	return ("Free");
	case 'R':	return ("Recording");
	case 'S':	return ("Stopped");
	default:	return ("Unknown");
	}
}

}; // StationCU
}; // LOFAR
