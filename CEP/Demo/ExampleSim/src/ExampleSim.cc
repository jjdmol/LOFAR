//  ExampleSim.cc: An example simulator class
//
//  Copyright (C) 2000, 2002
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
//
//////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <Common/lofar_iostream.h>
#include <stdlib.h>
#include <Common/lofar_string.h>

#include <Common/Debug.h>
#include <CEPFrame/Step.h>
#include <CEPFrame/Composite.h>
#include <CEPFrame/WH_Empty.h>
#include <ExampleSim/ExampleSim.h>
#include <ExampleSim/WH_Source.h>
#include <ExampleSim/WH_Multiply.h>
#include <ExampleSim/WH_Add.h>

#include TRANSPORTERINCLUDE

using namespace LOFAR;

ExampleSim::ExampleSim()
{
}

ExampleSim::~ExampleSim()
{
  undefine();
}

/**
   Define function for the ExampleSim simulation. It defines the steps that 
   process part of the data.
 */
void ExampleSim::define(const KeyValueMap& params)
{
  // Free any memory previously allocated
  undefine();

  // Create the top-level Composite
  Composite topComposite(new WH_Empty(), "ExampleSim");
  setComposite(topComposite);

  // Set node and application number of Composite
  topComposite.runOnNode(0);
  topComposite.setCurAppl(0);

  // Get correction factor argument from input
  int corrFactor = params.getInt("corrfactor", 0);
 
  // Create the WorkHolder(s)
  WH_Source sourceWH("Source", 2);
  WH_Multiply multWH("Multiplier", 2);
  WH_Add addWH("Adder", corrFactor);

  // Create the Step(s)
  Step sourceStep(sourceWH, "sourceStep");
  Step multStep(multWH, "multiplierStep");
  Step addStep(addWH, "adderStep");

  // Create a composite Step
  WH_Multiply WHcomp("Composite", 2);
  Composite composite(WHcomp, "calcComposite");
  composite.addStep(multStep);
  composite.addStep(addStep);
  // and make the internal connections
  Step* stepPtr = &multStep;
  composite.connectInputToArray(&stepPtr, 1, 0, 0, TH_Mem(), false);
  addStep.connectInput(&multStep, TH_Mem(), false);
  stepPtr = &addStep;
  composite.connectOutputToArray(&stepPtr, 1, 0, 0, TH_Mem(), false);

  // Determine the node for each step to run on
  sourceStep.runOnNode(0);
  composite.runOnNode(1);

  // Add all Step(s) to Composite
  topComposite.addStep(sourceStep);
  topComposite.addStep(composite);

  // Create the connections between Steps
  composite.connectInput(&sourceStep, TH_Mem(), false);


}
  

void ExampleSim::run(int nSteps) {
  TRACER1("Call run()");
  Step::clearEventCount();

  TRACER4("Start Processing simul");    
  for (int i=0; i<nSteps; i++) {
    TRACER2("Call simul.process() ");
    getComposite().process();
  }

  TRACER4("END OF SIMUL on node " << TRANSPORTER::getCurrentRank () );
 
#if 0
  //     close environment
  TRANSPORTER::finalize();
#endif

}

void ExampleSim::dump() const {
  getComposite().dump();
}

void ExampleSim::quit() {  
}

void ExampleSim::undefine() {
  // Clean up
}
