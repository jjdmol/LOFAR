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
  Flagger(const Parset& parset, const unsigned nrStations,   const unsigned nrSubbands, const unsigned nrChannels, const float cutoffThreshold = 6.0f, 
	  float baseSentitivity = 1.0f, FlaggerStatisticsType flaggerStatisticsType = FLAGGER_STATISTICS_WINSORIZED);

private:
  float evaluateGaussian(const float x, const float sigma);
  float logBase2(const float x);
  void oneDimensionalConvolution(const float* data, const unsigned dataSize, float* dest, const float* kernel, const unsigned kernelSize);
  void sumThreshold2DHorizontal(MultiDimArray<float,2> &powers, MultiDimArray<bool,2> &flags, const unsigned window, const float threshold);
  void sumThreshold2DVertical(MultiDimArray<float,2> &powers, MultiDimArray<bool,2> &flags, const unsigned window, const float threshold);
  bool addToHistory(const float localMean, const float localStdDev, const float localMedian, std::vector<float> powers, FlaggerHistory& history);
  void apply1DflagsTo2D(MultiDimArray<bool,2> &flags, std::vector<bool> & integratedFlags);

protected:

  // Does simple thresholding.
  void thresholdingFlagger1D(std::vector<float>& powers, std::vector<bool>& flags);
  void thresholdingFlagger2D(const MultiDimArray<float,2> &powers, MultiDimArray<bool,2> &flags);

  // Does sum thresholding.
  void sumThresholdFlagger1D(std::vector<float>& powers, std::vector<bool>& flags, const float sensitivity);

  void sumThresholdFlagger1DWithHistory(std::vector<float>& powers, std::vector<bool>& flags, const float sensitivity, FlaggerHistory& history);

  void sumThresholdFlagger2D(MultiDimArray<float,2> &powers, MultiDimArray<bool,2> &flags, const float sensitivity);

  void sumThresholdFlagger2DWithHistory(MultiDimArray<float,2> &powers, MultiDimArray<bool,2> &flags, std::vector<float> &integratedPowers,
					std::vector<bool> &integratedFlags, const float sensitivity, 
					MultiDimArray<FlaggerHistory, 3>& history, unsigned station, unsigned subband, unsigned pol);


  // Does sum thresholding on samples, a gaussion smooth, calculates difference, and flags again.
  void sumThresholdFlagger1DSmoothed(std::vector<float>& powers, std::vector<float>& smoothedPowers, 
				     std::vector<float>& powerDiffs, std::vector<bool>& flags);

  void sumThresholdFlagger1DSmoothedWithHistory(std::vector<float>& powers, std::vector<float>& smoothedPowers, 
						std::vector<float>& powerDiffs, 
						std::vector<bool>& flags, FlaggerHistory& history);

  void calculateMeanAndStdDev(const std::vector<float>& powers, float& mean, float& stdDev);
  float calculateMedian(const std::vector<float>& powers);
  void calculateMeanAndStdDev(const std::vector<float>& powers, const std::vector<bool>& flags, float& mean, float& stdDev);
  float calculateMedian(const std::vector<float>& powers, const std::vector<bool>& flags);
  void calculateNormalStatistics(const std::vector<float>& powers, const std::vector<bool>& flags, float& mean, float& median, float& stdDev);
  void calculateWinsorizedStatistics(const std::vector<float>& powers, const std::vector<bool>& flags, float& mean, float& median, float& stdDev);
  void calculateStatistics(const std::vector<float>& powers, const std::vector<bool>& flags, float& mean, float& median, float& stdDev);
  void calculateStatistics(const MultiDimArray<float,2> &powers, MultiDimArray<bool,2> &flags, float& mean, float& median, float& stdDev);

  float calcThresholdI(float threshold1, unsigned window, float p);
  void sumThreshold1D(std::vector<float>& powers, std::vector<bool>& flags, const unsigned window, const float threshold);
  void oneDimensionalGausConvolution(const float* data, const unsigned dataSize, float*dest, const float sigma);

  std::string getFlaggerStatisticsTypeString();
  std::string getFlaggerStatisticsTypeString(FlaggerStatisticsType t);
  FlaggerStatisticsType getFlaggerStatisticsType(std::string t);

  const Parset& itsParset;
  const unsigned itsNrStations;
  const unsigned itsNrSubbands;
  const unsigned itsNrChannels;
  const float itsCutoffThreshold;
  const float itsBaseSensitivity;
  const FlaggerStatisticsType itsFlaggerStatisticsType;
};

} // namespace RTCP
} // namespace LOFAR

#endif // LOFAR_CNPROC_FLAGGER_H
