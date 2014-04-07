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
#include <ApplCommon/LofarDirs.h>
#include <ApplCommon/Observation.h>
#include <ApplCommon/StationConfig.h>
#include <ApplCommon/StationInfo.h>

#include <Common/ParameterSet.h>
#include <Common/SystemUtil.h>
#include <ApplCommon/AntennaSets.h>
#include <GCF/PVSS/GCF_PVTypes.h>
#include <GCF/RTDB/DP_Protocol.ph>
#include <MACIO/MACServiceInfo.h>
#include <APL/APLCommon/APL_Defines.h>
#include <APL/APLCommon/Controller_Protocol.ph>
#include <APL/IBS_Protocol/IBS_Protocol.ph>
#include <APL/RTDBCommon/RTDButilities.h>
#include <signal.h>

#include "BeamControl.h"
#include "PVSSDatapointDefs.h"
#include <StationCU/Package__Version.h>

#include <boost/date_time/posix_time/posix_time.hpp>

using namespace LOFAR::GCF::TM;
using namespace LOFAR::GCF::PVSS;
using namespace LOFAR::GCF::RTDB;
using namespace LOFAR::APL::RTDBCommon;
using namespace boost::posix_time;
using namespace std;

namespace LOFAR {
	using namespace APLCommon;
	namespace StationCU {

#define MAX2(a,b)	((a)>(b)?(a):(b))
	
// static pointer to this object for signal handler
static BeamControl*	thisBeamControl = 0;

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

	// need port for timers.
	itsTimerPort = new GCFTimerPort(*this, "TimerPort");

	// prepare TCP port to BeamServer.
	itsBeamServer = new GCFTCPPort (*this, MAC_SVCMASK_BEAMSERVER,
											GCFPortInterface::SAP, IBS_PROTOCOL);
	ASSERTSTR(itsBeamServer, "Cannot allocate TCPport to BeamServer");

	StationConfig	sc;
	itsObs = new Observation(globalParameterSet(), sc.hasSplitters);
	ASSERTSTR(itsObs, "Could not convert the parameter set into a legal observation definition");

