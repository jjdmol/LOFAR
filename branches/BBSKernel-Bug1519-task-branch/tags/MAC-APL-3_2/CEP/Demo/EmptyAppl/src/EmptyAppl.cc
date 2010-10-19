//  EmptyAppl.cc: An empty simulator class
//
//  Copyright (C) 2000, 2002
//  ASTRON (Netherlands Foundation for Research in Astronomy)
//  P.O.Box 2, 7990 AA Dingeloo, The Netherlands, seg@astron.nl
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

//#include <stdio.h>
#include <Common/lofar_iostream.h>
//#include <stdlib.h>
#include <Common/lofar_string.h>

#include <CEPFrame/Step.h>
#include <CEPFrame/Composite.h>
#include <tinyCEP/Profiler.h>
#include <Transport/TH_ShMem.h>
#include <EmptyAppl/EmptyAppl.h>
#include <Common/KeyValueMap.h>
#include <Common/LofarLogger.h>

#include TRANSPORTERINCLUDE


using namespace LOFAR;

EmptyAppl::EmptyAppl()
{
}

EmptyAppl::~EmptyAppl()
{
  undefine();
}

/**
   Define function for the EmptyAppl simulation. It defines the steps that 
   process part of the data.
 */
void EmptyAppl::define(const KeyValueMap&)
{

#ifdef HAVE_MPI
  // TH_ShMem only works in combination with MPI
  TH_MPI::initMPI(0, NULL);
#endif
  
  // Free any memory previously allocated
  undefine();

  // Create the top-level Composite
  Composite comp(0, 0, "EmptyAppl");
  setComposite(comp);

  // Set node and application number of Composite
  comp.runOnNode(0,0);
  comp.setCurAppl(0);

  // Optional: Get any extra params from input
  // Example:    noOfSourceSteps = params.getInt("sources",1);  

 
  // Create the WorkHolder(s)
  // Example:    WH_Empty sourceWH();
  //             WH_Empty targetWH();


  // Create the Step(s)
  // Example:    Step sourceStep(sourceWH, "sourceStep", 1);
  //             Step targetStep(targetWH, "targetStep", 1);


  // Determine the node and process for each step to run in
  // Example:    sourceStep.runOnNode(0, 0);
  //             targetStep.runOnNode(1, 0);


  // Add all Step(s) to Composite
  // Example:    comp.addBlock(sourceStep);
  //             comp.addBlock(targetStep);


  // Create the cross connections between Steps
  // Example:     #ifdef HAVE_MPI
  //              targetStep.connect(sourceStep, 0, 0, 1, new TH_MPI(0,1));
  //              #endif

}
  

void EmptyAppl::run(int nSteps) {
  LOG_TRACE_FLOW("Call run()");
  Profiler::init();
  Step::clearEventCount();

  LOG_TRACE_FLOW("Start Processing simul");    
  for (int i=0; i<nSteps; i++) {
    if (i==2) Profiler::activate();
    LOG_TRACE_FLOW("Call simul.process() ");
    getComposite().process();
    if (i==5) Profiler::deActivate();
  }

  LOG_TRACE_FLOW_STR("END OF SIMUL on node " 
		     << TRANSPORTER::getCurrentRank () );
 
#if 0
  //     close environment
  TRANSPORTER::finalize();
#endif

}

void EmptyAppl::dump() const {
  getComposite().dump();
}

void EmptyAppl::quit() {  
}

void EmptyAppl::undefine() {
  // Clean up
}
