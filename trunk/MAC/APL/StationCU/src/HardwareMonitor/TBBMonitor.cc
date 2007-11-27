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
#include <Common/lofar_datetime.h>

#include <GCF/GCF_PVTypes.h>
#include <GCF/GCF_ServiceInfo.h>
#include <APL/APLCommon/ControllerDefines.h>
#include <APL/APLCommon/APLUtilities.h>
#include <APL/TBB_Protocol/TBB_Protocol.ph>
#include <GCF/RTDB/DP_Protocol.ph>
//#include <APL/APLCommon/StationInfo.h>
#include <signal.h>

#include "TBBMonitor.h"
#include "RCUConstants.h"
#include "StationPermDatapointDefs.h"

using namespace LOFAR::GCF::Common;
using namespace LOFAR::GCF::TM;
using namespace LOFAR::GCF::RTDB;
using namespace std;

namespace LOFAR {
	using namespace APLCommon;
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
		LOG_TRACE_FLOW ("Updateing state to PVSS");
		itsOwnPropertySet->setValue(PN_HWM_TBB_CURRENT_ACTION, GCFPVString("initial"));
		itsOwnPropertySet->setValue(PN_HWM_TBB_ERROR,  GCFPVString(""));
		
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
		itsOwnPropertySet->setValue(PN_HWM_TBB_CURRENT_ACTION, GCFPVString("connecting"));
		itsOwnPropertySet->setValue(PN_HWM_TBB_CONNECTED,GCFPVBool(false));
		itsTBBDriver->open();		// will result in F_CONN or F_DISCONN
		break;

	case F_CONNECTED:
		if (&port == itsTBBDriver) {
			LOG_DEBUG ("Connected with TBBDriver, going to get the configuration");
			itsOwnPropertySet->setValue(PN_HWM_TBB_ERROR,  GCFPVString(""));
			itsOwnPropertySet->setValue(PN_HWM_TBB_CONNECTED,GCFPVBool(true));
			TRAN(TBBMonitor::askConfiguration);		// go to next state.
		}
		break;

