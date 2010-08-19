//#  TBBControl.cc: Implementation of the MAC Scheduler task
//#
//#  Copyright (C) 2007
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
#include <Common/LofarLocators.h>
#include <Common/Version.h>
#include <Common/ParameterSet.h>
#include <ApplCommon/StationInfo.h>
#include <ApplCommon/StationConfig.h>

#include <GCF/PVSS/GCF_PVTypes.h>
//#include <GCF/Utils.h>
#include <MACIO/MACServiceInfo.h>
#include <APL/APLCommon/APL_Defines.h>
#include <APL/APLCommon/APLUtilities.h>
#include <APL/APLCommon/Controller_Protocol.ph>
#include <APL/TBB_Protocol/TBB_Protocol.ph>
#include <APL/RSP_Protocol/RSP_Protocol.ph>
#include <GCF/RTDB/DP_Protocol.ph>
#include <signal.h>
#include <APL/RTCCommon/Timestamp.h>
#include <StationCU/Package__Version.h>

//# local includes
#include <VHECR/TBBTrigger.h>
#include <VHECR/TBBReadCmd.h>
#include <VHECR/VHECRTask.h>
#include "TBBControl.h"
#include "PVSSDatapointDefs.h"
#include "TBBObservation.h"

using namespace std;
namespace LOFAR {
	using namespace GCF::TM;
	using namespace GCF::PVSS;
	using namespace GCF::RTDB;
	using namespace VHECR;
	using namespace APLCommon;
	using namespace RTC;
	namespace StationCU {

#define VHECR_INTERVAL 0.1

//typedef bitset<MAX_RCUS> RCUset_t;

// static pointer to this object for signal handler
static TBBControl*  thisTBBControl = 0;

//
// TBBControl()
//
TBBControl::TBBControl(const string&    cntlrName) :
	GCFTask             ((State)&TBBControl::initial_state,cntlrName),
	itsPropertySet      (0),
	itsPropertySetInitialized (false),
	itsParentControl    (0),
	itsParentPort       (0),
	itsTimerPort        (0),
	itsTBBDriver        (0),
	itsState            (CTState::NOSTATE)
{
	StationConfig* itsStationConfig;
	itsStationConfig = globalStationConfig();
	itsNrTBBs = itsStationConfig->nrTBBs;
	itsNrRCUs = itsNrTBBs * 16;
	
	for (int i = 0; i < itsNrRCUs; i++) {
		itsTriggerMode[i] = 0;
	}
	LOG_TRACE_OBJ_STR (cntlrName << " construction");
	LOG_INFO(Version::getInfo<StationCUVersion>("TBBControl"));

	// First readin our observation related config file.
	LOFAR::ConfigLocator cl;
	LOG_DEBUG_STR("Reading parset file:" << cl.locate(cntlrName));
	itsParameterSet = new ParameterSet(cl.locate(cntlrName));
	itsObs = new TBBObservation(itsParameterSet);   // does all nasty conversions

	//LOG_DEBUG_STR("Reading parset file:" << LOFAR_SHARE_LOCATION << "/" << cntlrName);
	//itsParameterSet = new ParameterSet(string(LOFAR_SHARE_LOCATION)+"/"+cntlrName);
	//itsObs = new TBBObservation(itsParameterSet); // does all nasty conversions
	//globalParameterSet()->adoptFile(string(LOFAR_SHARE_LOCATION)+"/"+cntlrName);
	//itsObs = new TBBObservation(globalParameterSet());    // does all nasty conversions

	LOG_DEBUG_STR(*itsObs);
	// Readin some parameters from the ParameterSet.
	itsTreePrefix = itsParameterSet->getString("prefix");
	//itsInstanceNr = globalParameterSet()->getUint32("_instanceNr");

	// attach to parent control task
	itsParentControl = ParentControl::instance();
	itsParentPort = new GCFITCPort (*this, *itsParentControl, "ParentITCport",
									GCFPortInterface::SAP, CONTROLLER_PROTOCOL);
	ASSERTSTR(itsParentPort, "Cannot allocate ITCport for Parentcontrol");
	itsParentPort->open();      // will result in F_CONNECTED

	// need port for timers.
	itsTimerPort = new GCFTimerPort(*this, "TimerPort");

	// timer port for calling VHECRTask every 100mS
	itsVHECRtimer = new GCFTimerPort(*this, "VHECRtimer");

	// prepare TCP port to TBBDriver.
	itsTBBDriver = new GCFTCPPort (*this,
	                                MAC_SVCMASK_TBBDRIVER,
									GCFPortInterface::SAP,
									TBB_PROTOCOL);
	ASSERTSTR(itsTBBDriver, "Cannot allocate TCPport to TBBDriver");

	// prepare TCP port to RSPDriver.
	itsRSPDriver = new GCFTCPPort (*this,
	                                MAC_SVCMASK_RSPDRIVER,
									GCFPortInterface::SAP,
									RSP_PROTOCOL);
	ASSERTSTR(itsRSPDriver, "Cannot allocate TCPport to RSPDriver");



	// for debugging purposes
	registerProtocol (CONTROLLER_PROTOCOL, CONTROLLER_PROTOCOL_STRINGS);
	registerProtocol (DP_PROTOCOL, DP_PROTOCOL_STRINGS);
	registerProtocol (TBB_PROTOCOL, TBB_PROTOCOL_STRINGS);
	registerProtocol (RSP_PROTOCOL, RSP_PROTOCOL_STRINGS);

	setState(CTState::CREATED);

	itsStopCommandVector.clear();  // clear buffer
	itsReadCommandVector.clear();  // clear buffer

	itsVHECRTask = new VHECRTask(cntlrName);
	ASSERTSTR(itsVHECRTask, "Could not create the VHECR task");
	//itsVHECRTask->setSaveTask(this);
	itsRCUs.reset();
}


//
// ~TBBControl()
//
TBBControl::~TBBControl()
{
	if (itsVHECRTask) {
		delete itsVHECRTask;
	}

	LOG_TRACE_OBJ_STR (getName() << " destruction");
}

//
// sigintHandler(signum)
//
void TBBControl::sigintHandler(int signum)
{
	LOG_DEBUG (formatString("SIGINT signal detected (%d)",signum));

	if (thisTBBControl) {
		thisTBBControl->finish();
	}
}

//
// finish
//
void TBBControl::finish()
{
	TRAN(TBBControl::quiting_state);
}


//
// setState(CTstateNr)
//
void TBBControl::setState(CTState::CTstateNr newState)
{
	itsState = newState;

	if (itsPropertySet) {
		CTState cts;
		itsPropertySet->setValue(PN_FSM_CURRENT_ACTION, GCFPVString(cts.name(newState)));
	}
}


//==============================================================================
// initial_state(event, port)
//
// Connect to PVSS and report state back to startdaemon
//==============================================================================
GCFEvent::TResult TBBControl::initial_state(GCFEvent& event,
											GCFPortInterface& port)
{
	LOG_DEBUG_STR ("initial:" << eventName(event) << "@" << port.getName());

	GCFEvent::TResult status = GCFEvent::HANDLED;

	switch (event.signal) {
		case F_ENTRY: {
		} break;

		case F_INIT: {
			// Get access to my own propertyset.
			string propSetName( createPropertySetName(PSN_TBB_CONTROL,
								getName(),
								itsParameterSet->getString("_DPname")));
			LOG_INFO_STR ("Activating PropertySet " << propSetName);
			itsPropertySet = new RTDBPropertySet(   propSetName,
												    PST_TBB_CONTROL,
													PSAT_RW,
													this);
			LOG_INFO_STR ("Activating " << propSetName << " Done");
			// Wait for timer that is set on DP_CREATED event
		} break;

		case DP_CREATED: {
			// NOTE: thsi function may be called DURING the construction of the PropertySet.
			// Always exit this event in a way that GCF can end the construction.
			DPCreatedEvent  dpEvent(event);
			LOG_DEBUG_STR("Result of creating " << dpEvent.DPname << " = " << dpEvent.result);
			itsTimerPort->cancelAllTimers();
			itsTimerPort->setTimer(0.0);
		} break;

		case F_TIMER: {
			if (!itsPropertySetInitialized) {
				itsPropertySetInitialized = true;

				// Instruct codeloggingProcessor
				// TODO: get this from a .h file
				LOG_INFO_STR("MACProcessScope: LOFAR.PermSW.TBBControl");

				// first redirect signalHandler to our quiting state to leave PVSS
				// in the right state when we are going down
				thisTBBControl = this;
				signal (SIGINT,  TBBControl::sigintHandler);    // ctrl-c
				signal (SIGTERM, TBBControl::sigintHandler);    // kill

				// update PVSS.
				LOG_TRACE_FLOW ("Updateing state to PVSS");
	// TODO TBB
				itsPropertySet->setValue(PN_FSM_CURRENT_ACTION, GCFPVString("initial"));
				itsPropertySet->setValue(PN_FSM_ERROR,  GCFPVString(""));
				itsPropertySet->setValue(PN_TBC_CONNECTED,  GCFPVBool(false));

				itsPropertySet->setValue(PN_TBC_TRIGGER_RCU_NR, GCFPVInteger(0),0.0,false);
				itsPropertySet->setValue(PN_TBC_TRIGGER_SEQUENCE_NR,    GCFPVInteger(0),0.0,false);
				itsPropertySet->setValue(PN_TBC_TRIGGER_TIME,   GCFPVInteger(0),0.0,false);
				itsPropertySet->setValue(PN_TBC_TRIGGER_SAMPLE_NR,  GCFPVInteger(0),0.0,false);
				itsPropertySet->setValue(PN_TBC_TRIGGER_SUM,    GCFPVInteger(0),0.0,false);
				itsPropertySet->setValue(PN_TBC_TRIGGER_NR_SAMPLES, GCFPVInteger(0),0.0,false);
				itsPropertySet->setValue(PN_TBC_TRIGGER_PEAK_VALUE, GCFPVInteger(0),0.0,false);
				itsPropertySet->flush();

				// Start ParentControl task
				LOG_DEBUG ("Enabling ParentControl task and wait for my name");
				itsParentPort = itsParentControl->registerTask(this);
				// results in CONTROL_CONNECT
			}
		} break;

		case F_CONNECTED: {
			ASSERTSTR (&port == itsParentPort, "F_CONNECTED event from port " << port.getName());
		} break;

		case F_DISCONNECTED:
		case F_EXIT: {
		} break;

		case CONTROL_CONNECT: {
			CONTROLConnectEvent msg(event);
			LOG_DEBUG_STR("Received CONNECT(" << msg.cntlrName << ")");
			setState(CTState::CONNECTED);
			sendControlResult(port, CONTROL_CONNECTED, msg.cntlrName, CT_RESULT_NO_ERROR);

			// let ParentControl watch over the start and stop times for extra safety.
			ptime startTime = time_from_string(itsParameterSet->getString("Observation.startTime"));
			ptime stopTime  = time_from_string(itsParameterSet->getString("Observation.stopTime"));
			itsParentControl->activateObservationTimers(msg.cntlrName, startTime, stopTime);

			LOG_INFO ("Going to started state");
			TRAN(TBBControl::started_state);  // go to next state.
		} break;

		default: {
			status = _defaultEventHandler(event, port);
		} break;
	}

	return (status);
}


//==============================================================================
// started_state(event, port)
//
// wait for CLAIM event
//==============================================================================
GCFEvent::TResult TBBControl::started_state(GCFEvent& event, GCFPortInterface& port)
{
	LOG_DEBUG_STR ("started:" << eventName(event) << "@" << port.getName());

	GCFEvent::TResult status = GCFEvent::HANDLED;

	switch (event.signal) {
		case F_ENTRY: {
			// update PVSS
	//      itsPropertySet->setValue(string(PVSSNAME_FSM_CURACT),GCFPVString("started"));
			itsPropertySet->setValue(PN_FSM_ERROR,GCFPVString(""));
			itsPropertySet->setValue(PN_TBC_CONNECTED,  GCFPVBool(false)); //[TBB]
		} break;

		case F_CONNECTED: {
			if (&port == itsTBBDriver) {
				LOG_DEBUG_STR("F_CONNECTED event from port " << port.getName());
				LOG_INFO ("Connected with TBBDriver");
				itsPropertySet->setValue(PN_TBC_CONNECTED,  GCFPVBool(true));   // [TBB]
				itsRSPDriver->open();
			}
			if (&port == itsRSPDriver) {
				LOG_DEBUG_STR("F_CONNECTED event from port " << port.getName());
				LOG_INFO ("Connected with RSPDriver");
				itsPropertySet->setValue(PN_TBC_CONNECTED,  GCFPVBool(true));   // [TBB]
			}

			itsTimerPort->cancelAllTimers();
			if (itsTBBDriver->isConnected() && itsRSPDriver->isConnected()) {
				LOG_INFO ("Connected with TBBDriver and RSPDriver, going to claimed state");
				setState(CTState::CLAIMED);
				sendControlResult(*itsParentPort, CONTROL_CLAIMED, getName(), CT_RESULT_NO_ERROR);
				TRAN(TBBControl::claimed_state);                // go to next state.
			}
		} break;

		case F_DISCONNECTED: {
			port.close();
			if (&port == itsTBBDriver) {
				LOG_DEBUG_STR("F_DISCONNECTED event from port " << port.getName());
				LOG_WARN_STR ("Connection with TBBDriver failed, retry in 2 seconds");
				itsPropertySet->setValue(PN_TBC_CONNECTED,  GCFPVBool(false));  // [TBB]
				itsTimerPort->setTimer(2.0);
			}
			if (&port == itsRSPDriver) {
				LOG_DEBUG_STR("F_DISCONNECTED event from port " << port.getName());
				LOG_WARN_STR ("Connection with RSPDriver failed, retry in 2 seconds");
				itsPropertySet->setValue(PN_TBC_CONNECTED,  GCFPVBool(false));  // [TBB]
				itsTimerPort->setTimer(2.0);
			}
		} break;

		case F_TIMER: {
	//      GCFTimerEvent& timerEvent=static_cast<GCFTimerEvent&>(event);
			LOG_DEBUG ("Trying to reconnect to TBBDriver");
			itsTBBDriver->open();       // will result in F_CONN or F_DISCONN

		} break;

		// -------------------- EVENTS RECEIVED FROM PARENT CONTROL --------------------
		case CONTROL_CLAIM: {
			CONTROLClaimEvent msg(event);
			LOG_DEBUG_STR("Received CLAIM(" << msg.cntlrName << ")");
			setState(CTState::CLAIM);
			LOG_DEBUG ("Trying to connect to TBBDriver");
			itsTBBDriver->open();  // will result in F_CONN or F_DISCONN
		} break;

		default: {
			status = _defaultEventHandler(event, port);
		} break;
	}

	return (status);
}


//==============================================================================
// claimed_state(event, port)
//
// wait for PREPARE event.
//==============================================================================
GCFEvent::TResult TBBControl::claimed_state(GCFEvent& event, GCFPortInterface& port)
{
	LOG_DEBUG_STR ("claimed:" << eventName(event) << "@" << port.getName());

	GCFEvent::TResult status = GCFEvent::HANDLED;

	switch (event.signal) {
		case F_ENTRY: {
			// update PVSS
			// itsPropertySet->setValue(string(PVSSNAME_FSM_CURACT),GCFPVString("claimed"));
			itsPropertySet->setValue(PN_FSM_ERROR, GCFPVString(""));
		} break;

		// -------------------- EVENTS RECEIVED FROM PARENT CONTROL --------------------
		case CONTROL_PREPARE: {
			CONTROLPrepareEvent msg(event);
			LOG_DEBUG_STR("Received PREPARE(" << msg.cntlrName << ")");
			setState(CTState::PREPARE);
			if (itsObs->isLoaded()) {
				// start setting up the observation
				TRAN(TBBControl::doRSPtbbMode);
			} else {
				// no observations to setup, go direct prepared state
				sendControlResult(*itsParentPort, CONTROL_PREPARED, getName(), CT_RESULT_NO_ERROR);
				setState(CTState::PREPARED);
				TRAN(TBBControl::prepared_state);  // go to prepared state.
			}
		} break;

		default: {
			status = _defaultEventHandler(event, port);
		} break;
	}

	return (status);
}

//==============================================================================
// doRSPtbbMode(event, port)
//
// send RSP_TBB_MODE cmd to the RSPDriver
//==============================================================================
GCFEvent::TResult TBBControl::doRSPtbbMode(GCFEvent& event, GCFPortInterface& port)
{
	LOG_DEBUG_STR ("doRSPtbbMode:" << eventName(event) << "@" << port.getName());

	GCFEvent::TResult status = GCFEvent::HANDLED;

	switch (event.signal) {
		case F_ENTRY: {
			RSPSettbbEvent settbb;

			settbb.timestamp = Timestamp(0,0);
			settbb.rcumask.reset();

			settbb.settings().resize(1);
			settbb.settings()(0).reset();

			vector<TBBObservation::cSettings>::iterator it1;
			for (it1 = itsObs->TbbSettings.begin(); it1 != itsObs->TbbSettings.end(); it1++ ) {
				settbb.rcumask |= (*it1).RCUset;

				if ((*it1).operatingMode == TBB_MODE_SUBBANDS) {
					std::vector<int32>::iterator it2;
					for (it2 = (*it1).SubbandList.begin(); it2 != (*it1).SubbandList.end(); it2++) {
						if ((*it2) >= MEPHeader::N_SUBBANDS) continue;
						settbb.settings()(0).set(*it2);
					}
				}
			}
			
			// set itsRCUs for further use
			itsRCUs = settbb.rcumask;
			
			// info to pvss
			LOG_DEBUG_STR("send RSP_SET_TBB cmd");
			itsRSPDriver->send(settbb);
		} break;

		case RSP_SETTBBACK: {
			RSPSettbbackEvent ack(event);

			if (ack.status == RSP_SUCCESS) {
				TRAN(TBBControl::doTBBmode);  // go to next state.
			} else {
				LOG_DEBUG_STR ("returned status" << ack.status);
				LOG_ERROR_STR ("Failed to set the operating mode for all the rcus");
				itsPropertySet->setValue(PN_FSM_ERROR,GCFPVString("operatingMode error"));
				sendControlResult(*itsParentPort, CONTROL_PREPARED, getName(), CT_RESULT_MODESETUP_FAILED);
				setState(CTState::CLAIMED);
				TRAN(TBBControl::claimed_state);  // go to claimed_state state.
			}
		} break;

		default: {
			status = _defaultEventHandler(event, port);
		} break;
	}

	return (status);
}


//==============================================================================
// doTBBmode(event, port)
//
// send TBB_MODE cmd to the TBBDriver
//==============================================================================
GCFEvent::TResult TBBControl::doTBBmode(GCFEvent& event, GCFPortInterface& port)
{
	LOG_DEBUG_STR ("doTBBmode:" << eventName(event) << "@" << port.getName());

	GCFEvent::TResult status = GCFEvent::HANDLED;

	static RCUset_t RCUset;

	switch (event.signal) {
		case F_ENTRY: {
			TBBModeEvent cmd;
			RCUset.reset();
			// preset mode to transient
			for (int rcu = 0; rcu < itsNrRCUs; rcu++) {
				cmd.rec_mode[rcu] = TBB_MODE_TRANSIENT;
			}

			vector<TBBObservation::cSettings>::iterator it;
			for (it = itsObs->TbbSettings.begin(); it != itsObs->TbbSettings.end(); it++ ) {
				for (int rcu = 0; rcu < itsNrRCUs; rcu++) {
					if ((*it).RCUset.test(rcu)) {
						RCUset.set(rcu);
						cmd.rcu_mask.set(rcu);
						cmd.rec_mode[rcu] = (*it).operatingMode;
					}
				}
			}
			// info to pvss
			LOG_DEBUG_STR("send TBB_MODE cmd");
			itsTBBDriver->send(cmd);
		} break;

		case TBB_MODE_ACK: {
			TBBModeAckEvent ack(event);
			TRAN(TBBControl::doTBBfree);  // go to next state.
		} break;

		default: {
			status = _defaultEventHandler(event, port);
		} break;
	}

	return (status);
}

//==============================================================================
// doTBBstorage(event, port)
//
// send TBB_STORAGE cmd to the TBBDriver
//==============================================================================
GCFEvent::TResult TBBControl::doTBBstorage(GCFEvent& event, GCFPortInterface& port)
{
	LOG_DEBUG_STR ("doTBBstorage:" << eventName(event) << "@" << port.getName());

	GCFEvent::TResult status = GCFEvent::HANDLED;

	static RCUset_t RCUset;

	switch (event.signal) {
		case F_ENTRY: {
			TBBCepStorageEvent cmd;
			RCUset.reset();

			//cmd.destination = itsObs->TbbSettings.operatingMode;
			// info to pvss
			//LOG_DEBUG_STR("send TBB_MODE cmd");
			itsTBBDriver->send(cmd);
		} break;

		case TBB_CEP_STORAGE_ACK: {
			TBBModeAckEvent ack(event);
			bool status_ok = true;

			for (int b = 0; b < itsNrTBBs; b++) {
			    if (isBoardUsed(b) == false) { continue; }
				if (ack.status_mask[b] != TBB_SUCCESS) {
					status_ok = false;  // error
					LOG_DEBUG_STR (formatString("error, board-%d status=%u", b, ack.status_mask[b])); 
				}
			}

			if (status_ok) {
				TRAN(TBBControl::doTBBfree);  // go to next state.
			} else {
				LOG_ERROR_STR ("Failed to set the operating mode for all the rcus");
				itsPropertySet->setValue(PN_FSM_ERROR,GCFPVString("operatingMode error"));
				sendControlResult(*itsParentPort, CONTROL_PREPARED, getName(), CT_RESULT_MODESETUP_FAILED);
				setState(CTState::CLAIMED);
				TRAN(TBBControl::claimed_state);  // go to claimed_state state.
			}
		} break;

		default: {
			status = _defaultEventHandler(event, port);
		} break;
	}

	return (status);
}

//==============================================================================
// doTBBfree (event, port)
//
// send TBB_FREE cmd to the TBBDriver
//==============================================================================
GCFEvent::TResult TBBControl::doTBBfree(GCFEvent& event, GCFPortInterface& port)
{
	LOG_DEBUG_STR ("doTBBfree:" << eventName(event) << "@" << port.getName());

	GCFEvent::TResult status = GCFEvent::HANDLED;

	switch (event.signal) {
		case F_ENTRY: {
			TBBFreeEvent cmd;

			cmd.rcu_mask.set(); // select all RCUs
			// info to pvss
			LOG_DEBUG_STR("send TBB_FREE cmd");
			itsTBBDriver->send(cmd);
		} break;

		case TBB_FREE_ACK: {
			TBBFreeAckEvent ack(event);
			bool status_ok = true;

			for (int b = 0; b < itsNrTBBs; b++) {
				if (isBoardUsed(b) == false) { continue; }
				if (ack.status_mask[b] != TBB_SUCCESS) {  // if error, check if rcu is used
					status_ok = false;
					LOG_DEBUG_STR (formatString("error, board-%d status=0x%x", b, ack.status_mask[b])); 
				}
			}

			if (status_ok) {
				TRAN(TBBControl::doTBBalloc);  // go to next state.
			} else {
				LOG_ERROR_STR ("Failed to free the memmory for all the rcus");
				itsPropertySet->setValue(PN_FSM_ERROR,GCFPVString("free error"));
				sendControlResult(*itsParentPort, CONTROL_PREPARED, getName(), CT_RESULT_MODESETUP_FAILED);
				setState(CTState::CLAIMED);
				TRAN(TBBControl::claimed_state);  // go to claimed_state state.
			}
		} break;

		default: {
			status = _defaultEventHandler(event, port);
		} break;
	}

	return (status);
}


//==============================================================================
// doTBBalloc(event, port)
//
// send TBB_ALLOC cmd to the TBBDriver
//==============================================================================
GCFEvent::TResult TBBControl::doTBBalloc(GCFEvent& event, GCFPortInterface& port)
{
	LOG_DEBUG_STR ("doTBBalloc:" << eventName(event) << "@" << port.getName());

	GCFEvent::TResult status = GCFEvent::HANDLED;

	static RCUset_t RCUset;

	switch (event.signal) {
		case F_ENTRY: {
			TBBAllocEvent cmd;
			RCUset.reset();
			vector<TBBObservation::cSettings>::iterator it;
			for (it = itsObs->TbbSettings.begin(); it != itsObs->TbbSettings.end(); it++ ) {
				for (int i = 0; i < itsNrRCUs; i++) {
					if ((*it).RCUset.test(i)) {
						RCUset.set(i);
						cmd.rcu_mask.set(i);
					}
				}
			}
			// info to pvss
			LOG_DEBUG_STR("send TBB_ALLOC cmd");
			itsTBBDriver->send(cmd);
		} break;

		case TBB_ALLOC_ACK: {
			TBBModeAckEvent ack(event);
			bool status_ok = true;

			for (int b = 0; b < itsNrTBBs; b++) {
			    if (isBoardUsed(b) == false) { continue; }
				if (ack.status_mask[b] != TBB_SUCCESS) {
					status_ok = false;  // error
					LOG_DEBUG_STR (formatString("error, board-%d status=%u", b, ack.status_mask[b])); 
				}
			}

			if (status_ok) {
				TRAN(TBBControl::doTBBtrigsetup);  // go to next state.
			} else {
				LOG_ERROR_STR ("Failed to allocate the memory for the selected rcus");
				itsPropertySet->setValue(PN_FSM_ERROR,GCFPVString("alloc error"));
				sendControlResult(*itsParentPort, CONTROL_PREPARED, getName(), CT_RESULT_ALLOC_FAILED);
				setState(CTState::CLAIMED);
				TRAN(TBBControl::claimed_state);  // go to claimed_state state.
			}
		} break;

		default: {
			status = _defaultEventHandler(event, port);
		} break;
	}

	return (status);
}


//==============================================================================
// doTBBsetup(event, port)
//
// send TBB_TRIG_SETUP cmd to the TBBDriver
//==============================================================================
GCFEvent::TResult TBBControl::doTBBtrigsetup(GCFEvent& event, GCFPortInterface& port)
{
	LOG_DEBUG_STR ("doTBBtrigsetup:" << eventName(event) << "@" << port.getName());

	GCFEvent::TResult status = GCFEvent::HANDLED;

	static RCUset_t RCUset;

	switch (event.signal) {
		case F_ENTRY: {
			TBBTrigSetupEvent cmd;
			RCUset.reset();
			vector<TBBObservation::cSettings>::iterator it;
			for (it = itsObs->TbbSettings.begin(); it != itsObs->TbbSettings.end(); it++ ) {
				for (int i = 0; i < itsNrRCUs; i++) {
					if ((*it).RCUset.test(i)) {
						RCUset.set(i);
						cmd.rcu[i].level         = (*it).triggerLevel;
						cmd.rcu[i].start_mode    = (*it).startLevel;
						cmd.rcu[i].stop_mode     = (*it).stopLevel;
						cmd.rcu[i].filter_select = (*it).filter;
						cmd.rcu[i].window        = (*it).detectWindow;
						cmd.rcu[i].trigger_mode  = (*it).triggerMode;
						itsTriggerMode[i]        = (*it).triggerMode;
					}
				}
			}
			// info to pvss
			LOG_DEBUG_STR("send TBB_TRIG_SETUP cmd");
			itsTBBDriver->send(cmd);
		} break;

		case TBB_TRIG_SETUP_ACK: {
			TBBTrigSetupAckEvent ack(event);
			bool status_ok = true;

			for (int b = 0; b < itsNrTBBs; b++) {
			    if (isBoardUsed(b) == false) { continue; }
				if (ack.status_mask[b] != TBB_SUCCESS) {
					status_ok = false;  // error
					LOG_DEBUG_STR (formatString("error, board-%d status=%u", b, ack.status_mask[b])); 
				}
			}

			if (status_ok) {
				TRAN(TBBControl::doTBBtrigcoef);  // go to next state.
			} else {
				LOG_ERROR_STR ("Failed to setup the trigger system for the selected rcus");
				itsPropertySet->setValue(PN_FSM_ERROR,GCFPVString("setup error"));
				sendControlResult(*itsParentPort, CONTROL_PREPARED, getName(), CT_RESULT_TRIGSETUP_FAILED);
				setState(CTState::CLAIMED);
				TRAN(TBBControl::claimed_state);  // go to claimed_state state.
			}
		} break;

		default: {
			status = _defaultEventHandler(event, port);
		} break;
	}

	return (status);
}


//==============================================================================
// doTBBcoef(event, port)
//
// send TBB_TRIG_COEF cmd to the TBBDriver
//==============================================================================
GCFEvent::TResult TBBControl::doTBBtrigcoef(GCFEvent& event, GCFPortInterface& port)
{
	LOG_DEBUG_STR ("doTBBtrigcoef:" << eventName(event) << "@" << port.getName());

	GCFEvent::TResult status = GCFEvent::HANDLED;

	static RCUset_t RCUset;

	switch (event.signal) {
		case F_ENTRY: {
			TBBTrigCoefEvent cmd;
			RCUset.reset();
			vector<TBBObservation::cSettings>::iterator it;
			for (it = itsObs->TbbSettings.begin(); it != itsObs->TbbSettings.end(); it++ ) {
				for (int i = 0; i < itsNrRCUs; i++) {
					if ((*it).RCUset.test(i)) {
						RCUset.set(i);
						// each filter has 4 coeffiecients
						for (int c = 0; c < 4; c++) {
							cmd.rcu[i].filter0[c] = (*it).filter0[c];
							cmd.rcu[i].filter1[c] = (*it).filter1[c];
						}
					}
				}
			}
			// info to pvss
			LOG_DEBUG_STR("send TBB_TRIG_COEF cmd");
			itsTBBDriver->send(cmd);
		} break;

		case TBB_TRIG_COEF_ACK: {
			TBBTrigCoefAckEvent ack(event);
			bool status_ok = true;

			for (int b = 0; b < itsNrTBBs; b++) {
			    if (isBoardUsed(b) == false) { continue; }
				if (ack.status_mask[b] != TBB_SUCCESS) {
					status_ok = false;  // error
					LOG_DEBUG_STR (formatString("error, board-%d status=%u", b, ack.status_mask[b])); 
				}
			}

			if (status_ok) {
				TRAN(TBBControl::doTBBrecord);  // go to next state.
			} else {
				LOG_ERROR_STR ("Failed to setup the trigger coefficients for the selected rcus");
				itsPropertySet->setValue(PN_FSM_ERROR,GCFPVString("setup error"));
				sendControlResult(*itsParentPort, CONTROL_PREPARED, getName(), CT_RESULT_TRIGSETUP_FAILED);
				setState(CTState::CLAIMED);
				TRAN(TBBControl::claimed_state);  // go to claimed_state state.
			}
		} break;

		default: {
			status = _defaultEventHandler(event, port);
		} break;
	}

	return (status);
}






//==============================================================================
// doTBBrecord(event, port)
//
// send TBB_RECORD cmd to the TBBDriver
//==============================================================================
GCFEvent::TResult TBBControl::doTBBrecord(GCFEvent& event, GCFPortInterface& port)
{
	LOG_DEBUG_STR ("doTBBrecord:" << eventName(event) << "@" << port.getName());

	GCFEvent::TResult status = GCFEvent::HANDLED;

	static RCUset_t RCUset;

	switch (event.signal) {
		case F_ENTRY: {
			TBBRecordEvent cmd;
			RCUset.reset();
			vector<TBBObservation::cSettings>::iterator it;
			for (it = itsObs->TbbSettings.begin(); it != itsObs->TbbSettings.end(); it++ ) {
				for (int i = 0; i < itsNrRCUs; i++) {
					if ((*it).RCUset.test(i)) {
						RCUset.set(i);
						cmd.rcu_mask.set(i);
					}
				}
			}
			// info to pvss
			LOG_DEBUG_STR("send TBB_RECORD cmd");
			itsTBBDriver->send(cmd);
		} break;

		case TBB_RECORD_ACK: {
			TBBRecordAckEvent ack(event);
			bool status_ok = true;

			for (int b = 0; b < itsNrTBBs; b++) {
			    if (isBoardUsed(b) == false) { continue; }
				if (ack.status_mask[b] != TBB_SUCCESS) {
					status_ok = false;  // error
					LOG_DEBUG_STR (formatString("error, board-%d status=%u", b, ack.status_mask[b])); 
				}
			}

			if (status_ok) {
				TRAN(TBBControl::doTBBsubscribe);  // go to next state.
			} else {
				LOG_ERROR_STR ("Failed to start recording for the selected rcus");
				itsPropertySet->setValue(PN_FSM_ERROR,GCFPVString("record error"));
				sendControlResult(*itsParentPort, CONTROL_PREPARED, getName(), CT_RESULT_RECORD_FAILED);
				setState(CTState::CLAIMED);
				TRAN(TBBControl::claimed_state);  // go to claimed_state state.
			}

		} break;

		case TBB_TRIGGER:{
			status = _triggerEventHandler(event);
		} break;

		case TBB_TRIG_RELEASE_ACK: {
			status = _triggerReleaseAckEventHandler(event);
		} break;

		default: {
			status = _defaultEventHandler(event, port);
		} break;
	}

	return (status);
}


//==============================================================================
// doTBBsubscribe(event, port)
//
// send TBB_SUBSCRIBE cmd to the TBBDriver
//==============================================================================
GCFEvent::TResult TBBControl::doTBBsubscribe(GCFEvent& event, GCFPortInterface& port)
{
	LOG_DEBUG_STR ("doTBBsubscribe:" << eventName(event) << "@" << port.getName());

	GCFEvent::TResult status = GCFEvent::HANDLED;

	switch (event.signal) {
		case F_ENTRY: {
			TBBSubscribeEvent cmd;
			cmd.triggers = true;
			cmd.hardware = false;
			// info to pvss
			LOG_DEBUG_STR("send TBB_SUBSCRIBE cmd");
			itsTBBDriver->send(cmd);
		} break;

		case TBB_SUBSCRIBE_ACK: {
			TBBSubscribeAckEvent ack(event);
			sendControlResult(*itsParentPort, CONTROL_PREPARED, getName(), CT_RESULT_NO_ERROR);
			setState(CTState::PREPARED);
			TRAN(TBBControl::prepared_state);  // go to prepared state.
		} break;

		default: {
			status = _defaultEventHandler(event, port);
		} break;
	}

	return (status);
}

//==============================================================================
// doTBBrelease, port)
//
// send TBB_TRIG_RELEASE cmd to the TBBDriver
//==============================================================================
GCFEvent::TResult TBBControl::doTBBrelease(GCFEvent& event, GCFPortInterface& port)
{
	LOG_DEBUG_STR ("doTBBrelease:" << eventName(event) << "@" << port.getName());

	GCFEvent::TResult status = GCFEvent::HANDLED;

	static RCUset_t RCUset;

	switch (event.signal) {
		case F_ENTRY: {
			TBBTrigReleaseEvent cmd;
			RCUset.reset();
			vector<TBBObservation::cSettings>::iterator it;
			for (it = itsObs->TbbSettings.begin(); it != itsObs->TbbSettings.end(); it++ ) {
				for (int i = 0; i < itsNrRCUs; i++) {
					if ((*it).RCUset.test(i)) {
						RCUset.set(i);
						cmd.rcu_stop_mask.set(i);
						cmd.rcu_start_mask.set(i);
					}
				}
			}
			// info to pvss
			LOG_DEBUG_STR("send TBB_RELEASE cmd");
			itsTBBDriver->send(cmd);
		} break;

		case TBB_TRIG_RELEASE_ACK: {
			TBBTrigReleaseAckEvent ack(event);
			bool status_ok = true;

			for (int b = 0; b < itsNrTBBs; b++) {
			    if (isBoardUsed(b) == false) { continue; }
				if (ack.status_mask[b] != TBB_SUCCESS) {
					status_ok = false;  // error
					LOG_DEBUG_STR (formatString("error, board-%d status=%u", b, ack.status_mask[b])); 
				}
			}

			if (status_ok) {
				setState(CTState::RESUMED);
				TRAN(TBBControl::active_state);
			} else {
				LOG_ERROR_STR ("Failed to release the trigger system for the selected rcus");
				itsPropertySet->setValue(PN_FSM_ERROR,GCFPVString("release error"));
				sendControlResult(*itsParentPort, CONTROL_PREPARED, getName(), CT_RESULT_RELEASE_FAILED);
				setState(CTState::PREPARED);
				TRAN(TBBControl::prepared_state);
			}
		} break;

		default: {
			status = _defaultEventHandler(event, port);
		} break;
	}

	return (status);
}



//==============================================================================
// prepared_state(event, port)
//
// wait for RESUME event.
//==============================================================================
GCFEvent::TResult TBBControl::prepared_state(GCFEvent& event, GCFPortInterface& port)
{
	LOG_DEBUG_STR ("prepared:" << eventName(event) << "@" << port.getName());

	GCFEvent::TResult status = GCFEvent::HANDLED;

	switch (event.signal) {
		case F_ENTRY: {
			// update PVSS
			// itsPropertySet->setValue(string(PVSSNAME_FSM_CURACT),GCFPVString("claimed"));
			itsPropertySet->setValue(PN_FSM_ERROR, GCFPVString(""));
		} break;

		// -------------------- EVENTS RECEIVED FROM PARENT CONTROL --------------------
		case CONTROL_RESUME: {  // nothing to do, just send answer
			CONTROLResumeEvent msg(event);
			LOG_DEBUG_STR("Received RESUME(" << msg.cntlrName << ")");
			setState(CTState::RESUME);
			sendControlResult(port, CONTROL_RESUMED, msg.cntlrName, CT_RESULT_NO_ERROR);
			TRAN(TBBControl::doTBBrelease);
		} break;

		default: {
			status = _defaultEventHandler(event, port);
		} break;
	}

	return (status);
}

//==============================================================================
// active_state(event, port)
//
// Normal operation state.
//==============================================================================
GCFEvent::TResult TBBControl::active_state(GCFEvent& event, GCFPortInterface& port)
{
	LOG_DEBUG_STR ("active:" << eventName(event) << "@" << port.getName());

	GCFEvent::TResult status = GCFEvent::HANDLED;

	switch (event.signal) {
		case F_ENTRY: {
			// update PVSS
			// itsPropertySet->setValue(string(PVSSNAME_FSM_CURACT),GCFPVString("active"));
			itsPropertySet->setValue(PN_FSM_ERROR, GCFPVString(""));
			// start periodic timer to call VHECR task
			itsVHECRtimer->setTimer(VHECR_INTERVAL,VHECR_INTERVAL);
		} break;

		case F_TIMER: {
			if (&port == itsVHECRtimer) {
				itsVHECRTask->getReadCmd(itsStopCommandVector);
				if (!itsStopCommandVector.empty()) {
					itsVHECRtimer->cancelAllTimers();
					TRAN(TBBControl::doTBBread);
				}
			}
		} break;

		// -------------------- EVENTS RECEIVED FROM PARENT CONTROL --------------------

		case CONTROL_SCHEDULE: {
			CONTROLScheduledEvent msg(event);
			LOG_DEBUG_STR("Received SCHEDULE(" << msg.cntlrName << ")");
			// TODO: do something usefull with this information!
		} break;

		case CONTROL_RESUME: {  // nothing to do, just send answer
			CONTROLResumeEvent msg(event);
			LOG_DEBUG_STR("Received RESUME(" << msg.cntlrName << ")");
			setState(CTState::RESUME);
			sendControlResult(port, CONTROL_RESUMED, msg.cntlrName, CT_RESULT_NO_ERROR);
			setState(CTState::RESUMED);
		} break;

		case CONTROL_SUSPEND: {  // nothing to do, just send answer
			CONTROLSuspendEvent msg(event);
			LOG_DEBUG_STR("Received SUSPEND(" << msg.cntlrName << ")");
			setState(CTState::SUSPEND);
			sendControlResult(port, CONTROL_SUSPENDED, msg.cntlrName, CT_RESULT_NO_ERROR);
			setState(CTState::SUSPENDED);
			itsVHECRtimer->cancelAllTimers();
			TRAN(TBBControl::prepared_state);
		} break;

		// -------------------- EVENTS RECEIVED FROM TBBDRIVER --------------------
	// TODO TBB
		case TBB_TRIGGER:{
			status = _triggerEventHandler(event);
		} break;

		case TBB_TRIG_RELEASE_ACK: {
			status = _triggerReleaseAckEventHandler(event);
		} break;

		default: {
			status = _defaultEventHandler(event, port);
		}   break;
	}

	return (status);
}

//==============================================================================
// doTBBread(event, port)
//
// send TBB_READ cmd to the TBBDriver
//==============================================================================
GCFEvent::TResult TBBControl::doTBBread(GCFEvent& event, GCFPortInterface& port)
{
	LOG_DEBUG_STR ("doTBBread:" << eventName(event) << "@" << port.getName());

	GCFEvent::TResult status = GCFEvent::HANDLED;

	static int rcuNr;
	static RCUset_t RCUset;
	bool status_ok;

	switch (event.signal) {
		case F_ENTRY: {
			itsTimerPort->setTimer(0.0);
		} break;

		case F_TIMER: {
			if (&port == itsVHECRtimer) {
				// left empty
			}
			else {
				if (!itsStopCommandVector.empty()) {
					TBBStopEvent cmd;
					RCUset.reset();
					// first select RCUs to stop
					vector<TBBReadCmd>::iterator it1;
					for ( it1 = itsStopCommandVector.begin() ; it1 < itsStopCommandVector.end(); it1++ ) {
						// check if RCU is selected for observing
						vector<TBBObservation::cSettings>::iterator it2;
						for (it2 = itsObs->TbbSettings.begin(); it2 != itsObs->TbbSettings.end(); it2++ ) {
							if ((*it2).RCUset.test((*it1).itsRcuNr)) {
								RCUset.set((*it1).itsRcuNr);
								cmd.rcu_mask.set((*it1).itsRcuNr);
							}
						}
					}

					LOG_DEBUG_STR("send TBB_STOP cmd");
					itsTBBDriver->send(cmd);
				}

				// read CEP status to check if sending data to CEP is finished
				else {
					TBBCepStatusEvent cmd;
					cmd.boardmask = (1 << (rcuNr / 16));
					LOG_DEBUG_STR(formatString("send TBB_CEP_STATUS cmd to board %d", (rcuNr / 16)));
					itsTBBDriver->send(cmd);
				}
			}
		} break;

		case TBB_STOP_ACK: {
			TBBStopAckEvent ack(event);
			status_ok = true;

			for (int b = 0; b < itsNrTBBs; b++) {
			    if (isBoardUsed(b) == false) { continue; }
				if (ack.status_mask[b] != TBB_SUCCESS) {  // if error, check if rcu is used
					status_ok = false;
					LOG_DEBUG_STR(formatString("TBB_STOP_ACK err, board=%d", b));
				}
			}

			if (!status_ok) {
				LOG_ERROR_STR ("Failed to stop recording for selected rcus");
				itsPropertySet->setValue(PN_FSM_ERROR,GCFPVString("stop error"));
				TRAN(TBBControl::active_state); // go back to active state.
			}
			else {
				vector<TBBReadCmd>::iterator it;
				for ( it=itsStopCommandVector.begin() ; it < itsStopCommandVector.end(); it++ ) {
					// add stopped rcus to ReadVector
					if (RCUset.test((*it).itsRcuNr)) {
						itsReadCommandVector.push_back(*it);
					}
				}
				itsStopCommandVector.clear();

				// send read cmd to TBBDriver
				if (!itsReadCommandVector.empty()) {
					TBBReadCmd read;
					TBBReadEvent cmd;
					read = itsReadCommandVector.back();
					rcuNr           = static_cast<int>(read.itsRcuNr);
					cmd.rcu         = static_cast<int>(read.itsRcuNr);
					cmd.secondstime = read.itsTime;
					cmd.sampletime  = read.itsSampleTime;
					cmd.prepages    = read.itsPrePages;
					cmd. postpages  = read.itsPostPages;
					LOG_DEBUG_STR(formatString("send TBB_READ cmd: %u,%u,%u,%u,%u",
									read.itsRcuNr, read.itsTime, read.itsSampleTime, read.itsPrePages, read.itsPostPages));
					itsTBBDriver->send(cmd);
				}
			}
		} break;

		case TBB_READ_ACK: {
			TBBReadAckEvent ack(event);
			if (ack.status_mask == TBB_SUCCESS) {
				itsReadCommandVector.pop_back();
			}
			itsTimerPort->setTimer(0.01);
		} break;

		case TBB_CEP_STATUS_ACK: {
			TBBCepStatusAckEvent ack(event);
			LOG_DEBUG_STR(formatString("TBB_CEP_STATUS_ACK, rcuNr=%d  board=%d", rcuNr, (rcuNr / 16)));
			if (ack.pages_left[rcuNr / 16] != 0) {
				LOG_DEBUG_STR(formatString("TBB_CEP_STATUS_ACK pages_left = %d", ack.pages_left[rcuNr / 16]));
				itsTimerPort->setTimer(0.1);
			}
			else if (!itsReadCommandVector.empty()) {
				TBBReadEvent cmd;
				TBBReadCmd read;
				read = itsReadCommandVector.back();
				rcuNr           = static_cast<int>(read.itsRcuNr);
				cmd.rcu         = static_cast<int>(read.itsRcuNr);
				cmd.secondstime = read.itsTime;
				cmd.sampletime  = read.itsSampleTime;
				cmd.prepages    = read.itsPrePages;
				cmd. postpages  = read.itsPostPages;
				LOG_DEBUG_STR(formatString("send TBB_READ cmd: %u,%u,%u,%u,%u",
									read.itsRcuNr, read.itsTime, read.itsSampleTime, read.itsPrePages, read.itsPostPages));
				itsTBBDriver->send(cmd);
			}
			else {
				// set RCUs in record mode again
				TBBRecordEvent cmd;

				for (int i = 0; i < itsNrRCUs; i++) {
					cmd.rcu_mask.set(i,RCUset.test(i));
				}
				// info to pvss
				LOG_DEBUG_STR("send TBB_RECORD cmd");
				itsTBBDriver->send(cmd);
			}
		} break;

		case TBB_RECORD_ACK: {
			TBBRecordAckEvent ack(event);
			status_ok = true;

			for (int b = 0; b < itsNrTBBs; b++) {
			    if (isBoardUsed(b) == false) { continue; }
				if (ack.status_mask[b] != TBB_SUCCESS) {
					status_ok = false;  // error
					LOG_DEBUG_STR (formatString("error, board-%d status=%u", b, ack.status_mask[b])); 
				}
			}

			if (!status_ok) {
				LOG_WARN_STR ("Failed to start recording for the selected rcus");
				itsPropertySet->setValue(PN_FSM_ERROR,GCFPVString("record error"));
			}
			itsReadCommandVector.clear();

			// release all channels again
			TBBTrigReleaseEvent cmd;

			for (int i = 0; i < itsNrRCUs; i++) {
				if (RCUset.test(i)) {
					cmd.rcu_stop_mask.set(i);
					cmd.rcu_start_mask.set(i);
				}
			}

			LOG_DEBUG_STR("send TBB_RELEASE cmd");
			itsTBBDriver->send(cmd);
		} break;

		case TBB_TRIGGER:{
			status = _triggerEventHandler(event);
		} break;

		case TBB_TRIG_RELEASE_ACK: {
			TBBTrigReleaseAckEvent ack(event);
			status_ok = true;

			for (int b = 0; b < itsNrTBBs; b++) {
			    if (isBoardUsed(b) == false) { continue; }
				if (ack.status_mask[b] != TBB_SUCCESS) {
					status_ok = false;  // error
					LOG_DEBUG_STR (formatString("error, board-%d status=%u", b, ack.status_mask[b])); 
				}
			}

			if (!status_ok) {
				LOG_ERROR_STR ("Failed to release trigger system for selected rcus");
				itsPropertySet->setValue(PN_FSM_ERROR,GCFPVString("trigger release error"));
			}

			if (itsReadCommandVector.empty() && itsStopCommandVector.empty()) {
				TRAN(TBBControl::active_state); // go back to active state.
			}
		} break;

		case CONTROL_SUSPEND: {  // nothing to do, just send answer
			itsReadCommandVector.clear();
			CONTROLSuspendEvent msg(event);
			LOG_DEBUG_STR("Received SUSPEND(" << msg.cntlrName << ")");
			setState(CTState::SUSPEND);
			sendControlResult(port, CONTROL_SUSPENDED, msg.cntlrName, CT_RESULT_NO_ERROR);
			setState(CTState::SUSPENDED);
			TRAN(TBBControl::prepared_state);
		} break;

		default: {
			status = _defaultEventHandler(event, port);
		} break;
	}

	return (status);
}


//==============================================================================
// doTBBunsubscribe(event, port)
//
// send TBB_UNSUBSCRIBE cmd to the TBBDriver
//==============================================================================
GCFEvent::TResult TBBControl::doTBBunsubscribe(GCFEvent& event, GCFPortInterface& port)
{
	LOG_DEBUG_STR ("doTBBunsubscribe:" << eventName(event) << "@" << port.getName());

	GCFEvent::TResult status = GCFEvent::HANDLED;

	switch (event.signal) {
		case F_ENTRY: {
			TBBUnsubscribeEvent cmd;
			// info to pvss
			LOG_DEBUG_STR("send TBB_UNSUBSCRIBE cmd");
			itsTBBDriver->send(cmd);
		} break;

		case TBB_UNSUBSCRIBE_ACK: {
			TBBUnsubscribeAckEvent ack(event);
			setState(CTState::RELEASED);
			sendControlResult(*itsParentPort, CONTROL_RELEASED, getName(), CT_RESULT_NO_ERROR);
			TRAN(TBBControl::released_state);
		} break;

		default: {
			status = _defaultEventHandler(event, port);
		} break;
	}

	return (status);
}


//==============================================================================
// released_state(event, port)
//
// wait for PREPARE event.
//==============================================================================
GCFEvent::TResult TBBControl::released_state(GCFEvent& event, GCFPortInterface& port)
{
	LOG_DEBUG_STR ("released:" << eventName(event) << "@" << port.getName());

	GCFEvent::TResult status = GCFEvent::HANDLED;

	switch (event.signal) {
		case F_ENTRY: {
			// update PVSS
			// itsPropertySet->setValue(string(PVSSNAME_FSM_CURACT),GCFPVString("claimed"));
			itsPropertySet->setValue(PN_FSM_ERROR, GCFPVString(""));
		} break;

		// -------------------- EVENTS RECEIVED FROM PARENT CONTROL --------------------

		case CONTROL_QUIT: {
			TRAN(TBBControl::quiting_state);
		} break;

		default: {
			status = _defaultEventHandler(event, port);
		} break;
	}

	return (status);
}


//==============================================================================
// quiting_state(event, port)
//
// Quiting: send QUITED, wait for answer max 5 seconds, stop
//==============================================================================
GCFEvent::TResult TBBControl::quiting_state(GCFEvent& event, GCFPortInterface& port)
{
	LOG_DEBUG_STR ("quiting:" << eventName(event) << "@" << port.getName());

	GCFEvent::TResult status = GCFEvent::HANDLED;

	switch (event.signal) {
		case F_ENTRY: {
			// update PVSS
			setState(CTState::QUIT);
			// tell Parent task we like to go down.
			itsParentControl->nowInState(getName(), CTState::QUIT);

	//      itsPropertySet->setValue(string(PVSSNAME_FSM_CURACT),GCFPVString("quiting"));
			itsPropertySet->setValue(PN_FSM_ERROR, GCFPVString(""));
			// disconnect from TBBDriver
			itsTBBDriver->close();
		} break;

		case F_INIT:
		case F_EXIT: {
		} break;

		case F_DISCONNECTED: {      // propably from beamserver
			port.close();
			ASSERTSTR (&port == itsTBBDriver,
									"F_DISCONNECTED event from port " << port.getName());
			LOG_INFO("Connection with TBBDriver down, sending QUITED to parent");
			CONTROLQuitedEvent request;
			request.cntlrName = getName();
			request.result    = CT_RESULT_NO_ERROR;
			itsParentPort->send(request);
			itsTimerPort->setTimer(1.0);  // wait 1 second to let message go away

		} break;

		case F_TIMER: {
			GCFScheduler::instance()->stop();
		} break;

		default: {
			status = _defaultEventHandler(event, port);
		} break;
	}

	return (status);
}

//
// send data(see data info) to CEP from selected rcus
//
void TBBControl::readTBBdata(vector<TBBReadCmd> readCmd)
{
	LOG_DEBUG_STR("Received read cmd from VHECRTask");
	//itsCommandVector.clear();
	itsStopCommandVector = readCmd;
	itsTimerPort->setTimer(0.0);
}

// _triggerEventHandler(event, port)
//
GCFEvent::TResult TBBControl::_triggerEventHandler(GCFEvent& event)
{
	GCFEvent::TResult   result(GCFEvent::NOT_HANDLED);

	// if not in active_state, return.
	if (itsState != CTState::RESUMED) return (result);

	TBBTriggerEvent msg(event);

	TBBTrigger trigger( static_cast<uint32>(msg.rcu),
											msg.sequence_nr,
											msg.time,
											msg.sample_nr,
											msg.trigger_sum,
											msg.trigger_samples,
											msg.peak_value,
											0);
	itsVHECRTask->addTrigger(trigger);

	LOG_TRACE_FLOW ("Sending trigger to PVSS");

	itsPropertySet->setValue(PN_TBC_TRIGGER_RCU_NR,      GCFPVInteger(msg.rcu),            0.0, false);
	itsPropertySet->setValue(PN_TBC_TRIGGER_SEQUENCE_NR, GCFPVInteger(msg.sequence_nr),    0.0, false);
	itsPropertySet->setValue(PN_TBC_TRIGGER_TIME,        GCFPVInteger(msg.time),           0.0, false);
	itsPropertySet->setValue(PN_TBC_TRIGGER_SAMPLE_NR,   GCFPVInteger(msg.sample_nr),      0.0, false);
	itsPropertySet->setValue(PN_TBC_TRIGGER_SUM,         GCFPVInteger(msg.trigger_sum),    0.0, false);
	itsPropertySet->setValue(PN_TBC_TRIGGER_NR_SAMPLES,  GCFPVInteger(msg.trigger_samples),0.0, false);
	itsPropertySet->setValue(PN_TBC_TRIGGER_PEAK_VALUE,  GCFPVInteger(msg.peak_value),     0.0, false);
	// The Navigator also needs all values combined into one field
	string collection(formatString("%d|%d|%d|%d|%d|%d|%d|0", msg.rcu, msg.sequence_nr, msg.time,
							msg.sample_nr, msg.trigger_sum, msg.trigger_samples, msg.peak_value));
	itsPropertySet->setValue(PN_TBC_TRIGGER_TABLE,       GCFPVString (collection),         0.0, false);
	itsPropertySet->flush();

	if((itsTriggerMode[msg.rcu] & 1) == 0) {
		// release trigger system if mode is one shot (bit0 = 0)
		TBBTrigReleaseEvent release;
		release.rcu_stop_mask.set(msg.rcu);
		release.rcu_start_mask.set(msg.rcu);
		itsTBBDriver->send(release);
	}
	return (result);
}

// _triggerReleaseAckEventHandler(event, port)
//
GCFEvent::TResult TBBControl::_triggerReleaseAckEventHandler(GCFEvent& event)
{
	GCFEvent::TResult result(GCFEvent::NOT_HANDLED);

	TBBTrigReleaseAckEvent ack(event);

	bool status_ok = true;

	for (int b = 0; b < itsNrTBBs; b++) {
	    if (isBoardUsed(b) == false) { continue; }
		if (ack.status_mask[b] != TBB_SUCCESS) {  // if error, check if rcu is used
			status_ok = false;  // check if rcu is selected
			LOG_DEBUG_STR (formatString("error, board-%d status=%u", b, ack.status_mask[b])); 
		}
	}

	if (!status_ok) {
		LOG_ERROR_STR ("Failed to release trigger system for selected rcus");
		itsPropertySet->setValue(PN_FSM_ERROR,GCFPVString("trigger release error"));
	}
	return (result);
}


// _defaultEventHandler(event, port)
//
GCFEvent::TResult TBBControl::_defaultEventHandler(GCFEvent&            event,
													GCFPortInterface&   port)
{
	CTState     cts;
	LOG_DEBUG_STR("Received " << eventName(event) << " in state " << cts.name(itsState)
				  << ". DEFAULT handling.");

	GCFEvent::TResult   result(GCFEvent::HANDLED);

	switch (event.signal) {

		case CONTROL_CONNECT:
		case CONTROL_RESYNC:
		case CONTROL_SCHEDULE:  // TODO: we should do something with this
		case CONTROL_CLAIM:
		case CONTROL_PREPARE:
		case CONTROL_RESUME:
		case CONTROL_SUSPEND:
		case CONTROL_QUIT: {
			if (!sendControlResult(port, event.signal, getName(), CT_RESULT_NO_ERROR)) {
				result = GCFEvent::NOT_HANDLED;
			}   break;
		}

		case CONTROL_CONNECTED:
		case CONTROL_RESYNCED:
		case CONTROL_SCHEDULED:
		case CONTROL_CLAIMED:
		case CONTROL_PREPARED:
		case CONTROL_RESUMED:
		case CONTROL_SUSPENDED:
		case CONTROL_RELEASED:
		case CONTROL_QUITED: {
		} break;

		case F_DISCONNECTED: {
			port.close();
			if (&port == itsTBBDriver) {
				itsVHECRtimer->cancelAllTimers();
				LOG_DEBUG_STR("Connection with TBBDriver lost, going to started state");
				itsPropertySet->setValue(PN_TBC_CONNECTED,  GCFPVBool(false));  // [TBB]
				itsPropertySet->setValue(PN_FSM_ERROR,GCFPVString("connection lost"));
				TRAN (TBBControl::started_state);
			}
		} break;

		case CONTROL_RELEASE: {
			CONTROLReleaseEvent     msg(event);
			LOG_DEBUG_STR("Received RELEASE(" << msg.cntlrName << ")");
			sendControlResult(port, CONTROL_RELEASED, msg.cntlrName, CT_RESULT_NO_ERROR);
			setState(CTState::RELEASE);
			TRAN(TBBControl::doTBBunsubscribe);
		} break;

		case DP_CHANGED: {
			LOG_DEBUG_STR("DP " << DPChangedEvent(event).DPname << " was changed");
		} break;

		case DP_SET: {
			LOG_DEBUG_STR("DP " << DPSetEvent(event).DPname << " was set");
		}   break;

		case F_INIT:
		case F_EXIT: {
		}   break;

		default: {
			result = GCFEvent::NOT_HANDLED;
			LOG_WARN_STR("Event " << eventName(event) << " NOT handled in state " << cts.name(itsState));
		}
	}

	return (result);
}

bool TBBControl::isBoardUsed(int board)
{
    int start = board * 16;
    int stop = start + 16;
    for (int i = start; i < stop; i++) {
        if (itsRCUs.test(i)) { return(true); }
    }
    return(false);
}

	}  // end off namespace
}  // end off namespace
