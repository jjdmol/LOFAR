//#  WH_Correlator.cc:
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

#include <stdio.h>

// General includes
#include <Common/LofarLogger.h>

// Application specific includes
#include <DH_Vis.h>
#include <DH_CorrCube.h>
#include <WH_Correlator.h>

using namespace LOFAR;

WH_Correlator::WH_Correlator(const string& name, 
			     unsigned int nin, 
			     unsigned int nout, 
			     const int    elements,
			     const int    samples,
			     const int    channels) 
  : WorkHolder( nin, nout, name, "WH_Correlator"),
    itsNelements(elements),
    itsNsamples (samples),
    itsNchannels(channels)
{

  char str[8];
  for (unsigned int i = 0; i < nin; i++) {
    sprintf(str, "%d", i);
    getDataManager().addInDataHolder(i, new DH_CorrCube(string("in_") + str, 
							elements, 
							samples, 
							channels));
  }

  for (unsigned int i = 0; i < nout; i++) {
    sprintf(str, "%d", i);
    getDataManager().addOutDataHolder(i, new DH_Vis(string("out_") + str, 
						    elements, channels));
  }

}

WH_Correlator::~WH_Correlator() {
}

WorkHolder* WH_Correlator::construct (const string& name, 
				     unsigned int nin, 
				     unsigned int nout, 
				     const int    nelements, 
				     const int    nsamples,
				     const int    nchannels)
{
  return new WH_Correlator(name, nin, nout, nelements, nsamples, nchannels);
}

WH_Correlator* WH_Correlator::make (const string& name) {
  return new WH_Correlator(name,
			   getDataManager().getInputs(),
			   getDataManager().getOutputs(),
			   itsNelements,
			   itsNsamples,
			   itsNchannels);
}

void WH_Correlator::process() {
  
  int x, y, z;

  int teller = 0;

  double starttime, stoptime, cmults;

  DH_CorrCube::BufferType*  signal;
  DH_Vis::BufferType*       corr;

  // try to access the DataHolder buffer directly to prevent extra memcpy's
  signal = ((DH_CorrCube*)getDataManager().getInHolder(0))->getBuffer();
  corr   = (DH_Vis::BufferType*) malloc(itsNelements * itsNelements * sizeof(DH_Vis::BufferType));
  
  for (int i = 0; i<itsNelements; i++) {
    for (int j = 0; j<itsNelements; j++) {

      *(corr + i*itsNelements + j) = DH_Vis::BufferType (0,0);
    }
  }

  starttime = timer();

  teller=0;
  for (x = 0; x < itsNsamples; x++) {
    for (y = 0; y < itsNelements; y++) {
      for (z = 0; z <= y; z++) {

 	*(corr+y*itsNelements+z) += 
	  *(signal+x*itsNelements+y) * *(signal+x*itsNelements+z);

      }
    }
  }
  
  stoptime = timer();

  cmults = itsNsamples * (itsNelements*itsNelements/2 + ceil(itsNelements/2.0));
  //  cout << "Performance: " << 10e-6*cmults/(stoptime-starttime) << " Mcprod/sec" << endl;
  cout << itsNsamples << " " << itsNelements << " " << 10e-6*cmults/(stoptime-starttime) << endl;

  memcpy(((DH_Vis*)getDataManager().getOutHolder(0))->getBuffer(), corr, itsNelements * itsNelements * sizeof(DH_Vis::BufferType));
  free(corr);
}				     

void WH_Correlator::dump() {

}

double timer() {
  struct timeval curtime;
  gettimeofday(&curtime, NULL);

  return (curtime.tv_sec + 1.0e-6*curtime.tv_usec);
}
