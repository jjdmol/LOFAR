//#  ACDaemonMain.cc: daemon for launching application controllers.
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
#include<Common/LofarLogger.h>
#include<ACC/ACDaemon.h>

using namespace LOFAR;
using namespace LOFAR::ACC;

//
// MAIN (parameterfile)
//
int main (int argc, char* argv[]) {

	// Always bring up he logger first
	string		progName = basename(argv[0]);
	INIT_LOGGER (progName.c_str());

	// Check invocation syntax
	if (argc < 2) {
		LOG_FATAL_STR ("Invocation error, syntax: " << progName <<
															" parameterfile");
		return (-1);
	}

	// Tell operator we are trying to start up.
	LOG_INFO_STR("Starting up: " << argv[0] << "(" << argv[1] << ")");

	try {
#if REAL_DAEMON
		pid_t pid = fork();
		switch (pid) {
		case -1:	// error
			LOG_FATAL("Unable to fork daemon process, exiting.");
			return (-1);
		case 0:		// child (the real daemon)
			// do nothing;
		default:	// parent
			LOG_INFO_STR("Daemon succesfully started, pid = " << pid);
			return (0);
		}
#endif		

		ACDaemon	theDaemon(argv[1]);

		theDaemon.doWork();

		LOG_INFO_STR("Shutting down: " << argv[0]);
	}
	catch (LOFAR::Exception&	ex) {
		LOG_FATAL_STR("Caught exception: " << ex);
		LOG_FATAL     ("Terminated by exception!");
		return (1);
	}

	LOG_INFO("Terminated normally");
	return (0);

}
