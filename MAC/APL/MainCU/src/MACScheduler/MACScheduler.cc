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
#include <GCF/PAL/GCF_PVSSInfo.h>
#include <GCF/Utils.h>
#include <APL/APLCommon/APL_Defines.h>
#include <APL/APLCommon/APLCommonExceptions.h>

#include "MACSchedulerDefines.h"
#include "MACScheduler.h"

using namespace LOFAR::GCF::Common;
using namespace LOFAR::GCF::TM;
using namespace LOFAR::GCF::PAL;
using namespace LOFAR::OTDB;
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
	itsDummyPort		(0),
	itsChildControl		(0),
	itsSecondTimer		(0),
	itsQueuePeriod		(0),
	itsClaimPeriod		(0),
	itsOTDBconnection	(0),
	itsOTDBpollInterval	(0),
	itsNextOTDBpolltime (0)
{
	LOG_TRACE_OBJ ("MACscheduler construction");

#ifndef USE_PVSSPORT
	LOG_WARN("Using GCFTCPPort in stead of GCFPVSSPort");
#endif

	// Readin some parameters from the ParameterSet.
	itsOTDBpollInterval = globalParameterSet()->getTime("OTDBpollInterval");
	itsQueuePeriod 		= globalParameterSet()->getTime("QueuePeriod");
	itsClaimPeriod 		= globalParameterSet()->getTime("ClaimPeriod");

	// get pointer to childcontrol task
	itsChildControl = ChildControl::instance();
}


