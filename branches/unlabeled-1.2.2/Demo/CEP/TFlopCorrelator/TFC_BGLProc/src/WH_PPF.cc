//#  WH_PPH.cc: 256 kHz polyphase filter
//#
//#  Copyright (C) 2002-2005
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

//# Includes
#include <APS/ParameterSet.h>
#include <WH_PPF.h>

#include <TFC_Interface/DH_PPF.h>
#include <TFC_Interface/DH_CorrCube.h>

#include <Common/Timer.h>

#include <fftw.h>
#include <assert.h>

using namespace LOFAR;


FIR::FIR()
{
  for (int tap = 0; tap < NR_TAPS; tap ++) {
    itsDelayLine[tap] = makefcomplex(0.0f, 0.0f);
  }
}


extern "C"
{
  void _filter(fcomplex delayLine[NR_TAPS], const float weights[NR_TAPS], const i16complex samples[], fcomplex *out, int nr_samples_div_16);
  //void _convert(fcomplex dest[], const i16complex src[], int size);
}

WH_PPF::WH_PPF(const string& name, const short subBandID):
  WorkHolder(1, NR_CORRELATORS_PER_FILTER, name, "WH_Correlator"),
  itsSubBandID(subBandID)
{
#if 0
  //assert(((int)&itsFIRs[0][0][0].itsDelayLine[0])&7==0);
  for (int tap = 0; tap < NR_TAPS; tap ++) {
    itsWeights[0][tap] = itsWeights[0][tap] = tap + 2;
    itsFIRs[0][0][0].itsDelayLine[tap] = makefcomplex(tap,1);
  }
  fcomplex dummy;
  itsFIRs[0][0][0].test(&dummy,itsWeights[0],33,makefcomplex(2,3));
  std::cerr << itsFIRs[0][0][0].itsDelayLine[0] << '\n';
  std::cerr << itsFIRs[0][0][0].itsDelayLine[1] << '\n';
  std::cerr << itsFIRs[0][0][0].itsDelayLine[2] << '\n';
  std::cerr << dummy << '\n';
  exit(0);
#endif
  ACC::APS::ParameterSet myPS("TFlopCorrelator.cfg");

  int NrTaps		     = myPS.getInt32("PPF.NrTaps");
  int NrStations	     = myPS.getInt32("PPF.NrStations");
  int NrStationSamples	     = myPS.getInt32("PPF.NrStationSamples");
  int NrPolarizations	     = myPS.getInt32("PPF.NrPolarizations");
  int NrSubChannels	     = myPS.getInt32("PPF.NrSubChannels");
  int NrCorrelatorsPerFilter = myPS.getInt32("PPF.NrCorrelatorsPerFilter");
  
  assert(NrTaps			== NR_TAPS);
  assert(NrStations		== NR_STATIONS);
  assert(NrStationSamples	== NR_STATION_SAMPLES);
  assert(NrPolarizations	== NR_POLARIZATIONS);
  assert(NrSubChannels		== NR_SUB_CHANNELS);
  assert(NrCorrelatorsPerFilter == NR_CORRELATORS_PER_FILTER);

  getDataManager().addInDataHolder(0, new DH_PPF("input", itsSubBandID, myPS));
  
  for (int corr = 0; corr < NR_CORRELATORS_PER_FILTER; corr ++) {
    getDataManager().addOutDataHolder(corr, new DH_CorrCube("output", itsSubBandID)); 
  }

  for (int chan = 0; chan < NR_SUB_CHANNELS; chan ++)
    for (int tap = 0; tap < NR_TAPS; tap ++)
      itsWeights[chan][tap] = 1.0f;

  fftw_import_wisdom_from_string("(FFTW-2.1.5 (256 529 -1 0 1 1 1 352 0) (128 529 -1 0 1 1 0 2817 0) (64 529 -1 0 1 1 0 1409 0) (32 529 -1 0 1 1 0 705 0) (16 529 -1 0 1 1 0 353 0) (8 529 -1 0 1 1 0 177 0) (4 529 -1 0 1 1 0 89 0) (2 529 -1 0 1 1 0 45 0))");
  itsFFTWPlan = fftw_create_plan(NR_SUB_CHANNELS, FFTW_FORWARD, FFTW_USE_WISDOM);
}


WH_PPF::~WH_PPF()
{
  fftw_destroy_plan(itsFFTWPlan);
}


WorkHolder* WH_PPF::construct(const string& name, const short subBandID)
{
  return new WH_PPF(name, subBandID);
}


WH_PPF* WH_PPF::make(const string& name)
{
  return new WH_PPF(name, itsSubBandID);
}


void WH_PPF::preprocess()
{
}


#if defined __xlC__
inline __complex__ float to_fcomplex(i16complex z)
{
  return (float) z.real() + 1.0fi * z.imag();
}
#else
#define to_fcomplex(Z) (static_cast<fcomplex>(Z))
#endif


