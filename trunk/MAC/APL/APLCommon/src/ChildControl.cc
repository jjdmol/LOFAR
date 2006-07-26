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
#include <Common/LofarLogger.h>
#include <Deployment/StationInfo.h>
#include <APS/ParameterSet.h>
#include <GCF/GCF_ServiceInfo.h>
#include <GCF/Utils.h>
#include <APL/APLCommon/APLUtilities.h>
#include <APL/APLCommon/ChildControl.h>
#include <Controller_Protocol.ph>
#include <StartDaemon_Protocol.ph>
#include <boost/lexical_cast.hpp>
using namespace boost;

namespace LOFAR {
  using namespace Deployment;
  using namespace GCF::TM;
  using namespace ACC::APS;
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
	itsGarbageInterval		(30),		// TODO: set to 300 for real life
	itsCompletionTimer		(0),
	itsCompletionPort		(0)
{
	// Log the protocols I use.
	registerProtocol(CONTROLLER_PROTOCOL, CONTROLLER_PROTOCOL_signalnames);
	registerProtocol(STARTDAEMON_PROTOCOL,STARTDAEMON_PROTOCOL_signalnames);

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
	itsListener->open();
}

//
// startChild (name, obsId, aCntlType, instanceNr, hostname)
//
bool ChildControl::startChild (const string&		aName, 
							   OTDBtreeIDType		anObsID, 
							   uint16				aCntlrType, 
							   uint32				instanceNr,
							   const string&		hostname)
{
	// first check if child already exists
	if (findController(aName) != itsCntlrList->end()) {
		return (false);
	}	

	// make sure there is a parameterSet for the program.
	// search observation-parset for controller name and determine prefix
	// NOTE: this name must be the same as in the MACScheduler.
	string	baseSetName = formatString("%s/Observation_%d", LOFAR_SHARE_LOCATION, anObsID);
	LOG_DEBUG_STR ("Reading parameterfile: " << baseSetName);
	ParameterSet	wholeSet (baseSetName);
	string			prefix = wholeSet.getString("prefix");

	// Create a parameterset with software related issues.
	string	cntlrSetName(formatString("%s/%s", LOFAR_SHARE_LOCATION, aName.c_str()));
	LOG_DEBUG_STR("Creating parameterfile: " << cntlrSetName);
	// first add the controller specific stuff
	string	nodeName(parsetNodeName(aCntlrType));
	string	position(wholeSet.locateModule(nodeName));
	LOG_DEBUG_STR("Ctype=" << aCntlrType << ", name=" << nodeName << 
				  ", position=" << position);
	ParameterSet	cntlrSet = wholeSet.makeSubset(position+nodeName+".");
	// always add Observation and all its children to the Parset.
	cntlrSet.adoptCollection(wholeSet.makeSubset(wholeSet.locateModule("Observation")));
	// Add some comment lines and some extra fields to the file
	cntlrSet.add("prefix", prefix+position+nodeName+".");
	cntlrSet.add("_instanceNr", lexical_cast<string>(instanceNr));
	cntlrSet.add("_treeID", lexical_cast<string>(anObsID));
	cntlrSet.add("# modulename", nodeName);
	cntlrSet.add("# pathname", prefix+position+nodeName+".");
	cntlrSet.add("# treeID", lexical_cast<string>(anObsID));
	// Finally write to subset to the file.
	cntlrSet.writeFile (cntlrSetName);

	// Alright, child does not exist yet. 
	// construct structure with all information
	ControllerInfo		ci;
	ci.cntlrName	  = aName;
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

	// Update our administration.
	itsCntlrList->push_back(ci);
	LOG_DEBUG_STR("Added " << aName << " to the controllerList");

	// Add it to the action list.
	itsActionList.push_back(ci);

	// Trigger statemachine.
	if (itsListener) {
		itsActionTimer = itsListener->setTimer(0.0);
	}

	LOG_TRACE_COND_STR ("Scheduled start of " << aName << " for obs " << anObsID);

	return (true);
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
			// send request to child
			iter->requestedState = aState;
			iter->requestTime    = currentTime;
			iter->failed		 = false;
			iter->nrRetries		 = 0;
			iter->retryTime		 = 0;
		}
			
