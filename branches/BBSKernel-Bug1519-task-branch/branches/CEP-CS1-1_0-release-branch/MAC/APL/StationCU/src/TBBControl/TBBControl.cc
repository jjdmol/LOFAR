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

/*
####################################
// moeten nog in ControllerDefines.h
CT_RESULT_NOBOARD_ERROR,
CT_RESULT_MODE_ERROR,
CT_RESULT_ALLOC_ERROR,
CT_RESULT_TRIGSETUP_ERROR,
CT_RESULT_TRIGCOEF_ERROR,
CT_RESULT_RECORD_ERROR,

CNTLRTYPE_TBBCTRL,					// TBBControl

####################################
*/
#include <lofar_config.h>
#include <Common/LofarLogger.h>

#include <APS/ParameterSet.h>
#include <GCF/GCF_PVTypes.h>
#include <GCF/Utils.h>
#include <GCF/GCF_ServiceInfo.h>
#include <APL/APLCommon/APL_Defines.h>
#include <APL/APLCommon/APLUtilities.h>
#include <APL/APLCommon/Controller_Protocol.ph>
#include <APL/APLCommon/StationInfo.h>
#include <APL/TBB_Protocol/TBB_Protocol.ph>
#include <GCF/RTDB/DP_Protocol.ph>
#include <signal.h>

//# local includes
#include "TBBControl.h"
#include "TBBControlDefines.h"
#include "TBBObservation.h"
#include "TBBTrigger.h"
#include "TBBReadCmd.h"
#include "VHECRTask.h"

using namespace LOFAR::GCF::Common;
using namespace LOFAR::GCF::TM;
using namespace LOFAR::GCF::RTDB;
using namespace std;

using namespace LOFAR;
using namespace APLCommon;
using namespace ACC::APS;
using namespace StationCU;
	
// static pointer to this object for signal handler
static TBBControl*	thisTBBControl = 0;

//
// TBBControl()
//
TBBControl::TBBControl(const string&	cntlrName) :
	GCFTask 			((State)&TBBControl::initial_state,cntlrName),
	itsPropertySet		(0),
	itsPropertySetInitialized (false),
	itsParentControl	(0),
	itsParentPort		(0),
	itsTimerPort		(0),
	itsTBBDriver		(0),
	itsState			(CTState::NOSTATE)
{
	LOG_TRACE_OBJ_STR (cntlrName << " construction");

	// First readin our observation related config file.
	LOG_DEBUG_STR("Reading parset file:" << LOFAR_SHARE_LOCATION << "/" << cntlrName);
	globalParameterSet()->adoptFile(string(LOFAR_SHARE_LOCATION)+"/"+cntlrName);

	itsObs = new TBBObservation(globalParameterSet());	// does all nasty conversions

	// Readin some parameters from the ParameterSet.
	itsTreePrefix = globalParameterSet()->getString("prefix");
	itsInstanceNr = globalParameterSet()->getUint32("_instanceNr");

	// attach to parent control task
	itsParentControl = ParentControl::instance();
	itsParentPort = new GCFITCPort (*this, *itsParentControl, "ParentITCport", 
									GCFPortInterface::SAP, CONTROLLER_PROTOCOL);
	ASSERTSTR(itsParentPort, "Cannot allocate ITCport for Parentcontrol");
	itsParentPort->open();		// will result in F_CONNECTED

	// need port for timers.
	itsTimerPort = new GCFTimerPort(*this, "TimerPort");

	// prepare TCP port to TBBDriver.
	itsTBBDriver = new GCFTCPPort (*this, MAC_SVCMASK_TBBDRIVER,
											GCFPortInterface::SAP, TBB_PROTOCOL);
	ASSERTSTR(itsTBBDriver, "Cannot allocate TCPport to TBBDriver");

	// for debugging purposes
	GCF::TM::registerProtocol (CONTROLLER_PROTOCOL, CONTROLLER_PROTOCOL_STRINGS);
	GCF::TM::registerProtocol (DP_PROTOCOL, 		DP_PROTOCOL_STRINGS);
	GCF::TM::registerProtocol (TBB_PROTOCOL, 		TBB_PROTOCOL_STRINGS);

	setState(CTState::CREATED);
	
	itsCommandVector.clear();					// clear buffer
	itsVHECRTask = VHECRTask::instance();
	itsVHECRTask->setSaveTask(this);
}


