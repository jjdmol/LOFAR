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
#include <Common/Net/Socket.h>
#include <Transport/TH_Socket.h>
//#include <ACC/ApplProcessPool.h>
#include <ACC/ApplControlServer.h>	//# communication stub
#include <ACC/ACCmdImpl.h>			//# the real implementation
#include <ACC/CmdStack.h>
#include <ACC/DataHolderSelector.h>
#include <ACC/DH_ProcessControl.h>
#include <ACC/ParameterSet.h>

using namespace LOFAR;
using namespace LOFAR::ACC;

static	ParameterSet		thePS;
//static	ApplProcessPool		theAPPool;

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
		DataHolderSelector	theDHpool;			// DH pool for polling
		time_t				curTime;			// Current timestamp
		time_t				commandTimer;		// execution time guard
		CmdStack			theCmdStack;		// future commands
		DH_ApplControl* 	newCmd;				// command to handle now

		// Setup listener for ACC user and wait for connection
		ApplControlServer	serverStub(thePS.getInt("AC.portnr"), &theACCmdImpl);

		// Setup listener for application processes
		Socket theProcListener("APlistener", thePS.getString("AC.procPortnr"),
										Socket::TCP, thePS.getInt("AC.backlog"));
		ASSERTSTR(theProcListener.ok(), 
							"Can't start listener for application processes");

		LOG_INFO("Entering main loop");

		bool running = true;
		while (running) {
			//  First check if a new command is available from the user.
			LOG_DEBUG("Polling user side");
			if (serverStub.pollForMessage()) {			// new command received?
				newCmd = serverStub.getDataHolder();
				time_t cmdTime = newCmd->getScheduleTime();
				curTime = time(0);
				if (cmdTime <= curTime) {				// direct command?
					LOG_TRACE_FLOW("Executing command");
					serverStub.handleMessage(newCmd);	// handle it
				}
				else {									// future command
					LOG_TRACE_FLOW("Scheduling command");
					DH_ApplControl*	theDHAC = dynamic_cast<DH_ApplControl*>
																	(newCmd);
					ACCommand	ACcmd(theDHAC->getCommand(),
									  theDHAC->getScheduleTime(),
									  theDHAC->getWaitTime(),
									  theDHAC->getOptions(),
									  "processList",
									  "nodeList");
					// schedule it.
					theCmdStack.add(theDHAC->getScheduleTime(), &ACcmd);
					serverStub.sendResult(AcCmdMaskOk | AcCmdMaskScheduled);
				}
			}

			// Any new incomming connections from the appl. processes?
			LOG_DEBUG("New processes to connect?");
			Socket*		newAPSocket;
			while ((newAPSocket = theProcListener.accept(100))) {
				LOG_DEBUG("Incomming process connection");
				// Construct a DH
				DH_ProcessControl		DHclient;
				DH_ProcessControl*		thisDH = new DH_ProcessControl;
				DHclient.setID(500);
				thisDH->setID(501);
				// Connect the TH side to it
				DHclient.connectBidirectional(*thisDH,
							TH_Socket("", "localhost", 4, true, false),
							TH_Socket("localhost", "", 4, false, false),	
							false);
				// Sneaky replace the socket with the connected one.
				TH_Socket* 	newTHS = dynamic_cast<TH_Socket*>
							(thisDH->getTransporter().getTransportHolder());
				newTHS->setDataSocket(newAPSocket);
				// Do the init (needed for the blob)
				thisDH->init();
				// Finally add it to the pool
				theDHpool.add(thisDH);
			}

			// Anything received from the application processes?
			LOG_DEBUG("Polling process side");
			DataHolder*		activeDH;
			if ((activeDH = theDHpool.poll())) {
//				serverStub.handleMessage(activeDH);	// handle it
			}

			LOG_DEBUG("Cleaning process side");
			while(theDHpool.cleanup()) {
				;
			}

			// Still a command timer running?
			if (commandTimer && (commandTimer < curTime)) {
//TODO			serverStub.sendCmd(makeFailCmd(.....));		// notify master
				commandTimer = 0;							// stop timer
			}
	
			// DH's are handled, time for new command?
			if (theCmdStack.timeExpired()) {
				ACCommand 	ACCmd = theCmdStack.pop();
				serverStub.handleMessage(&ACCmd);
			}

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


