//  Pipeline.cc: Concrete Simulator class for demonstration
//               of a processing pipeline 
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
#include "Pipeline/Pipeline.h"
#include "BaseSim/Simul.h"
#include "BaseSim/Profiler.h"
#include "Pipeline/WH_FillTFMatrix.h"
#include "Pipeline/WH_Delay.h"
#include "Pipeline/WH_Transpose.h"
#include "Pipeline/WH_Correlate.h"
#include "BaseSim/WH_Empty.h"
#include "BaseSim/ShMem/TH_ShMem.h"
#include "BaseSim/TH_Mem.h"
#include TRANSPORTERINCLUDE

#ifdef HAVE_CORBA
#include "BaseSim/Corba/BS_Corba.h"
#include "BaseSim/Corba/TH_Corba.h"
#endif

#include "BaseSim/ShMem/TH_ShMem.h"


Pipeline::Pipeline():
  itsSourceSteps(0),
  itsDestSteps(0)
{
  Sworkholders = NULL;
  Ssteps       = NULL;
  Dworkholders = NULL;
  Dsteps       = NULL;
}

Pipeline::~Pipeline()
{
  undefine();
}

/**
   define function for the Pipeline simulation. It defines a list
   of steps that each process a part of the data.
 */
void Pipeline::define(const ParamBlock& params)
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
  
  char name[20];  // name used during Step/WH creation
  
  int rank = TRANSPORTER::getCurrentRank();
  unsigned int size = TRANSPORTER::getNumberOfNodes();

  if (rank == 0) 
    cout << "************************************** Start Run ************************************" << endl;


  if (rank == 0) params.show (cout);
  
  // free any memory previously allocated
  undefine();

  TRACER2("Pipeline Processor " << rank << " of " << size << " operational.");

  WH_Empty empty;

  Simul simul(empty, params.getString("name","Pipeline").c_str(),true,false);
  setSimul(simul);
  int applicationnr = params.getInt("application",0); 
  // the top-level simul
  simul.runOnNode(1,applicationnr);
  
  
  TRACER2("Default settings");
  simul.setCurAppl(applicationnr);
  itsSourceSteps = params.getInt("stations",3); // nr of stations (?)
  itsDestSteps   = params.getInt("correlators",3);
    //itsDoLogProfile = params.getBool("log",false);
  itsDoLogProfile = params.getInt("log",0) == 1;
   
  // Create the Workholders and Steps
  Sworkholders = new (WH_FillTFMatrix*)[itsSourceSteps];
  Ssteps       = new (Step*)[itsSourceSteps];
  Iworkholders = new (WH_Delay*)[itsSourceSteps];
  Isteps       = new (Step*)[itsSourceSteps];
  Dworkholders = new (WH_Transpose*)[itsDestSteps];
  Dsteps       = new (Step*)[itsDestSteps];
  Cworkholders = new (WH_Correlate*)[itsDestSteps];
  Csteps       = new (Step*)[itsDestSteps];
  
  // now go and create the source and destination steps
  // the two loops do duplicate quite some code, so a private method  
  // should be made later...

  // Create the Source Steps
  int timeDim = params.getInt("times",1);
  itsTimeDim = timeDim;
  int freqDim = params.getInt("freqbandsize",4096);
  itsFreqs  = freqDim;
  for (int iStep = 0; iStep < itsSourceSteps; iStep++) {
    
    // Create the Source Step
    sprintf(name, "Filler[%d]", iStep);
    Sworkholders[iStep] = new WH_FillTFMatrix(name,
					      iStep, // source ID
					      0,     // NO inputs
					      itsDestSteps, //nout
					      timeDim,
					      freqDim);
    
    Ssteps[iStep] = new Step(Sworkholders[iStep], "PipelineSourceStep", iStep);

    Ssteps[iStep]->runOnNode(iStep ,0); // run in App 0
  }

//    // The intermediate steps
//    for (int iStep = 0; iStep < itsSourceSteps; iStep++) {
//      sprintf(name, "Delay[%d]", iStep);
//      Iworkholders[iStep] = new WH_Delay(name,
//  				       itsSourceSteps, //nin
//  				       itsSourceSteps, // nout
//  				       timeDim,
//  				       freqDim); 
    