#if 0
void WH_PPF::process()
{
  typedef i16complex inputType[NR_STATIONS][NR_SAMPLES_PER_INTEGRATION][NR_SUB_CHANNELS][NR_POLARIZATIONS];
  typedef fcomplex (*outputType)[NR_CHANNELS_PER_CORRELATOR][NR_STATIONS][NR_SAMPLES_PER_INTEGRATION][NR_POLARIZATIONS];

  inputType *input = (inputType *) static_cast<DH_PPF*>(getDataManager().getInHolder(0))->getBuffer();
  outputType outputs[NR_CORRELATORS_PER_FILTER];
  fcomplex   fftInData[NR_SUB_CHANNELS], fftOutData[NR_SUB_CHANNELS];

  for (int corr = 0; corr < NR_CORRELATORS_PER_FILTER; corr ++) {
    outputs[corr] = (outputType) static_cast<DH_CorrCube*>(getDataManager().getOutHolder(corr))->getBuffer();
  }

  for (int stat = 0; stat < NR_STATIONS; stat ++) {
    for (int timeSlot = 0; timeSlot < NR_SAMPLES_PER_INTEGRATION; timeSlot ++) {
      for (int pol = 0; pol < NR_POLARIZATIONS; pol ++) {
	for (int chan = 0; chan < NR_SUB_CHANNELS; chan ++) {
	  fcomplex sample = to_fcomplex((*input)[stat][timeSlot][chan][pol]);
	  fftInData[pol][chan] = itsFIRs[stat][chan][pol].processNextSample(sample,itsWeights[chan]);
	}

	fftw_one(itsFFTWPlan, (fftw_complex *) fftInData, (fftw_complex *) fftOutData);

	// Assume that channel 0 is bogus.  We now divide the remaining channels
	// over the correlators.

	for (int corr = 0; corr < NR_CORRELATORS_PER_FILTER; corr ++) {
	  for (int chan = 0; chan < NR_CHANNELS_PER_CORRELATOR; chan ++) {
	    (*outputs[corr])[chan][stat][timeSlot][pol] = fftOutData[pol][corr * NR_CHANNELS_PER_CORRELATOR + chan + 1];
	  }
	}
      }
    }
  }
}

#else

void WH_PPF::process()
{
  static NSTimer timer("WH_PPF::process()", true), fftTimer("FFT", true);
  static NSTimer inTimer("inTimer", true), convertTimer("convert", true);

  timer.start();

  typedef i16complex inputType[NR_STATIONS][NR_SAMPLES_PER_INTEGRATION][NR_SUB_CHANNELS][NR_POLARIZATIONS];
  typedef fcomplex (*outputType)[NR_CHANNELS_PER_CORRELATOR][NR_STATIONS][NR_SAMPLES_PER_INTEGRATION][NR_POLARIZATIONS];

  inputType *input = (inputType *) static_cast<DH_PPF*>(getDataManager().getInHolder(0))->getBuffer();
  outputType outputs[NR_CORRELATORS_PER_FILTER];
  fcomplex   fftInData[NR_SAMPLES_PER_INTEGRATION][NR_SUB_CHANNELS], fftOutData[NR_SUB_CHANNELS];

  for (int corr = 0; corr < NR_CORRELATORS_PER_FILTER; corr ++) {
    outputs[corr] = (outputType) static_cast<DH_CorrCube*>(getDataManager().getOutHolder(corr))->getBuffer();
  }

  for (int stat = 0; stat < NR_STATIONS; stat ++) {
#if 0
    static fcomplex converted[NR_SAMPLES_PER_INTEGRATION][NR_SUB_CHANNELS][NR_POLARIZATIONS] __attribute__ ((aligned(8)));
    convertTimer.start();
    _convert(&converted[0][0][0], &(*input)[stat][0][0][0], sizeof converted / sizeof(fcomplex));
    convertTimer.stop();
#endif

    for (int pol = 0; pol < NR_POLARIZATIONS; pol ++) {
      inTimer.start();

      for (int chan = 0; chan < NR_SUB_CHANNELS; chan ++) {
#if 0
	for (int timeSlot = 0; timeSlot < NR_SAMPLES_PER_INTEGRATION; timeSlot ++) {
	  fcomplex sample = to_fcomplex((*input)[stat][timeSlot][chan][pol]);
	  fftInData[timeSlot][chan] = itsFIRs[stat][chan][pol].processNextSample(sample,itsWeights[chan]);
	}
#else
	_filter(itsFIRs[stat][chan][pol].itsDelayLine, itsWeights[chan], &(*input)[stat][0][chan][pol], &fftInData[0][chan], NR_SAMPLES_PER_INTEGRATION / 16);
#endif
      }

      inTimer.stop();

      for (int timeSlot = 0; timeSlot < NR_SAMPLES_PER_INTEGRATION; timeSlot ++) {
	fftTimer.start();
	fftw_one(itsFFTWPlan, (fftw_complex *) fftInData[timeSlot], (fftw_complex *) fftOutData);
	fftTimer.stop();

	// Assume that channel 0 is bogus.  We now divide the remaining channels
	// over the correlators.

	for (int corr = 0; corr < NR_CORRELATORS_PER_FILTER; corr ++) {
	  for (int chan = 0; chan < NR_CHANNELS_PER_CORRELATOR; chan ++) {
	    (*outputs[corr])[chan][stat][timeSlot][pol] = fftOutData[corr * NR_CHANNELS_PER_CORRELATOR + chan + 1];
	  }
	}
      }
    }
  }

  timer.stop();
}

#endif


void WH_PPF::postprocess()
{
}


void WH_PPF::dump() const
{
}
