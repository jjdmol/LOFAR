//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

#include <Common/Timer.h>

#include <PreCorrelationNoChannelsFlagger.h>

/*
  WE CANNOT INTEGRATE BY ADDING SAMPLES, AND THEN TAKING POWER. WE HAVE TO CALCULATE THE POWER FOR EACH SAMPLE, AND ADD THE POWERS.
  Interleaved adding does not work: integrate samples modulo itsFFTSize.
  The fast and slow version do not have the same result. The slow version does an FFT per block, takes the powers of the result, and integrates that.
  The fast version integrates all samples to the block size, then does only 1 fft, and takes the power.
  The sum of the powers is not the same as the sum of the samples, en then taking the power. The slow version works much better...

  history is kept per subband, as we can get different subbands over time on this compute node.
  Always flag poth polarizations as a unit.

  FFT followed by an inverse FFT multiplies all samples by N. Thus, we have to divide by N after we are done.


  First, we flag in the time direction, while integrating to imrove signal-to-noise.
  This was empirically verified to work much better than flagging on the raw data.
  We can then replace flagged samples with 0s or mean/ median.

  Two options for frequency flagging:

  - integrate until we have FFTSize samples, so we improve signal-to-noise
  - do FFT
  - flag, keep frequency ranges that are flagged.
  - move over the raw data at full time resolution; FFT, replace with 0, mean or median; inverse FFT
 
  or

  - do not integrate, but move over raw data in full time resolution
  - do fft
  - flag on this data only
  - replace with 0, mean or median
  - inverse fft

  In all these cases replacing with median would be best, but expensive.
  Also, which median? compute it once on the raw data for all samples, or once per fft?

  Option 1 is cheaper, since we flag only once, instead of integrationFactor times.
  It may also be better due to the improved signal-to-noise ratio.
*/

