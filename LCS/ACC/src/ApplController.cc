//#  ApplController.cc: Controls all processes of an application.
//#
//#  Copyright (C) 2004
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
#include <Common/hexdump.h>			// TODO: remove in final version
#include <ACC/ApplController.h>
#include <ACC/ProcList.h>			// @@

namespace LOFAR {
  namespace ACC {

//
// ApplController(Parameterset*		aPS)
//
ApplController::ApplController(const string&	configID) :
	itsParamSet		(new ParameterSet),
	itsACCmdImpl    (0),
	itsCmdStack     (new CmdStack),
    itsAPAPool      (0),
	itsServerStub	(0),
	itsProcListener (0),
	itsCurTime      (0),
	itsIsRunning    (false),
	itsCurState     (StateNone),
	itsCurACMsg		(0)
{
	LOG_TRACE_OBJ ("ApplController constructor");

	// Read in the parameterfile
	itsParamSet->adoptFile(configID);			// May throw

	// Instanciate a Cmd impl oobject
	itsACCmdImpl = new ACCmdImpl(itsParamSet);

	// Get pointer to singleton APAdminPool
    itsAPAPool = &(APAdminPool::getInstance());

	ProcList		myPL(*itsParamSet);

}


//
// ApplController(Parameterset*		aPS)
//
ApplController::~ApplController()
{
	LOG_TRACE_OBJ ("ApplController destructor");

	if (itsParamSet)     {	delete itsParamSet;     }
	if (itsACCmdImpl)    {	delete itsACCmdImpl;    }
	if (itsCmdStack)     {	delete itsCmdStack;     }
    if (itsAPAPool)      {	delete itsAPAPool;      }
	if (itsServerStub)   {	delete itsServerStub;   }
	if (itsProcListener) {	delete itsProcListener; }
	if (itsCurACMsg)	 {	delete itsCurACMsg; 	}

}

//
// startupNetwork()
//
// Starts the listeners for the user side and the process side.
//
void ApplController::startupNetwork()
{
	LOG_TRACE_FLOW("ApplController:startupNetwork()");

	// Setup listener for ACC user and wait (max 10 sec) for connection
	itsServerStub = new ApplControlServer(itsParamSet->getInt("AC.portnr"), 
																	itsACCmdImpl);

	// Setup listener for application processes
	itsProcListener = new Socket("APlistener", 
								 itsParamSet->getString("AC.process.APportnr"),
								 Socket::TCP, 
								 itsParamSet->getInt("AC.backlog"));
	ASSERTSTR(itsProcListener->ok(), 
						"Can't start listener for application processes");

	itsIsRunning = true;
}


//
// handleProcMessage(APAdmin*)
//
// Handles a message that was received from an application process
//
void ApplController::handleProcMessage(APAdmin*	anAP)
{
	LOG_TRACE_FLOW("ApplController:handleProcMessage()");

	DH_ProcControl*		DHProcPtr = anAP->getDH();
	PCCmd				command   = DHProcPtr->getCommand();

	hexdump (DHProcPtr->getDataPtr(), DHProcPtr->getDataSize());
	if (command & PCCmdResult)
		cout << "Ack on command: " << (command^PCCmdResult);
	else
		cout << "command=" << command << endl;
	cout << "options=" << DHProcPtr->getOptions() << endl;

	switch (command) {
	case PCCmdInfo:
		// TODO: AP ask for some info, we should answer this
		break;
	case PCCmdAnswer:
		// TODO: AP returns an answer on a question we have sent it
		break;
	case PCCmdStart:
		// send by the AP as soon as it is on the air.
		anAP->setName(DHProcPtr->getOptions());	// register name
		itsAPAPool->markAsOnline(anAP);			// mark it ready for reception
		break;
	case PCCmdReport:
		// TODO: AM.report (.....);
		break;
	case PCCmdAsync:
		// TODO: implement this
		break;

	default:
		// The rest of the command should be an Ack on the outstanding command.
		if (command & PCCmdResult) {
			uint16	result  = DHProcPtr->getResult();
			if (result & AcCmdMaskOk) {
				itsAPAPool->registerAck(
								static_cast<PCCmd>(command^PCCmdResult), anAP);
			}
			else {			// result is a NACK!
				sendExecutionResult(AcCmdMaskOk, "Nack");	
			}
		}
		else {
			LOG_WARN(formatString(
					"Unexpected command (%04X) received from process (%s)",
					command, anAP->getName().c_str()));
		}
	} // switch
}

//
// sendExecutionResult(result)
//
// Send the given result to the AC-side and clears the neccesary timers
// and stacks so that further handling of the command is stopped.
//
void ApplController::sendExecutionResult(uint16			result,
										 const string&	comment)
{
	LOG_TRACE_FLOW_STR("ApplController:sendExecutionResult(" << result << "," 
															<< comment << ")");

	itsServerStub->sendResult(result, comment);		// notify user
	itsAPAPool   ->stopAckCollection();				// stop collecting
	itsACCmdImpl ->resetCmdExpireTime();			// stop timer
	itsCurState	 = StateNone;						// reset Cmd state
	itsStateEngine.reset();							// reset state Engine
	if (itsCurACMsg) {								// delete old command
		delete itsCurACMsg;
		itsCurACMsg = 0;
	}
}

//
// startState (newMsg)
//
// Used to start a new state of an AC command. When newMsg is pointing at a
// received message the stateEngine is started for this new message.
// Is newMsg NULL than the state that is already set in itsCurState is started.
//
// NOTE: A state is the handling of one command-reply sequence with the AP's.
//
void ApplController::startCmdState(DH_ApplControl*		newMsg)
{
	if (newMsg) {
		// store a copy of this message for the further states.
		if (itsCurACMsg) {
			delete itsCurACMsg;			// delete previous message
		}
		itsCurACMsg = newMsg->makeDataCopy(*newMsg);
		itsCurState = itsStateEngine.getState();
	}
	else {
		newMsg = itsCurACMsg;			// recall last message
	}

	LOG_TRACE_FLOW_STR ("ApplController:startCmdState(" << newMsg << "," 
										<< itsCurState << ")");
	hexdump (newMsg->getDataPtr(), newMsg->getDataSize());

	// execute the new state
	switch (itsCurState) {
	case StateNone:
		break;
	case StatePowerUpNodes:
		// TODO: Communicate with Node Manager
		itsStateEngine.ready();				// report this state is ready.
		break;
	case StatePowerDownNodes:
		// TODO: Communicate with Node Manager
		itsStateEngine.ready();				// report this state is ready.
		break;
	case StateCreatePSubset:
		// TODO: implement this
		itsStateEngine.ready();				// report this state is ready.
		break;
    case StateStartupAppl:
		// TODO: do the real startup = command(appl.exe paramfile);
		//       for all processes found in the paramfile.
		itsStateEngine.ready();				// report this state is ready.
		break;
	case StateKillAppl:
		// TODO: hard kill the AP's that did not reported themselves ready
		itsStateEngine.ready();				// report this state is ready.
		break;
	case StateInfoCmd:
		itsServerStub->handleMessage(newMsg);
		itsStateEngine.reset();				// no further processing
		break;

	case StateDefineCmd:
	case StateInitCmd:
    case StateRunCmd:
	case StatePauseCmd:
	case StateRecoverCmd:
	case StateSnapshotCmd:
	case StateReinitCmd:
	case StateQuitCmd:
		// All these command must be sent to the AP's and the responses
		// that com back will finally result in a Nack of the next state.
		itsServerStub->handleMessage(newMsg);
		break;
	}
}

//
// doEventLoop()
//
// (Almost) never ending loop that executes the Application Controller functions.
//
void ApplController::doEventLoop()
{
	while (itsIsRunning) {
		checkForACCommands();
		checkForAPMessages();	
		checkForConnectingAPs();
		checkForDisconnectingAPs();	
		checkAckCompletion();
		checkCmdTimer();
		checkCmdStack();
		checkStateEngine();

		cout << *itsAPAPool; 

		sleep (5);
	}
}

//
// checkForACCommands()
//
// See if the AC has send us a new command the AP's should execute.
// A received command may be scheduled or may start a new state-sequence.
//
void ApplController::checkForACCommands() 
{
	// NOTE: The AC should guard the connection with the AM. When the
	// AM disconnects a reconnect-timer should be started allowed the
	// AM some time to reconnect before shutting down everything.
	// Since the Socket of the AM is hidden behind TH_stuff we don't
	// know anything anymore about its connection state. So it is not
	// possible to implement this feature when using TH technology.
	//
	// The pollForMessage call does a read on the DH which will always
	// check its connection first, and tries to connect if there is no
	// connection (yet/anymore). This will ensure that we at least will
	// pickup a (re)connect from the AM.

	LOG_DEBUG("Polling user side");
	if (itsServerStub->pollForMessage()) {			// new command received?
		DH_ApplControl* newMsg   = itsServerStub->getDataHolder();
		ACCmd			newCmd   = newMsg->getCommand();
		time_t          execTime = newMsg->getScheduleTime();
		itsCurTime = time(0);
		if (execTime <= itsCurTime) {			// execute immediately?
			LOG_TRACE_FLOW("Immediate command");
			// still commands in progress?
			if (itsCurState != StateNone) {
				// some command is running, has new command overrule 'rights'?
				if ((newCmd != ACCmdQuit) && (newCmd != ACCmdReplace)){
					// No overrule rights, reject new command
					sendExecutionResult (0, "Previous command still running");
					return;
				}
			}
			// A command may exist of several states, start with first.
			itsStateEngine.startSequence(newCmd);
			startCmdState(newMsg);
			// start expire timer for this command
			// TODO:How to get the right expire time
//			itsCmdExpireTime = itsCurTime + 20;
		}
		else {									// future command
			LOG_TRACE_FLOW("Scheduling command");
			// construct a generic command structure
			ACCommand	ACcmd(newMsg->getCommand(),
							  newMsg->getScheduleTime(),
							  newMsg->getWaitTime(),
							  newMsg->getOptions(),
							  newMsg->getProcList(),
							  newMsg->getNodeList());
			// schedule it.
			itsCmdStack->add(newMsg->getScheduleTime(), &ACcmd);
			// Tell user it is scheduled.
			itsServerStub->sendResult(AcCmdMaskOk | AcCmdMaskScheduled);
		}
	}
}


//
// CheckForAPMessages()
//
// The AP's may snet Ack or other types of messages to us. Reroute them to the
// right functions.
//
void ApplController::checkForAPMessages() 
{
	// Anything received from the application processes?
	LOG_DEBUG("Polling process side");
	APAdmin*		activeAP;
	if ((activeAP = itsAPAPool->poll(1000))) {
		handleProcMessage(activeAP);	// handle it
	}
}


//
// checkForConnectingAPs()
// 
// Check the AP listener socket for new AP's that want to connect. New AP's are
// added to the APAdminPool but they will not receive commands until they have
// sent their StartCmd to acknowledge existance.
//
void ApplController::checkForConnectingAPs() 
{
	// Any new incomming connections from the appl. processes?
	LOG_DEBUG("New processes to connect?");
	Socket*		newAPSocket;
	while ((newAPSocket = itsProcListener->accept(100))) {
		LOG_DEBUG("Incomming process connection");
		APAdmin*	APAdminPtr = new APAdmin(newAPSocket);
		itsAPAPool->add(APAdminPtr);
	}
}

//
// checkForDisconnectingAPs
//
// During a read the AP may have ended the connection. Clean up its mess.
//
void ApplController::checkForDisconnectingAPs() 
{
	// Check for disconnected client processes. During the read action
	// it may have turned out that a process has dropped the connection
	// this is registered in the Socket. Cleanup these APadmins.
	LOG_DEBUG("Cleaning process side");
	while(APAdmin*	anAPA = itsAPAPool->cleanup()) {
		// TODO: AM.report(anAPA->getName() << " has disconnected");
		LOG_DEBUG_STR (anAPA->getName() << " has disconnected");
		delete anAPA;		// finally delete it.
	}
}


//
// checkAckCompletion
//
// When the last ack on the outstanding command was received we should mark
// this state ready. The stateEngine will decide somewhere else if there is
// another state to execute or to sent an Ack to the AC user.
void ApplController::checkAckCompletion() 
{
	LOG_DEBUG("All ack's received?");

	if (itsAPAPool->allAcksReceived()) {
		itsStateEngine.ready();		// report we are ready with this state.
	}
}


//
// checkCmdtimer()
//
// Every command has a certain lifetime. When its lifetime is expired the
// ACuser should be informed on this with a Nack.
//
void ApplController::checkCmdTimer() 
{
	LOG_DEBUG("Command timer still running?");

	if (itsACCmdImpl->IsCmdExpired()) {
		sendExecutionResult(0, "timed out");
	}
}


//
// checkCmdStack()
//
// Check if it is time to execute the command that is placed on the CmdStack.
//
void ApplController::checkCmdStack() 
{
	// DH's are handled, time for new command?
	// TODO: may we start a new one while the last one is still running?
	LOG_DEBUG("Time for a stack command?");

	if (itsCmdStack->timeExpired()) {
		ACCommand 	ACCmd = itsCmdStack->pop();
		itsServerStub->handleMessage(&ACCmd);
	}
}

//
// checkStateEngine()
//
// Somewhere in the process the stateEngine may have been told the current
// state is finished. If so, another state should be started or when the
// last state of the sequence was finished the AC user must have an ACK msg.
//
void ApplController::checkStateEngine()
{
	LOG_DEBUG("Time for next commmand phase?");
	if (!itsStateEngine.isNextStateWaiting()) {	
		return;
	}

	// State was flagged ready, check if there is another state we should 
	// execute.
	itsCurState = itsStateEngine.nextState();
	if (itsCurState == StateNone) {
		sendExecutionResult(AcCmdMaskOk, "Command ready");		// No more states
		return;
	}

	// there is a next state, start it.
	startCmdState(0);					// no new command
}


  } // namespace ACC
} // namespace LOFAR

