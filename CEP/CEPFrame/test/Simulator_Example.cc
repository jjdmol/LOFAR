//  Simulator_Example.cc: Program for testing simulation parser
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
/////////////////////////////////////////////////////////////////////////

#include <lofar_config.h>

#include <Simulator_Example.h>
#include <tinyCEP/SimulatorParseClass.h>
#include <CEPFrame/Step.h>
#include <Transport/TH_Mem.h>
#include <CEPFrame/Composite.h>
#include <CEPFrame/WH_Example.h>
#include <tinyCEP/Profiler.h>
#include <Common/KeyValueMap.h>
#include <Common/LofarLogger.h>
#include <Common/lofar_iostream.h>

using namespace LOFAR;

Simulator_Example::~Simulator_Example()
{}

void Simulator_Example::define (const KeyValueMap& params)
{
  params.show (cout);

  // define the top-level composite object
  WH_Example exSimul("ExampleSimul");
  Composite comp(exSimul);
  setComposite (comp);

  // tell the Composite where to run
  comp.runOnNode(0);
  
  // Now start filling the simulation. 
  // first create the Steps
  WH_Example whEx1("Step1");
  Step step1(whEx1);
  WH_Example whEx2("Step2");
  Step step2(whEx2);
  WH_Example whEx3("Step3");
  Step step3(whEx3);

  // Create the first composite and fill it
  WH_Example whComp1("Composite1");
  Composite composite1(whComp1, "simcomp1", false);
  composite1.addStep(step2);
  composite1.addStep(step3);

  // Create the second composite and fill it
  WH_Example whComp2("Composite2");
  Composite composite2(whComp2, "simcomp2", false);
  composite2.addStep(step1);
  composite2.addStep(composite1);

  // Add the Composite2 to the top-level Composite
  comp.addStep(composite2);

  // Now define the connections between the Steps and Composite objects:
  step3.connectInput(&step2,TH_Mem(), false);
  Step* addr2 = &composite2;
  comp.connectInputToArray(&addr2,1,0,0,TH_Mem(),false);

  //////////////////////////////////////////////////////////////////////
  //
  // Finished configuration definition
  // 
  //////////////////////////////////////////////////////////////////////
}

void Simulator_Example::run (int nsteps)
{
  if (nsteps < 0) {
    nsteps = 10;
  }
  LOG_TRACE_FLOW("Ready with definition of configuration");
  Profiler::init();

  cout << endl <<  "Start Process" << endl;    
  for (int i=0; i<nsteps; i++) {
    if (i==2) Profiler::activate();
    getComposite().process();
    if (i==5) Profiler::deActivate();
  }
}

void Simulator_Example::dump() const
{

  cout << endl << "DUMP Data from last Processing step: " << endl;
  getComposite().dump();
}

void Simulator_Example::quit()
{

}



int main (int argc, const char* argv[])
{
  // Set trace level.
  INIT_LOGGER("SimulatorExample.log_prop");
  try {
    // First test some Parse functions.
    // Note that C++ also requires a \ to escape special characters.
    ASSERTSTR (SimulatorParse::removeQuotes ("'abcd\"e'\"fgh'i\"'j'")
	       == string("abcd\"efgh'ij"),
	       "SimulatorParse::removeQuotes fails");
    ASSERTSTR (SimulatorParse::removeEscapes ("\\ab\\\\cd\\efghij\\")
	       == string("ab\\cdefghij"),
	       "SimulatorParse::removeEscapes fails");

    Simulator_Example simulator;
    simulator.setarg (argc, argv);
// #ifndef HAVE_MPI
    cout << endl;
    cout << "  * Type 'define;' to define the simulation" << endl;
    cout << "  * Type 'run;'    to run the simulation" << endl;
    cout << "  * Type 'dump;'   to dump the simulator data" << endl;
    cout << "  * Type 'quit'    to quit" << endl;
    cout << endl;
    try {
      SimulatorParse::parse (simulator);
    } catch (SimulatorParseError& x) {
      cout << x.what() << endl;
    }
    cout << endl;
    cout << "It was a pleasure working with you!" << endl << endl;
// #else
//     cout << "Welcome to LOFARSim" <<endl;
//     cout << "Running in batch mode " << endl;
//     cout << endl;
//     cout << "Call define" << endl;
//     simulator.baseDefine();
//     cout << endl;
//     cout << "Call run" << endl;
//     simulator.baseRun();
//     cout << endl;
//     cout << "Call dump " << endl;
//     simulator.baseDump();
//     cout << "Call quit " << endl;
//     simulator.baseQuit();
//     cout << endl;
//     cout << "Good Bye!" << endl;
// #endif
  } catch (...) {
    cout << "Unexpected exception";
  }
}
