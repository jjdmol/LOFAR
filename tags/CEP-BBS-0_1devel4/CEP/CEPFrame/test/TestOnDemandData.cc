//#  TestOnDemandData.cc: Test the OnDemand data transport mechanism
//#
//#  Copyright (C) 2000, 2001
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

#include <CEPFrame/BaseSim.h>
#include <CEPFrame/Transport.h>
#include <CEPFrame/Step.h>
#include <CEPFrame/Simul.h>
#include <CEPFrame/DH_Tester.h>
#include <CEPFrame/WH_TestOnDemandData.h>
#include <tinyCEP/Profiler.h>
#include <Common/LofarLogger.h>

int main (int argc, char *argv[])
{
  // Set trace level.
  INIT_LOGGER("TestOnDemand.log_prop");
  // initialise MPI environment
  TRANSPORTER::init(argc,argv);
  int rank = TRANSPORTER::getCurrentRank ();
  unsigned int size = TRANSPORTER::getNumberOfNodes();
  int appl = Step::getCurAppl ();
  cout << "CEPFrame Processor " << rank << " of " << size
       << " operational  (appl=" << appl << ')' << endl;

  // create the main Simul; Steps and Simuls will be added to this one
  WH_TestOnDemandData tester;
  Simul testerSim(&tester, "TestOnDemandDataSim"); //Uses an empty workholder.
  testerSim.runOnNode(0);
  
  // Now start defining the simulation. 
  // First put the antennas in the Station Simul
  WH_TestOnDemandData tester1;
  Step step1(&tester1, "step1", false);
  step1.runOnNode(0);
  
  WH_TestOnDemandData tester2;
  Step step2(&tester2, "step2", false);
  step2.runOnNode(1);
  

  WH_TestOnDemandData tester3;
  Step step3(&tester3, "step3", false);
  step3.runOnNode(2);

  testerSim.addStep (&step1);
  testerSim.addStep (&step2);
  testerSim.addStep (&step3);

  step2.connectOnDemand(&step1);
  step3.connectOnDemand(&step2);

  testerSim.connect ("step1", "step2");
  testerSim.connect ("step2", "step3");

  testerSim.checkConnections();
  testerSim.simplifyConnections();

  //  testerSim.connectOutputToArray(fft,ELEMENTS);
  //////////////////////////////////////////////////////////////////////
  //
  // Finished configuration 
//////////////////////////////////////////////////////////////

  LOG_TRACE_FLOW("Ready with definition of configuration");
  Profiler::init();

  testerSim.preprocess();

  cout << endl <<  "Start Process" << endl;    
  for (int i = 0; i < 5; i++) {
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

  cout << endl << "END OF SIMUL on node " << rank << endl;
 
 
  //     close MPI environment
  TRANSPORTER::finalize();
  return 0;
}
