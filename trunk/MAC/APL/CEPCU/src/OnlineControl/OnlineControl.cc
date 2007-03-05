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
#include <boost/shared_array.hpp>
#include <Common/StreamUtil.h>
//#include <Common/lofar_vector.h>
//#include <Common/lofar_string.h>
#include <APS/ParameterSet.h>
#include <APS/Exceptions.h>
#include <GCF/GCF_PVTypes.h>
#include <GCF/PAL/GCF_PVSSInfo.h>
#include <GCF/Utils.h>
#include <GCF/GCF_ServiceInfo.h>
#include <GCF/TM/GCF_Protocols.h>
#include <GCF/Protocols/PA_Protocol.ph>
#include <APL/APLCommon/APL_Defines.h>
#include <APL/APLCommon/APLUtilities.h>
#include <APL/APLCommon/APLCommonExceptions.h>
#include <APL/APLCommon/Controller_Protocol.ph>
#include <APL/APLCommon/StationInfo.h>
#include <APL/APLCommon/APLUtilities.h>
#include <APL/APLCommon/CTState.h>

#include "OnlineControl.h"
#include "OnlineControlDefines.h"

using namespace LOFAR::GCF::Common;
using namespace LOFAR::GCF::TM;
using namespace LOFAR::GCF::PAL;
using namespace std;

