//#  OnlineControl.cc: Implementation of the MAC Scheduler task
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
#include <signal.h>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <Common/LofarLogger.h>
#include <Common/LofarLocators.h>
#include <Common/StreamUtil.h>
#include <Common/SystemUtil.h>
#include <Common/ParameterSet.h>
#include <Common/ParameterRecord.h>
#include <Common/Exceptions.h>
#include <Common/SystemUtil.h>
#include <Common/hexdump.h>
#include <ApplCommon/StationInfo.h>
#include <ApplCommon/Observation.h>
#include <ApplCommon/LofarDirs.h>
#include <MACIO/MACServiceInfo.h>
#include <GCF/TM/GCF_Protocols.h>
#include <GCF/PVSS/GCF_PVTypes.h>
#include <GCF/PVSS/PVSSservice.h>
#include <GCF/RTDB/DP_Protocol.ph>
#include <APL/APLCommon/APL_Defines.h>
#include <APL/APLCommon/APLUtilities.h>
#include <APL/APLCommon/ControllerDefines.h>
#include <APL/APLCommon/Controller_Protocol.ph>
#include <APL/APLCommon/CTState.h>
#include "OnlineControl.h"
#include "Response.h"
#include "forkexec.h"
#include <OTDB/TreeValue.h>			// << need to include this after OnlineControl! ???
#include "PVSSDatapointDefs.h"

using namespace LOFAR::GCF::PVSS;
using namespace LOFAR::GCF::TM;
using namespace LOFAR::GCF::RTDB;
using namespace boost::posix_time;
using namespace std;

