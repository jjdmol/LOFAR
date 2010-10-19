//#  ChildControl.cc: one line description
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

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

//# Includes
#include <sys/stat.h>
#include <Common/LofarLogger.h>
#include <Common/SystemUtil.h>
#include <Common/ParameterSet.h>
#include <ApplCommon/StationInfo.h>
#include <GCF/TM/GCF_Protocols.h>
#include <MACIO/MACServiceInfo.h>
#include <APL/APLCommon/APLUtilities.h>
#include <APL/APLCommon/ControllerDefines.h>
#include <APL/APLCommon/ChildControl.h>
#include <Controller_Protocol.ph>
#include <StartDaemon_Protocol.ph>
#include <boost/lexical_cast.hpp>
using namespace boost;

namespace LOFAR {
  using namespace GCF::TM;
  namespace APLCommon {

//-------------------------- creation and destroy ---------------------------

ChildControl* ChildControl::instance()
{
	static	ChildControl*		theirChildControl;

	if (theirChildControl == 0) {
		theirChildControl = new ChildControl();
	}
	return (theirChildControl);
}

//
// ChildControl()
//
ChildControl::ChildControl() :
	GCFTask			 		((State)&ChildControl::initial, "ChildControl"),
	itsListener		 		(0),
	itsTimerPort			(*this, "TimerPort"),
	itsStartDaemonMap		(),
	itsStartupRetryInterval	(10),
	itsMaxStartupRetries	(5),
	itsCntlrList	 		(0),
	itsActionList	 		(),
	itsActionTimer			(0),
	itsGarbageTimer			(0),
	itsGarbageInterval		(10),		// TODO: set to 300 for real life
	itsCompletionTimer		(0),
	itsCompletionPort		(0)
{
	// Log the protocols I use.
	registerProtocol(CONTROLLER_PROTOCOL, CONTROLLER_PROTOCOL_STRINGS);
	registerProtocol(STARTDAEMON_PROTOCOL,STARTDAEMON_PROTOCOL_STRINGS);

	// adopt optional redefinition of startup-retry settings
	if (globalParameterSet()->isDefined("ChildControl.StartupRetryInterval")) {
		itsStartupRetryInterval = globalParameterSet()->
									getInt32("ChildControl.StartupRetryInterval");
	}
	if (globalParameterSet()->isDefined("ChildControl.MaxStartupRetries")) {
		itsStartupRetryInterval = globalParameterSet()->
									getInt32("ChildControl.MaxStartupRetries");
	}
	if (globalParameterSet()->isDefined("ChildControl.GarbageCollectionInterval")) {
		itsGarbageInterval = globalParameterSet()->
									getInt32("ChildControl.GarbageCollectionInterval");
	}

	itsCntlrList = &(ControllerAdmin::instance()->itsList);

}

//
// ~ChildControl
//
ChildControl::~ChildControl()
{
	// close Listener
	if (itsListener) {
		itsListener->close();
		delete itsListener;
	}

	// close all connections with the startDaemons.
	map <string, GCFTCPPort*>::iterator			iter = itsStartDaemonMap.begin();
	map <string, GCFTCPPort*>::const_iterator	end  = itsStartDaemonMap.end();
	while (iter != end) {
		iter->second->close();
		iter++;
	}
	itsStartDaemonMap.clear();

	// clear controller list.
	itsCntlrList->clear();

	// clear action list.
	itsActionList.clear();
}

// -------------------- Control functions for the customer --------------------
//
// openService(servicename, instanceNr)
//
// Starts the listener for the childs under the given name.
//
void ChildControl::openService(const string&	aServiceName,
							   uint32			instanceNr)
{
	// can't set name only once
	if (itsListener) {
		return;
	}

	itsListener = new GCFTCPPort(*this, aServiceName, GCFPortInterface::MSPP, 
														STARTDAEMON_PROTOCOL);
	ASSERTSTR(itsListener, "Can't create a listener port for my children");

	itsListener->setInstanceNr (instanceNr);
	itsListener->autoOpen(5,0,1);	// 5 tries with 1 second interval.
}

//
// startChild (name, obsId, aCntlType, instanceNr, hostname)
//
bool ChildControl::startChild (uint16				aCntlrType, 
							   OTDBtreeIDType		anObsID, 
							   uint32				instanceNr,
							   const string&		aHostname)
{
	string	hostname(realHostname(aHostname));
	LOG_DEBUG_STR("startChild(" << aCntlrType <<","<< anObsID <<","<< instanceNr 
											  <<","<< hostname << ")");

	// first check if child already exists
	string	cntlrName(controllerName(aCntlrType, instanceNr, anObsID, hostname));
	if (findController(cntlrName) != itsCntlrList->end()) {
		LOG_DEBUG_STR("Controller " << cntlrName << " already running.");
		return (false);
	}	

	// make sure there is a parameterSet for the program.
	// search observation-parset for controller name and determine prefix
	// NOTE: this name must be the same as in the MACScheduler.
	string	baseSetName(observationParset(anObsID));
	if (anObsID == 0  && isSharedController(aCntlrType)) {
		LOG_DEBUG_STR("Startup of shared controller " << cntlrName << 
									". Skipping creation of observation-based parset.");
		// just copy the base-file if child runs on another system
		if (hostname != myHostname(false)  && hostname != myHostname(true)) {
			APLUtilities::remoteCopy(baseSetName, hostname, baseSetName);
		}
	}
	else {	// Observation-based controller, make special parset.
		LOG_DEBUG_STR ("Reading parameterfile: " << baseSetName);
		ParameterSet	wholeSet (baseSetName);
		LOG_DEBUG_STR (wholeSet.size() << " elements in parameterfile");
		string			prefix = wholeSet.getString("prefix");

		// Create a parameterset with software related issues.
		string	cntlrSetName(formatString("%s/%s", LOFAR_SHARE_LOCATION, 
												   cntlrName.c_str()));
//											sharedControllerName(cntlrName).c_str()));
		LOG_DEBUG_STR("Creating parameterfile: " << cntlrSetName);
		// first add the controller specific stuff
		string	nodeName(parsetNodeName(aCntlrType));
		string	position(wholeSet.locateModule(nodeName));
		LOG_DEBUG_STR("Ctype=" << aCntlrType << ", name=" << nodeName << 
					  ", position=" << position);
		ParameterSet	cntlrSet = wholeSet.makeSubset(position+nodeName+".");
		// always add Observation and all its children to the Parset.
		cntlrSet.adoptCollection(wholeSet.makeSubset(
				wholeSet.locateModule("Observation")+"Observation","Observation"));
		// extend Observation with Observation ID
		cntlrSet.replace("Observation.ObsID", lexical_cast<string>(anObsID));
		// add hardware info.
		cntlrSet.adoptCollection(wholeSet.makeSubset(
				wholeSet.locateModule("PIC")+"PIC","PIC"));
		

		// is there a duplicate of the controller info?
//		string	nodePos(cntlrSet.locateModule(nodeName));
//		cntlrSet.substractSubset(nodePos+nodeName);
// [260407] the code above leads to problems in starting childcontrollers.

		// Add some comment lines and some extra fields to the file
		cntlrSet.add("prefix", prefix+position+nodeName+".");
		cntlrSet.add("_instanceNr", lexical_cast<string>(instanceNr));
		cntlrSet.add("_moduleName", nodeName);
		cntlrSet.add("_treeID", lexical_cast<string>(anObsID));
		cntlrSet.add("_DPname", wholeSet.getString("_DPname"));
		cntlrSet.add("# moduleName", nodeName);
		cntlrSet.add("# pathName", prefix+position+nodeName+".");
		cntlrSet.add("# treeID", lexical_cast<string>(anObsID));
		cntlrSet.add("# DPname", wholeSet.getString("_DPname"));
		// Finally write to subset to the file.
		cntlrSet.writeFile (cntlrSetName);

		// When program must run on another system scp file to that system
		if (hostname != myHostname(false)  && 
			hostname != myHostname(true)) {
			APLUtilities::remoteCopy(cntlrSetName, hostname, cntlrSetName);
			APLUtilities::remoteCopy(baseSetName, hostname, baseSetName);
		}
	}

	// Alright, child does not exist yet. 
	// construct structure with all information
	ControllerInfo		ci;
	ci.cntlrName	  = cntlrName;
	ci.instanceNr	  = instanceNr;
	ci.obsID		  = anObsID;
	ci.cntlrType	  = aCntlrType;
	ci.port			  = 0;
	ci.hostname		  = hostname;
	ci.requestedState = CTState::CONNECTED;
	ci.requestTime	  = time(0);
	ci.currentState	  = CTState::NOSTATE;
	ci.establishTime  = 0;
	ci.retryTime	  = 0;
	ci.nrRetries	  = 0;
	ci.result		  = CT_RESULT_NO_ERROR;
	ci.inResync		  = false;
	ci.startTime	  = 0;					// only used for reschedule
	ci.stopTime		  = 0;					// only used for reschedule

	// Update our administration.
	itsCntlrList->push_back(ci);
	LOG_DEBUG_STR("Added " << cntlrName << " to the controllerList");

	// Add it to the action list.
	itsActionList.push_back(ci);

	// Trigger statemachine.
	if (itsListener) {
		itsActionTimer = itsListener->setTimer(0.0);
		LOG_DEBUG_STR("ACTIONTIMER=" << itsActionTimer);
	}

	LOG_TRACE_COND_STR ("Scheduled start of " << cntlrName << " for obs " << anObsID);

	return (true);
}

//
// startChildControllers()
//
void ChildControl::startChildControllers()
{
	string	myname   = globalParameterSet()->getString("_moduleName");
	string	fullname = globalParameterSet()->locateModule(myname)+myname+".";
	ParameterSet	subset = globalParameterSet()->makeSubset(fullname);

	ParameterSet::const_iterator	iter = subset.begin();
	ParameterSet::const_iterator	end  = subset.end();
	string	childname;
	while (iter != end) {
		string::size_type pos = iter->first.find(".", 0);
		if (pos != string::npos && iter->first.substr(0,pos) != childname) {
			childname = iter->first.substr(0,pos);
			// collect the information to start the child controller
			vector<string>	hostnames = subset.getStringVector(childname+"._hostname");
			for (uint i = 0; i < hostnames.size(); i++) {
				uint16	childCntlrType = getControllerType(childname);
				uint32	treeID         = globalParameterSet()->getUint32("_treeID");
				uint16	instanceNr	   = 0;		// TODO
				string	childCntlrName = 
						controllerName(childCntlrType, instanceNr, 
												treeID, realHostname(hostnames[i]));

				// child already running???
				CTState::CTstateNr	requestedState = getRequestedState(childCntlrName);

				if (requestedState == CTState::NOSTATE) {
					// fire request for new controller, will result in CONTROL_STARTED
					startChild(childCntlrType, 
							   treeID, 
							   instanceNr,
							   hostnames[i]);
					// Note: controller is now in state NO_STATE/CONNECTED (C/R)

					LOG_DEBUG_STR("Requested start of " << childCntlrName);
				}
			} // for
		}
		iter++;
	}
}

//
// requestState(state, [name], [observation], [type])
//
// Sends a state-change request to the given child or childs.
//
bool ChildControl::requestState	(CTState::CTstateNr	aState, 
							 	 const string&		aName, 
							 	 OTDBtreeIDType		anObsID, 
							 	 uint16				aCntlrType)
{
	CTState		cts;
	LOG_TRACE_FLOW_STR("requestState(" << cts.name(aState) << "," << aName << "," 
						<< anObsID << "," << aCntlrType <<")" );

	bool	checkName   = (aName != "");
	bool	checkID     = (anObsID != 0);
	bool	checkType   = (aCntlrType != CNTLRTYPE_NO_TYPE);
	time_t	currentTime = time(0);

	CIiter			iter  = itsCntlrList->begin();
	const_CIiter	end   = itsCntlrList->end();
	while (iter != end) {
		// count the child when x has to be checked and matches:
		//  id==id	checkID	count_it
		//	  Y		  Y			Y
		//	  Y		  N			Y
		//	  N		  Y			N --> checkID && id!=id
		//	  N		  N			Y
		if (!(checkName && iter->cntlrName != aName) && 
			!(checkID && iter->obsID != anObsID) && 
		    !(checkType && iter->cntlrType != aCntlrType)) {

			// update controllerinfo to requested state and make an action of it.
			iter->requestedState = aState;
			iter->requestTime    = currentTime;
			iter->result		 = CT_RESULT_NO_ERROR;
			iter->nrRetries		 = 0;
			iter->retryTime		 = 0;
			itsActionList.push_back(*iter);
		}
			
		iter++;
	}	
	itsTimerPort.cancelTimer(itsActionTimer);
	itsActionTimer = itsTimerPort.setTimer(0.0);	// invoke _processActionList
	LOG_DEBUG_STR("ACTIONTIMER=" << itsActionTimer);

	return (true);
}

//
// rescheduleChilds(startTime, stopTime, [name], [observation], [type])
//
// Sends a reschedule request to the given child or childs.
//
bool ChildControl::rescheduleChilds	(time_t				aStartTime,
									 time_t				aStopTime,
									 const string&		aName, 
									 OTDBtreeIDType		anObsID, 
									 uint16				aCntlrType)
{
	CTState		cts;
	LOG_TRACE_FLOW_STR("reschedule(" << aStartTime << "," 
									 << aStopTime  << ","
									 << aName <<","<< anObsID <<","<< aCntlrType <<")" );

	bool	checkName   = (aName != "");
	bool	checkID     = (anObsID != 0);
	bool	checkType   = (aCntlrType != CNTLRTYPE_NO_TYPE);
	time_t	currentTime = time(0);

	CIiter			iter  = itsCntlrList->begin();
	const_CIiter	end   = itsCntlrList->end();
	while (iter != end) {
		// count the child when x has to be checked and matches:
		//  id==id	checkID	count_it
		//	  Y		  Y			Y
		//	  Y		  N			Y
		//	  N		  Y			N --> checkID && id!=id
		//	  N		  N			Y
		if (!(checkName && iter->cntlrName != aName) && 
			!(checkID && iter->obsID != anObsID) && 
		    !(checkType && iter->cntlrType != aCntlrType)) {
			// remember the reschedule.
			iter->startTime = aStartTime;
			iter->stopTime  = aStopTime;
			// don't change state of original, it's an interim state.
			ControllerInfo	cntlrCopy(*iter);
			cntlrCopy.requestedState = CTState::SCHEDULED;
			cntlrCopy.requestTime	 = currentTime;
			cntlrCopy.result		 = CT_RESULT_NO_ERROR;
			cntlrCopy.nrRetries		 = 0;
			cntlrCopy.retryTime		 = 0;
			itsActionList.push_back(cntlrCopy);	// TODO: push_front????
		}
			
		iter++;
	}	
	itsTimerPort.cancelTimer(itsActionTimer);
	itsActionTimer = itsTimerPort.setTimer(0.0);	// invoke _processActionList
	LOG_DEBUG_STR("ACTIONTIMER=" << itsActionTimer);

	return (true);
}

//
// getCurrentState(name)
//
// Returns the current state of the given controller.
//
CTState::CTstateNr ChildControl::getCurrentState	(const string&	aName)
{
	CIiter	controller = findController(aName);
	if (controller == itsCntlrList->end()) {
		return (CTState::NOSTATE);
	}

	return (controller->currentState);
}

//
// getRequestedState(name)
//
// Returns the requested state of the given controller.
//
CTState::CTstateNr ChildControl::getRequestedState (const string&	aName)
{
	CIiter	controller = findController(aName);
	// not found?
	if (controller == itsCntlrList->end()) {
		return (CTState::NOSTATE);
	}

	return (controller->requestedState);
}

//
// countChilds([obsid],[cntlrtype])
//
// Count the number of childs. The count can be limited to an
// observation or an controllertype or both.
//
uint32 ChildControl::countChilds (OTDBtreeIDType	anObsID, 
								  uint16			aCntlrType)
{
	bool	checkID   = (anObsID != 0);
	bool	checkType = (aCntlrType != CNTLRTYPE_NO_TYPE);

	if (!checkID && !checkType) {
		return (itsCntlrList->size());
	}

	uint32			count = 0;
	const_CIiter	iter  = itsCntlrList->begin();
	const_CIiter	end   = itsCntlrList->end();
	while (iter != end) {
		if (!(checkID && iter->obsID != anObsID) && 
		    !(checkType && iter->cntlrType != aCntlrType)) {
			count++;
		}
			
		iter++;
	}	

	return (count);
}


//
// getPendingRequest ([name], [observation], [type])
//
// Returns a vector with all requests that are not confirmed by the
// childs.
//
vector<ChildControl::StateInfo> 
ChildControl::getPendingRequest (const string&		aName, 
								 OTDBtreeIDType		anObsID, 
								 uint16				aCntlrType)
{
	vector<ChildControl::StateInfo>	resultVec;

	bool	checkName   = (aName != "");
	bool	checkID     = (anObsID != 0);
	bool	checkType   = (aCntlrType != CNTLRTYPE_NO_TYPE);

	const_CIiter	iter  = itsCntlrList->begin();
	const_CIiter	end   = itsCntlrList->end();
	while (iter != end) {
		if (!(checkName && iter->cntlrName != aName) && 
			!(checkID && iter->obsID != anObsID) && 
		    !(checkType && iter->cntlrType != aCntlrType) &&
			(iter->requestedState != iter->currentState)) {
			// add info to vector
			StateInfo	si;
			si.name	 		  = iter->cntlrName;
			si.cntlrType	  = iter->cntlrType;
			si.isConnected	  = (iter->port != 0);
			si.requestedState = iter->requestedState;
			si.requestTime	  = iter->requestTime;
			si.currentState	  = iter->currentState;
			si.establishTime  = iter->establishTime;
			si.result		  = iter->result;
			resultVec.push_back(si);
		}
		iter++;
	}	

	return (resultVec);
}

//
// getCompletedStates (lastPollTime)
//
// Returns a vector with all requests that are completed since lastPollTime
//
vector<ChildControl::StateInfo> 
ChildControl::getCompletedStates (time_t	lastPollTime)
{
	vector<ChildControl::StateInfo>	resultVec;

	const_CIiter	iter  = itsCntlrList->begin();
	const_CIiter	end   = itsCntlrList->end();
	while (iter != end) {
		if (iter->establishTime > lastPollTime) {
			// add info to vector
			StateInfo	si;
			si.name			  = iter->cntlrName;
			si.cntlrType	  = iter->cntlrType;
			si.isConnected	  = (iter->port != 0);
			si.requestedState = iter->requestedState;
			si.requestTime	  = iter->requestTime;
			si.currentState	  = iter->currentState;
			si.establishTime  = iter->establishTime;
			si.result		  = iter->result;
			resultVec.push_back(si);
		}
		iter++;
	}	

	return (resultVec);
}

// -------------------- PRIVATE ROUTINES --------------------

//
// _processActionList()
//
// Walks through the actionlist and tries to process the actions.
//
void ChildControl::_processActionList()
{
	LOG_TRACE_FLOW("_processActionList()");

	// always cancel timer that brought me here.
	itsTimerPort.cancelTimer(itsActionTimer);
	itsActionTimer = 0;

	uint32	nrActions = itsActionList.size();	// prevents handling rescheduled actions
	// when list is empty return;
	if (!nrActions) {
		return;
	}

	LOG_TRACE_VAR_STR("Found " << nrActions << " actions in list");

	// Walk through the action list
	CTState		cts;
	time_t		currentTime = time(0);
	CIiter		action = itsActionList.begin();
	while (nrActions > 0) {
		// don't process (rescheduled) action that lays in the future
		if (action->retryTime > currentTime) {	// retry in future?
			LOG_DEBUG_STR("parking:" << action->cntlrName << "->" << 
					cts.name(action->requestedState) << " because its too early");
			itsActionList.push_back(*action);	// add at back
			action++;							// hop to next
			itsActionList.pop_front();			// remove at front.
			nrActions--;						// one less to handle.
			continue;
		}

		// search if corresponding controller exists
		CIiter controller = findController(action->cntlrName);
		if (controller == itsCntlrList->end()) {
			LOG_WARN_STR("Controller " << action->cntlrName << 
						 " not in administration, discarding request for state " << 
						 cts.name(action->requestedState));
			action++;					// hop to next
			itsActionList.pop_front();	// remove 'handled' action.
			nrActions--;				// one less to handle.
			continue;
		}

		// is there a connection with this controller?
		if (action->requestedState > CTState::CONNECTED && !controller->port) {
			LOG_DEBUG_STR("parking:" << action->cntlrName << "->" << 
					cts.name(action->requestedState) << " until connection is made");
			itsActionList.push_back(*action);	// add at back
			action++;							// hop to next
			itsActionList.pop_front();			// remove at front.
			nrActions--;						// one less to handle.
			continue;
		}

		// in resync mode? don't bother child with unnec. msgs.
		if (controller->inResync) {
			LOG_DEBUG_STR("bounching:" << action->cntlrName << "->" << 
						  cts.name(action->requestedState) << " because of resync");
			CTState		cts;
			_setEstablishedState(controller->cntlrName, 
								 cts.stateAck(action->requestedState), 
								 time(0), CT_RESULT_NO_ERROR);
			action++;							// move to next action.
			itsActionList.pop_front();			// remove at front.
			nrActions--;						// one less to handle.
			continue;
		}
			
		// found an action that should be handled now.
		LOG_DEBUG_STR("handling:" << action->cntlrName << "->" << 
												cts.name(action->requestedState));
		switch (action->requestedState) {
		case CTState::CONNECTED: 	// start program, wait for CONNECTED msgs of child
			{
				// first check if connection with StartDaemon is made
				SDiter	startDaemon = itsStartDaemonMap.find(action->hostname);
				if (startDaemon == itsStartDaemonMap.end() || 
											!startDaemon->second->isConnected()) {
					LOG_TRACE_COND_STR("Startdaemon for " << action->hostname << 
						" not yet connected, defering startup command for " 
						<< action->cntlrType << ":" << action->obsID);
					// if not SDport at all for this host, create one first
					if (startDaemon == itsStartDaemonMap.end()) {
						itsStartDaemonMap[action->hostname] = new GCFTCPPort(*this, 
														MAC_SVCMASK_STARTDAEMON,
														GCFPortInterface::SAP, 
														STARTDAEMON_PROTOCOL);
						itsStartDaemonMap[action->hostname]->setHostName(action->hostname);
					}
					itsStartDaemonMap[action->hostname]->open();
					// leave action in list until connection with SD is made
					itsActionList.push_back(*action);
					break;
				}

				// There is an connection with the startDaemon
				if (action->nrRetries < itsMaxStartupRetries) {	// retries left?
					LOG_DEBUG_STR("Requesting start of " << action->cntlrName << " at " 
																	<< action->hostname);
					STARTDAEMONCreateEvent		startRequest;
					startRequest.cntlrType 	   = action->cntlrType;
					startRequest.cntlrName 	   = action->cntlrName;
					startRequest.parentHost	   = myHostname(true);
					startRequest.parentService = itsListener->makeServiceName();
					startDaemon->second->send(startRequest);

					// we don't know if startup is successful, reschedule startup
					// over x seconds for safety. Note: when a successful startup
					// is received the rescheduled action is removed.
//					action->retryTime = time(0) + itsStartupRetryInterval;
//					action->nrRetries++;
//					itsActionList.push_back(*action);	// reschedule
//					... Parent is responsible for rescheduling! ...
				}
				else {
					LOG_WARN_STR ("Could not start controller " << action->cntlrName << 
								  " for observation " << action->obsID << 
								  ", giving up.");
				}
			}
			break;

		case CTState::CLAIMED:
			{
				CONTROLClaimEvent		request;
				request.cntlrName = controller->cntlrName;
				controller->port->send(request);
			}
			break;

		case CTState::PREPARED:
			{
				CONTROLPrepareEvent		request;
				request.cntlrName = controller->cntlrName;
				controller->port->send(request);
			}
			break;

		case CTState::RESUMED:
			{
				CONTROLResumeEvent		request;
				request.cntlrName = controller->cntlrName;
				controller->port->send(request);
			}
			break;

		case CTState::SUSPENDED:
			{
				CONTROLSuspendEvent		request;
				request.cntlrName = controller->cntlrName;
				controller->port->send(request);
			}
			break;

		case CTState::RELEASED:
			{
				CONTROLReleaseEvent		request;
				request.cntlrName = controller->cntlrName;
				controller->port->send(request);
			}
			break;

		case CTState::SCHEDULED:
			{
				CONTROLScheduleEvent		request;
				request.cntlrName = controller->cntlrName;
				request.startTime = controller->startTime;
				request.stopTime  = controller->stopTime;
				controller->port->send(request);
			}
			break;

		case CTState::QUITED:
			{
				CONTROLQuitEvent		request;
				request.cntlrName = controller->cntlrName;
				controller->port->send(request);
			}
			break;

		default:
			CTState		cts;
			ASSERTSTR(false, "Unhandled action: " << cts.name(action->requestedState));
		}
		action++;						// hop to next
		itsActionList.pop_front();	// remove handled action.
		nrActions--;				// one less to handle.
	}

	if (itsActionList.size()) {							// when unhandled actions in list
		itsTimerPort.cancelTimer(itsActionTimer);
		itsActionTimer = itsTimerPort.setTimer(1.0);	// restart timer
		LOG_DEBUG_STR("ACTIONTIMER=" << itsActionTimer);
	}

}

//
// _removeAction (controller, requestedState)
//
void ChildControl::_removeAction (const string&			aName,
								  CTState::CTstateNr	requestedState)
{
	CIiter			iter = itsActionList.begin();
	const_CIiter	end  = itsActionList.end();

	while (iter != end) {
		if (iter->cntlrName == aName && iter->requestedState == requestedState) {
			itsActionList.erase(iter);
			return;
		}
		iter++;
	}	

	return;
}

//
// _setEstablishedState (name, state, time)
//
// A child has reached a new state, update admin and inform user.
//
void ChildControl::_setEstablishedState(const string&		aName,
										CTState::CTstateNr	newState,
										time_t				atTime,
										uint16				result)
{
	CTState		CntlrState;
	LOG_TRACE_FLOW_STR("_setEstablishedState(" << aName <<","<< CntlrState.name(newState) <<","<<
																result <<")");

	CIiter	controller = findController(aName);
	if (controller == itsCntlrList->end()) {
		LOG_WARN_STR ("Could not update state of unknown controller " << aName);
		return;
	}

	// update controller information
	if ((controller->result = result) == CT_RESULT_NO_ERROR) {
		controller->currentState  = newState;
		LOG_DEBUG_STR("Controller " << aName <<" now in state "<< CntlrState.name(newState));
	}
	else {
		LOG_DEBUG_STR("Controller " << aName <<" remains in state "<< CntlrState.name(controller->currentState));
	}
	controller->establishTime = atTime;

	// clear resync flag
	if (newState >= controller->requestedState) {
		controller->inResync = false;
	}

	// Don't report 'ANYSTATE = lost connection' to the maintask yet.
	if (newState == CTState::ANYSTATE) {
		return;
	}

	// inform user by expiring a timer?
	if (itsCompletionTimer) {
		itsCompletionTimer->setTimer(0.0);
	}

	// inform user by sending a message?
	if (!itsCompletionPort) {
		return;					// no, just return.
	}

	switch (newState) {
	case CTState::CREATED: {
			CONTROLStartedEvent	msg;
			msg.cntlrName = controller->cntlrName;
			msg.successful= (result == CT_RESULT_NO_ERROR);
			itsCompletionPort->sendBack(msg);
		}
		break;
	case CTState::CONNECTED: {
			CONTROLConnectedEvent	msg;
			msg.cntlrName = controller->cntlrName;
			msg.result    = result;
			itsCompletionPort->sendBack(msg);
		}
		break;
	case CTState::CLAIMED: {
			CONTROLClaimedEvent	msg;
			msg.cntlrName = controller->cntlrName;
			msg.result    = result;
			itsCompletionPort->sendBack(msg);
		}
		break;
	case CTState::PREPARED: {
			CONTROLPreparedEvent	msg;
			msg.cntlrName = controller->cntlrName;
			msg.result    = result;
			itsCompletionPort->sendBack(msg);
		}
		break;
	case CTState::SCHEDULED: {
			CONTROLScheduledEvent	msg;
			msg.cntlrName = controller->cntlrName;
			msg.result    = result;
			itsCompletionPort->sendBack(msg);
		}
		break;
	case CTState::RESUMED: {
			CONTROLResumedEvent	msg;
			msg.cntlrName = controller->cntlrName;
			msg.result    = result;
			itsCompletionPort->sendBack(msg);
		}
		break;
	case CTState::SUSPENDED: {
			CONTROLSuspendedEvent	msg;
			msg.cntlrName = controller->cntlrName;
			msg.result    = result;
			itsCompletionPort->sendBack(msg);
		}
		break;
	case CTState::RELEASED: {
			CONTROLReleasedEvent	msg;
			msg.cntlrName = controller->cntlrName;
			msg.result    = result;
			itsCompletionPort->sendBack(msg);
		}
		break;
	case CTState::QUITED: {
			CONTROLQuitedEvent	msg;
			msg.cntlrName = controller->cntlrName;
			msg.treeID	  = controller->obsID;
			msg.result    = result;
			// TODO: errormsg
			itsCompletionPort->sendBack(msg);
		}
		break;
	default:
		// do nothing
		break;
	}

}

//
// _doGarbageCollection()
//
// Walks through the controllerlist and erase disconnected controllers.
//
void ChildControl::_doGarbageCollection()
{
	LOG_DEBUG_STR ("Garbage collection(" << itsGarbageInterval << ")");

	CIiter			iter  = itsCntlrList->begin();
	const_CIiter	end   = itsCntlrList->end();
	while (iter != end) {
		// Note: Removing a controller is done in two stages.
		// 1: port == 0: inform main task about removal
		// 2: port == -1: remove from list
		// This is necc. because main task may poll childcontrol for results.
		if (!iter->port) {
			LOG_DEBUG_STR ("Controller " << iter->cntlrName << 
							" is still unreachable, informing main task");
			_setEstablishedState(iter->cntlrName, CTState::QUITED, time(0), 
													CT_RESULT_LOST_CONNECTION);
			iter->port = (GCFPortInterface*) -1;
			// start timer for second stage.
			if (itsGarbageTimer) {
				itsTimerPort.cancelTimer(itsGarbageTimer);
			}
			itsGarbageTimer = itsTimerPort.setTimer(1.0 * itsGarbageInterval);
			LOG_DEBUG_STR("GarbageTimer = " << itsGarbageTimer);
			iter++;
		} else if (iter->port == (GCFPortInterface*)-1) {
			LOG_DEBUG_STR ("Removing controller " << iter->cntlrName << 
												" from the controller list");
			CIiter	iterCopy = iter;
			iter++;
			itsCntlrList->erase(iterCopy);
			LOG_DEBUG_STR("Size of itsCntlrList = " << itsCntlrList->size());
		} else {
			iter++;
		}
	}
}

// -------------------- STATE MACHINES FOR GCFTASK --------------------

//
// initial (event, port)
//
// Remember: there is nothing to do in the initial state until the user called
// 'openService' for our listener. After that service is called our task is to
// bring the listener alive.
//
GCFEvent::TResult	ChildControl::initial (GCFEvent&			event, 
										   GCFPortInterface&	port)
{
	LOG_DEBUG_STR ("initial:" << eventName(event) << "@" << port.getName());

	GCFEvent::TResult	status = GCFEvent::HANDLED;

	switch (event.signal) {
	case F_INIT:
		break;

	case F_ENTRY:
		break;

	case F_CONNECTED:
		if (&port == itsListener) {
			port.cancelAllTimers();
			LOG_DEBUG("Listener for children opened, going to operational state");
			TRAN (ChildControl::operational);
		}
		break;

	case F_DISCONNECTED:
		port.close();
		if (&port == itsListener) {
			ASSERTSTR(false, "Unable to open the listener, bailing out.");
		}
		ASSERTSTR(false, "Programming error, unexpected port closed");
		break;

	default:
		LOG_DEBUG ("initial, default");
		status = GCFEvent::NOT_HANDLED;
		break;
	}

	return (status);
}

//
// operational (event, port)
//
// In the operational mode the users will request startup on new children (startChild)
// and state changes of existing children (requestState). 
//
GCFEvent::TResult	ChildControl::operational(GCFEvent&			event, 
											  GCFPortInterface&	port)

{
	LOG_DEBUG_STR ("operational:" << eventName(event) << "@" << port.getName());

	GCFEvent::TResult	status = GCFEvent::HANDLED;

	switch (event.signal) {
	case F_INIT:
		break;

	case F_ENTRY:	// process actions that where queued during my startup.
			_processActionList();
		break;

	case F_ACCEPT_REQ: {
			// Should be from a just started Child.
			// accept connection and add port to port-vector
			GCFTCPPort* client(new GCFTCPPort);

			// reminder: init (task, name, type, protocol [,raw])
			client->init(*this, "newChild", GCFPortInterface::SPP, 
														CONTROLLER_PROTOCOL);
			itsListener->accept(*client);

			// Note: we do not keep an administration of the accepted child
			// sockets. Their ports will be in the ControllerList as soon as
			// they have send a CONTROL_CONNECTED msg.
		}
		break;

	case F_CONNECTED:
		break;

	case F_DISCONNECTED: {
			// 170507: in one way or another the controllerport is not recognized here
			//		   anymore. So the code of cleaning up the admin is moved to the
			// 		   reception of the QUITED event.
#if 1
			CIiter		controller = itsCntlrList->begin();
			CIiter		end		   = itsCntlrList->end();
			while (controller != end) {
				// search corresponding controller
				LOG_DEBUG_STR("Check:" << controller->cntlrName << ":" << controller->port << "=?" << &port);
				if (controller->port != &port) {
					controller++;
					continue;
				}

				
				// found controller, close port
				if (controller->currentState >= CTState::RELEASED) {// expected disconnect?
					LOG_DEBUG_STR("Removing " << controller->cntlrName << 
															" from the controllerList");
					port.close();
					itsCntlrList->erase(controller);			// just remove
					LOG_DEBUG_STR("Size of itsCntlrList = " << itsCntlrList->size());
				}
				else {	// unexpected disconnect give child some time to reconnect(?)
					LOG_WARN_STR ("Lost connection with controller " << 
						controller->cntlrName << 
						" while not in QUITING state, waiting 5 minutes for reconnect");

					_setEstablishedState(controller->cntlrName, CTState::ANYSTATE, 
													time(0), CT_RESULT_LOST_CONNECTION);
					controller->port = 0;

#if 0
					// Try to restart the controller over 5 seconds
					// Add it to the action list.
					controller->retryTime = time(0) + 300 ;
					itsListener->cancelTimer(itsActionTimer);
					itsActionTimer = itsListener->setTimer(1.0);
					itsActionList.push_back(*controller);

#endif
				}

				// And schedule cleanup when everthing fails
				if (itsGarbageTimer) {
					itsTimerPort.cancelTimer(itsGarbageTimer);
				}
				itsGarbageTimer = itsTimerPort.setTimer(1.0 * itsGarbageInterval);
				LOG_DEBUG_STR("GarbageTimer = " << itsGarbageTimer);
				// controller was found and handled, break out of the while loop.
				break;
			} // while
#endif
			port.close();		// always close port
		}
		break;

	case F_TIMER: {
			GCFTimerEvent&      timerEvent = static_cast<GCFTimerEvent&>(event);
			LOG_DEBUG_STR("TIMERID=" << timerEvent.id);
			if (timerEvent.id == itsGarbageTimer) {
				itsGarbageTimer = 0;
				_doGarbageCollection();
			}
			else {
				_processActionList();
			}
		}
		break;

	case STARTDAEMON_CREATED: {	// startDaemon reports startup of program
			STARTDAEMONCreatedEvent		result(event);
			LOG_DEBUG_STR("Startup of " << result.cntlrName << " ready, result=" 	
															<< result.result);
			_setEstablishedState(result.cntlrName, CTState::CREATED, time(0),
								 result.result);

			// Was startdaemon unable to start the controller?
			if (result.result != CT_RESULT_NO_ERROR) {
				LOG_DEBUG_STR("Marking port of " << result.cntlrName << 
												" invalid because startup failed.");
				CIiter	controller = findController(result.cntlrName);
				if (controller != itsCntlrList->end()) {
					controller->port = (GCFPortInterface*)-1;
				}

				// Start garbagetimer to cleanup the idle controller entry.
				itsTimerPort.cancelTimer(itsGarbageTimer);
				itsGarbageTimer = itsTimerPort.setTimer(1.0 * itsGarbageInterval);
				LOG_DEBUG_STR("GarbageTimer = " << itsGarbageTimer);
			}
		}
		break;

	case CONTROL_CONNECT: {		// received from just started controller
			CONTROLConnectEvent		msg(event);
			CONTROLConnectedEvent		answer;

			CIiter	controller = findController(msg.cntlrName);
			if (controller == itsCntlrList->end()) {		// not found?
				LOG_WARN_STR ("CONNECT event received from unknown controller: " <<
							  msg.cntlrName);
				answer.result = CT_RESULT_UNSPECIFIED;
			}
			else {
				LOG_DEBUG_STR("CONNECT event received from " << msg.cntlrName);
				_setEstablishedState(msg.cntlrName, CTState::CONNECTED, time(0), CT_RESULT_NO_ERROR);
				// first direct contact with controller, remember port
				controller->port = &port;
				answer.result = CT_RESULT_NO_ERROR;
			}
			port.send(answer);
		}
		break;

	case CONTROL_RESYNC: {	// reconnected child sends it current state.
			CONTROLResyncEvent		msg(event);
			CTState					cts;

			CIiter controller = findController(msg.cntlrName);
			if (!isController(controller)) {
				// Reconstruct controller info.
				ControllerInfo		ci;
				ci.cntlrName	  = msg.cntlrName;
				ci.instanceNr	  = getInstanceNr(msg.cntlrName);
				ci.obsID		  = getObservationNr(msg.cntlrName);
				ci.cntlrType	  = getControllerType(msg.cntlrName);
				ci.port			  = &port;
				ci.hostname		  = msg.hostname;
				ci.requestedState = cts.stateNr(msg.curState);
				ci.requestTime	  = time(0);
				ci.currentState	  = cts.stateNr(msg.curState);
				ci.establishTime  = 0;
				ci.retryTime	  = 0;
				ci.nrRetries	  = 0;
				ci.result		  = CT_RESULT_NO_ERROR;
				ci.inResync		  = true;

				// Update our administration.
				itsCntlrList->push_back(ci);
				LOG_DEBUG_STR("Added reconnected " << msg.cntlrName << 
														" to the controllerList");
			}
			else {
				// Resync of known controller
				controller->requestedState = cts.stateNr(msg.curState);
				controller->currentState   = cts.stateNr(msg.curState);
				controller->hostname	   = msg.hostname;
				controller->port 		   = &port;
				controller->inResync	   = true;
				LOG_DEBUG_STR("Updated info of reconnected controller " << msg.cntlrName);

				// make sure that the maintask gets the Connect message it is waiting for.
				if (controller->currentState == CTState::CONNECTED) {
					_setEstablishedState(controller->cntlrName, CTState::CONNECTED, 
										 time(0), CT_RESULT_NO_ERROR);
				}
			}

			// Finally confirm resync action to child.
			CONTROLResyncedEvent	answer;
			answer.cntlrName = msg.cntlrName;
			answer.result    = CT_RESULT_NO_ERROR;
			port.send(answer);
		}
		break;

	case CONTROL_CLAIMED: {
			CONTROLClaimedEvent		result(event);
			_setEstablishedState(result.cntlrName, CTState::CLAIMED, time(0),
								 result.result);
		}
		break;
	
	case CONTROL_PREPARED: {
			CONTROLPreparedEvent		result(event);
			_setEstablishedState(result.cntlrName, CTState::PREPARED, time(0),
								 result.result);
		}
		break;
	
	case CONTROL_RESUMED: {
			CONTROLResumedEvent		result(event);
			_setEstablishedState(result.cntlrName, CTState::RESUMED, time(0),
								 result.result);
		}
		break;
	
	case CONTROL_SUSPENDED: {
			CONTROLSuspendedEvent		result(event);
			_setEstablishedState(result.cntlrName, CTState::SUSPENDED, time(0),
								 result.result);
		}
		break;
	
	case CONTROL_RELEASED: {
			CONTROLReleasedEvent		result(event);
			_setEstablishedState(result.cntlrName, CTState::RELEASED, time(0),
								 result.result);
		}
		break;
	
	case CONTROL_SCHEDULED:
		// do nothing, is not a state change.
		break;
	
	case CONTROL_QUITED: {
			CONTROLQuitedEvent		msg(event);
			_setEstablishedState(msg.cntlrName, CTState::QUITED, time(0),
								 msg.result);

#if 0
			CIiter	controller = findController(msg.cntlrName);
			ASSERTSTR(isController(controller), "Controller " << msg.cntlrName << 
															" not in our administration anymore!");
			// found controller, close port
			LOG_DEBUG_STR("Removing " << controller->cntlrName << 
														" from the controllerlist");
			itsCntlrList->erase(controller);			// just remove
#endif
		}
		break;
	

	default:
		LOG_DEBUG ("operational, default");
		status = GCFEvent::NOT_HANDLED;
		break;
	}

	return (status);
}

  } // namespace APLCommon
} // namespace LOFAR