//
// ~MACScheduler()
//
MACScheduler::~MACScheduler()
{
	LOG_TRACE_OBJ ("~MACscheduler");

	if (itsPropertySet) {
		itsPropertySet->setValue(string(PVSSNAME_FSM_STATE),GCFPVString("down"));
		itsPropertySet->disable();
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
	switch(answer.signal) {
	case F_MYPS_ENABLED: {
		GCFPropSetAnswerEvent* pPropAnswer=static_cast<GCFPropSetAnswerEvent*>(&answer);
		if(pPropAnswer->result != GCF_NO_ERROR) {
			LOG_ERROR(formatString("%s : PropertySet %s NOT ENABLED",
										getName().c_str(), pPropAnswer->pScope));
		}
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
	LOG_DEBUG ("MACScheduler::initial_state");

	GCFEvent::TResult status = GCFEvent::HANDLED;
  
	switch (event.signal) {
    case F_INIT:
		LOG_TRACE_FLOW("initial_state:F_INIT");
   		break;

	case F_ENTRY: {
		LOG_TRACE_FLOW("initial_state:F_ENTRY");
		// Get access to my own propertyset.
		LOG_DEBUG ("Activating PropertySet");
		itsPropertySet = GCFMyPropertySetPtr(new GCFMyPropertySet(MS_PROPSET_NAME,
																  MS_PROPSET_TYPE,
																  PS_CAT_PERM_AUTOLOAD,
																  &itsPropertySetAnswer));
		itsPropertySet->enable();
	  
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

		// need port for timers(!)
		itsDummyPort = new GCFTimerPort(*this, "MACScheduler:dummy4timers");
		TRAN(MACScheduler::recover_state);				// go to next state.
		}
		break;
#if 0
	case F_CONNECTED:
		break;

	case F_DISCONNECTED:
		break;
	
	case F_TIMER:
		break;
#endif  
	default:
		LOG_DEBUG_STR ("MACScheduler(" << getName() << ")::initial_state, default");
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
	LOG_DEBUG ("MACScheduler::recover_state");

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
		LOG_DEBUG(formatString("MACScheduler(%s)::recover_state, default",getName().c_str()));
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
	LOG_DEBUG ("MACScheduler::active_state");

	GCFEvent::TResult status = GCFEvent::HANDLED;

	switch (event.signal) {
	case F_INIT:
		break;

	case F_ENTRY: {
		// update PVSS
		itsPropertySet->setValue(string(PVSSNAME_FSM_STATE),GCFPVString("active"));
		itsPropertySet->setValue(string(PVSSNAME_FSM_ERROR),GCFPVString(""));

		// Timers must be connected to ports, so abuse serverPort for second timer.
		itsSecondTimer = itsDummyPort->setTimer(1L);
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

	case LOGICALDEVICE_SCHEDULED: {
		LOGICALDEVICEScheduledEvent scheduledEvent(event);
		// ...
		break;
	}

	case LOGICALDEVICE_SCHEDULECANCELLED: {
		LOGICALDEVICESchedulecancelledEvent schedulecancelledEvent(event);
		// ...
		break;
	}

	default:
		LOG_DEBUG(formatString("MACScheduler(%s)::active_state, default",
								getName().c_str()));
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

		// when queuetime is not reached yet were are finished with the list.
		if (timediff > seconds(itsQueuePeriod)) {
			break;
		}

		// get current state of Observation
		string		cntlrName = formatString("ObsCntlr_%d", newTreeList[idx].treeID());
		LDState::LDstateNr	observationState = itsChildControl->getRequestedState(cntlrName);

		// remember: timediff <= queueperiod
		if (timediff > seconds(itsClaimPeriod)) {
			// Observation is somewhere in the queueperiod
			if (observationState != LDState::CONNECTED) {	// requested a start before?
				// no, let database construct the parset for the whole observation
				OTDB::TreeMaintenance	tm(itsOTDBconnection);
				OTDB::treeIDType		treeID = newTreeList[idx].treeID();
				OTDBnode				topNode = tm.getTopNode(treeID);
				string					filename = formatString("%s/Observation_%d", 
														LOFAR_SHARE_LOCATION, treeID);
				if (!tm.exportTree(treeID, topNode.nodeID(), filename)) {
					LOG_ERROR_STR ("Cannot create startup file " << filename << 
								" for new observation. Observation CANNOT BE STARTED!");
				}
				else {
					// fire request for new controller
					itsChildControl->startChild(cntlrName, 
											treeID, 
											LDTYPE_OBSERVATIONCTRL, 
											myHostname());
				}
				idx++;
				continue;
			}
		}

		if (timediff > seconds(0)) {
			// Observation is somewhere in the claim period
			if (observationState != LDState::CLAIMED) {
//				_claimObservation(&newTreeList[idx]);
				idx++;
				continue;
			}
		}

		// observation must be running (otherwise it would not be in the newTreeList)
		if (observationState != LDState::ACTIVE) {
//			_executeObservation(&newTreeList[idx]);
		}
	
		idx++;	
	}

}

//
// readObservationParameters(ObsTreeID)
//
// Ask the OTDB to create an ParameterSet of the given Tree.
//
boost::shared_ptr<ACC::APS::ParameterSet> 
	MACScheduler::readObservationParameters(OTDB::treeIDType ObsTreeID)
{
	// Convert treeId to nodeID of top node.
	TreeMaintenance tm(itsOTDBconnection);
	OTDBnode topNode = tm.getTopNode(ObsTreeID);
	LOG_INFO_STR(topNode);

	// construct the filename
	string tempFileName = string(LOFAR_SHARE_LOCATION) + "/Obs_" + toString(ObsTreeID);

	// read the parameterset from the database:
	LOG_INFO(formatString("Exporting tree %d to '%s'",
										ObsTreeID, tempFileName.c_str()));
	if (!tm.exportTree(ObsTreeID, topNode.nodeID(), tempFileName)) {
		THROW(APLCommon::OTDBException, string("Unable to export tree ") + 
									toString(ObsTreeID) + " to " + tempFileName);
	}

	// read file into ParameterSet
	boost::shared_ptr<ACC::APS::ParameterSet> ps;
	ps.reset(new ACC::APS::ParameterSet(tempFileName));

//	createChildsSections (tm, ObsTreeID, topNode.nodeID(), string(""), ps);

	return (ps);
}

#if 0

//
// _schedule (rootID, port)
//
// One way or another they start an observation by creating and modifying a
// parameterSet, allocation beams?? and sending one startDaemon a schedule event.
//
void MACScheduler::_schedule(const string& VIrootID, GCFPortInterface* /*port*/)
{  
	string shareLocation = _getShareLocation();	//REO
	try {
		boost::shared_ptr<ACC::APS::ParameterSet> ps = _readParameterSet(VIrootID);

		// replace the parent port (assigned by the ServiceBroker)
		unsigned int parentPort = itsVIparentPort.getPortNumber();
		ACC::APS::KVpair kvPair(string("parentPort"),(int)parentPort);
		ps->replace(kvPair);

		// get some parameters and write it to the allocated CCU
		string allocatedCCU = ps->getString("allocatedCCU");
		string viName = ps->getString("name");
		string parameterFileName = viName + string(".param");

		// make all relative times absolute
//		_convertRelativeTimes(ps);

		string ldTypeString = ps->getString("logicalDeviceType");
		TLogicalDeviceTypes ldTypeRoot = APLUtilities::convertLogicalDeviceType(ldTypeString);

		bool allocationOk = true;
		TSASResult sasResult(SAS_RESULT_NO_ERROR);

#if 0
		// find the subbands allocations in VI sections
		vector<string> childKeys = ps->getStringVector("childs");
		for(vector<string>::iterator childsIt=childKeys.begin();
						allocationOk && childsIt!=childKeys.end();++childsIt) {
			string ldTypeString = ps->getString(*childsIt + ".logicalDeviceType");
			TLogicalDeviceTypes ldType = APLUtilities::convertLogicalDeviceType(ldTypeString);
			if(ldType == LDTYPE_VIRTUALINSTRUMENT) { //REO
				// allocate beamlets for VI's
				allocationOk = _allocateBeamlets(VIrootID, ps, *childsIt);
				if(!allocationOk) {
					sasResult = SAS_RESULT_ERROR_BEAMLET_ALLOCATION_FAILED;
				}
				else {
					allocationOk = _allocateLogicalSegments(VIrootID, ps, *childsIt);
					if(!allocationOk) {
						sasResult = SAS_RESULT_ERROR_LOGICALSEGMENT_ALLOCATION_FAILED;
					}
				}
			}
		}
		if(!allocationOk) {
			SASResponseEvent sasResponseEvent;
			sasResponseEvent.result = sasResult;
			itsPropertySet->setValue(string(MS_PROPNAME_STATUS),GCFPVInteger(sasResponseEvent.result));
		}
		else
#endif
		{
			string tempFileName = APLUtilities::getTempFileName();
			ps->writeFile(tempFileName);
			APLUtilities::remoteCopy(tempFileName,allocatedCCU,shareLocation+parameterFileName);
			remove(tempFileName.c_str());

			// send the schedule event to the VI-StartDaemon on the CCU
			STARTDAEMONScheduleEvent sdScheduleEvent;
			sdScheduleEvent.logicalDeviceType = ldTypeRoot;
			sdScheduleEvent.taskName = viName;
			sdScheduleEvent.fileName = parameterFileName;

			TStringRemotePortMap::iterator it = itsVISDclientPorts.find(allocatedCCU);
			if(it != itsVISDclientPorts.end()) {
				it->second->send(sdScheduleEvent);
			}
			else {
				SASResponseEvent sasResponseEvent;
				sasResponseEvent.result = SAS_RESULT_ERROR_VI_NOT_FOUND;
				itsPropertySet->setValue(string(MS_PROPNAME_STATUS),GCFPVInteger(sasResponseEvent.result));
			}
		}        
	}
	catch(Exception& e) {
		LOG_FATAL(formatString("Error reading schedule parameters: %s",e.message().c_str()));
		SASResponseEvent sasResponseEvent;
		sasResponseEvent.result = SAS_RESULT_ERROR_UNSPECIFIED;
		itsPropertySet->setValue(string(MS_PROPNAME_STATUS),GCFPVInteger(sasResponseEvent.result));
	}
	catch(exception& e) {
		LOG_FATAL(formatString("Error reading schedule parameters: %s",e.what()));
		SASResponseEvent sasResponseEvent;
		sasResponseEvent.result = SAS_RESULT_ERROR_UNSPECIFIED;
		itsPropertySet->setValue(string(MS_PROPNAME_STATUS),GCFPVInteger(sasResponseEvent.result));
	}
}


//
// _updateSchedule(rootVI, port)
//
void MACScheduler::_updateSchedule(const string& VIrootID, GCFPortInterface* port)
{
	string shareLocation = _getShareLocation();

	// search the port of the VI
	try
	{
		boost::shared_ptr<ACC::APS::ParameterSet> ps = _readParameterSet(VIrootID);

		// replace the parent port (assigned by the ServiceBroker)
		unsigned int parentPort = itsVIparentPort.getPortNumber();
		ACC::APS::KVpair kvPair(string("parentPort"),(int)parentPort);
		ps->replace(kvPair);

		string allocatedCCU = ps->getString("allocatedCCU");
		string viName = ps->getString("name");
		string parameterFileName = viName + string(".param");

		// make all relative times absolute
		_convertRelativeTimes(ps);

		string tempFileName = APLUtilities::getTempFileName();
		ps->writeFile(tempFileName);
		APLUtilities::remoteCopy(tempFileName,allocatedCCU,shareLocation+parameterFileName);
		remove(tempFileName.c_str());

		// send a SCHEDULE message
		TStringRemotePortMap::iterator it = itsconnectedVIclientPorts.find(viName);
		if(it != itsconnectedVIclientPorts.end()) {
			LOGICALDEVICEScheduleEvent scheduleEvent;
			scheduleEvent.fileName = parameterFileName;

			it->second->send(scheduleEvent);
		}
		else {
			SASResponseEvent sasResponseEvent;
			sasResponseEvent.result = SAS_RESULT_ERROR_VI_NOT_FOUND;
			itsPropertySet->setValue(string(MS_PROPNAME_STATUS),GCFPVInteger(sasResponseEvent.result));
		}        
	}
	catch(Exception& e) {
		LOG_FATAL(formatString("Error reading schedule parameters: %s",e.message().c_str()));
		SASResponseEvent sasResponseEvent;
		sasResponseEvent.result = SAS_RESULT_ERROR_UNSPECIFIED;
		sasResponseEvent.VIrootID = VIrootID;

		if(port != 0) {
			port->send(sasResponseEvent);      
		}
		itsPropertySet->setValue(string(MS_PROPNAME_STATUS),GCFPVInteger(sasResponseEvent.result));
	}
}


//
// _cancelSchedule(rootVI, port)
//
void MACScheduler::_cancelSchedule(const string& VIrootID, GCFPortInterface* /*port*/)
{
	string shareLocation = _getShareLocation(); //REO

	// search the port of the VI
	try {
		boost::shared_ptr<ACC::APS::ParameterSet> ps = _readParameterSet(VIrootID);

		string viName = ps->getString("name");

		// send a CANCELSCHEDULE message
		TStringRemotePortMap::iterator it = itsconnectedVIclientPorts.find(viName);
		if(it != itsconnectedVIclientPorts.end()) {
			LOGICALDEVICECancelscheduleEvent cancelScheduleEvent;
			it->second->send(cancelScheduleEvent);
		}
		else {
			SASResponseEvent sasResponseEvent;
			sasResponseEvent.result = SAS_RESULT_ERROR_VI_NOT_FOUND;
			itsPropertySet->setValue(string(MS_PROPNAME_STATUS),GCFPVInteger(sasResponseEvent.result));
		}

	}
	catch(Exception& e)
	{
		LOG_FATAL(formatString("Error reading schedule parameters: %s",e.message().c_str()));
		SASResponseEvent sasResponseEvent;
		sasResponseEvent.result = SAS_RESULT_ERROR_UNSPECIFIED;
		itsPropertySet->setValue(string(MS_PROPNAME_STATUS),GCFPVInteger(sasResponseEvent.result));
	}
}

//
// _isServerPort(server, port)
//
bool MACScheduler::_isServerPort(const GCFPortInterface& server, 
								 const GCFPortInterface& port) const
{
  return (&port == &server); // comparing two pointers. yuck?
}
   

//
// _isVISDclientPort(port, visd)
//
bool MACScheduler::_isVISDclientPort(const GCFPortInterface& port, 
									 string& visd) const
{
	bool found=false;
	TStringRemotePortMap::const_iterator it=itsVISDclientPorts.begin();
	while(!found && it != itsVISDclientPorts.end()) {
		found = (&port == it->second.get()); // comparing two pointers. yuck?
		if(found) {
			visd = it->first;
		}
		++it;
	}
	return (found);
}


//
// _isVIclientPort(port)
//
bool MACScheduler::_isVIclientPort(const GCFPortInterface& port) const
{
	bool found=false;
	TRemotePortVector::const_iterator it=itsVIclientPorts.begin();
	while(!found && it != itsVIclientPorts.end()) {
		found = (&port == (*it).get()); // comparing two pointers. yuck?
		++it;
	}
	return (found);
}


//
// _getVInameFromPort(port)
//
string MACScheduler::_getVInameFromPort(const GCF::TM::GCFPortInterface& port) const
{
	string viName("");
	if(_isVIclientPort(port)) {
		bool found = false;
		TStringRemotePortMap::const_iterator it = itsconnectedVIclientPorts.begin();
		while(!found && it != itsconnectedVIclientPorts.end()) {
			found = (&port == it->second.get());
			if(found) {
				viName = it->first;
			}
			++it;
		}
	}
	return (viName);
}

#endif

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
