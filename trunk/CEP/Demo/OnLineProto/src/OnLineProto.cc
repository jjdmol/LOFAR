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

//#include <lofar_config.h>

#include <stdio.h>
#include <stdlib.h>
#include <Common/lofar_string.h>
#include <Common/KeyValueMap.h>
//#include <Common/Debug.h>
#include <Common/lofar_iostream.h>
#include <ACC/ParameterSet.h>

//#ifdef HAVE_CONFIG_H
//#include <config.h>
//#endif

//CEPFrame basics
#include <CEPFrame/Step.h>
#include <CEPFrame/Composite.h>
#include <CEPFrame/WH_Empty.h>
#include <tinyCEP/Profiler.h>
#include <OnLineProto/OnLineProto.h>

//WorkHolders:
#include <OnLineProto/WH_SimStation.h>
#include <OnLineProto/WH_PreProcess.h>
#include <OnLineProto/WH_Transpose.h>
#include <OnLineProto/WH_Correlate.h>
#include <OnLineProto/WH_Dump.h>
#include <OnLineProto/WH_FringeControl.h>
#include <OnLineProto/WH_Merge.h>

//DataHolders:
#include <OnLineProto/DH_Beamlet.h>
#include <OnLineProto/DH_CorrCube.h>
#include <OnLineProto/DH_Vis.h>
#include <OnLineProto/DH_CorrectionMatrix.h>

//TransportHolders
#include <Transport/TH_ShMem.h>
#include <Transport/TH_Mem.h>


#include TRANSPORTERINCLUDE
#define nonblocking false

using namespace LOFAR;
using namespace LOFAR::ACC;

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

  // the default transportholder
  TH_Mem THproto;

#ifdef HAVE_MPI
  // TH_ShMem only works in combination with MPI
  TH_ShMem::init(0, NULL);
