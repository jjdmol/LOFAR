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

#include <boost/shared_array.hpp>
#include <APS/ParameterSet.h>
#include <APS/Exceptions.h>
#include <GCF/GCF_PVTypes.h>
#include <GCF/PAL/GCF_PVSSInfo.h>
#include <GCF/Utils.h>
#include <GCF/GCF_ServiceInfo.h>
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
    itsCepApplication   (*this, cntlrName),
	itsCepAppParams     (),
	itsResultParams     (),
	itsState			(CTState::NOSTATE),
	itsTreePrefix       (""),
	itsInstanceNr       (0),
	itsStartTime        (),
	itsStopTime         (),
	itsClaimPeriod      (),
	itsPreparePeriod    ()
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
// ~OnlineControl()
//
OnlineControl::~OnlineControl()
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
void    OnlineControl::setState(CTState::CTstateNr     newState)
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
		setState(CTState::ACTIVE);
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
		LOG_DEBUG_STR("Received RELEASED(" << msg.cntlrName << ")");
		setState(CTState::RELEASE);
		doRelease(event);
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
// doPrepare(cntlrName)
//
uint16_t OnlineControl::doClaim(const string& cntlrName)
{
  uint16_t result = CT_RESULT_NO_ERROR;
  try
  {
	itsCepAppParams.clear();

	itsCepAppParams.replace("AC.application", cntlrName);
	itsCepAppParams.replace("AC.resultfile", formatString("./ACC-%s_result.param", cntlrName.c_str()));
  
	string processScope("AC.process");
	string onlineCtrlPrefix(globalParameterSet()->locateModule("OnlineCtrl") + "OnlineCtrl.");
  
	string procName, startstoptype, executable, hostName;
	string ldName(getName().c_str());
	
	procName = globalParameterSet()->getString(onlineCtrlPrefix+"ApplCtrl.ACCprocess.name");
	startstoptype = globalParameterSet()->getString(onlineCtrlPrefix+"ApplCtrl.ACCprocess.startstopType");
	executable = globalParameterSet()->getString(onlineCtrlPrefix+"ApplCtrl.ACCprocess.executable");
	hostName = globalParameterSet()->getString(onlineCtrlPrefix+"ApplCtrl.ACCprocess.hostname");

	itsCepAppParams.adoptCollection(globalParameterSet()->makeSubset(onlineCtrlPrefix+"ApplCtrl", "AC"));
	itsCepAppParams.adoptCollection(globalParameterSet()->makeSubset(onlineCtrlPrefix+procName, procName));
	  
	// add some keys to cope with the differences between the OTDB and ACC
	itsCepAppParams.replace("AC.process[0].count","1");
	itsCepAppParams.replace("AC.application",procName);
	itsCepAppParams.replace("AC.process[1].ID",procName);
	itsCepAppParams.replace("AC.hostname",hostName);
	itsCepAppParams.replace(formatString("%s.%s[0].startstoptype",procName.c_str(),procName.c_str()),startstoptype);
	itsCepAppParams.replace(formatString("%s.%s[0].executable",procName.c_str(),procName.c_str()),executable);

	// create nodelist
	vector<string> nodes = globalParameterSet()->getStringVector("Observation.VirtualInstrument.BGLNodeList");
	int nodeIndex=1;
	for(vector<string>::iterator it=nodes.begin();it!=nodes.end();++it)
	{
	  itsCepAppParams.replace(formatString("AC.%s[%d].node",procName.c_str(),nodeIndex++),*it);
	}
		
  }
  catch(APSException &)
  {
	// key not found. skip
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

  try
  {
	// TODO use parameterset of 'cntlrname' when being shared controller

	string paramFileName(formatString("ACC-%s.param", cntlrName.c_str()));
	itsCepAppParams.writeFile(paramFileName);
  
	// schedule all ACC commands
	time_t startTime  = to_time_t(itsStartTime);
	time_t initTime   = startTime  - itsCepAppParams.getTime("AC.timeout_init");
	time_t defineTime = initTime   - itsCepAppParams.getTime("AC.timeout_define") - 
	                                 itsCepAppParams.getTime("AC.timeout_startup");
	time_t bootTime   = defineTime - itsCepAppParams.getTime("AC.timeout_createsubsets");
	time_t now = time(0);
	time_t stopTime = to_time_t(itsStopTime);
	LOG_DEBUG(formatString("%d boot %s",        bootTime,   ctime(&bootTime)));
	LOG_DEBUG(formatString("%d define %s",      defineTime, ctime(&defineTime)));
	LOG_DEBUG(formatString("%d init %s",        initTime,   ctime(&initTime)));
	LOG_DEBUG(formatString("%d start %s",       startTime,  ctime(&startTime)));
	LOG_DEBUG(formatString("%d now %s time %d", now,        ctime(&now), time(0)));
	LOG_DEBUG(formatString("%d stop %s",        stopTime,   ctime(&stopTime)));
	
	string hostName;
	hostName = itsCepAppParams.getString("AC.hostname");
	if (now > bootTime)
	{
	  APLCommon::APLUtilities::remoteCopy(paramFileName,hostName,LOFAR_SHARE_LOCATION);
	  LOG_WARN("Cannot guarantee all CEP processes are started in time.");
	}
	else
    {
	  switch (itsCepApplication.getLastOkCmd())
	  {
	    case ACCmdNone:
		  itsCepApplication.boot(bootTime, paramFileName);
		  break;
        
        case ACCmdBoot:
		  itsCepApplication.define(defineTime);
		  break;
        
        case ACCmdDefine:
        case ACCmdInit:
        case ACCmdRun:
		  itsCepApplication.recover(0, "snapshot-DB");
		  break;
              
        default:
		  assert(0);
		  break;
	  }    
	  APLCommon::APLUtilities::remoteCopy(paramFileName,hostName,LOFAR_SHARE_LOCATION);
	}
  }
  catch(APSException &)
  {
	// key not found. skip
	result = CT_RESULT_UNSPECIFIED;
  }
  return result;
}

//
// doRelease(event)
//
void OnlineControl::doRelease(GCFEvent&	/*event*/)
{
  string hostName, remoteFile, resultFile;
  hostName = itsCepAppParams.getString("AC.hostname");
  resultFile = formatString("ACC-%s_result.param", getName().c_str());
  remoteFile = string(LOFAR_SHARE_LOCATION) + string("/") + resultFile;
  APLCommon::APLUtilities::copyFromRemote(hostName,remoteFile,resultFile);
  itsResultParams.adoptFile(resultFile);
  //  itsResultParams.replace(KVpair(formatString("%s.quality", getName().c_str()), (int) _qualityGuard.getQuality()));
  if (!itsResultParams.isDefined(formatString("%s.faultyNodes", getName().c_str())))
  {
    itsResultParams.add(formatString("%s.faultyNodes", getName().c_str()), "");
  }
  itsResultParams.writeFile(formatString("%s_result.param", getName().c_str()));

  itsCepApplication.quit(0);
}


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

void OnlineControl::appBooted(uint16 result)
{
  if (result == (AcCmdMaskOk | AcCmdMaskScheduled))  
  {
    time_t startTime  = to_time_t(itsStartTime);
    time_t initTime   = startTime  - itsCepAppParams.getTime("AC.timeout_init");
    time_t defineTime = initTime   - itsCepAppParams.getTime("AC.timeout_define") - 
                                     itsCepAppParams.getTime("AC.timeout_startup");
    itsCepApplication.define(defineTime);
  }
  else if (result == 0) // Error
  {
    LOG_ERROR("Error in ACC. Stops CEP application and releases Online Control.");
    itsCepApplication.quit(0);
	//    _doStateTransition(LOGICALDEVICE_STATE_RELEASING, LD_RESULT_LOW_QUALITY);
  }
}

void OnlineControl::appDefined(uint16 result)
{
  if (result == (AcCmdMaskOk | AcCmdMaskScheduled))
  {
    time_t startTime  = to_time_t(itsStartTime);
    time_t initTime   = startTime  - itsCepAppParams.getTime("AC.timeout_init");
  
    itsCepApplication.init(initTime);
  }
  else if (result == 0) // Error
  {
    LOG_ERROR("Error in ACC. Stops CEP application and releases VB.");
    itsCepApplication.quit(0);
	//    _doStateTransition(LOGICALDEVICE_STATE_RELEASING, LD_RESULT_LOW_QUALITY);
  }
}

void OnlineControl::appInitialized(uint16 result)
{
  if (result == AcCmdMaskOk)
  {    
	//    _doStateTransition(LOGICALDEVICE_STATE_SUSPENDED);
  }
  else if (result == (AcCmdMaskOk | AcCmdMaskScheduled))  
  {
    itsCepApplication.run(to_time_t(itsStartTime));
  }
  else if (result == 0) // Error
  {
    LOG_ERROR("Error in ACC. Stops CEP application and releases VB.");
    itsCepApplication.quit(0);
	//    _doStateTransition(LOGICALDEVICE_STATE_RELEASING, LD_RESULT_LOW_QUALITY);
  }
}

void OnlineControl::appRunDone(uint16 result)
{
  if (result == (AcCmdMaskOk | AcCmdMaskScheduled))
  {      
    itsCepApplication.quit(to_time_t(itsStopTime));
  }
  else if (result == 0) // Error
  {
    LOG_ERROR("Error in ACC. Stops CEP application and releases VB.");
    itsCepApplication.quit(0);
	//    _doStateTransition(LOGICALDEVICE_STATE_RELEASING, LD_RESULT_LOW_QUALITY);
  }
}

void OnlineControl::appPaused(uint16 /*result*/)
{
}

void OnlineControl::appQuitDone(uint16 result)
{
  if (result == AcCmdMaskOk)
  {  
    //_qualityGuard.stopMonitoring(); // not in this increment
  }
}

void OnlineControl::appSnapshotDone(uint16 /*result*/)
{
  time_t rsto(0);
  try 
  {
    rsto = globalParameterSet()->getTime("rescheduleTimeOut");
  }
  catch (...) {}
  itsCepApplication.pause(0, rsto, "condition");
}

void OnlineControl::appRecovered(uint16 /*result*/)
{
  
  time_t startTime  = to_time_t(itsStartTime);
  time_t reinitTime = startTime  - itsCepAppParams.getTime("AC.timeout_reinit");
  
  string paramFileName(formatString("ACC-%s.param", getName().c_str()));
  
  itsCepApplication.reinit(reinitTime, paramFileName);
}

void OnlineControl::appReinitialized(uint16 result)
{
  if (result == AcCmdMaskOk)
  {    
	//    _doStateTransition(LOGICALDEVICE_STATE_SUSPENDED);
  }
  else if (result == (AcCmdMaskOk | AcCmdMaskScheduled))  
  {  
    itsCepApplication.run(to_time_t(itsStartTime));
  }
}

void OnlineControl::appReplaced(uint16 /*result*/)
{
}

string OnlineControl::appSupplyInfo(const string& keyList)
{
  string ret(keyList);
  return ret;
}

void OnlineControl::appSupplyInfoAnswer(const string& answer)
{
  LOG_INFO_STR("Answer: " << answer);
}


}; // CEPCU
}; // LOFAR
