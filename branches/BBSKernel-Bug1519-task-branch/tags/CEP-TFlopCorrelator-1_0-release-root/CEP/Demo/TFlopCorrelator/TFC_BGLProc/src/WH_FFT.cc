//#  WH_FFT.cc: 256 kHz polyphase filter
//#
//#  Copyright (C) 2002-2005
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//#  $Id$

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

//# Includes
#include <APS/ParameterSet.h>
#include <WH_FFT.h>

// #include <TFC_Interface/DH_PPF.h>
// #include <TFC_Interface/DH_CorrCube.h>

#include <fftw.h>

using namespace LOFAR;

WH_FFT::WH_FFT(const string& name) :
  WorkHolder (2, 5, name, "WH_Correlator"),
  itsNtaps   (0),
  itsNSamples(0),
  itsCpF     (0),
  itsInputs  (0)
{
  ACC::APS::ParameterSet  myPS("TFlopCorrelator.cfg");
  itsNtaps      = myPS.getInt32("WH_FFT.taps");
  itsNSamples   = myPS.getInt32("WH_FFT.times");
  itsCpF        = myPS.getInt32("Corr_per_Filter");
  itsInputs     = myPS.getInt32("WH_FFT.inputs");
  // todo: Pr-correlation correction DH in channel 0
  //   getDataManager().addInDataHolder(0, new DH_??("input", itsSBID));
  
  for (int c = 0; c < itsInputs; c++) {
    getDataManager().addInDataHolder(c, new DH_PPF("input", itsSBID));
  }
  
  for (int c = 0; c < itsCpF; c++) {
    getDataManager().addOutDataHolder(c, new DH_CorrCube("output", itsSBID)); 
  }
  
  // FFTW parameters
  itsFFTDirection = FFTW_FORWARD;
  itsFFTPlan = fftw_create_plan(itsNtaps, itsFFTDirection, FFTW_MEASURE);
  
  fft_in  = static_cast<FilterType*>(malloc(itsNtaps * sizeof(FilterType)));
  fft_out = static_cast<FilterType*>(malloc(itsNtaps * sizeof(FilterType)));
}

WH_FFT::~WH_FFT() {
}

WorkHolder* WH_FFT::construct(const string& name) {
  return new WH_FFT(name);
}

WH_FFT* WH_FFT::make(const string& name) {
  return new WH_FFT(name);
}

void WH_FFT::preprocess() {
}

void WH_FFT::process() {

  for (int i = 0; i < itsNinputs; i++) {
    // merge the input DH's into one buffer (!! POTENTIAL HOTSPOT !!)
    memcpy(fft_in, getDataManager().getInHolder(i)->getDataPtr(), itsNtaps * sizeof(FilterType));
  }

  fftw_one(itsFFTPlan, 
	   reinterpret_cast<fftw_complex *>(fft_in), 
	   reinterpret_cast<fftw_complex *>(fft_out));

  for (int cor = 0; cor < itsCpF; cor++) { 
    // assume that channel 0 is bogus
    // we can now devide the 255 remaining channels among 5(default) correlators
    
    // can we do this without memcpy?
    memcpy(static_cast<DH_CorrCube*>(getDataManager().getOutHolder(cor))->getBuffer(),  
	   fft_out + 1 + cor * ( itsNtaps - 1 ) / itsCpF,
	   itsNtaps);

    // static_cast<DH_CorrCube*>(getDataManager().getOutHolder(cor))->getBuffer() = fft_out + 1 + cor * ( itsNtaps - 1 ) / itsCpF ;
  }
}


void WH_FFT::postprocess() {
  fftw_destroy_plan(itsFFTPlan);

  free(fft_in);
  free(fft_out);
}

void WH_FFT::dump() const{
}

