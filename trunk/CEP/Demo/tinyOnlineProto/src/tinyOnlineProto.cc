//#  tinyOnlineProto.cc: 
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

#include <tinyOnlineProto/tinyOnlineProto.h>

// Prototype specific includes
#include <tinyOnlineProto/WH_SimStation.h>
#include <tinyOnlineProto/WH_FringeControl.h>
#include <tinyOnlineProto/WH_PreProcess.h>
#include <tinyOnlineProto/WH_Transpose.h>
#include <tinyOnlineProto/WH_Correlate.h>
#include <tinyOnlineProto/WH_Dump.h>

// Transport Holders
#include <Transport/TH_Mem.h>
#include <Transport/TH_ShMem.h>

using namespace LOFAR;

tinyOnlineProto::tinyOnlineProto():
  itsWHcount (0)
{}

tinyOnlineProto::~tinyOnlineProto() {
  // delete all WH's
//   for (int i = 0; i < itsWHcount; i++) {
//     delete itsWHs[i];
//   }
//   delete itsWHs;
}

void tinyOnlineProto::define(const KeyValueMap& params) {
  MAC myMac;
  char str[8];
  
  int NStations = myMac.getNumberOfStations();
  int NBeamlets = myMac.getNumberOfBeamlets();
  int myFBW     = myMac.getBeamletSize();
  int NCorr     = NBeamlets * myFBW;

  ////////////////////////////////////////////////////////////////
  //
  // create the station WH's
  //
  ////////////////////////////////////////////////////////////////
  WH_SimStation* myWHStations[NStations];

  for (int s = 0; s < NStations; s++) {
    myWHStations[s] = new WH_SimStation("noname",
					NBeamlets,
					string("/home/gerdes/temp/signal_2.txt"),
					myMac, 
					s);
    itsWHs[itsWHcount] = myWHStations[s];
    itsWHcount++;
  }


  ////////////////////////////////////////////////////////////////
  //
  // create the fringe control WH's
  //
  ////////////////////////////////////////////////////////////////
//   WH_FringeControl myWHFringeControl("noname", NStations, myMac);

//   itsWHs[itsWHcount] = &myWHFringeControl;
  WH_FringeControl* myWHFringeControl;
  myWHFringeControl = new WH_FringeControl("noname", NStations, myMac);

  itsWHs[itsWHcount] = myWHFringeControl;
  itsWHcount++;


  ////////////////////////////////////////////////////////////////
  //
  // create the preprocess WH's
  //
  ////////////////////////////////////////////////////////////////
  WH_PreProcess* myWHPreProcess[NStations];

  for (int s = 0; s < NStations; s++) {
    myWHPreProcess[s] = new WH_PreProcess("noname",
					  NBeamlets,
					  myMac, 
					  s);
    itsWHs[itsWHcount] = myWHPreProcess[s];
    itsWHcount++;

    // connect the preprocess WH's to the station WH's
    // in the same loop we set the DH's to non-blocking, 
    // since TH_Mem doesn't implement a blocking send (yet). 
    for (int b = 0; b < NBeamlets; b++) {
      myWHStations[s]->getDataManager().getOutHolder(b)->connectTo
	(*myWHPreProcess[s]->getDataManager().getInHolder(b),
	 TH_Mem());
      
      myWHStations[s]->getDataManager().getOutHolder(b)->setBlocking(false);
      myWHPreProcess[s]->getDataManager().getInHolder(b)->setBlocking(false);
    }

    
    // connect the preprocess WH's to the fringecontrol WH

    myWHPreProcess[s]->getDataManager().getInHolder(NBeamlets)->connectTo
      (*myWHFringeControl->getDataManager().getOutHolder(s),
       TH_Mem());
    
    myWHPreProcess[s]->getDataManager().getInHolder(NBeamlets)->setBlocking(false);
    myWHFringeControl->getDataManager().getOutHolder(s)->setBlocking(false);
  }


  ////////////////////////////////////////////////////////////////
  //
  // create the Transpose steps
  //
  ////////////////////////////////////////////////////////////////
  WH_Transpose* myWHTranspose[NBeamlets];

  for (int b = 0; b < NBeamlets; b++) {
    myWHTranspose[b] = new WH_Transpose("noname",
					NStations,
					NVis,
					myFBW);
    itsWHs[itsWHcount] = myWHTranspose[b];
    itsWHcount++;

    myWHTranspose[b]->getDataManager().setOutputRate(NVis);
  }
  
  
  ////////////////////////////////////////////////////////////////
  //
  // connect the Transpose steps to the preprocessors;
  // connection scheme implements transpose function
  //
  ////////////////////////////////////////////////////////////////
  for (int b = 0; b < NBeamlets; b++) {
    for (int s = 0; s < NStations; s++) {
      
      myWHTranspose[b]->getDataManager().getInHolder(s)->connectTo
	(*myWHPreProcess[s]->getDataManager().getOutHolder(b),
	 TH_Mem());
      
      myWHTranspose[b]->getDataManager().getInHolder(s)->setBlocking(false);
      myWHPreProcess[s]->getDataManager().getOutHolder(b)->setBlocking(false);
    }
  }
  
  
  ////////////////////////////////////////////////////////////////
  //
  // create the Correlator steps
  //
  ////////////////////////////////////////////////////////////////
  WH_Correlate* myWHCorrelate[NCorr];
  
  for (int c = 0; c < NCorr; c++) {
    myWHCorrelate[c] = new WH_Correlate("noname",
					1);
    
    itsWHs[itsWHcount] = myWHCorrelate[c];
    itsWHcount++;

    myWHCorrelate[c]->getDataManager().setInputRate(NVis);
    myWHCorrelate[c]->getDataManager().setProcessRate(NVis);
    myWHCorrelate[c]->getDataManager().setOutputRate(NVis);

  }


  ////////////////////////////////////////////////////////////////
  //
  // connect the correlator steps to the transpose steps
  //
  ////////////////////////////////////////////////////////////////
  int correlator = 0;
  for (int b = 0; b < NBeamlets; b++) {
    for (int f = 0; f < myFBW; f++) {
      
      myWHCorrelate[correlator]->getDataManager().getInHolder(0)->connectTo
	( *myWHTranspose[b]->getDataManager().getOutHolder(f),
	  TH_Mem() );

      myWHCorrelate[correlator]->getDataManager().getInHolder(0)->setBlocking(false);
      myWHTranspose[b]->getDataManager().getOutHolder(f)->setBlocking(false);

      correlator++;
    }
  }
  

  ////////////////////////////////////////////////////////////////
  //
  // create the Dump steps
  //
  ////////////////////////////////////////////////////////////////
  WH_Dump*  myWHDump[NCorr];
  
  for (int c = 0; c < NCorr; c++) {
    myWHDump[c] = new WH_Dump("noname",
			      1, 
			      c);

    itsWHs[itsWHcount] = myWHDump[c];
    itsWHcount++;
    // connect the dump WH to the correlator WH
    myWHDump[c]->getDataManager().getInHolder(0)->connectTo
      (*myWHCorrelate[c]->getDataManager().getOutHolder(0),
       TH_Mem());

    myWHDump[c]->getDataManager().getInHolder(0)->setBlocking(false);
    myWHCorrelate[c]->getDataManager().getOutHolder(0)->setBlocking(false);

    myWHDump[c]->getDataManager().setInputRate(NVis);
    myWHDump[c]->getDataManager().setProcessRate(NVis);
  }


  

}

void tinyOnlineProto::init() {
  // call preprocess method on all WH's

  for (int wh = 0; wh < itsWHcount; wh++) {
    
    itsWHs[wh]->basePreprocess();
    
  }
}


void tinyOnlineProto::run(int nsteps) {
  // call process method on all WH's  
  for (int i = 0; i < nsteps; i++) {
    for (int wh = 0; wh < itsWHcount; wh++) {

      itsWHs[wh]->process();

    }
  }
}

void tinyOnlineProto::dump() const {
  // call dump method on all WH's
  
  for (int wh = 0; wh < itsWHcount; wh++) {
    
    itsWHs[wh]->dump();

  }
}

void tinyOnlineProto::quit() {
  
  
}