namespace LOFAR {
	using namespace APLCommon;
	using namespace OTDB;
	namespace CEPCU {
	
// static pointer to this object for signal handler
static OnlineControl*	thisOnlineControl = 0;

//
// OnlineControl()
//
OnlineControl::OnlineControl(const string&	cntlrName) :
	GCFTask 			((State)&OnlineControl::initial_state,cntlrName),
	itsPropertySet		(0),
	itsBGPApplPropSet	(0),
	itsPropertySetInitialized (false),
	itsPVSSService		(0),
	itsPVSSResponse		(0),
	itsParentControl	(0),
	itsParentPort		(0),
	itsTimerPort		(0),
	itsForcedQuitTimer	(0),
	itsLogControlPort	(0),
	itsState			(CTState::NOSTATE),
	itsFeedbackListener	(0),					// QUICK FIX #4022
	itsFeedbackPort		(0),					// QUICK FIX #4022
	itsFeedbackResult	(CT_RESULT_NO_ERROR),	// QUICK FIX #4022
	itsTreePrefix       (""),
	itsInstanceNr       (0),
	itsStartTime        (),
	itsStopTime         (),
	itsStopTimerID      (0),
	itsFinishTimerID 	(0),
	itsInFinishState	(false),
	itsForceTimeout		(3600.0)
{
	LOG_TRACE_OBJ_STR (cntlrName << " construction");

	// First readin our observation related config file.
	LOG_DEBUG_STR("Reading parset file:" << LOFAR_SHARE_LOCATION << "/" << cntlrName);
	globalParameterSet()->adoptFile(string(LOFAR_SHARE_LOCATION)+"/"+cntlrName);

	// Readin some parameters from the ParameterSet.
	itsTreePrefix = globalParameterSet()->getString("prefix");
	itsInstanceNr = globalParameterSet()->getUint32("_instanceNr");
	itsStartTime  = time_from_string(globalParameterSet()->getString("Observation.startTime"));
	itsStopTime   = time_from_string(globalParameterSet()->getString("Observation.stopTime"));

	// attach to parent control task
	itsParentControl = ParentControl::instance();

	// need port for timers.
	itsTimerPort = new GCFTimerPort(*this, "TimerPort");
	ASSERTSTR(itsTimerPort, "Can't allocate the timer!");

	// Controlport to logprocessor
	itsLogControlPort = new GCFTCPPort(*this, MAC_SVCMASK_CEPLOGCONTROL, GCFPortInterface::SAP, CONTROLLER_PROTOCOL);
	ASSERTSTR(itsLogControlPort, "Can't allocate the logControlPort");

	// QUICK FIX #4022
	itsFeedbackListener = new GCFTCPPort (*this, "Feedbacklistener", GCFPortInterface::MSPP, CONTROLLER_PROTOCOL);
	ASSERTSTR(itsFeedbackListener, "Cannot allocate TCP port for feedback");
	itsForcedQuitTimer = new GCFTimerPort(*this, "EmergencyTimer");
	ASSERTSTR(itsForcedQuitTimer, "Can't allocate the emergency timer!");
	itsForceTimeout = globalParameterSet()->getTime("emergencyTimeout", 3600);

	// for debugging purposes
	registerProtocol (CONTROLLER_PROTOCOL, CONTROLLER_PROTOCOL_STRINGS);
	registerProtocol (DP_PROTOCOL, 		DP_PROTOCOL_STRINGS);

	_setState(CTState::CREATED);
}


//
// ~OnlineControl()
//
OnlineControl::~OnlineControl()
{
	LOG_TRACE_OBJ_STR (getName() << " destruction");
	if (itsLogControlPort) {
		itsLogControlPort->close();
		delete itsLogControlPort;
	}

	if (itsTimerPort) {
		delete itsTimerPort;
	}

	if (itsFeedbackListener) {
		itsFeedbackListener->close();
		delete itsFeedbackListener;
	}
}

//
// signalHandler(signum)
//
void OnlineControl::signalHandler(int	signum)
{
	LOG_INFO (formatString("SIGNAL %d detected", signum));

	if (thisOnlineControl) {
		thisOnlineControl->finish(CT_RESULT_MANUAL_ABORT);
	}
}

//
// finish(result)
//
void OnlineControl::finish(int	result)
{
	itsFeedbackResult = result;
	TRAN(OnlineControl::finishing_state);
}


//
// _setState(CTstateNr)
//
void    OnlineControl::_setState(CTState::CTstateNr     newState)
{
	CTState		cts;
	LOG_DEBUG_STR ("Going from state " << cts.name(itsState) << " to " << cts.name(newState));
	itsState = newState;

	// Update PVSS to inform operator.
	if (itsPropertySet) {
		CTState		cts;
		itsPropertySet->setValue(PN_FSM_CURRENT_ACTION, GCFPVString(cts.name(newState)));
	}
}   


//
// _databaseEventHandler(event)
//
void OnlineControl::_databaseEventHandler(GCFEvent& event)
{
	LOG_DEBUG_STR ("_databaseEventHandler:" << eventName(event));

	switch(event.signal) {

	case DP_CHANGED: {
		// TODO: implement something usefull.
		break;
	}  

	default:
		break;
	}  
}


//
// initial_state(event, port)
//
// Create my own propertySet
//
GCFEvent::TResult OnlineControl::initial_state(GCFEvent& event, GCFPortInterface& port)
{
	LOG_INFO_STR ("initial:" << eventName(event) << "@" << port.getName());

	GCFEvent::TResult status = GCFEvent::HANDLED;
  
	switch (event.signal) {
	case F_ENTRY:
   		break;

    case F_INIT: {
		// create PVSS hook for direct access to PVSS datapoints
		itsPVSSResponse = new Response;
		ASSERTSTR(itsPVSSResponse, "Failed to allocate a PVSS response class");
		itsPVSSService  = new PVSSservice(itsPVSSResponse);
		ASSERTSTR(itsPVSSService, "Failed to allocate a PVSS Service class");

		// Get access to my own propertyset.
//		uint32	obsID = globalParameterSet()->getUint32("Observation.ObsID");
		string obsDPname = globalParameterSet()->getString("_DPname");
		string	propSetName(createPropertySetName(PSN_ONLINE_CONTROL, getName(), obsDPname));
		LOG_DEBUG_STR ("Activating PropertySet: "<< propSetName);
		itsPropertySet = new RTDBPropertySet(propSetName,
											 PST_ONLINE_CONTROL,
											 PSAT_RW,
											 this);
	} break;

	case DP_CREATED: {
		// NOTE: this function may be called DURING the construction of the PropertySet.
		// Always exit this event in a way that GCF can end the construction.
		DPCreatedEvent  dpEvent(event);
		LOG_DEBUG_STR("Result of creating " << dpEvent.DPname << " = " << dpEvent.result);
		itsTimerPort->cancelAllTimers();
		itsTimerPort->setTimer(0.1);
	} break;

	case F_TIMER: {	// must be timer that PropSet is online.
		// update PVSS.
		LOG_TRACE_FLOW ("Updateing state to PVSS");
		itsPropertySet->setValue(PN_FSM_CURRENT_ACTION, GCFPVString("initial"));
		itsPropertySet->setValue(PN_FSM_ERROR, GCFPVString(""));

   		LOG_DEBUG ("Going to create BGPAppl datapoint");
		TRAN(OnlineControl::propset_state);				// go to next state.
	} break;

	case F_CONNECTED:
		break;

	case F_DISCONNECTED:
		_handleDisconnect(port);
		break;
	
	default:
		LOG_DEBUG_STR ("initial, default");
		status = GCFEvent::NOT_HANDLED;
		break;
	}    
	return (status);
}

//
// propset_state(event, port)
//
// Connect to BGPAppl DP and start rest of tasks
//
GCFEvent::TResult OnlineControl::propset_state(GCFEvent& event, GCFPortInterface& port)
{
	LOG_INFO_STR ("propset:" << eventName(event) << "@" << port.getName());

	GCFEvent::TResult status = GCFEvent::HANDLED;
  
	switch (event.signal) {
	case F_ENTRY: {
		// Get access to my own propertyset.
		string obsDPname = globalParameterSet()->getString("_DPname");
		string	propSetName(createPropertySetName(PSN_BGP_APPL, getName(), obsDPname));
		LOG_DEBUG_STR ("Activating PropertySet: "<< propSetName);
		itsBGPApplPropSet = new RTDBPropertySet(propSetName,
											 PST_BGP_APPL,
											 PSAT_RW,
											 this);
	} break;

	case DP_CREATED: {
		// NOTE: this function may be called DURING the construction of the PropertySet.
		// Always exit this event in a way that GCF can end the construction.
		DPCreatedEvent  dpEvent(event);
		LOG_DEBUG_STR("Result of creating " << dpEvent.DPname << " = " << dpEvent.result);
		itsTimerPort->cancelAllTimers();
		itsTimerPort->setTimer(0.1);
	} break;

	case F_TIMER: {	// must be timer that PropSet is online.
		// start StopTimer for safety.
		LOG_INFO("Starting QUIT timer that expires 5 seconds after end of observation");
		ptime	now(second_clock::universal_time());
		itsStopTimerID = itsTimerPort->setTimer(time_duration(itsStopTime - now).total_seconds() + 5.0);

		// Start ParentControl task
		LOG_DEBUG ("Enabling ParentControl task");
		itsParentPort = itsParentControl->registerTask(this);
		// results in CONTROL_CONNECT

		// QUICK FIX #4022
		uint32	obsID = globalParameterSet()->getUint32("Observation.ObsID");
		LOG_INFO_STR("Openening feedback port for OLAP: " << MAC_ONLINE_FEEDBACK_QF + obsID%1000);
		itsFeedbackListener->setPortNumber(MAC_ONLINE_FEEDBACK_QF + obsID%1000);
		itsFeedbackListener->open();	// will result in F_CONN

		LOG_DEBUG ("Going to operational state");
		TRAN(OnlineControl::active_state);				// go to next state.
	} break;

	case F_CONNECTED:
		break;

	case F_DISCONNECTED:
		_handleDisconnect(port);
		break;
	
	default:
		LOG_DEBUG_STR ("propset, default");
		status = GCFEvent::NOT_HANDLED;
		break;
	}    
	return (status);
}


//
// active_state(event, port)
//
// Normal operation state. 
//
GCFEvent::TResult OnlineControl::active_state(GCFEvent& event, GCFPortInterface& port)
{
	LOG_INFO_STR ("active:" << eventName(event) << "@" << port.getName());

	GCFEvent::TResult status = GCFEvent::HANDLED;

	switch (event.signal) {
	case F_ENTRY: {
		// update PVSS
		itsPropertySet->setValue(PN_FSM_CURRENT_ACTION, GCFPVString("active"));
		itsPropertySet->setValue(PN_FSM_ERROR, GCFPVString(""));
	} break;

	// QUICKFIX #4022
	case F_ACCEPT_REQ: {
		_handleAcceptRequest(port);
	} break;

	case F_CONNECTED: {
//		ASSERTSTR (&port == itsParentPort, "F_CONNECTED event from port " << port.getName());
		LOG_INFO_STR("F_CONNECTED event from port " << port.getName());
	} break;

	case F_DISCONNECTED: {
		_handleDisconnect(port);
	} break;

	// QUICKFIX #4022
	case F_DATAIN: {
		_handleDataIn(port);
	} break;

	case DP_CHANGED:
		_databaseEventHandler(event);
		break;

	case F_TIMER:  {
		GCFTimerEvent& timerEvent=static_cast<GCFTimerEvent&>(event);
		if (timerEvent.id == itsStopTimerID) {
			LOG_DEBUG("StopTimer expired, starting QUIT sequence");
			itsStopTimerID = 0;
			_setState(CTState::QUIT);
			_stopApplications();
			itsForcedQuitTimer->setTimer(itsForceTimeout);
			// wait for ABORT or FINISHED from applications or timer to expire.
		}
		else if (&port == itsForcedQuitTimer) {
			LOG_INFO("Forcing quit");
			finish(CT_RESULT_EMERGENCY_TIMEOUT);
		}
		else {
			LOG_WARN_STR("Received unknown timer event");
		}
	}
	break;

	// -------------------- EVENTS RECEIVED FROM PARENT CONTROL --------------------
	case CONTROL_CONNECT: {
		CONTROLConnectEvent		msg(event);
		LOG_DEBUG_STR("Received CONNECT(" << msg.cntlrName << ")");
		itsMyName = msg.cntlrName;
		// first inform CEPlogProcessor
		CONTROLAnnounceEvent		announce;
		announce.observationID = toString(getObservationNr(msg.cntlrName));
		itsLogControlPort->send(announce);
		// execute this state
		_setState(CTState::CONNECT);
		_setupBGPmappingTables();
		uint32 result = _startApplications();			// prep parset and call startBGP.sh
		// respond to parent
		sendControlResult(port, event.signal, msg.cntlrName, result);
		if (result == CT_RESULT_NO_ERROR) {
			_setState(CTState::CONNECTED);
		}
		break;
	}

	case CONTROL_SCHEDULED: {
		CONTROLScheduledEvent		msg(event);
		LOG_DEBUG_STR("Received SCHEDULED(" << msg.cntlrName << ")");
		// TODO: do something usefull with this information!
		break;
	}

	case CONTROL_CLAIM: {
		CONTROLClaimEvent		msg(event);
		LOG_DEBUG_STR("Received CLAIM(" << msg.cntlrName << ")");
		sendControlResult(port, event.signal, msg.cntlrName, CT_RESULT_NO_ERROR);
		_setState(CTState::CLAIMED);
		break;
	}

	case CONTROL_PREPARE: {
		CONTROLPrepareEvent		msg(event);
		LOG_DEBUG_STR("Received PREPARE(" << msg.cntlrName << ")");
		sendControlResult(port, event.signal, msg.cntlrName, CT_RESULT_NO_ERROR);
		_setState(CTState::PREPARED);
		break;
	}

	case CONTROL_RESUME: {
		CONTROLResumeEvent		msg(event);
		LOG_DEBUG_STR("Received RESUME(" << msg.cntlrName << ")");
		itsStartTime = second_clock::universal_time();	// adjust to latest run.
		sendControlResult(port, event.signal, msg.cntlrName, CT_RESULT_NO_ERROR);
		_setState(CTState::RESUMED);
		break;
	}

	case CONTROL_SUSPEND: {
		CONTROLSuspendEvent		msg(event);
		LOG_DEBUG_STR("Received SUSPEND(" << msg.cntlrName << ")");
		sendControlResult(port, event.signal, msg.cntlrName, CT_RESULT_NO_ERROR);
		_setState(CTState::SUSPEND);
		break;
	}

	case CONTROL_RELEASE: {
		CONTROLReleaseEvent		msg(event);
		LOG_DEBUG_STR("Received RELEASE(" << msg.cntlrName << ")");
		sendControlResult(port, event.signal, msg.cntlrName, CT_RESULT_NO_ERROR);
		_setState(CTState::RELEASED);
		break;
	}

	case CONTROL_QUIT: {
		CONTROLQuitEvent		msg(event);
		LOG_DEBUG_STR("Received QUIT(" << msg.cntlrName << ")");
		_setState(CTState::QUIT);
		_stopApplications();
		itsForcedQuitTimer->setTimer(itsForceTimeout);
		// wait for ABORT or FINISHED from applications or timer to expire.
		break;
	}

	default:
		LOG_DEBUG("active_state, default");
		status = GCFEvent::NOT_HANDLED;
		break;
	}

	return (status);
}

//
// completing_state(event, port)
//
//
GCFEvent::TResult OnlineControl::completing_state(GCFEvent& event, GCFPortInterface& port)
{
	LOG_INFO_STR ("completing:" << eventName(event) << "@" << port.getName());

	switch (event.signal) {
	case F_ENTRY: {
		// update PVSS
		itsPropertySet->setValue(PN_FSM_CURRENT_ACTION, GCFPVString("completing"));
		itsPropertySet->setValue(PN_FSM_ERROR, GCFPVString(""));
		itsInFinishState = true;

		_passMetadatToOTDB();

		TRAN(OnlineControl::finishing_state);
	} break;

	case F_TIMER:
		break;

	case F_DISCONNECTED:
		_handleDisconnect(port);
		break;

	case F_DATAIN:
		_handleDataIn(port);
		break;

	default:
		LOG_DEBUG("completing state default");
		return (GCFEvent::NOT_HANDLED);
	}

	return (GCFEvent::HANDLED);
}

//
// finishing_state(event, port)
//
//
GCFEvent::TResult OnlineControl::finishing_state(GCFEvent& event, GCFPortInterface& port)
{
	LOG_INFO_STR ("finishing:" << eventName(event) << "@" << port.getName());

	switch (event.signal) {
	case F_ENTRY: {
		// update PVSS
		itsPropertySet->setValue(PN_FSM_CURRENT_ACTION, GCFPVString("finished"));
		itsPropertySet->setValue(PN_FSM_ERROR, GCFPVString(""));

		CONTROLQuitedEvent      msg;
		msg.cntlrName = itsMyName;
		msg.result = itsFeedbackResult;
		itsParentPort->send(msg);
		itsTimerPort->cancelAllTimers();
		itsTimerPort->setTimer(0.3);
	} break;

	case F_TIMER:
		GCFScheduler::instance()->stop();
		break;

	case F_DISCONNECTED:
		_handleDisconnect(port);
		break;

	case F_DATAIN:
		_handleDataIn(port);
		break;

	default:
		LOG_DEBUG("finishing_state default");
		return (GCFEvent::NOT_HANDLED);
	}
	return (GCFEvent::HANDLED);
}

//
// _setupBGPmappingTables
//
void OnlineControl::_setupBGPmappingTables()
{
	Observation		theObs(globalParameterSet(), false);
	int	nrStreams = theObs.streamsToStorage.size();
	LOG_INFO_STR("_setupBGPmapping: " << nrStreams << " streams found.");

	// Which IOnodes and Adders are used is collected in arrays and written to BGPAppl datapoints.
	// e.g. BGPAppl.IONodelist = {0,1,2,3} ; BGPAppl.AdderList = {[0,2,3],[0,1,2],[3,6,2],[6,6,6]}
	// The dataproduct, writer and locus information is written to datapoint in the related adder.
	// eg. IONode99.Adder0.dataProductType=Correlated, IONode99.Adder0.dataProduct=L55522_SAP000_SB000_uv.MS, 
	//     IONode99.Adder0.locusnode=2, IONode99.Adder0.writer=0
	// BGPAppl vectors
	GCFPValueArray	ionodeArr;
	GCFPValueArray	adderArr;
	// Adder vector
	vector<string>		fields;
	fields.push_back("dataProductType");
	fields.push_back("dataProduct");
	fields.push_back("locusNode");
	fields.push_back("writer");

	uint	prevPset = (nrStreams ? theObs.streamsToStorage[0].sourcePset : -1);
	vector<int>		adderVector;
	for (int i = 0; i < nrStreams; i++) {
		// BGPAppl information
		if (theObs.streamsToStorage[i].sourcePset != prevPset) {	// other Pset? write current vector to the database.
			ionodeArr.push_back(new GCFPVInteger(prevPset));
			{	stringstream	os;
				writeVector(os, adderVector);
				adderArr.push_back (new GCFPVString(os.str()));
			}
			// clear the collecting vectors
			adderVector.clear();
			prevPset = theObs.streamsToStorage[i].sourcePset;
		}
		// extend vector with info
		adderVector.push_back (theObs.streamsToStorage[i].adderNr);

		// Adder information
		string	propSetMask(createPropertySetName(PSN_ADDER, "", ""));
		string	adderDPname(formatString(propSetMask.c_str(), theObs.streamsToStorage[i].sourcePset, 
							theObs.streamsToStorage[i].adderNr));
		vector<GCFPValue*>	values;
		values.push_back(new GCFPVString (theObs.streamsToStorage[i].dataProduct));
		values.push_back(new GCFPVString (theObs.streamsToStorage[i].filename));
		int	locusNodeNr(0);
		if (sscanf(theObs.streamsToStorage[i].destStorageNode.c_str(), "locus%d", &locusNodeNr) != 1) {
			LOG_ERROR_STR("Cannot determine number in '" << theObs.streamsToStorage[i].destStorageNode <<"'");
		}
		values.push_back(new GCFPVInteger(locusNodeNr));
		values.push_back(new GCFPVInteger(theObs.streamsToStorage[i].writerNr));
		itsPVSSService->dpeSetMultiple(adderDPname, fields, values, 0.0, false); // ignore answer
		// release claimed memory for Adder
		for (int i = values.size()-1; i>=0; i--) {
			delete values[i];
		}
	}
	itsBGPApplPropSet->setValue(PN_BGPA_IO_NODE_LIST, GCFPVDynArr(LPT_DYNINTEGER, ionodeArr));
	itsBGPApplPropSet->setValue(PN_BGPA_ADDER_LIST,   GCFPVDynArr(LPT_DYNSTRING,  adderArr));

	// release claimed memory for BGPAppl.
	for (int i = ionodeArr.size()-1; i>=0; i--) {
		delete ionodeArr[i];
		delete adderArr[i];
	}
}

//
// _startApplications()
//
// Create ParameterSets for all Applications the we have to manage, start all
// ACC's and give them the boot command.
//
uint32 OnlineControl::_startApplications()
{
	ParameterSet*	thePS  = globalParameterSet();		// shortcut to global PS.

	// Get list of all application that should be managed
	// Note: each application = 1 ACC
	vector<string> applList = thePS->getStringVector("applications");
	string 	paramFileName;

	uint32	result(CT_RESULT_NO_ERROR);
	for (size_t a = 0; a < applList.size(); a++) {
		// Initialize basic variables
		string 	applName  (applList[a]);
		string	applPrefix(applName+".");

		try {
			// Create a parameterSet for this AC.
			ParameterSet params;
			params.clear();
			// import and extend the ApplCtrl section
			params.adoptCollection(thePS->makeSubset("ApplCtrl","ApplCtrl"));
			params.replace("ApplCtrl.application", applName);
			params.replace("ApplCtrl.processes", thePS->getString(applPrefix+"processes"));
			params.replace("ApplCtrl.resultfile", formatString(
							"%s/ACC_%s_%s_result.param", LOFAR_SHARE_LOCATION, 
							getName().c_str(), applName.c_str()));

			// add application info
			params.adoptCollection(thePS->makeSubset(applPrefix,applName+"."));

			// import extra tree-parts if necc.
			vector<string>	extraParts=thePS->getStringVector(applPrefix+"extraInfo");
			for (size_t e = 0; e < extraParts.size(); e++) {
				if (extraParts[e][0] == '.') {	// relative part?
					string	partName = extraParts[e].substr(1);
					params.adoptCollection(thePS->makeSubset(partName, 
														 	 partName));
				}
				else {
					params.adoptCollection(thePS->makeSubset(extraParts[e],
															 extraParts[e]));
				}
			}

			// always add Observation and _DPname
			string	obsPrefix(thePS->locateModule("Observation"));
			params.adoptCollection(thePS->makeSubset(obsPrefix+"Observation", "Observation"));
			params.replace("_DPname", thePS->getString("_DPname"));

			// write parset to file.
			vector<string> procNames = thePS->getStringVector(applPrefix+"processes");
			string procName = procNames[0];
			uint32	obsID   = thePS->getUint32("Observation.ObsID");
			paramFileName = formatString("%s/%s_%d.param", LOFAR_SHARE_LOCATION, procName.c_str(), obsID);
			params.writeFile(paramFileName); 	// local copy
			string	accHost(thePS->getString(applPrefix+"_hostname"));
			LOG_DEBUG_STR("Controller for " << applName << " wil be running on " << accHost);
			remoteCopy(paramFileName,accHost,LOFAR_SHARE_LOCATION);

			string startCmd = formatString("ssh %s startBGL.sh %s %s %s %s %d 1", 
								accHost.c_str(),
								procName.c_str(),
								thePS->getString(applPrefix + procName + "._executable").c_str(),
								thePS->getString(applPrefix + procName + ".workingdir").c_str(),
								paramFileName.c_str(),
								obsID);

			// Finally start ApplController on the right host
			LOG_INFO_STR("Starting controller for " << applName << " in 5 seconds ");
			sleep(5);			 // sometimes we are too quick, wait a second.
			LOG_INFO_STR("About to start: " << startCmd);
			result |= forkexec(startCmd.c_str()) == 0 ? CT_RESULT_NO_ERROR : CT_RESULT_LOST_CONNECTION;
		} 
		catch (APSException &e) {
			// key not found. skip
			LOG_FATAL(e.text());
			result = CT_RESULT_UNSPECIFIED;
		}
	} // for

	return (result);
}

//
// _stopApplications()
//
void OnlineControl::_stopApplications()
{
	if (itsInFinishState) {
		LOG_INFO("Stop called twice, ignoring");
		return;
	}
	itsInFinishState = true;

	ParameterSet*	thePS  = globalParameterSet();		// shortcut to global PS.
	uint32	obsID   = thePS->getUint32("Observation.ObsID");
	vector<string> applList = thePS->getStringVector("applications");
	for (size_t a = 0; a < applList.size(); a++) {
		// Initialize basic variables
		string 	applName  (applList[a]);
		string	applPrefix(applName+".");
		vector<string> procNames = thePS->getStringVector(applPrefix+"processes","[]");
		string	procName = procNames[0];
		string	accHost  = thePS->getString(applPrefix+"_hostname", "UNKNOWN_HOST");

		// send stop to BGP
		string stopCmd = formatString("ssh %s stopBGL.sh %s %d", 
							accHost.c_str(),
							procName.c_str(),
							obsID);
		LOG_INFO_STR("About to execute: " << stopCmd);
		uint32	result = forkexec(stopCmd.c_str());
		LOG_INFO_STR ("Result of command = " << result);
	}

	// construct system command for starting an inspection program to qualify the measured data
	string  myPrefix    (thePS->locateModule("OnlineControl")+"OnlineControl.");
	string	inspectProg (thePS->getString(myPrefix+"inspectionProgram",  "@inspectionProgram@"));
	string	inspectHost (thePS->getString(myPrefix+"inspectionHost",     "@inspectionHost@"));
	bool	onRemoteMachine(inspectHost != myHostname(false)  && inspectHost != myHostname(true));
	string	startCmd;
	if (onRemoteMachine) {
		startCmd = formatString("ssh %s %s %d &", inspectHost.c_str(), inspectProg.c_str(), obsID);
	}
	else {
		startCmd = formatString("%s %d &", inspectProg.c_str(), obsID);
	}
	LOG_INFO_STR("About to start: " << startCmd);
	uint32	result = forkexec(startCmd.c_str());
	LOG_INFO_STR ("Result of command = " << result);
}

//
// _passMetadatToOTDB();
// THIS ROUTINE IS A MODIFIED COPY FROM PYTHONCONTROL.CC
//
void OnlineControl::_passMetadatToOTDB()
{
	// No name specified?
	bool	metadataFileAvailable (true);
	uint32	obsID(globalParameterSet()->getUint32("Observation.ObsID", 0));
	string  feedbackFile = observationParset(obsID)+"_feedback";
	LOG_INFO_STR ("Expecting metadata to be in file " << feedbackFile);
	if (feedbackFile.empty()) {
		metadataFileAvailable = false;
	}

	// read parameterset
	// Try to setup the connection with the database
	string	confFile = globalParameterSet()->getString("OTDBconfFile", "SASGateway.conf");
	ConfigLocator	CL;
	string	filename = CL.locate(confFile);
	LOG_INFO_STR("Trying to read database information from file " << filename);
	ParameterSet	otdbconf;
	otdbconf.adoptFile(filename);
	string database = otdbconf.getString("SASGateway.OTDBdatabase");
	string dbhost   = otdbconf.getString("SASGateway.OTDBhostname");
	OTDBconnection  conn("paulus", "boskabouter", database, dbhost);
	if (!conn.connect()) {
		LOG_FATAL_STR("Cannot connect to database " << database << " on machine " << dbhost);
		// WE DO HAVE A PROBLEM HERE BECAUSE THIS PIPELINE CANNOT BE SET TO FINISHED IN SAS.
		return;
	}
	LOG_INFO_STR("Connected to database " << database << " on machine " << dbhost);

	if (metadataFileAvailable) {
		try {
			TreeValue   tv(&conn, getObservationNr(getName()));
			ParameterSet	metadata;
			metadata.adoptFile(feedbackFile);
			// Loop over the parameterset and send the information to the KVTlogger.
			// During the transition phase from parameter-based to record-based storage in OTDB the
			// nodenames ending in '_' are implemented both as parameter and as record.
			ParameterSet::iterator		iter = metadata.begin();
			ParameterSet::iterator		end  = metadata.end();
			while (iter != end) {
				string	key(iter->first);	// make destoyable copy
				rtrim(key, "[]0123456789");
		//		bool	doubleStorage(key[key.size()-1] == '_');
				bool	isRecord(iter->second.isRecord());
				//   isRecord  doubleStorage
				// --------------------------------------------------------------
				//      Y          Y           store as record and as parameters
				//      Y          N           store as parameters
				//      N          *           store parameter
				if (!isRecord) {
					LOG_DEBUG_STR("BASIC: " << iter->first << " = " << iter->second);
					tv.addKVT(iter->first, iter->second, ptime(microsec_clock::local_time()));
				}
				else {
		//			if (doubleStorage) {
		//				LOG_DEBUG_STR("RECORD: " << iter->first << " = " << iter->second);
		//				tv.addKVT(iter->first, iter->second, ptime(microsec_clock::local_time()));
		//			}
					// to store is a node/param values the last _ should be stipped of
					key = iter->first;		// destroyable copy
		//			string::size_type pos = key.find_last_of('_');
		//			key.erase(pos,1);
					ParameterRecord	pr(iter->second.getRecord());
					ParameterRecord::const_iterator	prIter = pr.begin();
					ParameterRecord::const_iterator	prEnd  = pr.end();
					while (prIter != prEnd) {
						LOG_DEBUG_STR("ELEMENT: " << key+"."+prIter->first << " = " << prIter->second);
						tv.addKVT(key+"."+prIter->first, prIter->second, ptime(microsec_clock::local_time()));
						prIter++;
					}
				}
				iter++;
			}
			LOG_INFO_STR(metadata.size() << " metadata values send to SAS");
		}
		catch (APSException &e) {
			// Parameterfile not found
			LOG_FATAL(e.text());
		}
	}
}
// -------------------- Application-order administration --------------------

//
// _handleDisconnect(port)
//
void OnlineControl::_handleDisconnect(GCFPortInterface& port)
{
	port.close();
	// QUICKFIX #4022
	if (&port == itsFeedbackPort) {
		LOG_FATAL_STR("Lost connection with Feedback of OLAP.");
		delete itsFeedbackPort;
		itsFeedbackPort = 0;
	}
}

//
// _handleAcceptRequest(port)
//
void OnlineControl::_handleAcceptRequest(GCFPortInterface& port)
{
	ASSERTSTR(&port == itsFeedbackListener, "Incoming connection on main listener iso feedbackListener");
	itsFeedbackPort = new GCFTCPPort();
	itsFeedbackPort->init(*this, "feedback", GCFPortInterface::SPP, 0, true);   // raw port
	if (!itsFeedbackListener->accept(*itsFeedbackPort)) {
		delete itsFeedbackPort;
		itsFeedbackPort = 0;
		LOG_ERROR("Connection with Python feedback FAILED");
	}
	else {
		LOG_INFO("Connection made on feedback port, accepting commands");
	}
}

//
// _handleDataIn(port)
//
void OnlineControl::_handleDataIn(GCFPortInterface& port)
{
	ASSERTSTR(&port == itsFeedbackPort, "Didn't expect raw data on port " << port.getName());
	char    buf[1024];
	ssize_t btsRead = port.recv((void*)&buf[0], 1023);
	buf[btsRead] = '\0';
	string  s;
	hexdump(s, buf, btsRead);
	LOG_INFO_STR("Received command on feedback port: " << s);

	if (!strcmp(buf, "ABORT")) {
		itsFeedbackResult = CT_RESULT_PIPELINE_FAILED;
		TRAN(OnlineControl::completing_state);	// pass metadata
	}
	else if (!strcmp(buf, "FINISHED")) {
		TRAN(OnlineControl::completing_state);
	}
	else {
		LOG_FATAL_STR("Received command on feedback port unrecognized");
	}
}

}; // CEPCU
}; // LOFAR
