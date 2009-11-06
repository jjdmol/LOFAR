//#  ApplControllerMain.cc: Executes the Application Controller.
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
#include <signal.h>
#include <libgen.h>
#include <Common/lofar_string.h>
#include <Common/LofarLogger.h>
#include <Common/LofarLocators.h>
#include "ApplController.h"

using namespace LOFAR;
using namespace LOFAR::ACC;

//
// MAIN (ParameterFile)
//
int main (int	argc, char*	argv[]) 
{

	// close filedescriptors from our launcher
//	for (int f = dup(2); f > 2; --f) {
//		close(f);
//	}

	// Always bring up he logger first
	ConfigLocator	aCL;
	string			progName(basename(argv[0]));
#ifdef HAVE_LOG4CPLUS
        string			logPropFile(progName + ".log_prop");
	INIT_VAR_LOGGER (aCL.locate(logPropFile).c_str(), progName + "-" + argv[1]);
#else
        string logPropFile(progName + ".debug");
        INIT_LOGGER (aCL.locate(logPropFile).c_str());	
#endif
	LOG_DEBUG_STR("Initialized logsystem with: " << aCL.locate(logPropFile));

	// Check invocation syntax
	if (argc < 2) {
		LOG_FATAL_STR ("Invocation error, syntax: " << progName << " configID");
		cout << "Invocation error, syntax: " << progName << " configID" << endl;
		return (-1);
	}

	// Tell operator we are tryingto start up.
	LOG_INFO_STR("Starting up: " << argv[0] << "(" << argv[1] << ")");

	try {
		signal (SIGPIPE, SIG_IGN);		// ignore write errors on sockets
		// close filedescriptors from our launcher
// 		for (int f = dup(2); f > 2; --f) {
// 			close(f);
// 		}

		ApplController		theAC(argv[1]);

		theAC.startupNetwork();

		theAC.doEventLoop();
		
		LOG_INFO_STR("Shutting down: " << argv[0]);
	}
	catch (LOFAR::Exception&		ex) {
		LOG_FATAL_STR("Caught exception: " << ex << endl);
		LOG_FATAL 	 ("Terminated by exception!");
		return(1);
	}

	LOG_INFO("Terminated normally");
	return (0);
}
