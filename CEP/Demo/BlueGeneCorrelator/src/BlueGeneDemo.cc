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

using namespace LOFAR;

BlueGeneDemo::BlueGeneDemo(bool BGL):
  itsOutsideWHCount(0),
  itsInsideWHCount (0),
  itsIsBlueGene    (BGL)
{
}

BlueGeneDemo::~BlueGeneDemo() {
}

void BlueGeneDemo::define(const KeyValueMap& /*params*/) {

  int NBeamlets = NBEAMLETS;
  int myFBW     = BFBW;
  int NCorr     = NBeamlets * myFBW;

  int PortNo    = 2048;
  
  // create the random generators
  WH_Random* myRandom[NBeamlets];
  
  for (int i=0; i<NBeamlets; i++) {
    myRandom[i] = new WH_Random("noname",
				0,
				NVis,
				myFBW);
    
//     myRandom[i]->runOnNode(0,0);
    itsOutsideWHs[itsOutsideWHCount] = myRandom[i];
    itsOutsideWHCount++;
  }
  
  // create the correlators
  WH_Correlate* myWHCorrelate[NCorr];
  
  for (int c=0; c<NCorr; c++) {
    myWHCorrelate[c] = new WH_Correlate("noname",
					1);
    
//     myWHCorrelate[c]->runOnNode(1,1);
    
    itsInsideWHs[itsInsideWHCount] = myWHCorrelate[c];
    itsInsideWHCount++;
  }
  
  // connect the correlators to the random generators
  int correlator = 0;
  for (int b=0; b<NBeamlets; b++) {
    for (int f=0; f<myFBW; f++) {
      
      TH_Socket proto ("localhost", "localhost", PortNo++);
      myWHCorrelate[correlator]->getDataManager().getInHolder(0)->connectTo
	( *myRandom[b]->getDataManager().getOutHolder(f),
	  proto );
      correlator++;
    }
  }

  // create the dump workholders
  WH_Dump* myWHDump[NCorr];
  
  for (int c = 0; c < NCorr; c++) {
    myWHDump[c] = new WH_Dump("noname",
			      1, 
			      c);
    
//     myWHDump[c]->runOnNode(0, 0);
    
    itsOutsideWHs[itsOutsideWHCount] = myWHDump[c];
    itsOutsideWHCount++;
  
    TH_Socket proto("localhost", "localhost", PortNo++, false);

    // connect the dump workholders to the correlators
    myWHDump[c]->getDataManager().getInHolder(0)->connectTo
      ( *myWHCorrelate[c]->getDataManager().getOutHolder(0),
	proto );
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
