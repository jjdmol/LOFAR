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
#include <DH_CorrCube.h>
#include <DH_Vis.h>

namespace LOFAR
{

  WH_Random::WH_Random (const string& name,
			const int nelements, 
			const int nsamples,
			const int nchannels,
			const int npolarisations)
    : WorkHolder (0, 1, name, "WH_Random"),
      itsIntegrationTime (0),
      itsIndex           (0),
      itsNelements       (nelements),
      itsNsamples        (nsamples),
      itsNchannels       (nchannels),
      itsNpolarisations  (npolarisations)
  {
    getDataManager().addOutDataHolder(0, new DH_CorrCube ("out",
							  itsNelements, 
							  itsNsamples, 
							  itsNchannels,
							  itsNpolarisations));

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
				   const int nchannels,
				   const int npolarisations) {

    return new WH_Random(name, nelements, nsamples, nchannels, npolarisations);

  }
  


  WH_Random* WH_Random::make(const string& name) {
    return new WH_Random (name, 
			  itsNelements, 
			  itsNsamples, 
			  itsNchannels, 
			  itsNpolarisations);
  }


  
  void WH_Random::process() {
    
    DH_Vis::BufferType acc = DH_Vis::BufferType(0,0);
    float seed = rand();
    seed = seed/(RAND_MAX);

    for (int channel = 0; channel < itsNchannels; channel++) {
      for (int station = 0; station < itsNelements; station++) {
	for (int sample = 0; sample < itsNsamples; sample++) {
	  for (int polarisation = 0; polarisation < itsNpolarisations; polarisation++) {
	    DH_CorrCube::BufferType rval = DH_CorrCube::BufferType ((DH_CorrCube::BufferPrimitive) round(10*seed) , (DH_CorrCube::BufferPrimitive) round(2*seed));
	    if (channel == 0 && station == 0 && polarisation == 0) {
// 	      acc += DH_CorrCube::BufferType(
// 					     rval.real() * rval.real() - 
// 					     rval.imag() * rval.imag(), 
					     
// 					     rval.real() * rval.imag() +
// 					     rval.imag() * rval.real() 
// 					     );

	      acc += rval * rval;
	    }
	  

	  
	    ((DH_CorrCube*)getDataManager().getOutHolder(0))->setBufferElement(sample, 
									       channel,
									       station,
									       polarisation,
									       &rval);
	  }
	}
      }
    }
    cout << "REF [" << itsIndex++ <<"]: " << acc << endl;
  }


  void WH_Random::dump() {
  }

} // namespace LOFAR
