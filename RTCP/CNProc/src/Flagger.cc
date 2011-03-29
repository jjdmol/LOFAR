//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

#include <Flagger.h>
#include <math.h>
#include <algorithm>
#include <string.h>

namespace LOFAR {
namespace RTCP {

// TODO float* -> float[]

Flagger::Flagger(const unsigned nrStations, const unsigned nrChannels, const float cutoffThreshold, float baseSentitivity, float firstThreshold, 
		 FlaggerType flaggerType, FlaggerStatisticsType flaggerStatisticsType) :
  itsNrStations(nrStations), itsNrChannels(nrChannels), itsCutoffThreshold(cutoffThreshold), itsBaseSensitivity(baseSentitivity), itsFirstThreshold(firstThreshold), 
  itsFlaggerType(flaggerType), itsFlaggerStatisticsType(flaggerStatisticsType)
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

} // namespace RTCP
} // namespace LOFAR
