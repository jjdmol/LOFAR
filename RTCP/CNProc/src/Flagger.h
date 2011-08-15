#ifndef LOFAR_CNPROC_FLAGGER_H
#define LOFAR_CNPROC_FLAGGER_H

#include <Interface/MultiDimArray.h>

#include <FlaggerHistory.h>

#include <string>
#include <vector>

namespace LOFAR {
namespace RTCP {

class Parset;

enum FlaggerStatisticsType {
  FLAGGER_STATISTICS_NORMAL,
  FLAGGER_STATISTICS_WINSORIZED
};

class Flagger {

public:

  // The firstThreshold of 6.0 is taken from Andre's code.
  Flagger(const Parset& parset, const unsigned nrStations, const unsigned nrChannels, const float cutoffThreshold = 6.0f, float baseSentitivity = 1.0f,
	  FlaggerStatisticsType flaggerStatisticsType = FLAGGER_STATISTICS_WINSORIZED);

private:
  void calculateNormalStatistics(const float* data, const unsigned size, float& mean, float& median, float& stdDev);
  void calculateWinsorizedStatistics(const float* data, const unsigned size, float& mean, float& median, float& stdDev);
  float evaluateGaussian(const float x, const float sigma);
  float logBase2(const float x);
  void oneDimensionalConvolution(const float* data, const unsigned dataSize, float*dest, const float* kernel, const unsigned kernelSize);

protected:

  // Does simple thresholding.
  void thresholdingFlagger1D(std::vector<float>& powers, std::vector<bool>& flags);
  void thresholdingFlagger2D(const MultiDimArray<float,2> &powers, MultiDimArray<bool,2> &flags);

  // Does sum thresholding.
  void sumThresholdFlagger1D(std::vector<float>& powers, std::vector<bool>& flags, const float sensitivity);

  // Does sum thresholding on samples, a gaussion smooth, calculates difference, and flags again.
  void sumThresholdFlaggerSmoothed1D(std::vector<float>& powers, std::vector<float>& smoothedPowers, std::vector<float>& powerDiffs, std::vector<bool>& flags);

  void sumThresholdFlaggerSmoothedWithHistory1D(std::vector<float>& powers, std::vector<float>& smoothedPowers, std::vector<float>& powerDiffs, 
						std::vector<bool>& flags, HistoryList& history);

  void calculateStatistics(const float* data, const unsigned size, float& mean, float& median, float& stdDev);
  float calcThresholdI(float threshold1, unsigned window, float p);
  void sumThreshold(std::vector<float>& powers, std::vector<bool>& flags, const unsigned window, const float threshold);
  void oneDimensionalGausConvolution(const float* data, const unsigned dataSize, float*dest, const float sigma);
  void calculateStdDevAndSum(const float* data, const unsigned size, const float mean, float& stdDev, float& sum);
  float calculateMedian(const float* data, const unsigned size);

//  FlaggerType getFlaggerType(std::string t);
  FlaggerStatisticsType getFlaggerStatisticsType(std::string t);

//  std::string getFlaggerTypeString();
  std::string getFlaggerStatisticsTypeString();
//  std::string getFlaggerTypeString(FlaggerType t);
  std::string getFlaggerStatisticsTypeString(FlaggerStatisticsType t);

  const Parset& itsParset;
  const unsigned itsNrStations;
  const unsigned itsNrChannels;
  const float itsCutoffThreshold;
  const float itsBaseSensitivity;
//  const FlaggerType itsFlaggerType;
  const FlaggerStatisticsType itsFlaggerStatisticsType;
};

} // namespace RTCP
} // namespace LOFAR

#endif // LOFAR_CNPROC_FLAGGER_H
