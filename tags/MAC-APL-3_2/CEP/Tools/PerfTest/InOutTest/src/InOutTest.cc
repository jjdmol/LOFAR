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

#include <lofar_config.h>

//#include <stdio.h>
#include <Common/lofar_iostream.h>
//#include <stdlib.h>
#include <Common/lofar_string.h>

//#include <Common/LofarLogger.h>
#include <CEPFrame/Step.h>
#include <CEPFrame/Composite.h>
#include <tinyCEP/Profiler.h>
#include <Transport/TH_ShMem.h>
#include <InOutTest/InOutTest.h>
#include <InOutTest/WH_Source.h>
#include <InOutTest/WH_InOut.h>
#include <InOutTest/WH_Sink.h>

#include TRANSPORTERINCLUDE

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
void InOutTest::define(const KeyValueMap& params)
{

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

  LOG_TRACE_FLOW_STR("InOutTest Processor " << rank << " of " << size << " operational.");

  Composite comp(0, 0, "InOutTest");
  setComposite(comp);
  comp.runOnNode(0);
  comp.setCurAppl(0);

  itsFixedSize   = params.getInt("fixedsize",1028);
  itsNPerf       = params.getInt("numberperf",100);  //Number of iterations over
                                                   //which performance is 
                                                   //measured
  itsIOshared    = params.getInt("shareIO",1);     //Share input & output in
                                                   //WH_InOut Default no sharing
  // Create the Source Step
  WH_Source SourceWH("Source", (itsFixedSize?itsFixedSize:MAX_GROW_SIZE));
    
  Step SourceSt(SourceWH, "SourceStep", 0);
  SourceSt.runOnNode(0, 0); // run in App 0

  WH_InOut InOutWH("InOut", (itsFixedSize?itsFixedSize:MAX_GROW_SIZE));
  Step WorkSt(InOutWH, "InOutStep", 0);
  WorkSt.runOnNode(1, 0);
    
  WorkSt.setInBufferingProperties(0, true, itsIOshared); // Share in/output?

  // Create the Destination Step
  WH_Sink DestWH("Dest", false, (itsFixedSize?itsFixedSize:MAX_GROW_SIZE),
		 itsNPerf);  
  Step DestSt(DestWH, "DestStep", 0);
  DestSt.runOnNode(1,0); // run in App 0

  // Now Add the steps to the simul;

  LOG_TRACE_STAT("Add steps");
  comp.addBlock(SourceSt);
  comp.addBlock(WorkSt);
  comp.addBlock(DestSt);
   
  // Create the cross connections

#ifdef HAVE_MPI
  LOG_TRACE_FLOW("Connect using MPI");
  DestSt.connect(0, &WorkSt, 0, 1, new TH_MPI(1,0));
  WorkSt.connect(0, &SourceSt, 0, 1, new TH_MPI(1,0));
#else
    DestSt.connect(0, &WorkSt, 0, 1, new TH_Mem(), false);
    WorkSt.connect(0, &SourceSt, 0, 1, new TH_Mem(), false);
#endif

  
#ifdef HAVE_MPI
  // TH_ShMem only works in combination with MPI
  //     if (useShMem) simul.optimizeConnectionsWith(TH_ShMem::proto);
#endif
}

void doIt (Composite& comp, const std::string& name, int nsteps) {
#if 0
  comp.resolveComm();
#endif
  LOG_TRACE_FLOW("Ready with definition of configuration");
  Profiler::init();
  Block::clearEventCount();

  LOG_TRACE_FLOW_STR("Start Processing simul " << name);    
  for (int i=0; i<nsteps; i++) {
    if (i==2) Profiler::activate();
    LOG_TRACE_FLOW("Call simul.process() ");
    comp.process();
    if (i==5) Profiler::deActivate();
  }

  LOG_TRACE_FLOW_STR("END OF SIMUL on node " << TRANSPORTER::getCurrentRank () );
 
#if 0
  //     close environment
  TRANSPORTER::finalize();
#endif
}

void InOutTest::run(int nSteps) {
  nSteps = nSteps;
  doIt(getComposite(), "InOutTest Simulator", nSteps);

}

void InOutTest::dump() const {
  getComposite().dump();
}

void InOutTest::quit() {  
}

void InOutTest::undefine() {
}
