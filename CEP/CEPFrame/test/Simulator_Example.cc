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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "Simulator_Example.h"
#include "CEPFrame/SimulatorParseClass.h"
#include "CEPFrame/ParamBlock.h"
#include "CEPFrame/Transport.h"
#include "CEPFrame/Step.h"
#include "CEPFrame/BaseSim.h"
#include "CEPFrame/Simul.h"
#include "CEPFrame/WH_Example.h"
#include "CEPFrame/Profiler.h"
#include "Common/Debug.h"
#include <Common/lofar_iostream.h>


Simulator_Example::~Simulator_Example()
{}

void Simulator_Example::define (const ParamBlock& params)
{
  params.show (cout);
  int rank = TRANSPORTER::getCurrentRank ();
  unsigned int size = TRANSPORTER::getNumberOfNodes();
  int appl = Step::getCurAppl ();
  cout << "CEPFrame Processor " << rank << " of " << size
       << " operational  (appl=" << appl << ')' << endl;

  // define the top-level simul object
  Simul simul(WH_Example("ExampleSimul"));
  setSimul (simul);

  // tell the Simul where to run
  simul.runOnNode(0);
  
  // Now start filling the simulation. 
  // first create the Steps
  Step step1(WH_Example("Step1"));
  Step step2(WH_Example("Step2"));
  Step step3(WH_Example("Step3"));

  // Create the first composite and fill it
  Simul composite1(WH_Example("Composite1"), "simcomp1", false);
  composite1.addStep(step2);
  composite1.addStep(step3);

  // Create the second composite and fill it
  Simul composite2(WH_Example("Composite2"), "simcomp2", false);
  composite2.addStep(step1);
  composite2.addStep(composite1);

  // Add the Composite2 to the top-level Simul
  simul.addStep(composite2);

  // Now define the connections between the Steps and Simuls objects:
  step3.connectInput(&step2);
  Step* addr2 = &composite2;
  simul.connectInputToArray(&addr2,1);

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
  TRACER2("Ready with definition of configuration");
  Profiler::init();

  cout << endl <<  "Start Process" << endl;    
  for (int i=0; i<nsteps; i++) {
    if (i==2) Profiler::activate();
    getSimul().process();
    if (i==5) Profiler::deActivate();
  }
}

void Simulator_Example::dump() const
{

  cout << endl << "DUMP Data from last Processing step: " << endl;
  getSimul().dump();
}

void Simulator_Example::quit()
{

  cout << endl << "END OF SIMUL on node " 
       << TRANSPORTER::getCurrentRank() 
       << endl;
}



int main (int argc, const char* argv[])
{
  // Set trace level.
  Debug::initLevels (argc, argv);
  try {
    // First test some Parse functions.
    // Note that C++ also requires a \ to escape special characters.
    AssertStr (SimulatorParse::removeQuotes ("'abcd\"e'\"fgh'i\"'j'")
	       == string("abcd\"efgh'ij"),
	       "SimulatorParse::removeQuotes fails");
    AssertStr (SimulatorParse::removeEscapes ("\\ab\\\\cd\\efghij\\")
	       == string("ab\\cdefghij"),
	       "SimulatorParse::removeEscapes fails");

    Simulator_Example simulator;
    simulator.setarg (argc, argv);
#ifndef HAVE_MPI
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
#else
    cout << "Welcome to LOFARSim" <<endl;
    cout << "Running in batch mode " << endl;
    cout << endl;
    cout << "Call define" << endl;
    simulator.baseDefine();
    cout << endl;
    cout << "Call run" << endl;
    simulator.baseRun();
    cout << endl;
    cout << "Call dump " << endl;
    simulator.baseDump();
    cout << "Call quit " << endl;
    simulator.baseQuit();
    cout << endl;
    cout << "Good Bye!" << endl;
#endif
  } catch (...) {
    cout << "Unexpected exception";
  }
}
