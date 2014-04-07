//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

#include <Common/Timer.h>

#include <PostCorrelationFlagger.h>
#include <Correlator.h>

namespace LOFAR {
namespace RTCP {

#define MAX_ITERS 5

static NSTimer RFIStatsTimer("RFI post statistics calculations", true, true);
static NSTimer thresholdingFlaggerTimer("RFI post Thresholding flagger", true, true);
static NSTimer detectBrokenStationsTimer("RFI post DetectBrokenStations", true, true);

// CorrelatedData samples: [nrBaselines][nrChannels][NR_POLARIZATIONS][NR_POLARIZATIONS]

// We have the data for one second, all frequencies in a subband.
// If one of the polarizations exceeds the threshold, flag them all.
// All baselines are flagged completely independently.
// Autocorrelations are ignored, and are not flagged!

// TODO: some data could already be flagged, take that into account! --Rob

PostCorrelationFlagger::PostCorrelationFlagger(const unsigned nrStations, const unsigned nrChannels, const float cutoffThreshold, float baseSentitivity, float firstThreshold, FlaggerType flaggerType,
    FlaggerStatisticsType flaggerStatisticsType) :
  Flagger(nrStations, nrChannels, cutoffThreshold, baseSentitivity, firstThreshold, flaggerType, flaggerStatisticsType), itsNrBaselines((nrStations * (nrStations + 1) / 2)) {
  itsPowers.resize(itsNrChannels);
  itsFlags.resize(itsNrChannels);
  itsSummedBaselinePowers.resize(itsNrBaselines);
  itsSummedStationPowers.resize(itsNrStations);
}

void PostCorrelationFlagger::flag(CorrelatedData* correlatedData) {
  float mean;
  float stdDev;
  float median;

  wipeSums();

  for (unsigned baseline = 0; baseline < itsNrBaselines; baseline++) {
    if (Correlator::baselineIsAutoCorrelation(baseline)) {
      LOG_DEBUG_STR(" baseline " << baseline << " is an autocorrelation, skipping");
      continue;
    }

    wipeFlags();
    for (unsigned pol1 = 0; pol1 < NR_POLARIZATIONS; pol1++) {
      for (unsigned pol2 = 0; pol2 < NR_POLARIZATIONS; pol2++) {
        calculatePowers(baseline, pol1, pol2, correlatedData);

        switch (itsFlaggerStatisticsType) {
        case FLAGGER_STATISTICS_NORMAL:
          calculateStatistics(itsPowers.data(), itsPowers.size(), mean, median, stdDev);
          break;
        case FLAGGER_STATISTICS_WINSORIZED:
          calculateWinsorizedStatistics(itsPowers.data(), itsPowers.size(), mean, median, stdDev);
          break;
        default:
          LOG_INFO_STR("ERROR, illegal FlaggerStatisticsType. Skipping online flagger.");
          return;
        }

        LOG_DEBUG_STR("RFI post global stats baseline " << baseline << ": mean = " << mean << ", median = " << median << ", stddev = " << stdDev);

        switch (itsFlaggerType) {
        case FLAGGER_THRESHOLD:
          thresholdingFlagger(mean, stdDev, median);
          break;
        case FLAGGER_SUM_THRESHOLD:
          sumThresholdFlagger(mean, stdDev, median);
          break;
        default:
          LOG_INFO_STR("ERROR, illegal FlaggerType. Skipping online flagger.");
          return;
        }
        calculateSummedbaselinePowers(baseline);
      }
    }
    applyFlags(baseline, correlatedData);
  }
}

void PostCorrelationFlagger::calculateSummedbaselinePowers(unsigned baseline) {
  for (unsigned channel = 0; channel < itsNrChannels; channel++) {
    if (!itsFlags[channel]) {
      itsSummedBaselinePowers[baseline] += itsPowers[channel];
    }
  }
}

void PostCorrelationFlagger::thresholdingFlagger(const float mean, const float stdDev, const float median) {
  float threshold = median + itsCutoffThreshold * stdDev;

  for (unsigned channel = 0; channel < itsNrChannels; channel++) {
    if (itsPowers[channel] > threshold) {
      itsFlags[channel] = true;
    }
  }
}

void PostCorrelationFlagger::sumThresholdFlagger(const float mean, const float stdDev, const float median) {
  float factor;
  if (stdDev == 0.0f) {
    factor = itsBaseSensitivity;
  } else {
    factor = stdDev * itsBaseSensitivity;
  }

  unsigned window = 1;
  for (unsigned iter = 1; iter <= MAX_ITERS; iter++) {
    float thresholdI = calcThresholdI(itsFirstThreshold, iter, 1.5f) * factor;
    sumThreshold(window, thresholdI);
    window *= 2;
  }
}

// TODO shouldn't threshold depend on median??? --Rob
void PostCorrelationFlagger::sumThreshold(unsigned window, float threshold) {
  for (unsigned base = 0; base + window < itsNrChannels; base++) {
    float sum = 0.0f;

    for (unsigned pos = base; pos < base + window; pos++) {
      if (itsFlags[pos]) { // If it was flagged in a previous iteration, replace sample with current threshold
        sum += threshold;
        itsPowers[pos] = threshold; // for stats calc
      } else {
        sum += itsPowers[pos];
      }
    }

    if (sum >= window * threshold) {
      // flag all samples in the sequence!
      for (unsigned pos = base; pos < base + window; pos++) {
        itsFlags[pos] = true;
        itsPowers[pos] = threshold; // for stats calc
      }
    }
  }
}

void PostCorrelationFlagger::detectBrokenStations() {
  detectBrokenStationsTimer.start();

  // Sum all baselines that involve a station (both horizontally and vertically).
  float total = 0.0f;

  for (unsigned station = 0; station < itsNrStations; station++) {
    float sum = 0.0f;
    for (unsigned stationH = station+1; stationH < itsNrStations; stationH++) { // do not count autocorrelation
      unsigned baseline = Correlator::baseline(station, stationH);
      sum += itsSummedBaselinePowers[baseline];
    }
    for (unsigned stationV = 0; stationV < station; stationV++) {
      unsigned baseline = Correlator::baseline(stationV, station);
      sum += itsSummedBaselinePowers[baseline];
    }

    itsSummedStationPowers[station] = sum;
    total += sum;
  }

  float sum;
  float stdDev;
  float mean = total / itsNrStations;
  calculateStdDevAndSum(itsSummedStationPowers.data(), itsSummedStationPowers.size(), mean, stdDev, sum);
  float median = calculateMedian(itsSummedStationPowers.data(), itsSummedStationPowers.size());
  float threshold = mean + itsCutoffThreshold * stdDev;

  LOG_DEBUG_STR("RFI post detectBrokenStations: mean = " << mean << ", median = " << median << " stdDev = " << stdDev << ", threshold = " << threshold);

  for (unsigned station = 0; station < itsNrStations; station++) {
    LOG_INFO_STR("RFI post detectBrokenStations: station " << station << " total summed power = " << itsSummedStationPowers[station]);
    if (itsSummedStationPowers[station] > threshold) {
      LOG_INFO_STR(
          "RFI post detectBrokenStations: WARNING, station " << station << " seems to be corrupted, total summed power = " << itsSummedStationPowers[station]);
    }
  }

  detectBrokenStationsTimer.stop();
}

void PostCorrelationFlagger::wipeSums() {
  for (unsigned baseline = 0; baseline < itsNrBaselines; baseline++) {
    itsSummedBaselinePowers[baseline] = 0.0f;
  }

  for (unsigned station = 0; station < itsNrStations; station++) {
    itsSummedStationPowers[station] = 0.0f;
  }
}

void PostCorrelationFlagger::wipeFlags() {
  for (unsigned channel = 0; channel < itsNrChannels; channel++) {
    itsFlags[channel] = false;
  }
}

void PostCorrelationFlagger::applyFlags(unsigned baseline, CorrelatedData* correlatedData) {
  for (unsigned channel = 0; channel < itsNrChannels; channel++) {
    if (itsFlags[channel]) {
#ifdef LOFAR_STMAN_V2
      correlatedData->nrValidSamplesV2[baseline] = 0;
#else
      correlatedData->nrValidSamples[baseline][channel] = 0;
#endif
      // TODO: currently, we can only flag all channels at once! This is a limitation in CorrelatedData.
      //	    correlatedData->flags[station].include(time, time);
    }
  }
}

void PostCorrelationFlagger::calculatePowers(unsigned baseline, unsigned pol1, unsigned pol2, CorrelatedData* correlatedData) {
  for (unsigned channel = 0; channel < itsNrChannels; channel++) {
    fcomplex sample = correlatedData->visibilities[baseline][channel][pol1][pol2];
    float power = real(sample) * real(sample) + imag(sample) * imag(sample);
    itsPowers[channel] = power;
  }
}

} // namespace RTCP
} // namespace LOFAR
