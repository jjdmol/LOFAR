//#  BlueGeneFrontEnd.cc: Runs on the FrontEnd of BG/L to provide & dump data
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

// Application specific includes

// WorkHolders
#include <BlueGeneCorrelator/WH_Random.h>
#include <BlueGeneCorrelator/WH_Dump.h>
#include <BlueGeneCorrelator/WH_Correlate.h>

// TransportHolders
#include <Transport/TH_Mem.h>
#include <Transport/TH_Socket.h>

using namespace LOFAR;

BlueGeneFrontEnd::BlueGeneFrontEnd (bool input):
  itsWHs     (0),
  itsWHcount (0),
  itsInput   (input),
  itsPort    (BASEPORT)
{
}

BlueGeneFrontEnd::~BlueGeneFrontEnd () {
}

void BlueGeneFrontEnd::define(const KeyValueMap& /*params*/) {
  
  if (itsInput) {

    // this is the input application. Create a WH_Random object.
    
    itsWHs = new WH_Random("noname",
			   0,
			   1, 
			   NVis*BFBW);
    
    WH_Correlate myWHCorrelate("noname",
			       1);
    TH_Socket TH_proto(LOCALHOST_IP, LOCALHOST_IP, itsPort, true);
    
    itsWHs->getDataManager().getOutHolder(0)->connectTo
      ( *myWHCorrelate.getDataManager().getInHolder(0),
	TH_proto );
        
  } else {

    // this is the output application. Create a WH_Dump object.

    itsWHs = new WH_Dump("noname",
			 1,
			 1);

    WH_Correlate myWHCorrelate("noname",
			       1);
    TH_Socket TH_proto(LOCALHOST_IP, FRONTEND_IP, itsPort+1, false);
    
    myWHCorrelate.getDataManager().getOutHolder(0)->connectTo
      ( *itsWHs->getDataManager().getInHolder(0),
	TH_proto );
		     
  } 
}


void BlueGeneFrontEnd::init() {
  itsWHs->basePreprocess();
}


void BlueGeneFrontEnd::run(int nsteps) {
  
  for (int s = 0; s < nsteps; s++) {

    itsWHs->baseProcess();

  }
}

void BlueGeneFrontEnd::dump() const {
  
  itsWHs->dump();

}

void BlueGeneFrontEnd::quit() {
}
