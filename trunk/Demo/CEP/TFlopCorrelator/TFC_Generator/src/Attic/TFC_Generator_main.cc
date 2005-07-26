//#  TFC_Generator_main.cc:
//#
//#  Copyright (C) 2002-2004
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//#  $Id$

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

#include <Common/lofar_iostream.h> 
#include <Common/LofarLogger.h>

#include <tinyCEP/Profiler.h>
#include <tinyCEP/ApplicationHolderController.h>
#include <TFC_Generator/AH_FakeStation.h>
#include <APS/ParameterSet.h>

using namespace LOFAR;

int main (int argc, const char** argv) {
  INIT_LOGGER("TFC_Generator");

  // Check invocation syntax
  try {
    if (argc < 2) {
      LOG_TRACE_FLOW("Main program not started by ACC");
      // there are no commandline arguments, so we were not called by ACC
      AH_FakeStation myAH;

      ACC::APS::ParameterSet ps("TFlopCorrelator.cfg"); 
      int NoRuns = ps.getInt32("Generator.NoRuns");
      myAH.setParameters(ps);
      
      myAH.setarg(argc, argv);
      myAH.baseDefine();
      cout << "defined" << endl;
      Profiler::init();
      myAH.basePrerun();
      cout << "init done" << endl;
      Profiler::activate();
      cout << "run" << endl;
      if (NoRuns == 0) {
	while (1) {
	  myAH.baseRun(1000);
	};
      } else {
	myAH.baseRun(NoRuns);
      }
      cout << "run complete" << endl;
      myAH.baseDump();
      myAH.baseQuit();
      Profiler::deActivate();
    } else {
      LOG_TRACE_FLOW("Main program started by ACC");
      // we were called by ACC so execute the ACCmain
      AH_FakeStation myAH;
      ApplicationHolderController myAHController(myAH);
      myAHController.main(argc, argv);
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
