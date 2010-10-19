//# tClient.cc: Example client for testing tServer.
//#
//# Copyright (C) 2002-2005
//# ASTRON (Netherlands Institute for Radio Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands
//#
//# This file is part of the LOFAR software suite.
//# The LOFAR software suite is free software: you can redistribute it and/or
//# modify it under the terms of the GNU General Public License as published
//# by the Free Software Foundation, either version 3 of the License, or
//# (at your option) any later version.
//#
//# The LOFAR software suite is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License along
//# with the LOFAR software suite. If not, see <http://www.gnu.org/licenses/>.
//#
//# $Id$

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

#ifdef USE_NOSOCKETS
int main()
{
  return 3;
}

#else

//# Includes
#include <time.h>
#include <libgen.h>
#include <Common/LofarLogger.h>
#include <Transport/TH_Socket.h>
#include <Transport/Connection.h>
#include <DH_Socket.h>

using namespace LOFAR;

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
					  " portnr");
		return (-1);
	}

	LOG_INFO_STR("Starting up: " << argv[0] << "(" << argv[1] << ")");

	try {
		// get access to server
		TH_Socket	testSocket("localhost", string(argv[1]), true);
		LOG_DEBUG("Setting up connection...");
		while (!testSocket.init()) {
			LOG_DEBUG("no connection yet, retry in 1 second");
			sleep (1);
		}
		LOG_DEBUG("Connection made succesfully");

		// Construct a DH
		DH_Socket	theDH;
		theDH.init();			// DONT FORGET THIS !!!!!

		// Setup a Connection
		Connection	readConn  ("read",  0, &theDH, &testSocket);
		Connection	writeConn ("write", &theDH, 0, &testSocket);

		// Enter main loop
		for (int count = 0; count < 1; count++) {
			// fill dataBuffer
			theDH.setBufferSize(250);
			theDH.setCounter(250);
			uchar*	buffer = theDH.getBuffer();
			for (int32 i = 0; i < 250; i++) {
				buffer[i] = (i & 0xFF);
			}
			
			// Send the data
			LOG_DEBUG("Sending test packet");
			writeConn.write();
			writeConn.waitForWrite();

			// wait for answer
			LOG_DEBUG("Waiting for answer");
			readConn.read();
			readConn.waitForRead();

			// Check answer
			DH_Socket*	aDHS = dynamic_cast<DH_Socket*>
												   (readConn.getDataHolder());
			int32 nrBytes = aDHS->getCounter();
			buffer = static_cast<uchar*>(aDHS->getBuffer());
			for (int32 b = 0; b < nrBytes; b++) {
				if (buffer[b] != (b ^= 0xFF)) {
					THROW(Exception, "Element " << b << " is wrong");
				}
			}
			LOG_DEBUG("Answer OK");

			sleep (3);
		}	// for

		LOG_INFO_STR("Shutting down: " << argv[0]);

		// IMPLEMENT: do all neccesary shutdown actions.
	}
	catch (Exception&	ex) {
		LOG_FATAL_STR("Caught exception: " << ex << endl);
		LOG_FATAL_STR(argv[0] << " terminated by exception!");
		return(1);
	}


	LOG_INFO_STR(argv[0] << " terminated normally");
	return (0);
}

#endif
