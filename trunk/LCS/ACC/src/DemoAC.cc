//#  DemoAC.cc: Implements a manual (demo) Application Controller
//#
//#  Copyright (C) 2002-2004
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
//#
//#  This is a prototype of the application controller that may be used in the
//#	 ACC project. It is not intended to be complete (esp. for error checking)
//#	 but it should be usefull in getting the idea behind the concept.

#include <ACC/ProcessState.h>
#include <ACC/Event.h>
#include <ACC/ACCEvents.h>
#include <ACC/Socket.h>
#include <ACC/ACCProcessList.h>
#include <ACC/ACCProcess.h>
#include <Common/LofarLogger.h>

using namespace LOFAR;

int main (int	argc, char	*argv[]) {
	INIT_LOGGER ("ACCdemo.log_prop");

	//# Need at least one argument.
	if (argc != 2) {
		LOG_FATAL (formatString("Illegal invocation. Syntax: %s parameterfile", 
					basename(argv[0])));
		return 1;
	}
	//# Allocate a map of processInfo objects.
	ACCProcessList		theProcs;

	//# read in our parameterset and get number of processes to fire up
	ParameterSet		theParams(argv[1]);
	int16				nrOfProcs  = theParams.getInt("AC.nrOfProcesses");
	int16				nrOfCycles = theParams.getInt("AC.process.runmode.nrOfCycles");
	int16				basePortnr = theParams.getInt("AC.process.command.port");
	LOG_INFO (formatString("Controlling %d processes", nrOfProcs));

	//# First create all the parameter files for the processes
	for (int processNr = 0; processNr < nrOfProcs; processNr++) {
		//# Setup the process dependent values.
		ParameterSet		procParams(argv[1]);
		string				processLabel = formatString("process%d", processNr);
		string				processName(theParams.getString(processLabel + ".name"));
		
		// QAD add process dependant parameters to paramSet
		char		paramBuffer [1024];
		sprintf(paramBuffer, "process_controller=%s\n", processName.c_str());
		sprintf(paramBuffer+strlen(paramBuffer), "%s.command.port=%d\n", 
								processName.c_str(), basePortnr + processNr);
		sprintf(paramBuffer+strlen(paramBuffer), "%s.runmode.nrOfCycles=%d\n",
								processName.c_str(), nrOfCycles);
		procParams.adoptBuffer(string(paramBuffer));

		// QAD construct the proces info for our own admin
		char		execName[128];
		char		paramFile[128];
		sprintf(execName, "tDemoProg");
		sprintf(paramFile, "%s%d.ps", execName, processNr);

		//# Write the result
		LOG_TRACE_LOOP_STR("Creating parameterfile " << paramFile);
		procParams.writeFile(paramFile);

		//# register the process in our admin
		theProcs.registerProcess(ACCProcess(processName, execName, paramFile, 
														basePortnr + processNr));
	}

	//# Open TCP/IP channel with MAC
	//# but not in this demo.
	//	... cmdPort = listen(getInt("command.port")) ...

	//# Enter main loop: wait for instructions from MAC
	//# and execute the corresponding sequences.
	//# The user has the role of MAC in this demo.
	bool		running = true;
	while (running) {
		string		answer;
		cout << endl << endl;
		cout << "1. Prepare" << endl;
		cout << "2. Run" << endl;
		cout << "3. Stop" << endl;
		cout << "4. Resume" << endl;
		cout << "5. Snapshot" << endl;
		cout << "6. Recover" << endl;
		cout << "7. Reconfigure" << endl;
		cout << "8. Quit application" << endl;
		cout << endl;
		cout << "9. Quit demo" << endl;
		cout << endl;
		cout << "Enter command: ";
		cin >> answer;

		//# Note: we use atoi(string) instead of an int because
		//# when you enter a non-digit in an int the program
		//# starts looping and there is no such thing as flush on
		//# istream.
		switch (atoi(answer.c_str())) {
		case 1: {									// # MCS_PREPARE
			//# Start up all processess.
			ACCProcessList::iterator		PI	  = theProcs.begin();
			bool							error = false;
			while (PI != theProcs.end()) {
				if ((PI->state() != ACCProcess::Running) && 
					(PI->state() != ACCProcess::Booting)) {
					string 	command = PI->execName() + " " + PI->paramFile();
					LOG_TRACE_COND_STR("Booting process: " << PI->procName());
					int result = system (command.c_str());
					if (result == -1) {
						//... oops an error ...
						error = true;
					}
					PI->setState(ACCProcess::Booting);
					//CmdChannels.addClientPort(PI->commandPort());
				}
				*PI++;								//# hop to next process
			}

			//# Now all processes are fired up we should be able to connect
			//# to them.
			int16		retry = 0;
			int16		MAX_FIREUP_RETRY = 3;
			int16		CHOKE_TIMEOUT = 3;
			bool	allChannelsUp = false;
			while (!allChannelsUp && retry < MAX_FIREUP_RETRY) {
				bool failure = false;
				PI 		= theProcs.begin();
				while (PI != theProcs.end()) {
					//# if not yet connected but successfull connection is made
					if (PI->state() != ACCProcess::Running) {
						LOG_DEBUG_STR("Trying to connect to proc at " << PI->portnr());
						if (PI->socket().connect("localhost", PI->portnr())) {
							LOG_TRACE_COND("connected.");
							PI->setState(ACCProcess::Running);
						}
						else {
							failure = true;
						}
					}
					*PI++;							//# try next channel
				}
				allChannelsUp = !failure;
				if (!allChannelsUp) {
					LOG_TRACE_FLOW("Not all processes are up yet, going to retry");
					sleep (CHOKE_TIMEOUT);
					++retry;
				}
			}

			if (!allChannelsUp) {
				//... report problems to MAC ...
				LOG_ERROR("Unable to fireup all required processess");
			}
		} 
		break;

		case 2:							//# life is easy sometimes.
			theProcs.sendAll(ACE_START);
			break;
		case 3:
			theProcs.sendAll(ACE_STOP);
			break;
		case 4:
			theProcs.sendAll(ACE_RESUME);
			break;
		case 5:
			theProcs.sendAll(ACE_SNAPSHOT);
			break;
		case 6:
			theProcs.sendAll(ACE_RECOVER);
			break;
		case 7:
			theProcs.sendAll(ACE_RECONFIGURE);
			break;
		case 8:
			theProcs.sendAll(ACE_QUIT);
			break;
		case 9:
			running = false;
			break;
		default:
			LOG_INFO_STR ("Received unknown event code: E" << answer);
		} //# switch

	} //# while
		
	//# Nicely close our connections.
	theProcs.shutdownAll();

	return (0);
}

