//  Example.cc:
//
//  Copyright (C) 2000-2002
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
//
///////////////////////////////////////////////////////////////

#include <Common/lofar_iostream.h>
#include "CEPFrame/Transport.h"
#include "CEPFrame/Step.h"
#include "CEPFrame/Simul.h"
#include "CEPFrame/BaseSim.h"
#include "CEPFrame/WH_Example.h"
#include "CEPFrame/Simul2XML.h"
#include "CEPFrame/Profiler.h"
#include "Common/Debug.h"


void doIt (Simul& simul, const std::string& name, int nsteps)
{
  cout << "CheckConn before shortcut:" << endl;
  simul.checkConnections (cout);
  simul.shortcutConnections();
  cout << "CheckConn after shortcut:" << endl;
  simul.checkConnections (cout);
  TRACER2("Ready with definition of configuration");
  Profiler::init();
  Step::clearEventCount();

  simul.preprocess();

  cout << endl << "Start Processing simul " << name << endl;    
  for (int i=0; i<nsteps; i++) {
    if (i==2) Profiler::activate();
    cout << "Call simul.process() " << i << endl;
    simul.process();
    if (i==5) Profiler::deActivate();
  }

  cout << endl << "DUMP Data from last Processing step: " << endl;
  simul.dump ();
  cout << endl << "END OF SIMUL on node " 
       << TRANSPORTER::getCurrentRank () 
       << endl;
 
  simul.postprocess();
}


