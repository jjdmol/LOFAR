//#  APExample.cc: Example program how a Application Process should respond.
//#
//#  Copyright (C) 2002-2005
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
#include <time.h>
#include <libgen.h>
#include <Common/LofarLogger.h>
#include <ACC/ProcControlServer.h>
#include <ACC/ParameterSet.h>
#include <APCmdImpl.h>

using namespace LOFAR;
using namespace LOFAR::ACC;

//
// This program demonstrates how an Application Process should communicate
// with the Application Controller. It shows the minimal implementation possible:
// [A] Connect to the AC.
// [B] Register at the AC so we receive messages.
// [C] See if the are new messages arrived.
// [D] Dispatch the message to the right routine in our APCmdImpl.
// [E] Unregister at the AC.
//
// The places where other program code should be added are marked with
// IMPLEMENT.
//
// Note: The structure of the program is conform the MAIN.cc template.
//
int main (int argc, char *argv[]) {

	// Always bring up the logger first
	string  progName = basename(argv[0]);
	INIT_LOGGER(progName.c_str());

	// Check invocation syntax
	if (argc < 2) {
		LOG_FATAL_STR("Invocation error, syntax: " << progName << 
					  " parameterfile");
		return (-1);
	}

	LOG_INFO_STR("Starting up: " << argv[0] << "(" << argv[1] << ")");

	try {
		// --- begin of example code ---

		// Read in parameterfile and get my name
		ParameterSet	itsParamSet(argv[1]);			// may throw
		string			itsProcID = itsParamSet.getString("process.name");

		// Create a APCmdImpl object. This object contains the real 
		// implementation of the commands we should support.
		APCmdImpl		itsAPCmdImpl;

		// [A] Connect to the Application Controller
		ProcControlServer	itsPCcomm(itsParamSet.getInt(itsProcID+".ACport"),
									  &itsAPCmdImpl);

		// IMPLEMENT: do other launch activities like processing the ParamSet

		// [B] Tell AC we are ready for commands
		itsPCcomm.registerAtAC(itsProcID);

		// Main processing loop
		bool itsIsRunning = true;
		while (itsIsRunning) {
			// [C] scan AC port to see if a new command was sent.
			if (itsPCcomm.pollForMessage()) {
				// get pointer to received data
				DH_ProcControl* newMsg = itsPCcomm.getDataHolder();

				// we can update our runstate here if we like
				itsIsRunning = (newMsg->getCommand() != PCCmdQuit);

				// [D] let PCcomm dispatch and handle the message
				itsPCcomm.handleMessage(newMsg);
			}

			// IMPLEMENT: do other stuff

			// TEST: simulate we are busy
			sleep (3);
		}

		LOG_INFO_STR("Shutting down: " << argv[0]);

		// [E] unregister at AC and send last results
		// As an example only 1 KV pair is returned but it is allowed to pass
		// a whole parameterset.
		ParameterSet	resultSet;
		string			resultBuffer;
		resultSet.add(itsProcID+".result", "IMPLEMENT useful information");
		resultSet.writeBuffer(resultBuffer);		// convert to stringbuffer
LOG_TRACE_VAR_STR("=====" << resultBuffer << "======");
		itsPCcomm.unregisterAtAC(resultBuffer);		// send to AC before quiting

		// IMPLEMENT: do other neccesary shutdown actions.

		// TEST: simulate we are busy.
		sleep (3);

		// --- end of example code ---
	}
	catch (Exception&	ex) {
		LOG_FATAL_STR("Caught exception: " << ex << endl);
		LOG_FATAL_STR(argv[0] << " terminated by exception!");
		return(1);
	}


	LOG_INFO_STR(argv[0] << " terminated normally")
	return (0);
}

