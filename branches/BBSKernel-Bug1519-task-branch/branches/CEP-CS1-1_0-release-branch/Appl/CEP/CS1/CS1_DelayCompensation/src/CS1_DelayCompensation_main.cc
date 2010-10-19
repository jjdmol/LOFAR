//#  CS1_DelayCompensation_main.cc:
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
#include <Common/LofarLocators.h>

#include <tinyCEP/ApplicationHolderController.h>
#include <CS1_DelayCompensation/AH_DelayCompensation.h>

using namespace LOFAR;
using namespace LOFAR::CS1;

#if 1
#include <CS1_DelayCompensation/ACCmain_DelayCompensation.h>

int main(int argc, char* argv[]) {

  ConfigLocator aCL;
  string        progName = basename(argv[0]);
  string        logPropFile(progName + ".log_prop");
  INIT_LOGGER (aCL.locate(logPropFile).c_str());
  LOG_DEBUG_STR("Initialized logsystem with: " << aCL.locate(logPropFile));

  AH_DelayCompensation myAH;
  ApplicationHolderController myAHC(myAH, 1); // listen to ACC Controller once every 1 runs.
  return ACCmain_DelayCompensation(argc, argv, &myAHC);
}

#else

int main (int argc, const char** argv) 
{
  INIT_LOGGER("CS1_DelayCompensation");

  // Check invocation syntax
  try {
    if ((argc>=3) && (toUpper(argv[1]) == "ACC")) {
      LOG_TRACE_FLOW("Main program started by ACC");
      // we were called by ACC so execute the ACCmain
      AH_DelayCompensation myAH;
      ApplicationHolderController myAHController(myAH);
      myAHController.main(argc, argv);
    } else {
      LOG_TRACE_FLOW("Main program not started by ACC");
      // there are no commandline arguments, so we were not called by ACC

      AH_DelayCompensation myAH;

      ACC::APS::ParameterSet ps("CS1.parset");
      myAH.setParameters(ps);

      myAH.setarg(argc, argv);
      myAH.baseDefine();
      LOG_INFO("defined");
      Profiler::init();
      myAH.basePrerun();
      LOG_INFO("init done");
      Profiler::activate();
      int nrRuns = ps.getInt32("General.NRuns");
      LOG_INFO_STR("run " << nrRuns << " times");
      myAH.baseRun(nrRuns);
      LOG_INFO("run complete");
      myAH.baseDump();
      myAH.baseQuit();
      Profiler::deActivate();
    }
  } catch (Exception& ex) {
    LOG_FATAL_STR("Caught exception: " << ex << endl);
    LOG_FATAL_STR(argv[0] << " terminated by exception!");
    return 1;
  } catch (...) {
    LOG_FATAL_STR("Caught unknown exception, exiting");
    return 1;
  }  
  LOG_INFO_STR(argv[0] << " terminated normally");
  return 0;
}
#endif
