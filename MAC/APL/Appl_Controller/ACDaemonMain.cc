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
#include <sys/stat.h>			// umask
#include <unistd.h>                     // fork, basename
#include <Common/LofarLogger.h>
#include <Common/LofarLocators.h>
#include "ACDaemon.h"

using namespace LOFAR;
using namespace LOFAR::ACC;

//
// MAIN (parameterfile)
//
int main (int /*argc*/, char* argv[]) {

	// Always bring up the logger first
	ConfigLocator	aCL;
	string		progName = basename(argv[0]);
#ifdef HAVE_LOG4CPLUS
	string		logPropFile(progName + ".log_prop");
#else
	string		logPropFile(progName + ".debug");
#endif
#ifdef HAVE_LOG4CPLUS
	INIT_VAR_LOGGER (aCL.locate(logPropFile).c_str(), progName);
#else
        INIT_LOGGER (aCL.locate(logPropFile).c_str());	
	
#endif	
	LOG_DEBUG_STR("Initialized logsystem with: " << aCL.locate(logPropFile));

	// Tell operator we are trying to start up.
	LOG_INFO_STR("Starting up: " << argv[0]);

	try {
//#if REAL_DAEMON
		pid_t pid = fork();
		switch (pid) {
		case -1:	// error
			LOG_FATAL("Unable to fork daemon process, exiting.");
			return (-1);
		case 0:		// child (the real daemon)
			// do nothing;
			break;
		default:	// parent
			LOG_INFO_STR("Daemon succesfully started, pid = " << pid);
			return (0);
		}
//#endif		
		// TODO: active the next two calls.
//		setsid();		// disconnect from terminalsession
//		chdir("/");		// might be on a mounted file system
		umask(0);		// no limits

		ACDaemon	theDaemon(progName);

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
