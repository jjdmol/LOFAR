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
#include <Common/LofarLocators.h>
#include <Common/StringUtil.h>
#include <ALC/ACCmd.h>
#include <PLC/ProcControlComm.h>
#include <MACIO/MACServiceInfo.h>
#include <MACIO/KVT_Protocol.ph>
#include "ApplController.h"
#include "PR_Shell.h"			// TODO: factory!
#include "PR_MPI.h"				// TODO: factory!
#include "PR_BGL.h"				// TODO: factory!
#include "ItemList.h"			// @@
#include "lofarDirs.h"

namespace LOFAR {
  using namespace MACIO;
  namespace ACC {

//
// ApplController(Parameterset*		aPS)
//
ApplController::ApplController(const string&	configID) :
	itsBootParamSet	 (new ParameterSet),
	itsObsParamSet	 (new ParameterSet),
	itsResultParamSet(new ParameterSet),
	itsProcList		 (0),
	itsACCmdImpl     (new ACCmdImpl),
	itsCmdStack      (new CmdStack),
	itsProcListener  (0),
    itsAPAPool       (0),
	itsServerStub	 (0),
	itsDaemonComm	 (0),
#ifdef KVLOGGER	
	itsKVLogger		 (0),
#endif	
	itsCurTime       (0),
	itsIsRunning     (false),
	itsStateEngine   (new StateEngine),
	itsCurState      (StateNone),
	itsCurACMsg		 (0)
{
	LOG_TRACE_OBJ ("ApplController constructor");

	// Read in the parameterfile with network parameters
	ConfigLocator		CL;
	string	bootPSname = CL.locate (configID+".param");
	LOG_INFO_STR("Booting from " << bootPSname);
	itsBootParamSet->adoptFile(bootPSname);			// May throw

	// Get pointer to singleton APAdminPool
    itsAPAPool = &(APAdminPool::getInstance());

}


//
// ApplController(Parameterset*		aPS)
//
ApplController::~ApplController()
{
	LOG_TRACE_OBJ ("ApplController destructor");

	if (itsBootParamSet)   { delete itsBootParamSet;   }
	if (itsObsParamSet)    { delete itsObsParamSet;    }
	if (itsResultParamSet) { delete itsResultParamSet; }
	if (itsProcList)       { delete itsProcList;       }
	if (itsACCmdImpl)      { delete itsACCmdImpl;      }
	if (itsCmdStack)       { delete itsCmdStack;       }
	if (itsProcListener)   { delete itsProcListener;   }
    if (itsAPAPool)        { delete itsAPAPool;        }
	if (itsServerStub)     { delete itsServerStub;     }
	if (itsDaemonComm)     { delete itsDaemonComm;     }
#ifdef KVLOGGER	
	if (itsKVLogger)	   { delete itsKVLogger;	   }
#endif	
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
									itsBootParamSet->getInt32("AC.userportnr"), 
									itsACCmdImpl);

	// Setup listener for application processes
	itsProcListener = new Socket("APlistener", 
							 itsBootParamSet->getString("AC.processportnr"),
							 Socket::TCP, 
							 itsBootParamSet->getInt32("AC.backlog"));
	ASSERTSTR(itsProcListener->ok(), 
						"Can't start listener for application processes");

	// Setup communication channel with ACDaemon
	itsDaemonComm = new ACDaemonComm(
							itsBootParamSet->getString("AC.pinghost"),
							itsBootParamSet->getString("AC.pingportnr"),
							itsBootParamSet->getString("AC.pingID"));

