//#  main: main function for CS1_InputSection
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
#include <Common/LofarLogger.h>
#include <CS1_InputSection/AH_InputSection.h>
#include <tinyCEP/Profiler.h>
#include <tinyCEP/ApplicationHolderController.h>

#ifdef HAVE_MPI
#include <Transport/TH_MPI.h>
#endif

using namespace LOFAR;
using namespace LOFAR::CS1_InputSection;

int main (int argc, const char** argv) {

  INIT_LOGGER("CS1_InputSection");

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

      ACC::APS::ParameterSet ps("CS1.cfg"); 
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
