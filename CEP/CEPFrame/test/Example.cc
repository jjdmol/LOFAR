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
#include <CEPFrame/Step.h>
#include <CEPFrame/Composite.h>
#include <Transport/TH_Mem.h>
#include <CEPFrame/WH_Example.h>
#include <CEPFrame/Profiler.h>
#include <Common/Debug.h>

using namespace LOFAR;

void doIt (Composite& comp, const std::string& name, int nsteps)
{
  TRACER2("Ready with definition of configuration");
  Profiler::init();
  Step::clearEventCount();

  try {
    comp.preprocess();

    cout << endl << "Start Processing composit " << name << endl;    
    for (int i=0; i<nsteps; i++) {
      if (i==2) Profiler::activate();
      cout << "Call composit.process() " << i << endl;
      comp.process();
      if (i==5) Profiler::deActivate();
    }

    cout << endl << "DUMP Data from last Processing step: " << endl;
    comp.dump ();
 
    comp.postprocess();
  }
  catch (LOFAR::Exception& e)
  {
    cout << "Lofar exception: " << e.what() << endl;
  }
  catch (std::exception& e)
  {
    cout << "Standard exception: " << e.what() << endl;
  }
  catch (...) {
    cout << "Unexpected exception in Simulate" << endl;
  }

}


