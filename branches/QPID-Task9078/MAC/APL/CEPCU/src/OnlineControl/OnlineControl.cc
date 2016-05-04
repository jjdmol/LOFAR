//#  OnlineControl.cc: Implementation of the MAC Scheduler task
//#
//#  Copyright (C) 2006
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, softwaresupport@astron.nl
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
#include <MessageBus/Protocols/TaskFeedbackState.h>
#include <ApplCommon/StationInfo.h>
#include <ApplCommon/Observation.h>
#include <ApplCommon/LofarDirs.h>
#include <MACIO/MACServiceInfo.h>
#include <GCF/TM/GCF_Protocols.h>
#include <GCF/PVSS/GCF_PVTypes.h>
#include <GCF/PVSS/PVSSservice.h>
#include <GCF/RTDB/DP_Protocol.ph>
#include <GCF/RTDB/DPservice.h>
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
	using namespace DP_Protocol;
	using namespace Controller_Protocol;
	using namespace APLCommon;
	using namespace OTDB;
	namespace CEPCU {
	
// static pointer to this object for signal handler
static OnlineControl*	thisOnlineControl = 0;

const double QUEUE_POLL_TIMEOUT = 1.0;

//
// OnlineControl()
//
OnlineControl::OnlineControl(const string&	cntlrName) :
	GCFTask 			((State)&OnlineControl::initial_state,cntlrName),
	itsPropertySet		(0),
	itsPropertySetInitialized (false),
	itsPVSSService		(0),
	itsPVSSResponse		(0),
	itsParentControl	(0),
	itsParentPort		(0),
	itsTimerPort		(0),
	itsForcedQuitTimer	(0),
	itsLogControlPort	(0),
	itsState			(CTState::NOSTATE),
	itsMsgQueue			(0),
	itsQueueTimer		(0),
	itsFeedbackResult	(CT_RESULT_NO_ERROR),
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
    itsQueueTimer = new GCFTimerPort(*this, "MsgQTimer");
    ASSERTSTR(itsQueueTimer, "Cannot allocate queue timer");

	// Controlport to logprocessor
	itsLogControlPort = new GCFTCPPort(*this, MAC_SVCMASK_CEPLOGCONTROL, GCFPortInterface::SAP, CONTROLLER_PROTOCOL);
	ASSERTSTR(itsLogControlPort, "Can't allocate the logControlPort");

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

	delete itsTimerPort;
	delete itsQueueTimer;
	delete itsMsgQueue;
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

		// start StopTimer for safety.
		LOG_INFO("Starting QUIT timer that expires 5 seconds after end of observation");
		ptime	now(second_clock::universal_time());
		itsStopTimerID = itsTimerPort->setTimer(time_duration(itsStopTime - now).total_seconds() + 5.0);

		// Start ParentControl task
		LOG_DEBUG ("Enabling ParentControl task");
		itsParentPort = itsParentControl->registerTask(this);
		// results in CONTROL_CONNECT

		// open connection with messagebus
		if (!itsMsgQueue) {
			string	queueName = globalParameterSet()->getString("TaskStateQueue");
			itsMsgQueue = new FromBus(queueName);
			LOG_INFO_STR("Starting to listen on " << queueName);
		}

		LOG_DEBUG ("Going to operational state");
		TRAN(OnlineControl::active_state);				// go to next state.
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
		itsQueueTimer->setTimer(QUEUE_POLL_TIMEOUT);
	} break;

	case F_CONNECTED: {
//		ASSERTSTR (&port == itsParentPort, "F_CONNECTED event from port " << port.getName());
		LOG_INFO_STR("F_CONNECTED event from port " << port.getName());
	} break;

	case F_DISCONNECTED: {
		_handleDisconnect(port);
	} break;

	case DP_CHANGED:
		_databaseEventHandler(event);
		break;

	case F_TIMER:  {
		GCFTimerEvent& timerEvent=static_cast<GCFTimerEvent&>(event);
		if (&port == itsQueueTimer) {
			Message	msg;
			if (itsMsgQueue->getMessage(msg, 0.1)) {
				Protocols::TaskFeedbackState content(msg.qpidMsg());
				string	obsIDstr = content.sasid.get();
				LOG_INFO_STR("Received message from task " << obsIDstr);
				if (atoi(obsIDstr.c_str()) == itsObsID) {
					string	result = content.state.get();
					if (result == "aborted") {
						itsFeedbackResult = CT_RESULT_PIPELINE_FAILED;
					}
					else if (result != "finished") {
						LOG_FATAL_STR("Unknown result received from correlator: " << result << " assuming failure!");
						itsFeedbackResult = CT_RESULT_PIPELINE_FAILED;
					}
					LOG_INFO_STR("Received '" << result << "' on the messagebus");
					itsMsgQueue->ack(msg);
					TRAN(OnlineControl::finishing_state);
					break;
				} // my obsid
				else {
					itsMsgQueue->reject(msg);
				}
			} // getMsg
			itsQueueTimer->setTimer(QUEUE_POLL_TIMEOUT);
		}
		else if (timerEvent.id == itsStopTimerID) {
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
		itsObsID  = getObservationNr(msg.cntlrName);
		// first inform CEPlogProcessor
		CONTROLAnnounceEvent		announce;
		announce.observationID = toString(itsObsID);
		itsLogControlPort->send(announce);
		// execute this state
		_setState(CTState::CONNECT);
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

	default:
		LOG_DEBUG("finishing_state default");
		return (GCFEvent::NOT_HANDLED);
	}
	return (GCFEvent::HANDLED);
}

