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
// #include <StationSim/WH_AWE.h>
// #include <StationSim/WH_BeamFormer.h>
#include <StationSim/DataGenConfig.h>
#include <StationSim/WH_AddSignals.h>
#include <StationSim/WH_CreateSource.h>
#include <StationSim/WH_Modulate.h>
#include <StationSim/WH_PhaseShift.h>
#include <StationSim/WH_ReadSignal.h>
#include <StationSim/WH_BandSep.h>
#include <StationSim/WH_BeamFormer.h>


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
  //if (rank == 1) sleep(30);

  // Get the various controls from the ParamBlock.
  const int nrcu                  = params.getInt    ("nrcu", 2);
  const int nsubband              = params.getInt    ("nsubband", 16);
  const int nchannel              = params.getInt    ("nchannel", 16);
  const int nselsubband           = params.getInt    ("nselsubband", 4);
  const string selFileName        = params.getString ("selfilename", "");
  const string coeffFileNameSub   = params.getString ("coefffilenamesub", "");
  const string coeffFileNameChan  = params.getString ("coefffilenamechan", "");
  const string datagenFileName    = params.getString ("datagenfilename", "");
  const int fifolength            = params.getInt    ("bf_fifolength", 512);
  const int buflength             = params.getInt    ("bf_bufferlength", 256);
  const int modulationWindowSize  = params.getInt    ("modwindowsize", 32);
  const int nfft_phaseshift       = params.getInt    ("nfftphaseshift", 128);
  const int aweRate               = params.getInt    ("awerate", 1);
  const int nbeam                 = params.getInt    ("nbeam", 1);
  const int maxNtarget            = params.getInt    ("maxntarget", 1);
  const int maxNrfi               = params.getInt    ("maxnrfi", 1);

  const int delayMod              = modulationWindowSize - 1;
  const int delayPhase            = nfft_phaseshift - 1;
  const int delaySubFilt          = nrcu * (nsubband - 1);

  // Read in the configuration for the sources
  DataGenerator* DG_Config = new DataGenerator (datagenFileName);

  // Check
  AssertStr (nrcu == DG_Config->itsArray->size(), "The array configfile doesn't match the simulator input!");

  // Create the overall simul object.
  WH_Empty wh;
  Simul simul(wh, "StationSim");
  setSimul (simul);

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
      
      simul.addStep (signal);
      
      // Create the modulation steps
      Step modulate (WH_Modulate (1,
								  1,
								  DG_Config->itsSources[i]->itsSignals[j]->itsModulationType,
								  DG_Config->itsSources[i]->itsSignals[j]->itsCarrierFreq,
								  DG_Config->itsSources[i]->itsSignals[j]->itsSamplingFreq,
								  DG_Config->itsSources[i]->itsSignals[j]->itsOpt,
								  DG_Config->itsSources[i]->itsSignals[j]->itsAmplitude, 
								  modulationWindowSize),
					 string ("modulate_") + suffix, 
					 false);
      
      modulate.getOutData (0).setWriteDelay (delayMod);
      simul.addStep (modulate);
    }

	// Create the source creator steps
    sprintf (suffix, "%d", i);
    Step create_source (WH_CreateSource ("",
										 DG_Config->itsSources[i]->itsNumberOfSignals, 
										 1),
						string ("create_source_") + suffix, 
						false);
    
    for (int j = 0; j < DG_Config->itsSources[i]->itsNumberOfSignals; ++j) {
      create_source.getInData (j).setReadDelay (delayMod);
	}
    create_source.getOutData (0).setWriteDelay (delayMod);
    simul.addStep (create_source);

	// Create the phase shifters
    Step phase_shift (WH_PhaseShift (1,
									 1,
									 nrcu,
									 DG_Config,
									 nfft_phaseshift,
									 i,
									 modulationWindowSize),
					  string ("phase_shift_") + suffix, 
					  false);
    
    phase_shift.getInData (0).setReadDelay (delayMod);
    phase_shift.getOutData (0).setWriteDelay (delayMod + delayPhase);
    simul.addStep (phase_shift);
  }

  // Create the step that adds the signals 
  Step add_signals (WH_AddSignals ("",
								   DG_Config->itsNumberOfSources,
								   nrcu, 
								   nrcu), 
					"add_signals", 
					false); 
  for (int i = 0; i < DG_Config->itsNumberOfSources; ++i) {
    add_signals.getInData (i).setReadDelay (delayMod + delayPhase);
  }
  for (int i = 0; i < nrcu; ++i) {
    add_signals.getOutData (i).setWriteDelay (delayMod + delayPhase);
  }
  simul.addStep (add_signals);

  // Create the subband filterbank
  for (int i = 0; i < nrcu; ++i) {
	sprintf (suffix, "%d", i);
	  
	Step subband_filter (WH_BandSep(suffix,
									nsubband,
									coeffFileNameSub),
						 string ("subband_filter_") + suffix,
						 false);
	subband_filter.getInData (0).setReadDelay (delayMod + delayPhase);
	for (int j = 0; j < nsubband; ++j) {
	  subband_filter.getOutData (j).setWriteDelay (delayMod + delayPhase + delaySubFilt);
	}
	simul.addStep (subband_filter);	
  }


  // Create the individual steps. Set the rate of the steps 
  // The beamformer object
  for (int i = 0; i < nsubband; ++i) {
	sprintf (suffix, "%d", i);

	Step beam (WH_BeamFormer("", nrcu, nrcu, nrcu, nbeam, maxNtarget, maxNrfi), 
			   string("beam_former_") + suffix, 
			   false);

	for (int j = 0; j < nrcu; ++j) {
	  beam.getInData (j).setReadDelay (delayMod + delayPhase + delaySubFilt);
	  beam.getOutData (j).setWriteDelay (delayMod + delayPhase + delaySubFilt);
	}

	simul.addStep (beam);
  } 
  
