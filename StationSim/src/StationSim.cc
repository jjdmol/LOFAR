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
#include <StationSim/WH_AWE.h>
#include <StationSim/WH_BeamFormer.h>
#include <StationSim/DataGenConfig.h>
#include <StationSim/WH_AddSignals.h>
#include <StationSim/WH_CreateSource.h>
#include <StationSim/WH_Modulate.h>
#include <StationSim/WH_PhaseShift.h>
#include <StationSim/WH_ReadSignal.h>

#define MODULATION_WINDOW_SIZE  32	// this window size in the modulation
#define DELAY_MOD               MODULATION_WINDOW_SIZE-1
#define NFFT                    128
#define NRCU                    2
#define DELAY_PHASE             NFFT-1

StationSim::StationSim()
{}


void StationSim::define (const ParamBlock& params)
{

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
  bool splitrcu = params.getBool ("splitrcu", false);
  int nrcu = params.getInt ("nrcu", 1);
  int nsub1 = params.getInt ("nsubband1", 16);
  int nsub2 = params.getInt ("nsubband2", 0);
  int nsel = params.getInt ("nselband", 4);
  int nbeam = params.getInt ("nbeam", 1);
  int maxNtarget = params.getInt ("maxntarget", 1);
  int maxNrfi = params.getInt ("maxnrfi", 1);
  string rcuName = params.getString ("rcufilename", "");
  string selName = params.getString ("selfilename", "");
  string coeffName1 = params.getString ("coefffilename1", "");
  string coeffName2 = params.getString ("coefffilename2", "");
  string dipoleName = params.getString ("dipolefilename", "");
  string targetName = params.getString ("targettrackfilename", "");
  string rfiName = params.getString ("rfitrackfilename", "");
  int fifolength    = params.getInt ("bf_fifolength", 512);
  int buflength     = params.getInt ("bf_bufferlength", 256);

  /* The AWE parameters */
  int aweRate       = params.getInt ("awerate", 1);

  // Multiple files are used if rcu file name does not contain an *.
  bool rcumultifile = rcuName.find('*') != string::npos;
  cout << "multifile=" << rcumultifile << endl;

  char suffix[8];
  char suffix2[8];
  char suffix3[8];

  // Read in the configuration for the sources
  DataGenerator* DG_Config = new DataGenerator ("/home/alex/gerdes/DG_Input/datagenerator.txt");

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
								  MODULATION_WINDOW_SIZE),
					 string ("modulate_") + suffix, 
					 false);

      modulate.getOutData (0).setWriteDelay (DELAY_MOD);
      simul.addStep (modulate);
    }

    sprintf (suffix, "%d", i);
    Step create_source (WH_CreateSource ("",
										 DG_Config->itsSources[i]->itsNumberOfSignals, 
										 1),
						string ("create_source_") + suffix, 
						false);

    for (int j = 0; j < DG_Config->itsSources[i]->itsNumberOfSignals; ++j) {
      create_source.getInData (j).setReadDelay (DELAY_MOD);
	}
    create_source.getOutData (0).setWriteDelay (DELAY_MOD);
    simul.addStep (create_source);

    Step phase_shift (WH_PhaseShift (1,
									 1,
									 NRCU,
									 DG_Config,
									 NFFT,
									 i,
									 MODULATION_WINDOW_SIZE),
					  string ("phase_shift_") + suffix, 
					  false);

    phase_shift.getInData (0).setReadDelay (DELAY_MOD);
    phase_shift.getOutData (0).setWriteDelay (DELAY_MOD + DELAY_PHASE);
    simul.addStep (phase_shift);
  }

  Step add_signals (WH_AddSignals ("",
								   DG_Config->itsNumberOfSources,
								   NRCU, 
								   NRCU), 
					"add_signals", 
					false);

  for (int i = 0; i < DG_Config->itsNumberOfSources; ++i) {
    add_signals.getInData (i).setReadDelay (DELAY_MOD + DELAY_PHASE);
  }
  for (int i = 0; i < NRCU; ++i) {
    add_signals.getOutData (i).setWriteDelay (DELAY_MOD + DELAY_PHASE);
  }
  simul.addStep (add_signals);

  /*******************************/
  /* Create the individual steps */
  /* Set the rate of the steps   */
  /*******************************/

  // The beamformer object
  Step beam (WH_BeamFormer("", nrcu+1, nrcu+1, nrcu, nsel, nbeam, maxNtarget, maxNrfi,
			   fifolength, buflength), "beamformer", false);
//   beam.setRate (nsub1);
//   beam.setInRate (nsub1*aweRate, 1);
  for (int j = 0; j < nrcu+1; ++j) {
	beam.getInData (j).setReadDelay (DELAY_MOD + DELAY_PHASE);
	beam.getOutData (j).setWriteDelay (DELAY_MOD + DELAY_PHASE);
  }
  simul.addStep (beam);

  // The AWE object
  Step awe (WH_AWE("", 1, 1, nrcu, buflength),"awe", false);
//   awe.setRate (nsub1);
//   awe.setOutRate (nsub1*aweRate);
  awe.getInData (0).setReadDelay (DELAY_MOD + DELAY_PHASE);
  awe.getOutData (0).setWriteDelay (DELAY_MOD + DELAY_PHASE);
  simul.addStep (awe);

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

  for (int i = 0; i < DG_Config->itsNumberOfSources; ++i) {
    sprintf (suffix, "%d", i);
    simul.connect (string ("phase_shift_") + suffix + string (".out_0"),
				   string ("add_signals.in_") + suffix);
  }






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



