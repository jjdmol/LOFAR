//#  BlueGeneFrontEnd.cc: Definition of the FrontEnd application for the BG Correlator
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

#include <BlueGeneCorrelator/BlueGeneFrontEnd.h>
#include <BlueGeneCorrelator/WH_Random.h>
#include <BlueGeneCorrelator/WH_Correlate.h>
#include <BlueGeneCorrelator/WH_Dump.h>

// TransportHolders
#include <Transport/TH_Socket.h>
//#include <Transport/TH_Mem.h>

using namespace LOFAR;

BlueGeneFrontEnd::BlueGeneFrontEnd (bool frontend):
  isFrontEnd (frontend),
  itsPort    (BASEPORT)
{
}


BlueGeneFrontEnd::~BlueGeneFrontEnd() {
}

void BlueGeneFrontEnd::define(const KeyValueMap& /*params*/) {

  WH_Correlate myWHCorrelate("noname",
			     1);

  if (isFrontEnd) {
  
    itsWHs[0] = new WH_Random("noname",
			      1, 
			      1, 
			      NCHANNELS);
    itsWHs[0]->getDataManager().getOutHolder(0)->connectTo
      ( *myWHCorrelate.getDataManager().getInHolder(0),
	TH_Socket(LOCALHOST_IP, LOCALHOST_IP, itsPort, false) );

  } else {
	  
    itsWHs[0] = new WH_Dump("noname",
			    1, 
			    0);
    myWHCorrelate.getDataManager().getOutHolder(0)->connectTo
      ( *itsWHs[0]->getDataManager().getInHolder(0),
	TH_Socket(LOCALHOST_IP, LOCALHOST_IP, itsPort+1, true) );
    
  } 

  
  

}

void BlueGeneFrontEnd::init() {

  itsWHs[0]->basePreprocess();
//   itsWHs[1]->basePreprocess();

}

void BlueGeneFrontEnd::run(int nsteps) {
  
  for (int s = 0; s < nsteps; s++) {

    itsWHs[0]->baseProcess();
//     itsWHs[1]->baseProcess();

  }
}

void BlueGeneFrontEnd::dump() const {
  
  itsWHs[0]->dump();  
//   itsWHs[1]->dump();

}

void BlueGeneFrontEnd::quit() {
}
