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
    itsCepApplications  (),
	itsCepAppParams     (),
	itsResultParams     (),
	itsState			(CTState::NOSTATE),
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
	GCF::TM::registerProtocol (PA_PROTOCOL, 		   PA_PROTOCOL_STRINGS);

	setState(CTState::CREATED);
}


//
// ~OnlineControl()
//
OnlineControl::~OnlineControl()
{
	LOG_TRACE_OBJ_STR (getName() << " destruction");
}

//
// sigintHandler(signum)
//
void OnlineControl::sigintHandler(int	signum)
{
	LOG_DEBUG (formatString("SIGINT signal detected(%d)", signum));

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
// setState(CTstateNr)
//
void    OnlineControl::setState(CTState::CTstateNr     newState)
{
	itsState = newState;

	if (itsPropertySet) {
		CTState		cts;
		itsPropertySet->setValue(PVSSNAME_FSM_STATE, GCFPVString(cts.name(newState)));
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
			signal (SIGINT,  OnlineControl::sigintHandler);	// ctrl-c
			signal (SIGTERM, OnlineControl::sigintHandler);	// kill

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
		setState(CTState::CONNECTED);
		CONTROLConnectedEvent	answer;
		answer.cntlrName = msg.cntlrName;
		port.send(answer);
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
		setState(CTState::CLAIM);
		CONTROLClaimedEvent             answer;
		answer.cntlrName = getName();
		answer.result    = doClaim(msg.cntlrName);
		if (answer.result == CT_RESULT_NO_ERROR) {
			setState(CTState::CLAIMED);
		}
		port.send(answer);
		
		break;
	}

	case CONTROL_PREPARE: {
		CONTROLPrepareEvent		msg(event);
		LOG_DEBUG_STR("Received PREPARE(" << msg.cntlrName << ")");
		setState(CTState::PREPARE);
		CONTROLPreparedEvent    answer;
		answer.cntlrName = getName();
		answer.result    = doPrepare(msg.cntlrName);
		if (answer.result == CT_RESULT_NO_ERROR) {
			setState(CTState::PREPARED);
		}
		port.send(answer);
		break;
	}

	case CONTROL_RESUME: {
		CONTROLResumeEvent		msg(event);
		LOG_DEBUG_STR("Received RESUME(" << msg.cntlrName << ")");
		setState(CTState::RESUME);
		// TODO: implement something useful
		CONTROLResumedEvent		answer;
		answer.cntlrName = msg.cntlrName;
		port.send(answer);
		break;
	}

	case CONTROL_SUSPEND: {
		CONTROLSuspendEvent		msg(event);
		LOG_DEBUG_STR("Received SUSPEND(" << msg.cntlrName << ")");
		setState(CTState::SUSPENDED);
		// TODO: implement something useful
		CONTROLSuspendedEvent		answer;
		answer.cntlrName = msg.cntlrName;
		port.send(answer);
		break;
	}

	case CONTROL_RELEASE: {
		CONTROLReleaseEvent		msg(event);
		LOG_DEBUG_STR("Received RELEASE(" << msg.cntlrName << ")");
		setState(CTState::RELEASE);
		doRelease();
		setState(CTState::RELEASED);
		CONTROLReleasedEvent	answer;
		answer.cntlrName = msg.cntlrName;
		port.send(answer);
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
// doClaim(cntlrName)
//
// Create ParameterSets for all Applications the controller has to manage.
//
uint16_t OnlineControl::doClaim(const string& cntlrName)
{
	uint16_t 		result = CT_RESULT_NO_ERROR;
	ParameterSet*	thePS  = globalParameterSet();
	try {
		// get prefix of my stuff.
		// string myPrefix(thePS->locateModule("OnlineCtrl") + "OnlineCtrl.");
		string 	myPrefix;

		// Get list of all application that should be managed
		// Note: each application = 1 ACC
		vector<string> applList = thePS->getStringVector(myPrefix+"applications");

		for (size_t a = 0; a < applList.size(); a++) {
			// Allocate an CEPApplManager for each application
			string 			applName(applList[a]);
			string			applPrefix(myPrefix+applName+".");
			CEPApplMgrPtr	accClient(new CEPApplMgr(*this, applName));
			itsCepApplications[applName] = accClient;

			// Create a parameterSet for this AC.
			ParameterSet params;
			params.clear();
			// import and extend the ApplCtrl section
			params.adoptCollection(thePS->makeSubset(myPrefix+"ApplCtrl","ApplCtrl"));
			params.replace("ApplCtrl.application", applName);
			params.replace("ApplCtrl.processes", thePS->getString(applPrefix+"processes"));
			params.replace("ApplCtrl.resultfile", formatString(
										"./ACC-%s_result.param", applName.c_str()));

			// add application info
			params.adoptCollection(thePS->makeSubset(applPrefix,applName+"."));

			// import extra tree part if necc.
			vector<string>	extraParts=thePS->getStringVector(applPrefix+"extraInfo");
			for (size_t e = 0; e < extraParts.size(); e++) {
				if (extraParts[e][0] == '.') {	// relative part?
					string	partName = extraParts[e].substr(1);
					params.adoptCollection(thePS->makeSubset(myPrefix+partName, 
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
//			params.substractSubset(myPrefix);

			// create nodelist
//			int nodeIndex(1);
//			for (vector<string>::iterator it = nodes.begin();it != nodes.end(); ++it) {
//				params.replace(formatString("ApplCtrl.%s[%d].node",procName.c_str(),nodeIndex++),*it);
//			}

			itsCepAppParams.push_back(params);

		} // for applications
	}
	catch(APSException &e) {
		// key not found. skip
		LOG_FATAL(e.text());
		result = CT_RESULT_UNSPECIFIED;
	}
	return result;
}

//
// doPrepare(cntlrName)
//
uint16_t OnlineControl::doPrepare(const string&	cntlrName)
{
	uint16_t result = CT_RESULT_NO_ERROR;

	try {
		// TODO use parameterset of 'cntlrname' when being shared controller
		for (size_t i = 0; i < itsCepAppParams.size(); i++) {
			string applName = itsCepAppParams[i].getString("ApplCtrl.application");
			string paramFileName(formatString("ACC-%s.param", applName.c_str()));
			itsCepAppParams[i].writeFile(paramFileName);

			// REO where do we need all these times for????
			// schedule all ACC commands
			time_t startTime  = to_time_t(itsStartTime);
			time_t initTime   = startTime  - itsCepAppParams[i].getTime("ApplCtrl.timeout_init");
			time_t defineTime = initTime   - itsCepAppParams[i].getTime("ApplCtrl.timeout_define") - 
			itsCepAppParams[i].getTime("ApplCtrl.timeout_startup");
			time_t bootTime   = defineTime - itsCepAppParams[i].getTime("ApplCtrl.timeout_createsubsets");
			time_t now = time(0);
			time_t stopTime = to_time_t(itsStopTime);
			LOG_DEBUG(formatString("%d now %s time %d", now,        ctime(&now), time(0)));
			LOG_DEBUG(formatString("%d boot %s",        bootTime,   ctime(&bootTime)));
			LOG_DEBUG(formatString("%d define %s",      defineTime, ctime(&defineTime)));
			LOG_DEBUG(formatString("%d init %s",        initTime,   ctime(&initTime)));
			LOG_DEBUG(formatString("%d start %s",       startTime,  ctime(&startTime)));
			LOG_DEBUG(formatString("%d stop %s",        stopTime,   ctime(&stopTime)));

			if (now > bootTime) {
//				APLCommon::APLUtilities::remoteCopy(paramFileName,hostName,LOFAR_SHARE_LOCATION);
				LOG_WARN("Cannot guarantee all CEP processes are started in time.");
			}
			else {
				CEPApplMgrPtr cepAppPtr = itsCepApplications[applName];
				if(cepAppPtr) {
					switch (cepAppPtr->getLastOkCmd()) {
					case ACCmdNone:
						cepAppPtr->boot(bootTime, paramFileName);
						break;

					case ACCmdBoot:
						cepAppPtr->define(defineTime);
						break;

					case ACCmdDefine:
					case ACCmdInit:
					case ACCmdRun:
						cepAppPtr->recover(0, "snapshot-DB");
						break;

					default:
						assert(0);
						break;
					}   
				} 
//				APLCommon::APLUtilities::remoteCopy(paramFileName,hostName,LOFAR_SHARE_LOCATION);
			}
		}
	}
	catch(APSException &e) {
		// key not found. skip
		LOG_FATAL(e.text());
		result = CT_RESULT_UNSPECIFIED;
	}

	return (result);
}

//
// doRelease()
//
void OnlineControl::doRelease(void)
{
	try {
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
	}
	catch(...) {
	}
	map<string, CEPApplMgrPtr>::iterator it;
	for(it = itsCepApplications.begin();it != itsCepApplications.end();++it) {
		it->second->quit(0);
	}
}

//
// finishController
//
void OnlineControl::finishController(uint16_t /*result*/)
{
	setState(CTState::RELEASE);
	doRelease();
	setState(CTState::RELEASED);

	LOG_DEBUG ("Going to finishing state");
	TRAN(OnlineControl::finishing_state); // go to next state.
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
// appBooted(procName, result)
//
void OnlineControl::appBooted(const string& procName, uint16 result)
{
	LOG_INFO_STR("appBooted from " << procName);
	if (result == (AcCmdMaskOk | AcCmdMaskScheduled))  {
		time_t startTime  = to_time_t(itsStartTime);
		time_t initTime   = startTime  - itsCepAppParams[0].getTime("ApplCtrl.timeout_init");
		time_t defineTime = initTime   - itsCepAppParams[0].getTime("ApplCtrl.timeout_define") - 
										 itsCepAppParams[0].getTime("ApplCtrl.timeout_startup");
		map<string,CEPApplMgrPtr>::iterator it = itsCepApplications.find(procName);
		if(it != itsCepApplications.end()) {
			it->second->define(defineTime);
		}
	}
	else if (result == 0) { // Error
		LOG_ERROR("Error in ACC. Stops CEP application and releases Online Control.");
		finishController(CT_RESULT_UNSPECIFIED);
	}
}

//
// appDefined(procName, result)
//
void OnlineControl::appDefined(const string& procName, uint16 result)
{
	LOG_INFO_STR("appDefined from " << procName);
	if (result == (AcCmdMaskOk | AcCmdMaskScheduled)) {
		time_t startTime  = to_time_t(itsStartTime);
		time_t initTime   = startTime  - itsCepAppParams[0].getTime("ApplCtrl.timeout_init");

		map<string,CEPApplMgrPtr>::iterator it =  itsCepApplications.find(procName);
		if(it != itsCepApplications.end()) {
			it->second->init(initTime);
		}
	}
	else if (result == 0) { // Error
		LOG_ERROR("Error in ACC. Stops CEP application and releases VB.");
		finishController(CT_RESULT_UNSPECIFIED);
	}
}

//
// appInitialized(procName, result)
//
void OnlineControl::appInitialized(const string& procName, uint16 result)
{
	LOG_INFO_STR("appInitialized from " << procName);
	if (result == AcCmdMaskOk) {    
		//    _doStateTransition(LOGICALDEVICE_STATE_SUSPENDED);
	}
	else if (result == (AcCmdMaskOk | AcCmdMaskScheduled))  {
		map<string,CEPApplMgrPtr>::iterator it =  itsCepApplications.find(procName);
		if(it != itsCepApplications.end()) {
			it->second->run(to_time_t(itsStartTime));
		}
	}
	else if (result == 0) { // Error
		LOG_ERROR("Error in ACC. Stops CEP application and releases VB.");
		finishController(CT_RESULT_UNSPECIFIED);
	}
}

//
// appRunDone(procName, result)
//
void OnlineControl::appRunDone(const string& procName, uint16 result)
{
	LOG_INFO_STR("appRunDone from " << procName);
	if (result == (AcCmdMaskOk | AcCmdMaskScheduled)) {      
		map<string,CEPApplMgrPtr>::iterator it =  itsCepApplications.find(procName);
		if(it != itsCepApplications.end()) {
			it->second->quit(to_time_t(itsStopTime));
		}
	}
	else if (result == 0) { // Error
		LOG_ERROR("Error in ACC. Stops CEP application and releases VB.");
		finishController(CT_RESULT_UNSPECIFIED);
	}
}

//
// appPaused(procname, result)
//
void OnlineControl::appPaused(const string& procName, uint16 /*result*/)
{
	LOG_INFO_STR("appPaused from " << procName);
}

//
// appQuitDone(procName, result)
//
void OnlineControl::appQuitDone(const string& procName, uint16 result)
{
	LOG_INFO_STR("appQuitDone from " << procName);
	if (result == (AcCmdMaskOk | AcCmdMaskScheduled)) {  
		//_qualityGuard.stopMonitoring(); // not in this increment
	}
	else {
		finishController(CT_RESULT_NO_ERROR);
	}
}

//
// appSnapshotDone(procName, result)
//
void OnlineControl::appSnapshotDone(const string& procName, uint16 /*result*/)
{
	LOG_INFO_STR("appSnapshotDone from " << procName);
	time_t rsto(0);
	try {
		rsto = globalParameterSet()->getTime("rescheduleTimeOut");
	}
	catch (...) {}

	map<string,CEPApplMgrPtr>::iterator it =  itsCepApplications.find(procName);
	if(it != itsCepApplications.end()) {
		it->second->pause(0, rsto, "condition");
	}
}

//
// appRecovered(procName, result)
//
void OnlineControl::appRecovered(const string& procName, uint16 /*result*/)
{
	LOG_INFO_STR("appRecovered from " << procName);

	time_t startTime  = to_time_t(itsStartTime);
	time_t reinitTime = startTime  - itsCepAppParams[0].getTime("ApplCtrl.timeout_reinit");

	string paramFileName(formatString("ACC-%s.param", getName().c_str()));

	map<string,CEPApplMgrPtr>::iterator it =  itsCepApplications.find(procName);
	if(it != itsCepApplications.end()) {
		it->second->reinit(reinitTime, paramFileName);
	}
}

//
// appReinitialized(procName, result)
//
void OnlineControl::appReinitialized(const string& procName, uint16 result)
{ 
	LOG_INFO_STR("appReinitialized from " << procName);
	if (result == AcCmdMaskOk) {    
		//    _doStateTransition(LOGICALDEVICE_STATE_SUSPENDED);
	}
	else if (result == (AcCmdMaskOk | AcCmdMaskScheduled))  {  
		map<string,CEPApplMgrPtr>::iterator it =  itsCepApplications.find(procName);
		if(it != itsCepApplications.end()) {
			it->second->run(to_time_t(itsStartTime));
		}
	}
}

//
// appReplaced(procNAme, result)
//
void OnlineControl::appReplaced(const string& procName, uint16 /*result*/)
{
	LOG_INFO_STR("appReplaced from " << procName);
}

//
// appSupplyInfo(procName, keyList)
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
void OnlineControl::appSupplyInfoAnswer(const string& procName, const string& answer)
{
	LOG_INFO_STR("Answer from " << procName << ": " << answer);
}


}; // CEPCU
}; // LOFAR
