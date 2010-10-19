//#  TFC_InputSection_main.cc:
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

#include <tinyCEP/Profiler.h>
#include <tinyCEP/ApplicationHolderController.h>
#include <TFC_InputSection/AH_InputSection.h>
#include <APS/ParameterSet.h>

#ifdef HAVE_MPI
#include <Transport/TH_MPI.h>
#endif

using namespace LOFAR;

int main (int argc, const char** argv) {
  INIT_LOGGER("TFC_InputSection");

  // Check invocation syntax
  try {
    if ((argc==3) && (strncmp("ACC", argv[1], 3) == 0)) {
      LOG_TRACE_FLOW("Main program started by ACC");
      // we were called by ACC so execute the ACCmain
      AH_InputSection myAH;
      ApplicationHolderController myAHController(myAH);
      myAHController.main(argc, argv);
    } else {
      LOG_TRACE_FLOW("Main program not started by ACC");
      // there are no commandline arguments, so we were not called by ACC
      AH_InputSection myAH;

      ACC::APS::ParameterSet ps("TFlopCorrelator.cfg"); 
      myAH.setParameters(ps);
      
      myAH.setarg(argc, argv);
      myAH.baseDefine();
      cout << "defined" << endl;
      Profiler::init();
      myAH.basePrerun();
      cout << "init done" << endl;
      // This is for synchronisation between WH_RSPInput and WH_SBCollect
      // WH_SBCollect won't exit the preprocess before the connection is made
      // WH_RSPInput won't start the bufferthread before the barrier is passed
#ifdef HAVE_MPI
  TH_MPI::synchroniseAllProcesses();
#endif
      Profiler::activate();
      cout << "run" << endl;
      myAH.baseRun(ps.getInt32("General.NRuns"));
      cout << "run complete" << endl;
#ifdef HAVE_MPI
      TH_MPI::synchroniseAllProcesses();
#endif
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
