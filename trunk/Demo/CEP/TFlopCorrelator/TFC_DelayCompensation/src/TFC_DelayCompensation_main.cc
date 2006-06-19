//#  TFC_DelayCompensation_main.cc:
//#
//#  Copyright (C) 2002-2004
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//#  $Id$

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

// for strncmp
#include <string.h>

#include <Common/lofar_iostream.h> 
#include <Common/LofarLogger.h>

#include <tinyCEP/ApplicationHolderController.h>
#include <TFC_DelayCompensation/AH_DelayCompensation.h>

using namespace LOFAR;

int main (int argc, char** argv) {
  INIT_LOGGER("TFC_DelayCompensation");

  // Check invocation syntax
  try {
    if ((argc==3) && (strncmp("ACC", argv[1], 3) == 0)) {
      LOG_TRACE_FLOW("Main program started by ACC");
      // we were called by ACC so execute the ACCmain
      AH_DelayCompensation myAH;
      ApplicationHolderController myAHController(myAH);
      myAHController.main(argc, argv);
    } else {
      LOG_TRACE_FLOW("Main program not started by ACC");
      // there are no commandline arguments, so we were not called by ACC

      AH_DelayCompensation myAH;
      myAH.setarg(argc, argv);

      ACC::APS::ParameterSet pSet("TFlopCorrelator.cfg");
      myAH.setParameters(pSet);

      myAH.baseDefine();
      cout << "defined" << endl;
      Profiler::init();
      myAH.basePrerun();
      cout << "init done" << endl;
      Profiler::activate();
      int nrRuns = pSet.getInt32("General.NRuns");     // The same number of runs as
      cout << "run " << nrRuns << " times" << endl;  // the input section
      myAH.baseRun(nrRuns);
      cout << "run complete" << endl;
      myAH.baseDump();
      myAH.baseQuit();
      Profiler::deActivate();
    }
  } catch (Exception& ex) {
    LOG_FATAL_STR("Caught exception: " << ex << endl);
    LOG_FATAL_STR(argv[0] << " terminated by exception!");
    exit(1);
  } catch (...) {
    LOG_FATAL_STR("Caught unknown exception, exitting");
    exit (1);
  }  
  LOG_INFO_STR(argv[0] << " terminated normally");
  return (0);
}
