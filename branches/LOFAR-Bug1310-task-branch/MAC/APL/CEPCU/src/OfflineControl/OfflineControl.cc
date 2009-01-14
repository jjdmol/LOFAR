//#  OfflineControl.cc: Implementation of the MAC Scheduler task
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

#include <boost/shared_array.hpp>
#include <Common/ParameterSet.h>
#include <Common/Exceptions.h>
#include <GCF/GCF_PVTypes.h>
#include <GCF/PAL/GCF_PVSSInfo.h>
#include <GCF/Utils.h>
#include <MACIO/MACServiceInfo.h>
#include <GCF/Protocols/PA_Protocol.ph>
#include <APL/APLCommon/APL_Defines.h>
#include <APL/APLCommon/APLUtilities.h>
#include <APL/APLCommon/APLCommonExceptions.h>
#include <APL/APLCommon/Controller_Protocol.ph>
#include <APL/APLCommon/StationInfo.h>
#include <APL/APLCommon/APLUtilities.h>

#include "OfflineControl.h"
#include "OfflineControlDefines.h"

using namespace LOFAR::GCF::TM;
using namespace LOFAR::GCF::PAL;
using namespace std;

namespace LOFAR {
	using namespace APLCommon;
    using namespace ACC::ALC;
	namespace CEPCU {
	
//
// OfflineControl()
//
OfflineControl::OfflineControl(const string&	cntlrName) :
	GCFTask 			((State)&OfflineControl::initial_state,cntlrName),
	GCFPropertySetAnswerHandlerInterface(),
	itsPropertySetAnswer(*this),
	itsPropertySet		(),
	itsPropertySetInitialized (false),
	itsParentControl	(0),
	itsParentPort		(0),
	itsTimerPort		(0),
    itsCepApplications  (),
	itsCepAppParams     (),
	itsResultParams     (),
	itsProcessDependencies(),
	itsCepAppStartTimes (),
	itsState			(CTState::NOSTATE),
	itsTreePrefix       (""),
	itsInstanceNr       (0),
	itsStartTime        (),
	itsStopTime         (),
	itsClaimPeriod      (),
	itsPreparePeriod    (),
	itsCntlrName        (cntlrName)
{
	LOG_TRACE_OBJ_STR (cntlrName << " construction");

	// First readin our observation related config file.
	LOG_DEBUG_STR("Reading parset file:" << LOFAR_SHARE_LOCATION << "/" << cntlrName);
	globalParameterSet()->adoptFile(string(LOFAR_SHARE_LOCATION)+"/"+cntlrName);


	// Readin some parameters from the ParameterSet.
	itsTreePrefix = globalParameterSet()->getString("prefix");
	itsInstanceNr = globalParameterSet()->getUint32("_instanceNr");

	// get Observation based information
	itsStartTime     = time_from_string(globalParameterSet()->
											 getString("Observation.startTime"));
	itsStopTime      = time_from_string(globalParameterSet()->
											 getString("Observation.stopTime"));
	itsClaimPeriod   = globalParameterSet()->getTime  ("Observation.claimPeriod");
	itsPreparePeriod = globalParameterSet()->getTime  ("Observation.preparePeriod");

	// attach to parent control task
	itsParentControl = ParentControl::instance();
	itsParentPort = new GCFITCPort (*this, *itsParentControl, "ParentITCport", 
									GCFPortInterface::SAP, CONTROLLER_PROTOCOL);
	ASSERTSTR(itsParentPort, "Cannot allocate ITCport for Parentcontrol");
	itsParentPort->open();		// will result in F_CONNECTED

	// need port for timers.
	itsTimerPort = new GCFTimerPort(*this, "TimerPort");

	// for debugging purposes
	registerProtocol (CONTROLLER_PROTOCOL, CONTROLLER_PROTOCOL_signalnames);
	registerProtocol (PA_PROTOCOL, 		   PA_PROTOCOL_signalnames);

	setState(CTState::CREATED);
}


//
// ~OfflineControl()
//
OfflineControl::~OfflineControl()
{
	LOG_TRACE_OBJ_STR (getName() << " destruction");

	if (itsPropertySet) {
		itsPropertySet->setValue(string(PVSSNAME_FSM_STATE),GCFPVString("down"));
		itsPropertySet->disable();
	}

	// ...
}

//
// setState(CTstateNr)
//
void    OfflineControl::setState(CTState::CTstateNr     newState)
{
	itsState = newState;

	if (itsPropertySet) {
		CTState		cts;
		itsPropertySet->setValue(string(PVSSNAME_FSM_STATE),
								 GCFPVString(cts.name(newState)));
	}
}   


//
// handlePropertySetAnswer(answer)
//
void OfflineControl::handlePropertySetAnswer(GCFEvent& answer)
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
GCFEvent::TResult OfflineControl::initial_state(GCFEvent& event, 
													GCFPortInterface& port)
{
	LOG_DEBUG_STR ("initial:" << evtstr(event) << "@" << port.getName());

	GCFEvent::TResult status = GCFEvent::HANDLED;
  
	switch (event.signal) {
	case F_ENTRY:
   		break;

    case F_INIT: {
		// Get access to my own propertyset.
		LOG_DEBUG ("Activating PropertySet");
		string	propSetName = formatString(ONC_PROPSET_NAME, itsInstanceNr);
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

			// update PVSS.
			LOG_TRACE_FLOW ("Updateing state to PVSS");
			itsPropertySet->setValue(string(PVSSNAME_FSM_STATE),GCFPVString("initial"));
			itsPropertySet->setValue(string(PVSSNAME_FSM_ERROR),GCFPVString(""));
		  
			// Start ParentControl task
			LOG_DEBUG ("Enabling ParentControl task");
			itsParentPort = itsParentControl->registerTask(this);

			LOG_DEBUG ("Going to operational state");
			TRAN(OfflineControl::active_state);				// go to next state.
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
GCFEvent::TResult OfflineControl::active_state(GCFEvent& event, GCFPortInterface& port)
{
	LOG_DEBUG_STR ("active:" << evtstr(event) << "@" << port.getName());

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
		setState(CTState::CLAIMED);
		CONTROLClaimedEvent             answer;
		answer.cntlrName = getName();
		answer.result    = doClaim(msg.cntlrName);
		if(answer.result == CT_RESULT_NO_ERROR) 
		{
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
		if(answer.result == CT_RESULT_NO_ERROR) 
		{
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
// finished_state(event, port)
//
// Finishing state. 
//
GCFEvent::TResult OfflineControl::finished_state(GCFEvent& event, GCFPortInterface& port)
{
	LOG_DEBUG_STR ("active:" << evtstr(event) << "@" << port.getName());

	GCFEvent::TResult status = GCFEvent::HANDLED;

	switch (event.signal) {
	case F_ENTRY: {
      GCFTask::stop();
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
// doClaim(cntlrName)
//
uint16_t OfflineControl::doClaim(const string& cntlrName)
{
  uint16_t result = CT_RESULT_NO_ERROR;
  try {
	vector<string> procNames, nodes;
	string executable, hostName, startstoptype;
	
	procNames = globalParameterSet()->getStringVector("ApplCtrl.application");

	for (size_t i = 0; i < procNames.size(); i++) {
	  string procName = procNames[i];

	  startstoptype = globalParameterSet()->getString(procName + "._startstopType");
	  executable    = globalParameterSet()->getString(procName + "._executable");
	  hostName      = globalParameterSet()->getString(procName + "._hostname");
	  nodes   		= globalParameterSet()->getStringVector(procName + "._nodes");
	  itsProcessDependencies[procName] = 
					  globalParameterSet()->getStringVector(procName + "._depends");

	  CEPApplicationManagerPtr accClient(new CEPApplicationManager(*this, procName));
	  itsCepApplications[procName] = accClient;
	  
	  ParameterSet params;
	  params.clear();
	  // import the ApplCtrl section
	  params.adoptCollection(globalParameterSet()->makeSubset("ApplCtrl", "ApplCtrl"));
	  // import the <procname> section
	  params.adoptCollection(globalParameterSet()->makeSubset(procName, procName));
	  
	  // add some keys to cope with the differences between the OTDB and ACC
	  params.replace("ApplCtrl.resultfile", formatString("./ACC-%s_%s_result.param", cntlrName.c_str(),procName.c_str()));
	  params.replace("ApplCtrl.process[0].count","1");
	  params.replace("ApplCtrl.application",procName);
	  params.replace("ApplCtrl.process[1].ID",procName);
	  params.replace("ApplCtrl.hostname",hostName);
	  params.replace(formatString("%s[0]._startstopType", procName.c_str()),startstoptype);
	  params.replace(formatString("%s[0]._executable",procName.c_str()),executable);

	  // create nodelist
	  int nodeIndex=1;
	  for (vector<string>::iterator it=nodes.begin(); it != nodes.end(); ++it) {
		params.replace(formatString("ApplCtrl.%s[%d]._node",procName.c_str(),nodeIndex++),*it);
	  }
	  itsCepAppParams[procName] = params;
	}
  }
  catch(APSException &e)
  {
	// key not found. skip
	LOG_FATAL(e.text());
	result = CT_RESULT_UNSPECIFIED;
  }
  return result;
}

//
// doPrepare(cntlrName)
//
uint16_t OfflineControl::doPrepare(const string& cntlrName)
{
  uint16_t result = CT_RESULT_NO_ERROR;

  try
  {
	map<string, ParameterSet>::iterator it;
	for(it = itsCepAppParams.begin(); it != itsCepAppParams.end();++it)
	{
	  // only start processes that do not depend on other processes
	  // other process will start once the processes they depend on have finished
	  if(itsProcessDependencies[it->first].size() == 0)
	  {
		prepareProcess(cntlrName, it->first, to_time_t(itsStartTime));
	  }
	}
  }
  catch(APSException &e)
  {
	// key not found. skip
	LOG_FATAL(e.text());
	result = CT_RESULT_UNSPECIFIED;
  }
  return result;
}

//
// _prepareProcess(procName, startTime)
//
void OfflineControl::prepareProcess(const string& cntlrName, const string& procName, const time_t startTime)
{
  map<string, ParameterSet>::iterator it = itsCepAppParams.find(procName);
  if(it != itsCepAppParams.end()) {
	string procName = it->second.getString("ApplCtrl.process[1].ID");
	string hostName = it->second.getString("ApplCtrl.hostname");
	string paramFileName(formatString("ACC-%s_%s.param", cntlrName.c_str(),procName.c_str()));
	it->second.writeFile(paramFileName);
  
	// schedule all ACC commands
	itsCepAppStartTimes[procName] = startTime;
	time_t initTime   = startTime  - it->second.getTime("ApplCtrl.timeout_init");
	time_t defineTime = initTime   - it->second.getTime("ApplCtrl.timeout_define") - 
	                                 it->second.getTime("ApplCtrl.timeout_startup");
	time_t bootTime   = defineTime - it->second.getTime("ApplCtrl.timeout_createsubsets");
	time_t now = time(0);
	time_t stopTime = to_time_t(itsStopTime);
	if(now >= stopTime) {
	  // stoptime has already passed. Calculate a new stoptime based on the observation parameters
	  time_t startOffset = startTime - to_time_t(itsStartTime);
	  stopTime += startOffset;
	  // convert time_t to local time before constructing a ptime object from it
	  tm* ptm = localtime(&startTime);
	  itsStartTime = ptime_from_tm(*ptm);
	  ptm = localtime(&stopTime);
	  itsStopTime  = ptime_from_tm(*ptm);
	  stopTime = to_time_t(itsStopTime);
	}
	LOG_DEBUG(formatString("%d now %s time %d", now,        ctime(&now), time(0)));
	LOG_DEBUG(formatString("%d boot %s",        bootTime,   ctime(&bootTime)));
	LOG_DEBUG(formatString("%d define %s",      defineTime, ctime(&defineTime)));
	LOG_DEBUG(formatString("%d init %s",        initTime,   ctime(&initTime)));
	LOG_DEBUG(formatString("%d start %s",       startTime,  ctime(&startTime)));
	LOG_DEBUG(formatString("%d stop %s",        stopTime,   ctime(&stopTime)));
	
	if (now > bootTime) {
	  APLCommon::APLUtilities::remoteCopy(paramFileName,hostName,LOFAR_SHARE_LOCATION);
	  LOG_WARN("Cannot guarantee all CEP processes are started in time.");
	}
	else {
	  CEPApplicationManagerPtr cepAppPtr = itsCepApplications[procName];
	  if(NULL != cepAppPtr) {
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
	  else {
		LOG_WARN(formatString("Unable to find ACC process for %s.",procName.c_str()));
	  }
	  APLCommon::APLUtilities::remoteCopy(paramFileName,hostName,LOFAR_SHARE_LOCATION);
	}
  }
}

//
// doRelease()
//
void OfflineControl::doRelease(void)
{
  try {
	map<string, ParameterSet>::iterator it;
	for(it = itsCepAppParams.begin(); it != itsCepAppParams.end();++it) {
	  string hostName, remoteFile, resultFile, procName;
	  hostName = it->second.getString("ApplCtrl.hostname");
	  procName = it->second.getString("ApplCtrl.process[1].ID");
	  resultFile = formatString("ACC-%s_%s_result.param", getName().c_str(),procName.c_str());
	  remoteFile = string(LOFAR_SHARE_LOCATION) + string("/") + resultFile;
//	  APLCommon::APLUtilities::copyFromRemote(hostName,remoteFile,resultFile);
	  itsResultParams.adoptFile(resultFile);
	  //  itsResultParams.replace(KVpair(formatString("%s.quality", getName().c_str()), (int) _qualityGuard.getQuality()));
	  if (!itsResultParams.isDefined(formatString("%s.faultyNodes", getName().c_str()))) {
		itsResultParams.add(formatString("%s.faultyNodes", getName().c_str()), "");
	  }
	  itsResultParams.writeFile(formatString("%s_result.param", getName().c_str()));
	}
  }
  catch(...)
  {
  }
  map<string, CEPApplicationManagerPtr>::iterator it;
  for(it = itsCepApplications.begin();it != itsCepApplications.end();++it) {
	it->second->quit(0);
  }
}

//
// finishController
//
void OfflineControl::finishController(uint16_t /*result*/)
{
  setState(CTState::RELEASE);
  doRelease();
  setState(CTState::RELEASED);
  LOG_DEBUG ("Going to finished state");
  TRAN(OfflineControl::finished_state); // go to next state.
}

//
// _connectedHandler(port)
//
void OfflineControl::_connectedHandler(GCFPortInterface& /*port*/)
{
}

//
// _disconnectedHandler(port)
//
void OfflineControl::_disconnectedHandler(GCFPortInterface& port)
{
	port.close();
}

void OfflineControl::appBooted(const string& procName, uint16 result)
{
  LOG_INFO_STR("appBooted from " << procName);
  if (result == (AcCmdMaskOk | AcCmdMaskScheduled))  
  {
	map<string,CEPApplicationManagerPtr>::iterator itApp = itsCepApplications.find(procName);
	map<string, ParameterSet>::iterator itParam = itsCepAppParams.find(procName);
	map<string,time_t>::iterator itStart = itsCepAppStartTimes.find(procName);
	if(itApp != itsCepApplications.end() && itParam != itsCepAppParams.end() && itStart != itsCepAppStartTimes.end())
    {
	  time_t now = time(0);
	  time_t startTime  = itStart->second;
	  time_t initTime   = startTime  - itParam->second.getTime("AC.timeout_init");
	  time_t defineTime = initTime   - itParam->second.getTime("AC.timeout_define") - 
                                       itParam->second.getTime("AC.timeout_startup");

	LOG_DEBUG(formatString("%d now %s time %d", now,        ctime(&now), time(0)));
	LOG_DEBUG(formatString("%d define %s",      defineTime, ctime(&defineTime)));
	LOG_DEBUG(formatString("%d init %s",        initTime,   ctime(&initTime)));
	LOG_DEBUG(formatString("%d start %s",       startTime,  ctime(&startTime)));

	  itApp->second->define(defineTime);
	}
	else
    {
	  LOG_ERROR("Error in ACC. Stops CEP application and releases Offline Control.");
	  finishController(CT_RESULT_UNSPECIFIED);
    }
  }
  else if (result == 0) // Error
  {
    LOG_ERROR("Error in ACC. Stops CEP application and releases Offline Control.");
	finishController(CT_RESULT_UNSPECIFIED);
  }
}

void OfflineControl::appDefined(const string& procName, uint16 result)
{
  LOG_INFO_STR("appDefined from " << procName);
  if (result == (AcCmdMaskOk | AcCmdMaskScheduled))
  {
	map<string,CEPApplicationManagerPtr>::iterator itApp = itsCepApplications.find(procName);
	map<string, ParameterSet>::iterator itParam = itsCepAppParams.find(procName);
	map<string,time_t>::iterator itStart = itsCepAppStartTimes.find(procName);
	if(itApp != itsCepApplications.end() && itParam != itsCepAppParams.end() && itStart != itsCepAppStartTimes.end())
    {
	  time_t now = time(0);
	  time_t startTime  = itStart->second;
	  time_t initTime   = startTime  - itParam->second.getTime("AC.timeout_init");
  
	LOG_DEBUG(formatString("%d now %s time %d", now,        ctime(&now), time(0)));
	LOG_DEBUG(formatString("%d init %s",        initTime,   ctime(&initTime)));
	LOG_DEBUG(formatString("%d start %s",       startTime,  ctime(&startTime)));

	  itApp->second->init(initTime);
	}
	else
    {
	  LOG_ERROR("Error in ACC. Stops CEP application and releases Offline Control.");
	  finishController(CT_RESULT_UNSPECIFIED);
    }
  }
  else if (result == 0) // Error
  {
	LOG_ERROR("Error in ACC. Stops CEP application and releases Offline Control.");
	finishController(CT_RESULT_UNSPECIFIED);
  }
}

void OfflineControl::appInitialized(const string& procName, uint16 result)
{
  LOG_INFO_STR("appInitialized from " << procName);
  if (result == AcCmdMaskOk)
  {    
	//    _doStateTransition(LOGICALDEVICE_STATE_SUSPENDED);
  }
  else if (result == (AcCmdMaskOk | AcCmdMaskScheduled))  
  {
	map<string,CEPApplicationManagerPtr>::iterator itApp =  itsCepApplications.find(procName);
	map<string,time_t>::iterator itStart = itsCepAppStartTimes.find(procName);
	if(itApp != itsCepApplications.end() && itStart != itsCepAppStartTimes.end())
    {
	  itApp->second->run(itStart->second);
	}
	else
    {
	  LOG_ERROR("Error in ACC. Stops CEP application and releases Offline Control.");
	  finishController(CT_RESULT_UNSPECIFIED);
    }
  }
  else if (result == 0) // Error
  {
	LOG_ERROR("Error in ACC. Stops CEP application and releases Offline Control.");
	finishController(CT_RESULT_UNSPECIFIED);
  }
}

void OfflineControl::appRunDone(const string& procName, uint16 result)
{
  LOG_INFO_STR("appRunDone from " << procName);
  if (result == (AcCmdMaskOk | AcCmdMaskScheduled))
  {      
	map<string,CEPApplicationManagerPtr>::iterator itApp =  itsCepApplications.find(procName);
	if(itApp != itsCepApplications.end())
	{
	  itApp->second->quit(to_time_t(itsStopTime));
	}
  }
  else if (result == 0) // Error
  {
    LOG_ERROR("Error in ACC. Stops CEP application and releases VB.");
	finishController(CT_RESULT_UNSPECIFIED);
  }
}

void OfflineControl::appPaused(const string& procName, uint16 /*result*/)
{
  LOG_INFO_STR("appPaused from " << procName);
}

void OfflineControl::appQuitDone(const string& procName, uint16 result)
{
  LOG_INFO_STR("appQuitDone from " << procName);
  if (result == (AcCmdMaskOk | AcCmdMaskScheduled))
  {  
    //_qualityGuard.stopMonitoring(); // not in this increment
  }
  else if (result == AcCmdMaskOk)
  {
	// process finished. Check if processes depend on this process.
	map<string, vector<string> >::iterator itDep;
	for(itDep = itsProcessDependencies.begin(); itDep != itsProcessDependencies.end();++itDep)
	{
	  if(itDep->second.size() > 0)
	  {
		vector<string>::iterator itDepProc = itDep->second.begin();
		bool found = false;
		while(itDepProc != itDep->second.end() && !found)
		{
		  LOG_DEBUG(formatString("appQuitDone: comparing %s with %s",itDepProc->c_str(),procName.c_str()));
		  if((*itDepProc) == procName)
		  {
			found = true;
			// This process depends on the process that just finished
			// If the list only contains the finished process, the new process can be started
			if(itDep->second.size() == 1)
			{
			  map<string, ParameterSet>::iterator itParam = itsCepAppParams.find(procName);
			  if(itParam != itsCepAppParams.end())
			  {
				// boot the process in 30 seconds
				time_t bootTime = time(0) + 30;
				time_t startTime = bootTime + itParam->second.getTime("AC.timeout_init") +
				                              itParam->second.getTime("AC.timeout_define") + 
				                              itParam->second.getTime("AC.timeout_startup") + 
				                              itParam->second.getTime("AC.timeout_createsubsets");

				prepareProcess(itsCntlrName,itDep->first,startTime);
			  }
			}
		  }
		  else
		  {
			++itDepProc;
		  }
		}
		if(itDepProc != itDep->second.end() && found)
		{
		  LOG_DEBUG(formatString("appQuitDone: removing dependency of %s",itDepProc->c_str()));
		  // Remove the process from the dependencies list. 
		  itDep->second.erase(itDepProc);
		}
	  }
	}
	// remove the process from the AC process list. If the list is empty, OfflineControl can be finished
	itsCepApplications.erase(procName);
	if(itsCepApplications.size() == 0)
    {
	  finishController(CT_RESULT_NO_ERROR);
  	}
  }
  else if(result != 0)
  {
 	finishController(CT_RESULT_UNSPECIFIED);
  }
}

void OfflineControl::appSnapshotDone(const string& procName, uint16 /*result*/)
{
  LOG_INFO_STR("appSnapshotDone from " << procName);
  time_t rsto(0);
  try 
  {
    rsto = globalParameterSet()->getTime("rescheduleTimeOut");
  }
  catch (...) {}
  map<string,CEPApplicationManagerPtr>::iterator it =  itsCepApplications.find(procName);
  if(it != itsCepApplications.end())
  {
	it->second->pause(0, rsto, "condition");
  }
}

void OfflineControl::appRecovered(const string& procName, uint16 /*result*/)
{
  LOG_INFO_STR("appRecovered from " << procName);
  
  map<string,CEPApplicationManagerPtr>::iterator itApp = itsCepApplications.find(procName);
  map<string, ParameterSet>::iterator itParam = itsCepAppParams.find(procName);
  map<string,time_t>::iterator itStart = itsCepAppStartTimes.find(procName);
  if(itApp != itsCepApplications.end() && itParam != itsCepAppParams.end() && itStart != itsCepAppStartTimes.end())
  {
	time_t startTime  = itStart->second;
	time_t reinitTime = startTime  - itParam->second.getTime("AC.timeout_reinit");
  
	string paramFileName(formatString("ACC-%s.param", getName().c_str()));
  
	itApp->second->reinit(reinitTime, paramFileName);
  }
}

void OfflineControl::appReinitialized(const string& procName, uint16 result)
{ 
  LOG_INFO_STR("appReinitialized from " << procName);
  if (result == AcCmdMaskOk)
  {    
	//    _doStateTransition(LOGICALDEVICE_STATE_SUSPENDED);
  }
  else if (result == (AcCmdMaskOk | AcCmdMaskScheduled))  
  {  
	map<string,CEPApplicationManagerPtr>::iterator it =  itsCepApplications.find(procName);
	if(it != itsCepApplications.end())
    {
	  it->second->run(to_time_t(itsStartTime));
	}
  }
}

void OfflineControl::appReplaced(const string& procName, uint16 /*result*/)
{
  LOG_INFO_STR("appReplaced from " << procName);
}

string OfflineControl::appSupplyInfo(const string& procName, const string& keyList)
{
  LOG_INFO_STR("appSupplyInfo from " << procName);
  string ret(keyList);
  return ret;
}

void OfflineControl::appSupplyInfoAnswer(const string& procName, const string& answer)
{
  LOG_INFO_STR("Answer from " << procName << ": " << answer);
}


}; // CEPCU
}; // LOFAR
