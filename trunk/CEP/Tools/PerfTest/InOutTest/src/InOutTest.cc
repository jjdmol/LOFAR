//  InOutTest.cc: Concrete Simulator class for performance measurements on
//            
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <Common/Debug.h>
#include "CEPFrame/Transport.h"
#include "CEPFrame/Step.h"
#include "CEPFrame/Simul.h"
#include "CEPFrame/Profiler.h"
#include "CEPFrame/WH_Empty.h"
#include "CEPFrame/ParamBlock.h"
#include "CEPFrame/ShMem/TH_ShMem.h"
#include "InOutTest/InOutTest.h"
#include "InOutTest/WH_Source.h"
#include "InOutTest/WH_InOut.h"
#include "InOutTest/WH_Sink.h"

#include TRANSPORTERINCLUDE

#ifdef HAVE_CORBA
#include "CEPFrame/Corba/BS_Corba.h"
#include "CEPFrame/Corba/TH_Corba.h"
#endif

using namespace LOFAR;

InOutTest::InOutTest()
{
}

InOutTest::~InOutTest()
{
  undefine();
}

/**
   define function for the P2Perf simulation. It defines a list
   of steps that each process a part of the data.
 */
void InOutTest::define(const ParamBlock& params)
{
#ifdef HAVE_CORBA
  // Start Orb Environment
  AssertStr (BS_Corba::init(), "Could not initialise CORBA environment");
#endif

#ifdef HAVE_MPI
  // TH_ShMem only works in combination with MPI
  // initialize TH_ShMem
  int useShMem = params.getInt("shmem",1);
  if (useShMem) TH_ShMem::init(0, NULL);
#endif
  
  int rank = TRANSPORTER::getCurrentRank();
  unsigned int size = TRANSPORTER::getNumberOfNodes();

  // free any memory previously allocated
  undefine();

  TRACER2("InOutTest Processor " << rank << " of " << size << " operational.");

  WH_Empty empty;

  Simul simul(empty, "InOutTest", true);
  setSimul(simul);
  simul.runOnNode(0);
  simul.setCurAppl(0);

  itsFixedSize   = params.getInt("fixedsize",0);
  itsNPerf       = params.getInt("numberperf",0);  //Number of iterations over
                                                   //which performance is 
                                                   //measured
  itsIOshared    = params.getInt("shareIO",0);     //Share input & output in
                                                   //WH_InOut Default no sharing
  // Create the Source Step
  WH_Source SourceWH("Source", (itsFixedSize?itsFixedSize:MAX_GROW_SIZE));
    
  Step SourceSt(SourceWH, "SourceStep", 0);
  SourceSt.runOnNode(0, 0); // run in App 0

  WH_InOut InOutWH("InOut", itsIOshared, 
                   (itsFixedSize?itsFixedSize:MAX_GROW_SIZE));
  Step WorkSt(InOutWH, "InOutStep", 0);
  WorkSt.runOnNode(1, 0);
    
  // Create the Destination Step
  WH_Sink DestWH("Dest", false, (itsFixedSize?itsFixedSize:MAX_GROW_SIZE),
		 itsNPerf);  
  Step DestSt(DestWH, "DestStep", 0);
  DestSt.runOnNode(1,0); // run in App 0

  // Now Add the steps to the simul;

  TRACER4("Add steps");
  simul.addStep(SourceSt);
  simul.addStep(WorkSt);
  simul.addStep(DestSt);
   
  // Create the cross connections

#ifdef HAVE_MPI
  TRACER2("Connect using MPI");
  DestSt.connect(&WorkSt,0,0,1,TH_MPI::proto);
  WorkSt.connect(&SourceSt,0,0,1,TH_MPI::proto);
#else
  DestSt.connect(&WorkSt,0,0,1,TH_Mem::proto);
  WorkSt.connect(&SourceSt,0,0,1,TH_Mem::proto);
#endif

  
#ifdef HAVE_MPI
  // TH_ShMem only works in combination with MPI
  //     if (useShMem) simul.optimizeConnectionsWith(TH_ShMem::proto);
#endif
}

void doIt (Simul& simul, const std::string& name, int nsteps) {
#if 0
  simul.resolveComm();
#endif
  TRACER1("Ready with definition of configuration");
  Profiler::init();
  Step::clearEventCount();

  TRACER4("Start Processing simul " << name);    
  for (int i=0; i<nsteps; i++) {
    if (i==2) Profiler::activate();
    TRACER2("Call simul.process() ");
    simul.process();
    if (i==5) Profiler::deActivate();
  }

  TRACER4("END OF SIMUL on node " << TRANSPORTER::getCurrentRank () );
 
#if 0
  //     close environment
  TRANSPORTER::finalize();
#endif
}

void InOutTest::run(int nSteps) {
  nSteps = nSteps;
  doIt(getSimul(), "InOutTest Simulator", nSteps);

}

void InOutTest::dump() const {
  getSimul().dump();
}

void InOutTest::quit() {  
}

void InOutTest::undefine() {
}
