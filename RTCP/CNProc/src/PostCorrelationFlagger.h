#ifndef LOFAR_CNPROC_POST_CORRELATION_FLAGGER_H
#define LOFAR_CNPROC_POST_CORRELATION_FLAGGER_H

#include <Flagger.h>
#include <Interface/CorrelatedData.h>

namespace LOFAR {
namespace RTCP {

  class PostCorrelationFlagger : public Flagger
{
  public:
  PostCorrelationFlagger(const unsigned nrStations, const unsigned nrChannels, float cutoffThreshold = 7.0f);

  void flag(CorrelatedData* correlatedData);

  // Does simple thresholding
  void thresholdingFlagger(CorrelatedData* correlatedData);

  // Tries to detect broken stations
  void detectBrokenStations();

  // calculates mean, stddev, and median.
  void calculateGlobalStatistics(CorrelatedData* correlatedData);

  private:
  const unsigned itsNrBaselines;
  
  std::vector<float> itsSummedBaselinePowers; // [nrBaselines]

  std::vector<float> itsSummedStationPowers; // [nrStations]

};


} // namespace RTCP
} // namespace LOFAR

#endif // LOFAR_CNPROC_POST_CORRELATION_FLAGGER_H