//
// ~TBBControl()
//
TBBControl::~TBBControl()
{
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
void    TBBControl::setState(CTState::CTstateNr     newState)
{
	itsState = newState;

	if (itsPropertySet) {
		CTState		cts;
		itsPropertySet->setValue(PVSSNAME_FSM_CURACT, GCFPVString(cts.name(newState)));
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
		}	break;
	
	  case F_INIT: {
			// Get access to my own propertyset.
			string	propSetName(createPropertySetName(PSN_TBB_CTRL, getName()));
			LOG_INFO_STR ("Activating PropertySet" << propSetName);
			itsPropertySet = new RTDBPropertySet(propSetName,
												 PST_TBB_CTRL,
												 PSAT_RW,
												 this);
			// Wait for timer that is set on DP_CREATED event
		}	break;
	
		case DP_CREATED: {
			// NOTE: thsi function may be called DURING the construction of the PropertySet.
			// Always exit this event in a way that GCF can end the construction.
			DPCreatedEvent	dpEvent(event);
			LOG_DEBUG_STR("Result of creating " << dpEvent.DPname << " = " << dpEvent.result);
			itsTimerPort->cancelAllTimers();
			itsTimerPort->setTimer(0.0);
		}	break;
		  
		case F_TIMER: {
			if (!itsPropertySetInitialized) {
				itsPropertySetInitialized = true;
	
				// Instruct codeloggingProcessor
				// TODO: get this from a .h file
				LOG_INFO_STR("MACProcessScope: LOFAR.PermSW.TBBCtrl");
	
				// first redirect signalHandler to our quiting state to leave PVSS
				// in the right state when we are going down
				thisTBBControl = this;
				signal (SIGINT,  TBBControl::sigintHandler);	// ctrl-c
				signal (SIGTERM, TBBControl::sigintHandler);	// kill
	
				// update PVSS.
				LOG_TRACE_FLOW ("Updateing state to PVSS");
	// TODO TBB
				itsPropertySet->setValue(PVSSNAME_FSM_CURACT,	GCFPVString("initial"));
				itsPropertySet->setValue(PVSSNAME_FSM_ERROR,	GCFPVString(""));
				itsPropertySet->setValue(PN_TBC_CONNECTED,	GCFPVBool(false));
				
				itsPropertySet->setValue(PN_TBC_TRIGGER_RCU_NR,	GCFPVInteger(0),0.0,false);			
				itsPropertySet->setValue(PN_TBC_TRIGGER_SEQUENCE_NR,	GCFPVInteger(0),0.0,false);
				itsPropertySet->setValue(PN_TBC_TRIGGER_TIME,	GCFPVInteger(0),0.0,false);
				itsPropertySet->setValue(PN_TBC_TRIGGER_SAMPLE_NR,	GCFPVInteger(0),0.0,false);
				itsPropertySet->setValue(PN_TBC_TRIGGER_SUM,	GCFPVInteger(0),0.0,false);
				itsPropertySet->setValue(PN_TBC_TRIGGER_NR_SAMPLES,	GCFPVInteger(0),0.0,false);
				itsPropertySet->setValue(PN_TBC_TRIGGER_PEAK_VALUE,	GCFPVInteger(0),0.0,false);
				itsPropertySet->flush();
			  
				// Start ParentControl task
				LOG_DEBUG ("Enabling ParentControl task and wait for my name");
				itsParentPort = itsParentControl->registerTask(this);
				// results in CONTROL_CONNECT
			}
		} break;
	
		case F_CONNECTED: {
			ASSERTSTR (&port == itsParentPort, 
										"F_CONNECTED event from port " << port.getName());
		} break;
	
		case F_DISCONNECTED:
		case F_CLOSED:
		case F_EXIT: {
		} break;
		
		case CONTROL_CONNECT: {
			CONTROLConnectEvent		msg(event);
			LOG_DEBUG_STR("Received CONNECT(" << msg.cntlrName << ")");
			setState(CTState::CONNECTED);
			sendControlResult(port, CONTROL_CONNECTED, msg.cntlrName, CT_RESULT_NO_ERROR);
	
			// let ParentControl watch over the start and stop times for extra safety.
			ptime	startTime = time_from_string(globalParameterSet()->
														getString("Observation.startTime"));
			ptime	stopTime  = time_from_string(globalParameterSet()->
														getString("Observation.stopTime"));
			itsParentControl->activateObservationTimers(msg.cntlrName, startTime, stopTime);
	
			LOG_INFO ("Going to started state");
			TRAN(TBBControl::started_state);				// go to next state.
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
	//		itsPropertySet->setValue(string(PVSSNAME_FSM_CURACT),GCFPVString("started"));
			itsPropertySet->setValue(PVSSNAME_FSM_ERROR,GCFPVString(""));
			itsPropertySet->setValue(PN_TBC_CONNECTED,	GCFPVBool(false)); //[TBB]
		} break;
	
		case F_INIT:
		case F_EXIT: {
		} break;
	
		case F_CONNECTED: {
			ASSERTSTR (&port == itsTBBDriver, "F_CONNECTED event from port " 
																		<< port.getName());
			itsTimerPort->cancelAllTimers();
			LOG_INFO ("Connected with TBBDriver, going to claimed state");
			itsPropertySet->setValue(PN_TBC_CONNECTED,	GCFPVBool(true));	// [TBB]
			setState(CTState::CLAIMED);
			sendControlResult(*itsParentPort, CONTROL_CLAIMED, getName(), CT_RESULT_NO_ERROR);
			TRAN(TBBControl::claimed_state);				// go to next state.
		} break;
	
		case F_DISCONNECTED: {
			port.close();
			ASSERTSTR (&port == itsTBBDriver, 
									"F_DISCONNECTED event from port " << port.getName());
			LOG_WARN ("Connection with TBBDriver failed, retry in 2 seconds");
			itsTimerPort->setTimer(2.0);
		} break;
	
		case F_CLOSED: {
		} break;
	
		case F_TIMER: { 
	//		GCFTimerEvent& timerEvent=static_cast<GCFTimerEvent&>(event);
			LOG_DEBUG ("Trying to reconnect to TBBDriver");
			itsTBBDriver->open();		// will result in F_CONN or F_DISCONN
		} break;
	
		// -------------------- EVENTS RECEIVED FROM PARENT CONTROL --------------------
		case CONTROL_CLAIM: {
			CONTROLClaimEvent		msg(event);
			LOG_DEBUG_STR("Received CLAIM(" << msg.cntlrName << ")");
			setState(CTState::CLAIM);
			LOG_DEBUG ("Trying to connect to TBBDriver");
			itsTBBDriver->open();		// will result in F_CONN or F_DISCONN
		} break;
	
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
			itsPropertySet->setValue(PVSSNAME_FSM_ERROR, GCFPVString(""));
		} break;
	
		case F_DISCONNECTED: {
			port.close();
			ASSERTSTR (&port == itsTBBDriver, 
									"F_DISCONNECTED event from port " << port.getName());
			LOG_WARN("Connection with TBBDriver lost, going to reconnect state.");
			TRAN(TBBControl::started_state);
		} break;
	
		// -------------------- EVENTS RECEIVED FROM PARENT CONTROL --------------------
		case CONTROL_PREPARE: {
			CONTROLPrepareEvent		msg(event);
			LOG_DEBUG_STR("Received PREPARE(" << msg.cntlrName << ")");
			setState(CTState::PREPARE);
			
			// start setting up the observation
			TRAN(TBBControl::doTBBmode);		
		} break;
	
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
// doTBBmode(event, port)
//
// send TBB_MODE cmd to the TBBDriver
//==============================================================================
GCFEvent::TResult TBBControl::doTBBmode(GCFEvent& event, GCFPortInterface& port)
{
	LOG_DEBUG_STR ("doTBBmode:" << eventName(event) << "@" << port.getName());

	GCFEvent::TResult status = GCFEvent::HANDLED;

	switch (event.signal) {
		case F_ENTRY: {
			TBBModeEvent cmd;
			
			cmd.rec_mode = itsObs->operatingMode;
			// info to pvss
		
			itsTBBDriver->send(cmd);
		} break;

		case TBB_MODE_ACK: {
			TBBModeAckEvent ack(event);
			bool status_ok = true;
			
			for (int i = 0; i < MAX_N_TBBBOARDS; i++) {
				if (ack.status_mask[i] != TBB_SUCCESS) {  // if error, check if rcu is used
					for (int i = (i * 16); i < ((i + 1) * 16); i++) {  // loop over rcu's on board[i]
						if (itsObs->allRCUset.test(i)) { status_ok = false;	} // check if rcu is selected
					}
				}
			}
			
			if (status_ok) {
				TRAN(TBBControl::doTBBalloc);				// go to next state.
			} else {
				LOG_ERROR_STR ("Failed to set the operating mode for all the rcus");
				itsPropertySet->setValue(PVSSNAME_FSM_ERROR,GCFPVString("operatingMode error"));
				setState(CTState::CLAIMED);
				TRAN(TBBControl::claimed_state);			// go to claimed_state state.
			}
		} break;
	
		case F_DISCONNECTED: {
			_disconnectedHandler(port);
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

	switch (event.signal) {
		case F_ENTRY: {
			TBBAllocEvent cmd;

			for (int i = 0; i < MAX_N_RCUS; i++) {
				cmd.rcu_mask.set(i,itsObs->allRCUset.test(i));
			}    
			// info to pvss
		
			itsTBBDriver->send(cmd);
		} break;

		case TBB_ALLOC_ACK: {
			TBBModeAckEvent ack(event);
			bool status_ok = true;
			
			for (int i = 0; i < MAX_N_TBBBOARDS; i++) {
				if (ack.status_mask[i] != TBB_SUCCESS) {  // if error, check if rcu is used
					for (int i = (i * 16); i < ((i + 1) * 16); i++) {  // loop over rcu's on board[i]
						if (itsObs->allRCUset.test(i)) { status_ok = false;	} // check if rcu is selected
					}
				}
			}
			
			if (status_ok) {
				TRAN(TBBControl::doTBBtrigsetup);				// go to next state.
			} else {
				LOG_ERROR_STR ("Failed to allocate the memory for the selected rcus");
				itsPropertySet->setValue(PVSSNAME_FSM_ERROR,GCFPVString("alloc error"));
				setState(CTState::CLAIMED);
				TRAN(TBBControl::claimed_state);			// go to claimed_state state.
			}
		} break;
	
		case F_DISCONNECTED: {
			_disconnectedHandler(port);
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

	switch (event.signal) {
		case F_ENTRY: {
			TBBTrigSetupEvent cmd;
			
			vector<TBBObservation::sTBBsetting>::iterator it;
			for (it = itsObs->TBBsetting.begin(); it != itsObs->TBBsetting.end(); it++ ) {
				for (int i = 0; i < MAX_N_RCUS; i++) {
					if ((*it).RCUset.test(i)) {
						cmd.setup[i].level				= (*it).triggerLevel;
						cmd.setup[i].start_mode		= (*it).startLevel;
						cmd.setup[i].stop_mode		= (*it).stopLevel;
						cmd.setup[i].filter_select= (*it).filter;
						cmd.setup[i].window				= (*it).detectWindow;
					}    
				}   
			}
			// info to pvss
		
			itsTBBDriver->send(cmd);
		} break;

		case TBB_TRIG_SETUP_ACK: {
			TBBTrigSetupAckEvent ack(event);
			bool status_ok = true;
			
			for (int i = 0; i < MAX_N_TBBBOARDS; i++) {
				if (ack.status_mask[i] != TBB_SUCCESS) {  // if error, check if rcu is used
					for (int i = (i * 16); i < ((i + 1) * 16); i++) {  // loop over rcu's on board[i]
						if (itsObs->allRCUset.test(i)) { status_ok = false;	} // check if rcu is selected
					}
				}
			}
			
			if (status_ok) {
				TRAN(TBBControl::doTBBtrigcoef);				// go to next state.
			} else {
				LOG_ERROR_STR ("Failed to setup the trigger system for the selected rcus");
				itsPropertySet->setValue(PVSSNAME_FSM_ERROR,GCFPVString("setup error"));
				setState(CTState::CLAIMED);
				TRAN(TBBControl::claimed_state);			// go to claimed_state state.
			}
		} break;
	
		case F_DISCONNECTED: {
			_disconnectedHandler(port);
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

	switch (event.signal) {
		case F_ENTRY: {
			TBBTrigCoefEvent cmd;
			
			vector<TBBObservation::sTBBsetting>::iterator it;
			for (it = itsObs->TBBsetting.begin(); it != itsObs->TBBsetting.end(); it++ ) {
				for (int i = 0; i < MAX_N_RCUS; i++) {
					if ((*it).RCUset.test(i)) {
						cmd.coefficients[i].c0	= (*it).c0;
						cmd.coefficients[i].c1	= (*it).c1;
						cmd.coefficients[i].c2	= (*it).c2;
						cmd.coefficients[i].c3	= (*it).c3;
					}    
				}   
			}
			// info to pvss
		
			itsTBBDriver->send(cmd);
		} break;

		case TBB_TRIG_COEF_ACK: {
			TBBTrigCoefAckEvent ack(event);
			bool status_ok = true;
			
			for (int i = 0; i < MAX_N_TBBBOARDS; i++) {
				if (ack.status_mask[i] != TBB_SUCCESS) {  // if error, check if rcu is used
					for (int i = (i * 16); i < ((i + 1) * 16); i++) {  // loop over rcu's on board[i]
						if (itsObs->allRCUset.test(i)) { status_ok = false;	} // check if rcu is selected
					}
				}
			}
			
			if (status_ok) {
				TRAN(TBBControl::doTBBrecord);				// go to next state.
			} else {
				LOG_ERROR_STR ("Failed to setup the trigger coefficients for the selected rcus");
				itsPropertySet->setValue(PVSSNAME_FSM_ERROR,GCFPVString("setup error"));
				setState(CTState::CLAIMED);
				TRAN(TBBControl::claimed_state);			// go to claimed_state state.
			}
		} break;
	
		case F_DISCONNECTED: {
			_disconnectedHandler(port);
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

	switch (event.signal) {
		case F_ENTRY: {
			TBBRecordEvent cmd;
			
			for (int i = 0; i < MAX_N_RCUS; i++) {
				cmd.rcu_mask.set(i,itsObs->allRCUset.test(i));
			} 
			// info to pvss
		
			itsTBBDriver->send(cmd);
		} break;

		case TBB_RECORD_ACK: {
			TBBTrigCoefAckEvent ack(event);
			bool status_ok = true;
			
			for (int i = 0; i < MAX_N_TBBBOARDS; i++) {
				if (ack.status_mask[i] != TBB_SUCCESS) {  // if error, check if rcu is used
					for (int i = (i * 16); i < ((i + 1) * 16); i++) {  // loop over rcu's on board[i]
						if (itsObs->allRCUset.test(i)) { status_ok = false;	} // check if rcu is selected
					}
				}
			}
			
			if (status_ok) {
				TRAN(TBBControl::doTBBrelease);				// go to next state.
			} else {
				LOG_ERROR_STR ("Failed to start recording for the selected rcus");
				itsPropertySet->setValue(PVSSNAME_FSM_ERROR,GCFPVString("record error"));
				setState(CTState::CLAIMED);
				TRAN(TBBControl::claimed_state);			// go to claimed_state state.
			}
		} break;
	
		case F_DISCONNECTED: {
			_disconnectedHandler(port);
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
			// info to pvss
			itsTBBDriver->send(cmd);
		} break;

		case TBB_SUBSCRIBE_ACK: {
			TBBSubscribeAckEvent ack(event);
			
			TRAN(TBBControl::doTBBrelease);				// go to next state.
		} break;
	
		case F_DISCONNECTED: {
			_disconnectedHandler(port);
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

	switch (event.signal) {
		case F_ENTRY: {
			TBBTrigReleaseEvent cmd;
			
			for (int i = 0; i < MAX_N_RCUS; i++) {
				cmd.rcu_stop_mask.set(i,itsObs->allRCUset.test(i));
				cmd.rcu_start_mask.set(i,itsObs->allRCUset.test(i));
			} 
				
			// info to pvss
			itsTBBDriver->send(cmd);
		} break;

		case TBB_TRIG_RELEASE_ACK: {
			TBBTrigReleaseAckEvent ack(event);
			bool status_ok = true;
			
			for (int i = 0; i < MAX_N_TBBBOARDS; i++) {
				if (ack.status_mask[i] != TBB_SUCCESS) {  // if error, check if rcu is used
					for (int i = (i * 16); i < ((i + 1) * 16); i++) {  // loop over rcu's on board[i]
						if (itsObs->allRCUset.test(i)) { status_ok = false;	} // check if rcu is selected
					}
				}
			}
			
			if (status_ok) {
				TRAN(TBBControl::prepared_state);				// go to next state.
			} else {
				LOG_ERROR_STR ("Failed to release the trigger system for the selected rcus");
				itsPropertySet->setValue(PVSSNAME_FSM_ERROR,GCFPVString("release error"));
				setState(CTState::CLAIMED);
				TRAN(TBBControl::claimed_state);			// go to claimed_state state.
			}
		} break;
	
		case F_DISCONNECTED: {
			_disconnectedHandler(port);
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
			itsPropertySet->setValue(PVSSNAME_FSM_ERROR, GCFPVString(""));
		} break;
		
		case F_DISCONNECTED: {
			port.close();
			ASSERTSTR (&port == itsTBBDriver, 
									"F_DISCONNECTED event from port " << port.getName());
			LOG_WARN("Connection with TBBDriver lost, going to reconnect state.");
			TRAN(TBBControl::started_state);
		} break;
	
		// -------------------- EVENTS RECEIVED FROM PARENT CONTROL --------------------
		case CONTROL_RESUME: {		// nothing to do, just send answer
			CONTROLResumeEvent		msg(event);
			LOG_DEBUG_STR("Received RESUME(" << msg.cntlrName << ")");
			setState(CTState::RESUME);
			sendControlResult(port, CONTROL_RESUMED, msg.cntlrName, CT_RESULT_NO_ERROR);
			setState(CTState::RESUMED);
			TRAN(TBBControl::active_state);
		} break;
			
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
			itsPropertySet->setValue(PVSSNAME_FSM_ERROR, GCFPVString(""));
		} break;
	
		case F_INIT:
		case F_EXIT: {
		} break;
	
		case F_DISCONNECTED: {
			port.close();
			ASSERTSTR (&port == itsTBBDriver, 
									"F_DISCONNECTED event from port " << port.getName());
			LOG_WARN("Connection with TBBDriver lost, going to reconnect");
			TRAN(TBBControl::started_state);
		} break;
		
		case F_TIMER: {
			// readCmd received from VHECRTask
			TRAN(TBBControl::doTBBstop);
		} break;
				
		case F_CLOSED: {
		} break;
	
		// -------------------- EVENTS RECEIVED FROM PARENT CONTROL --------------------
	
		case CONTROL_SCHEDULE: {
			CONTROLScheduledEvent		msg(event);
			LOG_DEBUG_STR("Received SCHEDULE(" << msg.cntlrName << ")");
			// TODO: do something usefull with this information!
		} break;
	
		case CONTROL_RESUME: {		// nothing to do, just send answer
			CONTROLResumeEvent		msg(event);
			LOG_DEBUG_STR("Received RESUME(" << msg.cntlrName << ")");
			setState(CTState::RESUME);
			sendControlResult(port, CONTROL_RESUMED, msg.cntlrName, CT_RESULT_NO_ERROR);
			setState(CTState::RESUMED);
		} break;
	
		case CONTROL_SUSPEND: {		// nothing to do, just send answer
			CONTROLSuspendEvent		msg(event);
			LOG_DEBUG_STR("Received SUSPEND(" << msg.cntlrName << ")");
			setState(CTState::SUSPEND);
			sendControlResult(port, CONTROL_SUSPENDED, msg.cntlrName, CT_RESULT_NO_ERROR);
			setState(CTState::SUSPENDED);
		} break;
	
		case CONTROL_RELEASE: {
			CONTROLReleaseEvent		msg(event);
			LOG_DEBUG_STR("Received RELEASED(" << msg.cntlrName << ")");
			setState(CTState::RELEASE);
			/*
			if (!doRelease()) {
				LOG_WARN_STR("Cannot release a beam that was not allocated, continuing");
				setState(CTState::RELEASED);
				sendControlResult(*itsParentPort, CONTROL_RELEASED, getName(), 
																CT_RESULT_NO_ERROR);
				TRAN(TBBControl::claimed_state);
			}
			*/
			
		} break;
	
		case CONTROL_QUIT: {
			TRAN(TBBControl::quiting_state);
		} break;
	
		// -------------------- EVENTS RECEIVED FROM TBBDRIVER --------------------
	// TODO TBB
		case TBB_TRIGGER:{
			TBBTriggerEvent msg(event);
			
			TBBTrigger trigger(	static_cast<uint32>(msg.rcu),
													msg.sequence_nr,
													msg.time,
													msg.sample_nr,
													msg.trigger_sum,
													msg.trigger_samples,
													msg.peak_value,
													0);
			itsVHECRTask->addTrigger(trigger);
			
			LOG_TRACE_FLOW ("Sending trigger to PVSS");
			
			itsPropertySet->setValue(PN_TBC_TRIGGER_RCU_NR,	GCFPVInteger(msg.rcu),0.0,false);				
			itsPropertySet->setValue(PN_TBC_TRIGGER_SEQUENCE_NR,	GCFPVInteger(msg.sequence_nr),0.0,false);
			itsPropertySet->setValue(PN_TBC_TRIGGER_TIME,	GCFPVInteger(msg.time),0.0,false);
			itsPropertySet->setValue(PN_TBC_TRIGGER_SAMPLE_NR,	GCFPVInteger(msg.sample_nr),0.0,false);
			itsPropertySet->setValue(PN_TBC_TRIGGER_SUM,	GCFPVInteger(msg.trigger_sum),0.0,false);
			itsPropertySet->setValue(PN_TBC_TRIGGER_NR_SAMPLES,	GCFPVInteger(msg.trigger_samples),0.0,false);
			itsPropertySet->setValue(PN_TBC_TRIGGER_PEAK_VALUE,	GCFPVInteger(msg.peak_value),0.0,false);
			itsPropertySet->flush();
		} break;
		
		case TBB_STOP_ACK:{
			TBBStopAckEvent ack;
			bool status_ok = true;
			
			for (int i = 0; i < MAX_N_TBBBOARDS; i++) {
				if (ack.status_mask[i] != TBB_SUCCESS) {  // if error, check if rcu is used
					for (int i = (i * 16); i < ((i + 1) * 16); i++) {  // loop over rcu's on board[i]
						if (itsObs->allRCUset.test(i)) { status_ok = false;	} // check if rcu is selected
					}
				}
			}
			
			if (!status_ok) {
				LOG_ERROR_STR ("Failed to stop recording for selected rcus");
				itsPropertySet->setValue(PVSSNAME_FSM_ERROR,GCFPVString("stop error"));
			}
			
		} break;
	
		case TBB_READ_ACK: {
			TBBReadAckEvent ack;

		} break; 
	
		default: {
			status = _defaultEventHandler(event, port);
		}	break;
	}

	return (status);
}

//==============================================================================
// doTBBstop(event, port)
//
// send TBB_STOP cmd to the TBBDriver
//==============================================================================
GCFEvent::TResult TBBControl::doTBBstop(GCFEvent& event, GCFPortInterface& port)
{
	LOG_DEBUG_STR ("doTBBstop:" << eventName(event) << "@" << port.getName());

	GCFEvent::TResult status = GCFEvent::HANDLED;

	switch (event.signal) {
		case F_ENTRY: {
			TBBStopEvent cmd;
			
			vector<TBBReadCmd>::iterator it;
  		for ( it=itsCommandVector.begin() ; it < itsCommandVector.end(); it++ ) {
  			cmd.rcu_mask.set((*it).itsRcuNr);
  		}
						
			// info to pvss
			itsTBBDriver->send(cmd);
		} break;

		case TBB_STOP_ACK: {
			TBBStopAckEvent ack(event);
			
			TRAN(TBBControl::doTBBread);				// go to next state.
		} break;
	
		case F_DISCONNECTED: {
			_disconnectedHandler(port);
		} break;
					
		default: {
			status = _defaultEventHandler(event, port);
		} break;
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

	switch (event.signal) {
		case F_ENTRY: {
			TBBReadCmd read;
			TBBReadEvent cmd;
			if (!itsCommandVector.empty()) {
				read = itsCommandVector.back();
				cmd.rcu 				= read.itsRcuNr;              
			  cmd.secondstime	=	read.itsTime;      
			  cmd.sampletime	=	read.itsSampleTime;
			  cmd.prepages		= read.itsPrePages;  
			  cmd. postpages	=	read.itsPostPages; 
				itsTBBDriver->send(cmd);    
			}
			// info to pvss
			
		} break;

		case TBB_READ_ACK: {
			TBBReadAckEvent ack(event);
			TBBReadCmd read;
			TBBReadEvent cmd;
			
			itsCommandVector.pop_back();
			
			if (!itsCommandVector.empty()) {
				read = itsCommandVector.back();
				cmd.rcu 				= static_cast<int>(read.itsRcuNr);              
			  cmd.secondstime	=	read.itsTime;      
			  cmd.sampletime	=	read.itsSampleTime;
			  cmd.prepages		= read.itsPrePages;  
			  cmd. postpages	=	read.itsPostPages; 
				itsTBBDriver->send(cmd);    
			} else {
				TRAN(TBBControl::active_state);		// go back to active state.
			}
		} break;
	
		case F_DISCONNECTED: {
			_disconnectedHandler(port);
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
			itsTBBDriver->send(cmd);
		} break;

		case TBB_UNSUBSCRIBE_ACK: {
			TBBUnsubscribeAckEvent ack(event);
			
			TRAN(TBBControl::doTBBfree);				// go to next state.
		} break;
	
		case F_DISCONNECTED: {
			_disconnectedHandler(port);
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
			
			for (int i = 0; i < MAX_N_RCUS; i++) {
				cmd.rcu_mask.set(i,itsObs->allRCUset.test(i));
			} 
		
			// info to pvss
			itsTBBDriver->send(cmd);
		} break;

		case TBB_FREE_ACK: {
			TBBFreeAckEvent ack(event);
			
			TRAN(TBBControl::quiting_state);				// go to next state.
		} break;
	
		case F_DISCONNECTED: {
			_disconnectedHandler(port);
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
	
	//		itsPropertySet->setValue(string(PVSSNAME_FSM_CURACT),GCFPVString("quiting"));
			itsPropertySet->setValue(PVSSNAME_FSM_ERROR, GCFPVString(""));
			// disconnect from TBBDriver
			itsTBBDriver->close();
		} break;
	
		case F_INIT:
		case F_EXIT: {
		} break;
	
		case F_DISCONNECTED: 		// propably from beamserver
			port.close();
			// fall through!!! 
		case F_CLOSED: {
			ASSERTSTR (&port == itsTBBDriver, 
									"F_DISCONNECTED event from port " << port.getName());
			LOG_INFO("Connection with TBBDriver down, sending QUITED to parent");
			CONTROLQuitedEvent		request;
			request.cntlrName = getName();
			request.result	  = CT_RESULT_NO_ERROR;
			itsParentPort->send(request);
			itsTimerPort->setTimer(1.0);		// wait 1 second to let message go away
			
		} break;
		
		case F_TIMER: {
			GCFTask::stop();
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
	itsCommandVector.clear();
	itsCommandVector = readCmd;
	itsTimerPort->setTimer(0.0);
}

// _defaultEventHandler(event, port)
//
GCFEvent::TResult TBBControl::_defaultEventHandler(GCFEvent&			event, 
													GCFPortInterface&	port)
{
	CTState		cts;
	LOG_DEBUG_STR("Received " << eventName(event) << " in state " << cts.name(itsState)
				  << ". DEFAULT handling.");

	GCFEvent::TResult	result(GCFEvent::NOT_HANDLED);

	switch (event.signal) {
		case CONTROL_CONNECT:
		case CONTROL_RESYNC:
		case CONTROL_SCHEDULE:	// TODO: we should do something with this
		case CONTROL_CLAIM:
		case CONTROL_PREPARE:
		case CONTROL_RESUME:
		case CONTROL_SUSPEND:
		case CONTROL_RELEASE:
		case CONTROL_QUIT:
			 if (sendControlResult(port, event.signal, getName(), CT_RESULT_NO_ERROR)) {
				result = GCFEvent::HANDLED;
			}
			break;
		
		case CONTROL_CONNECTED:
		case CONTROL_RESYNCED:
		case CONTROL_SCHEDULED:
		case CONTROL_CLAIMED:
		case CONTROL_PREPARED:
		case CONTROL_RESUMED:
		case CONTROL_SUSPENDED:
		case CONTROL_RELEASED:
		case CONTROL_QUITED:
			result = GCFEvent::HANDLED;
			break;

		case DP_CHANGED: {
			LOG_DEBUG_STR("DP " << DPChangedEvent(event).DPname << " was changed"); 
			result = GCFEvent::HANDLED;
		}
		break;
	}

	if (result == GCFEvent::NOT_HANDLED) {
		LOG_WARN_STR("Event " << eventName(event) << " NOT handled in state " << 
					 cts.name(itsState));
	}

	return (result);
}

// _connectedHandler(port)
//
void TBBControl::_connectedHandler(GCFPortInterface& /*port*/)
{
}


//
// _disconnectedHandler(port)
//
void TBBControl::_disconnectedHandler(GCFPortInterface& port)
{
	port.close();
	if (&port == itsTBBDriver) {
		LOG_DEBUG_STR("Connection with TBBDriver lost, going to started state");
		itsPropertySet->setValue(PVSSNAME_FSM_ERROR,GCFPVString("connection lost"));
		TRAN (TBBControl::started_state);
	}
}


/*
case TBB_ALLOC_ACK: {

			int16 result = handleTBBAllocAck(event);

			if (result == CT_RESULT_NO_ERROR) {
				
				// Alloc OK, next try to send a TRIGSETUP event to the TBBDriver.
				if (!doPrepare(TBB_TRIGSETUP_STAGE)) { // could not sent it?
					LOG_WARN_STR("TBB trigsetup error: " << 
								 ", staying in CLAIMED mode");	// [TBB]
					setState(CTState::CLAIMED);
					LOG_DEBUG_STR("Sending PREPARED(" << getName() << "," << 
														CT_RESULT_CONFLICTING_ARGS << ")");
					sendControlResult(*itsParentPort, CONTROL_PREPARED, getName(), 
																	CT_RESULT_CONFLICTING_ARGS);
				} 
			} else {
				LOG_WARN_STR("TBB allocation failed with error " << result << 
							 ", staying in CLAIMED mode");
				setState(CTState::CLAIMED);
			}
		} break;

 // try to send a RSP TBBMODE event to the RSPDriver.
			if (!doPrepare(RSP_SET_TBBMODE_STAGE)) { // could not sent it?
				LOG_WARN_STR("RSP tbbmode error: " << 
							 ", staying in CLAIMED mode");	// [TBB]
				setState(CTState::CLAIMED);
				LOG_DEBUG_STR("Sending PREPARED(" << getName() << "," << 
													CT_RESULT_CONFLICTING_ARGS << ")");
				sendControlResult(*itsParentPort, CONTROL_PREPARED, getName(), 
																CT_RESULT_CONFLICTING_ARGS);
			}
			
*/
