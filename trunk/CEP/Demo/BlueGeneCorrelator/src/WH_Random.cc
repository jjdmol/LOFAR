//#  WH_Random.cc: Random generator for BG correlator
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

#include <stdio.h>                // for sprintf
#include <sys/time.h>             // for gettimeofday

#include <Common/Debug.h>
#include <Common/KeyValueMap.h>

#include <WH_Random.h>
#include <DH_Empty.h>
#include <DH_CorrCube.h>

namespace LOFAR
{

  WH_Random::WH_Random (const string& name,
			unsigned int nin,
			unsigned int nout,
			const int FBW)
    : WorkHolder (nin, nout, name, "WH_Random"),
      itsFBW             (FBW) ,
      itsIntegrationTime (0),
      index(0.0){

    char str[8];

    for (unsigned int i = 0; i < nin; i++) {

      sprintf(str, "%d", i);
      getDataManager().addInDataHolder(i, new DH_Empty (string("in_") + str));

    }

    for (unsigned int i = 0; i < nout; i++) {
      
      sprintf(str, "%d", i);
      getDataManager().addOutDataHolder(i, new DH_CorrCube (string("out_") + str));

    }
			
    // seed the random generator with a "random" int
    timeval tv;
    gettimeofday(&tv, NULL);
    srandom(tv.tv_sec);
  }



  WH_Random::~WH_Random() {
  }



  WorkHolder* WH_Random::construct(const string& name,
				   unsigned int nin,
				   unsigned int nout,
				   const int FBW) {

    return new WH_Random(name, nin, nout, FBW);

  }
  


  WH_Random* WH_Random::make(const string& name) {
    return new WH_Random (name, 
			  getDataManager().getInputs(),
			  getDataManager().getOutputs(),
			  itsFBW);
  }


  
  void WH_Random::process() {
    TRACER4("WH_Random::Process()");
    
    for (int channel = 0; channel < NCHANNELS; channel++) {
      for (int station = 0; station < NSTATIONS; station++) {
	for (int sample = 0; sample < NSAMPLES; sample++) {
	  
	  complex<float> rval = complex<float> (-2.5 + 5.0*rand()/(RAND_MAX),
						-2.5 + 5.0*rand()/(RAND_MAX));
	  
	  if (channel == 0 && station == 0 && sample == 0) cout << rval << endl;
	  // 	  complex<float> rval = complex<float> (index++, 0);
	  
	  ((DH_CorrCube*)getDataManager().getOutHolder(0))
	    ->setBufferElement(channel, 
			       station,
			       sample,
			       &rval);
	}
      }
    }
  }


  void WH_Random::dump() {
  }

} // namespace LOFAR
