//#  StationCorrelatorMain.cc: Main program station correlator
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

#include <Common/KeyParser.h>
#include <Common/KeyValueMap.h>
#include <Common/LofarLogger.h>
#include <Common/lofar_iostream.h>
#include <Common/lofar_string.h>

#include <Transport/TH_MPI.h>
#include <tinyCEP/Profiler.h>
#include <ACC/ProcControlServer.h>
#include <ACC/ParameterSet.h>
#include <ACC/ProcessControl.h>

#include <StationCorrelator.h>
#include <StationCorrelatorController.h>

using namespace LOFAR;
using namespace LOFAR::ACC;



int ACCmain (int argc, const char** argv) {
  INIT_LOGGER("StationCorrelator");

  try {
    // Read in parameterfile and get my name
    ParameterSet	itsParamSet(argv[1]);			// may throw
    string			itsProcID = itsParamSet.getString("process.name");
    LOFAR::KeyValueMap kvm = KeyParser::parseFile(itsParamSet.getString(itsProcID+".APsOwnParam"));

    StationCorrelator correlator(kvm);
    correlator.setarg(argc, argv);

    // the object that issues the commands to the StationCorrelator
    StationCorrelatorController corrContr(correlator,1);

    ProcControlServer itsPCcomm(itsParamSet.getString(itsProcID+".ACnode"),
				itsParamSet.getInt(itsProcID+".ACport"),
				&corrContr);

    sleep(2);
    LOG_TRACE_FLOW("Registering at ApplController");
    itsPCcomm.registerAtAC(itsProcID);
    LOG_TRACE_FLOW("Registered at ApplController");

    // Main processing loop
    bool shouldQuit = false;
    bool isRunning = false;
    while (!shouldQuit) {
      LOG_TRACE_FLOW("Polling ApplController for message");
      if (itsPCcomm.pollForMessage()) {
	LOG_TRACE_FLOW("Message received from ApplController");

	// get pointer to received data
	DH_ProcControl* newMsg = itsPCcomm.getDataHolder();

	// we can update our runstate here if we like
	shouldQuit = (newMsg->getCommand() == PCCmdQuit);
	if (newMsg->getCommand() == PCCmdRun) {
	  isRunning = true;
	} else if (newMsg->getCommand() == PCCmdPause || newMsg->getCommand() == PCCmdQuit) {
	  isRunning = false;
	}

	itsPCcomm.handleMessage(newMsg);
      } 

      // once in a while report some intermediate results
      if (0) {
	ParameterSet resultSet;
	string resultBuffer;
	resultSet.add(KVpair("StationCorrelator.interimresult", 
			     "Everything fine",
			     true));
	resultSet.writeBuffer(resultBuffer);
	itsPCcomm.sendResultParameters(resultBuffer);
      }
      
      // If we are not running, we should sleep 1 second
      if (!isRunning) sleep(1);

    }
    
    LOG_INFO_STR("Shutting down: StationCorrelator");

    // unregister at AC and send last results
    // As an example only 1 KV pair is returned but it is allowed to pass
    // a whole parameterset.
    ParameterSet resultSet;
    string resultBuffer;
    resultSet.add(KVpair("StationCorrelator.result", 
			 string("run completed"),
			 true));
    resultSet.writeBuffer(resultBuffer);		// convert to stringbuffer
    itsPCcomm.unregisterAtAC(resultBuffer);		// send to AC before quiting

  }
  catch (Exception&	ex) {
    LOG_FATAL_STR("Caught exception: " << ex << endl);
    LOG_FATAL_STR(argv[0] << " terminated by exception!");
    return(1);
  }
  
  LOG_INFO_STR(argv[0] << " terminated normally");
  return (0);

}

int oldmain (int argc, const char** argv) {

  INIT_LOGGER("StationCorrelator");

  try {
    KeyValueMap kvm = KeyParser::parseFile("/home/zwart/TestRange");

    StationCorrelator correlator(kvm);
    correlator.setarg(argc, argv);
    correlator.baseDefine(kvm);
    cout << "defined" << endl;
    Profiler::init();
    correlator.basePrerun();
    cout << "init done" << endl;
    Profiler::activate();
    correlator.baseRun(kvm.getInt("runsteps",1));
    cout << "run" << endl;
    //correlator.baseDump();
    correlator.baseQuit();
    Profiler::deActivate();

  } catch (std::exception& x) {
    cerr << "Unexpected exception: " << x.what() << endl; 
    cout << "Unexpected exception: " << x.what() << endl; 
  }
  return 0;
}


int main (int argc, const char** argv) {
  // Check invocation syntax
#ifdef HAVE_MPI
  TH_MPI::init(argc, argv);
#endif

  // Check invocation syntax
  if (argc < 2) {
    LOG_TRACE_FLOW("Main program not started by ACC");
    // there are no commandline arguments, so we were not called by ACC
    return oldmain(argc, argv);
  } else {
    LOG_TRACE_FLOW("Main program started by ACC");
    // we were called by ACC so execute the ACCmain
    return ACCmain(argc, argv);
  }
}
