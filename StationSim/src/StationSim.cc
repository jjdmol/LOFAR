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

#include <StationSim/StationSim.h>
#include <StationSim/WH_AWE.h>
#include <StationSim/WH_BeamFormer.h>
#include <StationSim/WH_BandSep.h>
#include <StationSim/WH_Selector.h>
#include <StationSim/WH_SubBandSel.h>
#include <StationSim/WH_Detection.h>
#include <StationSim/WH_Calibration.h>
#include <StationSim/WH_Cancel.h>

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

#include <Common/Debug.h>
#include <Common/lofar_iostream.h>


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
  int aweRate = params.getInt ("awerate", 16);
  string rcuName = params.getString ("rcufilename", "");
  string selName = params.getString ("selfilename", "");
  string coeffName1 = params.getString ("coefffilename1", "");
  string coeffName2 = params.getString ("coefffilename2", "");
  string dipoleName = params.getString ("dipolefilename", "");
  string targetName = params.getString ("targettrackfilename", "");
  string rfiName = params.getString ("rfitrackfilename", "");
  // Multiple files are used if rcu file name does not contain an *.
  bool rcumultifile = rcuName.find('*') != string::npos;
  cout << "multifile=" << rcumultifile << endl;

  // Set detection and calibration rate
//   int detectionRate = 2;
//   int calibrationRate = 3;

  // Create the overall simul object.
  WH_Empty wh;
  Simul simul(wh, "StationSim");
  setSimul (simul);

  // Create all the steps.
  // Set the rate of the steps.
  // Add the steps to the Simul.

  // The RCU data is read from a file.
  // It can be done from one (ASCII) file by WH_RCU or WH_RCUAll
  // or from multiple binary files by WH_RCUAll.
//   if (splitrcu) {
//     Step rcu (WH_RCU ("", nrcu, rcuName),
// 	      "rcu", false);
//     simul.addStep (rcu);
//     Step merge (WH_Merge ("", nrcu, 1),
// 		"rcuall", false);
//     simul.addStep (merge);
//   } else {
//     Step rcu (WH_RCUAll ("", 1, nrcu, rcuName, rcumultifile),
// 	      "rcuall", false);
//     simul.addStep (rcu);
//   }

//   if (nsub1 > 0) {
//     Step bandsep (WH_BandSep ("", 1, nrcu, nsub1, coeffName1),
// 		  "bandsep", false);
//     bandsep.setOutRate (nsub1, 0);
//     simul.addStep (bandsep);
//   }

//   Step subsel (WH_SubBandSel ("", nsub1, selName),
// 	       "subseldef", false);
//   subsel.setRate (nsub1);
//   simul.addStep (subsel);

//   Step select (WH_Selector ("", 3, nrcu, nsub1, nsel), // Added two output for the rfimit
// 	       "select", false);
//   select.setRate (nsub1);
//    simul.addStep (select);

  // Make instances of the Detection and Calibration workholder and connect them to 
  // each other
//   Step Calibration(WH_Calibration ("", 1, nrcu, nsub1), "calibration", false);
//   Calibration.setRate(nsub1); 
//   simul.addStep(Calibration);

//   Step Detection(WH_Detection ("", 1, nrcu, nsub1), "detection", false);
//   Detection.setRate(nsub1);
//   simul.addStep(Detection);
  
//   Step Cancel(WH_Cancel ("", 2, nrcu, nsub1), "cancel", false);
//   Cancel.setRate(nsub1);
//   simul.addStep(Cancel);

//   Step VerifyRFI(WH_VerifyRFI("", 0, nrcu, nsub1), "verifyrfi", false);
//   VerifyRFI.setRate(nsub1);
//   simul.addStep(VerifyRFI);

//   Step target (WH_TargetTrack ("", maxNtarget, targetName),
// 	       "targettrack", false);
//   target.setRate (nsub1);
//   simul.addStep (target);

//   Step rfi (WH_RFITrack ("", maxNtarget, rfiName),
// 	    "rfitrack", false);
//   rfi.setRate (nsub1);
//   simul.addStep (rfi);

//   Step beam (WH_BeamFormer("", 2, nrcu, nsel, nbeam, maxNtarget, maxNrfi,
// 			   dipoleName),
// 	     "beamformer", false);
//   beam.setRate (nsub1);
//   beam.setInRate (nsub1*aweRate, 1);
//   simul.addStep (beam);

//   Step awe (WH_AWE("", 1, nsel, nbeam),
// 	    "awe", false);
//   awe.setRate (nsub1);
//   awe.setOutRate (nsub1*aweRate);
//   simul.addStep (awe);

//   if (nsub2 > 0) {
//     Step chanfsep (WH_BandSep ("", 1, nsel*nbeam, nsub2, coeffName2),
// 		  "chanfsep", false);
//     chanfsep.setOutRate (nsub1*nsub2, 1);
//     simul.addStep (chanfsep);
//   }

//   // Connect the steps.
//   if (splitrcu  &&  !rcumultifile) {
//     simul.connect ("rcu", "rcuall");
//   }
//   if (nsub1 > 0) {
//     simul.connect ("rcuall.out_0", "bandsep.in");
//     simul.connect ("bandsep.out_0", "select.in");
//   } else {
//     simul.connect ("rcuall.out_0", "select.in");
//   }
//   simul.connect ("subseldef.out", "select.sel");
//   //  simul.connect ("select.out_0", "beamformer.in");

//   simul.connect ("select.out_0", "cancel.in");
//   simul.connect ("cancel.out_1", "verifyrfi.in");
//   simul.connect ("cancel.out_0", "beamformer.in");
//   //  simul.connect ("verifyrfi.out_0", "beamformer.in");
//   simul.connect ("cancel.out_0", "beamformer.in");

//   simul.connect ("targettrack.out", "beamformer.target");
//   simul.connect ("rfitrack.out", "beamformer.rfi");
//   simul.connect ("beamformer.out_1", "awe.in");
//   simul.connect ("awe.out_0", "beamformer.weight");
//   if (nsub2 > 0) {
//     simul.connect ("beamformer.out_0", "chansep.in");
//   }

//   simul.connect ("select.out_1", "detection.in");
//   simul.connect ("select.out_2", "calibration.in");
//   simul.connect ("detection.out_0", "cancel.flag");
//   simul.connect ("calibration.out_0", "detection.threshold"); 

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