#endif
  
  // Free any memory previously allocated
  undefine();

  // Create the top-level Composite
  Composite app(new WH_Empty(), 
	    "OnLineProto",
	    true, 
	    true,  // controllable	      
	    true); // monitor
  setComposite(app);

  // Set node and application number of Composite
  app.runOnNode(0,0);
  app.setCurAppl(0);

  char str[8];

  // Read in the parameter sets
  const ACC::ParameterSet myPS(params.getString("psfile", "params.ps"));

  ////////////////////////////////////////////////////////////////
  //
  // create the station steps
  //
  ////////////////////////////////////////////////////////////////
  vector<WH_SimStation*> myWHStations;
  vector<Step*>          myStationSteps;

  for (int s=0; s < myPS.getInt("general.nstations"); s++) {
    // ToDo: pass stationID, freq offset etc. to DH
    if (s==0){
      myWHStations.push_back(new WH_SimStation("mysimstation",  // name,
					       myPS.getInt("station.nbeamlets"),  // nout
					       string("signal_1.txt"),
					       myPS,
					       s));
    } else {
      myWHStations.push_back(new WH_SimStation("mysimstation",  // name,
					       myPS.getInt("station.nbeamlets"),  // nout
					       string("signal_2.txt"),
					       myPS,
					       s));
    }

    myStationSteps.push_back(new Step(myWHStations[s],"mystationstep"));
    myStationSteps[s]->runOnNode(0,0);
    app.addStep(myStationSteps[s]);
  }

  ////////////////////////////////////////////////////////////////
  //
  // create the fringe control step
  //
  ////////////////////////////////////////////////////////////////
  WH_FringeControl myWHFringeControl("myfringecontrol", myPS.getInt("general.nstations"), myPS);
  Step myFringeControl(myWHFringeControl, "myfringecontrol_step");
  myFringeControl.runOnNode(0,0);
  app.addStep(myFringeControl);
 
  ////////////////////////////////////////////////////////////////
  //
  // create the merge step
  //
  ////////////////////////////////////////////////////////////////
  WH_Merge myWHMerge("mymerge", 1, myPS.getInt("general.nstations"),
		     myPS.getInt("general.nstations"), 
		     myPS.getInt("station.nchannels"));
  Step myMerge(myWHMerge, "mymergestep");
  myMerge.runOnNode(0,0);
  app.addStep(myMerge);  

  // connect the fringe control step to the merge step
  myMerge.connectInput(&myFringeControl,THproto,nonblocking);

  ////////////////////////////////////////////////////////////////
  //
  // create the preproces steps
  //
  ////////////////////////////////////////////////////////////////
  vector<WH_PreProcess*> myWHPreProcess;
  vector<Step*>          myPreProcessSteps;

  for (int s=0; s < myPS.getInt("general.nstations"); s++) {
    // ToDo: pass stationID, freq offset etc. to DH
    myWHPreProcess.push_back(new WH_PreProcess("mypreprocess",  // name,
					       myPS.getInt("station.nbeamlets"),  // channels
					       myPS,
					       s));

    myPreProcessSteps.push_back(new Step(myWHPreProcess[s],"mypreprocess_step"));
    myPreProcessSteps[s]->runOnNode(0,0);
    app.addStep(myPreProcessSteps[s]);

    // connect the preprocess step to the station step
    for (int b = 0; b < myPS.getInt("station.nbeamlets"); b++) {
      myPreProcessSteps[s]->connect(myStationSteps[s], b, b, 1, THproto, nonblocking);
    }
     
    // connect the preprocess steps to the fringecontrol step
    myPreProcessSteps[s]->connect(&myMerge, myPS.getInt("station.nbeamlets"), s, 1, THproto, nonblocking);
  }
   
  ////////////////////////////////////////////////////////////////
  //
  // create the Transpose steps
  //
  ////////////////////////////////////////////////////////////////
  vector<WH_Transpose*>  myWHTranspose;
  vector<Step*>          myTransposeSteps;

   // create a Transpose step for each beamlet (raw freq channel)
  for (int b=0; b < myPS.getInt("station.nbeamlets"); b++) {
    // ToDo: pass stationID, freq offset etc. to DH
    myWHTranspose.push_back(new WH_Transpose("mytranspose",  // name,
					     myPS.getInt("general.nstations"),  // nin
					     myPS.getInt("corr.ncorr"),         //nout
					     myPS
					     ));


    myTransposeSteps.push_back(new Step(myWHTranspose[b],"mytranspose_step"));
    myTransposeSteps[b]->runOnNode(0,0);

    app.addStep(myTransposeSteps[b]);
  }


  ////////////////////////////////////////////////////////////////
  //
  // connect the Transpose steps to the preprocessors;
  // connection scheme implements transpose function
  //
  ////////////////////////////////////////////////////////////////
  for (int b = 0; b < myPS.getInt("station.nbeamlets"); b++) {
    for (int s = 0; s < myPS.getInt("general.nstations"); s++) {
      TRACER2("Pipeline!; try to connect " << b << "   " << s);    
      myTransposeSteps[b]->connect(myPreProcessSteps[s],s,b,1, THproto, nonblocking);
    }
  }

  ////////////////////////////////////////////////////////////////
  //
  // create the Correlator steps
  //
  ////////////////////////////////////////////////////////////////
  vector<WH_Correlate*>  myWHCorrelators;
  vector<Step*>          myCorrelatorSteps;

  for (int b=0; b < myPS.getInt("station.nbeamlets"); b++) { //NBeamlets
    for (int f=0; f < myPS.getInt("corr.ncorr"); f++) {
      int correlator = b * myPS.getInt("corr.ncorr") + f;
      
      // ToDo: pass stationID, freq offset etc. to DH
      myWHCorrelators.push_back(new WH_Correlate("mycorrelate",     // name,
						 1,                 // channels (=nin=nout)
						 myPS
						 ));
      

      myCorrelatorSteps.push_back(new Step(myWHCorrelators[correlator],"mycorrelate_step"));
      myCorrelatorSteps[correlator]->runOnNode(0,0);
      // todo: only output rate?
      myCorrelatorSteps[correlator]->getWorker()->getDataManager().setOutputRate(myPS.getInt("corr.tsize"));
      app.addStep(myCorrelatorSteps[correlator]);

      ////////////////////////////////////////////////////////////////
      //
      // connect the correlator steps to the transpose steps
      // each Transpose step will be connected to "corr.ncorr" 
      // correlator steps.
      ////////////////////////////////////////////////////////////////
      LOG_TRACE_LOOP_STR("Call: myCorrelatorSteps["<<correlator<<"]->connect(myTransposeSteps["
			 <<b<<"],"<<f<<",0,1, THproto, nonblocking)");
      myCorrelatorSteps[correlator]->connect(myTransposeSteps[b],0,f,1, THproto, nonblocking);
    }
  }


  ////////////////////////////////////////////////////////////////
  //
  // create the Dump steps
  //
  ////////////////////////////////////////////////////////////////
  vector<WH_Dump*>  myWHDumps;
  vector<Step*>     myDumpSteps;


  for (int s=0; s < myPS.getInt("corr.ncorr") * myPS.getInt("station.nbeamlets"); s++) {
    // ToDo: pass stationID, freq offset etc. to DH

    myWHDumps.push_back(new WH_Dump("mydump",1,myPS,s));

    myDumpSteps.push_back(new Step(myWHDumps[s],"mydump_step"));
    myDumpSteps[s]->runOnNode(0,0);

    myDumpSteps[s]->getWorker()->getDataManager().setInputRate(myPS.getInt("corr.tsize"));
    myDumpSteps[s]->getWorker()->getDataManager().setProcessRate(myPS.getInt("corr.tsize"));
    myDumpSteps[s]->getWorker()->getDataManager().setOutputRate(myPS.getInt("corr.tsize"));
    app.addStep(myDumpSteps[s]);
    // connect the preprocess step to the station step
    myDumpSteps[s]->connectInput(myCorrelatorSteps[s], THproto, nonblocking);
  }
}
  

void OnLineProto::run(int nSteps) {
  TRACER1("Call run()");
  Profiler::init();
  Step::clearEventCount();

  TRACER4("Start Processing app");    
  for (int i=0; i<nSteps; i++) {
    if (i==2) Profiler::activate();
    TRACER2("Call app.process() ");
    getComposite().process();
    if (i==5) Profiler::deActivate();
  }

  TRACER4("END OF SIMUL on node " << TRANSPORTER::getCurrentRank () );
 
#if 0
  //     close environment
  TRANSPORTER::finalize();
#endif

}

void OnLineProto::dump() const {
  getComposite().dump();
}

void OnLineProto::quit() {  
}

void OnLineProto::undefine() {
  // Clean up
}
