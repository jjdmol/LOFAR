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
//  Revision 1.6  2002/05/24 14:17:18  schaaf
//  %[BugId: 11]%
//  Use Parameter block for definition of source/dest steps etc.
//
//  Revision 1.5  2002/05/23 15:38:57  schaaf
//
//  %[BugId: 11]%
//  Add correlator steps
//
//  Revision 1.4  2002/05/16 15:08:00  schaaf
//  overall update; removed command line arguments
//
//  Revision 1.3  2002/05/14 11:39:41  gvd
//  Changed for new build environment
//
//  Revision 1.2  2002/05/07 11:15:38  schaaf
//  minor
//
//  Revision 1.1.1.1  2002/05/06 11:49:20  schaaf
//  initial version
//
//
//
//////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <Common/lofar_iostream.h>
#include <stdlib.h>
#include <Common/lofar_string.h>

#include "Common/Debug.h"
#include "BaseSim/Transport.h"
#include "BaseSim/Step.h"
#include "Transpose/Transpose.h"
#include "BaseSim/Simul.h"
#include "BaseSim/Profiler.h"
#include "Transpose/WH_FillTFMatrix.h"
#include "Transpose/WH_Transpose.h"
#include "Transpose/WH_Correlate.h"
#include "BaseSim/WH_Empty.h"
#include "BaseSim/ShMem/TH_ShMem.h"
#include TRANSPORTERINCLUDE

#ifdef HAVE_CORBA
#include "BaseSim/Corba/BS_Corba.h"
#include "BaseSim/Corba/TH_Corba.h"
#endif

#include "BaseSim/ShMem/TH_ShMem.h"


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

#ifdef HAVE_MPI
  // TH_ShMem only works in combination with MPI
  // initialize TH_ShMem
  TH_ShMem::init(0, NULL);
#endif

  params.show (cout);

  char name[20];  // name used during Step/WH creation
  
  int rank = TRANSPORTER::getCurrentRank();
  unsigned int size = TRANSPORTER::getNumberOfNodes();

  // free any memory previously allocated
  undefine();

  TRACER2("Transpose Processor " << rank << " of " << size << " operational.");

  WH_Empty empty;

  Simul simul(empty, "Transpose",true,true);
  setSimul(simul);
  simul.runOnNode(0);
  
  
  TRACER2("Default settings");
  simul.setCurAppl(0);
  itsSourceSteps = params.getInt("stations",1); // nr of stations (?)
  itsDestSteps   = params.getInt("correlators",1);
  cout << "stations = " << itsSourceSteps << "  correlators = " << itsDestSteps << endl;
  
  // Create the Workholders and Steps
  Sworkholders = new (WH_FillTFMatrix*)[itsSourceSteps];
  Ssteps       = new (Step*)[itsSourceSteps];
  Dworkholders = new (WH_Transpose*)[itsDestSteps];
  Dsteps       = new (Step*)[itsDestSteps];
  Cworkholders = new (WH_Correlate*)[itsDestSteps];
  Csteps       = new (Step*)[itsDestSteps];
  
  // now go and create the source and destination steps
  // the two loops do duplicate quite some code, so a private method  
  // should be made later...

  // Create the Source Steps
  int timeDim = params.getInt("times",1);
  int freqDim = params.getInt("freqbandsize",4096);
  for (int iStep = 0; iStep < itsSourceSteps; iStep++) {
    
    // Create the Source Step
    sprintf(name, "Filler[%d]", iStep);
    Sworkholders[iStep] = new WH_FillTFMatrix(name,
					      iStep, // source ID
					      1,     // in; should be 0
					             //     ...see %[BugId: 14]% 
					      itsDestSteps, //nout
					      timeDim,
					      freqDim);
    
    Ssteps[iStep] = new Step(Sworkholders[iStep], "TransposeSourceStep", iStep);
    //Ssteps[iStep]->connectInput(NULL);

    // Determine the node and process to run in
    // ... sory, this should go in a private method later
    //Ssteps[iStep]->runOnNode(0); // MPI 1 process
       Ssteps[iStep]->runOnNode(iStep ,0); // run in App 0
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
    Dsteps[iStep] = new Step(Dworkholders[iStep], 
			     "TransposeDestStep", 
			     iStep);
    Dsteps[iStep]->runOnNode(iStep+itsSourceSteps,0); 

    // Create the correlator step
    sprintf(name, "Correlate[%d]", iStep);
    Cworkholders[iStep] = new WH_Correlate(name,
					   timeDim,        // inputs
					   1,              // outputs
					   itsSourceSteps, // stations
					   freqDim);       // frequency

    Csteps[iStep] = new Step(Cworkholders[iStep],
			     "Correlator",
			     iStep);
    Csteps[iStep]->runOnNode(iStep+itsSourceSteps,0); 
    // connect the correlator to the corresponding transpose step
    Csteps[iStep]->connectInput(Dsteps[iStep]);
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
  // and the correlators
  for (int iStep = 0; iStep < itsDestSteps; iStep++) {
    TRACER4("Add Dest step " << iStep);
    simul.addStep(Csteps[iStep]);
  }

  // Create the cross connections
  for (int step = 0; step < itsDestSteps; step++) {
    for (int ch = 0; ch < itsSourceSteps; ch++) {
      // Set up the connections
      // Correlator Style
      TRACER2("Transpose; try to connect " << step << "   " << ch);
#ifdef HAVE_CORBA
      Dsteps[step]->connect(Ssteps[ch],ch,step,1,TH_Corba::proto);
#elif HAVE_MPI
      Dsteps[step]->connect(Ssteps[ch],ch,step,1,TH_MPI::proto);
#else
      Dsteps[step]->connect(Ssteps[ch],ch,step,1);
#endif
    }
  }

  //simul.optimizeConnectionsWith(TH_ShMem::proto);
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
#ifdef HAVE_MPI
     TH_MPI::synchroniseAllProcesses();
#endif
    if (i==2) Profiler::activate();
    simul.process();
    if (i==5) Profiler::deActivate();
  }

  TRACER4("END OF SIMUL on node " << TRANSPORTER::getCurrentRank () );
 
  //     close environment
  //  TRANSPORTER::finalize();
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
