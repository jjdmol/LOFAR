//#  BlueGeneCorrelator.cc: Runs on BG/L doing complete correlations
//#
//#  Copyright (C) 2002-2004
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


// Application specific includes
#include <BlueGeneCorrelator/BlueGeneCorrelator.h>
#include <BlueGeneCorrelator/definitions.h>

// WorkHolders
#include <BlueGeneCorrelator/WH_Correlate.h>
#include <BlueGeneCorrelator/WH_Random.h>
#include <BlueGeneCorrelator/WH_Dump.h>

// TransportHolders
// #include <Transport/TH_Socket.h>
// #include <Transport/TH_Mem.h>

#ifdef __BLRTS__
#include <mpi.h>
#endif

using namespace LOFAR;

BlueGeneCorrelator::BlueGeneCorrelator () :
  itsWHcount (0),
  itsPort    (BASEPORT),
  itsRank    (-1)
{

#ifdef __BLRTS__
  MPI_Comm_rank(MPI_COMM_WORLD, &itsRank);
#endif
}

BlueGeneCorrelator::~BlueGeneCorrelator() {
}

void BlueGeneCorrelator::define(const KeyValueMap& /*params*/) {

  if (itsRank == 0) {

    itsWHs[0] = new WH_Correlate("noname",
				 1);
  } else {
    itsWHs[0] = new WH_Correlate("noname",
				 0);
  }

  // Dummy workholders to connect to
  WH_Random myWHRandom("noname",
		       1,
		       1,
		       NCHANNELS);

  WH_Dump myWHDump("noname",
		   1,
		   0);

  // the Correlator cannot accept connections because of the limitations 
  // of the current BG/L system software. Therefore all connections must be 
  // opened from this application and accepted in the frontend application.

  if (itsRank == 0) {

    proto_input = new TH_Socket(FRONTEND_IP, FRONTEND_IP, BASEPORT, true);
    proto_output = new TH_Socket(FRONTEND_IP, FRONTEND_IP, BASEPORT+1, false);

    myWHRandom.getDataManager().getOutHolder(0)->connectTo
      ( *itsWHs[0]->getDataManager().getInHolder(0), 
	*proto_input );

    itsWHs[0]->getDataManager().getOutHolder(0)->connectTo
      ( *myWHDump.getDataManager().getInHolder(0),
	*proto_output );

  }    
}

void BlueGeneCorrelator::init() {
  itsWHs[0]->basePreprocess();
}

void BlueGeneCorrelator::run (int nsteps) {
  for (int s = 0; s < nsteps; s++) {
    
    itsWHs[0]->baseProcess();
    
  }
}

void BlueGeneCorrelator::dump () const {

  itsWHs[0]->dump();

}

void BlueGeneCorrelator::postrun() {

  itsWHs[0]->basePostprocess();

}

void BlueGeneCorrelator::quit () {

}