//
// _startApplications()
//
// Create ParameterSets for all Applications the we have to manage, start all
// ACC's and give them the boot command.
//
uint32 OnlineControl::_startApplications()
{
	_clearCobaltDatapoints();

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

			string startCmd = formatString("ssh %s LOFARENV=$LOFARENV startBGL.sh %s %s %s %s %d 1", 
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
		string stopCmd = formatString("ssh %s LOFARENV=$LOFARENV stopBGL.sh %s %d", 
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
// _clearCobaltDatapoints()
//
void OnlineControl::_clearCobaltDatapoints()
{
	ParameterSet*	thePS  = globalParameterSet();		// shortcut to global PS.

	// create a datapoint service for clearing all the datapoints
	DPservice*	myDPservice = new DPservice(this);
	if (!myDPservice) {
		LOG_ERROR_STR("Can't allocate DPservice to PVSS to clear Cobalt values! Navigator contents no longer guaranteed");
		return;
	}

	// _DPname=LOFAR_ObsSW_TempObs0185
	string	DPbasename(thePS->getString("_DPname", "NO_DPNAME_IN_PARSET"));

	// OSCBT<000>_CobaltGPUProc<00> for 001-009 and 00-01
	string	propSetNameMask(createPropertySetName(PSN_COBALTGPU_PROC, getName(), DPbasename));
	// prepare 'cleared value set'
	vector<string>		fields;
	vector<GCFPValue*>	values;
	fields.push_back(PN_CGP_OBSERVATION_NAME);
	fields.push_back(PN_CGP_DATA_PRODUCT_TYPE);
	fields.push_back(PN_CGP_SUBBAND);
	fields.push_back(PN_CGP_DROPPING);
	fields.push_back(PN_CGP_WRITTEN);
	fields.push_back(PN_CGP_DROPPED);
	GCFPValueArray	emptyArr;
	values.push_back(new GCFPVString(""));
	values.push_back(new GCFPVString(""));
	values.push_back(new GCFPVDynArr(LPT_DYNINTEGER, emptyArr));
	values.push_back(new GCFPVDynArr(LPT_DYNBOOL, emptyArr));
	values.push_back(new GCFPVDynArr(LPT_DYNDOUBLE, emptyArr));
	values.push_back(new GCFPVDynArr(LPT_DYNDOUBLE, emptyArr));
	for (int nodeNr = 1; nodeNr <= 9; ++nodeNr) {
		for (int gpuNr = 0; gpuNr <= 1; ++gpuNr) {
			string	DPname(formatString(propSetNameMask.c_str(), nodeNr, gpuNr));
			LOG_DEBUG_STR("Clearing " << DPname);

			PVSSresult	result = myDPservice->setValue(DPname, fields, values, 0.0, false);
			if (result != SA_NO_ERROR) {
				LOG_WARN_STR("Call to PVSS for setValue for " << DPname << " returned: " << result);
			}
		}
	}
	// free allocated GCFValues.
	for (int i = values.size()-1 ; i >= 0; i--) {
		delete values[i];
	}
	values.clear();
	fields.clear();


	// CobaltOutputProc
	string	DPname(createPropertySetName(PSN_COBALT_OUTPUT_PROC, getName(), DPbasename));
	// prepare 'cleared value set'
	fields.push_back(PN_COP_LOCUS_NODE);
	fields.push_back(PN_COP_DATA_PRODUCT_TYPE);
	fields.push_back(PN_COP_FILE_NAME);
	fields.push_back(PN_COP_DIRECTORY);
	fields.push_back(PN_COP_DROPPING);
	fields.push_back(PN_COP_WRITTEN);
	fields.push_back(PN_COP_DROPPED);
	values.push_back(new GCFPVDynArr(LPT_DYNINTEGER, emptyArr));
	values.push_back(new GCFPVDynArr(LPT_DYNSTRING, emptyArr));
	values.push_back(new GCFPVDynArr(LPT_DYNSTRING, emptyArr));
	values.push_back(new GCFPVDynArr(LPT_DYNSTRING, emptyArr));
	values.push_back(new GCFPVDynArr(LPT_DYNBOOL, emptyArr));
	values.push_back(new GCFPVDynArr(LPT_DYNDOUBLE, emptyArr));
	values.push_back(new GCFPVDynArr(LPT_DYNDOUBLE, emptyArr));

	LOG_DEBUG_STR("Clearing " << DPname);
	PVSSresult	result = myDPservice->setValue(DPname, fields, values, 0.0, false);
	if (result != SA_NO_ERROR) {
		LOG_WARN_STR("Call to PVSS for setValue for " << DPname << " returned: " << result);
	}
	// free allocated GCFValues.
	for (int i = values.size()-1 ; i >= 0; i--) {
		delete values[i];
	}
	values.clear();
	fields.clear();


	// CS<000><xBAy>_CobaltStationInput
	propSetNameMask = createPropertySetName(PSN_COBALT_STATION_INPUT, getName(), DPbasename);
	// LOFAR_PermSW_@stationfield@_CobaltStationInput	--> @stationfield@ := %s
	// prepare 'cleared value set'
	fields.push_back(PN_CSI_NODE);
	fields.push_back(PN_CSI_CPU);
	fields.push_back(PN_CSI_OBSERVATION_NAME);
	fields.push_back(PN_CSI_STREAM0_BLOCKS_IN);
	fields.push_back(PN_CSI_STREAM0_REJECTED);
	fields.push_back(PN_CSI_STREAM1_BLOCKS_IN);
	fields.push_back(PN_CSI_STREAM1_REJECTED);
	fields.push_back(PN_CSI_STREAM2_BLOCKS_IN);
	fields.push_back(PN_CSI_STREAM2_REJECTED);
	fields.push_back(PN_CSI_STREAM3_BLOCKS_IN);
	fields.push_back(PN_CSI_STREAM3_REJECTED);
	values.push_back(new GCFPVString(""));
	values.push_back(new GCFPVInteger(0));
	values.push_back(new GCFPVString(""));
	for (int i = 0; i < 8; ++i) {
		values.push_back(new GCFPVInteger(0));
	}
	const string	AntFields[] = {"LBA", "HBA", "HBA0", "HBA1" };
	string			ObsLocation(globalParameterSet()->fullModuleName("Observation"));
	vector<string>	stationList(globalParameterSet()->getStringVector(ObsLocation+".VirtualInstrument.stationList"));
	int firstAF   = (globalParameterSet()->getString(ObsLocation+".antennaArray") == "LBA") ? 0 : 1;
	vector<string>::const_iterator	iter = stationList.begin();
	vector<string>::const_iterator	end  = stationList.end();
	while (iter != end) {
		int	nrAFs2Clean = 1 + ((firstAF>0 && iter->substr(0,2)=="CS") ? 2 : 0);
		for (int AFindex = firstAF; AFindex < firstAF+nrAFs2Clean; ++AFindex) {
			string	stationField(*iter + AntFields[AFindex]);	// eg. CS001 + LBA
			string	DPname(formatString(propSetNameMask.c_str(), stationField.c_str()));
			LOG_DEBUG_STR("Clearing " << DPname);

			PVSSresult	result = myDPservice->setValue(DPname, fields, values, 0.0, false);
			if (result != SA_NO_ERROR) {
				LOG_WARN_STR("Call to PVSS for setValue for " << DPname << " returned: " << result);
			}
		} // for
		++iter;
	}
	// free allocated GCFValues.
	for (int i = values.size()-1 ; i >= 0; i--) {
		delete values[i];
	}
	values.clear();
	fields.clear();

	delete myDPservice;
}

// -------------------- Application-order administration --------------------

//
// _handleDisconnect(port)
//
void OnlineControl::_handleDisconnect(GCFPortInterface& port)
{
	port.close();
}

}; // CEPCU
}; // LOFAR
