// Simulation.cpp
// This is the main program for the LOFAR prototype simulation using the 
// LOFARSim simulation environment.
// 
// In the current version the definition of the simulation steps that form the
// simulation is done in SysDef.h/cpp. This produces a multi-dimensional array with
// parameters that are used during the construction of the Steps and Simuls.
//

#include "Transport.h"
#include "Step.h"
#include "ParamBlock.h"
#include "general.h"
//#include "geometry.h"

#include "LOFARSim.h"

#include "Simul.h"
//#include "WH_Antenna.h"
#include "WH_WAV.h"
#include "WH_FFT.h"
#include "WH_TransF.h"
#include "WH_Beam.h"
#include "WH_TransB.h"
#include "WH_Station.h"
#include "WH_Corr.h"
#include "WH_Empty.h"
#include "WH_DataProc.h"
#include "WH_CPInput.h"
#include "WH_Integrator.h"
#include "WH_Convolve.h"
#include "WH_MakeMS.h"
#include "WH_Controller.h"
#include "Profiler.h"
#include "TH_MPI.h"
#include "TH_Mem.h"
#include TRANSPORTERINCLUDE

#ifdef CORBA_
#include "BS_Corba.h"
#include "TH_Corba.h"
#endif

#include <unistd.h>
#include <iostream.h>
#include <stdlib.h>
#include <string>
#include <map>

#define CORR_FILE       "./WAVE/Corr.ext"
#define CORR_TMP_FILE   "./WAVE/Corr.tmp"
#define WAVE_FILE_1 "./WAVE/antenna1.wav"
#define CORRNODE (STATIONS+1)

#ifdef CORBA_
int atexit(void (*function)(void))
{
  return 0;
}
#endif

LOFARSim::LOFARSim()
: itsFiller (0)
{
  // disable the RunInApp settings for all apps by default
  // later on, applications can be activated by setting to 0;
  for (int i=0; i<10; i++) itsRunInApp[i]=0;//999;
}


void LOFARSim::define(const ParamBlock&)
{
  const TH_Mem   TH_Mem_Proto;
#ifdef MPI_
  const TH_MPI   TH_MPI_Proto;
#endif

#ifdef CORBA_
  const TH_Corba TH_Corba_Proto;

  // Start Orb Environment
  Firewall::Assert(BS_Corba::init(),
		   __HERE__,
		   "Could not initialise CORBA environment");
#endif

  // Set the Application number
  int    argc = 0;
  char** argv = NULL;
  int App=0;
  getarg(&argc, &argv);
  if (argc == 3)
  {
    if (!strncmp(argv[1], "-app", 4))
    {
      App = atoi(argv[2]);
      cout << "Application = " << App << endl;
    }
  }
  // App=2;
  int itsSkipApp1=0;
  int itsSkipApp2=0;
  if (App==2) {
    itsSkipApp1=999;
    itsRunInApp[2]=0;
    cout << "Skip Application 1 code " << endl;
    sleep(1);
  }
  if (App==1) {
    itsSkipApp2=999;
    itsRunInApp[1]=0;
    cout << "Skip Application 2 code " << endl;
    sleep(1);
  }


  unsigned int rank = TRANSPORTER::getCurrentRank ();
  unsigned int size = TRANSPORTER::getNumberOfNodes();
  cout << "LOFARSim Processor " << rank << " of " 
       << size << "  operational." << flush << endl;
  //if (rank == 1) sleep(30);

  // counters for the construction loops
  int element, freq, station, beam, freqband;

  // create the main Simul; Steps and Simuls will be added to this one
  // Uses an empty workholder.

  Simul* simul = new Simul(new WH_Empty(),"LofarSim",0);
  setSimul (*simul);

  // define the arrays of Simuls and Steps to filled later
  Simul *theStations[STATIONS];
  Step  *fft[ELEMENTS];
  Step  *transf;		// forwards transpose
  Step  *beamformer[FREQS];
  Step  *transb;		// backwardss transpose

  Simul *dataprocessor;
  Step  *cpinput[STATIONS];

  // Now start defining stations. 
  for (station = 0; station < STATIONS; station++)    {  
        
    // Station simuls will be created on every node, even if it has no substeps
    theStations[station] =
      new Simul (new WH_Station (ELEMENTS,1),
		 "Lofar Station",
		 0);
    theStations[station]->runOnNode(station+itsSkipApp1);
    ////////////////////////////////////////////////////////////////////////
    // First put the antenna's in the Sation Simul
    for (element = 0; element < ELEMENTS; element++)
      {

	antenna[station][element] =
	  new Step (new WH_WAV(1,1,station),
		    "Antenna", 
		    0);
	antenna[station][element]->runOnNode(station+itsSkipApp1);
	theStations[station]->addStep (antenna[station][element]);
      }
    
    //////////////////////////////////////////////////////////////////////////
    // Add FFTs to the station
      
    for (element = 0; element < ELEMENTS; element++) {
	  fft[element] = new Step(new WH_FFT(1,1),
				  "FFT",
				  0);
	  fft[element]->runOnNode(station+itsSkipApp1);
	  cout << "Connect FFT " ;
	  fft[element]->connectInput(antenna[station][element] /*,
								 TH_Corba_Proto*/);
	  theStations[station]->addStep(fft[element]); 
    }
    ///////////////////////////////////////////////////////////////////////////
    // Define the Forward transpose

    {
      // create the transpose
      transf =
	new Step (new WH_TransF (ELEMENTS,
				 FREQS),
		  "TransposeF",
		  0);
      transf->runOnNode(station+itsSkipApp1);
      transf->connectInputArray(fft,ELEMENTS);
      theStations[station]->addStep(transf);
    }

    //////////////////////////////////////////////////////////////////////////
    // Define beamformers

    for (freq = 0; freq < FREQS; freq++) {
	// create the beamformer
	beamformer[freq] =
	  new Step (new
		    WH_Beam (1,1),
		    "BeamFormer", 
		    0);
	beamformer[freq]->runOnNode(station+itsSkipApp1);
	theStations[station]->addStep (beamformer[freq]);
    } 
   // Connect the inputs of the beamformers to the transf step.
    transf->connectOutputArray(beamformer,FREQS);
    
	
    /////////////////////////////////////////////////////////////////////
    // Add backwards transpose
    {
      TRACER(debug,"Define TransB on node " << rank);

      transb =
	new Step (new WH_TransB (FREQS,1),
		  "TransposeB",
		  0);
      transb->runOnNode(station+itsSkipApp1);
      transb->connectInputArray(beamformer,FREQS);
      //transb->getWorker()->setProcMode(zeroes);
      theStations[station]->addStep (transb);
    }
	
    ///////////////////////
    // Station is filled now;
    // Set station communication parameters
	    
    theStations[station]->connectOutputToArray(&transb,1);
    simul->addStep (theStations[station]);      
  } // loop stations

  /***************************************************************************/
  {     
    for (station=0; station<STATIONS; station++) {
      cpinput[station] = new Step(new WH_CPInput(1,BEAMS*FREQBANDS),
				  "CPInput",
				  0);
      cpinput[station]->runOnNode(STATIONS+itsSkipApp1);
      simul->addStep(cpinput[station]); 
      cpinput[station]->connectInput(theStations[station]
				     /*,TH_Corba_Proto*/);
    }
    
      //    dataprocessor->connectInputToArray(cpinput,STATIONS);
  
    char correlatorname[80];
    for (beam = 0; beam < BEAMS; beam++) {
      for (freqband=0; freqband<FREQBANDS; freqband++) {
	// create the correlator
	sprintf(correlatorname,"Correlator_%2i_%2i",beam*FREQBANDS+freqband,(2*STATIONS+1));
	correlator[beam*FREQBANDS+freqband] =
	  new Step (new WH_Corr (STATIONS,1),
		    correlatorname,
		    0);
	simul->addStep (correlator[beam*FREQBANDS+freqband]);
	correlator[0]->runOnNode(CORRNODE+itsSkipApp2); 
	
	// Connect the correlators with the CPInput steps;
	// these connects are the typical crosses needed in correlation
	// The connects may be optimised for specific network
	// topologies or protocols (such as rings)
	for (station=0; station<STATIONS; station++) {
	  correlator[beam*FREQBANDS+freqband]->connect(cpinput[station],
						       station,
						       beam*FREQBANDS+freqband,
						       1   /*,
							     TH_Corba_Proto*/);
	}
      }
    } 
    
    
    //    simul.addStep(dataprocessor);
  }


//itsSiuml->setLocalComm();
  
  //////////////////////////////////////////////////////////////////////
  //
  // Finished configuration definition
  // 
  //////////////////////////////////////////////////////////////////////
  cout << "Ready with definition of configuration on node " << rank << endl;  
}



