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
#include <APS/ParameterSet.h>
#include <MainCU/MACScheduler/ChildControl.h>
#include <GCF/GCF_ServiceInfo.h>
#include <GCF/Utils.h>
#include <APL/APLCommon/APLUtilities.h>
#include <APL/APLCommon/Controller_Protocol.ph>
#include <APL/APLCommon/StartDaemon_Protocol.ph>

namespace LOFAR {
  using namespace GCF::TM;
  using namespace APLCommon;
  using namespace ACC::APS;
  namespace MainCU {

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
	itsStartDaemonMap		(),
	itsStartupRetryInterval	(10),
	itsMaxStartupRetries	(5),
	itsCntlrList	 		(),
	itsActionList	 		(),
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
	itsCntlrList.clear();

	// clear action list.
	itsActionList.clear();
}

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
							   OTDB::treeIDType		anObsID, 
							   const string&		aCntlrType, 
							   uint32				instanceNr,
							   const string&		hostname)
{
	// first check if child already exists
	if (findController(aName) != itsCntlrList.end()) {
		return (false);
	}	

	// make sure there is a parameterSet for the program.
	if (aCntlrType != CNTLRTYPE_OBSERVATIONCTRL) {
		// search observation-parset for controller name and determine prefix
		string			baseSetName = formatString("%s/Observation_%d", 
													LOFAR_SHARE_LOCATION, anObsID);
		LOG_DEBUG_STR ("Reading parameterfile: " << baseSetName);
		ParameterSet	wholeSet (baseSetName);
		ParameterSet::iterator		iter = wholeSet.begin();
		ParameterSet::iterator		end  = wholeSet.end();
		while (iter != end) {
			// search a parameter that is meant for this controller
			// to determine the position of the controller in the tree.
			if (keyName(moduleName(iter->first)) == aName) {
				string	cntlrSetName = formatString("%s/%s", LOFAR_SHARE_LOCATION, 
															 aName.c_str());
				LOG_DEBUG_STR("Creating parameterfile: " << cntlrSetName);
				ParameterSet	cntlrSet = wholeSet.makeSubset(moduleName(iter->first));
				cntlrSet.add("prefix", moduleName(iter->first));
				cntlrSet.writeFile (cntlrSetName);
				break;
			}
			iter++;
		}
		if (iter == end) {		// could not create a parameterset, report failure.
			LOG_ERROR_STR("No parameter information found for controller " << aName <<
						  " in file " << baseSetName << ". Cannot start controller!");
			return (false);
		}
	}

	// Alright, child does not exist yet. 
	// construct structure with all information
	ControllerInfo		ci;
	ci.name			  = aName;
	ci.instanceNr	  = instanceNr;
	ci.obsID		  = anObsID;
	ci.cntlrType	  = aCntlrType;
	ci.port			  = 0;
	ci.hostname		  = hostname;
	ci.requestedState = LDState::CONNECTED;
	ci.requestTime	  = time(0);
	ci.currentState	  = LDState::NOSTATE;
	ci.establishTime  = 0;
	ci.retryTime	  = 0;
	ci.nrRetries	  = 0;

	// Update our administration.
	itsCntlrList.push_back(ci);

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
bool ChildControl::requestState	(LDState::LDstateNr	aState, 
							 	 const string&		aName, 
							 	 OTDB::treeIDType	anObsID, 
							 	 const string&		aCntlrType)
{
	bool	checkName   = (aName != "");
	bool	checkID     = (anObsID != 0);
	bool	checkType   = (aCntlrType != CNTLRTYPE_NO_TYPE);
	time_t	currentTime = time(0);

//	stateChangeEvent	request;
//	request.newState = aState;
//	request.time	 = currentTime;

	CIiter			iter  = itsCntlrList.begin();
	const_CIiter	end   = itsCntlrList.end();
	while (iter != end) {
		// count the child when x has to be checked and matches:
		//  id==id	checkID	count_it
		//	  Y		  Y			Y
		//	  Y		  N			Y
		//	  N		  Y			N --> checkID && id!=id
		//	  N		  N			Y
		if (!(checkName && iter->name != aName) && 
			!(checkID && iter->obsID != anObsID) && 
		    !(checkType && iter->cntlrType != aCntlrType)) {
			// send request to child
			iter->requestedState = aState;
			iter->requestTime    = currentTime;
//			iter->port.send(request);
		}
			
		iter++;
	}	

	return (true);
}

//
// getCurrentState(name)
//
// Returns the current state of the given controller.
//
LDState::LDstateNr ChildControl::getCurrentState	(const string&	aName)
{
	CIiter	controller = findController(aName);
	if (controller == itsCntlrList.end()) {
		return (LDState::NOSTATE);
	}

	return (controller->currentState);
}

//
// getRequestedState(name)
//
// Returns the requested state of the given controller.
//
LDState::LDstateNr ChildControl::getRequestedState (const string&	aName)
{
	CIiter	controller = findController(aName);
	if (controller == itsCntlrList.end()) {
		return (LDState::NOSTATE);
	}

	return (controller->requestedState);
}

//
// countChilds([obsid],[cntlrtype])
//
// Count the number of childs. The count can be limited to an
// observation or an controllertype or both.
//
uint32 ChildControl::countChilds (OTDB::treeIDType	anObsID, 
								  const string&		aCntlrType)
{
	bool	checkID   = (anObsID != 0);
	bool	checkType = (aCntlrType != CNTLRTYPE_NO_TYPE);

	if (!checkID && !checkType) {
		return (itsCntlrList.size());
	}

	uint32			count = 0;
	const_CIiter	iter  = itsCntlrList.begin();
	const_CIiter	end   = itsCntlrList.end();
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
								 OTDB::treeIDType	anObsID, 
								 const string&		aCntlrType)
{
	vector<ChildControl::StateInfo>	resultVec;

	bool	checkName   = (aName != "");
	bool	checkID     = (anObsID != 0);
	bool	checkType   = (aCntlrType != CNTLRTYPE_NO_TYPE);

	const_CIiter	iter  = itsCntlrList.begin();
	const_CIiter	end   = itsCntlrList.end();
	while (iter != end) {
		if (!(checkName && iter->name != aName) && 
			!(checkID && iter->obsID != anObsID) && 
		    !(checkType && iter->cntlrType != aCntlrType) &&
			(iter->requestedState != iter->currentState)) {
			// add info to vector
			StateInfo	si;
			si.name			  = iter->name;
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

	const_CIiter	iter  = itsCntlrList.begin();
	const_CIiter	end   = itsCntlrList.end();
	while (iter != end) {
		if (iter->establishTime > lastPollTime) {
			// add info to vector
			StateInfo	si;
			si.name			  = iter->name;
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
// findController (name)
//
ChildControl::CIiter ChildControl::findController(const string&		aName)
{
	CIiter			iter = itsCntlrList.begin();
	const_CIiter	end  = itsCntlrList.end();

	while (iter != end && iter->name != aName) {
		iter++;
	}	
	return (iter);
}

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
		if (action->retryTime > currentTime) {
			itsActionList.push_back(*action);	// add at back
			action++;							// hop to next
			itsActionList.pop_front();			// remove at front.
			nrActions--;						// one less to handle.
			continue;
		}

		// search if corresponding controller exists
		CIiter controller = findController(action->name);
		if (controller == itsCntlrList.end()) {
			LOG_WARN_STR("Controller " << action->name << 
						 " not in administration, discarding request for state " << 
						 action->requestedState);
			action++;					// hop to next
			itsActionList.pop_front();	// remove 'handled' action.
			nrActions--;				// one less to handle.
			continue;
		}
			
		// found an action that should be handled now.
		switch (action->requestedState) {
		case LDState::CONNECTED: 	// start program, wait for CONNECTED msgs of child
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
					LOG_DEBUG_STR("Requesting start of " << action->name << " at " 
																	<< action->hostname);
					STARTDAEMONCreateEvent		startRequest;
					startRequest.cntlrType 	   = action->cntlrType;
					startRequest.cntlrName 	   = action->name;
					startRequest.parentHost	   = GCF::Common::myHostname();
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
					LOG_WARN_STR ("Could not start controller " << action->name << 
								  " for observation " << action->obsID << 
								  ", giving up.");
				}
			}
			break;

		case LDState::CLAIMED:
			{
				CONTROLClaimEvent		request;
				request.cntlrName = controller->name;
				controller->port->send(request);
			}
			break;

		case LDState::PREPARED:
			{
				CONTROLPrepareEvent		request;
				request.cntlrName = controller->name;
				controller->port->send(request);
			}
			break;

		case LDState::ACTIVE:
			{
				CONTROLResumeEvent		request;
				request.cntlrName = controller->name;
				controller->port->send(request);
			}
			break;

		case LDState::SUSPENDED:
			{
				CONTROLSuspendEvent		request;
				request.cntlrName = controller->name;
				controller->port->send(request);
			}
			break;

		case LDState::RELEASED:
		case LDState::FINISHED:
			{
				CONTROLReleaseEvent		request;
				request.cntlrName = controller->name;
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
		itsActionTimer = itsListener->setTimer(1.0);	// restart timer
	}

}

//
// _removeAction (controller, requestedState)
//
void ChildControl::_removeAction (const string&			aName,
								  LDState::LDstateNr	requestedState)
{
	CIiter			iter = itsActionList.begin();
	const_CIiter	end  = itsActionList.end();

	while (iter != end) {
		if (iter->name == aName && iter->requestedState == requestedState) {
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
void ChildControl::_setEstablishedState(const string&		aName,
										LDState::LDstateNr	newState,
										time_t				atTime,
										bool				successful)
{
	CIiter	controller = findController(aName);
	if (controller == itsCntlrList.end()) {
		LOG_WARN_STR ("Could not update state of controller " << aName);
		return;
	}

	// update controller information
	if (!(controller->failed = !successful)) {;
		controller->currentState  = newState;
	}
	controller->establishTime = atTime;

	if (itsCompletionTimer) {
		itsCompletionTimer->setTimer(0.0);
	}

	if (!itsCompletionPort) {
		return;
	}

	LOG_DEBUG_STR("newState=" << newState);

	TLDResult	result = controller->failed ? LD_RESULT_UNSPECIFIED : LD_RESULT_NO_ERROR;
	switch (newState) {
	case LDState::CREATED: {
			CONTROLStartedEvent	msg;
			msg.cntlrName = controller->name;
			msg.successful= successful;
			itsCompletionPort->sendBack(msg);
		}
		break;
	case LDState::CONNECTED: {
			CONTROLConnectedEvent	msg;
			msg.cntlrName = controller->name;
			msg.result    = result;
			itsCompletionPort->sendBack(msg);
		}
		break;
	case LDState::CLAIMED: {
			CONTROLClaimedEvent	msg;
			msg.cntlrName = controller->name;
			msg.result    = result;
			itsCompletionPort->sendBack(msg);
		}
		break;
	case LDState::PREPARED: {
			CONTROLPreparedEvent	msg;
			msg.cntlrName = controller->name;
			msg.result    = result;
			itsCompletionPort->sendBack(msg);
		}
		break;
	case LDState::ACTIVE: {
			CONTROLResumedEvent	msg;
			msg.cntlrName = controller->name;
			msg.result    = result;
			itsCompletionPort->sendBack(msg);
		}
		break;
	case LDState::SUSPENDED: {
			CONTROLSuspendedEvent	msg;
			msg.cntlrName = controller->name;
			msg.result    = result;
			itsCompletionPort->sendBack(msg);
		}
		break;
	case LDState::RELEASED: {
			CONTROLReleasedEvent	msg;
			msg.cntlrName = controller->name;
			msg.result    = result;
			itsCompletionPort->sendBack(msg);
		}
		break;
	case LDState::FINISHED: {
			CONTROLFinishedEvent	msg;
			msg.cntlrName = controller->name;
			msg.result    = result;
			itsCompletionPort->sendBack(msg);
		}
		break;
	default:
		// do nothing
		break;
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
	LOG_DEBUG ("ChildControl::initial");

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
		LOG_DEBUG ("ChildControl::initial, default");
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
	LOG_DEBUG_STR ("ChildControl::operational:" << evtstr(event) 
											    << "@" << port.getName());

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
			// they have send a CONNECTED msg.
		}
		break;

	case F_CONNECTED:
		break;

	case F_DISCONNECTED: {
			port.close();		// always close port

			CIiter		controller = itsCntlrList.begin();
			CIiter		end		   = itsCntlrList.end();
			while (controller != end) {
				// search corresponding controller
				if (controller->port != &port) {
					controller++;
					continue;
				}

				// found controller, close port
				if (controller->currentState == LDState::FINISHED) {// expected disconnect?
					itsCntlrList.erase(controller);			// just remove
				}
				else {
					LOG_WARN_STR ("Lost connection with controller " << controller->name);
					controller->port = 0;
					// TODO: implement garbage collection
				}
			}
		}
		break;

	case F_TIMER:
		itsActionTimer = 0;
		_processActionList();
		break;

	case STARTDAEMON_CREATED:	// startDaemon reports startup of program
		{
		STARTDAEMONCreatedEvent		result(event);
		LOG_DEBUG_STR("Startup of " << result.cntlrName << " ready, result=" 	
														<< result.result);
		_setEstablishedState(result.cntlrName, LDState::CREATED, time(0),
							 result.result == SD_RESULT_NO_ERROR);
		}
		break;

	case CONTROL_CONNECT:		// received from just started controller
		{
			CONTROLConnectEvent		msg(event);
			CONTROLConnectedEvent		answer;

			CIiter	controller = findController(msg.cntlrName);
			if (controller == itsCntlrList.end()) {		// not found?
				LOG_WARN_STR ("CONNECT event received from unknown controller: " <<
							  msg.cntlrName);
				answer.result = LD_RESULT_UNSPECIFIED;
			}
			else {
				LOG_DEBUG_STR("CONNECT event received from " << msg.cntlrName);
				_setEstablishedState(msg.cntlrName, LDState::CONNECTED, time(0), true);
				// first direct contact with controller, remember port
				controller->port = &port;
				answer.result = LD_RESULT_NO_ERROR;
			}
			port.send(answer);
		}
		break;

	case CONTROL_CLAIMED:
		break;
	
	case CONTROL_PREPARED:
		break;
	
	case CONTROL_RESUMED:
		break;
	
	case CONTROL_SUSPENDED:
		break;
	
	case CONTROL_RELEASED:
		break;
	
	case CONTROL_FINISH:
		break;
	

	default:
		LOG_DEBUG ("ChildControl::operational, default");
		status = GCFEvent::NOT_HANDLED;
		break;
	}

	return (status);
}

  } // namespace MainCU
} // namespace LOFAR
