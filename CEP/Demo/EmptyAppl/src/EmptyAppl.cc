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

#include <stdio.h>
#include <Common/lofar_iostream.h>
#include <stdlib.h>
#include <Common/lofar_string.h>

#include <Common/Debug.h>
#include <CEPFrame/Step.h>
#include <CEPFrame/Composite.h>
#include <CEPFrame/WH_Empty.h>
#include <tinyCEP/Profiler.h>
#include <Transport/TH_ShMem.h>
#include <EmptyAppl/EmptyAppl.h>
#include <Common/KeyValueMap.h>


#include TRANSPORTERINCLUDE

#ifdef HAVE_CORBA
#include <CEPFrame/Corba/BS_Corba.h>
#include <CEPFrame/Corba/TH_Corba.h>
#endif

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

#ifdef HAVE_CORBA
  // Start Orb Environment
  AssertStr (BS_Corba::init(), "Could not initialise CORBA environment");
#endif

#ifdef HAVE_MPI
  // TH_ShMem only works in combination with MPI
  TH_ShMem::init(0, NULL);
#endif
  
  // Free any memory previously allocated
  undefine();

  // Create the top-level Simul
  Composite comp(new WH_Empty(), 
	        "EmptyAppl",
	        true, 
	        true,  // controllable	      
	        true); // monitor
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


  // Add all Step(s) to Simul
  // Example:    comp.addStep(sourceStep);
  //             comp.addStep(targetStep);


  // Create the cross connections between Steps
  // Example:     #ifdef HAVE_MPI
  //              targetStep.connect(sourceStep, 0, 0, 1, TH_MPI::proto);
  //              #endif


  // Optional: Performance optimisations
  // Example:     #ifdef HAVE_MPI
  //                simul.optimizeConnectionsWith(TH_ShMem::proto);
  //              #endif 

}
  

void EmptyAppl::run(int nSteps) {
  TRACER1("Call run()");
  Profiler::init();
  Step::clearEventCount();

  TRACER4("Start Processing simul");    
  for (int i=0; i<nSteps; i++) {
    if (i==2) Profiler::activate();
    TRACER2("Call simul.process() ");
    getComposite().process();
    if (i==5) Profiler::deActivate();
  }

  TRACER4("END OF SIMUL on node " << TRANSPORTER::getCurrentRank () );
 
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
