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
#include <Common/LofarLogger.h>

#include <signal.h>
#include <Common/StreamUtil.h>
//#include <Common/lofar_vector.h>
//#include <Common/lofar_string.h>
#include <Common/ParameterSet.h>
#include <Common/Exceptions.h>
#include <ApplCommon/StationInfo.h>
#include <GCF/PVSS/GCF_PVTypes.h>
#include <Common/SystemUtil.h>
#include <MACIO/MACServiceInfo.h>
#include <GCF/TM/GCF_Protocols.h>
#include <APL/APLCommon/APL_Defines.h>
#include <APL/APLCommon/APLUtilities.h>
#include <APL/APLCommon/Controller_Protocol.ph>
#include <APL/APLCommon/APLUtilities.h>
#include <APL/APLCommon/CTState.h>
#include <GCF/RTDB/DP_Protocol.ph>
#include <PLC/PCCmd.h>

#include "OnlineControl.h"
#include "PVSSDatapointDefs.h"

using namespace LOFAR::GCF::PVSS;
using namespace LOFAR::GCF::TM;
using namespace LOFAR::GCF::RTDB;
using namespace std;

namespace LOFAR {
	using namespace APLCommon;
    using namespace ACC::ALC;
	namespace CEPCU {
	
// static pointer to this object for signal handler
static OnlineControl*	thisOnlineControl = 0;

//
// OnlineControl()
//
OnlineControl::OnlineControl(const string&	cntlrName) :
	GCFTask 			((State)&OnlineControl::initial_state,cntlrName),
	itsPropertySet		(0),
	itsPropertySetInitialized (false),
	itsParentControl	(0),
	itsParentPort		(0),
	itsTimerPort		(0),
    itsCEPapplications  (),
	itsResultParams     (),
	itsState			(CTState::NOSTATE),
	itsUseApplOrder		(false),
	itsApplOrder		(),
	itsCurrentAppl		(),
	itsApplState		(CTState::NOSTATE),
	itsOverallResult	(0),
	itsNrOfAcks2Recv	(0),
	itsTreePrefix       (""),
	itsInstanceNr       (0),
	itsStartTime        (),
	itsStopTime         ()
//	itsStopTimerID      (0)
{
	LOG_TRACE_OBJ_STR (cntlrName << " construction");

	// First readin our observation related config file.
	LOG_DEBUG_STR("Reading parset file:" << LOFAR_SHARE_LOCATION << "/" << cntlrName);
	globalParameterSet()->adoptFile(string(LOFAR_SHARE_LOCATION)+"/"+cntlrName);

	// Readin some parameters from the ParameterSet.
	itsTreePrefix = globalParameterSet()->getString("prefix");
	itsInstanceNr = globalParameterSet()->getUint32("_instanceNr");

	// get Observation based information
// REO do we need those????
	itsStartTime     = time_from_string(globalParameterSet()->
											 getString("Observation.startTime"));
	itsStopTime      = time_from_string(globalParameterSet()->
											 getString("Observation.stopTime"));

	// attach to parent control task
	itsParentControl = ParentControl::instance();
	itsParentPort = new GCFITCPort (*this, *itsParentControl, "ParentITCport", 
									GCFPortInterface::SAP, CONTROLLER_PROTOCOL);
	ASSERTSTR(itsParentPort, "Cannot allocate ITCport for Parentcontrol");
	itsParentPort->open();		// will result in F_CONNECTED

	// need port for timers.
	itsTimerPort = new GCFTimerPort(*this, "TimerPort");

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
}

//
// signalHandler(signum)
//
void OnlineControl::signalHandler(int	signum)
{
	LOG_DEBUG (formatString("SIGNAL %d detected", signum));

	if (thisOnlineControl) {
		thisOnlineControl->finish();
	}
}

//
// finish()
//
void OnlineControl::finish()
{
	TRAN(OnlineControl::finishing_state);
}


//
// _setState(CTstateNr)
//
void    OnlineControl::_setState(CTState::CTstateNr     newState)
{
	CTState		cts;
	LOG_DEBUG_STR ("Going from state " << cts.name(itsState) << " to " 
										<< cts.name(newState));
	itsState = newState;

	// Update PVSS to inform operator.
	if (itsPropertySet) {
		CTState		cts;
		itsPropertySet->setValue(PN_FSM_CURRENT_ACTION, GCFPVString(cts.name(newState)));
	}
}   


//
// startNewState(newState)
//
void	OnlineControl::startNewState(CTState::CTstateNr		newState,
									 const string&			options)
{
	// TODO: check if previous state has ended?

	_setState (newState);

	if (!itsUseApplOrder) { 		// no depencies between applications?
		for (CAMiter iter = itsCEPapplications.begin(); 
										iter != itsCEPapplications.end(); ++iter) {
			iter->second->sendCommand(newState, options);
		}
		itsOverallResult = 0;
		itsOptions.clear();
		itsNrOfAcks2Recv = itsCEPapplications.size();
	}
	else {
		// The applications depend on each other send command to first application.
		CAMiter	iter = firstApplication(newState);
		iter->second->sendCommand(newState, options);
		itsOverallResult = 0;
		itsNrOfAcks2Recv = 1;
		itsOptions = options;
	}

	// TODO: start timer???
}


//
// appSetStateResult(procName, newState, result)
//
// A result of a new state was received. Update our admin with this result and
// inform parentController is all Applications have reached the newState now.
// When the applications are dependant of each order send the same command to 
// the next application.
//
// note: function is called by CEPApplMgr
//
void	OnlineControl::appSetStateResult(const string&			procName, 
										 CTState::CTstateNr		aState,
										 uint16					result)
{
	CTState		cts;
	LOG_DEBUG_STR("setStateResult(" << procName <<","<< cts.name(aState) 
												<<","<< result <<")");

	// is the result in sync?
	if (aState != itsState) {
		LOG_ERROR_STR("Application " << procName << " reports result " << result
			<< " for state " << cts.name(aState) << " while the current state is "
			<< cts.name(itsState) << ". Ignoring result!");
		return;
	}

	if (itsNrOfAcks2Recv <= 0) {
		LOG_INFO_STR("Application " << procName << " reports result " << result
			<< " for state " << cts.name(aState)
			<< " after parentController was informed. Result will be unknown to Parent.");
		return;
	}

	// result	useOrder	action
	//  OK		 J			if nextAppl sendCmd else inform parent. [A]
	//	ERROR	 J			if in start sequence: send Error to Parent, reset sequence.  [B1]
	//	ERROR	 J		    if in stop sequence:  if nextAppl sendCmd else inform parent [B2]
	//	OK		 N			decr nrOfAcks2Recv if 0 inform parent.  [C]
	//	ERROR	 N			decr nrOfAcks2Recv if 0 inform parent.	[D]

	itsOverallResult |= result;

	if (!itsUseApplOrder) {		// [C],[D]
		if (--itsNrOfAcks2Recv <= 0) {
			LOG_DEBUG("All results received, informing parent");
			sendControlResult(*itsParentPort, cts.signal(itsState), 
															getName(), itsOverallResult);
			if (aState == CTState::QUIT) {
				finish();
			}
		}
		return;
	}
	// [A],[B] not handled yet.	
		
	if ((result == CT_RESULT_NO_ERROR) || (itsState >= CTState::SUSPEND)) {	// [A],[B2]
		if (hasNextApplication()) {
			CAMiter		nextApp = nextApplication();
			LOG_DEBUG_STR("Sending " << cts.name(itsState) << " to next application: "
							<< nextApp->second->getName());
			nextApp->second->sendCommand(itsState, itsOptions);
			return;
		}
	}

	// no more application for [A]or[B2], or error in [B1]
	sendControlResult(*itsParentPort, cts.signal(itsState), getName(), itsOverallResult);
	itsNrOfAcks2Recv = 0;
	noApplication();		// reset order-sequence

	if (aState == CTState::QUIT) {
		finish();
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
// Setup all connections.
//
GCFEvent::TResult OnlineControl::initial_state(GCFEvent& event, 
													GCFPortInterface& port)
{
	LOG_DEBUG_STR ("initial:" << eventName(event) << "@" << port.getName());

	GCFEvent::TResult status = GCFEvent::HANDLED;
  
	switch (event.signal) {
	case F_ENTRY:
   		break;

    case F_INIT: {
		// Get access to my own propertyset.
//		uint32	obsID = globalParameterSet()->getUint32("Observation.ObsID");
		string	propSetName(createPropertySetName(PSN_ONLINE_CONTROL, getName()));
		LOG_DEBUG_STR ("Activating PropertySet: "<< propSetName);
		itsPropertySet = new RTDBPropertySet(propSetName,
											 PST_ONLINE_CONTROL,
											 PSAT_RW,
											 this);
		}
		break;

	case DP_CREATED: {
		// NOTE: thsi function may be called DURING the construction of the PropertySet.
		// Always exit this event in a way that GCF can end the construction.
		DPCreatedEvent  dpEvent(event);
		LOG_DEBUG_STR("Result of creating " << dpEvent.DPname << " = " << dpEvent.result);
		itsTimerPort->cancelAllTimers();
		itsTimerPort->setTimer(0.0);
		}
		break;

	case F_TIMER: {	// must be timer that PropSet is online.
		// update PVSS.
		LOG_TRACE_FLOW ("Updateing state to PVSS");
		itsPropertySet->setValue(PN_FSM_CURRENT_ACTION, GCFPVString("initial"));
		itsPropertySet->setValue(PN_FSM_ERROR, GCFPVString(""));
	  
		// Start ParentControl task
		LOG_DEBUG ("Enabling ParentControl task");
		itsParentPort = itsParentControl->registerTask(this);
		// results in CONTROL_CONNECT

		LOG_DEBUG ("Going to operational state");
		TRAN(OnlineControl::active_state);				// go to next state.
		}
		break;

	case F_CONNECTED:
		ASSERTSTR (&port == itsParentPort, 
									"F_CONNECTED event from port " << port.getName());
		break;

	case F_DISCONNECTED:
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
	LOG_DEBUG_STR ("active:" << eventName(event) << "@" << port.getName());

	GCFEvent::TResult status = GCFEvent::HANDLED;

	switch (event.signal) {
	case F_ENTRY: {
		// update PVSS
		itsPropertySet->setValue(PN_FSM_CURRENT_ACTION, GCFPVString("active"));
		itsPropertySet->setValue(PN_FSM_ERROR, GCFPVString(""));
	}
	break;

	case F_ACCEPT_REQ:
		break;

	case F_CONNECTED: {
		ASSERTSTR (&port == itsParentPort, "F_CONNECTED event from port " << port.getName());
	}
	break;

	case F_DISCONNECTED: {
		port.close();
	}
	break;

	case DP_CHANGED:
		_databaseEventHandler(event);
		break;

	case F_TIMER:  {
//		GCFTimerEvent& timerEvent=static_cast<GCFTimerEvent&>(event);
//		if (timerEvent.id == itsStopTimerID) {
//			LOG_DEBUG("StopTimer expired, starting QUIT sequence");
//			startNewState(CTState::QUIT, ""/*options*/);
//		}
//		else {
//			LOG_WARN_STR("Received unknown timer event");
//		}
	}
	break;

	// -------------------- EVENTS RECEIVED FROM PARENT CONTROL --------------------
	case CONTROL_CONNECT: {
		CONTROLConnectEvent		msg(event);
		LOG_DEBUG_STR("Received CONNECT(" << msg.cntlrName << ")");
		_setState(CTState::CONNECT);
		_doBoot();			// start ACC's and boot them
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
		startNewState(CTState::CLAIM, ""/*options*/);
		break;
	}

	case CONTROL_PREPARE: {
		CONTROLPrepareEvent		msg(event);
		LOG_DEBUG_STR("Received PREPARE(" << msg.cntlrName << ")");
		startNewState(CTState::PREPARE, ""/*options*/);
		break;
	}

	case CONTROL_RESUME: {
		CONTROLResumeEvent		msg(event);
		LOG_DEBUG_STR("Received RESUME(" << msg.cntlrName << ")");
		itsStartTime = second_clock::universal_time();	// adjust to latest run.
		startNewState(CTState::RESUME, ""/*options*/);
//		LOG_DEBUG_STR("Starttime set to " << to_simple_string(itsStartTime));
		break;
	}

	case CONTROL_SUSPEND: {
		CONTROLSuspendEvent		msg(event);
		LOG_DEBUG_STR("Received SUSPEND(" << msg.cntlrName << ")");
		startNewState(CTState::SUSPEND, PAUSE_OPTION_NOW);
		break;
	}

	case CONTROL_RELEASE: {
		CONTROLReleaseEvent		msg(event);
		LOG_DEBUG_STR("Received RELEASE(" << msg.cntlrName << ")");
		startNewState(CTState::RELEASE, ""/*options*/);
		break;
	}

	case CONTROL_QUIT: {
		CONTROLQuitEvent		msg(event);
		LOG_DEBUG_STR("Received QUIT(" << msg.cntlrName << ")");
//		ptime	now(second_clock::universal_time());
//		LOG_DEBUG_STR("now set to " << to_simple_string(now));
//		LOG_DEBUG_STR("period is " << time_duration(now - itsStartTime).total_seconds());
//		uint32	waitTime = 16 - (time_duration(now - itsStartTime).total_seconds()%16);
//		if (waitTime == 16) {	// precisely on a multiple of 16! Stop immediately
			startNewState(CTState::QUIT, ""/*options*/);
//		}
//		else {
//			// wait for multiple of 16 seconds.
//			itsStopTimerID = itsTimerPort->setTimer(waitTime * 1.0);
//			LOG_INFO_STR("Delaying quit command for " << waitTime << " seconds to sync on multiple of 16");
//		}
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
	LOG_DEBUG_STR ("finishing:" << eventName(event) << "@" << port.getName());

	GCFEvent::TResult status = GCFEvent::HANDLED;

	switch (event.signal) {
	case F_ENTRY: {
		// update PVSS
		itsPropertySet->setValue(PN_FSM_CURRENT_ACTION, GCFPVString("finished"));
		itsPropertySet->setValue(PN_FSM_ERROR, GCFPVString(""));

		itsTimerPort->setTimer(1.0);
		break;
	}

	case F_TIMER:
		GCFScheduler::instance()->stop();
		break;

	default:
		LOG_DEBUG("finishing_state, default");
		status = GCFEvent::NOT_HANDLED;
		break;
	}

	return (status);
}


//
// _doBoot()
//
// Create ParameterSets for all Applications the we have to manage, start all
// ACC's and give them the boot command.
//
void OnlineControl::_doBoot()
{
	ParameterSet*	thePS  = globalParameterSet();		// shortcut to global PS.

	// Get list of all application that should be managed
	// Note: each application = 1 ACC
	vector<string> applList = thePS->getStringVector("applications");
	string 	paramFileName;

	for (size_t a = 0; a < applList.size(); a++) {
		// Initialize basic variables
		uint16	result    (CT_RESULT_NO_ERROR);
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

			// always add Observation
			string	obsPrefix(thePS->locateModule("Observation"));
			params.adoptCollection(thePS->makeSubset(obsPrefix+"Observation", "Observation"));
			// write parset to file.
			paramFileName = formatString("%s/ACC_%s_%s.param", LOFAR_SHARE_LOCATION,
											  getName().c_str(), applName.c_str());
			params.writeFile(paramFileName); 	// local copy
			string	accHost(thePS->getString(applPrefix+"_hostname"));
			LOG_DEBUG_STR("Controller for " << applName << " wil be running on " << accHost);
			APLCommon::APLUtilities::remoteCopy(paramFileName,accHost,LOFAR_SHARE_LOCATION);

			// Finally start ApplController on the right host
			LOG_INFO_STR("Starting controller for " << applName << " in 3 seconds ");
			sleep(3);			 // sometimes we are too quick, wait a second.
			int32	expectedRuntime = time_duration(itsStopTime - itsStartTime).total_seconds();
			uint32	obsID = globalParameterSet()->getUint32("Observation.ObsID");
			CEPApplMgrPtr	accClient (new CEPApplMgr(*this, formatString("%s%d", applName.c_str(), obsID),
													  expectedRuntime, accHost, paramFileName));
			itsCEPapplications[applName] = accClient;
		} 
		catch (APSException &e) {
			// key not found. skip
			LOG_FATAL(e.text());
			result = CT_RESULT_UNSPECIFIED;
			appSetStateResult(applList[a], CTState::CONNECT, result);
		}
	} // for

	// finally setup application Order
	vector<string>	anApplOrder(thePS->getStringVector("applOrder"));
	if (!anApplOrder.empty()) {
		setApplOrder(anApplOrder);
	}

	// Finally send the boot command.
	startNewState(CTState::CONNECT, "");

}


//
// _doQuit()
//
void OnlineControl::_doQuit(void)
{
	try {
#if 0
		for(size_t i = 0;i < itsCepAppParams.size();i++) {
			string remoteFile, resultFile, applName;
			applName = itsCepAppParams[i].getString("ApplCtrl.application");
			resultFile = formatString("ACC-%s_result.param", applName.c_str());
			remoteFile = string(LOFAR_SHARE_LOCATION) + string("/") + resultFile;
//			APLCommon::APLUtilities::copyFromRemote(hostName,remoteFile,resultFile);
			itsResultParams.adoptFile(resultFile);
			//  itsResultParams.replace(KVpair(formatString("%s.quality", getName().c_str()), (int) _qualityGuard.getQuality()));
			if (!itsResultParams.isDefined(formatString("%s.faultyNodes", getName().c_str()))) {
				itsResultParams.add(formatString("%s.faultyNodes", getName().c_str()), "");
			}
			itsResultParams.writeFile(formatString("%s_result.param", getName().c_str()));
		}
#endif
	}
	catch(...) {
	}
	map<string, CEPApplMgrPtr>::iterator it;
	for(it = itsCEPapplications.begin();it != itsCEPapplications.end();++it) {
		it->second->quit(0);
	}
}

// -------------------- Application-order administration --------------------

//
// setApplOrder(appl-vector)
//
void OnlineControl::setApplOrder(vector<string>&	anApplOrder)
{
	itsUseApplOrder = true;			// assume everything is right.
	itsApplOrder	= anApplOrder;

	LOG_DEBUG_STR("setApplOrder: Checking " << itsApplOrder);

	// every application must be in the order list.
	ASSERTSTR(itsApplOrder.size() == itsCEPapplications.size(), 
				"Application orderlist conflicts with length of applicationlist");

	// check that all applications exist 
	CAMiter						applEnd   = itsCEPapplications.end();
	vector<string>::iterator	orderIter = itsApplOrder.begin();
	while (orderIter != itsApplOrder.end()) {
		CAMiter		applIter = itsCEPapplications.begin();
		while (applIter != applEnd) {
			LOG_DEBUG_STR("compare: " << applIter->first << " with " << *orderIter);
			if (applIter->first == *orderIter) {
				break;
			}
			applIter++;
		}
		ASSERTSTR(applIter != applEnd,  *orderIter << 
							" is not a registered application, orderlist is illegal");
		orderIter++;
	}
	LOG_INFO_STR ("Using application order: " << itsApplOrder);
}


//
// firstApplication(newState)
//
OnlineControl::CAMiter OnlineControl::firstApplication(CTState::CTstateNr	newState)
{
	if (itsCurrentAppl !=  "") {
		LOG_ERROR_STR("Starting new command-chain while previous command-chain was still at application " 
			<< itsCurrentAppl << ". Results are unpredictable!");
	}

	itsApplState = newState;
	vector<string>::iterator		newApplIter;
	switch (newState) {
	case CTState::CONNECT:
	case CTState::CLAIM:
	case CTState::PREPARE:
	case CTState::RESUME:
		newApplIter = itsApplOrder.begin();
		break;

	case CTState::SUSPEND:
	case CTState::RELEASE:
	case CTState::QUIT:
		newApplIter = itsApplOrder.end();
		newApplIter--;
		break;

	default:		// satisfy compiler
		CTState		cts;
		ASSERTSTR(false, "Illegal new state in firstApplication(): " 
														<< cts.name(newState));	
		break;
	}

	itsCurrentAppl = *newApplIter;
	LOG_DEBUG_STR("First application is " << itsCurrentAppl);
	return (itsCEPapplications.find(itsCurrentAppl));
}


//
// nextApplication()
//
OnlineControl::CAMiter OnlineControl::nextApplication()
{
	ASSERTSTR (hasNextApplication(), "Programming error, must use application ordering");

	// search current application in the list.
	vector<string>::iterator		iter = itsApplOrder.begin();
	while (iter != itsApplOrder.end()) {
		if (*iter == itsCurrentAppl) {
			break;
		}
		iter++;
	}
	ASSERTSTR (iter != itsApplOrder.end(), "Application " << itsCurrentAppl 
												<< "not found in applicationList");

	switch (itsApplState) {
	case CTState::CONNECT:
	case CTState::CLAIM:
	case CTState::PREPARE:
	case CTState::RESUME:
		iter++;
		break;

	case CTState::SUSPEND:
	case CTState::RELEASE:
	case CTState::QUIT:
		iter--;
		break;

	default:
		ASSERT("Satisfy compiler");
	}

	itsCurrentAppl = *iter;
	LOG_DEBUG_STR("Next application is " << itsCurrentAppl);
	return (itsCEPapplications.find(itsCurrentAppl));
}


//
// noApplication()
//
void OnlineControl::noApplication()
{
	itsCurrentAppl = "";
	itsOptions.clear();
}


//
// hasNextApplication()
//
bool OnlineControl::hasNextApplication()
{
	if (!itsUseApplOrder) {
		return (false);
	}

	switch (itsApplState) {
	case CTState::CONNECT:
	case CTState::CLAIM:
	case CTState::PREPARE:
	case CTState::RESUME:
		return (itsCurrentAppl != *(itsApplOrder.rbegin()));
		break;

	case CTState::SUSPEND:
	case CTState::RELEASE:
	case CTState::QUIT:
		return (itsCurrentAppl != *(itsApplOrder.begin()));
		break;

	default: {
		CTState		cts;
		ASSERTSTR(false, "Illegal state in hasNextApplication(): " 
															<< cts.name(itsApplState));
		}
	}

}

//
// _connectedHandler(port)
//
void OnlineControl::_connectedHandler(GCFPortInterface& /*port*/)
{
}

//
// _disconnectedHandler(port)
//
void OnlineControl::_disconnectedHandler(GCFPortInterface& port)
{
	port.close();
}

//
// appSupplyInfo(procName, keyList)
//
// note: function is called by CEPApplMgr
//
string OnlineControl::appSupplyInfo(const string& procName, const string& keyList)
{
	LOG_INFO_STR("appSupplyInfo from " << procName);
	string ret(keyList);
	return ret;
}

//
// appSupplyInfoAnswer(procName, answer)
//
// note: function is called by CEPApplMgr
//
void OnlineControl::appSupplyInfoAnswer(const string& procName, const string& answer)
{
	LOG_INFO_STR("Answer from " << procName << ": " << answer);
}


}; // CEPCU
}; // LOFAR
