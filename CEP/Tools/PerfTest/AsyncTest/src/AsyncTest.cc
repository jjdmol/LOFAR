//  AsyncTest.cc: Concrete Simulator class for performance measurements on
//            a sequence of cross-connected steps
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
#include "AsyncTest/AsyncTest.h"
#include "AsyncTest/WH_Source.h"
#include "AsyncTest/WH_Sink.h"

#include "CEPFrame/TH_Mem_Bl.h"

#include TRANSPORTERINCLUDE

#ifdef HAVE_CORBA
#include "CEPFrame/Corba/BS_Corba.h"
#include "CEPFrame/Corba/TH_Corba.h"
#endif

AsyncTest::AsyncTest():
  itsSourceSteps(0),
  itsDestSteps(0),
  itsSyncRW(true)
{
  Sworkholders = NULL;
  Ssteps       = NULL;
  Dworkholders = NULL;
  Dsteps       = NULL;
}

AsyncTest::~AsyncTest()
{
  undefine();
}

/**
   define function for the AsyncTest simulation. It defines a list
   of steps that each process a part of the data.
 */
void AsyncTest::define(const ParamBlock& params)
{
#ifdef HAVE_CORBA
  // Start Orb Environment
  AssertStr (BS_Corba::init(), "Could not initialise CORBA environment");
#endif

  char name[20];  // name used during Step/WH creation
  
  // free any memory previously allocated
  undefine();

  WH_Empty empty;

  Simul simul(empty, 
	      "AsyncTest",
	      true, 
	      true,  // controllable	      
	      true); // monitor
  setSimul(simul);
  simul.runOnNode(0);
  simul.setCurAppl(0);

  itsSourceSteps = 2;
  itsDestSteps   = 1;
  itsFixedSize   = params.getInt("fixedsize",0);
  itsSyncRW     = params.getInt("synchronous",1);

  bool  WithMPI=false;

#ifdef HAVE_MPI    
  WithMPI = true;
#endif

  // Create the Workholders and Steps
  Sworkholders = new (WH_Source*)[itsSourceSteps];
  Ssteps       = new (Step*)[itsSourceSteps];
  Dworkholders = new (WH_Sink*)[itsDestSteps];
  Dsteps       = new (Step*)[itsDestSteps];
  
  
  // now go and create the source and destination steps
  // the two loops do duplicate quite some code, so a private method  
  // should be made later...

  // Create the Source Steps
  bool monitor;
  for (int iStep = 0; iStep < itsSourceSteps; iStep++) {
    
    // Create the Source Step
    sprintf(name, "GrowSizeSource[%d]", iStep);
    Sworkholders[iStep] = new WH_Source(name, 
				  1, // should be 0 
				  itsDestSteps,
				  (itsFixedSize?itsFixedSize:MAX_GROW_SIZE),
				  itsFixedSize!=0,
				  itsSyncRW);
    
      monitor = (iStep==0) ? true:false;
      Ssteps[iStep] = new Step(Sworkholders[iStep], 
			       "GrowSizeSourceStep", 
			       iStep,
			       monitor);
      
      // Determine the node and process to run in
      Ssteps[iStep]->runOnNode(iStep  ,0); // run in App 0
  }
  
  
  // Create the destination steps
  for (int iStep = 0; iStep < itsDestSteps; iStep++) {
    
    // Create the Destination Step
    sprintf(name, "GrowSizeDest[%d]", iStep);
    Dworkholders[iStep] = new WH_Sink(name, 
				  itsSourceSteps, 
				  1, // should be 0 
				  (itsFixedSize?itsFixedSize:MAX_GROW_SIZE),
				  itsFixedSize!=0,
				  itsSyncRW);
    
    Dsteps[iStep] = new Step(Dworkholders[iStep], "GrowSizeDestStep", iStep);
    // Determine the node and process to run in
    if (WithMPI) {
      TRACER2("Dest MPI runonnode (" << iStep << ")");
      Dsteps[iStep]->runOnNode(iStep+itsSourceSteps,0); // run in App 0
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
      if (itsSyncRW)
      {
	Dsteps[step]->connect(Ssteps[ch],ch,step,1,TH_Mem::proto);
      }
      else
      {
	Dsteps[step]->connect(Ssteps[ch],ch,step,1,TH_Mem_Bl::proto);
      }

#endif
#endif
    }
  }
  
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

void AsyncTest::run(int nSteps) {
  nSteps = nSteps;
  doIt(getSimul(), "AsyncTest Simulator", nSteps);

}

void AsyncTest::dump() const {
  getSimul().dump();
}

void AsyncTest::quit() {  
}

void AsyncTest::undefine() {
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
