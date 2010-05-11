//#  BeamControl.cc: Implementation of the MAC Scheduler task
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
#include <Common/LofarLogger.h>
#include <Common/StreamUtil.h>
#include <Common/Version.h>
#include <ApplCommon/Observation.h>
#include <ApplCommon/StationConfig.h>
#include <ApplCommon/StationInfo.h>

#include <Common/ParameterSet.h>
#include <Common/SystemUtil.h>
#include <GCF/PVSS/GCF_PVTypes.h>
#include <GCF/RTDB/DP_Protocol.ph>
#include <MACIO/MACServiceInfo.h>
#include <APL/APLCommon/APL_Defines.h>
#include <APL/APLCommon/Controller_Protocol.ph>
#include <APL/BS_Protocol/BS_Protocol.ph>
#include <APL/RTDBCommon/RTDButilities.h>
#include <signal.h>

#include "BeamControl.h"
#include "PVSSDatapointDefs.h"
#include <StationCU/Package__Version.h>

using namespace LOFAR::GCF::TM;
using namespace LOFAR::GCF::PVSS;
using namespace LOFAR::GCF::RTDB;
using namespace LOFAR::APL::RTDBCommon;
using namespace std;

namespace LOFAR {
	using namespace APLCommon;
	namespace StationCU {
	
// static pointer to this object for signal handler
static BeamControl*	thisBeamControl = 0;
static uint16		gResult 		= CT_RESULT_NO_ERROR;

//
// BeamControl()
//
BeamControl::BeamControl(const string&	cntlrName) :
	GCFTask 			((State)&BeamControl::initial_state,cntlrName),
	itsPropertySet		(0),
	itsPropertySetInitialized (false),
	itsParentControl	(0),
	itsParentPort		(0),
	itsTimerPort		(0),
	itsBeamServer		(0),
	itsState			(CTState::NOSTATE),
	itsNrBeams			(0)
{
	LOG_TRACE_OBJ_STR (cntlrName << " construction");
	LOG_INFO(Version::getInfo<StationCUVersion>("BeamControl"));

	// First readin our observation related config file.
	LOG_DEBUG_STR("Reading parset file:" << LOFAR_SHARE_LOCATION << "/" << cntlrName);
	globalParameterSet()->adoptFile(string(LOFAR_SHARE_LOCATION)+"/"+cntlrName);

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

	// prepare TCP port to BeamServer.
	itsBeamServer = new GCFTCPPort (*this, MAC_SVCMASK_BEAMSERVER,
											GCFPortInterface::SAP, BS_PROTOCOL);
	ASSERTSTR(itsBeamServer, "Cannot allocate TCPport to BeamServer");

	// for debugging purposes
	registerProtocol (CONTROLLER_PROTOCOL, CONTROLLER_PROTOCOL_STRINGS);
	registerProtocol (DP_PROTOCOL, 		DP_PROTOCOL_STRINGS);
	registerProtocol (BS_PROTOCOL, 		BS_PROTOCOL_STRINGS);

	setState(CTState::CREATED);
}


//
// ~BeamControl()
//
BeamControl::~BeamControl()
{
	LOG_TRACE_OBJ_STR (getName() << " destruction");

}

//
// sigintHandler(signum)
//
void BeamControl::sigintHandler(int signum)
{
	LOG_DEBUG (formatString("SIGINT signal detected (%d)",signum));

	if (thisBeamControl) {
		thisBeamControl->finish();
	}
}

//
// finish
//
void BeamControl::finish()
{
	TRAN(BeamControl::quiting_state);
}


//
// setState(CTstateNr)
//
void    BeamControl::setState(CTState::CTstateNr     newState)
{
	itsState = newState;

	if (itsPropertySet) {
		CTState		cts;
		itsPropertySet->setValue(PN_FSM_CURRENT_ACTION, GCFPVString(cts.name(newState)));
	}
}   


//
// convertDirection(string) : int32
//
int32 BeamControl::convertDirection(const string&	typeName)
{
  LOG_INFO_STR ("Receiving DirectionType: " << typeName );	
  if (typeName == "J2000") 	{ return (1); }
  if (typeName == "AZEL") 	{ return (2); }
  if (typeName == "LMN")	{ return (3); }
  LOG_WARN_STR ("Unknown DirectionType : " << typeName << " Will use J2000");
  return (1);
}

//
// initial_state(event, port)
//
// Connect to PVSS and report state back to startdaemon
//
GCFEvent::TResult BeamControl::initial_state(GCFEvent& event, 
													GCFPortInterface& port)
{
	LOG_DEBUG_STR ("initial:" << eventName(event) << "@" << port.getName());

	GCFEvent::TResult status = GCFEvent::HANDLED;
  
	switch (event.signal) {
	case F_ENTRY:
   		break;

    case F_INIT: {
		// Get access to my own propertyset.
		string	propSetName(createPropertySetName(PSN_BEAM_CONTROL, getName(), 
												  globalParameterSet()->getString("_DPname")));
		LOG_INFO_STR ("Activating PropertySet" << propSetName);
		itsPropertySet = new RTDBPropertySet(propSetName,
											 PST_BEAM_CONTROL,
											 PSAT_RW,
											 this);
		// Wait for timer that is set on DP_CREATED event

		// Instruct loggingProcessor
//		LOG_INFO_STR("MACProcessScope: LOFAR.ObsSW.Observation" << treeID << ".BeamControl");
		LOG_INFO_STR("MACProcessScope: " << propSetName);
		// NOTE: the SASgateway is not yet aware of claimMgr so the data will not be transferred to SAS.
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
	  
	case F_TIMER:
		if (!itsPropertySetInitialized) {
			itsPropertySetInitialized = true;

			// first redirect signalHandler to our quiting state to leave PVSS
			// in the right state when we are going down
			thisBeamControl = this;
			signal (SIGINT,  BeamControl::sigintHandler);	// ctrl-c
			signal (SIGTERM, BeamControl::sigintHandler);	// kill

			// update PVSS.
			LOG_TRACE_FLOW ("Updateing state to PVSS");
			GCFPValueArray		dpeValues;
			itsPropertySet->setValue(PN_FSM_CURRENT_ACTION,	GCFPVString("initial"));
			itsPropertySet->setValue(PN_FSM_ERROR,	GCFPVString(""));
			itsPropertySet->setValue(PN_BC_CONNECTED,		GCFPVBool  (false));
			itsPropertySet->setValue(PN_BC_SUB_ARRAY,		GCFPVString(""));
			itsPropertySet->setValue(PN_BC_SUBBAND_LIST,	GCFPVDynArr(LPT_DYNSTRING, dpeValues));
			itsPropertySet->setValue(PN_BC_BEAMLET_LIST,	GCFPVDynArr(LPT_DYNSTRING, dpeValues));
			itsPropertySet->setValue(PN_BC_ANGLE1,			GCFPVDynArr(LPT_DYNDOUBLE, dpeValues));
			itsPropertySet->setValue(PN_BC_ANGLE2,			GCFPVDynArr(LPT_DYNDOUBLE, dpeValues));
//			itsPropertySet->setValue(PN_BC_ANGLETIMES,		GCFPVDynArr(LPT_DYNUNSIGNED, dpeValues));
			itsPropertySet->setValue(PN_BC_DIRECTION_TYPE,	GCFPVDynArr(LPT_DYNSTRING, dpeValues));
			itsPropertySet->setValue(PN_BC_BEAM_NAME,		GCFPVDynArr(LPT_DYNSTRING, dpeValues));
		  
			// Start ParentControl task
			LOG_DEBUG ("Enabling ParentControl task and wait for my name");
			itsParentPort = itsParentControl->registerTask(this);
			// results in CONTROL_CONNECT
		}
		break;

	case F_CONNECTED:
		ASSERTSTR (&port == itsParentPort, 
									"F_CONNECTED event from port " << port.getName());
		break;

	case F_DISCONNECTED:
	case F_EXIT:
		break;
	
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

		LOG_INFO ("Killing running beamctl's if any");
		system ("killall beamctl");

		LOG_INFO ("Going to started state");
		TRAN(BeamControl::started_state);				// go to next state.
		break;
	}

	default:
		status = _defaultEventHandler(event, port);
		break;
	}

	return (status);
}


//
// started_state(event, port)
//
// wait for CLAIM event
//
GCFEvent::TResult BeamControl::started_state(GCFEvent& event, GCFPortInterface& port)
{
	LOG_DEBUG_STR ("started:" << eventName(event) << "@" << port.getName());

	GCFEvent::TResult status = GCFEvent::HANDLED;

	switch (event.signal) {
	case F_ENTRY: {
		// update PVSS
//		itsPropertySet->setValue(string(PN_FSM_CURRENT_ACTION),GCFPVString("started"));
		itsPropertySet->setValue(PN_FSM_ERROR,GCFPVString(""));
		itsPropertySet->setValue(PN_BC_CONNECTED,	GCFPVBool  (false));
		break;
	}

	case F_INIT:
	case F_EXIT:
		break;

	case F_CONNECTED: {
		ASSERTSTR (&port == itsBeamServer, "F_CONNECTED event from port " 
																	<< port.getName());
		itsTimerPort->cancelAllTimers();
		LOG_INFO ("Connected with BeamServer, going to claimed state");
		itsPropertySet->setValue(PN_BC_CONNECTED,	GCFPVBool(true));
		setState(CTState::CLAIMED);
		sendControlResult(*itsParentPort, CONTROL_CLAIMED, getName(), CT_RESULT_NO_ERROR);
		TRAN(BeamControl::claimed_state);				// go to next state.
		break;
	}

	case F_DISCONNECTED: {
		port.close();
		ASSERTSTR (&port == itsBeamServer, 
								"F_DISCONNECTED event from port " << port.getName());
		LOG_WARN ("Connection with BeamServer failed, retry in 2 seconds");
		itsTimerPort->setTimer(2.0);
		break;
	}

	case F_TIMER: 
//		GCFTimerEvent& timerEvent=static_cast<GCFTimerEvent&>(event);
		LOG_DEBUG ("Trying to (re)connect to BeamServer");
		itsBeamServer->open();		// will result in F_CONN or F_DISCONN
		break;

	// -------------------- EVENTS RECEIVED FROM PARENT CONTROL --------------------
	case CONTROL_CLAIM: {
		CONTROLClaimEvent		msg(event);
		LOG_INFO_STR("Received CLAIM(" << msg.cntlrName << "), connecting to BeamServer in 5 seconds");
		setState(CTState::CLAIM);
		// wait several seconds before connecting to the BeamServer. When the state of the splitters are changed
		// the Beamserver will through us out anyway. So don't frustate the BeamServer with needless connections.
		itsTimerPort->setTimer(5.0);
		break;
	}

	case CONTROL_QUIT:
		TRAN(BeamControl::quiting_state);
		break;

	default:
		status = _defaultEventHandler(event, port);
		break;
	}

	return (status);
}

//
// claimed_state(event, port)
//
// wait for PREPARE event.
//
GCFEvent::TResult BeamControl::claimed_state(GCFEvent& event, GCFPortInterface& port)
{
	LOG_DEBUG_STR ("claimed:" << eventName(event) << "@" << port.getName());

	GCFEvent::TResult status = GCFEvent::HANDLED;

	switch (event.signal) {
	case F_ENTRY: {
		// update PVSS
//		itsPropertySet->setValue(string(PN_FSM_CURRENT_ACTION),GCFPVString("claimed"));
		itsPropertySet->setValue(PN_FSM_ERROR, GCFPVString(""));
		break;
	}

	case F_INIT:
	case F_EXIT:
		break;

	case F_DISCONNECTED: {
		port.close();
		ASSERTSTR (&port == itsBeamServer, 
								"F_DISCONNECTED event from port " << port.getName());
		LOG_WARN("Connection with BeamServer lost, may be due to splitter change, going to reconnect.");
		setObjectState("Connection with BeamServer lost!", itsPropertySet->getFullScope(), RTDB_OBJ_STATE_BROKEN);
		itsTimerPort->setTimer(2.0);	// start timer for reconnect and switch state.
		TRAN(BeamControl::started_state);
		break;
	}

	// -------------------- EVENTS RECEIVED FROM PARENT CONTROL --------------------
	case CONTROL_PREPARE: {
		CONTROLPrepareEvent		msg(event);
		LOG_INFO_STR("Received PREPARE(" << msg.cntlrName << ")");
		setState(CTState::PREPARE);
// TODO
		gResult = CT_RESULT_NO_ERROR;
		// try to send a BEAMALLOC event to the BeamServer.
		if (!doPrepare()) { // could not sent it?
			LOG_WARN_STR("Beamallocation error: nr subbands != nr beamlets" << 
						 ", staying in CLAIMED mode");
			setState(CTState::CLAIMED);
			setObjectState("Number of subbands != number of beamlets", itsPropertySet->getFullScope(), RTDB_OBJ_STATE_BROKEN);
			LOG_INFO_STR("Sending PREPARED(" << getName() << "," << 
												CT_RESULT_CONFLICTING_ARGS << ")");
			sendControlResult(*itsParentPort, CONTROL_PREPARED, getName(), 
															CT_RESULT_CONFLICTING_ARGS);
		}
		// else one or more BS_BEAMALLOCACK events will happen.
		break;
	}

	case CONTROL_QUIT:
		TRAN(BeamControl::quiting_state);
		break;

	// -------------------- EVENTS RECEIVED FROM BEAMSERVER --------------------
	case BS_BEAMALLOCACK: {
		BSBeamallocackEvent		msg(event);
		gResult |= handleBeamAllocAck(event);
		if (itsBeamIDs.size() == itsNrBeams) {	// answer on all beams received?
			if (gResult == CT_RESULT_NO_ERROR) {
				setState(CTState::PREPARED);
				LOG_INFO("Beam allocated, going to active state");
				TRAN(BeamControl::active_state);
			}
			else {
				LOG_WARN_STR("Beamallocation failed with error " << errorName(gResult) << 
							 ", staying in CLAIMED mode");
				setState(CTState::CLAIMED);
			}
			LOG_INFO_STR("Sending PREPARED(" << getName() << "," << gResult << ") event");
			sendControlResult(*itsParentPort, CONTROL_PREPARED, getName(), gResult);
		}
		else {
			LOG_DEBUG_STR("Still waiting for " << itsNrBeams - itsBeamIDs.size() << " alloc answers");
		}
		break;
	}

	default:
		status = _defaultEventHandler(event, port);
		break;
	}

	return (status);
}

//
// active_state(event, port)
//
// Normal operation state. 
//
GCFEvent::TResult BeamControl::active_state(GCFEvent& event, GCFPortInterface& port)
{
	LOG_DEBUG_STR ("active:" << eventName(event) << "@" << port.getName());

	GCFEvent::TResult status = GCFEvent::HANDLED;

	switch (event.signal) {
	case F_ENTRY: {
		// update PVSS
//		itsPropertySet->setValue(string(PN_FSM_CURRENT_ACTION),GCFPVString("active"));
		itsPropertySet->setValue(PN_FSM_ERROR, GCFPVString(""));
		break;
	}

	case F_INIT:
	case F_EXIT:
		break;

	case F_DISCONNECTED: {
		port.close();
		ASSERTSTR (&port == itsBeamServer, 
								"F_DISCONNECTED event from port " << port.getName());
		LOG_WARN("Connection with BeamServer lost");
		setObjectState("Connection with BeamServer lost!", itsPropertySet->getFullScope(), RTDB_OBJ_STATE_BROKEN);
		finish();
//		TRAN(BeamControl::started_state);
		break;
	}
	
	// -------------------- EVENTS RECEIVED FROM PARENT CONTROL --------------------

	case CONTROL_SCHEDULE: {
		CONTROLScheduledEvent		msg(event);
		LOG_INFO_STR("Received SCHEDULE(" << msg.cntlrName << ")");
		// TODO: do something usefull with this information!
		break;
	}

	case CONTROL_RESUME: {
		CONTROLResumeEvent		msg(event);
		LOG_INFO_STR("Received RESUME(" << msg.cntlrName << ")");
		setState(CTState::RESUME);
		sendControlResult(port, CONTROL_RESUMED, msg.cntlrName, CT_RESULT_NO_ERROR);
		setState(CTState::RESUMED);
		break;
	}

	case CONTROL_SUSPEND: {
		CONTROLSuspendEvent		msg(event);
		LOG_INFO_STR("Received SUSPEND(" << msg.cntlrName << ")");
		setState(CTState::SUSPEND);
		sendControlResult(port, CONTROL_SUSPENDED, msg.cntlrName, CT_RESULT_NO_ERROR);
		setState(CTState::SUSPENDED);
		break;
	}

	case CONTROL_RELEASE: {
		CONTROLReleaseEvent		msg(event);
		LOG_INFO_STR("Received RELEASED(" << msg.cntlrName << ")");
		setState(CTState::RELEASE);
		if (!doRelease()) {
			LOG_WARN_STR("Cannot release a beam that was not allocated, continuing");
			setState(CTState::RELEASED);
			sendControlResult(*itsParentPort, CONTROL_RELEASED, getName(), 
															CT_RESULT_NO_ERROR);
			TRAN(BeamControl::claimed_state);
		}
		// else a BS_BEAMFREEACK event will be sent
		break;
	}

	case CONTROL_QUIT:
		TRAN(BeamControl::quiting_state);
		break;

	// -------------------- EVENTS RECEIVED FROM BEAMSERVER --------------------
	case BS_BEAMFREEACK: {
		if (!handleBeamFreeAck(event)) {
			LOG_WARN("Error in freeing beam, trusting on disconnect.");
		}
		if (itsBeamIDs.empty()) {	// answer on all beams received?
			LOG_INFO("Released beam(s) going back to 'claimed' mode");
			setState(CTState::RELEASED);
			sendControlResult(*itsParentPort, CONTROL_RELEASED, getName(), 
															CT_RESULT_NO_ERROR);
			TRAN(BeamControl::claimed_state);
		}
	}
	break;

	default:
		status = _defaultEventHandler(event, port);
		break;
	}

	return (status);
}

//
// quiting_state(event, port)
//
// Quiting: send QUITED, wait for answer max 5 seconds, stop
//
GCFEvent::TResult BeamControl::quiting_state(GCFEvent& event, GCFPortInterface& port)
{
	LOG_DEBUG_STR ("quiting:" << eventName(event) << "@" << port.getName());

	GCFEvent::TResult status = GCFEvent::HANDLED;

	switch (event.signal) {
	case F_ENTRY: {
		// update PVSS
		setState(CTState::QUIT);
		// tell Parent task we like to go down.
		itsParentControl->nowInState(getName(), CTState::QUIT);

//		itsPropertySet->setValue(string(PN_FSM_CURRENT_ACTION),GCFPVString("quiting"));
		itsPropertySet->setValue(PN_FSM_ERROR, GCFPVString(""));
		// disconnect from BeamServer
		itsBeamServer->close();

		LOG_INFO("Connection with BeamServer closed, sending QUITED to parent");
		CONTROLQuitedEvent		request;
		request.cntlrName = getName();
		request.result	  = CT_RESULT_NO_ERROR;
		itsParentPort->send(request);
		itsTimerPort->setTimer(1.0);		// wait 1 second to let message go away
		break;
	}
	
	case F_TIMER:
		GCFScheduler::instance()->stop();
		break;

	default:
		status = _defaultEventHandler(event, port);
		break;
	}

	return (status);
}


//
// doPrepare()
//
// Send an allocation event for every beam to the beamserver
//
bool BeamControl::doPrepare()
{
	Observation		theObs(globalParameterSet());	// does all nasty conversions

	GCFPValueArray		angle1Arr;
	GCFPValueArray		angle2Arr;
	GCFPValueArray		dirTypesArr;
//	GCFPValueArray		angleTimesArr;
	GCFPValueArray		subbandArr;
	GCFPValueArray		beamletArr;
	GCFPValueArray		beamIDArr;

	LOG_DEBUG_STR(theObs);
	bool		stereoBeams((theObs.antennaSet == "HBA_DUAL") && (stationRingName() == "Core"));
	itsNrBeams = theObs.beams.size() * (stereoBeams ? 2 : 1);
	LOG_DEBUG_STR("Controlling " << itsNrBeams << " beams.");

	for (uint32	i(0); i < theObs.beams.size(); i++) {
		if (theObs.beams[i].subbands.size() != theObs.beams[i].beamlets.size()) {
			LOG_FATAL_STR("size of subbandList (" << theObs.beams[i].subbands.size() << ") != " <<
							"size of beamletList (" << theObs.beams[i].beamlets.size() << ")");
			return (false);
		}

		LOG_TRACE_VAR_STR("nr Subbands:" << theObs.beams[i].subbands.size());
		LOG_TRACE_VAR_STR("nr Beamlets:" << theObs.beams[i].beamlets.size());

		// contruct allocation event.
		BSBeamallocEvent beamAllocEvent;
		// TODO: As long as the AntennaArray.conf uses different names as SAS we have to use this dirty hack to 
		//       get the name of the antennaField.
		beamAllocEvent.name 		= theObs.getAntennaArrayName(stereoBeams);
		beamAllocEvent.subarrayname = theObs.getBeamName(i);
		LOG_DEBUG_STR("subarray@field : " << beamAllocEvent.subarrayname << "@" << beamAllocEvent.name);
		beamAllocEvent.rcumask = theObs.getRCUbitset(0, 0, 0, false);		// get modified set of StationController
		StationConfig	sc;
		beamAllocEvent.ringNr  = ((sc.hasSplitters && (theObs.antennaSet == "HBA_ONE")) ? 1 : 0);

		// construct subband to beamlet map
		vector<int32>::iterator beamletIt = theObs.beams[i].beamlets.begin();
		vector<int32>::iterator subbandIt = theObs.beams[i].subbands.begin();
		while (beamletIt != theObs.beams[i].beamlets.end() && subbandIt != theObs.beams[i].subbands.end()) {
			LOG_TRACE_VAR_STR("alloc[" << *beamletIt << "]=" << *subbandIt);
			beamAllocEvent.allocation()[*beamletIt++] = *subbandIt++;
		}

		// Note: when HBA_DUAL is selected we should set up a beam on both HBA_0 and HBA_1 field.
		if (!stereoBeams) {
			LOG_DEBUG_STR("Sending Alloc event to BeamServer");
			itsBeamServer->send(beamAllocEvent);		// will result in BS_BEAMALLOCACK;
		}
		else {
			for (int rcu = sc.nrHBAs; rcu < sc.nrHBAs*2; rcu++) {	// clear second half of RCUs
				beamAllocEvent.rcumask.reset(rcu);
			}
			beamAllocEvent.name = "HBA_0";
			beamAllocEvent.subarrayname += "_0";
			LOG_DEBUG_STR("Sending Alloc event to BeamServer for ring 0");
			itsBeamServer->send(beamAllocEvent);		// will result in BS_BEAMALLOCACK;

			beamAllocEvent.rcumask = theObs.getRCUbitset(0, 0, 0, false);		// get modified set of StationController
			beamAllocEvent.ringNr  = 1;
			beamAllocEvent.name    = "HBA_1";
			beamAllocEvent.subarrayname = theObs.getBeamName(i) + "_1";
			for (int rcu = 0; rcu < sc.nrHBAs; rcu++) {	// clear first half of RCUs
				beamAllocEvent.rcumask.reset(rcu);
			}
			LOG_DEBUG_STR("Sending Alloc event to BeamServer for ring 1");
			itsBeamServer->send(beamAllocEvent);		// will result in BS_BEAMALLOCACK;
		}

		// store values in PVSS for operator
		stringstream		os;
		writeVector(os, theObs.beams[i].subbands);
		subbandArr.push_back   (new GCFPVString  (os.str()));
		os.clear();
		writeVector(os, theObs.beams[i].beamlets);
		beamletArr.push_back   (new GCFPVString  (os.str()));
		angle1Arr.push_back	   (new GCFPVDouble  (theObs.beams[i].angle1));
		angle2Arr.push_back	   (new GCFPVDouble  (theObs.beams[i].angle2));
		dirTypesArr.push_back  (new GCFPVString  (theObs.beams[i].directionType));
//		angleTimesArr.push_back(new GCFPVUnsigned(theObs.beams[i].angleTimes));
		beamIDArr.push_back	   (new GCFPVString  (""));
	}

	itsPropertySet->setValue(PN_BC_SUBBAND_LIST,	GCFPVDynArr(LPT_DYNSTRING, subbandArr));
	itsPropertySet->setValue(PN_BC_BEAMLET_LIST,	GCFPVDynArr(LPT_DYNSTRING, beamletArr));
	itsPropertySet->setValue(PN_BC_ANGLE1,			GCFPVDynArr(LPT_DYNDOUBLE, angle1Arr));
	itsPropertySet->setValue(PN_BC_ANGLE2,			GCFPVDynArr(LPT_DYNDOUBLE, angle2Arr));
//	itsPropertySet->setValue(PN_BC_ANGLETIMES,		GCFPVDynArr(LPT_DYNUINT,   angleTimesArr));
	itsPropertySet->setValue(PN_BC_DIRECTION_TYPE,	GCFPVDynArr(LPT_DYNSTRING, dirTypesArr));

	return (true);
}

//
// handleBeamAllocAck(event);
//
// When the allocation was succesfull send all pointing of that beam.
//
uint16	BeamControl::handleBeamAllocAck(GCFEvent&	event)
{
	// check the beam ID and status of the ACK message
	BSBeamallocackEvent ackEvent(event);
	if (ackEvent.status != BS_NO_ERR) {
		LOG_ERROR_STR("Beamlet allocation for beam " << ackEvent.subarrayname 
					  << " failed with errorcode: " << errorName(ackEvent.status));
		itsBeamIDs[ackEvent.subarrayname] = 0;
		setObjectState("Beamlet alloc error", itsPropertySet->getFullScope(), RTDB_OBJ_STATE_BROKEN);
		return (CT_RESULT_BEAMALLOC_FAILED);
	}
	itsBeamIDs[ackEvent.subarrayname] = ackEvent.handle;
	LOG_INFO_STR("Beam " << ackEvent.subarrayname << " allocated succesful, sending pointings");

	// read new angles from parameterfile.
	Observation		theObs(globalParameterSet());	// does all nasty conversions
	uint32			beamIdx(indexValue(ackEvent.subarrayname, "[]"));
	if (beamIdx >= theObs.beams.size()) {
		LOG_FATAL_STR("Beamnr " << beamIdx << " (=beam " << ackEvent.subarrayname << 
						") is out of range: 0.." << theObs.beams.size()-1);
		setObjectState("Beamlet alloc index error", itsPropertySet->getFullScope(), RTDB_OBJ_STATE_BROKEN);
		return (CT_RESULT_BEAMALLOC_FAILED);
	}
	Observation::Beam*	theBeam = &theObs.beams[beamIdx];

	// point the new beam
	// NOTE: for the time being we don't support multiple pointings for a beam: no other sw supports it.
	BSBeampointtoEvent beamPointToEvent;
	beamPointToEvent.handle = ackEvent.handle;
	beamPointToEvent.pointing.setType(static_cast<Pointing::Type> (convertDirection(theBeam->directionType)));

	beamPointToEvent.pointing.setTime(RTC::Timestamp()); // asap
	beamPointToEvent.pointing.setDirection(theBeam->angle1,theBeam->angle2);
	itsBeamServer->send(beamPointToEvent);
	return (CT_RESULT_NO_ERROR);
}

//
// doRelease(event)
//
bool BeamControl::doRelease()
{
	if (itsBeamIDs.empty()) {
		LOG_DEBUG ("doRelease: beam-map is empty");
		return (false);
	}

	BSBeamfreeEvent		beamFreeEvent;
	map<string, void*>::iterator	iter = itsBeamIDs.begin();
	map<string, void*>::iterator	end  = itsBeamIDs.end();
	while (iter != end) {
		beamFreeEvent.handle = iter->second;
		itsBeamServer->send(beamFreeEvent);	// will result in BS_BEAMFREEACK event
		iter++;
	}
	return (true);
}


//
// handleBeamFreeAck(event)
//
bool BeamControl::handleBeamFreeAck(GCFEvent&		event)
{
	BSBeamfreeackEvent	ack(event);
	if (ack.status != BS_NO_ERR) {
		LOG_ERROR_STR("Beam de-allocation failed with errorcode: " << errorName(ack.status));
		itsPropertySet->setValue(PN_FSM_ERROR,GCFPVString("De-allocation of the beam failed."));
		return (false);	
	}

	map<string, void*>::iterator	iter = itsBeamIDs.begin();
	map<string, void*>::iterator	end  = itsBeamIDs.end();
	while(iter != end) {
		if (iter->second == ack.handle) {
			// clear beam in PVSS
#if 0
			// TODO: those fields are dynArrays.
			itsPropertySet->setValue(PN_BC_SUBBAND_LIST,	GCFPVString(""));
			itsPropertySet->setValue(PN_BC_BEAMLET_LIST,	GCFPVString(""));
			itsPropertySet->setValue(PN_BC_ANGLE1,			GCFPVString(""));
			itsPropertySet->setValue(PN_BC_ANGLE2,			GCFPVString(""));
		//	itsPropertySet->setValue(PN_BC_ANGLETIMES,		GCFPVString(""));
#endif
			LOG_INFO_STR("De-allocation of beam " << iter->first << " succesful");
			itsBeamIDs.erase(iter);
			return (true);
		}
		iter++;
	}
	return (false);
}

// _defaultEventHandler(event, port)
//
GCFEvent::TResult BeamControl::_defaultEventHandler(GCFEvent&			event, 
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

		case DP_SET:
		break;
	}

	if (result == GCFEvent::NOT_HANDLED) {
		LOG_DEBUG_STR("Event " << eventName(event) << " NOT handled in state " << 
					 cts.name(itsState));
	}

	return (result);
}

// _connectedHandler(port)
//
void BeamControl::_connectedHandler(GCFPortInterface& /*port*/)
{
}


//
// _disconnectedHandler(port)
//
void BeamControl::_disconnectedHandler(GCFPortInterface& port)
{
	port.close();
}


}; // StationCU
}; // LOFAR