namespace LOFAR {
namespace RTCP {

PreCorrelationNoChannelsFlagger::PreCorrelationNoChannelsFlagger(const Parset& parset, const unsigned nrStations, const unsigned nrSubbands, const unsigned nrChannels, 
					     const unsigned nrSamplesPerIntegration, const float cutoffThreshold)
:
  Flagger(parset, nrStations, nrSubbands, nrChannels, cutoffThreshold, /*baseSentitivity*/ 0.6f, // 0.6 was emperically found to be a good setting for LOFAR
	  getFlaggerStatisticsType(parset.onlinePreCorrelationFlaggingStatisticsType(getFlaggerStatisticsTypeString(FLAGGER_STATISTICS_WINSORIZED)))),
  itsNrSamplesPerIntegration(nrSamplesPerIntegration)
{
  assert(itsNrSamplesPerIntegration % itsFFTSize == 0);
  assert(nrChannels == 1);

  itsIntegrationFactor = itsNrSamplesPerIntegration / itsFFTSize;

  itsSamples.resize(itsFFTSize);
  itsPowers.resize(itsFFTSize);
  itsFlagsTime.resize(itsFFTSize);
  itsFlagsFrequency.resize(itsFFTSize);
  itsFFTBuffer.resize(itsFFTSize);

#if USE_HISTORY_FLAGGER
  itsHistory.resize(boost::extents[itsNrStations][nrSubbands][NR_POLARIZATIONS]);
#endif

  initFFT();
}


void PreCorrelationNoChannelsFlagger::initFFT()
{
#if defined HAVE_FFTW3
  itsFFTWforwardPlan =  fftwf_plan_dft_1d(itsFFTSize, (fftwf_complex *) &itsSamples[0], (fftwf_complex *) &itsFFTBuffer[0], FFTW_FORWARD, FFTW_MEASURE);
  itsFFTWbackwardPlan = fftwf_plan_dft_1d(itsFFTSize, (fftwf_complex *) &itsFFTBuffer[0], (fftwf_complex *) &itsSamples[0], FFTW_FORWARD, FFTW_MEASURE);
#elif defined HAVE_FFTW2
// @@@ TODO
//  itsFFTWforwardPlan  = fftw_create_plan_specific(itsFFTsize, FFTW_FORWARD,  FFTW_ESTIMATE | FFTW_USE_WISDOM, (fftw_complex *) &itsSamples[0], 2, (fftw_complex *) &itsFFTBuffer[0], 1);
//  itsFFTWbackwardPlan = fftw_create_plan_specific(itsFFTsize, FFTW_BACKWARD, FFTW_ESTIMATE | FFTW_USE_WISDOM, (fftw_complex *) &itsFFTBuffer[0], 1, (fftw_complex *) &itsSamples[0], 2);
#endif
}


void PreCorrelationNoChannelsFlagger::forwardFFT()
{
#if defined HAVE_FFTW3
  fftwf_execute(itsFFTWforwardPlan);
#elif defined HAVE_FFTW2
//  fftw(itsFFTWforwardPlan, 2, (fftw_complex *) data, 2, 1, (fftw_complex *) &itsFFTedBuffer[0][0], 1, itsFFTsize);
#endif
}


void PreCorrelationNoChannelsFlagger::backwardFFT()
{
#if defined HAVE_FFTW3
  fftwf_execute(itsFFTWbackwardPlan);
#elif defined HAVE_FFTW2
//  fftw(itsFFTWbackwardPlan, 2, (fftw_complex *) &itsFFTedBuffer[0][0], 1, itsFFTsize, (fftw_complex *) data, 2, 1);
#endif
}


void PreCorrelationNoChannelsFlagger::flag(FilteredData* filteredData, unsigned currentSubband)
{
  NSTimer flaggerTimer("RFI noChannels flagger total", true, true);
  NSTimer flaggerTimeTimer("RFI noChannels time flagger", true, true);
  NSTimer flaggerFrequencyTimer("RFI noChannels frequency flagger", true, true);

  flaggerTimer.start();

  for(unsigned station = 0; station < itsNrStations; station++) {
    initFlagsTime(station, filteredData); // copy flags to my local format
    filteredData->resetFlags();           // Wipe original flags

    // init frequency flags
    for (unsigned i = 0; i < itsFFTSize; i++) {
      itsFlagsFrequency[i] = false;
    }

    for (unsigned pol = 0; pol < NR_POLARIZATIONS; pol++) {
#if FLAG_IN_TIME_DIRECTION
      flaggerTimeTimer.start();
      calcIntegratedPowers(station, pol, filteredData, currentSubband);
      sumThresholdFlagger1D(itsPowers, itsFlagsTime, itsBaseSensitivity); // flag in time direction
      flaggerTimeTimer.stop();
#endif
#if FLAG_IN_FREQUENCY_DIRECTION
      flaggerFrequencyTimer.start();
      calcIntegratedChannelPowers(station, pol, filteredData, currentSubband);

#if USE_HISTORY_FLAGGER
      sumThresholdFlagger1DWithHistory(itsPowers, itsFlagsFrequency, itsBaseSensitivity, itsHistory[station][currentSubband][pol]);
#else
      sumThresholdFlagger1D(itsPowers, itsFlagsFrequency, itsBaseSensitivity); // flag in freq direction
#endif

      flaggerFrequencyTimer.stop();

#if 0
  if(station == 0 && pol == 0) {
    cout << "INTEGRATED DATA AA AA AA for subband " << currentSubband << " ";
    for (unsigned i = 0; i < itsFFTSize; i++) {
      float val = itsFlagsFrequency[i] ? 0.0f : itsPowers[i];
      cout << " " << val;
    }
    cout << endl;
  }
#endif // PRINT

#endif // FLAG_IN_FREQUENCY_DIRECTION
    }

    flaggerTimeTimer.start();
    applyFlagsTime(station, filteredData); // copy flags from my original format into FilteredData again.
    flaggerTimeTimer.stop();

    flaggerFrequencyTimer.start();
    applyFlagsFrequency(station, filteredData); // do forward FFT; fix samples; backward FFT on the original samples in full resolution
    flaggerFrequencyTimer.stop();
  }

  flaggerTimer.stop();
}


void PreCorrelationNoChannelsFlagger::calcIntegratedPowers(unsigned station, unsigned pol, FilteredData* filteredData, unsigned currentSubband)
{
  (void) currentSubband; // avoids compiler warning

  memset(itsPowers.data(), 0, itsFFTSize * sizeof(float));
 
  for(unsigned t=0; t<itsNrSamplesPerIntegration; t++) {
    fcomplex sample = filteredData->samples[0][station][t][pol];
    itsPowers[t/itsIntegrationFactor] += real(sample) * real(sample) + imag(sample) * imag(sample);
  }
}


void PreCorrelationNoChannelsFlagger::calcIntegratedChannelPowers(unsigned station, unsigned pol, FilteredData* filteredData, unsigned currentSubband)
{
  memset(itsPowers.data(), 0, itsFFTSize * sizeof(float));

  for(unsigned block=0; block<itsIntegrationFactor; block++) {
    unsigned startIndex = block * itsFFTSize;

    for(unsigned minorTime=0; minorTime<itsFFTSize; minorTime++) {
      itsSamples[minorTime] = filteredData->samples[0][station][startIndex + minorTime][pol];
    } 

    forwardFFT();

    for (unsigned i = 0; i < itsFFTSize; i++) { // compute powers from FFT-ed data
      fcomplex sample = itsFFTBuffer[i];
      float power = real(sample) * real(sample) + imag(sample) * imag(sample);
      itsPowers[i] += power;
    }
  }

#if 0
  if(station == 0 && pol == 0) {
    cout << "INTEGRATED DATA AA AA AA for subband " << currentSubband << " ";
    for (unsigned i = 0; i < itsFFTSize; i++) {
      cout << " " << itsPowers[i];
    }
    cout << endl;
  }
#else
  (void) currentSubband; // avoids compiler warning
#endif
}


void PreCorrelationNoChannelsFlagger::initFlagsTime(unsigned station, FilteredData* filteredData)
{
  for (unsigned i = 0; i < itsFFTSize; i++) {
    itsFlagsTime[i] = false;
  }

  // Use the original flags to initialize the flags.
  // This could be done much faster by just iterating over the windows in the sparse flags set. // TODO
  for (unsigned time = 0; time < itsNrSamplesPerIntegration; time++) {
    if(filteredData->flags[0][station].test(time)) {
      itsFlagsTime[time/itsIntegrationFactor] = true;
    }
  }
}


void PreCorrelationNoChannelsFlagger::applyFlagsTime(unsigned station, FilteredData* filteredData)
{
  const fcomplex zero = makefcomplex(0, 0);

  for (unsigned i = 0; i < itsFFTSize; i++) {
    if(itsFlagsTime[i]) {
      unsigned startIndex = i * itsIntegrationFactor;
      filteredData->flags[0][station].include(startIndex, startIndex+itsIntegrationFactor);
      memset(&filteredData->samples[0][station][startIndex][0], 0, itsIntegrationFactor * NR_POLARIZATIONS * sizeof(fcomplex));
    }
  }
}


// Do forward FFT; fix samples; backward FFT on the original samples in full resolution. Flags are already set in itsFlagsFrequency.
// FFT followed by an inverse FFT multiplies all samples by N. Thus, we have to divide by N after we are done.
void PreCorrelationNoChannelsFlagger::applyFlagsFrequency(unsigned station, FilteredData* filteredData)
{
  unsigned count = 0;
  for(unsigned minorTime=0; minorTime < itsFFTSize; minorTime++) {
    if(itsFlagsFrequency[minorTime]) {
      count++;
    }
  }
//    cerr << "samples flagged in frequency: " << count << endl;

  if(count == 0) {
    return;
  }

  const fcomplex zero = makefcomplex(0, 0);

  for (unsigned time = 0; time < itsIntegrationFactor; time++) {
    unsigned startIndex = time * itsFFTSize;
    for (unsigned pol = 0; pol < NR_POLARIZATIONS; pol++) {
      for(unsigned minorTime=0; minorTime < itsFFTSize; minorTime++) {
	itsSamples[minorTime] = filteredData->samples[0][station][startIndex+minorTime][pol];
      }
      forwardFFT();
      for(unsigned minorTime=0; minorTime < itsFFTSize; minorTime++) {
	if(itsFlagsFrequency[minorTime]) {
	  itsFFTBuffer[minorTime] = zero;
	}
      }
      backwardFFT();
      for(unsigned minorTime=0; minorTime < itsFFTSize; minorTime++) {
	filteredData->samples[0][station][startIndex+minorTime][pol] = makefcomplex(real(itsSamples[minorTime]) / itsFFTSize, imag(itsSamples[minorTime]) / itsFFTSize);
      }
    }
  }
}


PreCorrelationNoChannelsFlagger::~PreCorrelationNoChannelsFlagger()
{
#if defined HAVE_FFTW3
  if(itsFFTWforwardPlan != 0) {
    fftwf_destroy_plan(itsFFTWforwardPlan);
  }
  if(itsFFTWbackwardPlan != 0) {
    fftwf_destroy_plan(itsFFTWbackwardPlan);
  }
#else
  if(itsFFTWforwardPlan != 0) {
    fftw_destroy_plan(itsFFTWforwardPlan);
  }
  if(itsFFTWbackwardPlan != 0) {
    fftw_destroy_plan(itsFFTWbackwardPlan);
  }
#endif
}

} // namespace RTCP
} // namespace LOFAR
