//  OnLineProto.cc:
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
#include <stdlib.h>
#include <Common/lofar_string.h>
#include <Common/KeyValueMap.h>
#include <Common/Debug.h>
#include <Common/lofar_iostream.h>
#include <OnLineProto/MAC.h>

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

//CEPFrame basics
#include <CEPFrame/Transport.h>
#include <CEPFrame/Step.h>
#include <CEPFrame/Simul.h>
#include <CEPFrame/WH_Empty.h>
#include <CEPFrame/Profiler.h>
#include <OnLineProto/OnLineProto.h>

//WorkHolders:
#include <OnLineProto/WH_SimStation.h>
#include <OnLineProto/WH_PreProcess.h>
#include <OnLineProto/WH_Transpose.h>
#include <OnLineProto/WH_Correlate.h>
#include <OnLineProto/WH_Dump.h>

//DataHolders:
#include <OnLineProto/DH_Beamlet.h>
#include <OnLineProto/DH_CorrCube.h>
#include <OnLineProto/DH_Vis.h>

//TransportHolders
#include <CEPFrame/ShMem/TH_ShMem.h>
#include <CEPFrame/TH_Mem.h>


#include TRANSPORTERINCLUDE


using namespace LOFAR;

OnLineProto::OnLineProto()
{
}

OnLineProto::~OnLineProto()
{
  undefine();
}

/**
   Define function for the OnLineProto application. 
   It defines the static structure of the pipeline
 */
