//#  MACScheduler.cc: Implementation of the MAC Scheduler task
//#
//#  Copyright (C) 2004-2008
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
#include <Common/SystemUtil.h>
#include <Common/StringUtil.h>
#include <Common/Version.h>

#include <Common/ParameterSet.h>
#include <GCF/TM/GCF_Protocols.h>
#include <MACIO/MACServiceInfo.h>
#include <GCF/PVSS/GCF_PVTypes.h>
#include <APL/APLCommon/APL_Defines.h>
#include <APL/APLCommon/ControllerDefines.h>
#include <APL/APLCommon/StationInfo.h>
#include <APL/APLCommon/Controller_Protocol.ph>
#include <GCF/RTDB/DP_Protocol.ph>
#include <APL/RTDBCommon/CM_Protocol.ph>
#include <OTDB/TreeStateConv.h>
#include <signal.h>

#include "MACSchedulerDefines.h"
#include "MACScheduler.h"
#include "../Package__Version.h"

using namespace LOFAR::GCF::PVSS;
using namespace LOFAR::GCF::TM;
using namespace LOFAR::GCF::RTDB;
using namespace LOFAR::OTDB;
using namespace LOFAR::Deployment;
using namespace std;

namespace LOFAR {
	using namespace APLCommon;
	namespace MainCU {

// static (this) pointer used for signal handling
static MACScheduler* pMacScheduler = 0;
	
//
// MACScheduler()
//
MACScheduler::MACScheduler() :
	GCFTask 			((State)&MACScheduler::initial_state,string("MACScheduler")),
	itsPropertySet		(0),
	itsChildControl		(0),
	itsChildPort		(0),
	itsClaimerTask		(0),
	itsClaimerPort		(0),
	itsTimerPort		(0),
	itsSecondTimer		(0),
	itsNextPlannedTime	(0),
	itsNextActiveTime	(0),
	itsNextFinishedTime	(0),
	itsOTDBconnection	(0)
{
	LOG_TRACE_OBJ ("MACscheduler construction");

	LOG_INFO_STR("MACProcessScope: " << PSN_MAC_SCHEDULER);
	LOG_INFO(Version::getInfo<MainCUVersion>("MACScheduler"));

	// Read timersettings from the ParameterSet
	itsPlannedItv	 = globalParameterSet()->getTime("pollIntervalPlanned", 60);
	itsActiveItv	 = globalParameterSet()->getTime("pollIntervalExecute", 5);
	itsFinishedItv	 = globalParameterSet()->getTime("pollIntervalFinished", 60);
	itsPlannedPeriod = globalParameterSet()->getTime("plannedPeriod",  86400) / 60;	// in minutes
	itsFinishedPeriod= globalParameterSet()->getTime("finishedPeriod", 86400) / 60; // in minutes

	// Read the schedule periods for starting observations.
	itsQueuePeriod 		= globalParameterSet()->getTime("QueuePeriod");
	itsClaimPeriod 		= globalParameterSet()->getTime("ClaimPeriod");

	// attach to child control task
	itsChildControl = ChildControl::instance();
	itsChildPort = new GCFITCPort (*this, *itsChildControl, "childITCport", 
									GCFPortInterface::SAP, CONTROLLER_PROTOCOL);
	ASSERTSTR(itsChildPort, "Cannot allocate ITCport for childcontrol");
	itsChildPort->open();		// will result in F_CONNECTED

	// create an PVSSprepare Task
	itsClaimerTask = new ObsClaimer(this);
	ASSERTSTR(itsClaimerTask, "Cannot construct a ObsClaimerTask");
	itsClaimerPort = new GCFITCPort (*this, *itsClaimerTask, "ObsClaimerPort",
									GCFPortInterface::SAP, CM_PROTOCOL);

	// need port for timers
	itsTimerPort = new GCFTimerPort(*this, "Timerport");

	registerProtocol(CONTROLLER_PROTOCOL, CONTROLLER_PROTOCOL_STRINGS);
	registerProtocol(DP_PROTOCOL, 		  DP_PROTOCOL_STRINGS);
}


//
// ~MACScheduler()
//
MACScheduler::~MACScheduler()
{
	LOG_TRACE_OBJ ("~MACscheduler");

	if (itsPropertySet) {
		delete itsPropertySet;
	}

	if (itsOTDBconnection) {
		delete itsOTDBconnection;
	}
}

//
// sigintHandler(signum)
//
void MACScheduler::sigintHandler(int signum)
{
	LOG_DEBUG (formatString("SIGINT signal detected (%d)",signum));

	if (pMacScheduler) {
		pMacScheduler->finish();
	}
}


//
// _databaseEventHandler(event)
//
void MACScheduler::_databaseEventHandler(GCFEvent& event)
{

//	LOG_DEBUG_STR ("_databaseEventHandler:" << eventName(event));

	switch(event.signal) {
	case DP_CHANGED: {
		DPChangedEvent	dpEvent(event);

#if 0
		// TODO: implement something usefull.
		if (strstr(dpEvent.DPname.c_str(), PVSSNAME_MS_QUEUEPERIOD) != 0) {
			uint32	newVal = ((GCFPVUnsigned*) (dpEvent.value._pValue))->getValue();
			LOG_INFO_STR ("Changing QueuePeriod from " << itsQueuePeriod << " to " << newVal);
			itsQueuePeriod = newVal;
		}
		if (strstr(dpEvent.DPname.c_str(), PVSSNAME_MS_CLAIMPERIOD) != 0) {
			uint32	newVal = ((GCFPVUnsigned*) (dpEvent.value._pValue))->getValue();
			LOG_INFO_STR ("Changing ClaimPeriod from " << itsClaimPeriod << " to " << newVal);
			itsClaimPeriod = newVal;
		}
#endif
	}  
	break;

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
	LOG_DEBUG_STR ("initial_state:" << eventName(event));

	GCFEvent::TResult status = GCFEvent::HANDLED;
  
	switch (event.signal) {
    case F_INIT:
   		break;

	case F_ENTRY: {
		// Get access to my own propertyset.
		LOG_DEBUG ("Activating PropertySet");
		itsPropertySet = new RTDBPropertySet(PSN_MAC_SCHEDULER,
											 PST_MAC_SCHEDULER,
											 PSAT_RW,
											 this);
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
	  
	case F_TIMER: {		// must be timer that PropSet is enabled.
		// update PVSS.
		LOG_TRACE_FLOW ("Updateing state to PVSS");
		itsPropertySet->setValue(PN_FSM_CURRENT_ACTION,		  GCFPVString  ("initial"));
		itsPropertySet->setValue(PN_FSM_ERROR,				  GCFPVString  (""));
		itsPropertySet->setValue(PN_MS_OTDB_CONNECTED,    	  GCFPVBool    (false));
		itsPropertySet->setValue(PN_MS_OTDB_LAST_POLL,    	  GCFPVString  (""));
		itsPropertySet->setValue(PN_MS_OTDB_POLLINTERVAL, 	  GCFPVInteger (itsPlannedItv));
		GCFPValueArray	emptyArr;
		itsPropertySet->setValue(PN_MS_ACTIVE_OBSERVATIONS,   GCFPVDynArr(LPT_STRING, emptyArr));
		itsPropertySet->setValue(PN_MS_PLANNED_OBSERVATIONS,  GCFPVDynArr(LPT_STRING, emptyArr));
		itsPropertySet->setValue(PN_MS_FINISHED_OBSERVATIONS, GCFPVDynArr(LPT_STRING, emptyArr));

      
		// Try to connect to the SAS database.
		ParameterSet* pParamSet = globalParameterSet();
		string username	= pParamSet->getString("OTDBusername");
		string DBname 	= pParamSet->getString("OTDBdatabasename");
		string password	= pParamSet->getString("OTDBpassword");
		string hostname	= pParamSet->getString("OTDBhostname");

		LOG_DEBUG_STR ("Trying to connect to the OTDB on " << hostname);
		itsOTDBconnection= new OTDBconnection(username, password, DBname, hostname);
		ASSERTSTR (itsOTDBconnection, "Memory allocation error (OTDB)");
		ASSERTSTR (itsOTDBconnection->connect(),
					"Unable to connect to database " << DBname << " on " << hostname << 
					" using " << username << "," << password);
		LOG_INFO ("Connected to the OTDB");
		itsPropertySet->setValue(PN_MS_OTDB_CONNECTED, GCFPVBool(true));

		// Start ChildControl task
		LOG_DEBUG ("Enabling ChildControltask");
		itsChildControl->openService(MAC_SVCMASK_SCHEDULERCTRL, 0);
		itsChildControl->registerCompletionPort(itsChildPort);

		// setup initial schedule: first planned, next run active, second run finished
		itsNextPlannedTime  = time(0);
		itsNextActiveTime   = itsNextPlannedTime +   itsPlannedItv;
		itsNextFinishedTime = itsNextPlannedTime + 2*itsPlannedItv;

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
	LOG_DEBUG_STR ("recover_state:" << eventName(event) << "@" << port.getName());

	GCFEvent::TResult status = GCFEvent::HANDLED;

	switch (event.signal) {
	case F_INIT:
		break;

	case F_ENTRY: {
		// update PVSS
		itsPropertySet->setValue(string(PN_FSM_CURRENT_ACTION),GCFPVString("recover"));
		itsPropertySet->setValue(string(PN_FSM_ERROR),GCFPVString(""));

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
// Normal operation state. Check OTDB every itsActiveItv seconds and control
// the running observations.
//
GCFEvent::TResult MACScheduler::active_state(GCFEvent& event, GCFPortInterface& port)
{
	LOG_DEBUG_STR ("active:" << eventName(event) << "@" << port.getName());

	GCFEvent::TResult status = GCFEvent::HANDLED;

	switch (event.signal) {
	case F_INIT:
		break;

	case F_ENTRY: {
  	    // install my own signal handler. GCFTask also installs a handler so we have 
		// to install our handler later than the GCFTask handler.
	    pMacScheduler = this;
		signal (SIGINT, MACScheduler::sigintHandler);	// ctrl-c
		signal (SIGTERM, MACScheduler::sigintHandler);	// kill

		// update PVSS
		itsPropertySet->setValue(string(PN_FSM_CURRENT_ACTION),GCFPVString("active"));
		itsPropertySet->setValue(string(PN_FSM_ERROR),GCFPVString(""));

		// Start heartbeat timer.
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

	case DP_CHANGED:
		_databaseEventHandler(event);
		break;

	case CM_CLAIM_RESULT: {
			// some observation was claimed by the claimMgr. Update our prepare_list.
			CMClaimResultEvent	cmEvent(event);
			LOG_INFO_STR(cmEvent.nameInAppl << " is mapped to " << cmEvent.DPname);
			ltrim(cmEvent.nameInAppl, "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_");
			int		obsID = atoi(cmEvent.nameInAppl.c_str());
			LOG_DEBUG_STR("PVSS preparation of observation " << obsID << " ready.");
			itsPreparedObs[obsID] = true;
		}
		break;

	case F_TIMER: {		// secondTimer or reconnectTimer.
		GCFTimerEvent& timerEvent=static_cast<GCFTimerEvent&>(event);
		if (timerEvent.id == itsSecondTimer) {
			// time to poll the OTDB?
			// Note: We assume here that the PlannedItv is smaller than the ActiveItv and the FinishedItv.
			//		 When it is not smaller (very unlikely) than the ActiveItv and FinishedItv those two
			//		 intervals will default to the PlannedItv.
			if (time(0) >= itsNextPlannedTime) {		// check shortest interval
				_doOTDBcheck();

				// reinit polltime at multiple of intervaltime.
				// (=more change to hit hh.mm:00)
				itsNextPlannedTime = time(0) + itsPlannedItv;
				itsNextPlannedTime -= (itsNextPlannedTime % itsPlannedItv);
			}
			port.cancelTimer(itsSecondTimer);
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
	//
	// That must be events from the ObservationControllers that are currently 
	// started or running.
	//
	case CONTROL_STARTED: {
		// Child control received a message from the startDaemon that the
		// observationController was started (or not)
		CONTROLStartedEvent	msg(event);
		if (msg.successful) {
			LOG_DEBUG_STR("Start of " << msg.cntlrName << 
						  " was successful, waiting for connection.");
		}
		else {
			LOG_ERROR_STR("Observation controller " << msg.cntlrName <<
						  " could not be started");
			LOG_INFO_STR("Observation is be removed from administration, " << 
						 "restart will occur in next cycle");
			itsControllerMap.erase(msg.cntlrName);
		}
		break;
	}

	case CONTROL_CONNECTED: {
		// The observationController has registered itself at childControl.
		CONTROLConnectedEvent conEvent(event);
		LOG_DEBUG_STR(conEvent.cntlrName << " is connected, updating SAS)");

		// Ok, controller is really up, update SAS so that obs will not appear in
		// in the SAS list again.
		CMiter	theObs(itsControllerMap.find(conEvent.cntlrName));
		if (theObs == itsControllerMap.end()) {
			LOG_WARN_STR("Cannot find controller " << conEvent.cntlrName << 	
						  ". Can't update the SAS database");
			break;
		}
		OTDB::TreeMaintenance	tm(itsOTDBconnection);
		TreeStateConv			tsc(itsOTDBconnection);
		tm.setTreeState(theObs->second, tsc.get("queued"));
		break;
	}

	case CONTROL_QUITED: {
		// The observationController is going down.
		CONTROLQuitedEvent quitedEvent(event);
		LOG_DEBUG_STR("Received QUITED(" << quitedEvent.cntlrName << "," << quitedEvent.result << ")");

		// update SAS database.
		CMiter	theObs(itsControllerMap.find(quitedEvent.cntlrName));
		if (theObs == itsControllerMap.end()) {
			LOG_WARN_STR("Cannot find controller " << quitedEvent.cntlrName << 	
						  ". Can't update the SAS database");
			break;
		}
		OTDB::TreeMaintenance	tm(itsOTDBconnection);
		TreeStateConv			tsc(itsOTDBconnection);
		if (quitedEvent.result == CT_RESULT_NO_ERROR) {
			tm.setTreeState(theObs->second, tsc.get("finished"));
		}
		else {
			tm.setTreeState(theObs->second, tsc.get("aborted"));
		}

		// update our administration
		LOG_DEBUG_STR("Removing observation " << quitedEvent.cntlrName << 
						" from activeList");
//		_removeActiveObservation(quitedEvent.cntlrName);
		break;
	}

	case CONTROL_RESUMED: {
		// update SAS database.
		CONTROLResumedEvent		msg(event);
		CMiter	theObs(itsControllerMap.find(msg.cntlrName));
		if (theObs == itsControllerMap.end()) {
			LOG_WARN_STR("Cannot find controller " << msg.cntlrName << 	
						  ". Can't update the SAS database");
			break;
		}
		OTDB::TreeMaintenance	tm(itsOTDBconnection);
		TreeStateConv			tsc(itsOTDBconnection);
		tm.setTreeState(theObs->second, tsc.get("active"));
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
// finishing_state(event, port)
//
// Write controller state to PVSS, wait for 1 second (using a timer) to let GCF handle the property change
// and close the controller
//
GCFEvent::TResult MACScheduler::finishing_state(GCFEvent& event, GCFPortInterface& port)
{
	LOG_DEBUG_STR ("finishing_state:" << eventName(event) << "@" << port.getName());

	GCFEvent::TResult status = GCFEvent::HANDLED;

	switch (event.signal) {
	case F_INIT:
		break;

	case F_ENTRY: {
		// update PVSS
		itsPropertySet->setValue(PN_FSM_CURRENT_ACTION, GCFPVString("finished"));
		itsPropertySet->setValue(PN_FSM_ERROR, 			GCFPVString(""));
		itsPropertySet->setValue(PN_MS_OTDB_CONNECTED,  GCFPVBool  (false));

		itsTimerPort->setTimer(1L);
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
// finish
//
// Make the transition to the finishing state
//
void MACScheduler::finish()
{
  TRAN(MACScheduler::finishing_state);
}

//
// _doOTDBcheck
//
// Check if a new action should be taken based on the contents of OTDB and our own
// administration.
//
void MACScheduler::_doOTDBcheck()
{
	// update PVSS database with polltime
	time_t		now = time(0);
	ptime	currentTime = from_time_t(now);
	itsPropertySet->setValue(string(PN_MS_OTDB_LAST_POLL), GCFPVString(to_simple_string(currentTime)));

	// always update planned list because we might need to start some of those
	// (and we assumed that the PlannedItv was the smallest)
	_updatePlannedList();

	// update active lists when its time to do so.
	if (now >= itsNextActiveTime) {
		_updateActiveList();
		while (itsNextActiveTime <= now) {
			itsNextActiveTime += itsActiveItv;
		}
	}

	// update finished lists when its time to do so.
	if (now >= itsNextFinishedTime) {
		_updateFinishedList();
		while (itsNextFinishedTime <= now) {
			itsNextFinishedTime += itsFinishedItv;
		}
	}
}

//
// _updatePlannedList()
//
void MACScheduler::_updatePlannedList()
{
	LOG_DEBUG("_updatePlannedList()");

	// get new list (list is ordered on starttime)
	vector<OTDBtree> plannedDBlist = itsOTDBconnection->getTreeGroup(1, itsPlannedPeriod);	// planned observations

	if (!plannedDBlist.empty()) {
		LOG_DEBUG(formatString("OTDBCheck:First planned observation is at %s (tree=%d)", 
				to_simple_string(plannedDBlist[0].starttime).c_str(), plannedDBlist[0].treeID()));
	}
	// NOTE: do not exit routine on emptylist: we need to write an empty list to clear the DB

	// walk through the list, prepare PVSS for the new obs, update own admin lists.
	GCFPValueArray	plannedArr;
	uint32			listSize = plannedDBlist.size();
	uint32			idx = 0;
	time_t			now = time(0);
	ptime			currentTime = from_time_t(now);
	ASSERTSTR (currentTime != not_a_date_time, "Can't determine systemtime, bailing out");

	while (idx < listSize)  {
		// construct name and timings info for observation
		treeIDType		obsID = plannedDBlist[idx].treeID();
		string			obsName(observationName(obsID));

		// must we claim this observation at the claimMgr?
		OLiter	prepIter = itsPreparedObs.find(obsID);
		if ((prepIter == itsPreparedObs.end()) || (prepIter->second == false)) {
			// create a ParameterFile for this Observation
			TreeMaintenance		tm(itsOTDBconnection);
			OTDBnode			topNode = tm.getTopNode(obsID);
			string				filename(observationParset(obsID));
			if (!tm.exportTree(obsID, topNode.nodeID(), filename)) {
				LOG_ERROR_STR ("Cannot create ParameterSet '" << filename << 
								"' for new observation. Observation CANNOT BE STARTED!");
			}
			else {
				// Claim a DP in PVSS and write obssettings to it so the operator can see it.
				LOG_DEBUG_STR("Requesting preparation of PVSS for " << obsName);
				itsClaimerTask->prepareObservation(obsName);
				itsPreparedObs[obsID] = false;	// requested claim but no answer yet.
			}
		}
		else {
			// only add observations to the PVSS list when the claim was succesfull
			// otherwise thing will go wrong in the Navigator
			plannedArr.push_back(new GCFPVString(obsName));
		}
	
		// should this observation (have) be(en) started?
		time_duration	timeBeforeStart(plannedDBlist[idx].starttime - currentTime);
//		LOG_TRACE_VAR_STR(obsName << " starts over " << timeBeforeStart << " seconds");
		if (timeBeforeStart > seconds(0) && timeBeforeStart <= seconds(itsQueuePeriod)) {
			if (itsPreparedObs[obsID] == false) {
				LOG_ERROR_STR("Observation " << obsID << " must be started but is not claimed yet.");
			}
			else {
				// starttime of observation lays in queuePeriod. Start the controller-chain,
				// this will result in CONTROL_STARTED event in our main task
				// Note: as soon as the ObservationController has reported itself to the MACScheduler
				//		 the observation will not be returned in the 'plannedDBlist' anymore.
				string	cntlrName(controllerName(CNTLRTYPE_OBSERVATIONCTRL, 0, obsID));
				LOG_DEBUG_STR("Requesting start of " << cntlrName);
				itsChildControl->startChild(CNTLRTYPE_OBSERVATIONCTRL, 
											obsID, 
											0,		// instanceNr
											myHostname(true));
				// Note: controller is now in state NO_STATE/CONNECTED (C/R)

				// add controller to our 'monitor' administration
				itsControllerMap[cntlrName] =  obsID;
				LOG_DEBUG_STR("itsControllerMap[" << cntlrName << "]=" <<  obsID);
			}
		}
		idx++;
	} // while processing all planned obs'

	// Finally we can pass the list with planned observations to PVSS.
	itsPropertySet->setValue(PN_MS_PLANNED_OBSERVATIONS, GCFPVDynArr(LPT_DYNSTRING, plannedArr));

}

//
// _updateActiveList()
//
void MACScheduler::_updateActiveList()
{
	LOG_DEBUG("_updateActiveList()");

	// get new list (list is ordered on starttime)
	vector<OTDBtree> activeDBlist = itsOTDBconnection->getTreeGroup(2, 0);
	if (activeDBlist.empty()) {
		LOG_DEBUG ("No active Observations");
		// NOTE: do not exit routine on emptylist: we need to write an empty list to clear the DB
	}

	// walk through the list, prepare PVSS for the new obs, update own admin lists.
	GCFPValueArray	activeArr;
	uint32			listSize = activeDBlist.size();
	uint32			idx = 0;
	while (idx < listSize)  {
		// construct name and timings info for observation
		string		obsName(observationName(activeDBlist[idx].treeID()));
		activeArr.push_back(new GCFPVString(obsName));

		// remove obs from planned-list if its still their.
		OLiter	prepIter = itsPreparedObs.find(activeDBlist[idx].treeID());
		if (prepIter != itsPreparedObs.end()) {
			itsPreparedObs.erase(prepIter);
		}

		idx++;
	} // while

	// Finally we can pass the list with active observations to PVSS.
	itsPropertySet->setValue(PN_MS_ACTIVE_OBSERVATIONS,	GCFPVDynArr(LPT_DYNSTRING, activeArr));
}

//
// _updateFinishedList()
//
void MACScheduler::_updateFinishedList()
{
	LOG_DEBUG("_updateFinishedList()");

	// get new list (list is ordered on starttime)
	vector<OTDBtree> finishedDBlist = itsOTDBconnection->getTreeGroup(3, itsFinishedPeriod);
	if (finishedDBlist.empty()) {
		LOG_DEBUG ("No finished Observations");
		// NOTE: do not exit routine on emptylist: we need to write an empty list to clear the DB
	}

	// walk through the list, prepare PVSS for the new obs, update own admin lists.
	GCFPValueArray	finishedArr;
	uint32			listSize = finishedDBlist.size();
	uint32			idx = 0;
	while (idx < listSize)  {
		// construct name and timings info for observation
		string		obsName(observationName(finishedDBlist[idx].treeID()));
		finishedArr.push_back(new GCFPVString(obsName));
		idx++;
	} // while

	// Finally we can pass the list with finished observations to PVSS.
	itsPropertySet->setValue(PN_MS_FINISHED_OBSERVATIONS,
								GCFPVDynArr(LPT_DYNSTRING, finishedArr));
}


//
// _connectedHandler(port)
//
void MACScheduler::_connectedHandler(GCFPortInterface& /*port*/)
{
}

//
// _disconnectedHandler(port)
//
void MACScheduler::_disconnectedHandler(GCFPortInterface& port)
{
	port.close();
}


};
};
