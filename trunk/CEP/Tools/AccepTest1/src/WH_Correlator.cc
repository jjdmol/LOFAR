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
			     const int    elements,
			     const int    samples,
			     const int    channels, 
			     const int    targets) 
  : WorkHolder( 1, 1, name, "WH_Correlator"),
    itsNelements(elements),
    itsNsamples (samples),
    itsNchannels(channels),
    itsNtargets (targets)
{

  getDataManager().addInDataHolder(0, new DH_CorrCube("in", 
						      elements, 
						      samples, 
						      channels));

  getDataManager().addOutDataHolder(0, new DH_Vis("out", 
						  elements, channels));

}

WH_Correlator::~WH_Correlator() {
}

WorkHolder* WH_Correlator::construct (const string& name, 
				     const int    nelements, 
				     const int    nsamples,
				     const int    nchannels, 
				     const int    ntargets)
{
  return new WH_Correlator(name, nelements, nsamples, nchannels, ntargets);
}

WH_Correlator* WH_Correlator::make (const string& name) {
  return new WH_Correlator(name,
			   itsNelements,
			   itsNsamples,
			   itsNchannels, 
			   itsNtargets);
}

void WH_Correlator::process() {
  
  int x, y, z;
  double starttime, stoptime, cmults;


  DH_CorrCube *inDH  = (DH_CorrCube*)(getDataManager().getInHolder(0));
  DH_Vis      *outDH = (DH_Vis*)(getDataManager().getOutHolder(0));

  // reset integrator.
  // todo: speed up by creating 0-filled array in C'tor and memcpy in one stroke.
  DH_Vis::BufferType *zero = new DH_Vis::BufferType(0,0);
    for (int fchannel = 0; fchannel < itsNchannels; fchannel++) {
      for (int   station1 = 0; station1 < itsNelements; station1++) {
	for (int station2 = 0; station2 <= station1;    station2++) {
	  outDH->setBufferElement(station1, station2, fchannel,zero);
	}
      }
    }

#define DO_TIMING
#ifdef DO_TIMING
  starttime = timer();
#endif
  
  // calculate the correlations and add to output DataHolder.
  DH_Vis::BufferType s1_val, s2_val;
  for (int sample = 0; sample < itsNsamples; sample++) {
    for (int fchannel = 0; fchannel < itsNchannels; fchannel++) {
      for (int   station1 = 0; station1 < itsNelements; station1++) {
	for (int station2 = 0; station2 <= station1;    station2++) {
	  // todo: use copy-free multiplication
	  // todo: remove inner loop getBufferElement calls; consecutive adressing
	  // todo: do short-> float conversion only once
	  
	  // convert complex<short> to complex<float>
	  s1_val = DH_Vis::BufferType((inDH->getBufferElement(sample, fchannel, station1))->real(),
				      (inDH->getBufferElement(sample, fchannel, station1)->imag()));
	  s2_val = DH_Vis::BufferType((inDH->getBufferElement(sample, fchannel, station2))->real(),
				      (inDH->getBufferElement(sample, fchannel, station2)->imag()));

	  outDH->addBufferElementVal(station1, station2, fchannel, 
				     s1_val * s2_val
				     );

	}
      }
    }
  }
  
#ifdef DO_TIMING
  stoptime = timer();
#endif

#ifdef DO_TIMING
  cmults = itsNsamples * (itsNelements*itsNelements/2 + ceil(itsNelements/2.0));
  cout << "Performance: " << 10e-6*cmults/(stoptime-starttime) << " Mcprod/sec" << endl;
  cout << itsNsamples << " " << itsNelements << " " << 10e-6*cmults/(stoptime-starttime) << endl;
#endif
}				     

void WH_Correlator::dump() {

}

double timer() {
  struct timeval curtime;
  gettimeofday(&curtime, NULL);

  return (curtime.tv_sec + 1.0e-6*curtime.tv_usec);
}
