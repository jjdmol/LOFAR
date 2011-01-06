//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

#include <CN_Math.h>
#include <Dedispersion.h>
#include <DedispersionAsm.h>
#include <Common/Timer.h>

#include <algorithm>


#if defined HAVE_FFTW3
#include <fftw3.h>
#include <vector>
#elif defined HAVE_FFTW2
#include <fftw.h>
#else
#error Should have FFTW3 or FFTW2 installed
#endif


namespace LOFAR {
namespace RTCP {


Dedispersion::Dedispersion(CN_Configuration &configuration, const std::vector<unsigned> &subbands)
:
  itsNrChannels(configuration.nrChannelsPerSubband()),
  itsNrSamplesPerIntegration(configuration.nrSamplesPerIntegration()),
  itsFFTsize(configuration.dedispersionFFTsize()),
  itsChannelBandwidth(configuration.sampleRate() / itsNrChannels),
  itsFFTedBuffer(NR_POLARIZATIONS, itsFFTsize)
{
  initChirp(configuration, subbands);
}


DedispersionBeforeBeamForming::DedispersionBeforeBeamForming(CN_Configuration &configuration, FilteredData *filteredData, const std::vector<unsigned> &subbands)
:
  Dedispersion(configuration, subbands),
  itsNrStations(configuration.nrMergedStations())
{
  initFFT(&filteredData->samples[0][0][0][0]);
}


DedispersionAfterBeamForming::DedispersionAfterBeamForming(CN_Configuration &configuration, BeamFormedData *beamFormedData, const std::vector<unsigned> &subbands)
:
  Dedispersion(configuration, subbands),
  itsNrBeams(configuration.flysEye() ? configuration.nrMergedStations() : configuration.nrPencilBeams())
{
  initFFT(&beamFormedData->samples[0][0][0][0]);
}


Dedispersion::~Dedispersion()
{
#if defined HAVE_FFTW3
  fftwf_destroy_plan(itsFFTWforwardPlan);
  fftwf_destroy_plan(itsFFTWbackwardPlan);
#else
  fftw_destroy_plan(itsFFTWforwardPlan);
  fftw_destroy_plan(itsFFTWbackwardPlan);
#endif

  for (unsigned i = 0; i < itsChirp.size(); i ++)
    delete itsChirp[i];
}


void Dedispersion::initFFT(fcomplex *data)
{
#if defined HAVE_FFTW3
  itsFFTWforwardPlan  = fftwf_plan_many_dft(1, (int *) &itsFFTsize, 2, (fftwf_complex *) data, 0, 2, 1, (fftwf_complex *) &itsFFTedBuffer[0][0], 0, 1, itsFFTsize, FFTW_FORWARD, FFTW_MEASURE);
  itsFFTWbackwardPlan = fftwf_plan_many_dft(1, (int *) &itsFFTsize, 2, (fftwf_complex *) &itsFFTedBuffer[0][0], 0, 1, itsFFTsize, (fftwf_complex *) data, 0, 2, 1, FFTW_BACKWARD, FFTW_MEASURE);
#elif defined HAVE_FFTW2
#if defined HAVE_BGP
  fftw_import_wisdom_from_string("(FFTW-2.1.5 (196608 529 1 0 1 2 1 77 0) (98304 529 1 0 1 2 1 99 0) (49152 529 1 0 1 2 1 715 0) (24576 529 1 0 1 2 1 715 0) (12288 529 1 0 1 2 1 715 0) (6144 529 1 0 1 2 1 77 0) (3072 529 1 0 1 2 1 715 0) (1536 529 1 0 1 2 1 187 0) (768 529 1 0 1 2 1 143 0) (384 529 1 0 1 2 1 143 0) (192 529 1 0 1 2 1 143 0) (96 529 1 0 1 2 1 143 0) (48 529 1 0 1 2 1 143 0) (24 529 1 0 1 2 1 143 0) (12 529 1 0 1 2 0 276 0) (6 529 1 0 1 2 0 144 0) (3 529 1 0 1 2 0 78 0) (196608 529 -1 0 2 1 1 704 0) (98304 529 -1 0 2 1 1 704 0) (49152 529 -1 0 2 1 1 704 0) (24576 529 -1 0 2 1 1 704 0) (12288 529 -1 0 2 1 1 704 0) (6144 529 -1 0 2 1 1 704 0) (3072 529 -1 0 2 1 1 132 0) (1536 529 -1 0 2 1 1 132 0) (768 529 -1 0 2 1 1 132 0) (384 529 -1 0 2 1 1 132 0) (192 529 -1 0 2 1 1 352 0) (96 529 -1 0 2 1 1 132 0) (48 529 -1 0 2 1 1 132 0) (24 529 -1 0 2 1 1 132 0) (12 529 -1 0 2 1 0 265 0) (6 529 -1 0 2 1 0 133 0) (3 529 -1 0 2 1 0 67 0) (2 529 -1 0 2 1 0 45 0) (4 529 -1 0 2 1 0 89 0) (8 529 -1 0 2 1 0 177 0) (16 529 -1 0 2 1 0 353 0) (32 529 -1 0 2 1 0 705 0) (64 529 -1 0 2 1 0 1409 0) (128 529 -1 0 2 1 0 2817 0) (256 529 -1 0 2 1 1 352 0) (512 529 -1 0 2 1 1 352 0) (1024 529 -1 0 2 1 1 704 0) (2048 529 -1 0 2 1 1 704 0) (4096 529 -1 0 2 1 1 704 0) (8192 529 -1 0 2 1 1 352 0) (16384 529 -1 0 2 1 1 704 0) (32768 529 -1 0 2 1 1 704 0) (65536 529 -1 0 2 1 1 704 0) (2 529 1 0 1 2 0 56 0) (4 529 1 0 1 2 0 100 0) (8 529 1 0 1 2 0 188 0) (16 529 1 0 1 2 0 364 0) (32 529 1 0 1 2 0 716 0) (64 529 1 0 1 2 0 1420 0) (128 529 1 0 1 2 0 2828 0) (256 529 1 0 1 2 1 715 0) (512 529 1 0 1 2 1 187 0) (1024 529 1 0 1 2 1 715 0) (2048 529 1 0 1 2 1 715 0) (4096 529 1 0 1 2 1 715 0) (8192 529 1 0 1 2 1 1419 0) (16384 529 1 0 1 2 1 99 0) (32768 529 1 0 1 2 1 715 0) (65536 529 1 0 1 2 1 715 0))");
#endif

  itsFFTWforwardPlan  = fftw_create_plan_specific(itsFFTsize, FFTW_FORWARD,  FFTW_ESTIMATE | FFTW_USE_WISDOM, (fftw_complex *) data, 2, (fftw_complex *) &itsFFTedBuffer[0][0], 1);
  itsFFTWbackwardPlan = fftw_create_plan_specific(itsFFTsize, FFTW_BACKWARD, FFTW_ESTIMATE | FFTW_USE_WISDOM, (fftw_complex *) &itsFFTedBuffer[0][0], 1, (fftw_complex *) data, 2);
#endif
}


void Dedispersion::forwardFFT(fcomplex *data)
{
#if defined HAVE_FFTW3
  fftwf_execute_dft(itsFFTWforwardPlan, (fftwf_complex *) data, (fftwf_complex *) &itsFFTedBuffer[0][0]);
#elif defined HAVE_FFTW2
  fftw(itsFFTWforwardPlan, 2, (fftw_complex *) data, 2, 1, (fftw_complex *) &itsFFTedBuffer[0][0], 1, itsFFTsize);
#endif
}


void Dedispersion::backwardFFT(fcomplex *data)
{
#if defined HAVE_FFTW3
  fftwf_execute_dft(itsFFTWbackwardPlan, (fftwf_complex *) &itsFFTedBuffer[0][0], (fftwf_complex *) data);
#elif defined HAVE_FFTW2
  fftw(itsFFTWbackwardPlan, 2, (fftw_complex *) &itsFFTedBuffer[0][0], 1, itsFFTsize, (fftw_complex *) data, 2, 1);
#endif
}


void Dedispersion::initChirp(CN_Configuration &configuration, const std::vector<unsigned> &subbands)
{
  itsChirp.resize(*std::max_element(subbands.begin(), subbands.end()) + 1, 0);
//std::cout << "newcurve linetype solid linethickness 3 marktype none color 0 .7 0 pts" << std::endl;

  for (unsigned subbandIndex = 0; subbandIndex < subbands.size(); subbandIndex ++) {
    unsigned subband	       = subbands[subbandIndex];
    double   subbandFrequency  = configuration.refFreqs()[subbandIndex];
    double   channel0frequency = subbandFrequency - (itsNrChannels * 0.5) * itsChannelBandwidth;
    double   binWidth	       = itsChannelBandwidth / itsFFTsize;
    double   dmConst	       = configuration.dispersionMeasure() * 2 * M_PI / 2.41e-16;

    itsChirp[subband] = new Matrix<fcomplex>(itsNrChannels, itsFFTsize);

    for (unsigned channel = 0; channel < itsNrChannels; channel ++) {
      double channelFrequency = channel0frequency + channel * itsChannelBandwidth;

      for (unsigned n = 0; n < itsFFTsize; n ++) {
	double binFrequency = n * binWidth;

	if (n > itsFFTsize / 2)
	  binFrequency -= itsChannelBandwidth;

	double	 frequencyDiv = binFrequency / channelFrequency;
	double	 frequencyFac = frequencyDiv * frequencyDiv / (channelFrequency + binFrequency);
	dcomplex dfactor      = cosisin(dmConst * frequencyFac);
	fcomplex factor	      = makefcomplex(real(dfactor), -imag(dfactor));
	float	 taper	      = sqrt(1 + pow(binFrequency / (.47 * itsChannelBandwidth), 80));
//if (channel == 0) std::cout << n << ' ' << 1/taper << std::endl;

	(*itsChirp[subband])[channel][n] = factor / (taper * itsFFTsize);
      }
    }
  }
}


void Dedispersion::applyChirp(unsigned subband, unsigned channel)
{
  static NSTimer chirpTimer("chirp timer", true, true);
  const fcomplex *chirp = &(*itsChirp[subband])[channel][0];

  chirpTimer.start();

#if defined HAVE_BGP
  _apply_chirp(&itsFFTedBuffer[0][0], &itsFFTedBuffer[1][0], chirp, itsFFTsize);
#else
  for (unsigned time = 0; time < itsFFTsize; time ++) {
    itsFFTedBuffer[0][time] *= chirp[time];
    itsFFTedBuffer[1][time] *= chirp[time];
  }
#endif

  chirpTimer.stop();
}


void DedispersionBeforeBeamForming::dedisperse(FilteredData *filteredData, unsigned subband)
{
  for (unsigned channel = 0; channel < itsNrChannels; channel ++) {
    for (unsigned station = 0; station < itsNrStations; station ++) {
      for (unsigned block = 0; block < itsNrSamplesPerIntegration; block += itsFFTsize) {
	forwardFFT(&filteredData->samples[channel][station][block][0]);
	applyChirp(subband, channel);
	backwardFFT(&filteredData->samples[channel][station][block][0]);
      }
    }
  }
}


void DedispersionAfterBeamForming::dedisperse(BeamFormedData *beamFormedData, unsigned subband)
{
  for (unsigned beam = 0; beam < itsNrBeams; beam ++) {
    for (unsigned channel = 0; channel < itsNrChannels; channel ++) {
      for (unsigned block = 0; block < itsNrSamplesPerIntegration; block += itsFFTsize) {
	forwardFFT(&beamFormedData->samples[beam][channel][block][0]);
	applyChirp(subband, channel);
	backwardFFT(&beamFormedData->samples[beam][channel][block][0]);
      }
    }
  }
}


} // namespace RTCP
} // namespace LOFAR