void OnLineProto::define(const KeyValueMap& params)
{

#ifdef HAVE_MPI
  // TH_ShMem only works in combination with MPI
  TH_ShMem::init(0, NULL);
#endif
  
  // Free any memory previously allocated
  undefine();

  // Create the top-level Simul
  Simul app(new WH_Empty(), 
	    "OnLineProto",
	    true, 
	    true,  // controllable	      
	    true); // monitor
  setSimul(app);

  // Set node and application number of Simul
  app.runOnNode(0,0);
  app.setCurAppl(0);

  // Get any extra params from input
  int NStations = params.getInt("stations",NSTATIONS);
  //  int NBeamlets = params.getInt("beamlets",32);
  int NBeamlets = (NBEAMLETS);
  //  int myFBW     = params.getInt("beamlet_FBW",256);
  int myFBW= (BFBW);

  // Create a global settings MAC
  MAC myMac;
  char str[8];


  ////////////////////////////////////////////////////////////////
  //
  // create the station steps
  //
  ////////////////////////////////////////////////////////////////
  WH_SimStation* myWHStations[NStations];
  Step*          myStationSteps[NStations];

  for (int s=0; s < NStations; s++) {
    sprintf (str, "%d", s+1);
    // ToDo: pass stationID, freq offset etc. to DH
    myWHStations[s] = new WH_SimStation("noname",  // name,
					myMac.getNumberOfBeamlets(),  // nout
					string("/home/alex/temp/signal_")+str+string(".txt"),
					myMac,
					s);


    myStationSteps[s] = new Step(myWHStations[s],"noname");
    myStationSteps[s]->runOnNode(0,0);
    app.addStep(myStationSteps[s]);
  }
  ////////////////////////////////////////////////////////////////
  //
  // create the preproces steps
  //
  ////////////////////////////////////////////////////////////////
  WH_PreProcess* myWHPreProcess[NStations];
  Step*          myPreProcessSteps[NStations];

  for (int s=0; s < NStations; s++) {
    // ToDo: pass stationID, freq offset etc. to DH
    myWHPreProcess[s] = new WH_PreProcess("noname",  // name,
					  myMac.getNumberOfBeamlets(),  // channels
					  myMac,
					  s);


    myPreProcessSteps[s] = new Step(myWHPreProcess[s],"noname");
    myPreProcessSteps[s]->runOnNode(0,0);
    app.addStep(myPreProcessSteps[s]);
    // connect the preprocess step to the station step
    myPreProcessSteps[s]->connectInput(myStationSteps[s]);
  }
  
  ////////////////////////////////////////////////////////////////
  //
  // create the Transpose steps
  //
  ////////////////////////////////////////////////////////////////
  WH_Transpose*  myWHTranspose[NBeamlets];
  Step*          myTransposeSteps[NBeamlets];

  for (int b=0; b < NBeamlets; b++) {
    // ToDo: pass stationID, freq offset etc. to DH
    myWHTranspose[b] = new WH_Transpose("noname",  // name,
					NStations,  // nin
					NVis, //nout
					myFBW // Freq BandWidth for beamlet
					);


    myTransposeSteps[b] = new Step(myWHTranspose[b],"noname");
    myTransposeSteps[b]->runOnNode(0,0);
    myTransposeSteps[b]->setOutRate(TSIZE);
    app.addStep(myTransposeSteps[b]);
  }


  ////////////////////////////////////////////////////////////////
  //
  // connect the Transpose steps to the preprocessors;
  // connection scheme implements transpose function
  //
  ////////////////////////////////////////////////////////////////
  for (int b = 0; b < NBeamlets; b++) {
    for (int s = 0; s < NStations; s++) {
      TRACER2("Pipeline; try to connect " << b << "   " << s);    
      myTransposeSteps[b]->connect(myPreProcessSteps[s],s,b,1);
    }
  }
  
  ////////////////////////////////////////////////////////////////
  //
  // create the Correlator steps
  //
  ////////////////////////////////////////////////////////////////
  WH_Correlate*  myWHCorrelators[NCorr];
  Step*          myCorrelatorSteps[NCorr];

  for (int c=0; c < NCorr; c++) {
    // ToDo: pass stationID, freq offset etc. to DH
    myWHCorrelators[c] = new WH_Correlate("noname",     // name,
					  1             // channels
					  );
    

    myCorrelatorSteps[c] = new Step(myWHCorrelators[c],"noname");
    myCorrelatorSteps[c]->runOnNode(0,0);
    myCorrelatorSteps[c]->setRate(TSIZE);
    app.addStep(myCorrelatorSteps[c]);
  }

  ////////////////////////////////////////////////////////////////
  //
  // connect the preprocess step to the station step
  //
  ////////////////////////////////////////////////////////////////
  int correlator=0;
  for (int t=0; t<NBeamlets; t++) {
    for (int c=0; c<NVis; c++) {
      myCorrelatorSteps[correlator]->connect(myTransposeSteps[t],0,c,1);
      correlator++;
    }
  }
  DbgAssertStr(correlator == NCorr,"error in correlator connection logic");



  ////////////////////////////////////////////////////////////////
  //
  // create the Dump steps
  //
  ////////////////////////////////////////////////////////////////
  WH_Dump*  myWHDumps[NCorr];
  Step*          myDumpSteps[NCorr];

  for (int s=0; s < NCorr; s++) {
    // ToDo: pass stationID, freq offset etc. to DH
    myWHDumps[s] = new WH_Dump("noname",1);    

    myDumpSteps[s] = new Step(myWHDumps[s],"noname");
    myDumpSteps[s]->runOnNode(NCorr,0);
    myDumpSteps[s]->setRate(TSIZE);
    app.addStep(myDumpSteps[s]);
    // connect the preprocess step to the station step
    myDumpSteps[s]->connectInput(myCorrelatorSteps[s]);
  }

  // Create the cross connections between Steps
  // Example:     #ifdef HAVE_MPI
  //              targetStep.connect(sourceStep, 0, 0, 1, TH_MPI::proto);
  //              #endif


  // Optional: Performance optimisations
  // Example:     #ifdef HAVE_MPI
  //                app.optimizeConnectionsWith(TH_ShMem::proto);
  //              #endif 

}
  

void OnLineProto::run(int nSteps) {
  TRACER1("Call run()");
  Profiler::init();
  Step::clearEventCount();

  TRACER4("Start Processing app");    
  for (int i=0; i<nSteps; i++) {
    if (i==2) Profiler::activate();
    TRACER2("Call app.process() ");
    getSimul().process();
    if (i==5) Profiler::deActivate();
  }

  TRACER4("END OF SIMUL on node " << TRANSPORTER::getCurrentRank () );
 
#if 0
  //     close environment
  TRANSPORTER::finalize();
#endif

}

void OnLineProto::dump() const {
  getSimul().dump();
}

void OnLineProto::quit() {  
}

void OnLineProto::undefine() {
  // Clean up
}
