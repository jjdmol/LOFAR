//  P2Perf.cc: Concrete Simulator class for performance measurements on
//            a sequence of cross-connected steps
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
#include "CEPFrame/Profiler.h"
#include "CEPFrame/WH_Empty.h"
#include "CEPFrame/ParamBlock.h"
#include "CEPFrame/ShMem/TH_ShMem.h"
#include "P2Perf/P2Perf.h"
#include "P2Perf/WH_GrowSize.h"

#include TRANSPORTERINCLUDE

#ifdef HAVE_CORBA
#include "CEPFrame/Corba/BS_Corba.h"
#include "CEPFrame/Corba/TH_Corba.h"
#endif

P2Perf::P2Perf():
  itsSourceSteps(0),
  itsDestSteps(0)
{
  Sworkholders = NULL;
  Ssteps       = NULL;
  Dworkholders = NULL;
  Dsteps       = NULL;
}

P2Perf::~P2Perf()
{
  undefine();
}

/**
   define function for the P2Perf simulation. It defines a list
   of steps that each process a part of the data.
 */
void P2Perf::define(const ParamBlock& params)
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
  
  char name[20];  // name used during Step/WH creation
  
  int rank = TRANSPORTER::getCurrentRank();
  unsigned int size = TRANSPORTER::getNumberOfNodes();

  // free any memory previously allocated
  undefine();

  TRACER2("P2Perf Processor " << rank << " of " << size << " operational.");

  WH_Empty empty;

  Simul simul(empty, 
	      "P2Perf",
	      true, 
	      true,  // controllable	      
	      true); // monitor
  setSimul(simul);
  simul.runOnNode(0);
  simul.setCurAppl(0);

  itsSourceSteps = params.getInt("sources",1);  
  itsDestSteps   = params.getInt("destinations",1);
  itsFixedSize   = params.getInt("fixedsize",0);

  bool  WithMPI=false;

#ifdef HAVE_MPI    
  WithMPI = true;
#endif

  // Create the Workholders and Steps
  Sworkholders = new (WH_GrowSize*)[itsSourceSteps];
  Ssteps       = new (Step*)[itsSourceSteps];
  Dworkholders = new (WH_GrowSize*)[itsDestSteps];
  Dsteps       = new (Step*)[itsDestSteps];
  
  
  // now go and create the source and destination steps
  // the two loops do duplicate quite some code, so a private method  
  // should be made later...

  // Create the Source Steps
  bool monitor;
  for (int iStep = 0; iStep < itsSourceSteps; iStep++) {
    
    // Create the Source Step
    sprintf(name, "GrowSizeSource[%d]", iStep);
    Sworkholders[iStep] = new WH_GrowSize(name, 
				  false, 
				  1, // should be 0 
				  itsDestSteps,
				  (itsFixedSize?itsFixedSize:MAX_GROW_SIZE),
				  false, // flag for source side
				  itsFixedSize!=0);
    
      monitor = (iStep==0) ? true:false;
      Ssteps[iStep] = new Step(Sworkholders[iStep], 
			       "GrowSizeSourceStep", 
			       iStep,
			       monitor);
      
      // Determine the node and process to run in
      Ssteps[iStep]->runOnNode(iStep  ,0); // run in App 0
  }
  
  // Report performance of the first source Step
  ((WH_GrowSize*)Ssteps[0]->getWorker())->setReportPerformance(true);
  
  // Create the destination steps
  for (int iStep = 0; iStep < itsDestSteps; iStep++) {
    
    // Create the Destination Step
    sprintf(name, "GrowSizeDest[%d]", iStep);
    Dworkholders[iStep] = new WH_GrowSize(name, 
				  false, 
				  itsSourceSteps, 
				  1, // should be 0 
				  (itsFixedSize?itsFixedSize:MAX_GROW_SIZE),
				  true,
				  itsFixedSize!=0);
    
    Dsteps[iStep] = new Step(Dworkholders[iStep], "GrowSizeDestStep", iStep);
    // Determine the node and process to run in
    if (WithMPI) {
      TRACER2("Dest MPI runonnode (" << iStep << ")");
      Dsteps[iStep]->runOnNode(iStep+itsSourceSteps,0); // run in App 0
    } else if (params.getInt("destside",0) != 0) {
      simul.setCurAppl(1);
      Dsteps[iStep]->runOnNode(iStep+1,1); // run in App 1
    } else {
      Dsteps[iStep]->runOnNode(iStep+1,0); // run in App 0
    }
  }

  // Now Add the steps to the simul;
  // first ALL the sources....
  for (int iStep = 0; iStep < itsSourceSteps; iStep++) {
    TRACER4("Add Source step " << iStep);
    simul.addStep(Ssteps[iStep]);
  }
  // ...then the destinations
  for (int iStep = 0; iStep < itsDestSteps; iStep++) {
    TRACER4("Add Dest step " << iStep);
    simul.addStep(Dsteps[iStep]);
  }
  
  // Create the cross connections
  for (int step = 0; step < itsDestSteps; step++) {
    for (int ch = 0; ch < itsSourceSteps; ch++) {
      // Set up the connections
      // Correlator Style
#ifdef HAVE_CORBA
      Dsteps[step]->connect(Ssteps[ch],ch,step,1,TH_Corba::proto);
#else
#ifdef HAVE_MPI
      TRACER2("Connect using MPI");
      Dsteps[step]->connect(Ssteps[ch],ch,step,1,TH_MPI::proto);
#else
      Dsteps[step]->connect(Ssteps[ch],ch,step,1,TH_Mem::proto);
#endif
#endif
    }
  }
  
#ifdef HAVE_MPI
  // TH_ShMem only works in combination with MPI
  if (useShMem) simul.optimizeConnectionsWith(TH_ShMem::proto);
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

void P2Perf::run(int nSteps) {
  nSteps = nSteps;
  doIt(getSimul(), "P2Perf Simulator", nSteps);

}

void P2Perf::dump() const {
  getSimul().dump();
}

void P2Perf::quit() {  
}

void P2Perf::undefine() {
  if (Sworkholders) {
    for (int iStep = 0; iStep < itsSourceSteps; iStep++) {
      delete Sworkholders[iStep];
    }
  }

  if (Ssteps) {
    for (int iStep = 0; iStep < itsSourceSteps; iStep++) {
      delete Ssteps[iStep];
    }
  }

  delete [] Sworkholders;
  delete [] Ssteps;

  if (Dworkholders) {
    for (int iStep = 0; iStep < itsDestSteps; iStep++) {
      delete Dworkholders[iStep];
    }
  }

  if (Dsteps) {
    for (int iStep = 0; iStep < itsDestSteps; iStep++) {
      delete Dsteps[iStep];
    }
  }

  delete [] Dworkholders;
  delete [] Dsteps;
}