namespace LOFAR {
	using namespace APLCommon;
	using namespace ACC::APS;
    using namespace ACC::ALC;
	namespace CEPCU {
	
// static pointer to this object for signal handler
static OnlineControl*	thisOnlineControl = 0;

//
// OnlineControl()
//
OnlineControl::OnlineControl(const string&	cntlrName) :
	GCFTask 			((State)&OnlineControl::initial_state,cntlrName),
	PropertySetAnswerHandlerInterface(),
	itsPropertySetAnswer(*this),
	itsPropertySet		(),
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
	GCF::TM::registerProtocol (CONTROLLER_PROTOCOL, CONTROLLER_PROTOCOL_STRINGS);
	GCF::TM::registerProtocol (PA_PROTOCOL, 		PA_PROTOCOL_STRINGS);

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
		itsPropertySet->setValue(PVSSNAME_FSM_STATE, GCFPVString(cts.name(newState)));
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
		itsNrOfAcks2Recv = itsCEPapplications.size();
	}
	else {
		// The applications depend on each other send command to first application.
		CAMiter	iter = firstApplication(newState);
		iter->second->sendCommand(newState, options);
		itsOverallResult = 0;
		itsNrOfAcks2Recv = 1;
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
	//	ERROR	 J			send Error to Parent, reset sequence.   [B]
	//	OK		 N			decr nrOfAcks2Recv if 0 inform parent.  [C]
	//	ERROR	 N			decr nrOfAcks2Recv if 0 inform parent.	[D]

	if (!itsUseApplOrder) {		// [C],[D]
		itsOverallResult |= result;
		if (--itsNrOfAcks2Recv <= 0) {
			LOG_DEBUG("All results received, informing parent");
			sendControlResult(*itsParentPort, cts.signal(itsState), 
															getName(), itsOverallResult);
		}
		if (aState == CTState::QUIT) {
			finish();
		}
		return;
	}
			
	if (result == CT_RESULT_NO_ERROR) {	// [B]
		if (hasNextApplication()) { // [A]
			CAMiter		nextApp = nextApplication();
			LOG_DEBUG_STR("Sending " << cts.name(itsState) << " to next application: "
							<< nextApp->second->getName());
			nextApp->second->sendCommand(itsState, "" /*options*/);
			return;
		}
	}

	// [A],[B]
	itsOverallResult = result;
	sendControlResult(*itsParentPort, cts.signal(itsState), getName(), itsOverallResult);
	itsNrOfAcks2Recv = 0;
	noApplication();		// reset order-sequence

	if (aState == CTState::QUIT) {
		finish();
	}
}


//
// handlePropertySetAnswer(answer)
//
void OnlineControl::handlePropertySetAnswer(GCFEvent& answer)
{
	LOG_DEBUG_STR ("handlePropertySetAnswer:" << evtstr(answer));

	switch(answer.signal) {
	case F_MYPS_ENABLED: {
		GCFPropSetAnswerEvent* pPropAnswer=static_cast<GCFPropSetAnswerEvent*>(&answer);
		if(pPropAnswer->result != GCF_NO_ERROR) {
			LOG_ERROR(formatString("%s : PropertySet %s NOT ENABLED",
										getName().c_str(), pPropAnswer->pScope));
		}
		// always let timer expire so main task will continue.
		itsTimerPort->setTimer(1.0);
		break;
	}

	case F_PS_CONFIGURED: {
		GCFConfAnswerEvent* pConfAnswer=static_cast<GCFConfAnswerEvent*>(&answer);
		if(pConfAnswer->result == GCF_NO_ERROR) {
			LOG_DEBUG(formatString("%s : apc %s Loaded",
										getName().c_str(), pConfAnswer->pApcName));
			//apcLoaded();
		}
		else {
			LOG_ERROR(formatString("%s : apc %s NOT LOADED",
										getName().c_str(), pConfAnswer->pApcName));
		}
		break;
	}

	case F_VGETRESP:
	case F_VCHANGEMSG: {
		// check which property changed
		// GCFPropValueEvent* pPropAnswer=static_cast<GCFPropValueEvent*>(&answer);

		// TODO: implement something usefull.
		break;
	}  

//	case F_SUBSCRIBED:
//	case F_UNSUBSCRIBED:
//	case F_PS_CONFIGURED:
//	case F_EXTPS_LOADED:
//	case F_EXTPS_UNLOADED:
//	case F_MYPS_ENABLED:
//	case F_MYPS_DISABLED:
//	case F_VGETRESP:
//	case F_VCHANGEMSG:
//	case F_SERVER_GONE:

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
		string	propSetName = formatString(ONC_PROPSET_NAME, itsInstanceNr);
		LOG_DEBUG_STR ("Activating PropertySet: "<< propSetName);
		itsPropertySet = GCFMyPropertySetPtr(new GCFMyPropertySet(propSetName.c_str(),
																  ONC_PROPSET_TYPE,
																  PS_CAT_TEMPORARY,
																  &itsPropertySetAnswer));
		itsPropertySet->enable();
		// Wait for timer that is set in PropertySetAnswer on ENABLED event
		}
		break;

	case F_TIMER:
		if (!itsPropertySetInitialized) {
			itsPropertySetInitialized = true;
	
			// First redirect signalHandler to our finishing state to leave PVSS
			// in the right state when we are going down
			thisOnlineControl = this;
			signal (SIGINT,  OnlineControl::signalHandler);	// ctrl-c
			signal (SIGTERM, OnlineControl::signalHandler);	// kill

			// update PVSS.
			LOG_TRACE_FLOW ("Updateing state to PVSS");
			itsPropertySet->setValue(string(PVSSNAME_FSM_STATE),GCFPVString("initial"));
			itsPropertySet->setValue(string(PVSSNAME_FSM_ERROR),GCFPVString(""));
		  
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
		itsPropertySet->setValue(string(PVSSNAME_FSM_STATE),GCFPVString("active"));
		itsPropertySet->setValue(string(PVSSNAME_FSM_ERROR),GCFPVString(""));
		break;
	}

	case F_INIT:
		break;

	case F_ACCEPT_REQ:
		break;

	case F_CONNECTED: {
		ASSERTSTR (&port == itsParentPort, "F_CONNECTED event from port " << port.getName());
		break;
	}

	case F_DISCONNECTED: {
		port.close();
		break;
	}

	case F_TIMER: 
//		GCFTimerEvent& timerEvent=static_cast<GCFTimerEvent&>(event);
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
		startNewState(CTState::RESUME, ""/*options*/);
		break;
	}

	case CONTROL_SUSPEND: {
		CONTROLSuspendEvent		msg(event);
		LOG_DEBUG_STR("Received SUSPEND(" << msg.cntlrName << ")");
		startNewState(CTState::SUSPEND, ""/*options*/);
		break;
	}

	case CONTROL_RELEASE: {
		CONTROLReleaseEvent		msg(event);
		LOG_DEBUG_STR("Received RELEASE(" << msg.cntlrName << ")");
		_setState(CTState::RELEASE);
		sendControlResult(*itsParentPort, event.signal, getName(), 0);
		_setState(CTState::RELEASED);
		break;
	}

