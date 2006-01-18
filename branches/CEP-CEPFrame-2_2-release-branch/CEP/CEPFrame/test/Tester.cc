//  Tester.cc:
//
//  Copyright (C) 2000, 2001
//  ASTRON (Netherlands Foundation for Research in Astronomy)
//  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
//  $Id$


// Simulation.cpp
// This is the main program for the LOFAR prototype simulation using the 
// LOFARSim simulation environment.
// 
#include <lofar_config.h>

#include <Transport/TH_Mem.h>
#include <Transport/TH_MPI.h>
#include <CEPFrame/Step.h>
#include <CEPFrame/Composite.h>
#include <CEPFrame/DH_Tester.h>
#include <CEPFrame/WH_Tester.h>
#include <tinyCEP/Profiler.h>
#include <Common/LofarLogger.h>

using namespace LOFAR;

int main (int argc, const char *argv[])
{
  // Set trace level.
  INIT_LOGGER("Tester.log_prop");

#ifdef HAVE_MPI
  // initialise MPI environment
  TH_MPI::initMPI(argc,argv);
#endif

  int rank = TRANSPORTER::getCurrentRank ();
  unsigned int size = TRANSPORTER::getNumberOfNodes();
  int appl = Step::getCurAppl ();
//   cout << "CEPFrame Processor " << rank << " of " << size
//        << " operational  (appl=" << appl << ')' << endl;

  // create the main Simul; Steps and Simuls will be added to this one
  WH_Tester tester;
  Composite testerSim(0, 0, "TesterSim"); 
  testerSim.runOnNode(0);
  
  // Now start defining the simulation. 
  // First put the antennas in the Station Simul
  WH_Tester tester1;
  Step step1(&tester1, "step1", false);
//   step1.runOnNode(0);
  
  WH_Tester tester2;
  Step step2(&tester2, "step2", false);
//   step2.runOnNode(1);
  

  WH_Tester tester3;
  Step step3(&tester3, "step3", false);
//   step3.runOnNode(2);

  testerSim.addBlock (&step1);
  testerSim.addBlock (&step2);
  testerSim.addBlock (&step3);

// #ifdef HAVE_MPI
//   testerSim.connect ("step1", "step2", new TH_MPI(0,1), false);
//   testerSim.connect ("step2", "step3", new TH_MPI(1,2), false);
// #else
  step2.connectInput (&step1, new TH_Mem(), false);
  step3.connectInput (&step2, new TH_Mem(), false);
// #endif

  //  testerSim.connectOutputToArray(fft,ELEMENTS);
  //////////////////////////////////////////////////////////////////////
  //
  // Finished configuration 
//////////////////////////////////////////////////////////////

  LOG_TRACE_FLOW("Ready with definition of configuration");
  Profiler::init();

  testerSim.preprocess();

  cout << endl <<  "Start Process" << endl;    
  for (int i = 0; i < 12; i++) {
    if (i%1 == 0) { // print a dot after every 10 process steps
      cout << " . " << flush;
    }
    if (i==3)   Profiler::activate();
    testerSim.process ();
    if (i==3)   Profiler::deActivate();
    
    cout << endl << "DUMP Data from last Processing step: " << endl;
    testerSim.dump ();  
  }

  testerSim.postprocess();

  cout << endl << "END OF SIMUL" << endl;
 
 
  //     close MPI environment
  TRANSPORTER::finalize();
  return 0;
}
