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
#if defined(__APPLE__)
# include <libgen.h>				//# basename
#endif
#include <Common/lofar_string.h>
#include <Common/LofarLogger.h>
#include <Common/hexdump.h>			// TODO: remove in final version
#include <Common/Net/Socket.h>
#include <Transport/TH_Socket.h>
#include <ACC/ApplControlServer.h>	//# communication stub
#include <ACC/ACCmdImpl.h>			//# the real implementation
#include <ACC/CmdStack.h>
#include <ACC/APAdminPool.h>
#include <ACC/DH_ProcControl.h>
#include <ACC/ParameterSet.h>

using namespace LOFAR;
using namespace LOFAR::ACC;

static	ParameterSet		thePS;

void handleProcMessage(APAdmin*	anAP)
{
	DH_ProcControl*		DHProcPtr = anAP->getDH();
	PCCmd				command   = DHProcPtr->getCommand();

	hexdump (DHProcPtr->getDataPtr(), DHProcPtr->getDataSize());
	cout << "command=" << DHProcPtr->getCommand() << endl;
	cout << "options=" << DHProcPtr->getOptions() << endl;

	switch (command) {
	case PCCmdInfo:
		// TODO
		break;
	case PCCmdAnswer:
		// TODO
		break;
	case PCCmdStart:						// send by AP asa it is on the air.
		anAP->setName(DHProcPtr->getOptions());
		APAdminPool::getInstance().markAsOnline(anAP);
		break;
	default:
		if (command & PCCmdResult) {
			uint16	result  = DHProcPtr->getResult();
			APAdminPool::getInstance().registerAck(
								static_cast<PCCmd>(command^PCCmdResult), anAP);
			//TODO do something with the result value !!!
			// Is it an ACK or a NACK !!!
		}
		else {
			LOG_WARN(formatString(
					"Unexpected command (%d) received from process (%s)",
					command, "procnameTODO"));
		}
	} // switch
}




//
// MAIN (ParameterFile)
//
int main (int	argc, char*	argv[]) {

	// Always get Logger up first
	string		progName = basename(argv[0]);
	INIT_LOGGER (progName.c_str());

	// Check invocation syntax
	if (argc < 2) {
		LOG_FATAL_STR ("Invocation error, syntax: " << progName << " configID");
		cout << "Invocation error, syntax: " << progName << " configID" << endl;
		return (-1);
	}

	// Tell operator we are tryingto start up.
	LOG_INFO_STR("Starting up: " << argv[1]);

	try {
		// Read in the parameterfile
		thePS.adoptFile(argv[1]);	// May throw

		ACCmdImpl			theACCmdImpl;		// The command implementation
		time_t				curTime;			// Current timestamp
		time_t				cmdExpireTime;		// expire time of command
		CmdStack			theCmdStack;		// future commands
		DH_ApplControl* 	newCmd;				// command to handle now
		APAdminPool&		theAPAPool = APAdminPool::getInstance();

		// Setup listener for ACC user and wait (max 10 sec) for connection
		ApplControlServer	serverStub(thePS.getInt("AC.portnr"), &theACCmdImpl);

		// Setup listener for application processes
		Socket theProcListener("APlistener", thePS.getString("AC.procPortnr"),
										Socket::TCP, thePS.getInt("AC.backlog"));
		ASSERTSTR(theProcListener.ok(), 
							"Can't start listener for application processes");

		LOG_INFO("Entering main loop");

		bool running = true;
		while (running) {
			// NOTE 1: The AC should guard the connection with the AM. When the
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

			//  First check if a new command is available from the user.
			LOG_DEBUG("Polling user side");
			if (serverStub.pollForMessage()) {			// new command received?
				newCmd = serverStub.getDataHolder();
				time_t execTime = newCmd->getScheduleTime();
				curTime = time(0);
				if (execTime <= curTime) {				// direct command?
					LOG_TRACE_FLOW("Executing command");
					// By calling handleMessage the routines of ACCmdImpl
					// are called.
					serverStub.handleMessage(newCmd);
					// start expire timer for this command
					// TODO:How to get the right expire time
					cmdExpireTime = curTime + 20;
				}
				else {									// future command
					LOG_TRACE_FLOW("Scheduling command");
					// construct a generic command structure
					ACCommand	ACcmd(newCmd->getCommand(),
									  newCmd->getScheduleTime(),
									  newCmd->getWaitTime(),
									  newCmd->getOptions(),
									  newCmd->getProcList(),
									  newCmd->getNodeList());
					// schedule it.
					theCmdStack.add(newCmd->getScheduleTime(), &ACcmd);
					// Tell user it is scheduled.
					serverStub.sendResult(AcCmdMaskOk | AcCmdMaskScheduled);
				}
			}

			// Any new incomming connections from the appl. processes?
			LOG_DEBUG("New processes to connect?");
			Socket*		newAPSocket;
			while ((newAPSocket = theProcListener.accept(100))) {
				LOG_DEBUG("Incomming process connection");
				APAdmin*	APAdminPtr = new APAdmin(newAPSocket);
				theAPAPool.add(APAdminPtr);
			}

			// Anything received from the application processes?
			LOG_DEBUG("Polling process side");
			APAdmin*		activeAP;
			if ((activeAP = theAPAPool.poll(1000))) {
				handleProcMessage(activeAP);	// handle it
			}

			// Check for disconnected client processes. During the read action
			// it may have turned out that a process has dropped the connection
			// this is registered in the Socket. Cleanup these APadmins.
			LOG_DEBUG("Cleaning process side");
			while(APAdmin*	anAPA = theAPAPool.cleanup()) {
				// TODO: AM.report(anAP.getName() << " has disconnected");
				delete anAPA;		// finally delete it.
			}

			// Received all acks?
			if (theAPAPool.allAcksReceived()) {
				serverStub.sendResult(AcCmdMaskOk, "");		// notify master
				theAPAPool.stopAckCollection();				// stop collection
				cmdExpireTime = 0;								// stop timer
			}

			// Still a command timer running?
			if (cmdExpireTime && (cmdExpireTime < time(0))) {
				serverStub.sendResult(0, "timed out");		// notify master
				cmdExpireTime = 0;
			}
	
			// DH's are handled, time for new command?
			// TODO: may we start a new one while the last one is still running?
			if (theCmdStack.timeExpired()) {
				ACCommand 	ACCmd = theCmdStack.pop();
				serverStub.handleMessage(&ACCmd);
			}

			cout << theAPAPool;

			sleep (1);
		}
		LOG_INFO("Connection with ACC user lost, exiting.");

	}
	catch (LOFAR::Exception&		ex) {
		LOG_FATAL_STR("Caught exception: " << ex << endl);
		LOG_FATAL 	 ("Terminated by exception!");
		return(1);
	}

	LOG_INFO("Terminated normally");
	return (0);
}