//   // The AWE object
//   Step awe (WH_AWE("", 1, 1, nrcu, buflength),"awe", false);
//   awe.getInData (0).setReadDelay (delayMod + delayPhase);
//   awe.getOutData (0).setWriteDelay (delayMod + delayPhase);
//   simul.addStep (awe);
  // workholders are created


  // Connect the steps.
  for (int i = 0; i < DG_Config->itsNumberOfSources; ++i) {
    sprintf (suffix2, "%d", i);

    // Connect the signals to the modulators
    for (int j = 0; j < DG_Config->itsSources[i]->itsNumberOfSignals; ++j) {
      sprintf (suffix, "%d_%d", i, j);
      sprintf (suffix3, "%d", j);
      simul.connect (string ("signal_") + suffix + string (".out_0"),
					 string ("modulate_") + suffix + string (".in_0"));
    }

    // Connect the modulators to a create_source
    for (int j = 0; j < DG_Config->itsSources[i]->itsNumberOfSignals; ++j) {
      sprintf (suffix, "%d_%d", i, j);
      sprintf (suffix3, "%d", j);
      simul.connect (string ("modulate_") + suffix + string (".out_0"),
					 string ("create_source_") + suffix2 +
					 string (".in_") + suffix3);
    }

    // Connect the create_sources to the phase_shifters
    simul.connect (string ("create_source_") + suffix2 + string (".out_0"),
				   string ("phase_shift_") + suffix2 + string (".in_0"));
  }

  // Connect the phaseshifters to the add signals
  for (int i = 0; i < DG_Config->itsNumberOfSources; ++i) {
    sprintf (suffix, "%d", i);
    simul.connect (string ("phase_shift_") + suffix + string (".out_0"),
				   string ("add_signals.in_") + suffix);
  }
  
  // Connect the add_signals (rcu signals generated by the DG) to the 
  // subband filterbank  
  for (int i = 0; i < nrcu; ++i) { 
	sprintf (suffix, "%d", i);
	simul.connect (string ("add_signals") + string (".out_") +
				   suffix, string ("subband_filter_") + suffix + string (".in"));
  }

  // Connect the subband filterbank with the beam former
//   for (int s = 0; s < nsubband; ++s) {
// 	for (int r = 0; r < nrcu; ++r) { 
// 	  sprintf (suffix, "%d", r);
// 	  sprintf (suffix2, "%d", s);

// 	  simul.connect (string ("subband_filter_") + suffix + string (".out_") + suffix2,
// 					 string ("beam_former_") + suffix2 + string (".in_") + suffix);
// 	}
//   }

  // end of dataprocessor definition
  simul.checkConnections();

  //itsSimul->setLocalComm();

  //////////////////////////////////////////////////////////////////////
  //
  // Finished configuration definition
  // 
  //////////////////////////////////////////////////////////////////////
  cout << "Ready with definition of configuration on node " << rank << endl;  

  Simul2XML xml(simul);
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
  return;
}

void StationSim::dump() const {
  cout << "Dump StationSim" << endl;
  getSimul().dump();
}


void StationSim::quit()
{}



