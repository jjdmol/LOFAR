//#  WH_Random.cc: Random generator for BG correlator
//#
//#  Copyright (C) 2002-2004
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, swe@astron.nl
//#
//#
//#  $Id$

#include <stdio.h>                // for sprintf
#include <sys/time.h>             // for gettimeofday

#include <Common/LofarLogger.h>
#include <Common/KeyValueMap.h>

#include <WH_Random.h>
#include <DH_Empty.h>
#include <DH_CorrCube.h>

namespace LOFAR
{

  WH_Random::WH_Random (const string& name,
			const int nelements, 
			const int nsamples,
			const int nchannels)
    : WorkHolder (0, 1, name, "WH_Random"),
      itsIntegrationTime (0),
      itsIndex           (0),
      itsNelements       (nelements),
      itsNsamples        (nsamples),
      itsNchannels       (nchannels) 
  {
    getDataManager().addOutDataHolder(0, new DH_CorrCube ("out",
							  itsNelements, 
							  itsNsamples, 
							  itsNchannels));

    // seed the random generator with a "random" int
    timeval tv;
    gettimeofday(&tv, NULL);
    srandom(tv.tv_sec);
  }



  WH_Random::~WH_Random() {
  }



  WorkHolder* WH_Random::construct(const string& name,
				   const int nelements, 
				   const int nsamples, 
				   const int nchannels) {

    return new WH_Random(name, nelements, nsamples, nchannels);

  }
  


  WH_Random* WH_Random::make(const string& name) {
    return new WH_Random (name, 
			  itsNelements, 
			  itsNsamples, 
			  itsNchannels);
  }


  
  void WH_Random::process() {
    
    DH_CorrCube::BufferType acc = DH_CorrCube::BufferType(0,0);
    short seed = rand()/(RAND_MAX);

    for (int channel = 0; channel < itsNchannels; channel++) {
      for (int station = 0; station < itsNelements; station++) {
	for (int sample = 0; sample < itsNsamples; sample++) {
	  
	  DH_CorrCube::BufferType rval = DH_CorrCube::BufferType (channel+sample+station*seed , station*seed - 1);
	  
	  if (channel == 0 && station == 0) {
	    acc += DH_CorrCube::BufferType(
				   rval.real() * rval.real() - 
				   rval.imag() * rval.imag(), 
				   
				   rval.real() * rval.imag() +
				   rval.imag() * rval.real() 
				   );
	  }

	  
	  ((DH_CorrCube*)getDataManager().getOutHolder(0))->setBufferElement(channel, 
									     station,
									     sample,
									     &rval);
	}
      }
    }
    cout << "REF [" << itsIndex++ <<"]: " << acc << endl;
  }


  void WH_Random::dump() {
  }

} // namespace LOFAR
