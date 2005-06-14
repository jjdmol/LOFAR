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
#include <ACC/ParameterSet.h>
#include <WH_FFT.h>

#include <TFC_Interface/DH_PPF.h>
#include <TFC_Interface/DH_CorrCube.h>

#include <fftw.h>

using namespace LOFAR;

WH_FFT::WH_FFT(const string& name) :
  WorkHolder( 1, 1, name, "WH_Correlator")
{
   ACC::ParameterSet  myPS("TFlopCorrelator.cfg");
   //ParameterCollection	myPC(myPS);
   itsNtaps      = myPS.getInt("WH_FFT.taps");
   itsNStations  = myPS.getInt("WH_FFT.stations");
   itsNTimes     = myPS.getInt("WH_FFT.times");
   itsNFChannels = myPS.getInt("WH_FFT.freqs");
   itsNPol       = myPS.getInt("WH_FFT.pols");
   itsCpF        = myPS.getInt("Corr_per_Filter");

   // todo: Pr-correlation correction DH in channel 0
   //   getDataManager().addInDataHolder(0, new DH_??("input", itsSBID));

   getDataManager().addInDataHolder(0, new DH_SubBand("input", itsSBID));

   for (int c=0; c<itsCpF; c++) {
     getDataManager().addOutDataHolder(0, new DH_CorrCube("output", itsSBID)); 
   }
   
   // FFTW parameters
   itsFFTDirection = FFTW_FORWARD;
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
  itsFFTPlan = fftw_create_plan(itsNStations, itsFFTDirection, FFTW_MEASURE);

  fft_in  = static_cast<FilterType*> (malloc(itsNStations * sizeof(FilterType)));
  fft_out = static_cast<FilterType*> (malloc(itsNStations * sizeof(FilterType)));
}

void WH_FFT::process() {

  for (int channel = 0; channel < itsNFChannels; channel++) {

    // reset the fft_in buffer in case we used it before.

    // todo: get data from DataHolder
    fftw_one(itsFFTPlan, 
	     reinterpret_cast<fftw_complex *>(fft_in), 
	     reinterpret_cast<fftw_complex *>(fft_out));
  }
}


void WH_FFT::postprocess() {
  fftw_destroy_plan(itsFFTPlan);

  free(fft_in);
  free(fft_out);
}

void WH_FFT::dump() {
}

