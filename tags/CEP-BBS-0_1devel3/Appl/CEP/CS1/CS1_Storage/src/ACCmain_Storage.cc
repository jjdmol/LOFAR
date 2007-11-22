//#  ACCmain_Storage.cc: main loop that can be used by any ACC enabled program
//#
//#  Copyright (C) 2006
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
#include <libgen.h>
#include <Common/LofarLogger.h>
#include <Common/LofarLocators.h>
#include <APS/ParameterSet.h>
#include <APS/Exceptions.h>
#include <PLC/ProcControlServer.h>
#include <sys/time.h>
#include <CS1_Storage/ACCmain_Storage.h>
#include <CS1_Interface/CS1_Parset.h>
#include <AMCBase/Epoch.h>

#ifdef HAVE_MPI
#include <Transport/TH_MPI.h>
#endif

namespace LOFAR {
namespace CS1 {

using ACC::APS::ParameterSet;

//
// ACCmain(argc, argv, procCtrl*)
//
int ACCmain_Storage (int argc, char* orig_argv[], ACC::PLC::ProcessControl* theProcess) {
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

	// 
	string	programName(basename(argv[0]));
	bool	ACCmode(true);

	try {
		// Check invocation syntax: [ACC] parsetfile UniqProcesName
		// When we are called by ACC the first argument is ACC.
		// otherwise we do all states right after each other.
		if ((argc < 2) || (strcmp("ACC", argv[1]) != 0)) {
			// we were not called by ACC
			LOG_DEBUG(programName + " not started by ACC");
			ACCmode = false;
		}
		else {
			LOG_DEBUG(programName + " started by ACC");
		}

		// Read in the parameterset.
		ConfigLocator	CL;
		string	ParsetFile = CL.locate(argv[1 + (ACCmode ? 1 : 0)]);
		ASSERTSTR(!ParsetFile.empty(), "Could not find parameterset " << argv[1]);
		LOG_INFO_STR("Using parameterset " << ParsetFile);
		ACC::APS::globalParameterSet()->adoptFile(ParsetFile);

		// When not under control of ACC execute all modes immediately
		if (!ACCmode) {
			LOG_DEBUG(programName + " starting define");
			if (!theProcess->define()) {
				return (1);
			}

			LOG_DEBUG(programName + " initializing");
			if (!theProcess->init()) {
				return (1);
			}

			LOG_DEBUG(programName + " running");
			int noRuns = atoi(argv[argc - 1]);
			if (noRuns == 0) {
				noRuns = 1;
			}
			for (int run = 0; run < noRuns; run++) {
				if (!theProcess->run()) {
					return (1);
				}
			}

			LOG_DEBUG(programName + " releasing");
			if (!theProcess->release()) {
				return (1);
			}
			
			LOG_DEBUG(programName + " quitting");
			if (!theProcess->quit()) {
				return (1);
			}

			LOG_DEBUG(programName + " deleting process");

		} 
		else {
			// we are under control of ACC
			// Note args are: ACC parsetfile UniqProcesName
			
			string	procID(argv[3]);
			string	prefix = ACC::APS::globalParameterSet()->getString("_parsetPrefix");
			
			// connect to Application Controller
			ACC::PLC::ProcControlServer pcServer(ACC::APS::globalParameterSet()->getString(prefix+"_ACnode"),
									           ACC::APS::globalParameterSet()->getUint16(prefix+"_ACport"),
									           theProcess);


			// Tell AC who we are.
			LOG_DEBUG_STR("Registering at ApplController as " << procID);
			sleep(1);
			pcServer.registerAtAC(procID);

			// Main processing loop
			
			bool quiting(false);
			uint nRuns = 1;
			
			while (!quiting) {
				LOG_TRACE_STAT("Polling ApplController for message");
				if (pcServer.pollForMessage()) {
					LOG_TRACE_COND("Message received from ApplController");

					// get pointer to received data
					ACC::PLC::DH_ProcControl* newMsg = pcServer.getDataHolder();

		                        if (newMsg->getCommand() == ACC::PLC::PCCmdRun) {
		                          pcServer.sendResult(newMsg->getCommand(), ACC::PLC::PcCmdMaskOk);
		                          newMsg->setCommand(ACC::PLC::PCCmd(newMsg->getCommand() &~ ACC::PLC::PCCmdResult));
		                        }
                                        
                                        if (newMsg->getCommand() == ACC::PLC::PCCmdPause) {
					  CS1_Parset itsCS1PS = CS1_Parset(ACC::APS::globalParameterSet());
  					  
			                  double startTime = itsCS1PS.startTime();
			                  double stopTime = itsCS1PS.stopTime();
					  
			                  double stepTime  = itsCS1PS.BGLintegrationTime();
			
			                  uint totalRuns = uint(1 + ceil((stopTime - startTime) / stepTime));
			
			                  totalRuns = ((totalRuns+15)&~15);
					  
					  totalRuns = totalRuns / itsCS1PS.IONintegrationSteps();
					  
					  int nrRunsLeft = totalRuns - nRuns;
					  
					  time_t now = time(0);
					  char buf[26];
					  ctime_r(&now, buf);
					  buf[24] = '\0';
					  
					  cout << "time = " << buf << ", rank = " << TH_MPI::getCurrentRank() <<", nrRunsLeft = totalRuns - nRuns = " << totalRuns << " - " << nRuns << " = " << nrRunsLeft << endl;
					  
					  if (totalRuns > nRuns){
					    for (int i=0; i<nrRunsLeft; i++) {
					      if (!theProcess->run()) {
		                                return (1);
		                              }
					    }
					  }  
					}

					if (newMsg->getCommand() == ACC::PLC::PCCmdQuit) {
					        pcServer.sendResult(newMsg->getCommand(), ACC::PLC::PcCmdMaskOk);
						newMsg->setCommand(ACC::PLC::PCCmd(newMsg->getCommand() &~ ACC::PLC::PCCmdResult));
						quiting = true;
					} 

					if (!pcServer.handleMessage(newMsg)) {
						LOG_ERROR("ProcControlServer::handleMessage() failed");
					}

				} else  {
					// no new command received. If we are in the runstate 
					// call the run-routine again.
					if (theProcess->inRunState()) {
					        nRuns++;
						ACC::PLC::DH_ProcControl		tmpMsg(ACC::PLC::PCCmdRun);
						pcServer.handleMessage(&tmpMsg);
					}
				}
			}

			LOG_INFO_STR("Shutting down: ApplicationController");
			pcServer.unregisterAtAC("");		// send to AC before quiting
		}
	} 
	catch (Exception& ex) {
		LOG_FATAL_STR("Caught exception: " << ex << endl);
		LOG_FATAL_STR(programName << " terminated by exception!");
		return (1);
	} 
	catch (std::exception& ex) {
		LOG_FATAL_STR("Caught std::exception: " << ex.what());
		return (1);
	} 
	catch (...) {
		LOG_FATAL_STR("Caught unknown exception, exitting");
		return (1);
	}  

#ifdef HAVE_MPI
	TH_MPI::finalize();
#endif

	LOG_INFO_STR(programName << " terminated normally");
	return (0);
}

  } // namespace CS1
} // namespace LOFAR
