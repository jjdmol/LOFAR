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
//////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <Common/lofar_iostream.h>
#include <stdlib.h>
#include <Common/lofar_string.h>
#include "Common/Debug.h"
#include "CEPFrame/Transport.h"
#include "CEPFrame/Step.h"
#include "Pipeline/Pipeline.h"
#include "CEPFrame/Simul.h"
#include "CEPFrame/Profiler.h"
#include "Pipeline/WH_FillTFMatrix.h"
#include "Pipeline/WH_Transpose.h"
#include "Pipeline/WH_PreCorrect.h"
#include "Pipeline/WH_Correlate.h"
#include "Pipeline/WH_Dump.h"
#include "CEPFrame/WH_Empty.h"
#include "CEPFrame/ShMem/TH_ShMem.h"
#include "CEPFrame/TH_Mem.h"
#include "CEPFrame/TH_File.h"
#include TRANSPORTERINCLUDE

string i2string(int i) {
  char str[32];
  sprintf(str, "%i", i);
  return string(str);
}

Pipeline::Pipeline():
  itsStations(0),
  itsCorrelators(0)
{
  FillSteps       = NULL;
  TransSteps      = NULL;
  PreCSteps       = NULL;
  CorrSteps       = NULL;
}

Pipeline::~Pipeline()
{
  undefine();
}

