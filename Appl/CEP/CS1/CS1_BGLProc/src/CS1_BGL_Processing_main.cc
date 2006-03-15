//#  CS1_BGL_Processing_main.cc:
//#
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


#include <lofar_config.h>

// for strncmp
#include <string.h>

#include <Common/lofar_iostream.h> 
#include <Common/LofarLogger.h>
#include <tinyCEP/Profiler.h>
#include <tinyCEP/ApplicationHolderController.h>

#include <Transport/TH_MPI.h>
#include <CS1_BGLProc/AH_BGL_Processing.h>

using namespace LOFAR;

int main (int argc, const char** argv) {
  INIT_LOGGER("CS1_BGL_Processing");

#ifdef HAVE_MPI
  TH_MPI::initMPI(argc, argv);
#endif

  // Check invocation syntax
  try {
    if ((argc==3) && (strncmp("ACC", argv[1], 3) == 0)) {
      LOG_TRACE_FLOW("Main program started by ACC");
      // we were called by ACC so execute the ACCmain
      AH_BGL_Processing myAH;
      ApplicationHolderController myAHController(myAH);
      myAHController.main(argc, argv);
    } else {
      LOG_TRACE_FLOW("Main program not started by ACC");
      // there are no commandline arguments, so we were not called by ACC
      AH_BGL_Processing myAH;
      myAH.setarg(argc, argv);
      cout << "Reading ParameterSet" << endl;
      cout.flush();
      ACC::APS::ParameterSet ps("CS1.cfg");
      myAH.setParameters(ps);
      cout << "Starting baseDefine" << endl;
      cout.flush();

      myAH.baseDefine();
      cout << "defined" << endl;
      cout.flush();
      Profiler::init();
      int syncState = Profiler::defineState("synchronise", "yellow");
      myAH.basePrerun();
      cout << "init done" << endl;
      cout.flush();
      Profiler::activate();
      int nrSeconds = ps.getInt32("General.NRuns");
      int nrSlaves = ps.getInt32("BGLProc.SlavesPerSubband");
      ASSERTSTR( (fmod ((float)nrSeconds, (float)nrSlaves)) == 0, 
		"General.NRuns should be a multiple of BGLProc.SlavesPerSubband");
      int nrRuns = nrSeconds / nrSlaves;
      cout << "run " << nrRuns << " time" << endl;
      cout.flush();
      Profiler::enterState(syncState);
      TH_MPI::synchroniseAllProcesses();
      Profiler::leaveState(syncState);
      myAH.baseRun(nrRuns);
      cout << "run complete" << endl;
      Profiler::enterState(syncState);
      TH_MPI::synchroniseAllProcesses();
      Profiler::leaveState(syncState);
      cout.flush();
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
    perror("hopefully this helps:");
    exit (1);
  }  
  LOG_INFO_STR(argv[0] << " terminated normally");
  return (0);
}
