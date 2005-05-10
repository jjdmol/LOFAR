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
 itsSBID       (subBandID)
{

   ACC::ParameterSet  myPS("TFlopCorrelator.cfg");
   //ParameterCollection	myPC(myPS);
   itsNtaps      = myPS.getInt("WH_SubBand.taps");
   itsNStations  = myPS.getInt("WH_SubBand.stations");
   itsNTimes     = myPS.getInt("WH_SubBand.times");
   itsNFChannels = myPS.getInt("WH_SubBand.freqs");
   itsNPol       = myPS.getInt("WH_SubBand.pols");
   itsCpF        = myPS.getInt("Corr_per_Filter");

   getDataManager().addInDataHolder(0, new DH_SubBand("input", itsSBID));

   for (int c=0; c<itsCpF; c++) {
     getDataManager().addOutDataHolder(0, new DH_CorrCube("output", itsSBID)); 
   }
   
   //todo: Add DH for filter coefficients;
   //      need functionality like the CEPFrame setAutotrigger.

   delayLine = new FilterType*[itsNStations];
   delayPtr  = new FilterType*[itsNStations];
   coeffPtr  = new float*[itsNStations];

   for (int filter = 0; filter <  itsNStations; filter++) {

     // Initialize the delay line
     delayLine[filter] = new FilterType[2*itsNtaps]; 
     memset(delayLine[filter], 0, 2*itsNtaps*sizeof(FilterType));
     delayPtr[filter] = delayLine[filter];

     // Need input: filter coefficients
     coeffPtr[filter] = new float[itsNtaps];
     for (int j = 0; j < itsNtaps; j++) {
       coeffPtr[filter][j] = (j + 1);
     }
   }

   // FFT
   itsFFTDirection = FFTW_FORWARD;
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
  itsFFTPlan = fftw_create_plan(itsNStations, itsFFTDirection, FFTW_MEASURE);

  fft_in  = static_cast<FilterType*> (malloc(itsNStations * sizeof(FilterType)));
  fft_out = static_cast<FilterType*> (malloc(2 * itsNStations * sizeof(FilterType)));
}

void WH_SubBand::process() {

  // reset the fft_in buffer in case we used it before.
  memset(fft_in, 0, itsNStations * 2);

  for (int f = 0; f < itsNStations; f++) {
    for (int i = 0; i < itsNtaps; i++) { 

      fft_in[f] += coeffPtr[f][i] * delayLine[f][i];

    }
  }

  fftw_one(itsFFTPlan, 
	   reinterpret_cast<fftw_complex *>(fft_in), 
	   reinterpret_cast<fftw_complex *>(fft_out));
}


void WH_SubBand::postprocess() {
  fftw_destroy_plan(itsFFTPlan);

  free(fft_in);
  free(fft_out);
}

void WH_SubBand::dump() {
}

void WH_SubBand::adjustDelayPtr() { 
  for (int i = itsNtaps - 2; i >= 0; i--) {
    delayLine[i+1] = delayLine[i];
  }
}
