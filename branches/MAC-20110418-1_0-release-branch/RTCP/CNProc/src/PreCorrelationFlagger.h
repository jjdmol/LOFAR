#ifndef LOFAR_CNPROC_PRE_CORRELATION_FLAGGER_H
#define LOFAR_CNPROC_PRE_CORRELATION_FLAGGER_H

#include <Flagger.h>
#include <Interface/FilteredData.h>

namespace LOFAR {
namespace RTCP {

class PreCorrelationFlagger : public Flagger {
  public:
  PreCorrelationFlagger(const unsigned nrStations, const unsigned nrChannels, const unsigned nrSamplesPerIntegration, float cutoffThreshold = 7.0f);

  void flag(FilteredData* filteredData);

  private:

  // Does simple thresholding.
  void thresholdingFlagger(const unsigned station, FilteredData* filteredData, const MultiDimArray<float,3> &powers, const float mean, const float stdDev, const float median);

  // calculates mean, stddev, and median.
  void calculateStatistics(unsigned station, FilteredData* filteredData, MultiDimArray<float,3> &powers, float& mean, float& stdDev, float& median);


  const unsigned itsNrSamplesPerIntegration;
};

} // namespace RTCP
} // namespace LOFAR

#endif // LOFAR_CNPROC_PRE_CORRELATION_FLAGGER_H