		iter++;
	}	
	itsActionTimer = itsTimerPort.setTimer(0.0);	// invoke _processActionList

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
	uint32	nrActions = itsActionList.size();	// prevents handling rescheduled actions
	// when list is empty return;
	if (!nrActions) {
		return;
	}

	LOG_TRACE_VAR_STR("Found " << nrActions << " actions in list");

	// Walk through the action list
	time_t		currentTime = time(0);
	CIiter		action = itsActionList.begin();
	while (nrActions > 0) {
		// don't process (rescheduled) action that lay in the future
		if (action->retryTime > currentTime) {	// retry in future?
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
						 action->requestedState);
			action++;					// hop to next
			itsActionList.pop_front();	// remove 'handled' action.
			nrActions--;				// one less to handle.
			continue;
		}
			
		// found an action that should be handled now.
		switch (action->requestedState) {
		case CTState::CONNECTED: 	// start program, wait for CONNECTED msgs of child
			{
				// first check if connection if StartDaemon is made
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
					startRequest.parentHost	   = GCF::Common::myHostname(true);
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

		case CTState::ACTIVE:
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
		case CTState::FINISHED:
			{
				CONTROLReleaseEvent		request;
				request.cntlrName = controller->cntlrName;
				controller->port->send(request);
			}
			break;

		default:
			ASSERTSTR(false, "Unhandled action: " << action->requestedState);
		}
		action++;						// hop to next
		itsActionList.pop_front();	// remove handled action.
		nrActions--;				// one less to handle.
	}

	if (itsActionList.size()) {							// when unhandled actions in list
		itsActionTimer = itsTimerPort.setTimer(1.0);	// restart timer
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
	CIiter	controller = findController(aName);
	if (controller == itsCntlrList->end()) {
		LOG_WARN_STR ("Could not update state of controller " << aName);
		return;
	}

	// update controller information
	if (!(controller->failed = (result != CT_RESULT_NO_ERROR))) {
		controller->currentState  = newState;
	}
	controller->establishTime = atTime;

	CTState		CntlrState;
	LOG_DEBUG_STR("Controller " << aName <<" now in state "<< CntlrState.name(newState));

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
	case CTState::ACTIVE: {
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
	case CTState::FINISH: {
			CONTROLFinishEvent	msg;
			msg.cntlrName = controller->cntlrName;
			msg.treeID	  = controller->obsID;
			msg.result    = result;
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
	LOG_DEBUG ("Garbage collection");

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
			_setEstablishedState(iter->cntlrName, CTState::FINISH, time(0), 
													CT_RESULT_LOST_CONNECTION);
			iter->port = (GCFPortInterface*) -1;
			// start timer for second stage.
			if (itsGarbageTimer) {
				itsTimerPort.cancelTimer(itsGarbageTimer);
			}
			itsGarbageTimer = itsTimerPort.setTimer(1.0 * itsGarbageInterval);
			iter++;
		} else if (iter->port == (GCFPortInterface*)-1) {
			LOG_DEBUG_STR ("Removing controller " << iter->cntlrName << 
												" from the controller list");
			CIiter	iterCopy = iter;
			iter++;
			itsCntlrList->erase(iterCopy);
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
	LOG_DEBUG_STR ("initial:" << evtstr(event) << "@" << port.getName());

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
			port.setTimer(1.0);
		}
		else {
//			_handleDisconnectEvent(event, port);
		}
		break;

	case F_TIMER:
		// is this always the reconnect timer?
		itsListener->open();
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
	LOG_DEBUG_STR ("operational:" << evtstr(event) << "@" << port.getName());

	GCFEvent::TResult	status = GCFEvent::HANDLED;

	switch (event.signal) {
	case F_INIT:
		break;

	case F_ENTRY:
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
			port.close();		// always close port

			CIiter		controller = itsCntlrList->begin();
			CIiter		end		   = itsCntlrList->end();
			while (controller != end) {
				// search corresponding controller
				if (controller->port != &port) {
					controller++;
					continue;
				}

				// found controller, close port
				if (controller->currentState == CTState::FINISHED) {// expected disconnect?
					LOG_DEBUG_STR("Removing " << controller->cntlrName << 
															" from the controllerList");
					itsCntlrList->erase(controller);			// just remove
				}
				else {	// unexpected disconnect give child some time to reconnect(?)
					LOG_WARN_STR ("Lost connection with controller " << 
						controller->cntlrName << 
						" while not in FINISHED state, waiting 5 minutes for reconnect");

					_setEstablishedState(controller->cntlrName, CTState::ANYSTATE, 
													time(0), CT_RESULT_LOST_CONNECTION);
					controller->port = 0;
					if (itsGarbageTimer) {
						itsTimerPort.cancelTimer(itsGarbageTimer);
					}
					itsGarbageTimer = itsTimerPort.setTimer(1.0 * itsGarbageInterval);
				}
			}
		}
		break;

	case F_TIMER:
		{
			GCFTimerEvent&      timerEvent = static_cast<GCFTimerEvent&>(event);
			if (timerEvent.id == itsGarbageTimer) {
				itsGarbageTimer = 0;
				_doGarbageCollection();
			}
			else {
				itsActionTimer = 0;
				_processActionList();
			}
		}
		break;

	case STARTDAEMON_CREATED:	// startDaemon reports startup of program
		{
		STARTDAEMONCreatedEvent		result(event);
		LOG_DEBUG_STR("Startup of " << result.cntlrName << " ready, result=" 	
														<< result.result);
		_setEstablishedState(result.cntlrName, CTState::CREATED, time(0),
							 result.result);
		}
		break;

	case CONTROL_CONNECT:		// received from just started controller
		{
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

				// Update our administration.
				itsCntlrList->push_back(ci);
				LOG_DEBUG_STR("Added reconnected " << msg.cntlrName << 
														" to the controllerList");
			}
			else {
				// Resunc of known controller (strange case!)
				controller->requestedState = cts.stateNr(msg.curState);
				controller->currentState   = cts.stateNr(msg.curState);
				controller->hostname	   = msg.hostname;
				LOG_DEBUG_STR("Updated info of reconnected controller " << msg.cntlrName);
			}

			// Finally confirm resync action to child.
			CONTROLResyncedEvent	answer;
			answer.cntlrName = msg.cntlrName;
			answer.result    = CT_RESULT_NO_ERROR;
			port.send(answer);
		}
		break;

	case CONTROL_CLAIMED: {
			CONTROLClaimedEvent		result;
			_setEstablishedState(result.cntlrName, CTState::CLAIMED, time(0),
								 result.result);
		}
		break;
	
	case CONTROL_PREPARED: {
			CONTROLPreparedEvent		result;
			_setEstablishedState(result.cntlrName, CTState::PREPARED, time(0),
								 result.result);
		}
		break;
	
	case CONTROL_RESUMED: {
			CONTROLResumedEvent		result;
			_setEstablishedState(result.cntlrName, CTState::ACTIVE, time(0),
								 result.result);
		}
		break;
	
	case CONTROL_SUSPENDED: {
			CONTROLSuspendedEvent		result;
			_setEstablishedState(result.cntlrName, CTState::SUSPENDED, time(0),
								 result.result);
		}
		break;
	
	case CONTROL_RELEASED: {
			CONTROLReleasedEvent		result;
			_setEstablishedState(result.cntlrName, CTState::RELEASED, time(0),
								 result.result);
		}
		break;
	
	case CONTROL_FINISH: {
			CONTROLFinishEvent		msg;
			_setEstablishedState(msg.cntlrName, CTState::FINISHED, time(0),
								 msg.result);

			// inform child its shutdown is registered
			CONTROLFinishedEvent	reply;
			reply.cntlrName = msg.cntlrName;
			port.send(reply);
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
