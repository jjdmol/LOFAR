//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

#include <Common/Timer.h>

#include <Correlator.h>
#include <PostCorrelationFlagger.h>

namespace LOFAR {
namespace RTCP {

#define MAX_ITERS 5

static NSTimer RFIStatsTimer("RFI post statistics calculations", true, true);
static NSTimer detectBrokenStationsTimer("RFI post DetectBrokenStations", true, true);

// CorrelatedData samples: [nrBaselines][nrChannels][NR_POLARIZATIONS][NR_POLARIZATIONS]

// We have the data for one second, all frequencies in a subband.
// If one of the polarizations exceeds the threshold, flag them all.
// All baselines are flagged completely independently.
// Autocorrelations are ignored, and are not flagged!

// TODO: some data could already be flagged, take that into account! --Rob
// TODO: if detectBrokenStations is not enabled, we do't have to wipe/calc summedbaselinePowers

PostCorrelationFlagger::PostCorrelationFlagger(const Parset& parset, const unsigned nrStations, const unsigned nrChannels, const float cutoffThreshold, float baseSentitivity, float firstThreshold) :
    Flagger(parset, nrStations, nrChannels, cutoffThreshold, baseSentitivity, firstThreshold, 
	    getFlaggerType(parset.onlinePostCorrelationFlaggingType(getFlaggerTypeString(FLAGGER_SMOOTHED_SUM_THRESHOLD_WITH_HISTORY))), 
	    getFlaggerStatisticsType(parset.onlinePostCorrelationFlaggingStatisticsType(getFlaggerStatisticsTypeString(FLAGGER_STATISTICS_WINSORIZED)))), 
    itsNrBaselines((nrStations * (nrStations + 1) / 2)) {

  itsPowers.resize(itsNrChannels);
  itsSmoothedPowers.resize(itsNrChannels);
  itsPowerDiffs.resize(nrChannels);
  itsFlags.resize(itsNrChannels);
  itsSummedBaselinePowers.resize(itsNrBaselines);
  itsSummedStationPowers.resize(itsNrStations);
  itsHistory.resize(boost::extents[NR_POLARIZATIONS][NR_POLARIZATIONS]);

  LOG_DEBUG_STR("post correlation flagging type = " << getFlaggerTypeString()
		<< ", statistics type = " << getFlaggerStatisticsTypeString());
}

void PostCorrelationFlagger::flag(CorrelatedData* correlatedData) {
  float mean, stdDev, median;

  NSTimer flaggerTimer("RFI post flagger", true, true);
  flaggerTimer.start();

  wipeSums();

  for (unsigned baseline = 0; baseline < itsNrBaselines; baseline++) {
    if (Correlator::baselineIsAutoCorrelation(baseline)) {
//      LOG_DEBUG_STR(" baseline " << baseline << " is an autocorrelation, skipping");
      continue;
    }

    wipeFlags();
    for (unsigned pol1 = 0; pol1 < NR_POLARIZATIONS; pol1++) {
      for (unsigned pol2 = 0; pol2 < NR_POLARIZATIONS; pol2++) {
        calculatePowers(baseline, pol1, pol2, correlatedData);

	RFIStatsTimer.start();
	calculateStatistics(itsPowers.data(), itsPowers.size(), mean, median, stdDev);
	RFIStatsTimer.stop();

//        LOG_DEBUG_STR("RFI post global stats baseline " << baseline << ": mean = " << mean << ", median = " << median << ", stddev = " << stdDev);

        switch (itsFlaggerType) {
        case FLAGGER_THRESHOLD:
	  thresholdingFlagger(itsPowers, itsFlags, mean, stdDev, median);
          break;
        case FLAGGER_SUM_THRESHOLD:
          sumThresholdFlagger(itsPowers, itsFlags, itsBaseSensitivity, mean, stdDev, median);
          break;
	case FLAGGER_SMOOTHED_SUM_THRESHOLD:
          sumThresholdFlaggerSmoothed(itsPowers, itsFlags, mean, stdDev, median);
	  break;
	case FLAGGER_SMOOTHED_SUM_THRESHOLD_WITH_HISTORY:
		sumThresholdFlaggerSmoothedWithHistory(itsPowers, itsFlags, pol1, pol2, mean, stdDev, median);
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
  flaggerTimer.stop();
  cout << flaggerTimer << std::endl;
}

void PostCorrelationFlagger::calculateSummedbaselinePowers(unsigned baseline) {
  for (unsigned channel = 0; channel < itsNrChannels; channel++) {
    if (!itsFlags[channel]) {
      itsSummedBaselinePowers[baseline] += itsPowers[channel];
    }
  }
}

void PostCorrelationFlagger::thresholdingFlagger(std::vector<float>& powers, std::vector<bool>& flags, const float mean, const float stdDev, const float median) {
  float threshold = median + itsCutoffThreshold * stdDev;

  for (unsigned channel = 0; channel < itsNrChannels; channel++) {
    if (powers[channel] > threshold) {
      flags[channel] = true;
    }
  }
}

void PostCorrelationFlagger::sumThresholdFlagger(std::vector<float>& powers, std::vector<bool>& flags, const float sensitivity, const float mean, const float stdDev, const float median) {
  float factor;
  if (stdDev == 0.0f) {
    factor = sensitivity;
  } else {
    factor = stdDev * sensitivity;
  }

  unsigned window = 1;
  for (unsigned iter = 1; iter <= MAX_ITERS; iter++) {
    float thresholdI = calcThresholdI(itsFirstThreshold, iter, 1.5f) * factor;
    sumThreshold(powers, flags, window, thresholdI);
    window *= 2;
  }
}


void PostCorrelationFlagger::sumThresholdFlaggerSmoothed(std::vector<float>& powers, std::vector<bool>& flags, const float mean, const float stdDev, const float median) {
  // first do an insensitive sumthreshold
  sumThresholdFlagger(powers, flags, 1.0f, mean, stdDev, median); // sets flags, and replaces flagged samples with threshold
	
  // smooth
  oneDimensionalGausConvolution(powers.data(), powers.size(), itsSmoothedPowers.data(), 0.5f); // last param is sigma, height of the gaussian curve

  // calculate difference
  for (unsigned int i = 0; i < itsNrChannels; i++) {
    itsPowerDiffs[i] = itsPowers[i] - itsSmoothedPowers[i];
  }
  
  // flag based on difference
  float diffMean, diffStdDev, diffMedian;
  RFIStatsTimer.start();
  calculateStatistics(itsPowerDiffs.data(), itsPowerDiffs.size(), diffMean, diffMedian, diffStdDev);
  RFIStatsTimer.stop();
  sumThresholdFlagger(itsPowerDiffs, flags, 1.0f, diffMean, diffStdDev, diffMedian); // sets additional flags
  
  // and one final, more sensitive pass on the flagged power
  sumThresholdFlagger(powers, flags, 0.8f, mean, stdDev, median); // sets flags, and replaces flagged samples with threshold
}


void PostCorrelationFlagger::sumThresholdFlaggerSmoothedWithHistory(std::vector<float>& powers, std::vector<bool>& flags, const unsigned pol1, const unsigned pol2, const float mean, const float stdDev, const float median) {

  float historyFlaggingThreshold = 7.0;

  sumThresholdFlaggerSmoothed(powers, flags, mean, stdDev, median);
  
  float localMean, localStdDev, localMedian;

  // calculate final statistics (flagged samples were replaced with threshold values)
  RFIStatsTimer.start();
  calculateStatistics(powers.data(), powers.size(), localMean, localMedian, localStdDev); // We calculate stats again, flagged samples were replaced with threshold values
  RFIStatsTimer.stop();

  // add the corrected power statistics to the history
  itsHistory[pol1][pol2].add(median);
  if (itsHistory[pol1][pol2].getSize() >= MIN_HISTORY_SIZE) {
    float meanMedian = itsHistory[pol1][pol2].getMeanMedian();
    float stdDevOfMedians = itsHistory[pol1][pol2].getStdDevOfMedians();
    bool flagSecond = median > (meanMedian + historyFlaggingThreshold * stdDevOfMedians);
    if (flagSecond) {
      for (unsigned i = 0; i < itsNrChannels; i++) {
        flags[i] = true;
      }
    }
  }
}


// TODO shouldn't threshold depend on median??? --Rob
void PostCorrelationFlagger::sumThreshold(std::vector<float>& powers, std::vector<bool>& flags, unsigned window, float threshold) {
  for (unsigned base = 0; base + window < itsNrChannels; base++) {
    float sum = 0.0f;

    for (unsigned pos = base; pos < base + window; pos++) {
      if (flags[pos]) { // If it was flagged in a previous iteration, replace sample with current threshold
        sum += threshold;
        powers[pos] = threshold; // for stats calc
      } else {
        sum += powers[pos];
      }
    }

    if (sum >= window * threshold) {
      // flag all samples in the sequence!
      for (unsigned pos = base; pos < base + window; pos++) {
        flags[pos] = true;
        powers[pos] = threshold; // for stats calc
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
      correlatedData->setNrValidSamples(baseline, channel, 0);
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
