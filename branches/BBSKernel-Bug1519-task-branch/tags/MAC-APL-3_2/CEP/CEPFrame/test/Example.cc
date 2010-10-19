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

#include <lofar_config.h>

#include <Common/lofar_iostream.h>
#include <CEPFrame/Step.h>
#include <CEPFrame/Composite.h>
#include <Transport/TH_Mem.h>
#include <CEPFrame/WH_Example.h>
#include <tinyCEP/Profiler.h>
#include <Common/LofarLogger.h>

#ifdef HAVE_MPI
#include <Transport/TH_MPI.h>
#endif

using namespace LOFAR;

void doIt (Composite& comp, const std::string& name, int nsteps)
{
  LOG_TRACE_FLOW("Ready with definition of configuration");
  Profiler::init();
  Step::clearEventCount();

  try {
    comp.preprocess();

    cout << endl << "Start Processing composite " << name << endl;    
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
#ifdef HAVE_MPI
  TH_MPI::initMPI(argc, argv);
#endif
  // Set trace level.
  INIT_LOGGER("Example.log_prop");

  // A simple example. Four steps connected to each other.
  // Each WorkHolder has a single DataHolder.
  {
    // Define the top-level composite object.
    Composite comp1(0,0);
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
    comp1.addBlock (step1);
    comp1.addBlock (step2);
    comp1.addBlock (step3);
    comp1.addBlock (step4);
    step4.connectInput (&step3, new TH_Mem(), false);
    step3.connectInput (&step2, new TH_Mem(), false );
    step2.connectInput (&step1, new TH_Mem(), false);
    // Run the simulation.
    doIt (comp1, "Example1", 10);
  }

  // A slightly more elaborate example.
  // Use WorkHolders with multiple data holders.
  // The number of input and output DataHolders still match,
  // so still a simple connect can be used.
  {
    Composite comp1(0, 0, "Example2");
    comp1.runOnNode(0);
    // Now start filling the simulation. 
    WH_Example whex1("Step1", 0, 2, 20);
    WH_Example whex2("Step2", 2, 3, 20);
    WH_Example whex3("Step3", 3, 1, 20);
    Step step1 (whex1);
    Step step2 (whex2);
    Step step3 (whex3);
    comp1.addBlock (step1);
    comp1.addBlock (step2);
    comp1.addBlock (step3);
    step3.connectInput (&step2, new TH_Mem(), false);
    step2.connectInput (&step1, new TH_Mem(), false);
    doIt (comp1, "Example2", 5);
  }

  // In this example a WorkHolder with 2 output DataHolders is
  // connected to 2 WorkHolders with a single input and output DataHolder.
  // That is in its turn connected to a WorkHolder with 2 inputs.
  {
    Composite comp1 (0, 0, "Example3");
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
    comp1.addBlock (step1);
    comp1.addBlock (step2a);
    comp1.addBlock (step2b);
    comp1.addBlock (step3);
    Block* blockPtrs[2];
    blockPtrs[0] = &step2a;
    blockPtrs[1] = &step2b;
    step3.connectInputArray (blockPtrs, 2, new TH_Mem(), false);
    step1.connectOutputArray (blockPtrs, 2, new TH_Mem(), false);
    doIt (comp1, comp1.getName(), 5);
  }

  // Use composites.
  {
    WH_Example whex("Example4", 0);
    Composite comp1 (0, 0, "simul1");
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
    Composite composite1 (1, 1, "simcomp1", false);
    composite1.addBlock (step2);
    composite1.addBlock (step3);
    step3.connectInput (&step2, new TH_Mem(), false);
    composite1.setInput (0, &step2, 0, -1);
    composite1.setOutput (0, &step3, 0, -1);
    // Create the second composite and fill it
    WH_Example whcomp2("Composite2");
    Composite composite2 (1, 1, "simcomp2", false);
    composite2.addBlock (composite1);
    composite2.addBlock (step4);
    step4.connectInput (&composite1, new TH_Mem(), false);
    composite2.setInput (0, &composite1, 0, -1);
    composite2.setOutput (0, &step4, 0, -1);
    // Finally create the total Simul.
    comp1.addBlock (step1);
    comp1.addBlock (composite2);
    composite2.connectInput (&step1, new TH_Mem(), false);
    doIt (comp1, whex.getName(), 10);
  }

}
