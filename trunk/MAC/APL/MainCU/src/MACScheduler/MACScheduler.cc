//#  MACScheduler.cc: Implementation of the MAC Scheduler task
//#
//#  Copyright (C) 2002-2004
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
#include <GCF/GCF_ServiceInfo.h>
#include <GCF/GCF_PVTypes.h>
#include <GCF/Protocols/PA_Protocol.ph>
#include <GCF/PAL/GCF_PVSSInfo.h>
#include <GCF/Utils.h>
#include <APL/APLCommon/APL_Defines.h>
#include <APL/APLCommon/ControllerDefines.h>
#include <APL/APLCommon/StationInfo.h>
#include <APL/APLCommon/APLCommonExceptions.h>
#include <APL/APLCommon/Controller_Protocol.ph>

#include "MACSchedulerDefines.h"
#include "MACScheduler.h"

using namespace LOFAR::GCF::Common;
using namespace LOFAR::GCF::TM;
using namespace LOFAR::GCF::PAL;
using namespace LOFAR::OTDB;
using namespace LOFAR::Deployment;
using namespace std;

namespace LOFAR {
	using namespace APLCommon;
	using namespace ACC::APS;
	namespace MainCU {
	
//
// MACScheduler()
//
MACScheduler::MACScheduler() :
	GCFTask 			((State)&MACScheduler::initial_state,string(MS_TASKNAME)),
	PropertySetAnswerHandlerInterface(),
	itsPropertySetAnswer(*this),
	itsPropertySet		(),
//	itsObsCntlrMap		(),
	itsObservations		(),
	itsTimerPort		(0),
	itsChildControl		(0),
	itsChildPort		(0),
	itsSecondTimer		(0),
	itsQueuePeriod		(0),
	itsClaimPeriod		(0),
	itsOTDBconnection	(0),
	itsOTDBpollInterval	(0),
	itsNextOTDBpolltime (0)
{
	LOG_TRACE_OBJ ("MACscheduler construction");

	// Readin some parameters from the ParameterSet.
	itsOTDBpollInterval = globalParameterSet()->getTime("OTDBpollInterval");
	itsQueuePeriod 		= globalParameterSet()->getTime("QueuePeriod");
	itsClaimPeriod 		= globalParameterSet()->getTime("ClaimPeriod");

	// attach to child control task
	itsChildControl = ChildControl::instance();
	itsChildPort = new GCFITCPort (*this, *itsChildControl, "childITCport", 
									GCFPortInterface::SAP, CONTROLLER_PROTOCOL);
	ASSERTSTR(itsChildPort, "Cannot allocate ITCport for childcontrol");
	itsChildPort->open();		// will result in F_CONNECTED

	// need port for timers
	itsTimerPort = new GCFTimerPort(*this, "Timerport");

	registerProtocol (CONTROLLER_PROTOCOL, CONTROLLER_PROTOCOL_signalnames);
	registerProtocol(PA_PROTOCOL, PA_PROTOCOL_signalnames);
}


//
// ~MACScheduler()
//
MACScheduler::~MACScheduler()
{
	LOG_TRACE_OBJ ("~MACscheduler");

	if (itsPropertySet) {
		itsPropertySet->setValue(string(PVSSNAME_FSM_STATE),GCFPVString("down"));
		// Note: disable is not neccesary because this is always done in destructor
		//		 of propertyset.
	}

	if (itsOTDBconnection) {
		delete itsOTDBconnection;
	}
}


//
// handlePropertySetAnswer(answer)
//
void MACScheduler::handlePropertySetAnswer(GCFEvent& answer)
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
		itsTimerPort->setTimer(0.0);
		break;
	}

