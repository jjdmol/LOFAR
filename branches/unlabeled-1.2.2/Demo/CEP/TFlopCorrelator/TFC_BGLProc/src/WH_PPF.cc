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
#include <TFC_Interface/TFC_Config.h>

#include <Common/Timer.h>

#ifdef HAVE_BGL
#include <hummer_builtin.h>
#endif

#include <fftw.h>
#include <assert.h>
#include <complex.h>

using namespace LOFAR;


FIR::FIR()
{
  for (int tap = 0; tap < NR_TAPS; tap ++) {
    itsDelayLine[tap] = makefcomplex(0.0f, 0.0f);
  }
}


extern "C"
{
  void _fft_16(const fcomplex *in, int in_stride, fcomplex *out, int out_stride);
  void _prefetch(const i16complex *, int nr_samples);
  void _transpose_2(fcomplex *out, const fcomplex *in);
  void _transpose_3(fcomplex *out, const fcomplex *in);
  void _filter(fcomplex delayLine[NR_TAPS], const float weights[NR_TAPS], const i16complex samples[], fcomplex *out, int nr_samples_div_16);
  void _zero_area(fcomplex *ptr, int nr_cache_lines);
}

WH_PPF::WH_PPF(const string& name, const short subBandID):
  WorkHolder(1, NR_CORRELATORS_PER_FILTER, name, "WH_Correlator"),
  itsSubBandID(subBandID)
{
  ACC::APS::ParameterSet myPS("TFlopCorrelator.cfg");

#if 0
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
#endif

  assert(NR_SAMPLES_PER_INTEGRATION % 16 == 0);

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



void WH_PPF::process()
{
#if 0
  static const fcomplex in[16] =
  {
    makefcomplex(1, 2),
    makefcomplex(3, 4),
    makefcomplex(5, 6),
    makefcomplex(7, 8),
    makefcomplex(9, 10),
    makefcomplex(11, 12),
    makefcomplex(13, 14),
    makefcomplex(15, 16),
    makefcomplex(17, 18),
    makefcomplex(19, 20),
    makefcomplex(21, 22),
    makefcomplex(23, 24),
    makefcomplex(25, 26),
    makefcomplex(27, 28),
    makefcomplex(29, 30),
    makefcomplex(31, 32),
  };
  fcomplex out[16];

  _fft_16(in, 8, out, 8);

  for (int i = 0; i < 16; i ++)
    std::cerr << out[i] << '\n';

  exit(0);
#endif

  static NSTimer timer("WH_PPF::process()", true), fftTimer("FFT", true);
  static NSTimer inTimer("inTimer", true), prefetchTimer("prefetch", true);

  timer.start();

  typedef i16complex inputType[NR_STATIONS][NR_SAMPLES_PER_INTEGRATION][NR_SUB_CHANNELS][NR_POLARIZATIONS];

  inputType *input = (inputType *) static_cast<DH_PPF*>(getDataManager().getInHolder(0))->getBuffer();
  DH_CorrCube::BufferType *outputs[NR_CORRELATORS_PER_FILTER];
  static fcomplex fftInData[NR_SAMPLES_PER_INTEGRATION][NR_POLARIZATIONS][NR_SUB_CHANNELS] __attribute__((aligned(32)));
  static fcomplex fftOutData[2][NR_POLARIZATIONS][NR_SUB_CHANNELS] __attribute__((aligned(32)));
  static fcomplex tmp[4][NR_SAMPLES_PER_INTEGRATION] __attribute__((aligned(32)));

  for (int corr = 0; corr < NR_CORRELATORS_PER_FILTER; corr ++) {
    outputs[corr] = (DH_CorrCube::BufferType *) (static_cast<DH_CorrCube*>(getDataManager().getOutHolder(corr))->getBuffer());
    assert((ptrdiff_t) outputs[corr] % 32 == 0);
  }

  for (int stat = 0; stat < NR_STATIONS; stat ++) {
    inTimer.start();

    for (int pol = 0; pol < NR_POLARIZATIONS; pol ++) {
      for (int chan = 0; chan < NR_SUB_CHANNELS; chan ++) {
#if 0
	for (int time = 0; time < NR_SAMPLES_PER_INTEGRATION; time ++) {
	  fcomplex sample = to_fcomplex((*input)[stat][time][chan][pol]);
	  fftInData[time][pol][chan] = itsFIRs[stat][chan][pol].processNextSample(sample,itsWeights[chan]);
	}
#else
	_filter(itsFIRs[stat][chan][pol].itsDelayLine, itsWeights[chan], &(*input)[stat][0][chan][pol], tmp[chan & 3], NR_SAMPLES_PER_INTEGRATION / NR_TAPS);

	if ((chan & 3) == 3) {
	  _transpose_2(&fftInData[0][pol][chan - 3], &tmp[0][0]);
	}
#endif
      }
    }

    inTimer.stop();

    for (int time = 0; time < NR_SAMPLES_PER_INTEGRATION; time += 2) {
      fftTimer.start();

      //_zero_area(fftOutData[0][0], sizeof fftOutData / 32);

      for (int t = 0; t < 2; t ++) {
	for (int pol = 0; pol < NR_POLARIZATIONS; pol ++) {
	  fftw_one(itsFFTWPlan, (fftw_complex *) fftInData[time + t][pol], (fftw_complex *) fftOutData[t][pol]);
	}
      }

      fftTimer.stop();

      for (int corr = 0; corr < NR_CORRELATORS_PER_FILTER; corr ++) {
#if 0
	for (int chan = 0; chan < NR_CHANNELS_PER_CORRELATOR; chan ++) {
	  for (int t = 0; t < 2; t ++) {
	    for (int pol = 0; pol < NR_POLARIZATIONS; pol ++) {
	      (*outputs[corr])[chan][stat][time + t][pol] = fftOutData[t][pol][corr * NR_CHANNELS_PER_CORRELATOR + chan];
	    }
	  }
	}
#else
	_transpose_3(&(*outputs[corr])[0][stat][time][0], &fftOutData[0][0][corr * NR_CHANNELS_PER_CORRELATOR]);
#endif
      }
    }
  }

  timer.stop();
}


void WH_PPF::postprocess()
{
}


void WH_PPF::dump() const
{
}
