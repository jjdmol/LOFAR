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

#include <BlueGeneCorrelator/BlueGeneCorrelator.h>

// Application specific includes

// WorkHolders
#include <BlueGeneCorrelator/WH_Correlate.h>

// TransportHolders
#include <Transport/TH_Socket.h>

using namespace LOFAR;

BlueGeneCorrelator::BlueGeneCorrelator (int rank) :
  itsWHcount (0),
  itsPort    (BASEPORT),
  itsRank    (rank) 
{
}

BlueGeneCorrelator::~BlueGeneCorrelator() {
}

void BlueGeneCorrelator::define(const KeyValueMap& /*params*/) {
  WH_Correlate myWHCorrelate("noname",
			     1);
			     
}

void BlueGeneCorrelator::init() {

}

void BlueGeneCorrelator::run (int nsteps) {
  for (int s = 0; s < nsteps; s++) {
    for (int w = 0; w < itsWHcount; w++) {
    
      itsWHs[w]->baseProcess();

    }
  }
}

void BlueGeneCorrelator::dump () const {

  for (int w = 0; w < itsWHcount; w++) {
 
    itsWHs[w]->dump();

  }
}

void BlueGeneCorrelator::quit () {

}


