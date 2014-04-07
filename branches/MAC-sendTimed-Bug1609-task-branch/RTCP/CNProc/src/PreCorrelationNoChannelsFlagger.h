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

// integrate in time untill we have itsFFTSize elements.
// Flag on that in time direction.
// Next, do FFT, flag in frequency direction, replace samples with median, inverseFFT
class PreCorrelationNoChannelsFlagger : public Flagger {
  public:
  PreCorrelationNoChannelsFlagger(const Parset& parset, const unsigned nrStations, const unsigned nrSubbands, const unsigned nrChannels, const unsigned nrSamplesPerIntegration, float cutoffThreshold = 7.0f);

  void flag(FilteredData* filteredData, unsigned currentSubband);

  ~PreCorrelationNoChannelsFlagger();

  private:

  static const unsigned itsFFTSize = 256;

  const unsigned itsNrSamplesPerIntegration;
  unsigned itsIntegrationFactor; 

  void integrateAndCalculatePowers(unsigned station, unsigned pol, FilteredData* filteredData);
  void initFlagsTime(unsigned station, FilteredData* filteredData);
  void applyFlagsTime(unsigned station, FilteredData* filteredData);
  void applyFlagsFrequency(unsigned station, FilteredData* filteredData);
  fcomplex computeMedianSample(unsigned station, FilteredData* filteredData);

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

};

} // namespace RTCP
} // namespace LOFAR

#endif // LOFAR_CNPROC_PRE_CORRELATION_NO_CHANNELS_FLAGGER_H