void LOFARSim::run (int nsteps)
{

  if (nsteps <= 0) nsteps=40;
  unsigned int rank = TRANSPORTER::getCurrentRank ();

  // create the time Controller; All the simuls will be synchronized with this object
  Simul Controller(new WH_Controller(),"Controller",CONTROLLER_NODE);

  //  for (int loops=0; loops<nsteps; loops++) {
        while (1) {

//      if (itsRunInApp[1]==0 && rank == 0)
//        ((WH_WAV*)antenna[0][0]->getWorker())->takeSamples();
//      if (itsRunInApp[1]==0 && rank == 1) 
//        ((WH_WAV*)antenna[1][0]->getWorker())->takeSamples();

    //    nsteps = LAGS;
    ((WH_Corr*)correlator[0]->getWorker())->openFile(CORR_TMP_FILE);
    



  // Produce process output only on node 0
  TRANSPORTER::synchroniseAllProcesses();
#ifdef MPI_
  double starttime=MPI_Wtime();
#endif
  Profiler::init();
  //  if (rank == 0) cout << endl <<  "Start Process" << endl;    
  //    for (int i = 1; i <= nsteps; i++) {
  //    if (i==4)   Profiler::activate();
  if (rank == (unsigned int)CONTROLLER_NODE) {
      Controller.getWorker()->process();
    } 
    if (rank != (unsigned int)CONTROLLER_NODE) TRANSPORTER::waitForBroadCast();
    TRANSPORTER::synchroniseAllProcesses();
    getSimul().process ();
    //    if (i==4)   Profiler::deActivate();
//    }
#ifdef MPI_
  double endtime=MPI_Wtime();
  cout << "Total Time on node " << rank << " : " << endtime-starttime << endl;
#endif
  //  cout << endl << "END OF SIMUL on node " << rank << endl;
 
  ((WH_Corr*)correlator[0]->getWorker())->closeFile();
  usleep(500);
  system("/bin/cp ./WAVE/Corr.tmp ./WAVE/Corr.ext");
  usleep(500);
  unlink(WAVE_FILE_1 ".ready"); 
  }
  
  return;
}

void LOFARSim::dump() const {
  cout << "Dump LOFARSim" << endl;
  getSimul().dump();
}


void LOFARSim::quit()
{
  delete itsFiller;
  itsFiller = 0;
}



