//#  BlueGeneDemo.cc:
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

#include <tinyOnlineProto/BlueGeneDemo.h>
#include <tinyOnlineProto/definitions.h>

// Demo specific includes
// WorkHolders
#include <tinyOnlineProto/WH_Random.h>
#include <tinyOnlineProto/WH_Correlate.h>
#include <tinyOnlineProto/WH_Dump.h>

// TransportHolders
#include <Transport/TH_Mem.h>
#include <Transport/TH_ShMem.h>
#include <Transport/TH_Socket.h>

// Include MPI headers when compiling for BlueGene
#ifdef __BLRTS__
#include <mpi.h>
#endif


using namespace LOFAR;

BlueGeneDemo::BlueGeneDemo(bool BGL, int bgnodes):
  itsOutsideWHCount(0),
  itsInsideWHCount (0),
  itsIsBlueGene    (BGL),
  itsBlueGeneNodes (bgnodes)
{
}

BlueGeneDemo::~BlueGeneDemo() {
}

void BlueGeneDemo::define(const KeyValueMap& /*params*/) {

  int NBeamlets = NBEAMLETS;
  int myFBW     = BFBW;
  int NCorr     = NBeamlets * myFBW;

  if (!itsIsBlueGene) {

    // We're running on the FrontEnd
   
    // create the random generators
    WH_Random* myRandom[MAX_BG_NODES];
    
    for (int i=0; i<itsBlueGeneNodes; i++) {
      myRandom[i] = new WH_Random("noname",
				  0,
				  NVis,
				  myFBW);
      
      //     myRandom[i]->runOnNode(0,0);
      itsOutsideWHs[itsOutsideWHCount] = myRandom[i];
      itsOutsideWHCount++;
    }
    
    // create the correlators
    WH_Correlate* myWHCorrelate[MAX_BG_NODES];
    
    for (int c=0; c<itsBlueGeneNodes; c++) {
      myWHCorrelate[c] = new WH_Correlate("noname",
					  1);
      
      //     myWHCorrelate[c]->runOnNode(1,1);
      
      itsInsideWHs[itsInsideWHCount] = myWHCorrelate[c];
      itsInsideWHCount++;
    }
    
    // connect the correlators to the random generators
    int correlator = 0;
    
    for (int r = 0 ; r < itsBlueGeneNodes; r++) {
	
      TH_Socket proto ("FRONTEND_IP", 
		       "LOCALHOST_IP", 
		       IN_BASEPORT+correlator,
		       true);

      myWHCorrelate[correlator]->getDataManager().getInHolder(0)->connectTo
	( *myRandom[r]->getDataManager().getOutHolder(0),
	  proto );
      correlator++;
    }
    
    // create the dump workholders
    WH_Dump* myWHDump[MAX_BG_NODES];
    
    for (int c = 0; c < itsBlueGeneNodes; c++) {
      myWHDump[c] = new WH_Dump("noname",
				1, 
				c);
      
      //     myWHDump[c]->runOnNode(0, 0);
      
      itsOutsideWHs[itsOutsideWHCount] = myWHDump[c];
      itsOutsideWHCount++;
      
      TH_Socket proto("LOCALHOST_IP", 
		      "FRONTEND_IP", 
		      OUT_BASEPORT+c, 
		      false);
      
      // connect the dump workholders to the correlators
      myWHDump[c]->getDataManager().getInHolder(0)->connectTo
	( *myWHCorrelate[c]->getDataManager().getOutHolder(0),
	  proto );
    }
  } else {

    // We're running on a BlueGene node
#ifdef __BLRTS__
    // only do MPI stuff when using the IBM compiler
    
    int myRank;
    MPI_Comm_rank(MPI_COMM_WORLD, &myRank);
    
    // Create the WorkHolders. Only the correlator Workholder is relevant, the others
    // are only used to connect to. 

    WH_Random myWHRandom("noname",
			 0,
			 NVis,
			 myFBW);
    
    WH_Correlate myWHCorrelate("noname",
			       1);

    WH_Dump myWHDump("noname",
		     1, 
		     0);

    TH_Socket BGinput(FRONTEND_IP, 
		      LOCALHOST_IP, 
		      IN_BASEPORT + myRank,
		      true);

    TH_Socket BGoutput(LOCALHOST_IP, 
		       FRONTEND_IP,
		       OUT_BASEPORT + myRank,
		       true);

    itsInsideWHs[0] = &myWHCorrelate;
    itsInsideWHCount++;

    myWHCorrelate.getDataManager().getInHolder(0)->connectTo
      ( *myWHRandom.getDataManager().getOutHolder(0),
	BGinput );

    myWHDump.getDataManager().getInHolder(0)->connectTo
      ( *myWHCorrelate.getDataManager().getOutHolder(0),
	BGoutput );


#else 
    // this should not normally occur. 
    cerr << "Trying to run BlueGene code not compiled with the IBM compiler. Exit." << endl;
    exit(-1);
#endif 
  }
}
  
  
  void BlueGeneDemo::init() {
    // call preprocess method on all WH's
  if (!itsIsBlueGene) {
    for (int i = 0; i < itsOutsideWHCount; i++) {
      
      itsOutsideWHs[i]->basePreprocess();
      
      //     for (int j = 0; j < itsWHs[i]->getDataManager().getInputs(); j++) {
      //       // TH_Mem is non-blocking. This needs to be set explicitly for now.
      //       itsWHs[i]->getDataManager().getInHolder(j)->setBlocking(false);
      //     }
      
      //     for (int j = 0; j < itsWHs[i]->getDataManager().getOutputs(); j++) {
      //       // TH_Mem is non-blocking. This needs to be set explicitly for now.
      //       itsWHs[i]->getDataManager().getOutHolder(j)->setBlocking(false);
      //     }
      
    }
  } else {
    for ( int i = 0; i < itsInsideWHCount; i++ ) {
      
      itsInsideWHs[i]->basePreprocess() ;
      
    }
  }
}
  
void BlueGeneDemo::run(int nsteps) {
  // call process method on all WH's
  for (int s = 0; s < nsteps; s++) {
    if (itsIsBlueGene) {
      for (int i = 0; i < itsInsideWHCount; i++) {

	itsInsideWHs[i]->baseProcess();

      }
    } else {
      for (int i = 0; i < itsOutsideWHCount; i++) {
	itsOutsideWHs[i]->baseProcess();
      }
    }

  }
}

void BlueGeneDemo::dump() const {
  // call dump method on all WH's
  if (itsIsBlueGene) {
    for (int i = 0; i < itsInsideWHCount; i++) {
      
      itsInsideWHs[i]->dump();

    }
  } else {
    for (int i = 0; i < itsOutsideWHCount; i++) {

      itsOutsideWHs[i]->dump();

    }
  }
}

void BlueGeneDemo::quit() {
}
