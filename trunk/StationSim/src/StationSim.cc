//# StationSim.cc
//#
//#  Copyright (C) 2002
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//#  This program is free software; you can redistribute it and/or modify
//#  it under the terms of the GNU General Public License as published by
//#  the Free Software Foundation; either version 2 of the License, or
//#  (at your option) any later version.
//#
//#  This program is distributed in the hope that it will be useful,
//#  but WITHOUT ANY WARRANTY; without even the implied warranty of
//#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//#  GNU General Public License for more details.
//#
//#  You should have received a copy of the GNU General Public License
//#  along with this program; if not, write to the Free Software
//#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#
//#  $Id$
//#

// This is the main program for the Station prototype simulation using the 
// StationSim simulation environment.
// 


#include <Common/Debug.h>
#include <Common/lofar_iostream.h>
#include <BaseSim/Step.h>
#include <BaseSim/WH_Empty.h>
#include <BaseSim/ParamBlock.h>
#include <BaseSim/Simul.h>
#include TRANSPORTERINCLUDE
#ifdef HAVE_CORBA
# include <BaseSim/Corba/TH_Corba.h>
# include <BaseSim/Corba/BS_Corba.h>
#endif
#include <BaseSim/Simul2XML.h>
#include <StationSim/StationSim.h>

// Beamformer includes
#include <StationSim/WH_BeamFormer.h>
#include <StationSim/WH_STA.h>
#include <StationSim/WH_WeightDetermination.h>
#include <StationSim/WH_Projection.h>

#include <StationSim/DataGenConfig.h>
#include <StationSim/WH_AddSignals.h>
#include <StationSim/WH_CreateSource.h>
#include <StationSim/WH_Modulate.h>
#include <StationSim/WH_PhaseShift.h>
#include <StationSim/WH_ReadSignal.h>
#include <StationSim/WH_BandSep.h>
#include <StationSim/WH_DataReader.h>
#include <StationSim/WH_DataGenerator.h>
#include <StationSim/WH_Station.h>
#include <StationSim/WH_LopesDataReader.h>


StationSim::StationSim()
{}

