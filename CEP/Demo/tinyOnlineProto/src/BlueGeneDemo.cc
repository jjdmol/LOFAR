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

using namespace LOFAR;

BlueGeneDemo::BlueGeneDemo():
 itsWHCount(0)
{}

BlueGeneDemo::~BlueGeneDemo() {
}

void BlueGeneDemo::define(const KeyValueMap& /*params*/) {

  int NBeamlets = NBEAMLETS;
  int myFBW     = BFBW;
  int NCorr     = NBeamlets * myFBW;
  
  // create the random generators
  WH_Random* myRandom[NBeamlets];
  
  for (int i=0; i<NBeamlets; i++) {
    myRandom[i] = new WH_Random("noname",
				0,
				NVis,
				myFBW);
    itsWHs[itsWHCount] = myRandom[i];
    itsWHCount++;
  }

  // create the correlators
  WH_Correlate* myWHCorrelate[NCorr];
  
  for (int c=0; c<NCorr; c++) {
    myWHCorrelate[c] = new WH_Correlate("noname",
					1);
    itsWHs[itsWHCount] = myWHCorrelate[c];
    itsWHCount++;
  }

  // connect the correlators to the random generators
  int correlator = 0;
  for (int b=0; b<NBeamlets; b++) {
    for (int f=0; f<myFBW; f++) {
      
      myWHCorrelate[correlator]->getDataManager().getInHolder(0)->connectTo
	( *myRandom[b]->getDataManager().getOutHolder(f),
	  TH_Mem() );
      correlator++;
    }
  }
  
  // create the dump workholders
  WH_Dump* myWHDump[NCorr];
  
  for (int c = 0; c < NCorr; c++) {
    myWHDump[c] = new WH_Dump("noname",
			      1, 
			      c);
    itsWHs[itsWHCount] = myWHDump[c];
    itsWHCount++;

    // connect the dump workholders to the correlators
    myWHDump[c]->getDataManager().getInHolder(0)->connectTo
      ( *myWHCorrelate[c]->getDataManager().getOutHolder(0),
	TH_Mem() );
    
  }
}

void BlueGeneDemo::init() {
  // call preprocess method on all WH's
  for (int i = 0; i < itsWHCount; i++) {

    itsWHs[i]->basePreprocess();

    for (int j = 0; j < itsWHs[i]->getDataManager().getInputs(); j++) {
      // TH_Mem is non-blocking. This needs to be set explicitly for now.
      itsWHs[i]->getDataManager().getInHolder(j)->setBlocking(false);
    }

    for (int j = 0; j < itsWHs[i]->getDataManager().getOutputs(); j++) {
      // TH_Mem is non-blocking. This needs to be set explicitly for now.
      itsWHs[i]->getDataManager().getOutHolder(j)->setBlocking(false);
    }

  }
}

void BlueGeneDemo::run(int nsteps) {
  // call process method on all WH's
  for (int s = 0; s < nsteps; s++) {
    for (int i = 0; i < itsWHCount; i++) {

      itsWHs[i]->baseProcess();

    }
  }
}

void BlueGeneDemo::dump() const {
  // call dump method on all WH's
  for (int i = 0; i < itsWHCount; i++) {

    itsWHs[i]->dump();

  } 
}

void BlueGeneDemo::quit() {
}
