//  P2Perf.cc: Concrete Simulator class for ring structure
//
//  Copyright (C) 2000, 2001
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
#include "Debug.h"
#include "Transport.h"
#include "Step.h"
#include "P2Perf.h"
#include "Simul.h"
#include "Profiler.h"
#include "WH_Empty.h"
#include "WH_GrowSize.h"
#include "ParamBlock.h"
#include TRANSPORTERINCLUDE

#ifdef HAVE_CORBA
#include "BS_Corba.h"
#include "TH_Corba.h"
#endif

#include "Debug.h"
#include <iostream.h>
#include <stdlib.h>
#include <string>

#define NR_OF_STEPS 6

/**
   This class is an example of a concrete Simulator.
*/

P2Perf::P2Perf()
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
  AssertStr (NR_OF_STEPS%2 == 0, "Need even number of steps");
#ifdef HAVE_CORBA
  // Start Orb Environment
  AssertStr (BS_Corba::init(), "Could not initialise CORBA environment");
#endif
  

  int    argc = 0;
  char** argv = NULL;
  
#ifdef HAVE_CORBA
  TH_Corba corbaProto;
#endif

  int rank = TRANSPORTER::getCurrentRank();
  unsigned int size = TRANSPORTER::getNumberOfNodes();

  // free any memory previously allocated
  undefine();

  cout << "P2Perf Processor " << rank << " of " << size << " operational."
       << flush << endl;

   WH_Empty empty;

#ifdef HAVE_CORBA
  Simul simul(empty, "P2Perf",true,false);
#else
  Simul simul(empty, "P2Perf",true,false);
#endif
  setSimul(simul);
  simul.runOnNode(0);


  getarg(&argc, &argv);

  // start looking at the command line arguments;
  // -odd and -even are used to have all Sources in one application 
  //                and all destinations in the other
  bool RunInOneAppl = false;
  bool SplitInTwoApps = false;
  bool OddSide=false;
  bool UseMPIRanks = true;
  if ((argc == 2 )
      && (((!strncmp(argv[1], "-odd", 4))) ||  ((!strncmp(argv[1], "-odd", 4)))) ){
    cout << "Split in Two Apps" << endl;
    if ((!strncmp(argv[1], "-odd", 4))) {
      SplitInTwoApps = true;
      OddSide = true;
      simul.setCurAppl(0);
    } else if (!strncmp(argv[1], "-even", 5) ) {
      SplitInTwoApps = true;
      OddSide = false;
      simul.setCurAppl(1);
    } 
  } else if ((argc == 2 )
	     && ((!strncmp(argv[1], "-one", 4))) ){
    cout << "Run in One Appl" << endl;
    simul.setCurAppl(0);
    RunInOneAppl = true;
  } else  if ((argc == 2 )
	      && ((!strncmp(argv[1], "-mpi", 4))) ){
    cout << "Split in MPI applications" << endl;
#ifdef HAVE_MPI    
    UseMPIRanks = true;
    simul.setCurAppl(rank);
#else
    AssertStr (false,"Need MPI for -mpi flag"); 
#endif
  } else {
    simul.setCurAppl(0);
  }



  // determine the number of source/destination steps
  int SourceSteps = NR_OF_STEPS/2;
  int DestSteps   = SourceSteps;

  // Create the Workholders and Steps
  Sworkholders = new (WH_GrowSize*)[SourceSteps];
  Ssteps       = new (Step*)[SourceSteps];
  Dworkholders = new (WH_GrowSize*)[DestSteps];
  Dsteps       = new (Step*)[DestSteps];
  
  AssertStr (SourceSteps == DestSteps, 
	     "Number of Source Steps must be equal to number of Destinations");
  for (int iStep = 0; iStep < SourceSteps; iStep++) {
    
    char name[20];

    // Create the Source Step
    sprintf(name, "GrowSizeSource[%d]", iStep);
    Sworkholders[iStep] = new WH_GrowSize(name, 
					  false, 
					  1, // should be 0 
					  DestSteps, 
					  MAX_GROW_SIZE ,
					  false); // flag for source side
    
    Ssteps[iStep] = new Step(Sworkholders[iStep], "GrowSizeSourceStep", iStep);
    //Ssteps[iStep]->connectInput(NULL);
    
    // Create the Destination Step
    sprintf(name, "GrowSizeDest[%d]", iStep);
    Dworkholders[iStep] = new WH_GrowSize(name, 
					  false, 
					  SourceSteps, 
					  1, // should be 0 
					  MAX_GROW_SIZE,
					  true);
    
    Dsteps[iStep] = new Step(Dworkholders[iStep], "GrowSizeDestStep", iStep);
      
    // Determine the node and process to run in
    //Ssteps[iStep]->runOnNode(0); // MPI 1 process
    if (SplitInTwoApps) {
      Ssteps[iStep]->runOnNode(iStep  ,0); // run in App 0
      Dsteps[iStep]->runOnNode(iStep+1,1); // run in App 1
    } else if (RunInOneAppl) {
      Ssteps[iStep]->runOnNode(iStep  ,0); // run in App 0
      Dsteps[iStep]->runOnNode(iStep+1,0); // run in App 0
    } else if (UseMPIRanks) {
      Ssteps[iStep]->runOnNode(iStep  ,iStep); // run in App 0
      Dsteps[iStep]->runOnNode(iStep+1,iStep); // run in App 0
    }    
    
  }

  // Now Add the steps;
  // first ALL the sources....
  for (int iStep = 0; iStep < SourceSteps; iStep++) {
    cout << "Add Source step " << iStep << endl;
    simul.addStep(Ssteps[iStep]);
  }
  // ...then the destinations
  for (int iStep = 0; iStep < DestSteps; iStep++) {
    cout << "Add Dest step " << iStep << endl;
    simul.addStep(Dsteps[iStep]);
  }
  
  for (int DStep = 0; DStep < SourceSteps; DStep++) {
    for (int DStepCh = 0; DStepCh < SourceSteps; DStepCh++) {
      // Set up the connections
      // Correlator Style
#ifdef HAVE_CORBA
      Dsteps[DStep]->connect(Ssteps[DStepCh],DStepCh,DStep,1,corbaProto);
#else
      Dsteps[DStep]->connectInput(Ssteps[DStep]);
      //Dsteps[DStep]->connect(Ssteps[DStepCh],DStepCh,DStep,1);
#endif
    }
  }
}

