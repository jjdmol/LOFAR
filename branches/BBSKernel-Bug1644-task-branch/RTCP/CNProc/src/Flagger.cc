//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

#include <Flagger.h>
#include <math.h>
#include <algorithm>

namespace LOFAR {
namespace RTCP {

Flagger::  Flagger(const unsigned nrStations, const unsigned nrChannels, const unsigned totalNrSamples, const float cutoffThreshold) :
  itsNrStations(nrStations),
  itsNrChannels(nrChannels),
  itsTotalNrSamples(totalNrSamples),
  itsCutoffThreshold(cutoffThreshold)
{
}


float Flagger::calculateStdDev(const std::vector<float>& data, const float mean) {
  float stdDev = 0.0f;

  for(unsigned i = 0; i<data.size(); i++) {
    float diff = data[i] - mean;
    stdDev += diff * diff;
  }
  stdDev /= data.size();
  stdDev = sqrt(stdDev);

  return stdDev;
}


float Flagger::calculateMedian(const std::vector<float>& origData) {
  // we have to copy the vector, nth_element changes the ordering.
  std::vector<float> data = origData;

  // calculate median, expensive, but nth_element is guaranteed to be O(n)
  std::vector<float>::iterator it = data.begin() + (data.size() / 2);
  std::nth_element(data.begin(), it, data.end());
  return *it;
}

} // namespace RTCP
} // namespace LOFAR
