//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

#include <Flagger.h>
#include <Common/LofarLogger.h>
#include <Common/Timer.h>

#include <math.h>
#include <algorithm>
#include <string.h>
#include <vector>

#define MAX_ITERS 5

namespace LOFAR {
namespace RTCP {

// TODO float* -> float[]

static NSTimer RFIStatsTimer("RFI post statistics calculations", true, true);

Flagger::Flagger(const Parset& parset, const unsigned nrStations, const unsigned nrChannels, const float cutoffThreshold, float baseSentitivity,
		 FlaggerStatisticsType flaggerStatisticsType) :
  itsParset(parset), itsNrStations(nrStations), itsNrChannels(nrChannels), itsCutoffThreshold(cutoffThreshold), itsBaseSensitivity(baseSentitivity),
  itsFlaggerStatisticsType(flaggerStatisticsType)
{
}

void Flagger::calculateStdDevAndSum(const float* data, const unsigned size, const float mean, float& stdDev, float& sum) {
  stdDev = 0.0f;
  sum = 0.0f;

  for (unsigned i = 0; i < size; i++) {
    sum += data[i];
    float diff = data[i] - mean;
    stdDev += diff * diff;
  }
  stdDev /= size;
  stdDev = sqrtf(stdDev);
}

float Flagger::calculateMedian(const float* data, const unsigned size) {
  // we have to copy the vector, nth_element changes the ordering.
  std::vector<float> copy;
  copy.resize(size);
  memcpy(copy.data(), data, size * sizeof(float));

  // calculate median, expensive, but nth_element is guaranteed to be O(n)
  std::vector<float>::iterator it = copy.begin() + (size / 2);
  std::nth_element(copy.begin(), it, copy.end());
  return *it;
}

void Flagger::calculateStatistics(const float* data, const unsigned size, float& mean, float& median, float& stdDev) {
  RFIStatsTimer.start();

  switch (itsFlaggerStatisticsType) {
  case FLAGGER_STATISTICS_NORMAL:
    calculateNormalStatistics(data, size, mean, median, stdDev);
    break;
  case FLAGGER_STATISTICS_WINSORIZED:
    calculateWinsorizedStatistics(data, size, mean, median, stdDev);
    break;
  default:
    LOG_INFO_STR("ERROR, illegal FlaggerStatisticsType.");
    return;
  }

  RFIStatsTimer.stop();
}

void Flagger::calculateNormalStatistics(const float* data, const unsigned size, float& mean, float& median, float& stdDev) {
  float sum;
  calculateStdDevAndSum(data, size, mean, stdDev, sum);
  median = calculateMedian(data, size);
  mean = sum / size;
}

void Flagger::calculateWinsorizedStatistics(const float* data, const unsigned size, float& mean, float& median, float& stdDev) {
  // we have to copy the vector, we will change the ordering.
  std::vector<float> tmpPower;
  tmpPower.resize(size);
  memcpy(tmpPower.data(), data, size * sizeof(float));
  std::sort(tmpPower.begin(), tmpPower.end());

  unsigned lowIndex = (unsigned) floor(0.1 * size);
  unsigned highIndex = (unsigned) ceil(0.9 * size) - 1;
  float lowValue = tmpPower[lowIndex];
  float highValue = tmpPower[highIndex];

  // Calculate mean
  // This can be done 20% quicker if we exploit the sorted array: if index < lowIndex -> val = lowValue.
  mean = 0.0f;
  unsigned count = 0;
  for (unsigned i = 0; i < size; i++) {
    float value = tmpPower[i];
    if (value < lowValue) {
      mean += lowValue;
    } else if (value > highValue) {
      mean += highValue;
    } else {
      mean += value;
    }
    count++;
  }

  if (count > 0) {
    mean /= count;
  }

  median = tmpPower[size / 2];

  // Calculate variance
  stdDev = 0.0f;
  count = 0;
  for (unsigned i = 0; i < size; i++) {
    float value = tmpPower[i];
    if (value < lowValue) {
      stdDev += (lowValue - mean) * (lowValue - mean);
    } else if (value > highValue) {
      stdDev += (highValue - mean) * (highValue - mean);
    } else {
      stdDev += (value - mean) * (value - mean);
    }
    count++;
  }
  if (count > 0) {
    stdDev = (float) sqrt(1.54 * stdDev / count);
  } else {
    stdDev = 0.0f;
  }
}

float Flagger::evaluateGaussian(const float x, const float sigma) {
    return (float) (1.0 / (sigma * sqrt(2.0 * M_PI)) * exp(-0.5 * x * x / sigma));
}

float Flagger::logBase2(const float x) {
  // log(base 2) x=log(base e) x/log(base e) 2
  return (float) (log(x) / log(2.0));
}

void Flagger::oneDimensionalConvolution(const float* data, const unsigned dataSize, float*dest, const float* kernel, const unsigned kernelSize) {
  for (unsigned i = 0; i < dataSize; ++i) {
    int offset = i - kernelSize / 2;
    unsigned start, end;
    
    if (offset < 0) {
      start = -offset;
    } else {
      start = 0;
    }
    if (offset + kernelSize > dataSize) {
      end = dataSize - offset;
    } else {
      end = kernelSize;
    }

    float sum = 0.0f;
    float weight = 0.0f;
    for (unsigned k = start; k < end; k++) {
      sum += data[k + offset] * kernel[k];
      weight += kernel[k];
    }

    if (weight != 0.0f) {
      dest[i] = sum / weight;
    }
  }
}

void Flagger::oneDimensionalGausConvolution(const float* data, const unsigned dataSize, float* dest, const float sigma) {
  unsigned kernelSize = (unsigned) round(sigma * 3.0);
  if(kernelSize < 1) {
    kernelSize = 1;
  } else if (kernelSize > dataSize) {
    kernelSize = dataSize;
  }

  float kernel[kernelSize];
  for (unsigned i = 0; i < kernelSize; ++i) {
    float x = i - kernelSize / 2.0f;
    kernel[i] = evaluateGaussian(x, sigma);
  }
  oneDimensionalConvolution(data, dataSize, dest, kernel, kernelSize);
}

float Flagger::calcThresholdI(float threshold1, unsigned window, float p) {
  if (p <= 0.0f) {
    p = 1.5f; // according to Andre's RFI paper, this is a good default value
  }
	
  return (float) (threshold1 * pow(p, logBase2(window)) / window);
}


void Flagger::sumThreshold(std::vector<float>& powers, std::vector<bool>& flags, const unsigned window, const float threshold) {
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


void Flagger::thresholdingFlagger1D(std::vector<float>& powers, std::vector<bool>& flags) {
  float mean, stdDev, median;
  calculateStatistics(powers.data(), powers.size(), mean, median, stdDev);

  float threshold = median + itsCutoffThreshold * stdDev;

  for (unsigned channel = 0; channel < powers.size(); channel++) {
    if (powers[channel] > threshold) {
      flags[channel] = true;
    }
  }
}


void Flagger::thresholdingFlagger2D(const MultiDimArray<float,2> &powers, MultiDimArray<bool,2> &flags) {
  float mean, stdDev, median;
  calculateStatistics(powers.data(), powers.size(), mean, median, stdDev);

  float threshold = median + itsCutoffThreshold * stdDev;

  for (unsigned channel = 0; channel < powers.shape()[0]; channel++) {
    for (unsigned time = 0; time < powers.shape()[1]; time++) {
      const float power = powers[channel][time];
      if (power > threshold) {
	// flag this sample, both polarizations.
	flags[channel][time] = true;
      }
    }
  }
}


void Flagger::sumThresholdFlagger1D(std::vector<float>& powers, std::vector<bool>& flags, const float sensitivity) {
  float mean, stdDev, median;
  calculateStatistics(powers.data(), powers.size(), mean, median, stdDev);

  float factor;
  if (stdDev == 0.0f) {
    factor = sensitivity;
  } else {
    factor = stdDev * sensitivity;
  }

  unsigned window = 1;
  for (unsigned iter = 1; iter <= MAX_ITERS; iter++) {
    float thresholdI = median + calcThresholdI(itsCutoffThreshold, iter, 1.5f) * factor;
//    LOG_DEBUG_STR("THRESHOLD in iter " << iter <<", window " << window << " = " << calcThresholdI(itsCutoffThreshold, iter, 1.5f) << ", becomes = " << thresholdI);
    sumThreshold(powers, flags, window, thresholdI);
    window *= 2;
  }
}


void Flagger::sumThresholdFlaggerSmoothed1D(std::vector<float>& powers, std::vector<float>& smoothedPowers, std::vector<float>& powerDiffs, std::vector<bool>& flags) {
  // first do an insensitive sumthreshold
  sumThresholdFlagger1D(powers, flags, 1.0f * itsBaseSensitivity); // sets flags, and replaces flagged samples with threshold
	
  // smooth
  oneDimensionalGausConvolution(powers.data(), powers.size(), smoothedPowers.data(), 0.5f); // last param is sigma, height of the gaussian curve

  // calculate difference
  for (unsigned int i = 0; i < powers.size(); i++) {
    powerDiffs[i] = powers[i] - smoothedPowers[i];
  }
  
  // flag based on difference
  sumThresholdFlagger1D(powerDiffs, flags, 1.0f * itsBaseSensitivity); // sets additional flags
  
  // and one final, more sensitive pass on the flagged power
  sumThresholdFlagger1D(powers, flags, 0.8f * itsBaseSensitivity); // sets flags, and replaces flagged samples with threshold
}


void Flagger::sumThresholdFlaggerSmoothedWithHistory1D(std::vector<float>& powers, std::vector<float>& smoothedPowers, std::vector<float>& powerDiffs, 
						       std::vector<bool>& flags, HistoryList& history) {
  float historyFlaggingThreshold = 7.0f;

  sumThresholdFlaggerSmoothed1D(powers, smoothedPowers, powerDiffs, flags);
  
  float localMean, localStdDev, localMedian;

  // calculate final statistics (flagged samples were replaced with threshold values)
  calculateStatistics(powers.data(), powers.size(), localMean, localMedian, localStdDev);

//  LOG_DEBUG_STR("median = " << median << ", local = " << localMedian);

//  LOG_DEBUG_STR("History flagger, history size is now: " << history.getSize());

  // add the corrected power statistics to the history
  if (history.getSize() < MIN_HISTORY_SIZE) {
    history.add(localMedian); // add it, we don't have enough history yet.
  } else {
    float meanMedian = history.getMeanMedian();
    float stdDevOfMedians = history.getStdDevOfMedians();

//    float factor =  (meanMedian + historyFlaggingThreshold * stdDevOfMedians) / localMedian;
//    LOG_DEBUG_STR("localMedian = " << localMedian << ", meanMedian = " << meanMedian << ", stdDevOfMedians = " << stdDevOfMedians << ", factor from cuttoff is: " << factor);

    bool flagSecond = localMedian > (meanMedian + historyFlaggingThreshold * stdDevOfMedians);
    if (flagSecond) {
//      LOG_DEBUG_STR("History flagger flagged this second");
      for (unsigned i = 0; i < powers.size(); i++) {
        flags[i] = true;
      }
      // this second was flagged, add the mean median to the history.
      history.add(meanMedian);
    } else {
      // add data
      history.add(localMedian);
    }
  }
}


FlaggerStatisticsType Flagger::getFlaggerStatisticsType(std::string t) {
  if (t.compare("NORMAL") == 0) {
    return FLAGGER_STATISTICS_NORMAL;
  } else if (t.compare("WINSORIZED") == 0) {
    return FLAGGER_STATISTICS_WINSORIZED;
  } else {
    LOG_DEBUG_STR("unknown flagger statistics type, using default FLAGGER_STATISTICS_WINSORIZED");
    return FLAGGER_STATISTICS_WINSORIZED;
  }
}

std::string Flagger::getFlaggerStatisticsTypeString(FlaggerStatisticsType t) {
  switch(t) {
  case FLAGGER_STATISTICS_NORMAL:
    return "FLAGGER_STATISTICS_NORMAL";
  case FLAGGER_STATISTICS_WINSORIZED:
    return "FLAGGER_STATISTICS_WINSORIZED";
  default:
    return "ILLEGAL FLAGGER STATISTICS TYPE";
  }
}

std::string Flagger::getFlaggerStatisticsTypeString() {
  return getFlaggerStatisticsTypeString(itsFlaggerStatisticsType);
}

} // namespace RTCP
} // namespace LOFAR