	// 												   client					synchrone
#ifdef KVLOGGER
	itsKVLogger = new EventPort(MAC_SVCMASK_KVTLOGGER, false, KVT_PROTOCOL, "", true);	
	ASSERTSTR(itsKVLogger, "Can't connect to KeyValueLogger");
#endif
	itsIsRunning = true;
}


//
// handleProcMessage(APAdmin*)
//
// Handles a message that was received from an application process
//
void ApplController::handleProcMessage(APAdmin*	anAP)
{
	LOG_TRACE_FLOW_STR("ApplController:handleProcMessage(" << anAP->getName() << ")");

	DH_ProcControl*		DHProcPtr = anAP->getDH();
	PCCmd				command   = DHProcPtr->getCommand();
	bool				ack       = false;

	if (command & PCCmdResult) {
		command = static_cast<PCCmd>(command ^ PCCmdResult);
		ack = true;
		LOG_TRACE_VAR_STR("Ack on command: " << command);
	}
	else {
		LOG_TRACE_VAR_STR("command=" << command);
	}
	LOG_TRACE_VAR_STR("  options=" << DHProcPtr->getOptions());

	switch (command) {
	case PCCmdInfo:
		// TODO: AP asks for some info, we should answer this
		break;

	case PCCmdAnswer:
		// TODO: AP returns an answer on a question we have sent it
		break;

	case PCCmdBoot:
		// send by the AP as soon as it is on the air (registerAtAC).
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
		// TODO: Write this information to the >>>> KeyValueLogger <<<<
		ParameterSet	resultParam;
		resultParam.adoptBuffer(DHProcPtr->getOptions());
		resultParam.writeFile(itsObsParamSet->getString("ApplCtrl.resultfile"), true);
		sendToKVLogger(resultParam);
		break;
		}

	case PCCmdQuit:	// send by UnregisterAtAC
		LOG_TRACE_OBJ("PCCmdQuit received");
		itsAPAPool->markAsOffline(anAP);		// don't send new commands
		itsAPAPool->registerAck(command, anAP);
		itsResultParamSet->adoptBuffer(DHProcPtr->getOptions());
		break;

	default:
		// The rest of the command should be an Ack on the outstanding command.
		if (ack) {
			// always register ack.
			bool	ackOnTime = itsAPAPool->registerAck(command, anAP);

			// note: the process can return three values: Ok/NotSupported/Error
			// Handle NotSupported as Ok.
			uint16	result  = DHProcPtr->getResult();
			bool	successful = ((result & (PcCmdMaskOk | PcCmdMaskNotSupported)) != 0);

			if (ackOnTime && !successful) { 
				sendExecutionResult(0, "Nack from process:" + anAP->getName());
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

	// notify user
	itsServerStub ->sendResult(itsCurACMsg->getCommand(), result, comment);	
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
// at <procname_from_proclist>.xxx
//
//
void ApplController::createParSubsets()
{
    // Step 1: A parameterset for a process is constructed from three set:
    // [A] the default params for the set of proces procName[0]
    // [B] for multiple cmdline processes of the same type, the params for 
	//	   a specific process (procName[i]), these overule the previous set
    // [C] additional info from the AC itself

	// ApplCtrl.application = applName
	string			applName = itsObsParamSet->getString("ApplCtrl.application");

	// ApplCtrl.processes = [ procName ... ]
	// procList contains a list of processes to control. Loop over this list
	// and make a ProcRuler and ParsetFile for each process of this application.
	string			prevProcName;
	ParameterSet	basePS;
	vector<string>	nodes;
	vector<string>	procList = itsObsParamSet->getStringVector("ApplCtrl.processes");
	LOG_TRACE_VAR_STR("Found " << procList.size() << " processes");
	for (uint procIdx = 0; procIdx < procList.size(); procIdx++) {
		LOG_TRACE_VAR_STR("Processing process:" << procList[procIdx]);

		// procList[x] = processName(x) | processName
		// procName  := processName
		// nrProcs  := x | 0
		string procName = procList[procIdx];
		int32  nrProcs  = indexValue(procName, "()");
		rtrim(procName, "()0123456789");
		string procPrefix = applName +"." + procName;

		if (nrProcs == 0) {
			itsNrOfProcs++;
		}
		else {
			itsNrOfProcs += nrProcs;
		}

		// The startstopType determines what information is put in the parsetfiles
		// for the processes.
		string startstopType = itsObsParamSet->getString(procPrefix+"._startstopType");
		string fileName 	 = string(LOFAR_SHARE_LOCATION) + "/" + procName + ".parset";

		LOG_DEBUG_STR("Creating parameterfile for process " << procName);

		// [A] Get the default parameters ( procName[0].* ) when procname changes
		if (procName != prevProcName) {
			LOG_TRACE_COND_STR("Making basePS for " << procName);
			basePS.clear();
			basePS.adoptCollection(*itsObsParamSet);
//			basePS = itsObsParamSet->makeSubset(procPrefix, procPrefix);
//			LOG_TRACE_VAR_STR(basePS);

			// [C] additional info from the AC itself
			basePS.replace(procPrefix+"._ACport", 
											itsBootParamSet->getString("AC.processportnr"));
			basePS.replace(procPrefix+"._ACnode", itsBootParamSet->getString("AC.node"));
			basePS.replace("_parsetPrefix", procPrefix+".");
			prevProcName = procName;
		}

		// --- cmdline ---
		if (startstopType == "cmdline") {
			if (nrProcs == 0) {
				LOG_TRACE_COND_STR("Single cmdline process " << procName);
				// This processSet is a single commandline process
				// add procName.* params to parset for process.
				ParameterSet myPS = itsObsParamSet->makeSubset(procPrefix+".",
																	procPrefix+".");
				LOG_TRACE_VAR_STR(myPS);
				myPS.adoptCollection(basePS);
				writeParSubset(myPS, procName, fileName);
				
				// construct ProcesRuler
				itsProcRuler.add(PR_Shell(myPS.getString(procPrefix + "._hostname"),
										  procName,
										  myPS.getString(procPrefix + "._executable"),
										  fileName));

			} else {
				// There are multiple processes of this type
				nodes = basePS.getStringVector(procPrefix+"._nodes");
				LOG_TRACE_COND_STR("Multiple(" << nrProcs << ") cmdline process " << procName);
				for (int32 p = 1; p <= nrProcs; ++p) {
					// [B] construct parameter subset with process specific settings
					string pName      = formatString("%s%d", procName.c_str(), p);
//					string oldPPrefix = formatString("%s[%d].", procPrefix.c_str(), p);
//					ParameterSet myPS(basePS);
//					myPS.adoptCollection(itsObsParamSet->makeSubset(oldPPrefix, 
//																	   procPrefix+"."));
					ParameterSet myPS = itsObsParamSet->makeSubset(procPrefix+".",
																		procPrefix+".");
					LOG_TRACE_VAR_STR(myPS);
					myPS.adoptCollection(basePS);

					// copy the default PS and give it a new prefix
//					myPS.adoptCollection(itsObsParamSet->makeSubset(procPrefix+".",
//																	procPrefix+"."));
					fileName  = string(LOFAR_SHARE_LOCATION) + "/" + pName + ".parset";
					writeParSubset(myPS, pName, fileName);

					// note: nodes[] may be smaller than nrProcs. by taking the remainder
					// of nodes.size() the nodes[] is made cyclic.
					itsProcRuler.add(PR_Shell(nodes[(p-1)%nodes.size()],
											  pName,
											  myPS.getString(procPrefix + "._executable"),
											  fileName));
				}
			}
			
			// IONProc processes do not connect to the ApplController.
			itsNrOfProcs -= nrProcs ? nrProcs : 1;
		}

		else if (startstopType == "mpirun") {
			// This processSet is an MPI program
			LOG_TRACE_COND_STR("mpi process " << procName);
			// fill 'nodes' with all nodenames of variable _nodes.
			nodes = basePS.getStringVector(procPrefix+"._nodes", true);// true:expand

			itsProcRuler.add(PR_MPI(basePS.getString(procPrefix + "._hostname"),
									procName,
									nodes,
									basePS.getString(procPrefix + "._executable"),
									fileName,
									nrProcs));
			writeParSubset(basePS, procName, fileName);
			// Storage processes do not connect to the ApplController.
			itsNrOfProcs -= nrProcs ? nrProcs : 1;
		}

		// --- cn ---
		else if (startstopType == "bgl") {
			// This processSet is a BG/L job
			LOG_TRACE_COND_STR("bgl process " << procName);
			itsProcRuler.add(PR_BGL(procName,				    
									basePS.getString("OLAP.CNProc.partition"),
									basePS.getString(procPrefix + "._executable"),
									basePS.getString(procPrefix + ".workingdir"),
									fileName, 
									nrProcs));
			writeParSubset(basePS, procName, fileName);
			// CN processes do not connect to the ApplController.
			itsNrOfProcs -= nrProcs ? nrProcs : 1;
		}
	} // for processes
}

//
// writeParSubset(ps, procName, fileName)
//
void ApplController::writeParSubset(ParameterSet ps, const string& procName, const string& fileName){
    // [C] Add AC parameters of any interest to process
    // TODO add some more like hostname and others?

    // append the new prefix
    ps.replace("_processName", procName);

    // Remove execute type from processes paramlist
    ps.remove(procName+"._startstopType");
    ps.remove(procName+"._executable");
    
    // Finally write process paramset to a file.
    ps.writeFile(fileName);

	LOG_DEBUG_STR("Create parameterfile " << fileName);
}

//
// writeResultFile
//
// Write the collected (end) results of the processes to the resultfile.
//
void ApplController::writeResultFile()
{
	// Save results from the processes to a file (append).
	if (itsResultParamSet && itsObsParamSet && 
							 itsObsParamSet->isDefined("ApplCtrl.resultfile")) {
	   itsResultParamSet->writeFile(
							itsObsParamSet->getString("ApplCtrl.resultfile"),
							true);
	}
}

//
// sendToKVLogger(parSet)
//
// Send the KV pairs to the KeyValueLogger
//
void ApplController::sendToKVLogger(ParameterSet&	aResultPS)
{
	// loop over PS and construct a msgpool event.
	ParameterSet::iterator	iter = aResultPS.begin();
	ParameterSet::iterator	end  = aResultPS.end();
	KVTSendMsgPoolEvent		poolEvent;
	poolEvent.seqnr = 1;
	poolEvent.msgCount = 0;
	while (iter != end) {
		poolEvent.keys.theVector.push_back(iter->first);
		poolEvent.values.theVector.push_back(iter->second);
		poolEvent.msgCount++;
		iter++;
	}

	// empty PS?
	if (!poolEvent.msgCount) {
		return;
	}

	// send message and wait for answer.
#ifdef KVLOGGER
	itsKVLogger->send(&poolEvent);
	KVTSendMsgPoolAckEvent		poolAck(*(itsKVLogger->receive()));

	if (poolAck.result != 0) {
		LOG_ERROR_STR("Storing metadata in PVSS resulted in errorcode " << poolAck.result);
	}
#endif	
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
		// CS1_HACK save name of parameterfile
		itsObsPSfilename = itsCurACMsg->getOptions();
		LOG_DEBUG_STR("Observation parsetfilename=" << itsObsPSfilename);
		// StateEngine needs to know the timeout values for the states
		itsStateEngine->init(itsObsParamSet);
		itsStateEngine->ready();				// report this state is ready.
		break;
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
	case StateReleaseCmd:
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
			// overrule default wait time if set
			if (itsCurACMsg->getWaitTime() > 0) {
				itsStateEngine->setStateLifeTime(itsCurACMsg->getWaitTime());
			}
		}

		// All these command must be sent to the AP's and the responses
		// that come back will finally result in a Nack of the next state.
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

	// Special case: flush command queue?
	if (newCmd == ACCmdCancelQueue) {
		itsCmdStack->clear();
		// send result without doing anything else.
		itsServerStub->sendResult(newCmd, AcCmdMaskOk, "Queue is flushed");	
		return;
	}

	// still commands in progress?
	if (itsCurState != StateNone) {
		// some command is running, has new command overrule 'rights'?
		if ((newCmd != ACCmdQuit) && (newCmd != ACCmdPause)){
			// No overrule rights, reject new command
			LOG_DEBUG_STR("Command rejected: Previous command is still running. itsCurState:" << itsCurState << ", newCmd:" << ACCmdName(newCmd));
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
	const uint16	loopDiff = 1;			// poll AP 5 times less than AM
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

 		// temp debug info
		LOG_TRACE_FLOW_STR(*itsAPAPool);
		LOG_TRACE_FLOW_STR(*itsStateEngine);
		LOG_TRACE_FLOW_STR("Ping at: " << timeString(nextPing));

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

	LOG_TRACE_STAT("Polling user side");
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
			itsServerStub->sendResult(newMsg->getCommand(), 
									  AcCmdMaskOk | AcCmdMaskScheduled,
									  "Command is scheduled");
		}
	}
}


//
// CheckForAPMessages()
//
// The AP's may sent Ack or other types of messages to us. Reroute them to the
// right functions.
//
void ApplController::checkForAPMessages() 
{
	// Anything received from the application processes?
	LOG_TRACE_STAT("Polling process side");
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
	LOG_TRACE_STAT("New processes to connect?");
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
	LOG_TRACE_STAT("Cleaning process side");
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
	LOG_TRACE_STAT("All ack's received?");

	if (itsCurState == StateStartupAppl) {
//		if (itsAPAPool->onlineCount() == itsProcRuler.size()) {
//		if (itsAPAPool->onlineCount() == itsAPAPool->processCount()) {
		if (itsAPAPool->onlineCount() == itsNrOfProcs) {
			itsStateEngine->ready();
		}
		else {
			LOG_TRACE_STAT_STR("Still waiting for: " << itsNrOfProcs << "-"
					<< itsAPAPool->onlineCount() << "=" << itsNrOfProcs-itsAPAPool->onlineCount() 
					<< " connections");
		}
		return;
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
	LOG_TRACE_STAT("State timer still running?");

	if (itsStateEngine->IsStateExpired()) {
		// Special case: handle Quit command extras
		// when quit state failed we must perform the kill state anyway.
		if (itsCurState == StateQuitCmd) {
			itsProcRuler.stopAll();
			writeResultFile();
			itsIsRunning = false;
		}

		sendExecutionResult(0, "Timed out");
	}

}


//
// checkCmdStack()
//
// Check if it is time to execute the command that is placed on the CmdStack.
//
void ApplController::checkCmdStack() 
{
	LOG_TRACE_STAT("Time for a stack command?");

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
	LOG_TRACE_STAT("Time for next commmand phase?");
	if (!itsStateEngine->isStateFinished()) {	
		return;
	}

	// State was flagged ready, check if there is another state we should 
	// execute.

	// Special case: handle Quit command extras
	if (itsCurState == StateKillAppl) {
		writeResultFile();
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

