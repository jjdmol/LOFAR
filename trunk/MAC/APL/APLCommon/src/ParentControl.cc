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
#include <Common/Deployment.h>
#include <APS/ParameterSet.h>
#include <GCF/GCF_ServiceInfo.h>
#include <GCF/Utils.h>
#include <APL/APLCommon/APLUtilities.h>
#include <APL/APLCommon/ParentControl.h>
#include <Controller_Protocol.ph>
#include <StartDaemon_Protocol.ph>

namespace LOFAR {
  using namespace GCF::TM;
  using namespace ACC::APS;
  namespace APLCommon {

typedef struct stateFlow_t {
	uint16				signal;
	CTState::CTstateNr	currentState;
	CTState::CTstateNr	requestedState;
} stateFlow;

static	stateFlow	stateFlowTable[] = {
//		received signal			expected in state		requested state
//		------------------------------------------------------------------
	{	CONTROL_CONNECTED,		CTState::CONNECT,		CTState::CONNECTED	},
	{	CONTROL_CLAIM,			CTState::CONNECTED,		CTState::CLAIMED	},
	{	CONTROL_PREPARE,		CTState::CLAIMED,		CTState::PREPARED	},
	{	CONTROL_RESUME,			CTState::PREPARED,		CTState::ACTIVE		},
	{	CONTROL_RESUME,			CTState::SUSPENDED,		CTState::ACTIVE		},
	{	CONTROL_SUSPEND,		CTState::ACTIVE,		CTState::SUSPENDED	},
	{	CONTROL_RELEASE,		CTState::ANYSTATE,		CTState::RELEASED	},
	{	CONTROL_FINISHED,		CTState::FINISH,		CTState::FINISHED	},
	{	CONTROL_RESYNCED,		CTState::ANYSTATE,		CTState::ANYSTATE	},
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
// ParentControl(name, porenthost, parentService))
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
	registerProtocol(CONTROLLER_PROTOCOL,  CONTROLLER_PROTOCOL_signalnames);
	registerProtocol(STARTDAEMON_PROTOCOL, STARTDAEMON_PROTOCOL_signalnames);

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
	itsMainTaskPort = new GCFITCPort(*mainTask, *this, mainTask->getName(), 
										GCFPortInterface::SAP, CONTROLLER_PROTOCOL);
	ASSERTSTR(itsMainTaskPort, "Can not allocate ITC port for parent-control");
	itsMainTaskPort->open();		// will result in F_CONN signal

	itsSDPort = new GCFTCPPort(*this, MAC_SVCMASK_STARTDAEMON, 
										GCFPortInterface::SAP, STARTDAEMON_PROTOCOL);
	ASSERTSTR(itsSDPort, "Can not allocate clientport to startDaemon");
	itsSDPort->open();				// will result in F_COON or F_DISCONN signal

	itsControllerName = mainTask->getName();		// remember for later

	return (itsMainTaskPort);
}

//
// isLegalSignal(signal, parent)
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
CTState::CTstateNr ParentControl::requestedState(uint16	aSignal)
{
	uint32	i = 0;
	while (stateFlowTable[i].signal) {
		if (stateFlowTable[i].signal == aSignal) {
			return (stateFlowTable[i].requestedState);
		}
		i++;
	}
	ASSERTSTR(false, "No new state defined for signal " << evtstr(aSignal));
}

//
// doRequestedAction(parent)
//
void ParentControl::doRequestedAction(PIiter	parent)
{
	CTState	cts;
	LOG_DEBUG_STR("doRequestedAction:" << parent->name << " : " << 
			cts.name(parent->currentState) << "-->" << cts.name(parent->requestedState));

	// state already reached? make sure the timer is off.
	if (parent->requestedState == parent->currentState) {
		parent->nrRetries = -1;
		itsTimerPort.cancelTimer(parent->timerID);
		parent->timerID = 0;
		return;
	}

	switch (parent->requestedState) {
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
			if (parent->currentState == CTState::NOSTATE) {
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
	case CTState::PREPARED:
//		concrete_prepare(parent);
		break;
	case CTState::ACTIVE:
//		concrete_resume(parent);
		break;
	case CTState::SUSPENDED:
//		concrete_suspend(parent);
		break;
	case CTState::RELEASED:
//		concrete_release(parent);
		break;
	case CTState::FINISHED:
		parent->port->close();
		itsParentList.erase(parent);
		break;
	default:
		ASSERTSTR(false, "Serious programming error, requestedState=" << 
					parent->requestedState << " for task " << parent->name);
	}

}


// -------------------- THE GCF STATEMACHINES --------------------

// Wait for the F_CONNECTED event from the ITC port with the master-task.
// Connect and register at startDaemon
//
GCFEvent::TResult	ParentControl::initial(GCFEvent&			event, 
										   GCFPortInterface&	port)

{
	LOG_DEBUG_STR ("initial:" << evtstr(event) << "@" << port.getName());

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
	LOG_DEBUG_STR ("operational:" << evtstr(event) << "@" << port.getName());

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
			PIiter		parent = findParent(&port);
			if (!isParent(parent)) {
				LOG_DEBUG("F_CONNECTED on non-parent port");
				break;
			}

			// first connection every made with this controller?
			if (parent->requestedState == CTState::CONNECTED && 
							parent->currentState == CTState::CONNECT) {
				doRequestedAction(parent);	// do queued action if any.
				break;
			}

			// Connecting is a reconnect, first resync with parent
			CTState		cts;
			LOG_DEBUG_STR ("Sending RESYNC(" << parent->name << "," 
											<< cts.name(parent->currentState) << ")");
			CONTROLResyncEvent	resync;
			resync.cntlrName = parent->name;
			resync.curState	 = parent->currentState;
			resync.hostname	 = myHostname(true);
			port.send(resync);		// will result in CONTROLF_RESYNCED;
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
			PIiter	parent = findParent(&port);
			if (isParent(parent)) {
				// trying to make first contact?
				if (parent->requestedState == CTState::CONNECTED &&
									parent->currentState != CTState::CONNECTED) {
					// try again in one second.
					parent->port->close();
					parent->timerID = itsTimerPort.setTimer(1.0);
					if (parent->nrRetries < 0) {
						parent->nrRetries = 30;
						LOG_WARN_STR ("Lost connection with new parent " << parent->name <<
									  ", starting reconnect sequence");
					}
					LOG_TRACE_VAR_STR("parent:" << parent->name << ", timerID:" 
												<< parent->timerID);
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
			PIiter				parent = findParent(timerEvent.id);
			if (!isParent(parent)) {
				LOG_DEBUG ("timerevent is not of a known parent, ignore");
				break;
			}

			if (parent->port->isConnected()) {		// not the reconnect timer?
				doRequestedAction(parent);
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
		PIiter	oldParent = findParent(NPevent.cntlrName);
		if (isParent(oldParent)) {
			CTState		cts;
			LOG_DEBUG_STR ("Attempt to start me up twice, just sending RESYNC(" 
				<< oldParent->name << "," << cts.name(oldParent->currentState) << ")");
			CONTROLResyncEvent	resync;
			resync.cntlrName = oldParent->name;
			resync.curState	 = oldParent->currentState;
			resync.hostname	 = myHostname(true);
			port.send(resync);		// will result in CONTROL_RESYNCED;
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
		dynamic_cast<GCFTCPPort*>(parent.port)->setHostName (parent.hostname);
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
			PIiter	parent = findParent(inMsg.cntlrName);
			if (!isParent(parent)) {
				LOG_ERROR_STR("Cannot forward CONNECT event for " << inMsg.cntlrName);
				break;
			}
			CONTROLConnectEvent	outMsg;
			outMsg.cntlrName = inMsg.cntlrName;
			parent->port->send(outMsg);
			parent->currentState = CTState::CONNECT;
			break;
		}
				
		// should be a parentport
		PIiter		parent = findParent(&port);
		if (!isParent(parent)) {
			LOG_WARN_STR ("Received CONNECTED event from unknown parent, ignoring");
			break;
		}

		// warn operator when we are out of sync with our parent.
		if (parent->requestedState != CTState::CONNECTED) {
			LOG_WARN_STR ("Received 'CONNECTED' event while requested state is " <<
						parent->requestedState);
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

	case CONTROL_RESYNCED:
	case CONTROL_CLAIM:
	case CONTROL_PREPARE:
	case CONTROL_RESUME:
	case CONTROL_SUSPEND:
	case CONTROL_RELEASE:
	case CONTROL_FINISHED:
		{
			// do we know this parent?
			PIiter		parent = findParent(&port);
			if (!isParent(parent)) {
				LOG_WARN_STR ("Received " << evtstr(event) << 
								" event from unknown parent, ignoring");
				break;
			}

			// warn operator when we are out of sync with our parent.
			if (!isLegalSignal(event.signal, parent)) {
				LOG_WARN_STR ("Received " << evtstr(event.signal) <<
							" event while requested state is " << parent->requestedState);
			}

			// When we were resyncing yust continue what we were trying to do.
			if (event.signal != CONTROL_RESYNCED) {
				// use stateFlowTable to determine the required state.
				parent->requestedState = requestedState(event.signal);	
			}
			parent->requestTime	   = time(0);
			doRequestedAction(parent);
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
// findParent(port)
//
ParentControl::PIiter	ParentControl::findParent(GCFPortInterface*	aPort)
{
	LOG_TRACE_COND_STR("findParent: " << aPort);
	const_PIiter	end  = itsParentList.end();
	PIiter			iter = itsParentList.begin();
	while (iter != end && iter->port != aPort) {
		iter++;	
	}
	return (iter);
}

//
// findParent(timerID)
//
ParentControl::PIiter	ParentControl::findParent(uint32	aTimerID)
{
	LOG_TRACE_COND_STR("findParent: " << aTimerID);
	const_PIiter	end  = itsParentList.end();
	PIiter			iter = itsParentList.begin();
	while (iter != end && iter->timerID != aTimerID) {
		iter++;	
	}
	return (iter);
}

//
// findParent(name)
//
ParentControl::PIiter	ParentControl::findParent(const string&		aName)
{
	LOG_TRACE_COND_STR("findParent: " << aName);
	const_PIiter	end  = itsParentList.end();
	PIiter			iter = itsParentList.begin();
	while (iter != end && iter->name != aName) {
		iter++;	
	}
	return (iter);
}

  } // namespace APLCommon
} // namespace LOFAR
