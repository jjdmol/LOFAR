#ifndef LOFAR_CNPROC_FLAGGER_H
#define LOFAR_CNPROC_FLAGGER_H

#include <vector>

namespace LOFAR {
namespace RTCP {

class Flagger {
  public:

  Flagger(const unsigned nrStations, const unsigned nrChannels, const float cutoffThreshold);

  static float calculateStdDev(const float* data, const unsigned size, const float mean);
  static float calculateMedian(const float* data, const unsigned size);

  const unsigned itsNrStations;
  const unsigned itsNrChannels;
  const float itsCutoffThreshold;
};

} // namespace RTCP
} // namespace LOFAR

#endif // LOFAR_CNPROC_FLAGGER_H
