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
#include TRANSPORTERINCLUDE
#ifdef CORBA_
#include "BS_Corba.h"
#endif

#include <unistd.h>
#include <iostream.h>
#include <stdlib.h>
#include <string>
#include <map>

#define CORR_FILE       "./WAVE/Corr.ext"
#define CORR_TMP_FILE   "./WAVE/Corr.tmp"
#define WAVE_FILE_1 "./WAVE/antenna1.wav"
#define WAVE_FILE_2 "./WAVE/antenna2.wav"
#define CORRNODE (STATIONS+1)

LOFARSim::LOFARSim()
: itsFiller (0)
{
}


void LOFARSim::define(const ParamBlock&)
{

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
    theStations[station]->runOnNode(station);
    ////////////////////////////////////////////////////////////////////////
    // First put the antenna's in the Sation Simul
    for (element = 0; element < ELEMENTS; element++)
      {
	  antenna[station][element] =
	    new Step (new WH_WAV(1,1,station*LAGS), //WH_Antenna (1,1),
		      "Antenna", 
		      station);
	  antenna[station][element]->runOnNode(station);
	  antenna[station][element]->connectInput(NULL);
	  theStations[station]->addStep (antenna[station][element]);
        }
      
    //////////////////////////////////////////////////////////////////////////
    // Add FFTs to the station
      
    for (element = 0; element < ELEMENTS; element++) {
	  fft[element] = new Step(new WH_FFT(1,1),
				  "FFT",
				  0);
	  fft[element]->runOnNode(station);
	  fft[element]->connectInput(antenna[station][element]);
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
      transf->runOnNode(station);
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
	beamformer[freq]->runOnNode(station);
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
      transb->runOnNode(station);
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
				  STATIONS);
      simul->addStep(cpinput[station]); 
      cpinput[station]->connectInput(theStations[station]);
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
		    STATIONS+1);
	simul->addStep (correlator[beam*FREQBANDS+freqband]);
	correlator[0]->runOnNode(CORRNODE); 
	
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
    
    
    //    simul->addStep(dataprocessor);
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

  unsigned int rank = TRANSPORTER::getCurrentRank ();
  // create the time Controller; All the simuls will be synchronized with this object
  Simul Controller(new WH_Controller(),"Controller",CONTROLLER_NODE);

  while (1) {

    if (rank == 0)  ((WH_WAV*)antenna[0][0]->getWorker())->readFile(WAVE_FILE_1);
    if (rank == 1)  ((WH_WAV*)antenna[1][0]->getWorker())->readFile(WAVE_FILE_1);

    nsteps = LAGS;

  if (getSimul() == NULL) {
    cout << "Empty simulator; skip" << endl;
    return;
  }
  
  

  // Produce process output only on node 0
  TRANSPORTER::synchroniseAllProcesses();
#ifdef MPI_
  double starttime=MPI_Wtime();
#endif
  Profiler::init();
  if (rank == 0) cout << endl <<  "Start Process" << endl;    
  if (rank == CORRNODE) ((WH_Corr*)correlator[0]->getWorker())->openFile(CORR_TMP_FILE);

  for (int i = 1; i <= nsteps; i++) {

//      if ((i%1 == 0) && (rank == 0)) { // print a dot after every 1 process steps
//        cout << "." << flush;
//      }
    if (i==4)   Profiler::activate();
    if (rank == (unsigned int)CONTROLLER_NODE) {
      Controller.getWorker()->process();
      //sleep(3);
    } 
    if (rank != CONTROLLER_NODE) TRANSPORTER::waitForBroadCast();
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
 
  if (rank == CORRNODE) ((WH_Corr*)correlator[0]->getWorker())->closeFile();
  usleep(500);
  system("/bin/cp ./WAVE/Corr.tmp ./WAVE/Corr.ext");
  usleep(500);

//    char arg[80];
//    sprintf(arg,"%s %s\n",CORR_TMP_FILE,CORR_FILE);
//    execl("/bin/cp",arg);
  unlink(WAVE_FILE_1 ".ready");
 

#ifdef MPI_
//   cout << "Node " << rank << " Totally Wrote " << TH_MPI::theirBytesWritten/1024 <<  " kB by MPI" << endl;
//   cout << "Node " << rank << " Totally Read "  << TH_MPI::theirBytesRead/1024    <<  " kB by MPI" << endl;
#endif // MPI_

  }

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



