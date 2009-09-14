//#  ParentControl.cc: Task that handles and dispatches all controllers events.
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
#include <Common/SystemUtil.h>
#include <Common/ParameterSet.h>
#include <ApplCommon/StationInfo.h>
#include <GCF/TM/GCF_Protocols.h>
#include <MACIO/MACServiceInfo.h>
#include <APL/APLCommon/APLUtilities.h>
#include <APL/APLCommon/ParentControl.h>
#include <Controller_Protocol.ph>
#include <StartDaemon_Protocol.ph>

namespace LOFAR {
  using namespace GCF::TM;
  namespace APLCommon {

typedef struct stateFlow_t {
	uint16				signal;
	CTState::CTstateNr	currentState;
	CTState::CTstateNr	requestedState;
} stateFlow;

static	stateFlow	stateFlowTable[] = {
//		received signal			expected in state		requested state
//		------------------------------------------------------------------
	{	CONTROL_STARTED,		CTState::ANYSTATE,		CTState::CREATED	},
	{	CONTROL_CONNECTED,		CTState::NOSTATE,		CTState::CONNECTED	},
	{	CONTROL_CONNECTED,		CTState::CONNECT,		CTState::CONNECTED	},
	{	CONTROL_CLAIM,			CTState::CONNECTED,		CTState::CLAIMED	},
	{	CONTROL_CLAIMED,		CTState::CLAIM,			CTState::CLAIMED	},
	{	CONTROL_PREPARE,		CTState::CLAIMED,		CTState::PREPARED	},
	{	CONTROL_PREPARED,		CTState::PREPARE,		CTState::PREPARED	},
	{	CONTROL_RESUME,			CTState::PREPARED,		CTState::RESUMED	},
	{	CONTROL_RESUME,			CTState::SUSPENDED,		CTState::RESUMED	},
	{	CONTROL_RESUMED,		CTState::RESUME,		CTState::RESUMED	},
	{	CONTROL_SUSPEND,		CTState::RESUMED,		CTState::SUSPENDED	},
	{	CONTROL_SUSPEND,		CTState::PREPARED,		CTState::SUSPENDED	},
	{	CONTROL_SUSPENDED,		CTState::SUSPEND,		CTState::SUSPENDED	},
	{	CONTROL_RELEASE,		CTState::CLAIMED,		CTState::RELEASED	},
	{	CONTROL_RELEASE,		CTState::PREPARED,		CTState::RELEASED	},
	{	CONTROL_RELEASE,		CTState::SUSPENDED,		CTState::RELEASED	},
	{	CONTROL_RELEASED,		CTState::RELEASE,		CTState::RELEASED	},
	{	CONTROL_QUIT,			CTState::CONNECTED,		CTState::QUITED		},
	{	CONTROL_QUIT,			CTState::RELEASED,		CTState::QUITED		},
	{	CONTROL_QUITED,			CTState::QUIT,			CTState::QUITED		},
	{	CONTROL_RESYNCED,		CTState::ANYSTATE,		CTState::ANYSTATE	},
	{	CONTROL_SCHEDULE,		CTState::ANYSTATE,		CTState::ANYSTATE	},
	{	0x00,					CTState::NOSTATE,		CTState::NOSTATE	}
};

//-------------------------- creation and destroy ---------------------------

ParentControl* ParentControl::instance()
{
	static	ParentControl*		theirParentControl;

	if (theirParentControl == 0) {
		theirParentControl = new ParentControl();
	}
	return (theirParentControl);
}


//
// ParentControl(name, parenthost, parentService))
//
ParentControl::ParentControl() :
	GCFTask			 ((State)&ParentControl::initial, "ParentControl"),
	itsParentList	 (),
	itsSDPort		 (0),
	itsMainTaskPort  (0),
	itsTimerPort	 (*this, "parentControlTimer"),
	itsControllerName()
{
	// Log the protocols I use.
	registerProtocol(CONTROLLER_PROTOCOL,  CONTROLLER_PROTOCOL_STRINGS);
	registerProtocol(STARTDAEMON_PROTOCOL, STARTDAEMON_PROTOCOL_STRINGS);

}

//
// ~ParentControl
//
ParentControl::~ParentControl()
{
	// close and delete the ports I created
	if (itsMainTaskPort) {
		itsMainTaskPort->close();
		delete itsMainTaskPort;
	}

	if (itsSDPort) {
		itsSDPort->close();
		delete itsSDPort;
	}
}

//
// registerTask(mainTask) : ITCPort
//
GCFITCPort*	ParentControl::registerTask(GCFTask*		mainTask)
{
	if (!itsMainTaskPort) {
		itsMainTaskPort = new GCFITCPort(*mainTask, *this, mainTask->getName(), 
											GCFPortInterface::SAP, CONTROLLER_PROTOCOL);
		ASSERTSTR(itsMainTaskPort, "Can not allocate ITC port for parent-control");
		itsMainTaskPort->open();		// will result in F_CONN signal

		itsSDPort = new GCFTCPPort(*this, MAC_SVCMASK_STARTDAEMON, 
											GCFPortInterface::SAP, STARTDAEMON_PROTOCOL);
		ASSERTSTR(itsSDPort, "Can not allocate clientport to startDaemon");
		itsSDPort->open();				// will result in F_CONN or F_DISCONN signal

		itsControllerName = mainTask->getName();		// remember for later
	}

	return (itsMainTaskPort);
}

//
// activateObservationTimers(cntlrName, startTime, stopTime)
//
// Initializes a timer for the begin and the end of the observation.
// At this time the ParentControl task will generate a RESUME and QUIT signal
// When one of the times is 'not_a_date_time' the corresponding timer is cleared.
// False is returned when (one of) the timers could not be set.
//
bool ParentControl::activateObservationTimers(const string&		cntlrName,
											  ptime				startTime, 
											  ptime				stopTime)
{
	LOG_DEBUG_STR("activateObsTimers(" << cntlrName <<","<< to_simple_string(startTime) <<
									","<< to_simple_string(stopTime) << ")");

	PIiter		parent = findParentOnName(cntlrName);
	if (!isParent(parent)) {
		LOG_ERROR_STR("Unknown controllername " << cntlrName << 
					  ". Can not activate observation-timers.");
		return (false);
	}

	bool	legalStartTime(!startTime.is_not_a_date_time());
	bool	legalStopTime (!stopTime.is_not_a_date_time());
	if (legalStartTime && legalStopTime && stopTime < startTime) {
		LOG_ERROR_STR("Stoptime(" << to_simple_string(stopTime) << 
						") lays BEFORE starttime(" << to_simple_string(startTime) << 
						"). Ignoring timesettings!");
		return (false);
	}

	ptime			now(from_time_t(time(0L)));		// get current time
	time_duration	startDiff(seconds(0));			// calc when to start
	time_duration	stopDiff (seconds(0));			// calc when to stop
	if (legalStartTime) {
		startDiff = startTime - now;				// calc when to start
		LOG_INFO_STR("Start is over " << to_simple_string(startDiff) << " hours at " 
					<< to_simple_string(startTime));
	}
	if (legalStopTime) {
		stopDiff = stopTime - now;
		LOG_INFO_STR("Stop is over  " << to_simple_string(stopDiff) << " hours at " 
					<< to_simple_string(stopTime));
	}

	//	startdiff	stopdiff	action
	//		>0			>0		everything in the future, set timers
	//		>0			<0		stopTime < startTime, captured above
	//		<0			>0		obs should be running by now!!
	//		<0			<0		obs is over!
	if (stopDiff.seconds() < 0) {
		LOG_ERROR("Stoptime is already past! Shutting down controller.");
		parent->requestedState = CTState::QUITED;
		parent->requestTime	   = time(0);
		_doRequestedAction(parent);
		return (false);
	}

	if (startDiff.seconds() < 0) {
		LOG_WARN("Observation should have been started, going to start-state a.s.a.p.");
		parent->requestedState = CTState::RESUMED;
		parent->requestTime	   = time(0);
		_doRequestedAction(parent);
	}

	// set or reset the real timers.
	itsTimerPort.cancelTimer(parent->startTimer);
	if (startDiff.seconds() > 0) {
		parent->startTimer = itsTimerPort.setTimer((double)startDiff.seconds());
	}
	else {
		parent->startTimer = 0;
	}

	itsTimerPort.cancelTimer(parent->stopTimer);
	if (stopDiff.seconds() > 0) {
		parent->stopTimer = itsTimerPort.setTimer((double)stopDiff.seconds());
	}
	else {
		parent->stopTimer = 0;
	}

	return (true);
}

//
// nowInState(name, newstate)
//
// The main task can inform the ParentControl-task what state it is in now.
// When the commands to change state come from the parent executable this is
// not neccesary because the ParentControl-task knows in what state the main-task
// should be. But when the main-task decides on his own that he needs to be in
// another state than he has to inform the ParentControl-task about it, otherwise
// these two tasks become out of sync.
//
bool ParentControl::nowInState(const string&		cntlrName,
							   CTState::CTstateNr	newState)
{
	CTState		cts;
	LOG_DEBUG_STR("nowInState(" << cntlrName <<","<< cts.name(newState) << ")");

	PIiter		parent = findParentOnName(cntlrName);
	if (!isParent(parent)) {
		LOG_ERROR_STR("Unknown controllername " << cntlrName << 
					  ", can not register new state: " << cts.name(newState));
		return (false);
	}

	parent->currentState   = newState;
	parent->requestedState = requestedState(cts.signal(newState));
	// Note: by converting the state to a signal and than back to the requested
	//		 state we translate xxxing states to xxxed states.
	return (true);
}

// -------------------- PRIVATE FUNCTIONS --------------------

//
// isLegalSignal(signal, parent)
//
// Checks if the given signal for the given parent is allowed in (t)his state.
//
bool ParentControl::isLegalSignal(uint16	aSignal,
							   PIiter	aParent)
{
	uint32	i = 0;
	while (stateFlowTable[i].signal) {
		if (stateFlowTable[i].signal == aSignal && 
			(stateFlowTable[i].currentState == aParent->currentState ||
			 stateFlowTable[i].currentState == CTState::ANYSTATE)) {
			return (true);
		}
		i++;
	}
	return (false);
}

//
// requestedState(signal)
//
// Translate received signal into the state that must be reached.
//
CTState::CTstateNr ParentControl::requestedState(uint16	aSignal)
{
	uint32	i = 0;
	while (stateFlowTable[i].signal) {
		if (stateFlowTable[i].signal == aSignal) {
			return (stateFlowTable[i].requestedState);
		}
		i++;
	}
	ASSERTSTR(false, "No new state defined for signal " << eventName(aSignal));
}

//
// getNextState(parent)
//
// Returns the state that must be realized. When a signal is received 'out of band'
// of the 'normal' sequence the missing state is returned.
//
CTState::CTstateNr ParentControl::getNextState(PIiter		parent)
{
	if (parent->currentState == parent->requestedState) {
		return (parent->requestedState);
	}

	// look if signal is inband
	uint32	i = 0;
	while (stateFlowTable[i].signal) {
		if ((stateFlowTable[i].requestedState == parent->requestedState) &&
			(stateFlowTable[i].currentState == parent->currentState)) {
			// yes, requested state is allowed in the current state
			return(stateFlowTable[i].requestedState);
		}
		i++;
	}

	// signal is not an inband signal, try to find a path the leads to the req. state
	CTState		cts;
	CTState::CTstateNr	requestedState = parent->requestedState;
	CTState::CTstateNr	currentState   = parent->currentState;
	i = 0;
	for(;;) {
		// find matching requested state
		if (stateFlowTable[i].requestedState == requestedState) {
			// does (moved) currentState match state of table?
			if (stateFlowTable[i].currentState == currentState) {
				LOG_INFO_STR("State change from " << cts.name(parent->currentState) <<
						" to " << cts.name(parent->requestedState) << 
						" is out of band. First going to state " << 
						cts.name(stateFlowTable[i].requestedState));
				return (stateFlowTable[i].requestedState);
			}

			// can requested state be reached from any state?
			if (stateFlowTable[i].currentState == CTState::ANYSTATE) {
				return (stateFlowTable[i].requestedState);
			}

			// does this state lay between currentstate of parent and requested?
			if (stateFlowTable[i].currentState > currentState) {
				// adopt this step and try to resolve it
				requestedState = stateFlowTable[i].currentState;
				i = 0;
				continue;
			}
		}

		i++;
		if (!stateFlowTable[i].signal) {
			// no matching route found. Just return requested state and 
			// hope for the best.
			LOG_WARN_STR("Not supported state change from " << 
						cts.name(parent->currentState) << " to " << 
						cts.name(parent->requestedState) << ". Hope it will work!");
			return (parent->requestedState);
		}
	}
}

//
// _doRequestedAction(parent)
//
void ParentControl::_doRequestedAction(PIiter	parent)
{
	CTState	cts;
	LOG_DEBUG_STR("_doRequestedAction:" << parent->name << " : " << 
			cts.name(parent->currentState) << "-->" << cts.name(parent->requestedState));

	// state already reached? make sure the timer is off.
	if (parent->requestedState == parent->currentState) {
		parent->nrRetries = -1;
		itsTimerPort.cancelTimer(parent->timerID);
		parent->timerID = 0;
	}

//	switch (parent->requestedState) {
	switch (getNextState(parent)) {
	case CTState::CONNECTED: {
			// try to make first contact with parent by identifying ourself
			if (parent->nrRetries == 0) {	// no more retries left?
				LOG_ERROR_STR("Cannot connect to parent: " << parent->servicename <<
							"@" << parent->hostname << ", giving up!");
				parent->establishTime = time(0);
				parent->failed = true;		// TODO:let garbage collection remove this entry.
				return;
			}
			// Note: when required state == CONNECTED there are two possibilities:
			// current state == NOSTATE: CONNECT msg was sent to main task, do nothing
			// current state == CONNECT: CONNECTED msg was received from main task,
			//							 send CONNECT msg to parent-controller.
			// current state == CONNECTED: we are in a RESYNC cycle, do nothing.
			if (parent->currentState == CTState::NOSTATE || 
				parent->currentState == CTState::CONNECTED) {
				return;
			}

			ASSERTSTR(parent->currentState == CTState::CONNECT, 
								"Unexpected state:" << parent->currentState);

			// construct and send message
			CONTROLConnectEvent		hello;
			hello.cntlrName = parent->name;
			parent->port->send(hello);

			// (re)set the parameters of this connection
			parent->timerID 	  = 0;
			parent->nrRetries 	  = -1;
			parent->currentState  = CTState::CONNECT;
			parent->establishTime = time(0);
		}
		break;

	case CTState::CLAIMED: {
			CONTROLClaimEvent	request;
			request.cntlrName = parent->name;
			itsMainTaskPort->sendBack(request);
		}
		break;
	case CTState::PREPARED: {
			CONTROLPrepareEvent	request;
			request.cntlrName = parent->name;
			itsMainTaskPort->sendBack(request);
		}
		break;
	case CTState::RESUMED: {
			CONTROLResumeEvent	request;
			request.cntlrName = parent->name;
			itsMainTaskPort->sendBack(request);
		}
		break;
	case CTState::SUSPENDED: {
			CONTROLSuspendEvent	request;
			request.cntlrName = parent->name;
			itsMainTaskPort->sendBack(request);
		}
		break;
	case CTState::RELEASED: {
			CONTROLReleaseEvent	request;
			request.cntlrName = parent->name;
			itsMainTaskPort->sendBack(request);
		}
		break;
	case CTState::SCHEDULED: {
			CONTROLScheduleEvent	request;
			request.cntlrName = parent->name;
			itsMainTaskPort->sendBack(request);
		}
		break;
	case CTState::QUITED: {
			CONTROLQuitEvent	request;
			request.cntlrName = parent->name;
			itsMainTaskPort->sendBack(request);
		}
		break;
	default:
		ASSERTSTR(false, "Serious programming error, requestedState=" << 
					parent->requestedState << " for task " << parent->name);
	}

}

//
// _confirmState(msg, cntlrName, result)
//
bool ParentControl::_confirmState(uint16			signal,
								  const string&		cntlrName,
								  uint16			result)
{
	PIiter		parent = findParentOnName(cntlrName);
	if (!isParent(parent)) {
		LOG_WARN_STR ("Received answer for unknown parent " << cntlrName <<", ignoring");
		return (false);
	}

	CTState	cts;
//	if (!isLegalSignal(event.signal, parent)) {
//		LOG_WARN_STR ("Received " << eventName(event.signal) <<
//					  " event while requested state is " << 
//					  cts.name(parent->requestedState));
//	}

	if (result != CT_RESULT_NO_ERROR) {		// error reaching a state?
		parent->failed = true;				// report problem
		LOG_ERROR_STR(cntlrName << " DID NOT reach the " << 
			cts.name(requestedState(signal)) << " state, error=" << result);
		// if we are NOT trying to quit, don't continue with the state-sequence.
		// when we ARE trying to reach the QUIT state continue otherwise we will 
		// be running forever.
		if (parent->requestedState != CTState::QUITED) {
			return (false);
		}
	}
	else {		// no error while reaching the new state
		LOG_DEBUG_STR(cntlrName << " reached " << cts.name(cts.signal2stateNr(signal)) << 
							" state succesfully");
	}
	parent->currentState = requestedState(signal);			// store new state

	if (parent->currentState != parent->requestedState) {	// chain of states?
		_doRequestedAction(parent);							// start next step
	}

	return (true);
}

// -------------------- THE GCF STATEMACHINES --------------------

// Wait for the F_CONNECTED event from the ITC port with the master-task.
// Connect and register at startDaemon
//
GCFEvent::TResult	ParentControl::initial(GCFEvent&			event, 
										   GCFPortInterface&	port)

{
	LOG_DEBUG_STR ("initial:" << eventName(event) << "@" << port.getName());

	GCFEvent::TResult	status = GCFEvent::HANDLED;

	switch (event.signal) {
	case F_INIT:
	case F_ENTRY:
		break;

	case F_CONNECTED:
		if (&port == itsMainTaskPort) {		// ITC port with parent open?
			LOG_DEBUG("Connection with maintask opened");
		}
		if (&port == itsSDPort) {
			STARTDAEMONAnnouncementEvent	msg;
			msg.cntlrName = itsControllerName;
			itsSDPort->send(msg);

			LOG_DEBUG_STR("Registered at startdaemon as " << itsControllerName
						<< 	", going to operational mode");
			TRAN(ParentControl::operational);
		}
		break;

	case F_DISCONNECTED:
		if (&port == itsSDPort) {
			port.close();
			itsTimerPort.setTimer(1.0);
			LOG_DEBUG("Could not connect to startDaemon, retry in 1 second");
		}
		break;

	case F_TIMER:
		// must be reconnect timer for startDaemon
		itsSDPort->open();
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
// Since this base class is also used for shared controllers the controller supports
// multiple parents. For each of these parents the controller plays the role of
// stand-alone controller having a certain state and allowing certain actions.
// It is therefor difficult to implement multiple statemachine since the controller
// should be in a (different) state for each parent.
//
GCFEvent::TResult	ParentControl::operational(GCFEvent&			event, 
											   GCFPortInterface&	port)

{
	LOG_DEBUG_STR ("operational:" << eventName(event) << "@" << port.getName());

	CTState				cts;
	GCFEvent::TResult	status = GCFEvent::HANDLED;

	switch (event.signal) {
	case F_INIT:
		break;

	case F_ENTRY:
		break;

	case F_ACCEPT_REQ:	// will not occur, using SPP port for startDaemon
		break;

	case F_CONNECTED: {
			// search which connection is succesfull connected.
			PIiter		parent = findParentOnPort(&port);
			if (!isParent(parent)) {
				LOG_DEBUG("F_CONNECTED on non-parent port");
				break;
			}

			// first connection every made with this controller?
			if (parent->requestedState == CTState::CONNECTED && 
							parent->currentState < CTState::CONNECTED) {
				_doRequestedAction(parent);	// do queued action if any.
				break;
			}

			// Connecting is a reconnect, first resync with parent
			LOG_DEBUG_STR ("Sending RESYNC(" << parent->name << "," 
											<< cts.name(parent->currentState) << ")");
			CONTROLResyncEvent	resync;
			resync.cntlrName = parent->name;
			resync.curState	 = parent->currentState;
			resync.hostname	 = myHostname(true);
			port.send(resync);		// will result in CONTROL_RESYNCED;
		}
		break;

	case F_DISCONNECTED: {
			// StartDaemonPort? no problem SD will reconnect when neccesary.
			if (&port == itsSDPort) {
				port.close();			// close connection with current SD
				port.open();			// start listener again
				break;
			}

			// Parent port? Might be temporarely problem, try to reconnect for a while.
			PIiter	parent = findParentOnPort(&port);
			if (isParent(parent)) {
				// trying to make first contact?
				if (parent->requestedState == CTState::CONNECTED &&
									parent->currentState < CTState::CONNECTED) {
					// try again in one second.
					parent->port->close();
					parent->timerID = itsTimerPort.setTimer(1.0);
					if (parent->nrRetries < 0) {
						parent->nrRetries = 30;
						LOG_WARN_STR ("Lost connection with new parent " << parent->name 
									<< ", starting reconnect sequence");
					}
					LOG_TRACE_VAR_STR("parent:" << parent->name << ", timerID:" 
												<< parent->timerID);
					break;
				}
				
				// lost connection during shutdown phase? ok, update admin.
				if (parent->requestedState >= CTState::RELEASED) {
					LOG_DEBUG_STR("Removing " << parent->name << " from administration");
					parent->port->close();
					itsParentList.erase(parent);
					break;
				}

				// lost connection during normal operation, start reconnect sequence
				parent->port->close();
				parent->timerID   = itsTimerPort.setTimer(10.0);// retry every 10 seconds
				if (parent->nrRetries < 0) {
					parent->nrRetries = 360;						// for 1 hour
					LOG_WARN_STR ("Lost connection with parent " << parent->name <<
								  ", starting reconnect sequence");
				}
				LOG_TRACE_VAR_STR("parent:" << parent->name << ", timerID:" 
											<< parent->timerID);
				break;
			} 

			// unknown port. just close it.
			LOG_INFO ("Lost connection with unknown port, closing it");
			port.close();
		}
		break;

	case F_TIMER: {
			GCFTimerEvent&		timerEvent = static_cast<GCFTimerEvent&>(event);
			LOG_TRACE_VAR_STR("timerID:" << timerEvent.id);
			PIiter				parent = findParentOnTimerID(timerEvent.id);
			if (!isParent(parent)) {
				LOG_DEBUG ("timerevent is not of a known parent, ignore");
				break;
			}

			if (parent->port->isConnected()) {		// not the reconnect timer?
				_doRequestedAction(parent);
			}

			// its the reconnect timer of this parent.
			if (parent->nrRetries > 0) {
				parent->port->open();		// result in F_CONN or F_DISCONN
				parent->nrRetries--;
				parent->timerID = 0;
				if (!parent->nrRetries%60) {
					LOG_WARN_STR ("Still no connection with parent " << parent->name);
				}
			}
			else {
				LOG_WARN_STR ("Could not reconnect to parent " << 
							parent->name << ", deleting entry");
//					concrete_release(parent);
				itsParentList.erase(parent);
			}
		}
		break;

	case STARTDAEMON_NEWPARENT: {
		// a new parent wants us to connect to it,
		STARTDAEMONNewparentEvent	NPevent(event);

		// Attempt to start me up for the second time??? send resync
		PIiter	oldParent = findParentOnName(NPevent.cntlrName);
		if (isParent(oldParent)) {
			LOG_DEBUG_STR ("Attempt to start me up twice, just sending RESYNC(" 
				<< oldParent->name << "," << cts.name(oldParent->currentState) << ")");
			CONTROLResyncEvent	resync;
			resync.cntlrName = oldParent->name;
			resync.curState	 = oldParent->currentState;
			resync.hostname	 = myHostname(true);
			oldParent->port->send(resync);		// will result in CONTROL_RESYNCED;
			break;
		}

		// construct the state information the parent think we have.
		ParentInfo_t			parent;
		parent.name 		  = NPevent.cntlrName;
		parent.port			  = new GCFTCPPort(*this, NPevent.parentService,
									GCFPortInterface::SAP, CONTROLLER_PROTOCOL);
		parent.hostname 	  = NPevent.parentHost;
		parent.servicename 	  = NPevent.parentService;
		parent.requestedState = CTState::CONNECTED;
		parent.requestTime 	  = time(0);
		parent.currentState   = CTState::NOSTATE;
		parent.establishTime  = 0;
		parent.timerID	 	  = 0;
		parent.nrRetries 	  = -1;	// means: not set
		ASSERTSTR(parent.port, "Unable to allocate socket for " << NPevent.cntlrName);

		// add parent to the pool and open the connection with the parent
		itsParentList.push_back(parent);

		// Dynamic cast causes crash!?? 
		static_cast<GCFTCPPort*>(parent.port)->setHostName (parent.hostname);

		parent.port->open();		// results in F_CONN of F_DISCONN
		LOG_DEBUG_STR("Registered parent "<< parent.name <<" on port "<< parent.port);

		// pass newParent event to mainTask as Connect event
		LOG_DEBUG_STR("Sending CONNECT(" << parent.name << ") event to maintask");
		CONTROLConnectEvent		request;
		request.cntlrName = parent.name;
		itsMainTaskPort->sendBack(request);
		}
		break;

	case CONTROL_CONNECTED: {
		// this event can be received from maintask and parent-controllers
		// is it the maintask? --> send CONNECT to parent controller.
		if (&port == itsMainTaskPort) {
			CONTROLConnectedEvent	inMsg(event);
			PIiter	parent = findParentOnName(inMsg.cntlrName);
			if (!isParent(parent)) {
				LOG_ERROR_STR("Cannot forward CONNECTED event for " << inMsg.cntlrName);
				break;
			}
			CONTROLConnectEvent	outMsg;
			outMsg.cntlrName = inMsg.cntlrName;
			parent->port->send(outMsg);
			parent->currentState = CTState::CONNECT;
			break;
		}
				
		// should be a parentport
		PIiter		parent = findParentOnPort(&port);
		if (!isParent(parent)) {
			CONTROLConnectedEvent	inMsg(event);
			LOG_WARN_STR ("Received CONNECTED event from unknown parent (" <<
													inMsg.cntlrName << "), ignoring");
			break;
		}

		// warn operator when we are out of sync with our parent.
		if (parent->requestedState != CTState::CONNECTED) {
			LOG_WARN_STR ("Received 'CONNECTED' event while requested state is " <<
						cts.name(parent->requestedState));
		}
		// always accept new state because parent thinks we are in this state.
		// TODO: when we are already beyond the claiming state should we release
		//		 the claimed resources?
		parent->port->cancelTimer(parent->timerID);
		parent->timerID 	  = 0;
		parent->currentState  = CTState::CONNECTED;
		parent->establishTime = time(0);
		parent->timerID		  = 0;
		parent->nrRetries 	  = -1;
		parent->failed 	 	  = false;
		}
		break;

	// -------------------- commands from parent executable --------------------
	case CONTROL_RESYNC:
	case CONTROL_SCHEDULE:
	case CONTROL_CLAIM:
	case CONTROL_PREPARE:
	case CONTROL_RESUME:
	case CONTROL_SUSPEND:
	case CONTROL_RELEASE:
	case CONTROL_QUIT:
		{
			// do we know this parent?
			PIiter		parent = findParentOnPort(&port);
			if (!isParent(parent)) {
				LOG_WARN_STR ("Received " << eventName(event) << 
								" event from unknown parent, ignoring");
				break;
			}

			// warn operator when we are out of sync with our parent.
			if (!isLegalSignal(event.signal, parent)) {
				LOG_WARN_STR ("Received " << eventName(event.signal) <<
							" event while requested state is " << 
							cts.name(parent->requestedState));
			}

			if (event.signal == CONTROL_SCHEDULE) {		// reschedule request?
				CONTROLScheduleEvent		schedMsg(event);
				if (parent->stopTimer) {				// do we keep track of the time?
					activateObservationTimers(parent->name, 	// yes
											  from_time_t(schedMsg.startTime), 
											  from_time_t(schedMsg.stopTime));
					// don't bother maintask with reschedule.
					CONTROLScheduledEvent	answer;
					answer.cntlrName = schedMsg.cntlrName;
					answer.result    = CT_RESULT_NO_ERROR;
					parent->port->send(answer);
					return (GCFEvent::HANDLED);
				}
				// maintask does not use my timers, just pass through the event.
				itsMainTaskPort->sendBack(schedMsg);
				return (GCFEvent::HANDLED);
			}

			// When we were resyncing just continue what we were trying to do.
			if (event.signal != CONTROL_RESYNCED && event.signal != CONTROL_SCHEDULE) {
				// use stateFlowTable to determine the required state.
				parent->requestedState = requestedState(event.signal);	
			}
			parent->requestTime	   = time(0);
			_doRequestedAction(parent);
		}
		break;

	// -------------------- SIGNALS FROM MAIN TASK --------------------
	case CONTROL_CLAIMED:
		{
			CONTROLClaimedEvent		msg(event);
			// do we know this parent?
			PIiter		parent = findParentOnName(msg.cntlrName);
			if (!isParent(parent)) {
				LOG_WARN_STR ("Received "<< eventName(event) <<" answer for unknown parent "
								<< msg.cntlrName <<", ignoring");
				break;
			}
			_confirmState(event.signal, msg.cntlrName, msg.result);
			parent->port->send(msg);
		}
		break;

	case CONTROL_PREPARED:
		{
			CONTROLPreparedEvent	msg(event);
			// do we know this parent?
			PIiter		parent = findParentOnName(msg.cntlrName);
			if (!isParent(parent)) {
				LOG_WARN_STR ("Received "<< eventName(event) <<" answer for unknown parent "
								<< msg.cntlrName <<", ignoring");
				break;
			}
			_confirmState(event.signal, msg.cntlrName, msg.result);
			parent->port->send(msg);
		}
		break;

	case CONTROL_RESUMED:
		{
			CONTROLResumedEvent	msg(event);
			// do we know this parent?
			PIiter		parent = findParentOnName(msg.cntlrName);
			if (!isParent(parent)) {
				LOG_WARN_STR ("Received "<< eventName(event) <<" answer for unknown parent "
								<< msg.cntlrName <<", ignoring");
				break;
			}
			_confirmState(event.signal, msg.cntlrName, msg.result);
			parent->port->send(msg);
		}
		break;

	case CONTROL_SUSPENDED:
		{
			CONTROLSuspendedEvent	msg(event);
			// do we know this parent?
			PIiter		parent = findParentOnName(msg.cntlrName);
			if (!isParent(parent)) {
				LOG_WARN_STR ("Received "<< eventName(event) <<" answer for unknown parent "
								<< msg.cntlrName <<", ignoring");
				break;
			}
			_confirmState(event.signal, msg.cntlrName, msg.result);
			parent->port->send(msg);
		}
		break;

	case CONTROL_RELEASED:
		{
			CONTROLReleasedEvent	msg(event);
			// do we know this parent?
			PIiter		parent = findParentOnName(msg.cntlrName);
			if (!isParent(parent)) {
				LOG_WARN_STR ("Received "<< eventName(event) <<" answer for unknown parent "
								<< msg.cntlrName <<", ignoring");
				break;
			}
			_confirmState(event.signal, msg.cntlrName, msg.result);
			parent->port->send(msg);
		}
		break;

	case CONTROL_RESYNCED:
		{
			CONTROLResyncedEvent	msg(event);
			// do we know this parent?
			PIiter		parent = findParentOnName(msg.cntlrName);
			if (isParent(parent)) {
				// note do not register this state, it is not a real state
				LOG_INFO_STR("Controller " << msg.cntlrName << " is resynced");
				// REO 300507
				_doRequestedAction(parent);	// do queued action if any.
			}
			else {
				LOG_WARN_STR("Received RESYNCED message from unknown controller " 
																	<< msg.cntlrName);
			}
		}
		break;

	case CONTROL_SCHEDULED:
		{
			CONTROLScheduledEvent	msg(event);
			// do we know this parent?
			PIiter		parent = findParentOnName(msg.cntlrName);
			if (isParent(parent)) {
				// note do not register this state, it is not a real state
				LOG_DEBUG_STR("Passing SCHEDULED event to parent controller");
				parent->port->send(msg);
			}
		}
		break;

	case CONTROL_QUITED:
		{
			CONTROLQuitedEvent	msg(event);
			// do we know this parent?
			PIiter		parent = findParentOnName(msg.cntlrName);
			if (!isParent(parent)) {
				LOG_WARN_STR("Controller " << msg.cntlrName << " not in administration, ignoring QUIT message");
				break;
			}

			// Known parent, update admin and pass message to main task
			_confirmState(event.signal, msg.cntlrName, msg.result);
			parent->port->send(msg);

			// when message is of a non-shared controller close the port.
			if (!isSharedController(getControllerType(msg.cntlrName))) {
				LOG_DEBUG_STR("Not a shared controller, closing port");
				parent->port->close();
				itsParentList.erase(parent);
			}
		}
		break;

	default:
		LOG_DEBUG ("operational, default");
		status = GCFEvent::NOT_HANDLED;
		break;
	}

	return (status);
}

//
// findParentOnPort(port)
//
ParentControl::PIiter	ParentControl::findParentOnPort(GCFPortInterface*	aPort)
{
	LOG_TRACE_COND_STR("findParentOnPort: " << aPort);
	const_PIiter	end  = itsParentList.end();
	PIiter			iter = itsParentList.begin();
	while (iter != end && iter->port != aPort) {
		iter++;	
	}
	return (iter);
}

//
// findParentOnTimerID(timerID)
//
ParentControl::PIiter	ParentControl::findParentOnTimerID(uint32	aTimerID)
{
	LOG_TRACE_COND_STR("findParentOnTimerID: " << aTimerID);
	const_PIiter	end  = itsParentList.end();
	PIiter			iter = itsParentList.begin();
	while (iter != end && iter->timerID != aTimerID) {
		iter++;	
	}
	return (iter);
}

//
// findParentOnName(name)
//
ParentControl::PIiter	ParentControl::findParentOnName(const string&		aName)
{
	LOG_TRACE_COND_STR("findParentOnName: " << aName);
	const_PIiter	end  = itsParentList.end();
	PIiter			iter = itsParentList.begin();
	while (iter != end && iter->name != aName) {
		iter++;	
	}
	return (iter);
}

  } // namespace APLCommon
} // namespace LOFAR