	case F_DISCONNECTED:
		port.close();
		ASSERTSTR (&port == itsTBBDriver, 
								"F_DISCONNECTED event from port " << port.getName());
		LOG_DEBUG("Connection with TBBDriver failed, retry in 2 seconds");
		itsOwnPropertySet->setValue(PN_HWM_TBB_ERROR, GCFPVString("connection timeout"));
		itsTimerPort->setTimer(2.0);
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
		itsOwnPropertySet->setValue(PN_HWM_TBB_CURRENT_ACTION,GCFPVString("asking configuration"));
		TBBGetConfigEvent	getconfig;
		itsTBBDriver->send(getconfig);
	}
	break;

	case F_DISCONNECTED:
		_disconnectedHandler(port);		// might result in transition to connect2TBB
		break;

	case TBB_GET_CONFIG_ACK: {
		TBBGetConfigAckEvent	ack(event);
	
		// calc size of the propertyset vectors
		itsBoardMask   = bitset<MAX_N_TBBBOARDS>(ack.active_boards_mask);
		itsNrTBboards  = ack.max_boards;
		itsNrRCUs	   = itsNrTBboards * NR_RCUS_PER_TBBOARD;

		// inform user
		LOG_DEBUG_STR("Active boards = " << itsBoardMask);
		LOG_DEBUG(formatString("nr TBboards = %d", itsNrTBboards));
	
		// do some checks
		if (itsNrTBboards != itsBoardMask.count()) {
			LOG_WARN_STR("Only " << itsBoardMask.count() << " of " << itsNrTBboards 
						<< " TBboards are available.");
		}

		LOG_DEBUG ("Going to allocate the property-sets");
		itsOwnPropertySet->setValue(PN_HWM_TBB_ERROR,GCFPVString(""));
		TRAN (TBBMonitor::finish_state);
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
		itsOwnPropertySet->setValue(PN_HWM_TBB_CURRENT_ACTION,GCFPVString("create PropertySets"));
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
				itsTBBs[TBB] = new RTDBPropertySet(PSname, PST_TB_BOARD, PSAT_WO, this);
			}

			// new RSPboard?
			if (rcu % (NR_RCUS_PER_RSPBOARD) == 0) {
				RSP++;
			}

			// allocate RCU PS
			string	PSname(formatString(rcuNameMask.c_str(), cabinet, subrack, RSP, rcu));
			itsRCUs[rcu] = new RTDBPropertySet(PSname, PST_RCU, PSAT_WO, this);
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
		LOG_DEBUG_STR("Allocation of all propertySets successfull, going to operational mode");
		itsOwnPropertySet->setValue(PN_HWM_TBB_ERROR,GCFPVString(""));
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
		itsOwnPropertySet->setValue(PN_HWM_TBB_CURRENT_ACTION,GCFPVString("getting version info"));
		itsOwnPropertySet->setValue(PN_HWM_TBB_ERROR,GCFPVString(""));
		TBBVersionEvent	getVersion;
		getVersion.boardmask = itsBoardMask.to_uint32();
		itsTBBDriver->send(getVersion);
	}
	break;

	case TBB_VERSION_ACK: {
		TBBVersionAckEvent		ack(event);
		// move the information to the database.
		string		versionStr;
		string		DPEname;
		for (uint32	tbb = 0; tbb < itsNrTBboards; tbb++) {
			if (ack.status_mask[tbb] & TBB_SUCCESS) {
				// TBB board version
				versionStr = formatString("%d.%d", ack.swversion[tbb] / 10, ack.swversion[tbb] % 10);
				itsTBBs[tbb]->setValue(PN_TBB_SW_VERSION, GCFPVString(versionStr));
				versionStr = formatString("%d.%d", ack.boardversion[tbb] / 10, ack.boardversion[tbb] % 10);
				itsTBBs[tbb]->setValue(PN_TBB_BOARD_VERSION, GCFPVString(versionStr));
				itsTBBs[tbb]->setValue(PN_TBB_BOARDID, GCFPVUnsigned(ack.boardid[tbb]));
			
				// BP version
				versionStr = formatString("%d.%d", ack.tpversion[tbb] / 10, ack.tpversion[tbb] % 10);
				itsTBBs[tbb]->setValue(PN_TBB_TP_VERSION, GCFPVString(versionStr));
			
				// MPx versions
				versionStr = formatString("%d.%d", ack.mp0version[tbb] / 10, ack.mp0version[tbb] % 10);
				itsTBBs[tbb]->setValue(PN_TBB_MP0_VERSION, GCFPVString(versionStr));
				versionStr = formatString("%d.%d", ack.mp1version[tbb] / 10, ack.mp1version[tbb] % 10);
				itsTBBs[tbb]->setValue(PN_TBB_MP1_VERSION, GCFPVString(versionStr));
				versionStr = formatString("%d.%d", ack.mp2version[tbb] / 10, ack.mp2version[tbb] % 10);
				itsTBBs[tbb]->setValue(PN_TBB_MP2_VERSION, GCFPVString(versionStr));
				versionStr = formatString("%d.%d", ack.mp3version[tbb] / 10, ack.mp3version[tbb] % 10);
				itsTBBs[tbb]->setValue(PN_TBB_MP3_VERSION, GCFPVString(versionStr));
			}
			else {	// board in error set ?.?
				itsTBBs[tbb]->setValue(PN_TBB_BOARDID,		 GCFPVUnsigned(0));
				itsTBBs[tbb]->setValue(PN_TBB_BOARD_VERSION, GCFPVString("?.?"));
				itsTBBs[tbb]->setValue(PN_TBB_TP_VERSION,	 GCFPVString("?.?"));
				itsTBBs[tbb]->setValue(PN_TBB_MP0_VERSION,	 GCFPVString("?.?"));
				itsTBBs[tbb]->setValue(PN_TBB_MP1_VERSION,	 GCFPVString("?.?"));
				itsTBBs[tbb]->setValue(PN_TBB_MP2_VERSION,	 GCFPVString("?.?"));
				itsTBBs[tbb]->setValue(PN_TBB_MP3_VERSION,	 GCFPVString("?.?"));
			}
		}

		LOG_DEBUG_STR ("Version information updated, going to status information");
		itsOwnPropertySet->setValue(PN_HWM_TBB_ERROR,GCFPVString(""));
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
		itsOwnPropertySet->setValue(PN_HWM_TBB_CURRENT_ACTION,GCFPVString("getting version info"));
		itsOwnPropertySet->setValue(PN_HWM_TBB_ERROR,GCFPVString(""));
		TBBSizeEvent	getSize;
		getSize.boardmask = itsBoardMask.to_uint32();
		itsTBBDriver->send(getSize);
	}
	break;

	case TBB_SIZE_ACK: {
		TBBSizeAckEvent		ack(event);
		// move the information to the database.
		for (uint32	tbb = 0; tbb < itsNrTBboards; tbb++) {
			if (ack.status_mask[tbb] & TBB_SUCCESS) {
				LOG_DEBUG_STR("RAMSIZE board " << tbb << ": " << ack.npages[tbb] << " pages = " << 2048.0*(double)ack.npages[tbb] << " bytes");
				itsTBBs[tbb]->setValue(PN_TBB_RAM_SIZE, GCFPVString(byteSize(2048.0*(double)ack.npages[tbb])));
			}
			else {	// board in error set ?.?
				itsTBBs[tbb]->setValue(PN_TBB_RAM_SIZE, GCFPVString(""));
			}
		}

		LOG_DEBUG_STR ("Size information updated, going to status information");
		itsOwnPropertySet->setValue(PN_HWM_TBB_ERROR,GCFPVString(""));
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
		itsOwnPropertySet->setValue(PN_HWM_TBB_CURRENT_ACTION,GCFPVString("getting flash info"));
		itsOwnPropertySet->setValue(PN_HWM_TBB_ERROR,GCFPVString(""));
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
		if (ack.status_mask & TBB_SUCCESS) {
			vector<GCFPValue*>		imageVersions;
			vector<GCFPValue*>		writeDates;
			vector<GCFPValue*>		TPfilenames;
			vector<GCFPValue*>		MPfilenames;
			for (int32	tbb = 0; tbb < MAX_N_IMAGES; tbb++) {
cout << "TIMESTAMP = " << ack.write_date[tbb] << endl;
				if (ack.write_date[tbb] != -1L) {
					imageVersions.push_back(new GCFPVString(formatString("%d.%d", ack.image_version[tbb]/10, ack.image_version[tbb]%10)));
					ptime		theTime(from_time_t(ack.write_date[tbb]));
					writeDates.push_back(new GCFPVString(to_simple_string(theTime)));
				}
				else {
					imageVersions.push_back(new GCFPVString("free"));
					writeDates.push_back(new GCFPVString("---"));
				}
				TPfilenames.push_back  (new GCFPVString(ack.tp_file_name[tbb]));
				MPfilenames.push_back  (new GCFPVString(ack.mp_file_name[tbb]));
			}
			itsTBBs[ack.board]->setValue(PN_TBB_IMAGE_INFO_VERSION,    GCFPVDynArr(LPT_DYNSTRING, imageVersions));
			itsTBBs[ack.board]->setValue(PN_TBB_IMAGE_INFO_WRITE_DATE, GCFPVDynArr(LPT_DYNSTRING, writeDates));
			itsTBBs[ack.board]->setValue(PN_TBB_IMAGE_INFO_TP_FILE,    GCFPVDynArr(LPT_DYNSTRING, TPfilenames));
			itsTBBs[ack.board]->setValue(PN_TBB_IMAGE_INFO_MP_FILE,    GCFPVDynArr(LPT_DYNSTRING, MPfilenames));
		}

		if (--nrOfRequests == 0) {
			LOG_DEBUG_STR ("Size information updated, going to status information");
			itsOwnPropertySet->setValue(PN_HWM_TBB_ERROR,GCFPVString(""));
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
		itsOwnPropertySet->setValue(PN_HWM_TBB_CURRENT_ACTION,GCFPVString("updating TBB info"));
		itsOwnPropertySet->setValue(PN_HWM_TBB_ERROR,GCFPVString(""));
		TBBStatusEvent	getStatus;
		getStatus.boardmask = itsBoardMask.to_uint32();
		itsTBBDriver->send(getStatus);
	}
	break;

	case TBB_STATUS_ACK: {
		TBBStatusAckEvent		ack(event);
		// move the information to the database.
		string		versionStr;
		string		DPEname;
		for (uint32	tbb = 0; tbb < itsNrTBboards; tbb++) {
			if (ack.status_mask[tbb] & TBB_SUCCESS) {
				// board voltages
				itsTBBs[tbb]->setValue(PN_TBB_VOLTAGE12, GCFPVDouble(double(ack.V12[tbb])*(2.5/192.0)));
				itsTBBs[tbb]->setValue(PN_TBB_VOLTAGE25, GCFPVDouble(double(ack.V25[tbb])*(3.3/192.0)));
				itsTBBs[tbb]->setValue(PN_TBB_VOLTAGE33, GCFPVDouble(double(ack.V33[tbb])*(5.0/192.0)));
				// all temperatures
				itsTBBs[tbb]->setValue(PN_TBB_TEMPPCB, GCFPVDouble(double(ack.Tpcb[tbb])));
				itsTBBs[tbb]->setValue(PN_TBB_TEMPTP,  GCFPVDouble(double(ack.Ttp [tbb])));
				itsTBBs[tbb]->setValue(PN_TBB_TEMPMP0, GCFPVDouble(double(ack.Tmp0[tbb])));
				itsTBBs[tbb]->setValue(PN_TBB_TEMPMP1, GCFPVDouble(double(ack.Tmp1[tbb])));
				itsTBBs[tbb]->setValue(PN_TBB_TEMPMP2, GCFPVDouble(double(ack.Tmp2[tbb])));
				itsTBBs[tbb]->setValue(PN_TBB_TEMPMP3, GCFPVDouble(double(ack.Tmp3[tbb])));
			}
		} // for all boards

		LOG_DEBUG_STR ("TBboard information updated, going to RCU information");
		itsOwnPropertySet->setValue(PN_HWM_TBB_ERROR,GCFPVString(""));
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
		itsOwnPropertySet->setValue(PN_HWM_TBB_CURRENT_ACTION,GCFPVString("updating RCU TBB info"));
		itsOwnPropertySet->setValue(PN_HWM_TBB_ERROR,GCFPVString(""));
		TBBRcuInfoEvent	getStatus;
		itsTBBDriver->send(getStatus);
		break;
	}

	case TBB_RCU_INFO_ACK: {
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
		itsOwnPropertySet->setValue(PN_HWM_TBB_ERROR,GCFPVString(""));
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
		itsOwnPropertySet->setValue(PN_HWM_TBB_CURRENT_ACTION,GCFPVString("updating RCU trigger info"));
		itsOwnPropertySet->setValue(PN_HWM_TBB_ERROR,GCFPVString(""));
		TBBTrigSettingsEvent	getSettings;
		itsTBBDriver->send(getSettings);
		break;
	}

	case TBB_TRIG_SETTINGS_ACK: {
		TBBTrigSettingsAckEvent	ack(event);

		// move the information to the database.
		for (uint32	rcu = 0; rcu < itsNrRCUs; rcu++) {
			LOG_TRACE_FLOW_STR("Updating rcu " << rcu);
			// update all RCU variables
			itsRCUs[rcu]->setValue(PN_RCU_TRIGGER_STARTLEVEL, GCFPVInteger(ack.setup[rcu].start_mode)),
			itsRCUs[rcu]->setValue(PN_RCU_TRIGGER_BASELEVEL,  GCFPVInteger(ack.setup[rcu].level)),
			itsRCUs[rcu]->setValue(PN_RCU_TRIGGER_STOPLEVEL,  GCFPVInteger(ack.setup[rcu].stop_mode)),
			itsRCUs[rcu]->setValue(PN_RCU_TRIGGER_FILTER, 	  GCFPVInteger(ack.setup[rcu].filter_select)),
			itsRCUs[rcu]->setValue(PN_RCU_TRIGGER_WINDOW, 	  GCFPVInteger(ack.setup[rcu].window)),
//			itsRCUs[rcu]->setValue(PN_RCU_TRIGGER_OPERATING_MODE, GCFPVInteger(ack.coefficients[rcu].xxx));
			itsRCUs[rcu]->setValue(PN_RCU_TRIGGER_COEFF0, 		  GCFPVInteger(ack.coefficients[rcu].c0));
			itsRCUs[rcu]->setValue(PN_RCU_TRIGGER_COEFF1, 		  GCFPVInteger(ack.coefficients[rcu].c1));
			itsRCUs[rcu]->setValue(PN_RCU_TRIGGER_COEFF2, 		  GCFPVInteger(ack.coefficients[rcu].c2));
			itsRCUs[rcu]->setValue(PN_RCU_TRIGGER_COEFF3, 		  GCFPVInteger(ack.coefficients[rcu].c3));
		} // for all boards

		LOG_DEBUG ("Updated all TriggerSetting information, waiting for next cycle");
		itsOwnPropertySet->setValue(PN_HWM_TBB_ERROR,GCFPVString(""));
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
		itsOwnPropertySet->setValue(PN_HWM_TBB_CURRENT_ACTION,GCFPVString("wait for next cycle"));
		int		waitTime = itsPollInterval - (time(0) % itsPollInterval);
		if (waitTime == 0) {
			waitTime = itsPollInterval;
		}
		itsTimerPort->cancelAllTimers();
		itsTimerPort->setTimer(double(waitTime));
		LOG_DEBUG_STR("Waiting " << waitTime << " seconds for next cycle");
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
		LOG_DEBUG("Connection with TBBDriver failed, going to reconnect state");
		itsOwnPropertySet->setValue(PN_HWM_TBB_ERROR,GCFPVString("connection lost"));
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
		itsOwnPropertySet->setValue(PN_HWM_TBB_CURRENT_ACTION,GCFPVString("finished"));
		itsOwnPropertySet->setValue(PN_HWM_TBB_CONNECTED,GCFPVBool(false));
		itsOwnPropertySet->setValue(PN_HWM_TBB_ERROR,GCFPVString(""));
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