int main (int argc, const char *argv[])
{
  // Set trace level.
  Debug::initLevels (argc, argv);

  // A simple example. Four steps connected to each other.
  // Each WorkHolder has a single DataHolder.
  {
    // Define the top-level simul object.
    WH_Example whex("Example1", 0);
    Composite comp1 (whex);
    // Tell the Composit where to run.
    comp1.runOnNode(0);
    // Now start filling the simulation. 
    // First create the Steps.
    WH_Example whEx1("Step1", 0);
    Step step1 (whEx1);
    WH_Example whEx2("Step2");
    Step step2 (whEx2);
    WH_Example whEx3("Step3");
    Step step3 (whEx3);
    WH_Example whEx4("Step4");
    Step step4 (whEx4);
    comp1.addStep (step1);
    comp1.addStep (step2);
    comp1.addStep (step3);
    comp1.addStep (step4);
    step4.connectInput (&step3, TH_Mem(), false);
    step3.connectInput (&step2, TH_Mem(), false );
    step2.connectInput (&step1, TH_Mem(), false);
    // Connect the output of the last step to the overall Simul WorkHolder.
    Step* stepPtr = &step4;
    comp1.connectOutputToArray (&stepPtr, 1,0,0, TH_Mem(), false);
    // Run the simulation.
    doIt (comp1, whex.getName(), 10);
  }

  // A slightly more elaborate example.
  // Use WorkHolders with multiple data holders.
  // The number of input and output DataHolders still match,
  // so still a simple connect can be used.
  {
    WH_Example whex("Example2", 0, 1, 20);
    Composite comp1 (whex);
    comp1.runOnNode(0);
    // Now start filling the simulation. 
    WH_Example whex1("Step1", 0, 2, 20);
    WH_Example whex2("Step2", 2, 3, 20);
    WH_Example whex3("Step3", 3, 1, 20);
    Step step1 (whex1);
    Step step2 (whex2);
    Step step3 (whex3);
    comp1.addStep (step1);
    comp1.addStep (step2);
    comp1.addStep (step3);
    step3.connectInput (&step2, TH_Mem(), false);
    step2.connectInput (&step1, TH_Mem(), false);
    Step* stepPtr = &step3;
    comp1.connectOutputToArray (&stepPtr, 1, 0, 0, TH_Mem(), false);
    doIt (comp1, whex.getName(), 5);
  }

  // In this example a WorkHolder with 2 output DataHolders is
  // connected to 2 WorkHolders with a single input and output DataHolder.
  // That is in its turn connected to a WorkHolder with 2 inputs.
  {
    WH_Example whex("Example3", 0);
    Composite comp1 (whex);
    comp1.runOnNode(0);
    // Now start filling the simulation. 
    WH_Example whex1("Step1", 0, 2);
    WH_Example whex2a("Step2a");
    WH_Example whex2b("Step2b");
    WH_Example whex3("Step3", 2, 1);
    Step step1 (whex1);
    Step step2a (whex2a);
    Step step2b (whex2b);
    Step step3 (whex3);
    comp1.addStep (step1);
    comp1.addStep (step2a);
    comp1.addStep (step2b);
    comp1.addStep (step3);
    Step* stepPtrs[2];
    stepPtrs[0] = &step2a;
    stepPtrs[1] = &step2b;
    step3.connectInputArray (stepPtrs, 2, TH_Mem(), false);
    step1.connectOutputArray (stepPtrs, 2, TH_Mem(), false);
    stepPtrs[0] = &step3;
    comp1.connectOutputToArray (stepPtrs, 1, 0, 0, TH_Mem(), false);
    doIt (comp1, whex.getName(), 5);
  }

  // Use composites.
  {
    WH_Example whex("Example4", 0);
    Composite comp1 (whex, "simul1");
    comp1.runOnNode(0);
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
    Composite composite1 (whcomp1, "simcomp1", false);
    composite1.addStep (step2);
    composite1.addStep (step3);
    step3.connectInput (&step2, TH_Mem(), false);
    Step* stepPtr = &step2;
    composite1.connectInputToArray (&stepPtr, 1, 0, 0, TH_Mem(), false);
    stepPtr = &step3;
    composite1.connectOutputToArray (&stepPtr, 1, 0, 0, TH_Mem(), false);
    // Create the second composite and fill it
    WH_Example whcomp2("Composite2");
    Composite composite2 (whcomp2, "simcomp2", false);
    composite2.addStep (composite1);
    composite2.addStep (step4);
    step4.connectInput (&composite1, TH_Mem(), false);
    stepPtr = &composite1;
    composite2.connectInputToArray (&stepPtr, 1, 0, 0, TH_Mem(), false);
    stepPtr = &step4;
    composite2.connectOutputToArray (&stepPtr, 1, 0, 0, TH_Mem(), false);
    // Finally create the total Simul.
    comp1.addStep (step1);
    comp1.addStep (composite2);
    composite2.connectInput (&step1, TH_Mem(), false);
    stepPtr = &composite2;
    comp1.connectOutputToArray (&stepPtr, 1, 0, 0, TH_Mem(), false);
    doIt (comp1, whex.getName(), 10);
  }

  // The same examples as above, but now connected by name.
  {
    // Define the top-level simul object.
    WH_Example whex("Example1", 0);
    Composite comp1 (whex);
    // Tell the Simul where to run.
    comp1.runOnNode(0);
    // Now start filling the simulation. 
    // First create the Steps.
    WH_Example whEx1("Step1", 0);
    Step step1 (whEx1);
    WH_Example whEx2("Step2");
    Step step2 (whEx2);
    WH_Example whEx3("Step3");
    Step step3 (whEx3);
    WH_Example whEx4("Step4");
    Step step4 (whEx4);
    comp1.addStep (step1);
    comp1.addStep (step2);
    comp1.addStep (step3);
    comp1.addStep (step4);
    comp1.connect ("aStep_0", "aStep_1", TH_Mem(), false);
    comp1.connect ("aStep_1", "aStep_2", TH_Mem(), false);
    comp1.connect ("aStep_2", "aStep_3", TH_Mem(), false);
    // Connect the output of the last step to the overall Simul WorkHolder.
    comp1.connect ("aStep_3", ".", TH_Mem(), false);
    // Run the simulation.
    doIt (comp1, whex.getName(), 10);
  }

  // A slightly more elaborate example.
  // Use WorkHolders with multiple data holders.
  // The number of input and output DataHolders still match,
  // so still a simple connect can be used.
  {
    WH_Example whex("Example2", 0, 1, 20);
    Composite comp1 (whex);
    comp1.runOnNode(0);
    // Now start filling the simulation. 
    WH_Example whex1("Step1", 0, 2, 20);
    WH_Example whex2("Step2", 2, 3, 20);
    WH_Example whex3("Step3", 3, 1, 20);
    Step step1 (whex1);
    Step step2 (whex2);
    Step step3 (whex3);
    comp1.addStep (step1);
    comp1.addStep (step2);
    comp1.addStep (step3);
    comp1.connect ("aStep_0", "aStep_1", TH_Mem(), false);
    comp1.connect ("aStep_1", "aStep_2", TH_Mem(), false);
    comp1.connect ("aStep_2", ".", TH_Mem(), false);
    doIt (comp1, whex.getName(), 5);
  }

  // In this example a WorkHolder with 2 output DataHolders is
  // connected to 2 WorkHolders with a single input and output DataHolder.
  // That is in its turn connected to a WorkHolder with 2 inputs.
  {
    WH_Example whex("Example3", 0);
    Composite comp1 (whex);
    comp1.runOnNode(0);
    // Now start filling the simulation. 
    WH_Example whex1("Step1", 0, 2);
    WH_Example whex2a("Step2a");
    WH_Example whex2b("Step2b");
    WH_Example whex3("Step3", 2, 1);
    Step step1 (whex1);
    Step step2a (whex2a);
    Step step2b (whex2b);
    Step step3 (whex3);
    comp1.addStep (step1);
    comp1.addStep (step2a);
    comp1.addStep (step2b);
    comp1.addStep (step3);
    comp1.connect ("aStep_0.out_0", "aStep_1", TH_Mem(), false);
    comp1.connect ("aStep_0.out_1", "aStep_2", TH_Mem(), false);
    comp1.connect ("aStep_1", "aStep_3.in_0", TH_Mem(), false);
    comp1.connect ("aStep_2", "aStep_3.in_1", TH_Mem(), false);
    comp1.connect ("aStep_3.out_0", ".out_0", TH_Mem(), false);
    doIt (comp1, whex.getName(), 5);
  }

  // Use Simul composites.
  {
    WH_Example whex("Example4", 0);
    Composite comp1 (whex, "comp1");
    comp1.runOnNode(0);
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
    Composite composite1 (whcomp1, "simcomp1", false);
    composite1.addStep (step2);
    composite1.addStep (step3);
    composite1.connect (".", "Step2", TH_Mem(), false);
    composite1.connect ("Step2", "Step3", TH_Mem(), false);
    composite1.connect ("Step3", ".", TH_Mem(), false);
    // Create the second composite and fill it
    WH_Example whcomp2("Composite2");
    Composite composite2 (whcomp2, "simcomp2", false);
    composite2.addStep (composite1);
    composite2.addStep (step4);
    composite2.connect (".", "simcomp1", TH_Mem(), false);
    composite2.connect ("simcomp1", "Step4", TH_Mem(), false);
    composite2.connect ("Step4", ".", TH_Mem(), false);
    // Finally create the total Simul.
    comp1.addStep (step1);
    comp1.addStep (composite2);
    comp1.connect ("Step1", "simcomp2", TH_Mem(), false);
    comp1.connect ("simcomp2", ".", TH_Mem(), false);
    doIt (comp1, whex.getName(), 10);
  }

}