	case F_PS_CONFIGURED:
	{
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
		GCFPropValueEvent* pPropAnswer=static_cast<GCFPropValueEvent*>(&answer);

		// TODO: implement something usefull.
		// change of queueTime
		if ((strstr(pPropAnswer->pPropName, MS_PROPSET_NAME) != 0) &&
			(pPropAnswer->pValue->getType() == LPT_INTEGER)) {
			uint32	newVal = (uint32) ((GCFPVInteger*)pPropAnswer->pValue)->getValue();
			if (strstr(pPropAnswer->pPropName, PVSSNAME_MS_QUEUEPERIOD) != 0) {
				LOG_INFO_STR ("Changing QueuePeriod from " << itsQueuePeriod <<
							  " to " << newVal);
				itsQueuePeriod = newVal;
			}
			else if (strstr(pPropAnswer->pPropName, PVSSNAME_MS_CLAIMPERIOD) != 0) {
				LOG_INFO_STR ("Changing ClaimPeriod from " << itsClaimPeriod <<
							  " to " << newVal);
				itsClaimPeriod = newVal;
			}
		}
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
GCFEvent::TResult MACScheduler::initial_state(GCFEvent& event, GCFPortInterface& /*port*/)
{
	LOG_DEBUG_STR ("initial_state:" << evtstr(event));

	GCFEvent::TResult status = GCFEvent::HANDLED;
  
	switch (event.signal) {
    case F_INIT:
   		break;

	case F_ENTRY: {
		// Get access to my own propertyset.
		LOG_DEBUG ("Activating PropertySet");
		itsPropertySet = GCFMyPropertySetPtr(new GCFMyPropertySet(MS_PROPSET_NAME,
																  MS_PROPSET_TYPE,
																  PS_CAT_PERM_AUTOLOAD,
																  &itsPropertySetAnswer));
		itsPropertySet->enable();
		// Wait for timer that is set in PropertySetAnswer on ENABLED event.
		}
		break;
	  
	case F_TIMER: {		// must be timer that PropSet is enabled.
		// update PVSS.
		LOG_TRACE_FLOW ("Updateing state to PVSS");
		itsPropertySet->setValue(string(PVSSNAME_FSM_STATE),GCFPVString  ("initial"));
		itsPropertySet->setValue(string(PVSSNAME_FSM_ERROR),GCFPVString  (""));
		itsPropertySet->setValue(string(MS_OTDB_CONNECTED), GCFPVBool    (false));
		itsPropertySet->setValue(string(MS_OTDB_LASTPOLL),  GCFPVString  (""));
		itsPropertySet->setValue(string(MS_OTDB_POLL_ITV),  GCFPVUnsigned(itsOTDBpollInterval));

      
		// Try to connect to the SAS database.
		ACC::APS::ParameterSet* pParamSet = ACC::APS::globalParameterSet();
		string username	= pParamSet->getString("OTDBusername");
		string DBname 	= pParamSet->getString("OTDBdatabasename");
		string password	= pParamSet->getString("OTDBpassword");

		LOG_DEBUG ("Trying to connect to the OTDB");
		itsOTDBconnection= new OTDBconnection(username, password, DBname);
		ASSERTSTR (itsOTDBconnection, "Memory allocation error (OTDB)");
		ASSERTSTR (itsOTDBconnection->connect(),
					"Unable to connect to database " << DBname << " using " <<
					username << "," << password);
		LOG_INFO ("Connected to the OTDB");
		itsPropertySet->setValue(string(MS_OTDB_CONNECTED),GCFPVBool(true));

		// Start ChildControl task
		LOG_DEBUG ("Enabling ChildControltask");
		itsChildControl->openService(MAC_SVCMASK_SCHEDULERCTRL, 0);
		itsChildControl->registerCompletionPort(itsChildPort);

		TRAN(MACScheduler::recover_state);				// go to next state.
		}
		break;

	case F_CONNECTED:
		break;

	case F_DISCONNECTED:
		break;
	
	default:
		LOG_DEBUG ("MACScheduler::initial, default");
		status = GCFEvent::NOT_HANDLED;
		break;
	}    
	return (status);
}


//
// recover_state(event, port)
//
// Read last PVSS states, compare those to the SAS states and try to
// recover to the last situation.
//
GCFEvent::TResult MACScheduler::recover_state(GCFEvent& event, GCFPortInterface& port)
{
	LOG_DEBUG_STR ("recover_state:" << evtstr(event) << "@" << port.getName());

	GCFEvent::TResult status = GCFEvent::HANDLED;

	switch (event.signal) {
	case F_INIT:
		break;

	case F_ENTRY: {
		// update PVSS
		itsPropertySet->setValue(string(PVSSNAME_FSM_STATE),GCFPVString("recover"));
		itsPropertySet->setValue(string(PVSSNAME_FSM_ERROR),GCFPVString(""));

		//
		// TODO: do recovery

		TRAN(MACScheduler::active_state);
		
		break;
	}
  
	default:
		LOG_DEBUG("recover_state, default");
		status = GCFEvent::NOT_HANDLED;
		break;
	}    
	return (status);
}

//
// active_state(event, port)
//
// Normal operation state. Check OTDB every OTDBpollInterval seconds and control
// the running observations.
//
GCFEvent::TResult MACScheduler::active_state(GCFEvent& event, GCFPortInterface& port)
{
	LOG_DEBUG_STR ("active:" << evtstr(event) << "@" << port.getName());

	GCFEvent::TResult status = GCFEvent::HANDLED;

	switch (event.signal) {
	case F_INIT:
		break;

	case F_ENTRY: {
		// update PVSS
		itsPropertySet->setValue(string(PVSSNAME_FSM_STATE),GCFPVString("active"));
		itsPropertySet->setValue(string(PVSSNAME_FSM_ERROR),GCFPVString(""));

		// Timers must be connected to ports, so abuse serverPort for second timer.
		itsSecondTimer = itsTimerPort->setTimer(1L);
		break;
	}

	case F_ACCEPT_REQ:
		break;

	case F_CONNECTED:	
		// Should be from the (lost) connection with the SD
		_connectedHandler(port);
		break;

	case F_DISCONNECTED:	
		// Can be from StartDaemon or ObsController.
		// StartDaemon: trouble! Try to reconnect asap.
		// ObsController: ok when obs is finished, BIG TROUBLE when not!
		_disconnectedHandler(port);
		break;

	case F_TIMER: {		// secondTimer or reconnectTimer.
		GCFTimerEvent& timerEvent=static_cast<GCFTimerEvent&>(event);
		if (timerEvent.id == itsSecondTimer) {
			// time to poll the OTDB?
			if (time(0) >= itsNextOTDBpolltime) {
				_doOTDBcheck();
				// reinit polltime at multiple of intervaltime.
				// (=more change to hit hh.mm:00)
				itsNextOTDBpolltime = time(0) + itsOTDBpollInterval;
				itsNextOTDBpolltime -= (itsNextOTDBpolltime % itsOTDBpollInterval);
			}
			itsSecondTimer = port.setTimer(1.0);
		}
		// a connection was lost and a timer was set to try to reconnect.
//		else if (...) {
			// TODO
//			map timer to port
//			port.open();
//		}
		break;
	}

	// -------------------- EVENTS FROM CHILDCONTROL --------------------

	case CONTROL_STARTED: {
		// Child control received a message from the startDaemon that the
		// observationController was started (or not)
		CONTROLStartedEvent	msg(event);
		if (msg.successful) {
			LOG_DEBUG_STR("Start of " << msg.cntlrName << " was successful");
		}
		else {
			LOG_ERROR_STR("Observation controller " << msg.cntlrName <<
						" could not be started");
			LOG_INFO("Observation will be removed from administration");
			itsObservations.erase(msg.cntlrName);
		}
		break;
	}

	case CONTROL_CONNECT: {
		// The observationController has registered itself at childControl.
		CONTROLConnectEvent conEvent(event);
		LOG_DEBUG_STR("Received CONNECT(" << conEvent.cntlrName << ")");
		// TODO: do something usefull with this information?
		break;
	}

	case CONTROL_FINISH: {
		// The observationController is going down.
		CONTROLFinishEvent finishEvent(event);
		LOG_DEBUG_STR("Received FINISH(" << finishEvent.cntlrName << ")");
		LOG_DEBUG_STR("Removing observation " << finishEvent.cntlrName << 
						" from activeList");
		itsObservations.erase(finishEvent.cntlrName);
		break;
	}

	// NOTE: ignore all other CONTROL events, we are not interested in the
	// states of the Observations. (not our job).
	default:
		LOG_DEBUG("MACScheduler::active, default");
		status = GCFEvent::NOT_HANDLED;
		break;
	}

	return (status);
}

//
// _doOTDBcheck
//
// Check if a new action should be taken based on the contents of OTDB and our own
// administration.
//
void MACScheduler::_doOTDBcheck()
{
	// get new list (list is ordered on starttime)
	vector<OTDBtree> newTreeList = itsOTDBconnection->getExecutableTrees();
	if (newTreeList.empty()) {
		return;
	}

	LOG_DEBUG(formatString("OTDBCheck:First observation is at %s (tree=%d)", 
				to_simple_string(newTreeList[0].starttime).c_str(), newTreeList[0].treeID()));

	// walk through the list and bring each observation in the right state when necc.
	uint32		listSize = newTreeList.size();
	uint32		idx = 0;
	ptime		currentTime = from_time_t(time(0));
	ASSERTSTR (currentTime != not_a_date_time, "Can't determine systemtime, bailing out");

	// REO: test pvss appl
	itsPropertySet->setValue(string(MS_OTDB_LASTPOLL),
										GCFPVString(to_simple_string(currentTime)));

	while (idx < listSize)  {
		// timediff = time to go before start of Observation
		time_duration	timediff = newTreeList[idx].starttime - currentTime;
		LOG_TRACE_VAR_STR("timediff=" << timediff);

		// when queuetime is not reached yet we are finished with the list.
		if (timediff > seconds(itsQueuePeriod)) {
			break;
		}

		// get current state of Observation
		string		cntlrName = controllerName(CNTLRTYPE_OBSERVATIONCTRL, 
												0, newTreeList[idx].treeID());
		CTState::CTstateNr	requestedState= itsChildControl->getRequestedState(cntlrName);

		// When in startup or claimtime we should try to start the controller.
		if ((timediff > seconds(0)) && (requestedState != CTState::CONNECTED)) {
			// no, let database construct the parset for the whole observation
			OTDB::TreeMaintenance	tm(itsOTDBconnection);
			OTDB::treeIDType		treeID = newTreeList[idx].treeID();
			OTDBnode				topNode = tm.getTopNode(treeID);
			// NOTE: this name must be the same as in the ChildControl.
			string					filename = formatString("%s/Observation_%d", 
														LOFAR_SHARE_LOCATION, treeID);
			if (!tm.exportTree(treeID, topNode.nodeID(), filename)) {
				LOG_ERROR_STR ("Cannot create parset file " << filename << 
							" for new observation. Observation CANNOT BE STARTED!");
			}
			else {
				// fire request for new controller, will result in CONTROL_STARTED
				itsChildControl->startChild(cntlrName, 
											treeID, 
											CNTLRTYPE_OBSERVATIONCTRL, 
											0,		// instanceNr
											myHostname(true));
				// Note: controller is now in state NO_STATE/CONNECTED (C/R)

				// register this Observation
				ParameterSet	obsPS(filename);
				Observation		newObs(&obsPS);
				newObs.name   = cntlrName;
				newObs.treeID = treeID;
				itsObservations[cntlrName] = newObs;
				LOG_DEBUG_STR("Observation " << cntlrName << " added to active Observations");
			}
			idx++;
			continue;
		}

		// in CLAIM period?
		if ((timediff > seconds(0)) && (timediff <= seconds(itsClaimPeriod))) {
			// Observation is somewhere in the claim period its should be up by now.
			if (requestedState != CTState::CLAIMED) {
				LOG_ERROR_STR("Observation " << cntlrName << 
							" should have reached the CLAIMING state by now," <<
							" check state of observation.");
			}
			idx++;
			continue;
		}

		// observation must be running (otherwise it would not be in the newTreeList)
		// TODO: check if endtime is reached and observation is still running.
	
		idx++;	
	}

}


//
// _connectedHandler(port)
//
void MACScheduler::_connectedHandler(GCFPortInterface& port)
{
}

//
// _disconnectedHandler(port)
//
void MACScheduler::_disconnectedHandler(GCFPortInterface& port)
{
	string visd;
	port.close();
#if 0
	if(_isServerPort(itsVIparentPort,port)) {
		LOG_FATAL("VI parent server closed");
		itsVIparentPort.open(); // server closed? reopen it
	}
	else if(_isVISDclientPort(port,visd)) {
		LOG_FATAL(formatString("VI Startdaemon port disconnected: %s",visd.c_str()));
		port.setTimer(3L);
	}
	else if(_isVIclientPort(port)) {
		LOG_FATAL("VI client port disconnected");
		// do something with the nodeId?
	}
#endif
}


};
};
