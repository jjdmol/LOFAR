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
#include <ACC/PR_Shell.h>			// TODO: factory!
#include <ACC/ItemList.h>			// @@

namespace LOFAR {
  namespace ACC {

//
// ApplController(Parameterset*		aPS)
//
ApplController::ApplController(const string&	configID) :
	itsBootParamSet	 (new ParameterSet),
	itsObsParamSet	 (new ParameterSet),
	itsResultParamSet(new ParameterSet),
	itsProcList		 (0),
	itsNodeList      (0),
	itsACCmdImpl     (new ACCmdImpl),
	itsCmdStack      (new CmdStack),
	itsProcListener  (0),
    itsAPAPool       (0),
	itsServerStub	 (0),
	itsDaemonComm	 (0),
	itsCurTime       (0),
	itsIsRunning     (false),
	itsStateEngine   (new StateEngine),
	itsCurState      (StateNone),
	itsCurACMsg		 (0)
{
	LOG_TRACE_OBJ ("ApplController constructor");

	// Read in the parameterfile with network parameters
	itsBootParamSet->adoptFile(configID);			// May throw

	// Get pointer to singleton APAdminPool
    itsAPAPool = &(APAdminPool::getInstance());

}


//
// ApplController(Parameterset*		aPS)
//
ApplController::~ApplController()
{
	LOG_TRACE_OBJ ("ApplController destructor");

	// Save results from the processes to a file (append).
	if (itsResultParamSet && itsObsParamSet && 
							 itsObsParamSet->isDefined("AC.resultfile")) {
	   itsResultParamSet->writeFile(itsObsParamSet->getString("AC.resultfile"),
								    true);
	}

	if (itsBootParamSet)   { delete itsBootParamSet;   }
	if (itsObsParamSet)    { delete itsObsParamSet;    }
	if (itsResultParamSet) { delete itsResultParamSet; }
	if (itsProcList)       { delete itsProcList;       }
	if (itsNodeList)       { delete itsNodeList;       }
	if (itsACCmdImpl)      { delete itsACCmdImpl;      }
	if (itsCmdStack)       { delete itsCmdStack;       }
	if (itsProcListener)   { delete itsProcListener;   }
    if (itsAPAPool)        { delete itsAPAPool;        }
	if (itsServerStub)     { delete itsServerStub;     }
	if (itsDaemonComm)     { delete itsDaemonComm;     }
	if (itsCurACMsg)	   { delete itsCurACMsg; 	   }
	if (itsStateEngine)	   { delete itsStateEngine;    }

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
	itsServerStub = new ApplControlServer(
									itsBootParamSet->getInt("AC.userportnr"), 
									itsACCmdImpl);

	// Setup listener for application processes
	itsProcListener = new Socket("APlistener", 
							 itsBootParamSet->getString("AC.processportnr"),
							 Socket::TCP, 
							 itsBootParamSet->getInt("AC.backlog"));
	ASSERTSTR(itsProcListener->ok(), 
						"Can't start listener for application processes");

	// Setup communication channel with ACDaemon
	itsDaemonComm = new ACDaemonComm(
							itsBootParamSet->getString("AC.pinghost"),
							itsBootParamSet->getString("AC.pingportnr"),
							itsBootParamSet->getString("AC.pingID"));
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
	bool				ack       = false;

	if (command & PCCmdResult) {
		command = static_cast<PCCmd>(command ^ PCCmdResult);
		ack = true;
		cout << "Ack on command: " << command;
	}
	else {
		cout << "command=" << command << endl;
	}
	cout << ", options=" << DHProcPtr->getOptions() << endl;

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
	case PCCmdParams: {
		ParameterSet	resultParam;
		resultParam.adoptBuffer(DHProcPtr->getOptions());
		resultParam.writeFile(itsObsParamSet->getString("AC.resultfile"), true);
		break;
		}
	case PCCmdQuit:
		LOG_TRACE_OBJ("PCCmdQuit received");
		itsAPAPool->markAsOffline(anAP);		// don't send new commands
		itsAPAPool->registerAck(command, anAP);
		itsResultParamSet->adoptBuffer(DHProcPtr->getOptions());
		break;

	default:
		// The rest of the command should be an Ack on the outstanding command.
		if (ack) {
			uint16	result  = DHProcPtr->getResult();
			if (result & AcCmdMaskOk) {
				itsAPAPool->registerAck(
								static_cast<PCCmd>(command), anAP);
			}
			else {			// result is a NACK!
				sendExecutionResult(0, "Nack from process");	
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
	LOG_DEBUG_STR("ApplController:sendExecutionResult(" << result << "," 
															<< comment << ")");

	itsServerStub ->sendResult(result, comment);	// notify user
	itsAPAPool    ->stopAckCollection();			// stop collecting
	itsCurState	  = StateNone;						// reset Cmd state
	itsStateEngine->reset();						// reset state Engine
	if (itsCurACMsg) {								// delete old command
		delete itsCurACMsg;
		itsCurACMsg = 0;
	}
}

//
// createParSubsets()
//
// Creates for each process of its ProcList a parameter file containing its
// own parameters. All the parameters from one process are in the masterPS
// at <applname>.<procname_from_proclist>.xxx
//
//
void ApplController::createParSubsets()
{
	ItemList::iterator	iter;
	string				applName = itsObsParamSet->getString("AC.application");
	string				prevBaseProc;
	int32				procNr;
	ParameterSet		basePS;

	// substract processlist from Appl. parameters
	if (itsProcList) {
		delete itsProcList;
	}
	itsProcList = new ItemList(*itsObsParamSet, "AC.process");

	// make a subset for all process in the process list.
	for (iter = itsProcList->begin(), procNr = 1; iter != itsProcList->end(); 
															++iter, ++procNr) {
		// First construct some names
		// baseProcName : processname			== name executable!
		// procName     : processname<nr>
		// fileName     : processname".ps"
		string baseProcName = *iter;			// = procname[n]
		int32  index = indexValue(baseProcName,"[]");
		rtrim(baseProcName, "0123456789[]");	// strip off indexnr.
		string procName = formatString("%s%d", baseProcName.c_str(), 
															index ? index : 1);
		string fileName = procName+".ps";
		LOG_DEBUG_STR("Creating parameterfile for process " << procName);

		// Step 1: A parameterset for a process is constructed from three set:
		// [A] the params for procName[0]
		// [B] the params for procName, these overule the previous set
		// [C] additional info from the AC itself

		// [A] this set can be the same for many processes. Only if procName
		//     differs from previous make a new paramset
		if (baseProcName.compare(prevBaseProc)) {
			basePS = itsObsParamSet->makeSubset(
										applName+"."+baseProcName+"[0].", 
										procName+".");
		}

		// [B] construct parameter subset with process specific settings
		string		procNameIndexed(baseProcName);
		if (index) {
			procNameIndexed = formatString("%s[%d]", 
											baseProcName.c_str(), index);
		}
		// merge the two subsets, take baseSet as a basis (sounds logical)
		ParameterSet	procPS(basePS);
		// overwrite/extend with proc specific settings
		procPS.adoptCollection(itsObsParamSet->makeSubset(
											applName+"."+procNameIndexed+".", 
											procName+"."));

		// [C] Add AC parameters of any interest to process
		// TODO add some more like hostname and others?
		procPS.add("process.name", procName);
		procPS.add(procName+".ACport", 
						 itsBootParamSet->getString("AC.processportnr"));

		// Step 2: The start/stop information (ruler info) is not for the
		// process but for the AC. Remove it from the paramset and store it in
		// a local ProcRule array.
		// I. Read the ruler info from the process paramset.
		string	nodeName = procPS.getString(procName+".node");

		// II. Save proc ruling info in separate map for later
		// TODO: make factory!!!!
		//string procType = procPS.getString(procName+".startstoptype");
		//itsProcRuler.add(makePR(procType, nodeNm, procNm, execNm, fileNm));
		string execName = procPS.getString(procName+".executable");
		itsProcRuler.add(PR_Shell(nodeName, procName, execName, fileName));

		// Remove execute type from processes paramlist
		procPS.remove(procName+".startstoptype");
		procPS.remove(procName+".executable");

		// Finally write process paramset to a file.
		procPS.writeFile(fileName);
	}
}
//
// startState (newMsg)
//
// Used to start a new state of an AC command. When newMsg is pointing at a
// received message the stateEngine is started for this new message.
// Is newMsg NULL we are in the middle of an state sequence, the new state
// is than already set in itsCurState.
//
// NOTE: A state is the handling of one command-reply sequence with the AP's.
//
void ApplController::startCmdState()
{
	LOG_TRACE_FLOW_STR ("ApplController:startCmdState(" << itsCurState << ")");

	// execute the new state
	switch (itsCurState) {
	case StateNone:
		break;
	case StateInitController:
		// read in the Application Parameter file
		itsObsParamSet->adoptFile(itsCurACMsg->getOptions());// May throw
		// StateEngine needs to know the timeout values for the states
		itsStateEngine->init(itsObsParamSet);
		itsStateEngine->ready();				// report this state is ready.
	case StateCreatePSubset:
		createParSubsets();
		itsStateEngine->ready();				// report this state is ready.
		break;
    case StateStartupAppl:
		if (!itsProcRuler.startAll()) {
			sendExecutionResult(0, "Startup failures");
			itsStateEngine->reset();			// no further processing
	 	}
		// the incomming acks decide the result of the start action
		break;
	case StateKillAppl:
		sleep (5);								// give procs some extra time
		itsProcRuler.stopAll();
		itsStateEngine->ready();				// report this state is ready.
		break;
	case StateInfoCmd:
		itsServerStub->handleMessage(itsCurACMsg);
		itsStateEngine->reset();				// no further processing
		break;

	case StateDefineCmd:
	case StateInitCmd:
    case StateRunCmd:
	case StatePauseCmd:
	case StateRecoverCmd:
	case StateSnapshotCmd:
	case StateReinitCmd:
	case StateQuitCmd:
		// if nothing is online we are ready
		if (itsAPAPool->onlineCount() == 0) {
			itsStateEngine->ready();
			break;
		}

		if (itsCurState == StatePauseCmd) {
			// overrule default wait time
			itsStateEngine->setStateLifeTime(itsCurACMsg->getWaitTime());
		}

		// All these command must be sent to the AP's and the responses
		// that com back will finally result in a Nack of the next state.
		itsServerStub->handleMessage(itsCurACMsg);
		break;
	case NR_OF_STATES:		// satisfy compiler
		break;
	}
}

//
// acceptOrRefuseACMsg(command, passOwnership)
//
// Decides whether or not the given command may be executed in this stage.
// When execution is not allowed, e.g. because the previous command is still
// running than a Nack is send to the ACuser. Otherwise the command is started.
//
void ApplController::acceptOrRefuseACMsg(DH_ApplControl*	anACMsg,
										 bool				passOwnership) 
{
	// what command should we execute?
	ACCmd newCmd = anACMsg->getCommand();

	// still commands in progress?
	if (itsCurState != StateNone) {
		// some command is running, has new command overrule 'rights'?
		if ((newCmd != ACCmdQuit) && (newCmd != ACCmdReplace)){
			// No overrule rights, reject new command
			sendExecutionResult (0, "Previous command is still running");
			return;
		}
	}

	// store a copy of this message for the further states.
	if (itsCurACMsg) {
		delete itsCurACMsg;			// delete previous message
	}
	if (passOwnership) {
		itsCurACMsg = anACMsg;
	}
	else {
		itsCurACMsg = anACMsg->makeDataCopy();
	}

	// Initialize the stateEngine and store our new state.
	itsStateEngine->startSequence(newCmd);
	itsCurState = itsStateEngine->getState();

	// start appropriate action
	startCmdState();
}

//
// doEventLoop()
//
// (Almost) never ending loop that executes the Application Controller functions.
//
void ApplController::doEventLoop()
{
	// Loop optimalisation when not waiting for ACK's
	const uint16	loopDiff = 5;			// poll AP 5 times less than AM
	uint16			loopCounter = loopDiff;

	// prepare ping information for ACDaemon
	time_t		nextPing     = 0;
	int32		pingInterval = itsBootParamSet->getTime("AC.pinginterval");

	while (itsIsRunning) {
		checkForACCommands();
		// AP's are less important when no command is running.
		if ((itsCurState != StateNone) || (loopCounter == 0)) {
			checkForAPMessages();	
			checkForConnectingAPs();
			checkForDisconnectingAPs();	
			checkAckCompletion();
		}
		checkStateTimer();
		checkCmdStack();
		checkStateEngine();

		// Should daemon be tickled?
		if (nextPing < time(0)) {
			if (!itsDaemonComm->sendPing()) {
				LOG_DEBUG("Ping message to ACD could not be written!");
			}
			nextPing = time(0) + pingInterval;
		}

		if (loopCounter == 0) {
			loopCounter = loopDiff;
		}
		else {
			--loopCounter;
		}

		cout << *itsAPAPool; 		// temp debug info
		cout << *itsStateEngine;
		cout << "Ping at: " << timeString(nextPing) << endl;

		// Only sleep when idle
		if (itsCurState == StateNone) {
			sleep (1);
		}
	}

	itsDaemonComm->unregister();
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
		time_t          execTime = newMsg->getScheduleTime();
		itsCurTime = time(0);
		if (execTime <= itsCurTime) {			// execute immediately?
			LOG_TRACE_FLOW("Immediate command");
			acceptOrRefuseACMsg(newMsg, false);
		}
		else {									// future command
			LOG_TRACE_FLOW("Scheduling command");
			// schedule it.
			itsCmdStack->add(newMsg->getScheduleTime(), newMsg);
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
	while ((activeAP = itsAPAPool->poll(1000))) {
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
		itsProcRuler.markAsStopped(anAPA->getName());
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

	if (itsCurState == StateStartupAppl) {
		if (itsAPAPool->onlineCount() == itsProcRuler.size()) {
			itsStateEngine->ready();
		}
	}

	if (itsAPAPool->allAcksReceived()) {
		itsStateEngine->ready();		// report we are ready with this state.
	}
}


//
// checkCmdtimer()
//
// Every command has a certain lifetime. When its lifetime is expired the
// ACuser should be informed on this with a Nack.
//
void ApplController::checkStateTimer() 
{
	LOG_DEBUG("State timer still running?");

	if (itsStateEngine->IsStateExpired()) {
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
	LOG_DEBUG("Time for a stack command?");

	if (itsCmdStack->timeExpired()) {
		acceptOrRefuseACMsg(itsCmdStack->pop(), true);
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
	if (!itsStateEngine->isStateFinished()) {	
		return;
	}

	// State was flagged ready, check if there is another state we should 
	// execute.
	// TODO: implement quit in a neat way!
	if (itsCurState == StateKillAppl) {
		itsIsRunning = false;
	}
	itsCurState = itsStateEngine->nextState();
	if (itsCurState == StateNone) {
		sendExecutionResult(AcCmdMaskOk, "Command ready");		// No more states
		return;
	}

	// there is a next state, start it.
	startCmdState();
}


  } // namespace ACC
} // namespace LOFAR