//      Isteps[iStep] = new Step(Iworkholders[iStep], "DelayStep", iStep);

//      // Determine the node and process to run in
//      // ... sory, this should go in a private method later
//      Isteps[iStep]->runOnNode(iStep ,0); // run in App 0
//      Isteps[iStep]->connectInput(Ssteps[iStep]);
//    }


  // Create the destination steps
  for (int iStep = 0; iStep < itsDestSteps; iStep++) {
    
    // Create the Destination Step
    sprintf(name, "Pipeline[%d]", iStep);
    Dworkholders[iStep] = new WH_Transpose(name, 
					   itsSourceSteps, 
					   timeDim,
					   timeDim,
					   freqDim); 
    Dsteps[iStep] = new Step(Dworkholders[iStep], 
			     "PipelineDestStep", 
			     iStep);

    int node = 2*iStep+itsSourceSteps;
    Dsteps[iStep]->runOnNode(node,0); 



    // Create the correlator step
    sprintf(name, "Correlator[%d]", iStep);
    Cworkholders[iStep] = new WH_Correlate(name,
					   timeDim,        // inputs
					   1,              // outputs
					   itsSourceSteps, // stations
					   freqDim);       // frequency

    Csteps[iStep] = new Step(Cworkholders[iStep],
			     "Correlator",
			     iStep);
    Csteps[iStep]->runOnNode(node+1,0); 
    // connect the correlator to the corresponding transpose step
    Csteps[iStep]->connectInput(Dsteps[iStep]);
    Csteps[iStep]->setOutRate(50); // integration time of the correlator
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
      TRACER2("Pipeline; try to connect " << step << "   " << ch);
#ifdef HAVE_MPI
      Dsteps[step]->connect(Ssteps[ch],ch,step,1,TH_MPI::proto);
#else
#ifdef HAVE_CORBA
      Dsteps[step]->connect(Ssteps[ch],ch,step,1,TH_Corba::proto);
#else 
      Dsteps[step]->connect(Ssteps[ch],ch,step,1);
#endif
#endif

    }
  }
  //  simul.optimizeConnectionsWith(TH_ShMem::proto);
}

void Pipeline::run(int nSteps) {
  nSteps = nSteps;


  TRACER1("Ready with definition of configuration");
  Profiler::init();
  Step::clearEventCount();

  TRACER4("Start Processing simul " );    
#ifdef HAVE_MPI
  int rank = TRANSPORTER::getCurrentRank();
  TH_MPI::synchroniseAllProcesses();
  double starttime=MPI_Wtime();
#endif
  for (int i=1; i<=nSteps; i++) {
    if (i==1 && itsDoLogProfile) Profiler::activate();
    getSimul().process();
    if (i==10 && itsDoLogProfile) Profiler::deActivate();

#ifdef HAVE_MPI
    if ((rank==0) && (i%10000 == 0)) cout << i/(MPI_Wtime()-starttime) << endl;
#endif
  }
#ifdef HAVE_MPI
  double endtime=MPI_Wtime();
  //  cout << "Total Run Time on node " << TH_MPI::getCurrentRank() << " : " << endtime-starttime << endl;
  if (rank == 0) {
    float F = nSteps/(endtime-starttime);
    float B = itsFreqs*nSteps/(endtime-starttime)*itsTimeDim*itsDestSteps*4*8/1024/1024/1024;
    cout << "===> " 
	 <<  itsFreqs << "  " 
	 <<  F << "  "
	 <<  itsTimeDim << "  "
	 <<  itsSourceSteps << "  "
	 <<  itsDestSteps << "  " 
	 <<  B << "  "
         <<  B * itsSourceSteps << "  "
	 << endl;
  }
#endif


  TRACER4("END OF SIMUL on node " << TRANSPORTER::getCurrentRank () );
 
  //     close environment
  //  TRANSPORTER::finalize();
}

void Pipeline::dump() const {
  getSimul().dump();
}

void Pipeline::quit() {  
}

void Pipeline::undefine() {
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