	case CONTROL_QUIT: {
		CONTROLQuitEvent		msg(event);
		LOG_DEBUG_STR("Received QUIT(" << msg.cntlrName << ")");
		startNewState(CTState::QUIT, ""/*options*/);
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
		itsPropertySet->setValue(PVSSNAME_FSM_STATE,GCFPVString("finished"));
		itsPropertySet->setValue(PVSSNAME_FSM_ERROR,GCFPVString(""));

		itsTimerPort->setTimer(1.0);
		break;
	}

	case F_TIMER:
		GCFTask::stop();
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
		// Start an CEPApplManager for this application
		uint16			result    (CT_RESULT_NO_ERROR);
		string 			applName  (applList[a]);
		string			applPrefix(applName+".");
		LOG_INFO_STR("Starting controller for " << applName);
		CEPApplMgrPtr	accClient (new CEPApplMgr(*this, applName));
		itsCEPapplications[applName] = accClient;

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
			params.writeFile(paramFileName);
			// TODO: waar komt de hostname vandaan???
//			string hostname(thePS->getString(xxx+"_hostname"));
//			APLCommon::APLUtilities::remoteCopy(paramFileName,hostName,LOFAR_SHARE_LOCATION);
		} 
		catch (APSException &e) {
			// key not found. skip
			LOG_FATAL(e.text());
			result = CT_RESULT_UNSPECIFIED;
			appSetStateResult(applList[a], CTState::CONNECT, result);
		}
	} // for

	// Finally send the boot command.
	startNewState(CTState::CONNECT, paramFileName);

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

	// every application must be in the order list.
	ASSERTSTR(itsApplOrder.size() == itsCEPapplications.size(), 
				"Application orderlist conflicts with length of applicationlist");

	// check that all application exist 
	CAMiter						applEnd   = itsCEPapplications.end();
	vector<string>::iterator	orderIter = itsApplOrder.begin();
	while (orderIter != itsApplOrder.end()) {
		CAMiter		applIter = itsCEPapplications.begin();
		while (applIter != applEnd) {
			if (applIter->second->getName() == *orderIter) {
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
		LOG_ERROR_STR("Starting new command-chain while previous command-chain was still at appplication " 
			<< itsCurrentAppl << ". Results are unpredictable!");
	}

	itsApplState = newState;
	switch (newState) {
	case CTState::CONNECT:
	case CTState::CLAIM:
	case CTState::PREPARE:
	case CTState::RESUME:
		itsCurrentAppl = itsCEPapplications.begin()->second->getName();
		break;

	case CTState::SUSPEND:
	case CTState::RELEASE:
	case CTState::QUIT:
		itsCurrentAppl = itsCEPapplications.end()->second->getName();
		break;

	default:		// satisfy compiler
		break;
	}
	CTState		cts;
	ASSERTSTR(false, "Illegal new state in firstApplication(): " 
														<< cts.name(newState));	
}


//
// nextApplication()
//
OnlineControl::CAMiter OnlineControl::nextApplication()
{
	ASSERTSTR (hasNextApplication(), "Programming error, must have next application");

	// search current application in the list.
	CAMiter		iter = itsCEPapplications.begin();
	while (iter != itsCEPapplications.end()) {
		if (iter->second->getName() == itsCurrentAppl) {
			break;
		}
		iter++;
	}
	ASSERTSTR (iter != itsCEPapplications.end(), "Application " << itsCurrentAppl 
												<< "not found in applicationList");

	switch (itsApplState) {
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

	itsCurrentAppl = iter->second->getName();
	return (iter);
}


//
// noApplication()
//
void OnlineControl::noApplication()
{
	itsCurrentAppl = "";
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
	case CTState::CLAIM:
	case CTState::PREPARE:
	case CTState::RESUME:
		return (itsCurrentAppl != itsCEPapplications.rbegin()->second->getName());
		break;

	case CTState::SUSPEND:
	case CTState::RELEASE:
	case CTState::QUIT:
		return (itsCurrentAppl != itsCEPapplications.begin()->second->getName());
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
