#ifndef LOFAR_CNPROC_PRE_CORRELATION_FLAGGER_H
#define LOFAR_CNPROC_PRE_CORRELATION_FLAGGER_H

#include <Flagger.h>
#include <Interface/FilteredData.h>

namespace LOFAR {
namespace RTCP {

  class PreCorrelationFlagger : public Flagger
{
  public:
  PreCorrelationFlagger(const unsigned nrStations, const unsigned nrChannels, const unsigned nrSamplesPerIntegration, float cutoffThreshold = 7.0f);

  void flag(FilteredData* filteredData);

  private:

  // Does simple thresholding
  void thresholdingFlagger(FilteredData* filteredData);

  // calculates mean, stddev, and median.
  void calculateGlobalStatistics(FilteredData* filteredData);


  const unsigned itsNrSamplesPerIntegration;
};


} // namespace RTCP
} // namespace LOFAR

#endif // LOFAR_CNPROC_PRE_CORRELATION_FLAGGER_H
