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
//  Revision 1.2  2002/11/20 11:05:43  schaaf
//
//  %[BugId: 117]%
//
//  working initial version for Scali
//
//  Revision 1.1.1.1  2002/11/13 15:58:06  schaaf
//  %[BugId: 117]%
//
//  Initial working version
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


Pipeline::Pipeline():
  itsSourceSteps(0),
  itsDestSteps(0)
{
  FillWH          = NULL;
  FillSteps       = NULL;
  TransWH         = NULL;
  TransSteps      = NULL;
  CorrWH          = NULL;
  CorrSteps       = NULL;
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
#ifdef HAVE_MPI
  // TH_ShMem only works in combination with MPI
  // initialize TH_ShMem
  TH_ShMem::init(0, NULL);
#endif
  
  
  int rank = TRANSPORTER::getCurrentRank();
  unsigned int size = TRANSPORTER::getNumberOfNodes();
  
  // free any memory previously allocated
  undefine();

  TRACER2("Pipeline Processor " << rank << " of " << size << " operational.");

  WH_Empty empty;
  Simul simul(empty, params.getString("name","Pipeline").c_str(),true,false);
  setSimul(simul);
  // the top-level simul

  
  TRACER2("Handle the input parameters");
  if (rank == 0) params.show (cout);
  int Pols       = params.getInt("polarisations",2); 
  itsSourceSteps = params.getInt("stations",3); 
  itsDestSteps   = params.getInt("correlators",3);
  itsDoLogProfile = params.getInt("log",0) == 1;
   
  int applicationnr = params.getInt("application",0);   
  simul.runOnNode(1,applicationnr);
  simul.setCurAppl(applicationnr);
  // now go and create the source and destination steps
  // the two loops do duplicate quite some code, so a private method  
  // should be made later...

  ////////////////////////////////////////////////////////////////////////////
  // Create the Source Steps
  int timeDim = params.getInt("times",1);
  itsTimeDim = timeDim;
  int freqDim = params.getInt("freqbandsize",4096);
  itsFreqs  = freqDim;
  FillWH       = new (WH_FillTFMatrix*)[itsSourceSteps];
  FillSteps    = new (Step*)[itsSourceSteps];

  for (int iStep = 0; iStep < itsSourceSteps; iStep++) {  
    char name[40];  
    sprintf(name, "Filler[%d]", iStep);
    FillWH[iStep] = new WH_FillTFMatrix(name,
					iStep, // source ID
					0,     // NO inputs
					itsDestSteps, //nout
					timeDim,
					freqDim,
					Pols); 
    
    FillSteps[iStep] = new Step(FillWH[iStep],
				"PipelineSourceStep", 
				iStep);
    FillSteps[iStep]->runOnNode(iStep ,0); // run in App 0
    simul.addStep(FillSteps[iStep]);
  }


  //////////////////////////////////////////////////////////////////////////////////////
  // Create the transpose steps
  TransWH      = new (WH_Transpose*)[itsDestSteps];
  TransSteps   = new (Step*)[itsDestSteps];
  for (int iStep = 0; iStep < itsDestSteps; iStep++) {
    
    // Create the Destination Step
    char name[40];  
    sprintf(name, "Pipeline[%d]", iStep);
    TransWH[iStep] = new WH_Transpose(name, 
				      itsSourceSteps, 
				      timeDim,
				      timeDim,
				      freqDim,
				      Pols);  
    TransSteps[iStep] = new Step(TransWH[iStep], 
				 "PipelineDestStep", 
				 iStep);
    
    int node = 2*iStep+itsSourceSteps;
    TransSteps[iStep]->runOnNode(node,0); 
    simul.addStep(TransSteps[iStep]);
  }    
  
  
  /////////////////////////////////////////////////////////////////////////////////////////
  // Create the correlator step
  
  CorrWH       = new (WH_Correlate*)[itsDestSteps];
  CorrSteps    = new (Step*)[itsDestSteps];
  for (int iStep = 0; iStep < itsDestSteps; iStep++) {
    char name[40];  
    sprintf(name, "Correlator[%d]", iStep);
    CorrWH[iStep] = new WH_Correlate(name,
				     timeDim,        // inputs
				     1,              // outputs
				     itsSourceSteps, // stations
				     freqDim,        // frequency
				     Pols);             
    
    CorrSteps[iStep] = new Step(CorrWH[iStep],
				"Correlator",
				iStep);
    int node = 2*iStep+itsSourceSteps; // same as for transpose step
    CorrSteps[iStep]->runOnNode(node+1,0); 
    // connect the correlator to the corresponding transpose step
    CorrSteps[iStep]->connectInput(TransSteps[iStep]);
    CorrSteps[iStep]->setOutRate(50); // integration time of the correlator
    simul.addStep(CorrSteps[iStep]);
  }

  // Create the cross connections between filler and transpose steps
  for (int step = 0; step < itsDestSteps; step++) {
    for (int ch = 0; ch < itsSourceSteps; ch++) {
      // Set up the connections
      // Correlator Style
      TRACER2("Pipeline; try to connect " << step << "   " << ch);
#ifdef HAVE_MPI
      TransSteps[step]->connect(FillSteps[ch],ch,step,1,TH_MPI::proto);
#else
      TransSteps[step]->connect(FillSteps[ch],ch,step,1);
#endif
      
    }
  }

  // Now some performance optimisations....
#ifdef HAVE_MPI
  // replace connections with TH_ShMem where possible
  simul.optimizeConnectionsWith(TH_ShMem::proto);
#endif
  // replace connections with TH_Mem where possible
  simul.optimizeConnectionsWith(TH_Mem::proto);
}

void Pipeline::run(int nSteps) {
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
    //TH_MPI::synchroniseAllProcesses();
#endif
  }
#ifdef HAVE_MPI
  double endtime=MPI_Wtime();
  cout << "Total Run Time on node " 
       << TH_MPI::getCurrentRank() << " : " 
       << endtime-starttime << endl;
#endif
  
  
  TRACER4("END OF SIMUL on node " << TRANSPORTER::getCurrentRank () );
  
  //     close environment
  //  TRANSPORTER::finalize();
}

void Pipeline::dump() const {
  getSimul().dump();
}

void Pipeline::quit() {  
  TRACER2("Pipeline::quit");
}

void Pipeline::undefine() {
  TRACER2("Enter Pipeline::undefine");
  if (FillWH) 
    for (int iStep = 0; iStep < itsSourceSteps; iStep++) 
      delete FillWH[iStep];
  if (FillSteps) 
    for (int iStep = 0; iStep < itsSourceSteps; iStep++) 
      delete FillSteps[iStep];
  delete [] FillWH;
  delete [] FillSteps;
  if (TransWH) 
    for (int iStep = 0; iStep < itsDestSteps; iStep++) 
      delete TransWH[iStep];
  if (TransSteps) 
    for (int iStep = 0; iStep < itsDestSteps; iStep++) 
      delete TransSteps[iStep];
  delete [] TransWH;
  delete [] TransSteps;
  if (CorrWH) 
    for (int iStep = 0; iStep < itsDestSteps; iStep++) 
      delete CorrWH[iStep];
  if (CorrSteps) 
    for (int iStep = 0; iStep < itsDestSteps; iStep++) 
      delete CorrSteps[iStep];
  delete [] CorrWH;
  delete [] CorrSteps;
  TRACER2("Leaving Pipeline::undefine");
}