void doIt (Simul& simul, const std::string& name, int nsteps)
  {
#if 0
  simul.resolveComm();
#endif
  TRACER1("Ready with definition of configuration");
  Profiler::init();
  Step::clearEventCount();

  cout << endl << "Start Processing simul " << name << endl;    
  for (int i=0; i<nsteps; i++) {
    if (i==2) Profiler::activate();
    TRACER2("Call simul.process() ");
    simul.process();
    if (i==5) Profiler::deActivate();
  }

  cout << endl << "DUMP Data from last Processing step: " << endl;
  simul.dump ();
  cout << endl << "END OF SIMUL on node " 
       << TRANSPORTER::getCurrentRank () 
       << endl;
 
  //     close environment
  TRANSPORTER::finalize();
}

void P2Perf::run(int nSteps)

{

  nSteps = nSteps;

  doIt(getSimul(), "P2Perf Simulator", nSteps);

}

void P2Perf::dump() const
{
  getSimul().dump();
}

void P2Perf::quit()
{
  
}

void P2Perf::undefine()
{
  if (Sworkholders)
  {
    for (int iStep = 0; iStep < NR_OF_STEPS; iStep++)
    {
      delete Sworkholders[iStep];
    }
  }

  if (Ssteps)
  {
    for (int iStep = 0; iStep < NR_OF_STEPS; iStep++)
    {
      delete Ssteps[iStep];
    }
  }

  delete [] Sworkholders;
  delete [] Ssteps;

  if (Dworkholders)
  {
    for (int iStep = 0; iStep < NR_OF_STEPS; iStep++)
    {
      delete Dworkholders[iStep];
    }
  }

  if (Dsteps)
  {
    for (int iStep = 0; iStep < NR_OF_STEPS; iStep++)
    {
      delete Dsteps[iStep];
    }
  }

  delete [] Dworkholders;
  delete [] Dsteps;
}