void Pipeline::define(const ParamBlock& params)
{
#ifdef HAVE_MPI
  TH_ShMem::init(0, NULL);
#endif
  
  // free any memory previously allocated
  undefine();

  // Create the top-level simul
  Simul simul(new WH_Empty(), params.getString("name","Pipeline").c_str(),true,false);
  setSimul(simul);

  
  TRACER2("Handle the input parameters");
  if (TRANSPORTER::getCurrentRank() <= 0) params.show (cout);
  int Pols          = params.getInt("polarisations",2); 
  itsStations       = params.getInt("stations",3); 
  itsCorrelators    = params.getInt("correlators",3);
  itsDoLogProfile   = params.getInt("log",0) == 1;   
  int timeDim       = params.getInt("times",1);
  int freqDim       = params.getInt("freqbandsize",4096);
  bool InRead       = params.getInt("inread",0) == 1;
  bool InWrite      = params.getInt("inwrite",0) == 1;
  bool OutWrite     = params.getInt("outwrite",0) == 1;
  DbgAssertStr (!(InRead  && InWrite), "Cannot read and write at the same time")
  DbgAssertStr (!(InWrite && OutWrite), "Cannot write two times")

  simul.runOnNode(1,0);
  simul.setCurAppl(0);


  // now go and create the source and destination steps
  //

  //Only create the Fill and Transpose steps if we do not read from input files
  
  ////////////////////////////////////////////////////////////////////////////
  // Create the Source Steps  
  FillSteps    = new (Step*)[itsStations];
  for (int iStep = 0; iStep < itsStations; iStep++) {  
    FillSteps[iStep] = new Step(WH_FillTFMatrix(string("Filler_" + i2string(iStep)),
						iStep, // source ID
						0,     // NO inputs
						itsCorrelators, //nout
						timeDim,
						freqDim,
						Pols),
				"PipelineSourceStep", 
				iStep);
    FillSteps[iStep]->runOnNode(iStep ,0); // run in App 0
    if (!InRead) simul.addStep(FillSteps[iStep]);
  }
  
  
  //////////////////////////////////////////////////////////////////////////////////////
  // Create the transpose steps
  TransSteps   = new (Step*)[itsCorrelators];
  for (int iStep = 0; iStep < itsCorrelators; iStep++) {
    
    // Create the Destination Step
    TransSteps[iStep] = new Step(WH_Transpose("Transpose_" + i2string(iStep), 
					      itsStations, 
					      timeDim,
					      timeDim,
					      freqDim,
					      Pols), 
				 "PipelineDestStep", 
				 iStep);
    
    int node = 2*iStep+itsStations;
    TransSteps[iStep]->runOnNode(node,0); 
    if (!InRead) simul.addStep(TransSteps[iStep]);
  }    
  
  
  /////////////////////////////////////////////////////////////////////////////////////////
  // Create the cross connections between filler and transpose steps
  for (int step = 0; step < itsCorrelators; step++) {
    for (int ch = 0; ch < itsStations; ch++) {
      TRACER2("Pipeline; try to connect " << step << "   " << ch);
#ifdef HAVE_MPI
      TransSteps[step]->connect(FillSteps[ch],ch,step,1,TH_MPI::proto);
#else
      TransSteps[step]->connect(FillSteps[ch],ch,step,1);
#endif 
    }
  }
  

  /////////////////////////////////////////////////////////////////////////////////////////
  // Create the PreCorrection step
  PreCSteps    = new (Step*)[itsCorrelators];
  for (int iStep = 0; iStep < itsCorrelators; iStep++) {
    PreCSteps[iStep] = new Step(WH_PreCorrect("PreCorrection_" + i2string(iStep),
					      timeDim,        // inputs
					      itsStations, // stations
					      freqDim,        // frequency
					      Pols),
				"PreCorrection",
				iStep);
    int node = 2*iStep+itsStations; // same as for transpose step
    PreCSteps[iStep]->runOnNode(node,0); 
    // connect the PreCorrect to the corresponding transpose step
    // Find out what TransportHolders we need where
    if (InRead) {
      // The PreCorrect step will read from file
      PreCSteps[iStep]->connectInput(TransSteps[iStep], 
				     TH_File("PreC_Input_" + i2string(iStep),TH_File::Read));
          
    } else if (InWrite) {
      // The PreCorrect step will write to file
      PreCSteps[iStep]->connectInput(TransSteps[iStep],
				     TH_File("PreC_Input_" + i2string(iStep),TH_File::Write));
    } else {
      PreCSteps[iStep]->connectInput(TransSteps[iStep]); 
    }
    simul.addStep(PreCSteps[iStep]);
 
  }
  
  if (!InWrite) { // rest of simulation not needed when only generating input data

    /////////////////////////////////////////////////////////////////////////////////////////
    // Create the correlator step  
    CorrSteps    = new (Step*)[itsCorrelators];
    for (int iStep = 0; iStep < itsCorrelators; iStep++) {
      CorrSteps[iStep] = new Step(WH_Correlate("Correlator_" + i2string(iStep),
					       timeDim,        // inputs
					       1,              // outputs
					       itsStations, // stations
					       freqDim,        // frequency
					       Pols),
				  "Correlator",
				  iStep);
      int node = 2*iStep+itsStations; // same as for transpose step
      CorrSteps[iStep]->runOnNode(node+1,0); 
      CorrSteps[iStep]->connectInput(PreCSteps[iStep]);
      CorrSteps[iStep]->setOutRate(50); // integration time of the correlator
      simul.addStep(CorrSteps[iStep]);
    }

    /////////////////////////////////////////////////////////////////////////////////////
    // Create the WH_Dump and allow for TH_File connection
    Step DumpStep(WH_Dump("DumpWH",
			  itsCorrelators, // inputs
			  itsStations, // stations
			  Pols),
		  "DumpStep");
    DumpStep.setInRate(50); // integration time of the correlator
    DumpStep.runOnNode(itsStations+2*itsCorrelators);
    simul.addStep(DumpStep);
    for (int i=0; i< itsCorrelators; i++) {
      if (OutWrite) {
	TRACER2("Make Correlator Output dump to file connection");
	DumpStep.connect(CorrSteps[i],i,0,1,
			 TH_File("Correlator_DUMP_" + i2string(i),TH_File::Write));
      }
    }


  }  
  
  // Now some performance optimisations....
  if (!(InRead || InWrite || OutWrite)) { // but only when TH_File is not used...
#ifdef HAVE_MPI
    // replace connections with TH_ShMem where possible
    simul.optimizeConnectionsWith(TH_ShMem::proto);
#endif
    // replace connections with TH_Mem where possible
    simul.optimizeConnectionsWith(TH_Mem::proto);
  }
}




void Pipeline::run(int nSteps) {
  TRACER1("Call Pipeline::run()");
  Profiler::init();
  Step::clearEventCount();

  TRACER4("Start Processing simul " );    
  int rank;
  double starttime;
#ifdef HAVE_MPI
  rank = TRANSPORTER::getCurrentRank();
  TH_MPI::synchroniseAllProcesses();
  starttime=MPI_Wtime();
#endif

  // The main loop
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
  if (FillSteps) 
    for (int iStep = 0; iStep < itsStations; iStep++) 
      delete FillSteps[iStep];
  delete [] FillSteps;
  if (TransSteps) 
    for (int iStep = 0; iStep < itsCorrelators; iStep++) 
      delete TransSteps[iStep];
  delete [] TransSteps;
  if (CorrSteps) 
    for (int iStep = 0; iStep < itsCorrelators; iStep++) 
      delete CorrSteps[iStep];
  delete [] CorrSteps;
  if (PreCSteps) 
    for (int iStep = 0; iStep < itsCorrelators; iStep++) 
      delete PreCSteps[iStep];
  delete [] PreCSteps;
  TRACER2("Leaving Pipeline::undefine");
}

