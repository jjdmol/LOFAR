//# ACCmain.cc: main loop that can be used by any ACC enabled program
//#
//# Copyright (C) 2006
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

//# Includes
#include <libgen.h>
#include <Common/LofarLogger.h>
#include <Common/LofarLocators.h>
#include <Common/ParameterSet.h>
#include <Common/Exceptions.h>
#include <PLC/ProcControlServer.h>
#include <PLC/ProcCtrlCmdLine.h>
#include <PLC/ProcCtrlRemote.h>
#include <PLC/ACCmain.h>
#ifdef HAVE_MPI
#include <Transport/TH_MPI.h>
#endif

namespace LOFAR {
  namespace ACC {
    namespace PLC {

using LOFAR::ParameterSet;

//
// ACCmain(argc, argv, procCtrl*)
//
int ACCmain (int argc, char* orig_argv[], ProcessControl* theProcess) {
	char** argv = orig_argv;

#ifdef HAVE_MPI
	TH_MPI::initMPI(argc, orig_argv);

	int myRank = TH_MPI::getCurrentRank();

	// The MPI standard does not demand that the commandline
	// arguments are distributed, so we do it ourselves.

	// Broadcast number of arguments
	MPI_Bcast(&argc, 1, MPI_INT, 0, MPI_COMM_WORLD);
	// Some MPI implementations block on the Bcast. Synchronize
	// the nodes to avoid deadlock.
	MPI_Barrier(MPI_COMM_WORLD);

	if (myRank != 0) {
	        argv = new char*[argc + 1];
	        argv[argc] = 0;
	} else {
		char** argv = orig_argv;
	}

	for (int arg = 0; arg < argc; arg++) {
		int arglen = 0;
		if (myRank == 0) {
			arglen = strlen(argv[arg]) + 1;
		}

		// Broadcast the length of this argument
		MPI_Bcast(&arglen, 1, MPI_INT, 0, MPI_COMM_WORLD);

		if (myRank != 0) {
			argv[arg] = new char[arglen];
		}
		// Broadcast the argument;
		MPI_Bcast(argv[arg], arglen, MPI_BYTE, 0, MPI_COMM_WORLD);
	}
#endif

	string	programName(basename(argv[0]));
	bool	ACCmode(true);
        int     result(0);

	// Check invocation syntax: [ACC] parsetfile UniqProcesName
	// When we are called by ACC the first argument is ACC.
	// otherwise we do all states right after each other.
	if ((argc < 2) || (strcmp("ACC", argv[1]) != 0)) {
		ACCmode = false;
	}
	LOG_DEBUG(programName + (ACCmode ? " " : " not ") + "started by ACC");

	try {
		// Read in the parameterset.
		ConfigLocator	CL;
		string ParsetFile;
    if (argc > 1) {
      ParsetFile = CL.locate(argv[1 + (ACCmode ? 1 : 0)]);
    }
    else {
      ParsetFile = CL.locate(programName + ".parset");
    }

		ASSERTSTR(!ParsetFile.empty(), "Could not find parameterset " << argv[1]);
		LOG_INFO_STR("Using parameterset " << ParsetFile);
		globalParameterSet()->adoptFile(ParsetFile);

                // Use a local parameterset to pass arguments.
                ParameterSet arg;
                arg.add("ProgramName", programName);

                // Create the correct ProcCtrlProxy and start it.
                if (ACCmode) {
                  arg.add("ProcID", argv[3]);
                  result = (ProcCtrlRemote(theProcess))(arg);
                }
                else {
                  if (argc > 1) {
                    arg.add("NoRuns", argv[argc-1]);
                  }
                  else {
                     arg.add("NoRuns", "1");
                  }
                  result = (ProcCtrlCmdLine(theProcess))(arg);
                }
	}
	catch (Exception& ex) {
		LOG_FATAL_STR("Caught exception: " << ex << endl);
		result = 1;
	}
	catch (std::exception& ex) {
		LOG_FATAL_STR("Caught std::exception: " << ex.what());
		result = 1;
	}
	catch (...) {
		LOG_FATAL_STR("Caught unknown exception.");
		result = 1;
	}

#ifdef HAVE_MPI
	TH_MPI::finalize();
#endif

	LOG_INFO(programName + " terminated " +
                 (result ? "with an error" : "normally"));

	return result;
}

    } // namespace PLC
  } // namespace ACC
} // namespace LOFAR
