//#  WH_SubBand.cc: 256 kHz polyphase filter
//#
//#  Copyright (C) 2002-2005
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//#  $Id$

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

//# Includes
#include <ACC/ParameterSet.h>
#include <WH_SubBand.h>

#include <TFlopCorrelator/DH_SubBand.h>
#include <TFlopCorrelator/DH_CorrCube.h>

#include <fftw.h>

using namespace LOFAR;

WH_SubBand::WH_SubBand(const string& name,
		       const short   subBandID):
 WorkHolder( 1, 1, name, "WH_Correlator"),
 itsSBID   (subBandID)
{

   ACC::ParameterSet  myPS("TFlopCorrelator.cfg");
   //ParameterCollection	myPC(myPS);
   itsNtaps     = myPS.getInt("WH_SubBand.taps");
   itsFFTLen    = myPS.getInt("WH_SubBand.fftlen");
   itsCpF       = myPS.getInt("Corr_per_Filter");

   getDataManager().addInDataHolder(0, new DH_SubBand("input", itsSBID));

   for (int c=0; c<itsCpF; c++) {
     getDataManager().addOutDataHolder(0, new DH_CorrCube("output", itsSBID)); 
   }
   
   //todo: Add DH for filter coefficients;
   //      need functionalit like the CEPFrame setAutotrigger.


   for (int filter = 0; filter <  itsFFTLen; filter++) {

     // Initialize the delay line
     delayLine[filter] = new FilterType[2*itsNtaps]; 
     memset(delayLine[filter],0,2*itsNtaps*sizeof(FilterType));
     delayPtr[filter] = delayLine[filter];

     // Need input: filter coefficients
     coeffPtr[filter] = new float[itsNtaps];
     for (int j = 0; j < itsNtaps; j++) {
       coeffPtr[filter][j] = (j + 1);
     }

   }

   
}

WH_SubBand::~WH_SubBand() {
}

WorkHolder* WH_SubBand::construct(const string& name,
				  const short   SBID) {
  return new WH_SubBand(name, SBID);
}

WH_SubBand* WH_SubBand::make(const string& name) {
  return new WH_SubBand(name, itsSBID);
}

void WH_SubBand::preprocess() {
  itsFFTPlan = fftw_create_plan(itsNsubbands, itsFFTDirection, FFTW_MEASURE);
}

void WH_SubBand::process() {
  acc = 0.0 + 0.0i;

  fftw_complex* in;
  fftw_complex* out;

  for (int f = 0; f < itsFFTLen; f++) {
    for (int i = 0; i < itsNtaps; i++) { 
      
      acc += static_cast<FilterType>(coeffPtr[f][i]) * static_cast<FilterType>(delayLine[f][i]);
      
    }
  }

  fftw_one(itsFFTPlan, in, out);
}


void WH_SubBand::postprocess() {
  fftw_destroy_plan(itsFFTPlan);
}

void WH_SubBand::dump() {
}

void WH_SubBand::adjustDelayPtr() { 
  for (int i = itsNtaps - 2; i >= 0; i--) {
    delayLine[i+1] = delayLine[i];
  }
}
