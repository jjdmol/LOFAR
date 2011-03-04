#ifndef LOFAR_CNPROC_FLAGGER_H
#define LOFAR_CNPROC_FLAGGER_H

#include <vector>

namespace LOFAR {
namespace RTCP {

class Flagger
{
  public:

  Flagger(const unsigned nrStations, const unsigned nrChannels, const unsigned itsTotalNrSamples, const float cutoffThreshold);

  static float calculateStdDev(const std::vector<float>& data, const float mean);
  static float calculateMedian(const std::vector<float>& origData);


  const unsigned itsNrStations, itsNrChannels;
  const unsigned itsTotalNrSamples;
  const float itsCutoffThreshold;

  std::vector<float> itsPowers;
  float itsPowerMean;
  float itsPowerStdDev;
  float itsPowerMedian;
};

} // namespace RTCP
} // namespace LOFAR

#endif // LOFAR_CNPROC_FLAGGER_H
