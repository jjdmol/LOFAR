//  Transpose.cc: Concrete Simulator class for performance measurements on
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
//  Revision 1.1.1.1  2002/05/06 11:49:20  schaaf
//  initial version
//
//
//
//////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <iostream.h>
#include <stdlib.h>
#include <string>

#include "Debug.h"
#include "Transport.h"
#include "Step.h"
#include "Transpose.h"
#include "Simul.h"
#include "Profiler.h"
#include "WH_FillTFMatrix.h"
#include "WH_Transpose.h"
#include "WH_Empty.h"
#include TRANSPORTERINCLUDE

#ifdef HAVE_CORBA
#include "BS_Corba.h"
#include "TH_Corba.h"
#endif


Transpose::Transpose():
  itsSourceSteps(0),
  itsDestSteps(0)
{
  Sworkholders = NULL;
  Ssteps       = NULL;
  Dworkholders = NULL;
  Dsteps       = NULL;
}

Transpose::~Transpose()
{
  undefine();
}

/**
   define function for the Transpose simulation. It defines a list
   of steps that each process a part of the data.
 */
void Transpose::define(const ParamBlock& params)
{
#ifdef HAVE_CORBA
  // Start Orb Environment
  AssertStr (BS_Corba::init(), "Could not initialise CORBA environment");
#endif
  

  char name[20];  // name used during Step/WH creation
  int    argc = 0;
  char** argv = NULL;
  
#ifdef HAVE_CORBA
  TH_Corba corbaProto;
#endif

  int rank = TRANSPORTER::getCurrentRank();
  unsigned int size = TRANSPORTER::getNumberOfNodes();

  // free any memory previously allocated
  undefine();

  TRACER2("Transpose Processor " << rank << " of " << size << " operational.");

  WH_Empty empty;

  Simul simul(empty, "Transpose",true,true);
  setSimul(simul);
  simul.runOnNode(0);


  getarg(&argc, &argv);
  AssertStr (argc >= 3,"Need nr of inputs and nr of outputs as argument"); 
  // start looking at the command line arguments;
  // first two arguments are nr of inputs/outputs
  // -odd and -even are used to have all Sources in one application 
  //                and all destinations in the other

  // determine the number of source/destination steps
  itsSourceSteps = atoi(argv[1]);
  itsDestSteps   = atoi(argv[2]);

  bool RunInOneAppl = false;
  bool SplitInTwoApps = false;
  bool OddSide=false;
  bool UseMPIRanks = true;
  if ((argc == 4 )
      && (((!strncmp(argv[3], "-odd", 4))) ||  ((!strncmp(argv[3], "-odd", 4)))) ){
    TRACER4("Split in Two Apps");
    if ((!strncmp(argv[3], "-odd", 4))) {
      SplitInTwoApps = true;
      OddSide = true;
      simul.setCurAppl(0);
    } else if (!strncmp(argv[3], "-even", 5) ) {
      SplitInTwoApps = true;
      OddSide = false;
      simul.setCurAppl(1);
    } 
  } else if ((argc == 2 )
	     && ((!strncmp(argv[3], "-one", 4))) ){
    TRACER4("Run in One Appl");
    simul.setCurAppl(0);
    RunInOneAppl = true;
  } else  if ((argc == 2 )
	      && ((!strncmp(argv[3], "-mpi", 4))) ){
    TRACER4("Split in MPI applications");
#ifdef HAVE_MPI    
    UseMPIRanks = true;
    simul.setCurAppl(rank);
#else
    AssertStr (false,"Need MPI for -mpi flag"); 
#endif
  } else {
    simul.setCurAppl(0);
    RunInOneAppl = true;
  }



  // Create the Workholders and Steps
  Sworkholders = new (WH_FillTFMatrix*)[itsSourceSteps];
  Ssteps       = new (Step*)[itsSourceSteps];
  Dworkholders = new (WH_Transpose*)[itsDestSteps];
  Dsteps       = new (Step*)[itsDestSteps];
  
  // now go and create the source and destination steps
  // the two loops do duplicate quite some code, so a private method  
  // should be made later...

  // Create the Source Steps
  int timeDim = 1;
  int freqDim = 4096;
  for (int iStep = 0; iStep < itsSourceSteps; iStep++) {
    
    // Create the Source Step
    sprintf(name, "Filler[%d]", iStep);
    Sworkholders[iStep] = new WH_FillTFMatrix(name,
					      iStep, // source ID
					      1,     // should be 0 
					      itsDestSteps,
					      timeDim,
					      freqDim);
    
    Ssteps[iStep] = new Step(Sworkholders[iStep], "TransposeSourceStep", iStep);
    //Ssteps[iStep]->connectInput(NULL);

    // Determine the node and process to run in
    // ... sory, this should go in a private method later
    //Ssteps[iStep]->runOnNode(0); // MPI 1 process
    if (SplitInTwoApps) {
      Ssteps[iStep]->runOnNode(iStep  ,0); // run in App 0
    } else if (RunInOneAppl) {
      Ssteps[iStep]->runOnNode(iStep  ,0); // run in App 0
    } else if (UseMPIRanks) {
      Ssteps[iStep]->runOnNode(iStep  ,iStep); // run in App 0
    }    
  }
  
  // Create the destination steps
  for (int iStep = 0; iStep < itsDestSteps; iStep++) {

    // Create the Destination Step
    sprintf(name, "Transpose[%d]", iStep);
    Dworkholders[iStep] = new WH_Transpose(name, 
					   itsSourceSteps, 
					   timeDim,
					   timeDim,
					   freqDim);
    
    
    Dsteps[iStep] = new Step(Dworkholders[iStep], "TransposeDestStep", iStep);
    // Determine the node and process to run in
    // ... sory, this should go in a private method later
    //Ssteps[iStep]->runOnNode(0); // MPI 1 process
    if (SplitInTwoApps) {
      Dsteps[iStep]->runOnNode(iStep,1); // run in App 1
    } else if (RunInOneAppl) {
      Dsteps[iStep]->runOnNode(iStep,0); // run in App 0
    } else if (UseMPIRanks) {
      Dsteps[iStep]->runOnNode(iStep,0); // run in App 0
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
      Dsteps[step]->connect(Ssteps[ch],ch,step,1,corbaProto);
#else
      //Dsteps[DStep]->connectInput(Ssteps[DStep]);
      TRACER2("Transpose; try to connect " << step << "   " << ch);
      Dsteps[step]->connect(Ssteps[ch],ch,step,1);
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
 
  //     close environment
  TRANSPORTER::finalize();
}

void Transpose::run(int nSteps) {
  nSteps = nSteps;
  doIt(getSimul(), "Transpose Simulator", nSteps);

}

void Transpose::dump() const {
  getSimul().dump();
}

void Transpose::quit() {  
}

void Transpose::undefine() {
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
