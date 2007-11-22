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

#include <APS/ParameterSet.h>
#include <GCF/TM/GCF_Protocols.h>
#include <GCF/GCF_ServiceInfo.h>
#include <GCF/GCF_PVTypes.h>
#include <GCF/Utils.h>
#include <APL/APLCommon/APL_Defines.h>
#include <APL/APLCommon/ControllerDefines.h>
#include <APL/APLCommon/StationInfo.h>
#include <APL/APLCommon/Controller_Protocol.ph>
#include <GCF/RTDB/DP_Protocol.ph>
#include <OTDB/TreeStateConv.h>
#include <signal.h>

#include "MACSchedulerDefines.h"
#include "MACScheduler.h"

using namespace LOFAR::GCF::Common;
using namespace LOFAR::GCF::TM;
using namespace LOFAR::GCF::RTDB;
using namespace LOFAR::OTDB;
using namespace LOFAR::Deployment;
using namespace std;

namespace LOFAR {
	using namespace APLCommon;
	using namespace ACC::APS;
	namespace MainCU {

// static (this) pointer used for signal handling
static MACScheduler* pMacScheduler = 0;
	
//
// MACScheduler()
//
MACScheduler::MACScheduler() :
	GCFTask 			((State)&MACScheduler::initial_state,string(MS_TASKNAME)),
	itsPropertySet		(0),
	itsObservations		(),
	itsPVSSObsList		(),
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

	LOG_INFO_STR("MACProcessScope: " << globalParameterSet()->getString("prefix"));

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

	itsObservations.reserve(10);		// already reserve memory for 10 observations.

	GCF::TM::registerProtocol(CONTROLLER_PROTOCOL, CONTROLLER_PROTOCOL_STRINGS);
	GCF::TM::registerProtocol(DP_PROTOCOL, 		   DP_PROTOCOL_STRINGS);
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

	LOG_DEBUG_STR ("_databaseEventHandler:" << eventName(event));

