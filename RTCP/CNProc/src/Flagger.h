#ifndef LOFAR_CNPROC_FLAGGER_H
#define LOFAR_CNPROC_FLAGGER_H

#include <string>

namespace LOFAR {
namespace RTCP {

class Parset;

enum FlaggerStatisticsType {
  FLAGGER_STATISTICS_NORMAL,
  FLAGGER_STATISTICS_WINSORIZED
};

enum FlaggerType {
  FLAGGER_THRESHOLD,
  FLAGGER_SUM_THRESHOLD,
  FLAGGER_SMOOTHED_SUM_THRESHOLD,
  FLAGGER_SMOOTHED_SUM_THRESHOLD_WITH_HISTORY
};

class Flagger {

public:

  // The firstThreshold of 6.0 is taken from Andre's code.
  Flagger(const Parset& parset, const unsigned nrStations, const unsigned nrChannels, const float cutoffThreshold = 6.0f, float baseSentitivity = 1.0f, float firstThreshold = 6.0f, 
	  FlaggerType flaggerType = FLAGGER_SUM_THRESHOLD, FlaggerStatisticsType flaggerStatisticsType = FLAGGER_STATISTICS_WINSORIZED);

private:
  void calculateNormalStatistics(const float* data, const unsigned size, float& mean, float& median, float& stdDev);
  void calculateWinsorizedStatistics(const float* data, const unsigned size, float& mean, float& median, float& stdDev);
  float evaluateGaussian(const float x, const float sigma);
  float logBase2(const float x);
  void oneDimensionalConvolution(const float* data, const unsigned dataSize, float*dest, const float* kernel, const unsigned kernelSize);

protected:
  void calculateStatistics(const float* data, const unsigned size, float& mean, float& median, float& stdDev);
  float calcThresholdI(float threshold1, unsigned window, float p);
  void oneDimensionalGausConvolution(const float* data, const unsigned dataSize, float*dest, const float sigma);
  void calculateStdDevAndSum(const float* data, const unsigned size, const float mean, float& stdDev, float& sum);
  float calculateMedian(const float* data, const unsigned size);

  FlaggerType getFlaggerType(std::string t);
  FlaggerStatisticsType getFlaggerStatisticsType(std::string t);

  std::string getFlaggerTypeString();
  std::string getFlaggerStatisticsTypeString();
  std::string getFlaggerTypeString(FlaggerType t);
  std::string getFlaggerStatisticsTypeString(FlaggerStatisticsType t);

  const Parset& itsParset;
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
