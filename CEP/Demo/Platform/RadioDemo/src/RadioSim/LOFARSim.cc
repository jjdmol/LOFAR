// Simulation.cpp
// This is the main program for the LOFAR prototype simulation using the 
// LOFARSim simulation environment.
// 
// In the current version the definition of the simulation steps that form the
// simulation is done in SysDef.h/cpp. This produces a multi-dimensional array with
// parameters that are used during the construction of the Steps and Simuls.
//

#include "TH_Mem.h"
#ifdef MPI_
#include "TH_MPI.h"
#endif
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
#include TRANSPORTERINCLUDE
#ifdef CORBA_
#include "BS_Corba.h"
#endif

#include <iostream.h>
#include <stdlib.h>
#include <string>
#include <map>


LOFARSim::LOFARSim()
: itsFiller (0)
{}


void LOFARSim::define(const ParamBlock&)
{
#ifdef MPI_
  TH_MPI thProtoInter;
#else
  TH_Mem thProtoInter;
#endif
  TH_Mem thProtoIntra;

#ifdef CORBA_
  // Start Orb Environment
  Firewall::Assert(BS_Corba::init(),
		   __HERE__,
		   "Could not initialise CORBA environment");
#endif

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
  setSimul (simul);

  // define the arrays of Simuls and Steps to filled later
#if 0
  Simul *theStations[STATIONS];
#endif
  Step  *antenna[ELEMENTS];
  Step  *fft[ELEMENTS];
  Step  *transf;		// forwards transpose
  Step  *beamformer[FREQS];
  Step  *transb[STATIONS];		// backwardss transpose

#if 0
  Simul *dataprocessor;
#endif
  Step  *cpinput[STATIONS];
  Step  *correlator[BEAMS*FCORR];
  // Step  *integrator[BEAMS][CORRELATORS];
  //   Step  *convolve[BEAMS][CORRELATORS];
  Step  *makeMS;

  Firewall::Assert(1 == FREQBANDS, __HERE__, "FREQBANDS != 1");

  // Now start defining stations. 
  for (station = 0; station < STATIONS; station++)    {  
    int stationnode = station+1;
        
#if 0
    // Station simuls will be created on every node, even if it has no substeps
    theStations[station] =
      new Simul (new WH_Station (ELEMENTS,1),
		 "Lofar Station",
		 0);
    theStations[station]->runOnNode(stationnode);
#endif

    ////////////////////////////////////////////////////////////////////////
    // First put the antenna's in the Sation Simul
    for (element = 0; element < ELEMENTS; element++)
      {
	  antenna[element] =
	    new Step (new WH_WAV(1,1,station*10), //WH_Antenna (1,1),
		      "Antenna", 
		      0);
	  antenna[element]->runOnNode(stationnode);
	  antenna[element]->connectInput(NULL, thProtoInter);
	  //            antenna[element]->getWorker()->setProcMode(ones);

	  // Set antenna position
	  // ((WH_Antenna*)antenna[element]->getWorker())
// 	    ->setPosition((STAT_POS[station][0])+(ANT_POS[element][0]),
// 			  (STAT_POS[station][1])+(ANT_POS[element][1]));
	  //	  float *pos=new float[2];

	  simul->addStep (antenna[element]);
        }
      
    //////////////////////////////////////////////////////////////////////////
    // Add FFTs to the station
      
    for (element = 0; element < ELEMENTS; element++) {
	  fft[element] = new Step(new WH_FFT(1,1),
				  "FFT",
				  0);
	  fft[element]->runOnNode(stationnode+STATIONS);
	  fft[element]->connectInput(antenna[element], thProtoInter);
	  //	    fft[element]->getWorker()->setProcMode(ones);
	  simul->addStep(fft[element]); 
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
      transf->runOnNode(stationnode+STATIONS+STATIONS);
      transf->connectInputArray(fft,ELEMENTS);
      simul->addStep(transf);
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
	beamformer[freq]->runOnNode(stationnode+STATIONS+STATIONS);
	simul->addStep (beamformer[freq]);
    } 
   // Connect the inputs of the beamformers to the transf step.
    transf->connectOutputArray(beamformer, FREQS, thProtoIntra);
    
	
    /////////////////////////////////////////////////////////////////////
    // Add backwards transpose
    {
      TRACER(debug,"Define TransB on node " << rank);

      transb[station] =
	new Step (new WH_TransB (FREQS,1),
		  "TransposeB",
		  0);
      transb[station]->runOnNode(stationnode+STATIONS+STATIONS);
      transb[station]->connectInputArray(beamformer,FREQS, thProtoIntra);
      //transb->getWorker()->setProcMode(zeroes);
      simul->addStep (transb[station]);
    }
	
    ///////////////////////
    // Station is filled now;
    // Set station communication parameters

#if 0
    theStations[station]->connectOutputToArray(&transb,1);
    simul->addStep (theStations[station]);      
#endif
  } // loop stations

  /***************************************************************************/
  { 
#if 0
    // The stations will be connected to correlator Steps for each beam
    dataprocessor =  new Simul(new WH_DataProc(STATIONS,BEAMS),  
			       "Data Processor",
			       0);
    // dataprocessor-input:
    //       [station1-beam1,..,station1-beamN,
    //              ..-beam1,..,      ..-beamN, 
    //        stationM-beam1,..,stationM-beamN] 
    
    dataprocessor->connectInputArray(theStations,STATIONS);
#endif
    
    for (station=0; station<STATIONS; station++) {
      cpinput[station] = new Step(new WH_CPInput(1,BEAMS*FREQBANDS),
				  "CPInput",
				  (3*STATIONS)+1);
      cpinput[station]->connectInput(transb[station], thProtoInter);
      simul->addStep(cpinput[station]); 
    }

#if 0
    dataprocessor->connectInputToArray(cpinput,STATIONS);
#endif
  
  for (beam = 0; beam < BEAMS; beam++) {
    for (freqband=0; freqband<FREQBANDS; freqband++) {
      // create the correlator
      correlator[beam*FREQBANDS+freqband] =
	new Step (new WH_Corr (STATIONS,1),
		  "Correlator",
		  0);
      simul->addStep (correlator[beam*FREQBANDS+freqband]);
      correlator[beam*FREQBANDS+freqband]->runOnNode((3*STATIONS)+2); 

      // Connect the correlators with the CPInput steps;
      // these connects are the typical crosses needed in correlation
      // The connects may be optimised for specific network
      // topologies or protocols (such as rings)
      for (station=0; station<STATIONS; station++) {
	correlator[beam*FREQBANDS+freqband]->connect(cpinput[station],
						     station,
						     beam*FREQBANDS+freqband,
						     1);
      }
    }
  } 
  
  
  
  //     for (beam = 0; beam < BEAMS; beam++) {
  //       for (corr=0; corr<CORRELATORS; corr++) {
  //       // create the integrator
  // 	integrator[beam][corr] =
  // 	  new Step (new WH_Integrator(1,1),
  // 		    "Integrator",
  // 		    0);
  
  // 	integrator[beam][corr]->connectInput(correlator[beam][corr]);
  // 	dataprocessor->addStep(integrator[beam][corr]);
  //       }
  //     }
  
  //     for (beam = 0; beam < BEAMS; beam++) {
  //       for (corr=0; corr<CORRELATORS; corr++) {
  // 	// create the integrator
  // 	convolve[beam][corr] =
  // 	  new Step(new WH_Convolve (1,1),
  // 		   "Convolve",
  // 		   0);
  
  // 	convolve[beam][corr]->connectInput(integrator[beam][corr]);
  // 	dataprocessor->addStep(convolve[beam][corr]);	
  //       }
  //     }
//   itsFiller = new LSFiller("LOFARSim.MS", 10., STATIONS, STAT_POS);
//   for (int i=0; i<FCORR; i++) {
//     itsFiller->addBand (POLS, CORRFREQS, INPUT_FREQ, 1e6);
//   }
//   for (int i=0; i<BEAMS; i++) {
//     itsFiller->addField (INPUT_AZIMUTH*PI/180., INPUT_ELEVATION*PI/180.);
//   }
//   makeMS = new Step(new WH_MakeMS(BEAMS*FCORR, itsFiller),
// 		    "Make MS",
// 		    0);
//   //makeMS->connectInputArray(convolve,BEAMS);
//   makeMS->connectInputArray(correlator,BEAMS*FCORR);
  
//   dataprocessor->addStep(makeMS);
  
//   //dataprocessor->connectOutputToArray(&makeMS,1);
  
  
  //  simul->addStep(dataprocessor);
  }
  // end of dataprocessor definition

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
  if (nsteps < 0) {
    nsteps = 10;
  }

  if (getSimul() == NULL) {
    cout << "Empty simulator; skip" << endl;
    return;
  }
  unsigned int rank = TRANSPORTER::getCurrentRank ();
  
  // create the time Controller; All the simuls will be synchronized with this object
  Simul Controller(new WH_Controller(),"Controller",CONTROLLER_NODE);

  

  // Produce process output only on node 0
  TRANSPORTER::synchroniseAllProcesses();
#ifdef MPI_
  double starttime=MPI_Wtime();
#endif
  Profiler::init();
  if (rank == 0) cout << endl <<  "Start Process" << endl;    
  for (int i = 1; i <= nsteps; i++) {
    if ((i%1 == 0) && (rank == 0)) { // print a dot after every 1 process steps
      cout << "." << flush;
    }
    if (i==4)   Profiler::activate();
    if (rank == (unsigned int)CONTROLLER_NODE) {
      Controller.getWorker()->process();
      //sleep(3);
    }

    if (rank != (unsigned int)CONTROLLER_NODE) TRANSPORTER::waitForBroadCast();
    TRANSPORTER::synchroniseAllProcesses();

    getSimul()->process ();
    if (i==4)   Profiler::deActivate();
  }
#ifdef MPI_
  double endtime=MPI_Wtime();
  cout << "Total Time on node " << rank << " : " << endtime-starttime << endl;
#endif
  //sleep(5+2*rank); // wait for all processes (?) and dump one by one in the right order 
  //cout << "DUMP from Node " << rank << endl;
  cout << endl << "END OF SIMUL on node " << rank << endl;
 
 

#ifdef MPI_
//   cout << "Node " << rank << " Totally Wrote " << TH_MPI::theirBytesWritten/1024 <<  " kB by MPI" << endl;
//   cout << "Node " << rank << " Totally Read "  << TH_MPI::theirBytesRead/1024    <<  " kB by MPI" << endl;
#endif // MPI_
  return;
}

void LOFARSim::dump() const {
  if (getSimul() == NULL) {
    cout << "Empty simulator; skip" << endl;
    return;
  }
  cout << "Dump LOFARSim" << endl;
  getSimul()->dump();
}


void LOFARSim::quit()
{
  delete itsFiller;
  itsFiller = 0;
}