	// for debugging purposes
	registerProtocol (CONTROLLER_PROTOCOL, CONTROLLER_PROTOCOL_STRINGS);
	registerProtocol (DP_PROTOCOL, 		DP_PROTOCOL_STRINGS);
	registerProtocol (IBS_PROTOCOL, 		IBS_PROTOCOL_STRINGS);

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
		ASSERTSTR (&port == itsParentPort, "F_CONNECTED event from port " << port.getName());
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
		int retval = system ("killall beamctl");
		(void)retval;

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
		TRAN(BeamControl::allocBeams_state);
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
// convertFilterSelection(string) : uint8
// NOTE: COPIED FROM CalibrationControl !!!!!!!
//
int32 BeamControl::convertFilterSelection(const string&	filterselection, const string&	antennaSet)  const
{
	// support new filternames
	if (antennaSet == "LBA_OUTER") {
		if (filterselection == "LBA_10_70")	{ return(1); }	// 160 Mhz
		if (filterselection == "LBA_10_90")	{ return(1); }	// 200 Mhz
		if (filterselection == "LBA_30_70")	{ return(2); }	// 160 Mhz
		if (filterselection == "LBA_30_90")	{ return(2); }	// 200 Mhz
	}
	if (antennaSet == "LBA_INNER") {
		if (filterselection == "LBA_10_70")	{ return(3); }	// 160 Mhz
		if (filterselection == "LBA_10_90")	{ return(3); }	// 200 Mhz
		if (filterselection == "LBA_30_70")	{ return(4); }	// 160 Mhz
		if (filterselection == "LBA_30_90")	{ return(4); }	// 200 Mhz
	}
	if (filterselection == "HBA_110_190")	{ return(5); }	// 200 Mhz
	if (filterselection == "HBA_170_230")	{ return(6); }	// 160 Mhz
	if (filterselection == "HBA_210_250")	{ return(7); }	// 200 Mhz

	LOG_WARN_STR ("filterselection value '" << filterselection << 
									"' not recognized, using LBA_10_70");
	return (1);
}


//
// allocBeams_state(event, port)
//
// Substate during the PREPARE fase.
//
GCFEvent::TResult BeamControl::allocBeams_state(GCFEvent& event, GCFPortInterface& port)
{
	static bool		reachedEnd(false);
	static bool		allocatingDigitalBeams(itsObs->beams.size() > 0);
	static uint		beamIdx(0);
//	static string	curBeamName(allocatingDigitalBeams ? itsObs->beams[beamIdx].name : itsObs->anaBeams[beamIdx].name);
	static string	curBeamName(itsObs->beams[beamIdx].name);

	LOG_DEBUG_STR("allocBeams:" << eventName(event) << "@" << port.getName());

	//
	// Create a new subarray
	//
	switch (event.signal) {
	case F_ENTRY: 
		itsTimerPort->cancelAllTimers();
		itsTimerPort->setTimer(0.01);		// give CalControl + CalServer some time to allocated the beams.
		break;

	case F_TIMER: {
		if (!allocatingDigitalBeams) {
			LOG_DEBUG_STR("NO DIGITAL BEAMS DECLARED, skipping allocation, moving forwards to sending Pointings");
			TRAN(BeamControl::sendPointings_state);
			return (GCFEvent::HANDLED);
		}

		// contruct allocation event.
		IBSBeamallocEvent beamAllocEvent;
		beamAllocEvent.beamName   = curBeamName;
		beamAllocEvent.antennaSet = allocatingDigitalBeams ? itsObs->beams[beamIdx].antennaSet : itsObs->anaBeams[beamIdx].antennaSet;
		LOG_DEBUG_STR("beam@antennaSet : " << beamAllocEvent.beamName << "@" << beamAllocEvent.antennaSet);
		beamAllocEvent.rcumask = itsObs->getRCUbitset(0, 0, beamAllocEvent.antennaSet) & 
								 globalAntennaSets()->RCUallocation(beamAllocEvent.antennaSet);	
		beamAllocEvent.ringNr = 0;
		beamAllocEvent.rcuMode = convertFilterSelection(itsObs->filter, beamAllocEvent.antennaSet);

		// digital part
		if (!itsObs->beams.empty()) {			// fill digital part if any
			StationConfig		sc;
			beamAllocEvent.ringNr = ((sc.hasSplitters && (beamAllocEvent.antennaSet.substr(0,7) == "HBA_ONE")) ? 1 : 0);

			vector<int> beamBeamlets = itsObs->getBeamlets(beamIdx);
			if (itsObs->beams[beamIdx].subbands.size() != beamBeamlets.size()) {
				LOG_FATAL_STR("size of subbandList (" << itsObs->beams[beamIdx].subbands.size() << ") != " <<
								"size of beamletList (" << beamBeamlets.size() << ")");
				setState(CTState::CLAIMED);
				sendControlResult(*itsParentPort, CONTROL_PREPARED, getName(), CT_RESULT_BEAMALLOC_FAILED);
				TRAN(BeamControl::claimed_state);
				return (GCFEvent::HANDLED);
			}
			LOG_DEBUG_STR("nr Subbands:" << itsObs->beams[beamIdx].subbands.size());
			LOG_DEBUG_STR("nr Beamlets:" << beamBeamlets.size());

			// construct subband to beamlet map
			vector<int32>::iterator beamletIt = beamBeamlets.begin();
			vector<int32>::iterator subbandIt = itsObs->beams[beamIdx].subbands.begin();
			while (beamletIt != beamBeamlets.end() && subbandIt != itsObs->beams[beamIdx].subbands.end()) {
				LOG_DEBUG_STR("alloc[" << *beamletIt << "]=" << *subbandIt);
				beamAllocEvent.allocation()[*beamletIt++] = *subbandIt++;
			}
		}
		LOG_INFO_STR("Sending Alloc event to BeamServer for beam: " << curBeamName);
		LOG_DEBUG_STR(beamAllocEvent);
		itsBeamServer->send(beamAllocEvent);		// will result in BS_BEAMALLOCACK;

		// find next beam if any.
		if (allocatingDigitalBeams) {									// which beam vector?
			if (++beamIdx < itsObs->beams.size()) {				// still dig beams left?
				curBeamName = itsObs->beams[beamIdx].name;
			}
			else {
				reachedEnd = true;
			}
		}
	}
	break;

	case IBS_BEAMALLOCACK: {
		IBSBeamallocackEvent		ack(event);
		if (ack.status != IBS_NO_ERR) {
			LOG_ERROR_STR("Beamlet allocation for beam " << ack.beamName 
					  << " failed with errorcode: " << errorName(ack.status));
			setObjectState("Beamlet alloc error", itsPropertySet->getFullScope(), RTDB_OBJ_STATE_BROKEN);
			setState(CTState::CLAIMED);
			sendControlResult(*itsParentPort, CONTROL_PREPARED, getName(), CT_RESULT_BEAMALLOC_FAILED);
			TRAN(BeamControl::claimed_state);
			return (GCFEvent::HANDLED);
		}
		itsBeamIDs.insert(ack.beamName);
		LOG_INFO_STR("Beam " << ack.beamName << " allocated succesfully");

		if (reachedEnd) {
			TRAN(BeamControl::sendPointings_state);
		}
		else {
			TRAN(BeamControl::allocBeams_state);	// tran to myself to exec the ENTRY state again.
		}
	}
	break;

	case F_EXIT:
		break;

	default:
		LOG_DEBUG_STR("allocBeams:Postponing event " << eventName(event) << " to next state");
		return (GCFEvent::NEXT_STATE);
	break;
	}

	return (GCFEvent::HANDLED);
}


//
// sendPointings_state(event, port)
//
// Substate during the PREPARE fase.
//
GCFEvent::TResult BeamControl::sendPointings_state(GCFEvent& event, GCFPortInterface& port)
{
	static bool		reachedEnd(false);
	static bool		sendingDigitalPts(itsObs->beams.size()>0);
	static uint		beamIdx(0);
	static vector<Observation::Pointing>::const_iterator	ptIter = 	// set ptr to first dig or ana pointing.
				sendingDigitalPts ? itsObs->beams[beamIdx].pointings.begin() : itsObs->anaBeams[beamIdx].pointings.begin();
	static string	curBeamName(sendingDigitalPts ? itsObs->beams[beamIdx].name : itsObs->anaBeams[beamIdx].name);

	LOG_DEBUG_STR("sendPointings:" << eventName(event) << "@" << port.getName());

	//
	// Create a new subarray
	//
	switch (event.signal) {
	case F_ENTRY: {
		// prepare pointTo event for current pts of current beam.
		IBSPointtoEvent 	ptEvent;
		ptEvent.beamName = curBeamName;
		ptEvent.pointing = IBS_Protocol::Pointing(ptIter->angle1, ptIter->angle2, ptIter->directionType,
												  RTC::Timestamp(ptIter->startTime,0), ptIter->duration);
		ptEvent.analogue = !sendingDigitalPts;
		ptEvent.rank	 = sendingDigitalPts ? 0 : itsObs->anaBeams[beamIdx].rank;	// rank not used by digital beams
		
		// find next pt if any.
		++ptIter;
		LOG_DEBUG_STR("sendingDigitalPts=" << sendingDigitalPts);
		if (sendingDigitalPts) {									// which beam vector?
			if (ptIter == itsObs->beams[beamIdx].pointings.end()) {	// last pt of this beam?
				LOG_DEBUG("last pt of this digbeam reached");
				if (++beamIdx < itsObs->beams.size()) {				// still dig beams left?
					LOG_DEBUG("going to the next digbeam");
					ptIter = itsObs->beams[beamIdx].pointings.begin();	// set ptIter to 1st pt of next beam
					curBeamName = itsObs->beams[beamIdx].name;
				}
				else {	// end of digital beams reached
					LOG_DEBUG("end of dig beams reached");
					if (itsObs->anaBeams.size()) {					// any analogue beams?
						LOG_DEBUG("detected analogue beams");
						sendingDigitalPts = false;
						beamIdx = 0;
						ptIter = itsObs->anaBeams[beamIdx].pointings.begin();
						curBeamName = itsObs->beams[beamIdx].name;		// NOTE: using DIGITALname!!!!
//						curBeamName = itsObs->anaBeams[beamIdx].name;
					}
					else {
						LOG_DEBUG_STR("reached end");
						reachedEnd = true;
					}
				}
			}
		}
		else {	// sending analogue pts
			if (ptIter == itsObs->anaBeams[beamIdx].pointings.end()) {	// last pt of this beam?
				LOG_DEBUG("last pt of this anabeam reached");
				if (++beamIdx < itsObs->anaBeams.size()) {				// still anabeams left?
					LOG_DEBUG("going to the next anabeam");
					ptIter = itsObs->anaBeams[beamIdx].pointings.begin();	// set iter to 1st pt of next beam
//					curBeamName = itsObs->beams[0].name;			// remember name
					curBeamName = itsObs->beams[beamIdx].name;			// remember name  NOTE: using DIGITALname!!!!
				}
				else {
					LOG_DEBUG_STR("reached end");
					reachedEnd = true;
				}
			}
		}

		// Fill in last field and send pointing.
		ptEvent.isLast = reachedEnd;
		itsBeamServer->send(ptEvent);		// results in POINTTOACK event.
		LOG_INFO_STR("Sending" << (ptEvent.isLast ? " last" : "") << " pointing: " << ptEvent.pointing);
		LOG_DEBUG_STR(ptEvent);
	}
	break;

	case IBS_POINTTOACK: {
		IBSPointtoackEvent ack(event);
		if (ack.status != IBS_Protocol::IBS_NO_ERR) {
			LOG_ERROR_STR("Sending pointing(s) for beam " << curBeamName << " failed, Error: " << errorName(ack.status));
			setState(CTState::CLAIMED);
			sendControlResult(*itsParentPort, CONTROL_PREPARED, getName(), CT_RESULT_BEAMALLOC_FAILED);
			TRAN(BeamControl::claimed_state);
			return (GCFEvent::HANDLED);
		}
		if (!reachedEnd) {
			TRAN(BeamControl::sendPointings_state);	// tran to myself to exec the ENTRY state again.
			return (GCFEvent::HANDLED);
		}
		// ready.
		setState(CTState::PREPARED);
		sendControlResult(*itsParentPort, CONTROL_PREPARED, getName(), CT_RESULT_NO_ERROR);
		beamsToPVSS();		// bring PVSS info uptodate.
		TRAN(BeamControl::active_state);
	}
	break;

	default:
		LOG_DEBUG_STR("sendPointings:Postponing event " << eventName(event) << " to next state");
		return (GCFEvent::NEXT_STATE);
	break;
	}

	return (GCFEvent::HANDLED);
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
		ASSERTSTR (&port == itsBeamServer, "F_DISCONNECTED event from port " << port.getName());
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
		LOG_INFO_STR("Received RELEASE(" << msg.cntlrName << ")");
		setState(CTState::RELEASE);
		if (!doRelease()) {
			LOG_WARN_STR("Cannot release a beam that was not allocated, continuing");
			setState(CTState::RELEASED);
			sendControlResult(*itsParentPort, CONTROL_RELEASED, getName(), CT_RESULT_NO_ERROR);
			TRAN(BeamControl::claimed_state);
		}
		// else a BS_BEAMFREEACK event will be sent
		break;
	}

