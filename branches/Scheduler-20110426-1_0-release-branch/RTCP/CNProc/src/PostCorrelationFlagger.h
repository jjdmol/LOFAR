#ifndef LOFAR_CNPROC_POST_CORRELATION_FLAGGER_H
#define LOFAR_CNPROC_POST_CORRELATION_FLAGGER_H

#include <Flagger.h>

namespace LOFAR {
namespace RTCP {

class CorrelatedData;

  class PostCorrelationFlagger : public Flagger
{
  public:
  // The firstThreshold of 6.0 is taken from Andre's code.
  PostCorrelationFlagger(const unsigned nrStations, const unsigned nrChannels, const float cutoffThreshold = 7.0f, float baseSentitivity = 1.0f, float firstThreshold = 6.0f, 
	  FlaggerType flaggerType = FLAGGER_SUM_THRESHOLD, FlaggerStatisticsType flaggerStatisticsType = FLAGGER_STATISTICS_WINSORIZED);

  void flag(CorrelatedData* correlatedData);

  // Does simple thresholding.
  void thresholdingFlagger(const float mean, const float stdDev, const float median);

  // Does sum thresholding.
  void sumThresholdFlagger(const float mean, const float stdDev, const float median);

  // Tries to detect broken stations
  void detectBrokenStations();

private:
  // calculates mean, stddev, and median.
  void calculatePowers(unsigned baseline, unsigned pol1, unsigned pol2, CorrelatedData* correlatedData);
  void sumThreshold(unsigned window, float threshold);
  void calculateSummedbaselinePowers(unsigned baseline);

  void wipeFlags();
  void applyFlags(unsigned baseline, CorrelatedData* correlatedData);
  void wipeSums();

  const unsigned itsNrBaselines;

  std::vector<float> itsPowers;
  std::vector<bool> itsFlags;
  std::vector<float> itsSummedBaselinePowers; // [nrBaselines]
  std::vector<float> itsSummedStationPowers; // [nrStations]

};


} // namespace RTCP
} // namespace LOFAR

#endif // LOFAR_CNPROC_POST_CORRELATION_FLAGGER_H