void StationSim::define (const ParamBlock& params)
{
  char suffix[8];
  char suffix2[8];
  char suffix3[8];

#ifdef HAVE_CORBA
  // Start Orb Environment
  AssertStr (BS_Corba::init(),
	     "Could not initialise CORBA environment");
#endif

  int rank = TRANSPORTER::getCurrentRank ();
  unsigned int size = TRANSPORTER::getNumberOfNodes();
  cout << "StationSim Processor " << rank << " of " 
       << size << "  operational." << flush << endl;

  // Get the various controls from the ParamBlock.
  const int nrcu                  = params.getInt    ("nrcu", 2);
  const int nsubband              = params.getInt    ("nsubband", 16);
  const int nchannel              = params.getInt    ("nchannel", 16);
  const int nselsubband           = params.getInt    ("nselsubband", 16);
  const string selFileName        = params.getString ("selfilename", "");
  const string coeffFileNameSub   = params.getString ("coefffilenamesub", "");
  const string coeffFileNameChan  = params.getString ("coefffilenamechan", "");
  const string datagenFileName    = params.getString ("datagenfilename", "");
  const string bfDipoleFile       = params.getString ("bffilename","");
  const int fifolength            = params.getInt    ("bf_fifolength", 512);
  const int buflength             = params.getInt    ("bf_bufferlength", 100);
  const int modulationWindowSize  = params.getInt    ("modwindowsize", 32);
  const int detNrfi               = params.getInt    ("detnrfi",0);
  const int maxNrfi               = params.getInt    ("maxnrfi", 1);
  const int STArate               = params.getInt    ("starate", 100);
  const int WDrate                = params.getInt    ("wdrate", 100);
  const string beamtrajectfile    = params.getString ("beamtrajectfile", "");
  const bool tapDG                = params.getInt    ("tapdg", 0);
  const bool tapFB1               = params.getInt    ("tapfb1", 0);
  const bool tapPRJ               = params.getInt    ("tapprj", 0);
  const bool tapBF                = params.getInt    ("tapbf", 0);
  const string BF_Algorithm       = params.getString ("bfalg", "EVD");
  const int PASTd_Init            = params.getInt    ("pd_init",0);
  const int PASTd_Interval        = params.getInt    ("pd_interval",5);
  const int PASTd_Update          = params.getInt    ("pd_update",100);
  const double PASTd_Beta         = params.getDouble ("pd_beta", 0.95);
  const bool enableDG             = params.getInt    ("enabledg", 1);
  const bool enableFB1            = params.getInt    ("enablefb1", 1);
  const bool enableBF             = params.getInt    ("enablebf", 1);
  const string lopesdirname       = params.getString ("lopesdirname", "");
  const int QMs                   = params.getInt    ("qms",0);

  const int shiftWeightCalc       = -1;


  // Create three simul objects, one for the data generator, one for the processing and a combining one. 
  // This is a nice way to handle the delay between them.
  WH_Empty wh_empty;
  Simul simulDataGenerator (WH_DataGenerator ("DataGenerator", nrcu), "DataGenerator");
  Simul simulStation       (WH_Station ("Station", nrcu), "Station");  
  Simul simulTotal         (wh_empty, "Simulation");


 /***********************************************************************************
   *  The creation of the data generator workholders                                 *
   ***********************************************************************************/  
  // Read in the configuration for the sources
  DataGenerator* DG_Config = new DataGenerator (datagenFileName);
  if (enableDG) {
	// Check
	AssertStr (nrcu == DG_Config->itsArray->size(), "The array configfile doesn't match the simulator input!");

	// Create the workholders
	for (int i = 0; i < DG_Config->itsNumberOfSources; ++i) {
	  // Create for every signal a read_signal step/workholder
	  // The signals are named as follows: signal_ + source number + _ + signal number (e.g. signal_1_3 )
	  for (int j = 0; j < DG_Config->itsSources[i]->itsNumberOfSignals; ++j) {
		// create suffix for name
		sprintf (suffix, "%d_%d", i, j);
		
		// create the read signal steps
		Step signal (WH_ReadSignal ("",
									1,
									DG_Config->itsSources[i]->itsSignals[j]->itsInputFileName),
					 string ("signal_") + suffix, 
					 false);
		
		simulDataGenerator.addStep (signal);
		
		// Create the modulation steps
		Step modulate (WH_Modulate (1,
									1,
									DG_Config->itsSources[i]->itsSignals[j]->itsModulationType,
									DG_Config->itsSources[i]->itsSignals[j]->itsCarrierFreq,
									DG_Config->itsSamplingFreq,
									DG_Config->itsSources[i]->itsSignals[j]->itsOpt,
									DG_Config->itsSources[i]->itsSignals[j]->itsAmplitude, 
									suffix,
									modulationWindowSize),
					   string ("modulate_") + suffix, 
					   false);
		
		simulDataGenerator.addStep (modulate);
	  }
	  
	  // Create the source creator steps
	  sprintf (suffix, "%d", i);
	  Step create_source (WH_CreateSource ("",
										   DG_Config->itsSources[i]->itsNumberOfSignals, 
										   1),
						  string ("create_source_") + suffix, 
						  false);
	  
	  simulDataGenerator.addStep (create_source);
	  
	  // Create the phase shifters
	  Step phase_shift (WH_PhaseShift (1,
									   1,
									   nrcu,
									   DG_Config,
									   DG_Config->itsNumberOfFFT,
									   i,
									   modulationWindowSize,
									   suffix),
						string ("phase_shift_") + suffix, 
						false);
	  
	  simulDataGenerator.addStep (phase_shift);
	}
	
	// Create the step that adds the signals 
	Step add_signals (WH_AddSignals ("AddSignals",
									 DG_Config->itsNumberOfSources,
									 1, 
									 nrcu,
									 (bool) tapDG), 
					  "add_signals", 
					  false); 
	
	simulDataGenerator.addStep (add_signals);

  } else if (lopesdirname !=  "") {
	Step lopes_data (WH_LopesDataReader ("LopesData", nrcu, lopesdirname), "lopes_data", false);
	simulDataGenerator.addStep (lopes_data);
  }

  /***********************************************************************************
   *  The creation of the polyphase filterbank workholders                           *
   ***********************************************************************************/  
  if (enableFB1) {
	// Create the subband filterbank
	int nout = 2;
	
	Step subband_filter (WH_BandSep("FB1", nsubband, nrcu, coeffFileNameSub, nout, (bool) tapFB1,
									selFileName, nselsubband, QMs),
						 string ("subband_filter"), false);
	  
	subband_filter.setOutRate(nsubband);
	simulStation.addStep (subband_filter);	
  }

  
  /***********************************************************************************
   *  The creation of the beamforming workholders                                    *
   ***********************************************************************************/  
  if (enableBF) {
	// The Space Time Analysis object
	Step sta (WH_STA ("STA", 2, nrcu, nselsubband, maxNrfi, buflength, BF_Algorithm,
					  PASTd_Init, PASTd_Update, PASTd_Interval, PASTd_Beta, detNrfi), 
			  "sta", false);
	  
	sta.setRate (nsubband);
	sta.setOutRate (nsubband * STArate);
	simulStation.addStep (sta);
	  
	// the Weight Determination Object
	Step weight_det (WH_WeightDetermination ("WD", 0, 1, nrcu, bfDipoleFile, beamtrajectfile),
					 "weight_det", false);   
	  
	weight_det.setRate(nsubband * WDrate);
	weight_det.setDoHandleShift (shiftWeightCalc);  // On all the outputs
	simulStation.addStep(weight_det);
	  
	// the projection object
	int ninProj = 3;
	Step projection (WH_Projection ("Proj", ninProj, 1, nrcu, nselsubband, maxNrfi, tapPRJ), 
					 "projection", false);
	
	projection.setDoHandleShift (shiftWeightCalc);
	if (STArate < WDrate) {
	  projection.setRate (STArate * nsubband);
	} else {
	  projection.setRate (WDrate * nsubband);
	}
	for (int i = 0; i < ninProj - 2; ++i) {
	  projection.setInRate (nsubband * WDrate, i);
	}
	for (int i = ninProj - 2; i < ninProj; ++i) {
	  projection.setInRate(nsubband * STArate, i);
	  projection.setInDoHandleShift (0, i);
	}
	simulStation.addStep(projection); 	

	// The beamformer object    
	Step beam (WH_BeamFormer ("BF", 2, 1, nrcu, nselsubband, (bool) tapBF), "beam_former", false);
	  
	beam.setRate(nsubband);
	if (STArate < WDrate) {
	  beam.setInRate (STArate * nsubband, 1);	
	} else {
	  beam.setInRate (WDrate * nsubband, 1);	
	}
	beam.setDoHandleShift (shiftWeightCalc);
	beam.setInDoHandleShift (0, 0);
	beam.setOutDoHandleShift (0);
	simulStation.addStep (beam);
  }
 

  /***********************************************************************************
   *  The connection of the datagenerator workholders                                *
   ***********************************************************************************/  
  if (enableDG) {
	// Connect the steps.
	for (int i = 0; i < DG_Config->itsNumberOfSources; ++i) {
	  sprintf (suffix2, "%d", i);
	  
	  // Connect the signals to the modulators
	  for (int j = 0; j < DG_Config->itsSources[i]->itsNumberOfSignals; ++j) {
		sprintf (suffix, "%d_%d", i, j);
		sprintf (suffix3, "%d", j);
		simulDataGenerator.connect (string ("signal_") + suffix + string (".out_0"),
					   string ("modulate_") + suffix + string (".in_0"));
	  }
	  
	  // Connect the modulators to a create_source
	  for (int j = 0; j < DG_Config->itsSources[i]->itsNumberOfSignals; ++j) {
		sprintf (suffix, "%d_%d", i, j);
		sprintf (suffix3, "%d", j);
		simulDataGenerator.connect (string ("modulate_") + suffix + string (".out_0"),
					   string ("create_source_") + suffix2 +
					   string (".in_") + suffix3);
	  }
	  
	  // Connect the create_sources to the phase_shifters
	  simulDataGenerator.connect (string ("create_source_") + suffix2 + string (".out_0"),
					 string ("phase_shift_") + suffix2 + string (".in_0"));
	}
	
	// Connect the phaseshifters to the add signals
	for (int i = 0; i < DG_Config->itsNumberOfSources; ++i) {
	  sprintf (suffix, "%d", i);
	  simulDataGenerator.connect (string ("phase_shift_") + suffix + string (".out_0"),
					 string ("add_signals.in_") + suffix);
	}
	
	// Connect the add_signals (rcu signals generated by the DG) to the 
	// output of the simul
	simulDataGenerator.connect ("add_signals.out_0", ".");	
  } else if (lopesdirname != "") {
	simulDataGenerator.connect ("lopes_data.out", ".");
  }

  /***********************************************************************************
   *  The connection of the filterbank and beam forming workholders                  *
   ***********************************************************************************/  
  if (enableFB1) {	
	// connect the input of the simul to the first step
	simulStation.connect (".", "subband_filter.in");

	if (enableBF) {
	  // connect the subband filterbank to STA
 	  simulStation.connect ("subband_filter.out_1", "sta.in");	
	  
	  // connect the STA to Projection
	  simulStation.connect ("sta.out_0", "projection.in_rfi");
	  simulStation.connect ("sta.out_mdl", "projection.in_mdl");

	  // connect the Weight determinator to Projection
	  simulStation.connect ("weight_det.out_0", "projection.in_0");
 
	  // connect the subband filterbank to the Beamformer
	  simulStation.connect ("subband_filter.out_0", "beam_former.in_0");

	  // connect Projection to Beamformer
	  simulStation.connect ("projection.out_0", "beam_former.weights");
	}
  }

  // Connect the two main simuls to each other
  if (enableDG || lopesdirname != "") {
	simulTotal.addStep (simulDataGenerator);
  }
  if (enableFB1) {
	simulStation.setOutRate (nsubband);
	if (enableDG) {
	  simulStation.setDelay (DG_Config->itsNumberOfFFT + modulationWindowSize - 2);
	}
	simulTotal.addStep (simulStation);
  }
  if ((enableDG || lopesdirname != "") && enableFB1) {
	simulTotal.connect ("DataGenerator_0", "Station_1");
  }
  
  // Display the delays
  const list<Step*>& steps = simulStation.getSteps();
  list<Step*>::const_iterator iList;
  for (int i = 0; i < simulStation.getWorker ()->getInputs (); i++) {
	cout << simulStation.getName () << " input read : " << simulStation.getInData (i).getReadDelay () << endl;
	cout << simulStation.getName () << " input write : " << simulStation.getInData (i).getWriteDelay () << endl;
  }
  for (int i = 0; i < simulStation.getWorker ()->getOutputs (); i++) {
	cout << simulStation.getName () << " output read : " << simulStation.getOutData (i).getReadDelay () << endl;
	cout << simulStation.getName () << " output write : " << simulStation.getOutData (i).getWriteDelay () << endl;
  }
  cout << endl;
  for (iList = steps.begin(); iList != steps.end(); ++iList) {
	for (int i = 0; i < (*iList)->getWorker()->getInputs(); i++) {
	  cout << (*iList)->getName () << " input : " << (*iList)->getInData (i).getReadDelay () << endl;
	}
	for (int i = 0; i < (*iList)->getWorker()->getOutputs(); i++) {
	  cout << (*iList)->getName () << " output : " << (*iList)->getOutData (0).getWriteDelay () << endl;
	}
	cout << endl;
  }
  // end of dataprocessor definition
  
  setSimul (simulTotal);
  
  simulTotal.checkConnections ();

  //itsSimul->setLocalComm();

  //////////////////////////////////////////////////////////////////////
  //
  // Finished configuration definition
  // 
  //////////////////////////////////////////////////////////////////////
  cout << "Ready with definition of configuration on node " << rank << endl;  

  Simul2XML xml(simulTotal);
  xml.write("out.xml");
}



void StationSim::run (int nsteps)
{
  if (nsteps < 0) {
    nsteps = 10;
  }

  int rank = TRANSPORTER::getCurrentRank ();

  if (rank == 0) cout << endl <<  "Start Process" << endl;    
  for (int i = 1; i <= nsteps; i++) {
    if ((i%1 == 0) && (rank == 0)) { // print a dot after every 1 process steps
      cout << "." << flush;
    }
    getSimul().process ();
  }
  cout << endl << "END OF SIMUL on node " << rank << endl;
  
  exit (1);
  return;
}

void StationSim::dump() const {
  cout << "Dump StationSim" << endl;
  getSimul().dump();
}


void StationSim::quit()
{}
