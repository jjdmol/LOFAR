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
//  $Log$
//  Revision 1.30  2002/07/19 13:52:44  wierenga
//
//  %[BugId: 73]%
//
//  Add fixedsize parameter to allow fixed message size transfers tests.
//
//  Revision 1.29  2002/06/19 10:49:11  wierenga
//  %[BugId: 33]%
//
//  First version of buffered MPI transportholder.
//
//  Revision 1.28  2002/06/07 10:44:54  schaaf
//
//  %[BugId: 36]%
//
//  Use ParameterBlock for:
//  sources            -> the number of source steps
//  destinations       -> the number of destination steps
//  destside           -> flag for selecting destination side application (needed for Corba connections)
//
//  Revision 1.27  2002/05/23 08:50:50  wierenga
//  %[BugId: 4]%
//  Adapt to new location of shmem_alloc.
//
//  Revision 1.26  2002/05/15 15:04:49  wierenga
//  Use static class prototypes.
//
//  Revision 1.25  2002/05/14 11:48:36  gvd
//  Use lofar_string and lofar_iostream
//
//  Revision 1.24  2002/05/08 14:39:50  schaaf
//  modified command line argument handling and MPI node distribution
//
//  Revision 1.23  2002/05/08 14:28:37  wierenga
//  DataHolder allocation moved from constructor to preprocess to be able to
//  use TransportHolder::allocate.
//  Bug fixes in P2Perf.cc for -mpi arguments.
//
//  Revision 1.22  2002/05/08 08:20:04  schaaf
//  Modified includes for new build env
//
//  Revision 1.21  2002/05/02 12:21:07  schaaf
//  Use monitor object in first step
//
//  Revision 1.20  2002/04/18 07:55:03  schaaf
//  Documentation and code update
//
//  Revision 1.19  2002/04/12 15:50:46  schaaf
//  Updated for multiple source steps and cross connects
//
//  Revision 1.18  2002/04/09 13:32:40  wierenga
//  include Debug.h
//
//  Revision 1.17  2002/03/26 11:58:52  schaaf
//  Use controllable Simul
//
//  Revision 1.16  2002/03/18 11:06:08  schaaf
//  moved cout into TRACER2()
//
//  Revision 1.15  2002/03/08 11:38:42  wierenga
//  Upgraded from firewalls.h use to Debug.h use. This version was used for performance tests.
//
//  Revision 1.14  2001/12/17 16:28:41  schaaf
//  removed first step flag
//
//  Revision 1.13  2001/11/28 16:15:40  schaaf
//  .
//
//  Revision 1.12  2001/11/05 17:02:38  schaaf
//  set divisor variable for MPI
//
//  Revision 1.11  2001/10/31 11:34:18  wierenga
//  LOFAR CVS Repository structure change and transition to autotools (autoconf, automake and libtool).
//
//  Revision 1.10  2001/10/26 10:06:28  wierenga
//  Wide spread changes to convert from Makedefs to autoconf/automake/libtool build environment
//
//  Revision 1.9  2001/10/26 08:55:01  schaaf
//  minor changes
//
//  Revision 1.8  2001/09/19 09:19:34  wierenga
//  Allocate WH_Empty on heap.
//
//  Revision 1.7  2001/09/19 09:05:09  wierenga
//  Allocate simul and empty on stack.
//
//  Revision 1.6  2001/09/19 08:47:20  wierenga
//  Make sure it compiles again with latest changes in BaseSim.
//
//  Revision 1.5  2001/09/19 08:10:51  wierenga
//  Changes to do perform bandwidth tests.
//
//  Revision 1.4  2001/09/19 08:00:13  wierenga
//  Added code to do performance tests.
//
//  Revision 1.3  2001/08/16 15:14:22  wierenga
//  Implement GrowSize DH and WH for performance measurements. Timing code still needs to be added.
//
//  Revision 1.2  2001/08/13 12:22:56  schaaf
//  Use BS_Corba class
//
//  Revision 1.1  2001/08/09 15:48:48  wierenga
//  Implemented first version of TH_Corba and test program
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
#include "BaseSim/Transport.h"
#include "BaseSim/Step.h"
#include "BaseSim/Simul.h"
#include "BaseSim/Profiler.h"
#include "BaseSim/WH_Empty.h"
#include "BaseSim/ParamBlock.h"
#include "BaseSim/ShMem/TH_ShMem.h"
#include "P2Perf/P2Perf.h"
#include "P2Perf/WH_GrowSize.h"

#include TRANSPORTERINCLUDE

#ifdef HAVE_CORBA
#include "BaseSim/Corba/BS_Corba.h"
#include "BaseSim/Corba/TH_Corba.h"
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
      Dsteps[step]->connect(Ssteps[ch],ch,step,1);
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