	case CONTROL_QUIT:
		TRAN(BeamControl::quiting_state);
		break;

	// -------------------- EVENTS RECEIVED FROM BEAMSERVER --------------------
	case IBS_BEAMFREEACK: {
		if (!handleBeamFreeAck(event)) {
			LOG_WARN("Error in freeing beam, jump to quit state.");
			TRAN(BeamControl::quiting_state);
		}
		if (itsBeamIDs.empty()) {	// answer on all beams received?
			LOG_INFO("Released beam(s) going back to 'claimed' mode");
			setState(CTState::RELEASED);
			sendControlResult(*itsParentPort, CONTROL_RELEASED, getName(), CT_RESULT_NO_ERROR);
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
// beamsToPVSS()
//
// Send an allocation event for every beam to the beamserver
//
void BeamControl::beamsToPVSS()
{
	GCFPValueArray		angle1Arr;
	GCFPValueArray		angle2Arr;
	GCFPValueArray		dirTypesArr;
	GCFPValueArray		subbandArr;
	GCFPValueArray		beamletArr;
	GCFPValueArray		beamIDArr;

	for (uint32 i(0); i < itsObs->beams.size(); i++) {
		// store values in PVSS for operator
		// TO CHANGE: get the info from the anaBeams when no digbeams are defined.
		stringstream		os;
		writeVector(os, itsObs->beams[i].subbands);
		subbandArr.push_back   (new GCFPVString  (os.str()));
		os.clear();
		writeVector(os, itsObs->getBeamlets(i));
		beamletArr.push_back   (new GCFPVString  (os.str()));
		angle1Arr.push_back	   (new GCFPVDouble  (itsObs->beams[i].pointings[0].angle1));
		angle2Arr.push_back	   (new GCFPVDouble  (itsObs->beams[i].pointings[0].angle2));
		dirTypesArr.push_back  (new GCFPVString  (itsObs->beams[i].pointings[0].directionType));
		beamIDArr.push_back	   (new GCFPVString  (""));
	}

	itsPropertySet->setValue(PN_BC_SUBBAND_LIST,	GCFPVDynArr(LPT_DYNSTRING, subbandArr));
	itsPropertySet->setValue(PN_BC_BEAMLET_LIST,	GCFPVDynArr(LPT_DYNSTRING, beamletArr));
	itsPropertySet->setValue(PN_BC_ANGLE1,			GCFPVDynArr(LPT_DYNDOUBLE, angle1Arr));
	itsPropertySet->setValue(PN_BC_ANGLE2,			GCFPVDynArr(LPT_DYNDOUBLE, angle2Arr));
	itsPropertySet->setValue(PN_BC_DIRECTION_TYPE,	GCFPVDynArr(LPT_DYNSTRING, dirTypesArr));
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

	IBSBeamfreeEvent		beamFreeEvent;
	set<string>::iterator	iter = itsBeamIDs.begin();
	set<string>::iterator	end  = itsBeamIDs.end();
	while (iter != end) {
		beamFreeEvent.beamName = *iter;
		LOG_INFO_STR("Asking BeamServer to release beam " << beamFreeEvent.beamName);
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
	IBSBeamfreeackEvent	ack(event);
	if (ack.status != IBS_NO_ERR) {
		LOG_ERROR_STR("Beam de-allocation failed with errorcode: " << errorName(ack.status));
		itsPropertySet->setValue(PN_FSM_ERROR,GCFPVString("De-allocation of the beam failed."));
		return (false);	
	}

	set<string>::iterator	iter = itsBeamIDs.begin();
	set<string>::iterator	end  = itsBeamIDs.end();
	while(iter != end) {
		if (ack.beamName == *iter) {
			// clear beam in PVSS
#if 0
			// TODO: those fields are dynArrays.
			itsPropertySet->setValue(PN_BC_SUBBAND_LIST,	GCFPVString(""));
			itsPropertySet->setValue(PN_BC_BEAMLET_LIST,	GCFPVString(""));
			itsPropertySet->setValue(PN_BC_ANGLE1,			GCFPVString(""));
			itsPropertySet->setValue(PN_BC_ANGLE2,			GCFPVString(""));
		//	itsPropertySet->setValue(PN_BC_ANGLETIMES,		GCFPVString(""));
#endif
			LOG_INFO_STR("De-allocation of beam " << *iter << " succesful");
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
			 if (sendControlResult(port, event.signal, getName(), CT_RESULT_NO_ERROR)) {
				result = GCFEvent::HANDLED;
			}
			break;
		
		case CONTROL_QUIT:
			TRAN(BeamControl::quiting_state);
			break;
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
			result = GCFEvent::HANDLED;
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
