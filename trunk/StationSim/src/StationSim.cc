// StationSim.cc
//
//  Copyright (C) 2002
//  ASTRON (Netherlands Foundation for Research in Astronomy)
//  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
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
//  $Log$
//


// This is the main program for the Station prototype simulation using the 
// StationSim simulation environment.
// 

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "StationSim/StationSim.h"
#include "StationSim/WH_RCU.h"
#include "StationSim/WH_AWE.h"
#include "StationSim/WH_BeamFormer.h"
#include "StationSim/WH_Merge.h"
#include "StationSim/WH_RCUAdd.h"
#include "StationSim/WH_Filter.h"
#include "StationSim/WH_Selector.h"
#include "StationSim/WH_SubBandSel.h"
#include "StationSim/WH_TargetTrack.h"
#include "StationSim/WH_RFITrack.h"

#include "BaseSim/Transport.h"
#include "BaseSim/Step.h"
#include "BaseSim/WH_Empty.h"
#include "BaseSim/ParamBlock.h"
#include "BaseSim/Simul.h"
#include "BaseSim/Profiler.h"
#include TRANSPORTERINCLUDE
#ifdef HAVE_CORBA
# include "BaseSim/Corba/TH_Corba.h"
# include "BaseSim/Corba/BS_Corba.h"
#endif
#include "BaseSim/Simul2XML.h"

#include "Common/Debug.h"
#include <Common/lofar_iostream.h>
#include <Common/lofar_strstream.h>
#include <Common/lofar_vector.h>



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
  int nrcu = params.getInt ("nrcu", 1);
  int nsub = params.getInt ("nsubband", 16);
  int nsel = params.getInt ("nselband", 4);
  int nbeam = params.getInt ("nbeam", 1);
  int maxNtarget = params.getInt ("maxntarget", 1);
  int maxNrfi = params.getInt ("maxnrfi", 1);
  int aweRate = params.getInt ("awerate", 1);
  string rcuName = params.getString ("rcufilename", "");
  string selName = params.getString ("selfilename", "");
  string coeffName = params.getString ("coefffilename", "");
  string dipoleName = params.getString ("dipolefilename", "");
  string targetName = params.getString ("targettrackfilename", "");
  string rfiName = params.getString ("rfitrackfilename", "");

  Step rcu (WH_RCU ("", nrcu, rcuName),
	    "rcu", false);
  Step merge (WH_Merge ("", nrcu, 1),
	      "merge", false);
  Step filter (WH_Filter ("", 1, nrcu, nsub, coeffName),
	       "filter", false);
  Step subsel (WH_SubBandSel ("", nsub, selName),
	       "subseldef", false);
  Step select (WH_Selector ("", 1, nrcu, nsub, nsel),
	       "select", false);
  Step target (WH_TargetTrack ("", maxNtarget, targetName),
	       "targettrack", false);
  Step rfi (WH_RFITrack ("", maxNtarget, rfiName),
	    "rfitrack", false);
  Step beam (WH_BeamFormer("", 2, nrcu, nsel, nbeam, maxNtarget, maxNrfi,
			   dipoleName),
	     "beamformer", false);
  Step awe (WH_AWE("", 1, nsel, nbeam),
	    "awe", false);
  awe.setOutRate (aweRate);
  beam.setInRate (aweRate, 1);
  //  awe.setRate (aweRate);
  //  beam.setOutRate (aweRate, 1);

  // Add the steps to the Simul.
  WH_Empty wh;
  Simul simul(wh, "StationSim");
  simul.addStep (rcu);
  simul.addStep (merge);
  simul.addStep (filter);
  simul.addStep (subsel);
  simul.addStep (select);
  simul.addStep (target);
  simul.addStep (rfi);
  simul.addStep (beam);
  simul.addStep (awe);
  setSimul (simul);

  // Connect the steps.
  simul.connect ("rcu", "merge");
  simul.connect ("merge.out_0", "filter.in");
  simul.connect ("filter.out_0", "select.in");
  simul.connect ("subseldef.out", "select.sel");
  simul.connect ("select.out_0", "beamformer.in");
  simul.connect ("targettrack.out", "beamformer.target");
  simul.connect ("rfitrack.out", "beamformer.rfi");
  simul.connect ("beamformer.out_1", "awe.in");
  simul.connect ("awe.out_0", "beamformer.weight");
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



