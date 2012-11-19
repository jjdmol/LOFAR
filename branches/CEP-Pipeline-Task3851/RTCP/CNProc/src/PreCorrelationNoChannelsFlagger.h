#ifndef LOFAR_CNPROC_PRE_CORRELATION_NO_CHANNELS_FLAGGER_H
#define LOFAR_CNPROC_PRE_CORRELATION_NO_CHANNELS_FLAGGER_H

#include <Flagger.h>
#include <Interface/FilteredData.h>

#if defined HAVE_FFTW3
#include <fftw3.h>
#elif defined HAVE_FFTW2
#include <fftw.h>
#else
#error Should have FFTW3 or FFTW2 installed
#endif

namespace LOFAR {
namespace RTCP {

#define FLAG_IN_TIME_DIRECTION 1
#define FLAG_IN_FREQUENCY_DIRECTION 1
#define USE_HISTORY_FLAGGER 1

// integrate in time untill we have itsFFTSize elements.
// Flag on that in time direction.
// Next, do FFT, flag in frequency direction, replace samples with median, inverseFFT
class PreCorrelationNoChannelsFlagger : public Flagger {
  public:
  PreCorrelationNoChannelsFlagger(const Parset& parset, const unsigned nrStations, const unsigned nrSubbands, const unsigned nrChannels, 
				  const unsigned nrSamplesPerIntegration, float cutoffThreshold = 7.0f);

  void flag(FilteredData* filteredData, unsigned currentSubband);

  ~PreCorrelationNoChannelsFlagger();

  private:

  static const unsigned itsFFTSize = 256;

  const unsigned itsNrSamplesPerIntegration;
  unsigned itsIntegrationFactor; 

  void calcIntegratedPowers(unsigned station, unsigned pol, FilteredData* filteredData, unsigned currentSubband);
  void calcIntegratedChannelPowers(unsigned station, unsigned pol, FilteredData* filteredData, unsigned currentSubband);

  void initFlagsTime(unsigned station, FilteredData* filteredData);
  void applyFlagsTime(unsigned station, FilteredData* filteredData);
  void applyFlagsFrequency(unsigned station, FilteredData* filteredData);

  void initFFT();
  void forwardFFT();
  void backwardFFT();

  vector<fcomplex> itsSamples; // [itsFFTSize]
  vector<float> itsPowers; // [itsFFTSize]
  vector<bool> itsFlagsTime;   // [itsFFTSize]
  vector<bool> itsFlagsFrequency;   // [itsFFTSize]
  vector<fcomplex> itsFFTBuffer; // [itsFFTSize]

#if defined HAVE_FFTW3
  fftwf_plan itsFFTWforwardPlan, itsFFTWbackwardPlan;
#elif defined HAVE_FFTW2
  fftw_plan  itsFFTWforwardPlan, itsFFTWbackwardPlan;
#endif

#if USE_HISTORY_FLAGGER
  MultiDimArray<FlaggerHistory, 3> itsHistory;   // [nrSations][nrSubbands][NR_POLARIZATIONS]
#endif
};

} // namespace RTCP
} // namespace LOFAR

#endif // LOFAR_CNPROC_PRE_CORRELATION_NO_CHANNELS_FLAGGER_H
