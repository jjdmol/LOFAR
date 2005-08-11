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

#include <lofar_config.h>

#include <stdio.h>
#include <Common/lofar_iostream.h>
#include <stdlib.h>
#include <Common/lofar_string.h>

#include <Common/LofarLogger.h>
#include <CEPFrame/Step.h>
#include <CEPFrame/Composite.h>
#include <ExampleSim/ExampleSim.h>
#include <ExampleSim/WH_Source.h>
#include <ExampleSim/WH_Multiply.h>
#include <ExampleSim/WH_Add.h>
#include <Transport/TH_Mem.h>

#ifdef HAVE_MPI
#include <Transport/TH_MPI.h>
#endif

using namespace LOFAR;

ExampleSim::ExampleSim()
{
}

ExampleSim::~ExampleSim()
{
  undefine();
}

/**
   Define function for the ExampleSim application. It defines the steps that 
   process part of the data.
 */
void ExampleSim::define(const KeyValueMap& params)
{
  // Free any memory previously allocated
  undefine();

  // Create the top-level Composite
  Composite topComposite(0, 0, "ExampleSim");
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
  Composite composite(2, 1, "calcComposite");
  composite.addBlock(multStep);
  composite.addBlock(addStep);
  // and make the internal connections
  composite.setInput(0, &multStep, 0, -1);
  addStep.connectInput(&multStep, new TH_Mem(), false);
  composite.setOutput(0, &addStep, 0, -1);

  // Determine the node for each step to run on
  sourceStep.runOnNode(0);
  composite.runOnNode(1);

  // Add all Step(s) to Composite
  topComposite.addBlock(sourceStep);
  topComposite.addBlock(composite);

  // Create the connections between Steps
#ifdef HAVE_MPI
  composite.connectInput(&sourceStep, new TH_MPI(0,1), false);
#else
  composite.connectInput(&sourceStep, new TH_Mem(), false);
#endif

}
  

void ExampleSim::run(int nSteps) {
  LOG_TRACE_FLOW("Call run()");
  Step::clearEventCount();

  LOG_TRACE_FLOW("Start Processing top level composite");    
  for (int i=0; i<nSteps; i++) {
    LOG_TRACE_FLOW("Calling top level composite process() ");
    getComposite().process();
  }

  LOG_TRACE_FLOW_STR("END OF COMPOSITE on node " 
		     << TRANSPORTER::getCurrentRank () );
 
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