	switch(event.signal) {
	case DP_CHANGED: {
		DPChangedEvent	dpEvent(event);

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
		itsPropertySet->setValue(PVSSNAME_FSM_CURACT,     GCFPVString  ("initial"));
		itsPropertySet->setValue(PVSSNAME_FSM_ERROR,      GCFPVString  (""));
		itsPropertySet->setValue(PN_MS_OTDB_CONNECTED,    GCFPVBool    (false));
		itsPropertySet->setValue(PN_MS_OTDB_LAST_POLL,    GCFPVString  (""));
		itsPropertySet->setValue(PN_MS_OTDB_POLLINTERVAL, GCFPVInteger (itsOTDBpollInterval));
		itsPropertySet->setValue(PN_MS_ACTIVE_OBSERVATIONS, GCFPVDynArr(LPT_STRING, itsPVSSObsList));

      
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
		itsPropertySet->setValue(PN_MS_OTDB_CONNECTED, GCFPVBool(true));

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
	LOG_DEBUG_STR ("recover_state:" << eventName(event) << "@" << port.getName());

	GCFEvent::TResult status = GCFEvent::HANDLED;

	switch (event.signal) {
	case F_INIT:
		break;

	case F_ENTRY: {
		// update PVSS
		itsPropertySet->setValue(string(PVSSNAME_FSM_CURACT),GCFPVString("recover"));
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
		itsPropertySet->setValue(string(PVSSNAME_FSM_CURACT),GCFPVString("active"));
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

	case DP_CHANGED:
		_databaseEventHandler(event);
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

	case CONTROL_STARTED: {
		// Child control received a message from the startDaemon that the
		// observationController was started (or not)
		CONTROLStartedEvent	msg(event);
		if (msg.successful) {
			LOG_DEBUG_STR("Start of " << msg.cntlrName << 
						  " was successful, waiting for connection.");
			// ---
			// Ok, controller is really up, update SAS so that obs will not appear in
			// in the SAS list again.
			Observation*	theObs(_findActiveObservation(msg.cntlrName));
			if (!theObs) {
				LOG_WARN_STR("Cannot find controller " << msg.cntlrName << 	
							  ". Can't update the SAS database");
				break;
			}
			OTDB::TreeMaintenance	tm(itsOTDBconnection);
			TreeStateConv			tsc(itsOTDBconnection);
			tm.setTreeState(theObs->treeID, tsc.get("queued"));
			// ---
		}
		else {
			LOG_ERROR_STR("Observation controller " << msg.cntlrName <<
						  " could not be started");
			LOG_INFO_STR("Observation is be removed from administration, " << 
						 "restart will occur in next cycle");
			_removeActiveObservation(msg.cntlrName);
		}
		break;
	}

// TODO: the ObsControls don't send the CONTROL_CONNECT yet.
// so the code is copied to the CONTROL STARTED event for now.
	case CONTROL_CONNECT: {
		// The observationController has registered itself at childControl.
		CONTROLConnectEvent conEvent(event);
		LOG_DEBUG_STR(conEvent.cntlrName << " is connected, updating SAS)");

		// Ok, controller is really up, update SAS so that obs will not appear in
		// in the SAS list again.
		Observation*	theObs(_findActiveObservation(conEvent.cntlrName));
		if (!theObs) {
			LOG_WARN_STR("Cannot find controller " << conEvent.cntlrName << 	
						  ". Can't update the SAS database");
			break;
		}
		OTDB::TreeMaintenance	tm(itsOTDBconnection);
		TreeStateConv			tsc(itsOTDBconnection);
		tm.setTreeState(theObs->treeID, tsc.get("queued"));
		break;
	}

	case CONTROL_QUITED: {
		// The observationController is going down.
		CONTROLQuitedEvent quitedEvent(event);
		LOG_DEBUG_STR("Received QUITED(" << quitedEvent.cntlrName << "," << quitedEvent.result << ")");

		// update SAS database.
		Observation*	theObs(_findActiveObservation(quitedEvent.cntlrName));
		if (!theObs) {
			LOG_WARN_STR("Cannot find controller " << quitedEvent.cntlrName << 	
						  ". Can't update the SAS database");
			break;
		}
		OTDB::TreeMaintenance	tm(itsOTDBconnection);
		TreeStateConv			tsc(itsOTDBconnection);
		if (quitedEvent.result == CT_RESULT_NO_ERROR) {
			tm.setTreeState(theObs->treeID, tsc.get("finished"));
		}
		else {
			tm.setTreeState(theObs->treeID, tsc.get("aborted"));
		}

		// update our administration
		LOG_DEBUG_STR("Removing observation " << quitedEvent.cntlrName << 
						" from activeList");
		_removeActiveObservation(quitedEvent.cntlrName);
		break;
	}

	case CONTROL_RESUMED: {
		// update SAS database.
		CONTROLResumedEvent		msg(event);
		Observation*			theObs(_findActiveObservation(msg.cntlrName));
		if (!theObs) {
			LOG_WARN_STR("Cannot find controller " << msg.cntlrName << 	
						  ". Can't update the SAS database");
			break;
		}
		OTDB::TreeMaintenance	tm(itsOTDBconnection);
		TreeStateConv			tsc(itsOTDBconnection);
		tm.setTreeState(theObs->treeID, tsc.get("active"));
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
		itsPropertySet->setValue(string(PVSSNAME_FSM_CURACT),GCFPVString("finished"));
		itsPropertySet->setValue(string(PVSSNAME_FSM_ERROR), GCFPVString(""));
		itsPropertySet->setValue(PN_MS_OTDB_CONNECTED,       GCFPVBool  (false));

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
	ptime	currentTime = from_time_t(time(0));
	itsPropertySet->setValue(string(PN_MS_OTDB_LAST_POLL),
										GCFPVString(to_simple_string(currentTime)));

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
	ASSERTSTR (currentTime != not_a_date_time, "Can't determine systemtime, bailing out");

	while (idx < listSize)  {
		// timediff = time to go before start of Observation
		time_duration	timediff = newTreeList[idx].starttime - currentTime;
		LOG_TRACE_VAR_STR("timediff=" << timediff);

		// when queuetime is not reached yet we are finished with the list.
		if (timediff > seconds(itsQueuePeriod)) {
			break;
		}

		// note: as soon as observation is up it will not show up anymore in the
		// SAS list because we changed the state.
		// If startup is in progress, it is in the SAS list but also in our admin.
		string		cntlrName = controllerName(CNTLRTYPE_OBSERVATIONCTRL, 
												0, newTreeList[idx].treeID());
		if (_findActiveObservation(cntlrName)) {
			LOG_DEBUG_STR("Skipping " << cntlrName);
			idx++;
			continue;
		}

		// get current state of Observation
//		CTState		cts;
//		CTState::CTstateNr	curState      = itsChildControl->getCurrentState(cntlrName);
//		LOG_DEBUG_STR(cntlrName << ":cur=" << cts.name(curState));

		// When in startup or claimtime we should try to start the controller.
//		if ((timediff > seconds(0)) && (curState < CTState::CREATED)) {

		// Obs is unknown so try to start it up as long as we didn't reach its starttime.
		if (timediff > seconds(0)) {
			// Let database construct the parset for the whole observation
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
				itsChildControl->startChild(CNTLRTYPE_OBSERVATIONCTRL, 
											treeID, 
											0,		// instanceNr
											myHostname(true));
				// Note: controller is now in state NO_STATE/CONNECTED (C/R)

				// register this Observation
				ParameterSet	obsPS(filename);
				Observation		newObs(&obsPS);
				newObs.name   = cntlrName;
				newObs.treeID = treeID;
				_addActiveObservation(newObs);
				LOG_DEBUG_STR("Observation " << cntlrName << " added to active Observations");
			}

			// TODO: due to problems with the PA we only start one obs every cycle.
			break;
//			idx++;
//			continue;
		}

#if 0
		// in CLAIM period?
		if ((timediff > seconds(0)) && (timediff <= seconds(itsClaimPeriod))) {
			// Observation is somewhere in the claim period its should be up by now.
			if (curState < CTState::CLAIMED) {
				LOG_ERROR_STR("Observation " << cntlrName << 
							" should have reached the CLAIMING state by now," <<
							" check state of observation.");
				LOG_DEBUG_STR("Its state is: " << cts.name(curState));
			}
			idx++;
			continue;
		}

		// observation must be running (otherwise it would not be in the newTreeList)
		// TODO: check if endtime is reached and observation is still running.
	
		idx++;	
#endif
	} // while

}

//
// _findActiveObservation(name)
//
Observation*	MACScheduler::_findActiveObservation(const string&	name)
{
	vector<Observation>::iterator	end  = itsObservations.end();
	vector<Observation>::iterator	iter = itsObservations.begin();
	while (iter != end) {
		if (iter->name == name) {
			return (&(*iter));
		}
		iter++;
	}

	return ((Observation*) 0);
}

//
// _addActiveObservation(name)
//
void MACScheduler::_addActiveObservation(const Observation&	newObs)
{
	// Observation already in vector?
	vector<Observation>::iterator	end  = itsObservations.end();
	vector<Observation>::iterator	iter = itsObservations.begin();
	while (iter != end) {
		if (iter->name == newObs.name) {
			return;
		}
		iter++;
	}

	// update own admin and PVSS datapoint
	itsObservations.push_back(newObs);
	itsPVSSObsList.push_back(new GCFPVString(formatString("Observation%d", newObs.treeID)));
	itsPropertySet->setValue(PN_MS_ACTIVE_OBSERVATIONS, GCFPVDynArr(LPT_STRING, itsPVSSObsList));

	LOG_DEBUG_STR("Added observation " << newObs.name << " to active observation-list");

}


//
// _removeActiveObservation(name)
//
void MACScheduler::_removeActiveObservation(const string& name)
{
	// search observation.
	OTDB::treeIDType		treeID;
	vector<Observation>::iterator	end  = itsObservations.end();
	vector<Observation>::iterator	iter = itsObservations.begin();
	bool	found(false);
	while (!found && (iter != end)) {
		if (iter->name == name) {
			found = true;
			treeID = iter->treeID;
			itsObservations.erase(iter);
			LOG_DEBUG_STR("Removed observation " << name << " from active observationList");
		}
		iter++;
	}

	if (!found) {
		return;
	}

	string		obsName(formatString("Observation%d", treeID));
	GCFPValueArray::iterator	pEnd  = itsPVSSObsList.end();
	GCFPValueArray::iterator	pIter = itsPVSSObsList.begin();
	while (pIter != pEnd) {
		if ((static_cast<GCFPVString*>(*pIter))->getValue() == obsName) {
			delete 	*pIter;
			itsPVSSObsList.erase(pIter);
			break;
		}
		pIter++;
	}
	itsPropertySet->setValue(PN_MS_ACTIVE_OBSERVATIONS, GCFPVDynArr(LPT_STRING, itsPVSSObsList));
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
