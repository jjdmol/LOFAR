//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

//# Includes
#include <PPF.h>
#include <FFT_Asm.h>
#include <FIR_Asm.h>

#include <CS1_Interface/Align.h>
#include <CS1_Interface/AlignedStdAllocator.h>

#include <Common/DataConvert.h>
#include <Common/Timer.h>

#include <complex>
#include <cmath>
#include <stdexcept>


namespace LOFAR {
namespace CS1 {

#if defined HAVE_MASS

extern "C"
{
  // the return conventions for std::complex<double> and double _Complex differ!
  double _Complex cosisin(double);
}

#else

inline static dcomplex cosisin(double x)
{
  return makedcomplex(cos(x), sin(x));
}

#endif


static NSTimer computeFlagsTimer("PPF::computeFlags()", true);
static NSTimer FIRtimer("PPF::FIRtimer", true);
static NSTimer FFTtimer("PPF::FFT", true);
static NSTimer PPFtimer("PPF::filter()", true);


template <typename SAMPLE_TYPE> PPF<SAMPLE_TYPE>::PPF(unsigned nrStations, unsigned nrChannels, unsigned nrSamplesPerIntegration, double channelBandwidth, bool delayCompensation)
:
  itsNrStations(nrStations),
  itsNrSamplesPerIntegration(nrSamplesPerIntegration),
  itsNrChannels(nrChannels),
  itsChannelBandwidth(channelBandwidth),
  itsDelayCompensation(delayCompensation),

#if defined PPF_C_IMPLEMENTATION
  itsFIRs(boost::extents[nrStations][NR_POLARIZATIONS][nrChannels]),
  itsFFTinData(boost::extents[NR_TAPS - 1 + nrSamplesPerIntegration][NR_POLARIZATIONS][nrChannels])
#else
  itsTmp(boost::extents[4][nrSamplesPerIntegration]),
  itsFFTinData(boost::extents[nrSamplesPerIntegration][NR_POLARIZATIONS][nrChannels + 4]),
  itsFFToutData(boost::extents[2][NR_POLARIZATIONS][nrChannels])
#endif

#if defined HAVE_BGL && !defined PPF_C_IMPLEMENTATION
, mutex(rts_allocate_mutex())
#endif
{
  if (!powerOfTwo(nrChannels))
    throw std::runtime_error("nrChannels must be a power of 2");

  if (nrChannels != 256)
    throw std::runtime_error("nrChannels != 256 not yet implemented");

  for (itsLogNrChannels = 0; 1U << itsLogNrChannels != itsNrChannels; itsLogNrChannels ++)
    ;

  init_fft();
  initConstantTable();
}


template <> void PPF<i4complex>::initConstantTable()
{
  extern fcomplex _FIR_fp_table[16][16];

  static const float map[] = {
     0.5,  1.5,  2.5,  3.5,  4.5,  5.5,  6.5,  7.5, 
    -7.5, -6.5, -5.5, -4.5, -3.5, -2.5, -1.5, -0.5,
  };

  for (unsigned i = 0; i < 16; i ++)
    for (unsigned j = 0; j < 16; j ++)
      _FIR_fp_table[i][j] = makefcomplex(map[j], map[i]);
}


template <> void PPF<i8complex>::initConstantTable()
{
  // This takes up pretty much space (.5 MB)
  extern fcomplex _FIR_fp_table[256][256];

  for (unsigned i = 0; i < 256; i ++)
    for (unsigned j = 0; j < 256; j ++)
      _FIR_fp_table[i][j] = makefcomplex((float) (signed char) i, (float) (signed char) j);
}


template <> void PPF<i16complex>::initConstantTable()
{
#if 0
  extern float _FIR_fp_table[65536];

  for (unsigned i = 0; i < 65536; i ++)
    _FIR_fp_table[i] = (float) byteSwap((signed short) i);
#endif
}


template <typename SAMPLE_TYPE> PPF<SAMPLE_TYPE>::~PPF()
{
  destroy_fft();
}


#if 0 && defined HAVE_BGL

static void FFTtest()
{
  fftw_plan plan = fftw_create_plan(256, FFTW_FORWARD, FFTW_ESTIMATE);

  fcomplex in[256], fout[256], sout[256];

  for (unsigned i = 0; i < 256; i ++)
    in[i] = makefcomplex(2 * i, 2 * i + 1);

  fftw_one(plan, (fftw_complex *) in, (fftw_complex *) fout);

  _fft256(in, sout);

  for (unsigned i = 0; i < 256; i ++) {
    fcomplex diff = fout[i] / sout[i];
    std::cout << i << " (" << real(fout[i]) << ',' << imag(fout[i]) << ") / (" << real(sout[i]) << ',' << imag(sout[i]) << ") = (" << real(diff) << ',' << imag(diff) << ")\n";
  }

  //std::exit(0);
}

#endif


template <typename SAMPLE_TYPE> void PPF<SAMPLE_TYPE>::init_fft()
{
#if defined HAVE_FFTW3
  std::vector<fftwf_complex, AlignedStdAllocator<fftwf_complex, 32> > cbuf1(itsNrChannels), cbuf2(itsNrChannels);
  itsFFTWPlan = fftwf_plan_dft_1d(itsNrChannels, &cbuf1[0], &cbuf2[0], FFTW_FORWARD, FFTW_ESTIMATE);
#elif defined HAVE_FFTW2
  itsFFTWPlan = fftw_create_plan(itsNrChannels, FFTW_FORWARD, FFTW_ESTIMATE);
#endif

  //FFTtest();
}


template <typename SAMPLE_TYPE> void PPF<SAMPLE_TYPE>::destroy_fft()
{
#if defined HAVE_FFTW3
  fftwf_destroy_plan(itsFFTWPlan);
#elif defined HAVE_FFTW2
  fftw_destroy_plan(itsFFTWPlan);
#endif
}


template <typename SAMPLE_TYPE> void PPF<SAMPLE_TYPE>::computeFlags(const TransposedData<SAMPLE_TYPE> *transposedData, FilteredData *filteredData)
{
  computeFlagsTimer.start();

  for (unsigned stat = 0; stat < itsNrStations; stat ++) {
    filteredData->flags[stat].reset();
    SparseSet<unsigned> flags = transposedData->metaData[stat].getFlags();
    const SparseSet<unsigned>::Ranges &ranges = flags.getRanges();

    for (SparseSet<unsigned>::const_iterator it = ranges.begin(); it != ranges.end(); it ++) {
      unsigned begin = std::max(0, (signed) (it->begin >> itsLogNrChannels) - NR_TAPS + 1);
      unsigned end   = std::min(itsNrSamplesPerIntegration, ((it->end - 1) >> itsLogNrChannels) + 1);

      filteredData->flags[stat].include(begin, end);
    }
  }

  computeFlagsTimer.stop();
}


#if defined PPF_C_IMPLEMENTATION

template <typename SAMPLE_TYPE> fcomplex PPF<SAMPLE_TYPE>::phaseShift(unsigned time, unsigned chan, double baseFrequency, double delayAtBegin, double delayAfterEnd) const
{
  double timeInterpolatedDelay = delayAtBegin + ((double) time / itsNrSamplesPerIntegration) * (delayAfterEnd - delayAtBegin);
  double frequency	       = baseFrequency + chan * itsChannelBandwidth;
  double phaseShift	       = timeInterpolatedDelay * frequency;
  double phi		       = -2 * M_PI * phaseShift;

  return makefcomplex(std::cos(phi), std::sin(phi));
}

#else

template <typename SAMPLE_TYPE> void PPF<SAMPLE_TYPE>::computePhaseShifts(struct phase_shift phaseShifts[/*itsNrSamplesPerIntegration*/], double delayAtBegin, double delayAfterEnd, double baseFrequency) const
{
  double   phiBegin = -2 * M_PI * delayAtBegin;
  double   phiEnd   = -2 * M_PI * delayAfterEnd;
  double   deltaPhi = (phiEnd - phiBegin) / itsNrSamplesPerIntegration;
  dcomplex v	    = cosisin(phiBegin * baseFrequency);
  dcomplex dv       = cosisin(phiBegin * itsChannelBandwidth);
  dcomplex vf       = cosisin(deltaPhi * baseFrequency);
  dcomplex dvf      = cosisin(deltaPhi * itsChannelBandwidth);

  for (unsigned time = 0; time < itsNrSamplesPerIntegration; time ++) {
    phaseShifts[time].v0 =  v;  v *=  vf;
    phaseShifts[time].dv = dv; dv *= dvf;
  }
}

#endif


template <typename SAMPLE_TYPE> void PPF<SAMPLE_TYPE>::filter(double centerFrequency, const TransposedData<SAMPLE_TYPE> *transposedData, FilteredData *filteredData)
{
  PPFtimer.start();

  double baseFrequency = centerFrequency - (itsNrChannels / 2) * itsChannelBandwidth;

#if defined HAVE_BGL && !defined PPF_C_IMPLEMENTATION
  // PPF puts a lot of pressure on the memory bus.  Avoid that both cores
  // run simultaneously, since it slows them both.
  _bgl_mutex_lock(mutex);
#endif

  for (unsigned stat = 0; stat < itsNrStations; stat ++) {
    unsigned alignmentShift = transposedData->metaData[stat].alignmentShift;

#if 0
    std::clog << setprecision(15) << "stat " << stat << ", basefreq " << baseFrequency << ": delay from " << delays[stat].delayAtBegin << " to " << delays[stat].delayAfterEnd << " sec" << std::endl;
#endif

#if defined PPF_C_IMPLEMENTATION
    std::vector<fcomplex, AlignedStdAllocator<fcomplex> > fftOutData(itsNrChannels);

    FIRtimer.start();
    for (unsigned pol = 0; pol < NR_POLARIZATIONS; pol ++) {
      for (unsigned chan = 0; chan < itsNrChannels; chan ++) {
	for (unsigned time = 0; time < NR_TAPS - 1 + itsNrSamplesPerIntegration; time ++) {
	  SAMPLE_TYPE tmp = transposedData->samples[stat][itsNrChannels * time + chan + alignmentShift][pol];

#if defined WORDS_BIGENDIAN
	  dataConvert(LittleEndian, &tmp, 1);
#endif
	  fcomplex sample = makefcomplex(real(tmp), imag(tmp));
	  itsFFTinData[time][pol][chan] = itsFIRs[stat][pol][chan].processNextSample(sample, FIR::weights[chan]);
	}
      }
    }
    FIRtimer.stop();

    FFTtimer.start();
    for (unsigned time = 0; time < itsNrSamplesPerIntegration; time ++) {
      for (unsigned pol = 0; pol < NR_POLARIZATIONS; pol ++) {
	if (filteredData->flags[stat].test(time)) {
	  for (unsigned chan = 0; chan < itsNrChannels; chan ++) {
	    filteredData->samples[chan][stat][time][pol] = makefcomplex(0, 0);
	  }
	} else {
#if defined HAVE_FFTW3
	  fftwf_execute_dft(itsFFTWPlan,
			    (fftwf_complex *) itsFFTinData[NR_TAPS - 1 + time][pol].origin(),
			    (fftwf_complex *) (void *) &fftOutData[0]);
#else
	  fftw_one(itsFFTWPlan,
		   (fftw_complex *) itsFFTinData[NR_TAPS - 1 + time][pol].origin(),
		   (fftw_complex *) (void *) &fftOutData[0]);
#endif

	  for (unsigned chan = 0; chan < itsNrChannels; chan ++) {
	    if (itsDelayCompensation) {
	      fftOutData[chan] *= phaseShift(time, chan, baseFrequency, transposedData->metaData[stat].delayAtBegin, transposedData->metaData[stat].delayAfterEnd);
	    }

	    filteredData->samples[chan][stat][time][pol] = fftOutData[chan];
	  }
	}
      }
    }
    FFTtimer.stop();
#else // assembly implementation
    int transpose_stride = sizeof(fcomplex) * (NR_POLARIZATIONS * (itsNrSamplesPerIntegration | 2) * itsNrStations - (itsDelayCompensation ? 3 : 0));

    for (unsigned chan = 0; chan < itsNrChannels; chan += 4) {
      for (unsigned pol = 0; pol < NR_POLARIZATIONS; pol ++) {
#if defined __GNUC__	// work around bug ???
	for (register unsigned ch asm ("r28") = 0; ch < 4; ch ++) {
#else
	for (unsigned ch = 0; ch < 4; ch ++) {
#endif
	  FIRtimer.start();
	  _filter(itsNrChannels,
		  FIR::weights[chan + ch],
		  &transposedData->samples[stat][chan + ch + alignmentShift][pol],
		  itsTmp[ch].origin(),
		  itsNrSamplesPerIntegration / NR_TAPS);
	  FIRtimer.stop();
	}

	_transpose_4x8(&itsFFTinData[0][pol][chan],
		       itsTmp.origin(),
		       itsNrSamplesPerIntegration,
		       sizeof(fcomplex) * itsNrSamplesPerIntegration,
		       sizeof(fcomplex) * NR_POLARIZATIONS * (itsNrChannels + 4));
      }
    }

    struct phase_shift phaseShifts[itsNrSamplesPerIntegration];

    if (itsDelayCompensation) {
      computePhaseShifts(phaseShifts, transposedData->metaData[stat].delayAtBegin, transposedData->metaData[stat].delayAfterEnd, baseFrequency);
    }

    const SparseSet<unsigned>::Ranges &ranges = filteredData->flags[stat].getRanges();
    SparseSet<unsigned>::const_iterator it = ranges.begin();

    for (unsigned time = 0; time < itsNrSamplesPerIntegration; time ++) {
      bool good = it == ranges.end() || time < it->begin || (time == it->end && (++ it, true));

      if (good) {
	FFTtimer.start();
#if 0
	_prefetch(itsFFTinData[time].origin(),
		  sizeof(fcomplex[NR_POLARIZATIONS][itsNrChannels]) / CACHE_LINE_SIZE,
		  CACHE_LINE_SIZE);
#endif

	for (unsigned pol = 0; pol < NR_POLARIZATIONS; pol ++) {
#if 0
	  fftw_one(itsFFTWPlan,
		   (fftw_complex *) itsFFTinData[time][pol].origin(),
		   (fftw_complex *) itsFFToutData[time & 1][pol].origin());
#else
	  _fft256(itsFFTinData[time][pol].origin(),
		  itsFFToutData[time & 1][pol].origin());
#endif
	}
	FFTtimer.stop();
      } else {
	  _memzero(itsFFToutData[time & 1].origin(),
		   itsFFToutData[time & 1].num_elements() * sizeof(fcomplex));
      }

      if (time & 1) {
	if (itsDelayCompensation) {
	  _phase_shift_and_transpose(&filteredData->samples[0][stat][time - 1][0],
				     itsFFToutData.origin(),
				     &phaseShifts[time - 1],
				     transpose_stride,
				     itsNrChannels);
	} else {
	  _transpose_4x8(&filteredData->samples[0][stat][time - 1][0],
			 itsFFToutData.origin(),
			 itsNrChannels,
			 sizeof(fcomplex) * itsNrChannels,
			 transpose_stride);
	}
      }
    }
#endif // PPF_C_IMPLEMENTATION
  }

#if defined HAVE_BGL && !defined PPF_C_IMPLEMENTATION
  _bgl_mutex_unlock(mutex);
#endif

  PPFtimer.stop();
}

template class PPF<i4complex>;
template class PPF<i8complex>;
template class PPF<i16complex>;

} // namespace CS1
} // namespace LOFAR
