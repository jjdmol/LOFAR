//  Params.cc: An example simulator class to demonstrate parameter transport
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <Common/Debug.h>
#include "CEPFrame/Transport.h"
#include "CEPFrame/Step.h"
#include "CEPFrame/Simul.h"
#include "CEPFrame/WH_Empty.h"
#include "CEPFrame/Profiler.h"
#include "CEPFrame/ParamBlock.h"
#include "CEPFrame/ShMem/TH_ShMem.h"
#include "Params/Params.h"
#include "Params/WH_ParamListener.h"
#include "Params/WH_ParamPublisher.h"
#include "CEPFrame/TH_Mem_Bl.h"


#include TRANSPORTERINCLUDE

#ifdef HAVE_CORBA
#include "CEPFrame/Corba/BS_Corba.h"
#include "CEPFrame/Corba/TH_Corba.h"
#endif

Params::Params()
{
}

Params::~Params()
{
  undefine();
}

/**
   Define function for the Params simulation. It defines the steps that 
   process part of the data.
 */
void Params::define(const ParamBlock& params)
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
  Simul simul(new WH_Empty(), 
              "Params",
	      true, 
	      true,  // controllable	      
	      true); // monitor
  setSimul(simul);

  // Set node and application number of Simul
  simul.runOnNode(0,0);
  simul.setCurAppl(0);

   // Create the WorkHolder(s)
  WH_ParamPublisher WH1("publisher");
  WH_ParamListener WH2("listener1", WH_ParamListener::Latest);
  WH_ParamListener WH3("listener2", WH_ParamListener::New);


  // Create the Step(s)
  Step step1(WH1, "publisherStep", 1);
  Step step2(WH2, "listenerStep1", 1);
  Step step3(WH3, "listenerStep2", 1);


  // Determine the node and process for each step to run in
  step1.runOnNode(0, 0);
  step2.runOnNode(1, 0);
  step3.runOnNode(2, 0);


  // Add all Step(s) to Simul
  simul.addStep(step1);
  simul.addStep(step2);
  simul.addStep(step3);


  // Create the cross connections between Steps
#ifdef HAVE_MPI
  step1.connectParam("param1", &step2, TH_MPI::proto);  
  step1.connectParam("param1", &step3, TH_MPI::proto);
#else
  step1.connectParam("param1", &step2, TH_Mem_Bl::proto);  
  step1.connectParam("param1", &step3, TH_Mem_Bl::proto);
#endif

}
  

void Params::run(int nSteps) {
  TRACER1("Call run()");
  Profiler::init();
  Step::clearEventCount();

#ifdef HAVE_MPI
  TH_MPI::synchroniseAllProcesses();
#endif

  TRACER4("Start Processing simul");    
  for (int i=0; i<nSteps; i++) {
    if (i==2) Profiler::activate();
    TRACER2("Call simul.process() ");
    getSimul().process();
    if (i==5) Profiler::deActivate();
  }

  TRACER4("END OF SIMUL on node " << TRANSPORTER::getCurrentRank () );
 
#if 0
  //     close environment
  TRANSPORTER::finalize();
#endif

}

void Params::dump() const {
  getSimul().dump();
}

void Params::quit() {  
}

void Params::undefine() {
  // Clean up
}
