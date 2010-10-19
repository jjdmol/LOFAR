//#  ApplicationHolderController.cc: interpretes commands from ACC and executes them on the ApplicationHolder object
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

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>
#include <Transport/TH_MPI.h>

#include <Common/LofarLogger.h>

#include <tinyCEP/Profiler.h>
#include <ACC/ProcControlServer.h>
#include <ACC/ParameterSet.h>
#include <ACC/ProcessControl.h>

#include <ApplicationHolderController.h>

using namespace LOFAR;
using namespace LOFAR::ACC;

ApplicationHolderController::ApplicationHolderController(TinyApplicationHolder& AH, int noRuns)
  : itsAH(AH),
    itsNoRuns(noRuns),
    itsNoTotalRuns(0),
    itsIsRunning(false),
    itsShouldQuit(false),
    itsPCcomm(0)
{};
ApplicationHolderController::~ApplicationHolderController()
{};

bool ApplicationHolderController::define   () 
{
  LOG_TRACE_FLOW("Define called by ACC");
  try {
    itsAH.baseDefine();
  } catch (Exception&	ex) {
    return false;
  }
  return true;
}
bool ApplicationHolderController::init     () 
{
  LOG_TRACE_FLOW("Init called by ACC");
  try {
    Profiler::init();
    itsAH.basePrerun();
  } catch (Exception&	ex) {
    return false;
  }
  return true;
}
bool ApplicationHolderController::run      () 
{
  LOG_TRACE_FLOW("Run called by ACC");
  try {
    Profiler::activate();
    itsIsRunning = true;
  } catch (Exception&	ex) {
    return false;
  }
  return true;
}
bool ApplicationHolderController::pause    (const	string&	condition)
{
  LOG_TRACE_FLOW("Pause called");
  itsIsRunning = false;
  return true;
}
bool ApplicationHolderController::quit  	 () 
{
  LOG_TRACE_FLOW("Quit called by ACC");
  try {
    itsIsRunning = false;
    Profiler::deActivate();
    itsAH.basePostrun();
    itsAH.baseQuit();
  } catch (Exception&	ex) {
    return false;
  }
  LOG_TRACE_FLOW("Quit ready");
  return true;
}
bool ApplicationHolderController::snapshot (const string&	destination) 
{
  LOG_TRACE_FLOW("Snapshot called by ACC");
  try {
    LOG_ERROR("Snapshot not implemented in ApplicationHolder");
  } catch (Exception&	ex) {
    return false;
  }
  return false; // this is not implemented
}
bool ApplicationHolderController::recover  (const string&	source) 
{
  LOG_TRACE_FLOW("Recover called by ACC");
  try {
    LOG_ERROR("recover not implemented in ApplicationHolder");
  } catch (Exception&	ex) {
    return false;
  }
  return false; // this is not implemented
}
bool ApplicationHolderController::reinit	 (const string&	configID) 
{
  LOG_TRACE_FLOW("Reinit called by ACC");
  try {
    itsAH.basePrerun();
  } catch (Exception&	ex) {
    return false;
  }
  return true;
}  
string ApplicationHolderController::askInfo   (const string& 	keylist) 
{
  LOG_TRACE_FLOW("AskInfo called by ACC");
  return "no info available yet";
}

int ApplicationHolderController::main (int argc, const char* argv[]) {
  try {

#ifdef HAVE_MPI
    // this is needed here, because argc and argv will be changed by mpirun
    // and MPI_Init will change them back
    TH_MPI::initMPI(argc, argv);
#endif

    // Read in parameterfile and get my name
    ParameterSet itsOrigParamSet; // may throw
    itsOrigParamSet.adoptFile(argv[1]);
    string itsProcID = itsOrigParamSet.getString("process.name");
    ParameterSet itsParamSet = itsOrigParamSet.makeSubset(itsProcID);
    itsAH.setParameters(itsParamSet);

    if (itsPCcomm == 0) {
      itsPCcomm = new ProcControlServer(itsParamSet.getString("ACnode"),
					itsParamSet.getInt("ACport"),
					this);
    }

    sleep(2);
    LOG_TRACE_FLOW("Registering at ApplController");
    itsPCcomm->registerAtAC(itsProcID);
    LOG_TRACE_FLOW("Registered at ApplController");

    // Main processing loop
    while (!itsShouldQuit) {
      LOG_TRACE_FLOW("Polling ApplController for message");
      if (itsPCcomm->pollForMessage()) {
	LOG_TRACE_FLOW("Message received from ApplController");

	// get pointer to received data
	DH_ProcControl* newMsg = itsPCcomm->getDataHolder();

	itsPCcomm->handleMessage(newMsg);
      } 

      if (itsIsRunning) { 
	// If we are running call run
	// this is needed in cepframeso our program will keep running once it is started
	itsAH.baseRun(itsNoRuns);
	itsNoTotalRuns += itsNoRuns;
      } else {
	// If we are not running, we should sleep 1 second
	sleep(1);
      }
    }
    
    LOG_INFO_STR("Shutting down: ApplicationController");

    // unregister at AC and send last results
    // As an example only 1 KV pair is returned but it is allowed to pass
    // a whole parameterset.
    ParameterSet resultSet;
    string resultBuffer;
    resultSet.add(KVpair("ApplicationController.result", 
			 formatString("%d runs completed", itsNoTotalRuns),
			 true));
    resultSet.writeBuffer(resultBuffer);		// convert to stringbuffer
    itsPCcomm->unregisterAtAC(resultBuffer);		// send to AC before quiting
    }
  catch (Exception&	ex) {
    LOG_FATAL_STR("Caught exception: " << ex << endl);
    LOG_FATAL_STR(argv[0] << " terminated by exception!");
    return(1);
  }
  
  LOG_INFO_STR(argv[0] << " terminated normally");

  return 0;
}