int main (int argc, const char *argv[])
{
  // Set trace level.
  Debug::initLevels (argc, argv);
  // initialise MPI environment
  TRANSPORTER::init(argc,argv);
  int rank = TRANSPORTER::getCurrentRank ();
  unsigned int size = TRANSPORTER::getNumberOfNodes();
  int appl = Step::getCurAppl();
  cout << "CEPFrame Processor " << rank << " of " << size
       << " operational  (appl=" << appl << ')' << endl;

  // A simple example. Four steps connected to each other.
  // Each WorkHolder has a single DataHolder.
  {
    // Define the top-level simul object.
    WH_Example whex("Example1", 0);
    Simul simul1 (whex);
    // Tell the Simul where to run.
    simul1.runOnNode(0);
    // Now start filling the simulation. 
    // First create the Steps.
    Step step1 (WH_Example("Step1", 0));
    Step step2 (WH_Example("Step2"));
    Step step3 (WH_Example("Step3"));
    Step step4 (WH_Example("Step4"));
    simul1.addStep (step1);
    simul1.addStep (step2);
    simul1.addStep (step3);
    simul1.addStep (step4);
    step4.connectInput (&step3);
    step3.connectInput (&step2);
    step2.connectInput (&step1);
    // Connect the output of the last step to the overall Simul WorkHolder.
    Step* stepPtr = &step4;
    simul1.connectOutputToArray (&stepPtr, 1);
    // Run the simulation.
    doIt (simul1, whex.getName(), 10);
  }

  // A slightly more elaborate example.
  // Use WorkHolders with multiple data holders.
  // The number of input and output DataHolders still match,
  // so still a simple connect can be used.
  {
    WH_Example whex("Example2", 0, 1, 20);
    Simul simul1 (whex);
    simul1.runOnNode(0);
    // Now start filling the simulation. 
    WH_Example whex1("Step1", 0, 2, 20);
    WH_Example whex2("Step2", 2, 3, 20);
    WH_Example whex3("Step3", 3, 1, 20);
    Step step1 (whex1);
    Step step2 (whex2);
    Step step3 (whex3);
    simul1.addStep (step1);
    simul1.addStep (step2);
    simul1.addStep (step3);
    step3.connectInput (&step2);
    step2.connectInput (&step1);
    Step* stepPtr = &step3;
    simul1.connectOutputToArray (&stepPtr, 1);
    doIt (simul1, whex.getName(), 5);
  }

  // In this example a WorkHolder with 2 output DataHolders is
  // connected to 2 WorkHolders with a single input and output DataHolder.
  // That is in its turn connected to a WorkHolder with 2 inputs.
  {
    WH_Example whex("Example3", 0);
    Simul simul1 (whex);
    simul1.runOnNode(0);
    // Now start filling the simulation. 
    WH_Example whex1("Step1", 0, 2);
    WH_Example whex2a("Step2a");
    WH_Example whex2b("Step2b");
    WH_Example whex3("Step3", 2, 1);
    Step step1 (whex1);
    Step step2a (whex2a);
    Step step2b (whex2b);
    Step step3 (whex3);
    simul1.addStep (step1);
    simul1.addStep (step2a);
    simul1.addStep (step2b);
    simul1.addStep (step3);
    Step* stepPtrs[2];
    stepPtrs[0] = &step2a;
    stepPtrs[1] = &step2b;
    step3.connectInputArray (stepPtrs, 2);
    step1.connectOutputArray (stepPtrs, 2);
    stepPtrs[0] = &step3;
    simul1.connectOutputToArray (stepPtrs, 1);
    doIt (simul1, whex.getName(), 5);
  }

  // Use Simul composites.
  {
    WH_Example whex("Example4", 0);
    Simul simul1 (whex, "simul1");
    simul1.runOnNode(0);
    // Now start filling the simulation. 
    WH_Example whex1("Step1", 0);
    WH_Example whex2("Step2");
    WH_Example whex3("Step3");
    WH_Example whex4("Step4");
    Step step1 (whex1, "Step1", false);
    Step step2 (whex2, "Step2", false);
    Step step3 (whex3, "Step3", false);
    Step step4 (whex4, "Step4", false);
    // Create the first composite, fill it, and make the internal connections.
    WH_Example whcomp1("Composite1");
    Simul composite1 (whcomp1, "simcomp1", false);
    composite1.addStep (step2);
    composite1.addStep (step3);
    step3.connectInput (&step2);
    Step* stepPtr = &step2;
    composite1.connectInputToArray (&stepPtr, 1);
    stepPtr = &step3;
    composite1.connectOutputToArray (&stepPtr, 1);
    // Create the second composite and fill it
    WH_Example whcomp2("Composite2");
    Simul composite2 (whcomp2, "simcomp2", false);
    composite2.addStep (composite1);
    composite2.addStep (step4);
    step4.connectInput (&composite1);
    stepPtr = &composite1;
    composite2.connectInputToArray (&stepPtr, 1);
    stepPtr = &step4;
    composite2.connectOutputToArray (&stepPtr, 1);
    // Finally create the total Simul.
    simul1.addStep (step1);
    simul1.addStep (composite2);
    composite2.connectInput (&step1);
    stepPtr = &composite2;
    simul1.connectOutputToArray (&stepPtr, 1);
    doIt (simul1, whex.getName(), 10);
  }

  // The same examples as above, but now connected by name.
  {
    // Define the top-level simul object.
    WH_Example whex("Example1", 0);
    Simul simul1 (whex);
    // Tell the Simul where to run.
    simul1.runOnNode(0);
    // Now start filling the simulation. 
    // First create the Steps.
    Step step1 (WH_Example("Step1", 0));
    Step step2 (WH_Example("Step2"));
    Step step3 (WH_Example("Step3"));
    Step step4 (WH_Example("Step4"));
    simul1.addStep (step1);
    simul1.addStep (step2);
    simul1.addStep (step3);
    simul1.addStep (step4);
    simul1.connect ("aStep_0", "aStep_1");
    simul1.connect ("aStep_1", "aStep_2");
    simul1.connect ("aStep_2", "aStep_3");
    // Connect the output of the last step to the overall Simul WorkHolder.
    simul1.connect ("aStep_3", ".");
    // Run the simulation.
    doIt (simul1, whex.getName(), 10);
  }

  // A slightly more elaborate example.
  // Use WorkHolders with multiple data holders.
  // The number of input and output DataHolders still match,
  // so still a simple connect can be used.
  {
    WH_Example whex("Example2", 0, 1, 20);
    Simul simul1 (whex);
    simul1.runOnNode(0);
    // Now start filling the simulation. 
    WH_Example whex1("Step1", 0, 2, 20);
    WH_Example whex2("Step2", 2, 3, 20);
    WH_Example whex3("Step3", 3, 1, 20);
    Step step1 (whex1);
    Step step2 (whex2);
    Step step3 (whex3);
    simul1.addStep (step1);
    simul1.addStep (step2);
    simul1.addStep (step3);
    simul1.connect ("aStep_0", "aStep_1");
    simul1.connect ("aStep_1", "aStep_2");
    simul1.connect ("aStep_2", ".");
    doIt (simul1, whex.getName(), 5);
  }

  // In this example a WorkHolder with 2 output DataHolders is
  // connected to 2 WorkHolders with a single input and output DataHolder.
  // That is in its turn connected to a WorkHolder with 2 inputs.
  {
    WH_Example whex("Example3", 0);
    Simul simul1 (whex);
    simul1.runOnNode(0);
    // Now start filling the simulation. 
    WH_Example whex1("Step1", 0, 2);
    WH_Example whex2a("Step2a");
    WH_Example whex2b("Step2b");
    WH_Example whex3("Step3", 2, 1);
    Step step1 (whex1);
    Step step2a (whex2a);
    Step step2b (whex2b);
    Step step3 (whex3);
    simul1.addStep (step1);
    simul1.addStep (step2a);
    simul1.addStep (step2b);
    simul1.addStep (step3);
    simul1.connect ("aStep_0.out_0", "aStep_1");
    simul1.connect ("aStep_0.out_1", "aStep_2");
    simul1.connect ("aStep_1", "aStep_3.in_0");
    simul1.connect ("aStep_2", "aStep_3.in_1");
    simul1.connect ("aStep_3.out_0", ".out_0");
    doIt (simul1, whex.getName(), 5);
  }

  // Use Simul composites.
  {
    WH_Example whex("Example4", 0);
    Simul simul1 (whex, "simul1");
    simul1.runOnNode(0);
    // Now start filling the simulation. 
    WH_Example whex1("Step1", 0);
    WH_Example whex2("Step2");
    WH_Example whex3("Step3");
    WH_Example whex4("Step4");
    Step step1 (whex1, "Step1", false);
    Step step2 (whex2, "Step2", false);
    Step step3 (whex3, "Step3", false);
    Step step4 (whex4, "Step4", false);
    // Create the first composite, fill it, and make the internal connections.
    WH_Example whcomp1("Composite1");
    Simul composite1 (whcomp1, "simcomp1", false);
    composite1.addStep (step2);
    composite1.addStep (step3);
    composite1.connect (".", "Step2");
    composite1.connect ("Step2", "Step3");
    composite1.connect ("Step3", ".");
    // Create the second composite and fill it
    WH_Example whcomp2("Composite2");
    Simul composite2 (whcomp2, "simcomp2", false);
    composite2.addStep (composite1);
    composite2.addStep (step4);
    composite2.connect (".", "simcomp1");
    composite2.connect ("simcomp1", "Step4");
    composite2.connect ("Step4", ".");
    // Finally create the total Simul.
    simul1.addStep (step1);
    simul1.addStep (composite2);
    simul1.connect ("Step1", "simcomp2");
    simul1.connect ("simcomp2", ".");
    Simul2XML toxml(simul1);
    toxml.write ("Example_tmp.xml");
    doIt (simul1, whex.getName(), 10);
  }

  //     close environment
  TRANSPORTER::finalize();
}
