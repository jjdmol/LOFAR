#ifndef LOFAR_CNPROC_FLAGGER_H
#define LOFAR_CNPROC_FLAGGER_H

#include <vector>

namespace LOFAR {
namespace RTCP {

enum FlaggerStatisticsType {
  FLAGGER_STATISTICS_NORMAL,
  FLAGGER_STATISTICS_WINSORIZED
};

enum FlaggerType {
  FLAGGER_THRESHOLD,
  FLAGGER_SUM_THRESHOLD
};

class Flagger {
  public:

  // The firstThreshold of 6.0 is taken from Andre's code.
  Flagger(const unsigned nrStations, const unsigned nrChannels, const float cutoffThreshold = 7.0f, float baseSentitivity = 1.0f, float firstThreshold = 6.0f, 
	  FlaggerType flaggerType = FLAGGER_SUM_THRESHOLD, FlaggerStatisticsType flaggerStatisticsType = FLAGGER_STATISTICS_WINSORIZED);

  void calculateStdDevAndSum(const float* data, const unsigned size, const float mean, float& stdDev, float& sum);
  float calculateMedian(const float* data, const unsigned size);

  void calculateStatistics(const float* data, const unsigned size, float& mean, float& median, float& stdDev);
  void calculateWinsorizedStatistics(const float* data, const unsigned size, float& mean, float& median, float& stdDev);

  float evaluateGaussian(const float x, const float sigma);
  float logBase2(const float x);
  void oneDimensionalConvolution(const float* data, const unsigned dataSize, float*dest, const float* kernel, const unsigned kernelSize);
  void oneDimensionalGausConvolution(const float* data, const unsigned dataSize, float*dest, const float sigma);
  float calcThresholdI(float threshold1, unsigned window, float p);

  const unsigned itsNrStations;
  const unsigned itsNrChannels;
  const float itsCutoffThreshold;
  const float itsBaseSensitivity;
  const float itsFirstThreshold;
  const FlaggerType itsFlaggerType;
  const FlaggerStatisticsType itsFlaggerStatisticsType;
};

} // namespace RTCP
} // namespace LOFAR

#endif // LOFAR_CNPROC_FLAGGER_H
