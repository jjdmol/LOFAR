//#  AsyncTest.cc: Concrete Simulator class for performance measurements on
//#            a sequence of cross-connected steps
//#
//#  Copyright (C) 2000, 2002
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//#  This program is free software; you can redistribute it and/or modify
//#  it under the terms of the GNU General Public License as published by
//#  the Free Software Foundation; either version 2 of the License, or
//#  (at your option) any later version.
//#
//#  This program is distributed in the hope that it will be useful,
//#  but WITHOUT ANY WARRANTY; without even the implied warranty of
//#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//#  GNU General Public License for more details.
//#
//#  You should have received a copy of the GNU General Public License
//#  along with this program; if not, write to the Free Software
//#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#
//#  $Id$

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

#include <Common/lofar_iostream.h>
#include <Common/lofar_string.h>

#include <AsyncTest/AsyncTest.h>

#include <Common/LofarLogger.h>
#include <CEPFrame/Step.h>
#include <CEPFrame/Composite.h>
#include <tinyCEP/Profiler.h>
#include <Transport/TH_Mem.h>
#include <AsyncTest/WH_Source.h>
#include <AsyncTest/WH_Sink.h>

#include TRANSPORTERINCLUDE

using namespace LOFAR;

AsyncTest::AsyncTest()
  : Sworkholders  (0),
    Dworkholders  (0),
    Ssteps        (0),
    Dsteps        (0),
    itsSourceSteps(0),
    itsDestSteps  (0),
    itsSyncRW     (true)
{}

AsyncTest::~AsyncTest()
{
  undefine();
}

/**
   define function for the AsyncTest simulation. It defines a list
   of steps that each process a part of the data.
 */
void AsyncTest::define(const KeyValueMap& params)
{
  char name[20];  // name used during Step/WH creation
  
  // free any memory previously allocated
  undefine();

  Composite comp(0, 0, "AsyncTest");
  setComposite(comp);
  comp.runOnNode(0);
  comp.setCurAppl(0);

  itsSourceSteps = 2;
  itsDestSteps   = 1;
  //  itsFixedSize   = params.getInt("fixedsize",0);
  //  itsSyncRW     = params.getInt("synchronous",1);

  itsFixedSize   = 1024;
  itsSyncRW     = 0;

  bool  WithMPI=false;

#ifdef HAVE_MPI    
  WithMPI = true;
#endif

  // Create the Workholders and Steps
  Sworkholders = new WH_Source*[itsSourceSteps];
  Ssteps       = new Step*[itsSourceSteps];
  Dworkholders = new WH_Sink*[itsDestSteps];
  Dsteps       = new Step*[itsDestSteps];
  
  
  // now go and create the source and destination steps
  // the two loops do duplicate quite some code, so a private method  
  // should be made later...

  // Create the Source Steps
  bool monitor;
  for (int iStep = 0; iStep < itsSourceSteps; iStep++) {
    
    // Create the Source Step
    sprintf(name, "GrowSizeSource[%d]", iStep);
    Sworkholders[iStep] = new WH_Source(name, 
				  0, 
				  itsDestSteps,
				  (itsFixedSize?itsFixedSize:MAX_GROW_SIZE),
				  itsFixedSize!=0, itsSyncRW);
    
      monitor = (iStep==0) ? true:false;
      Ssteps[iStep] = new Step(Sworkholders[iStep], 
			       "SourceStep", 
			       iStep);

      // Set the buffering properties
      for (int nrOutp=0; nrOutp<itsDestSteps; nrOutp++)
      {
	Ssteps[iStep]->setOutBuffer(nrOutp, itsSyncRW, 20);
      }
      // Determine the node and process to run in
      //      Ssteps[iStep]->runOnNode(iStep  ,0); // run in App 0
  }
  
  
  // Create the destination steps
  for (int iStep = 0; iStep < itsDestSteps; iStep++) {
    
    // Create the Destination Step
    sprintf(name, "GrowSizeDest[%d]", iStep);
    Dworkholders[iStep] = new WH_Sink(name, 
				  itsSourceSteps, 
				  0, 
				  (itsFixedSize?itsFixedSize:MAX_GROW_SIZE),
				  itsFixedSize!=0,
				  itsSyncRW);
    
    Dsteps[iStep] = new Step(Dworkholders[iStep], "GrowSizeDestStep", iStep);

    // Set the buffering properties
    for (int nrInp=0; nrInp<itsSourceSteps; nrInp++)
    {
      Dsteps[iStep]->setInBuffer(nrInp, itsSyncRW, 20);
    }

    // Determine the node and process to run in
    if (WithMPI) {
      LOG_TRACE_RTTI_STR("Dest MPI runonnode (" << iStep << ")");
      //      Dsteps[iStep]->runOnNode(iStep+itsSourceSteps,0); // run in App 0
    } else {
      //      Dsteps[iStep]->runOnNode(iStep+1,0); // run in App 0
    }
  }

  // Now Add the steps to the composite;
  // first ALL the sources....
  for (int iStep = 0; iStep < itsSourceSteps; iStep++) {
    LOG_TRACE_RTTI_STR("Add Source step " << iStep);
    comp.addBlock(Ssteps[iStep]);
  }
  // ...then the destinations
  for (int iStep = 0; iStep < itsDestSteps; iStep++) {
    LOG_TRACE_RTTI_STR("Add Dest step " << iStep);
    comp.addBlock(Dsteps[iStep]);
  }
  
  // Create the cross connections
  for (int step = 0; step < itsDestSteps; step++) {
    for (int ch = 0; ch < itsSourceSteps; ch++) {
      // Set up the connections
      // Correlator Style

// #ifdef HAVE_MPI
//       LOG_TRACE_RTTI("Connect using MPI");
//       Dsteps[step]->connect(ch, Ssteps[ch],step,1, 
// 			    new TH_MPI(ch, itsSourceSteps+step));
// #else
      if (itsSyncRW)
      {
	Dsteps[step]->connect(ch, Ssteps[ch],step,1, new TH_Mem(), false);
      }
      else
      {
	Dsteps[step]->connect(ch, Ssteps[ch],step,1, new TH_Mem(), true);
      }

// #endif
    }
  }
  
}

void doIt (Composite& comp, const std::string& name, int nsteps) {
#if 0
  simul.resolveComm();
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

  LOG_TRACE_FLOW_STR("END OF COMPOSITE on node " << TRANSPORTER::getCurrentRank () );
 
#if 0
  //     close environment
  TRANSPORTER::finalize();
#endif
}

void AsyncTest::run(int nSteps) {
  nSteps = nSteps;
  doIt(getComposite(), "AsyncTest Simulator", nSteps);

}

void AsyncTest::dump() const {
  getComposite().dump();
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
