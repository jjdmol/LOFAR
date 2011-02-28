//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

#include <Flagger.h>
#include <math.h>
#include <algorithm>
#include <string.h>

namespace LOFAR {
namespace RTCP {

Flagger::Flagger(const unsigned nrStations, const unsigned nrChannels, const float cutoffThreshold) :
  itsNrStations(nrStations),
  itsNrChannels(nrChannels),
  itsCutoffThreshold(cutoffThreshold)
{
}


float Flagger::calculateStdDev(const float* data, const unsigned size, const float mean)
{
  float stdDev = 0.0f;

  for(unsigned i = 0; i<size; i++) {
    float diff = data[i] - mean;
    stdDev += diff * diff;
  }
  stdDev /= size;
  stdDev = sqrt(stdDev);

  return stdDev;
}


float Flagger::calculateMedian(const float* data, const unsigned size)
{
  // we have to copy the vector, nth_element changes the ordering.
  std::vector<float> copy;
  copy.resize(size);
  memcpy(copy.data(), data, size * sizeof(float));

  // calculate median, expensive, but nth_element is guaranteed to be O(n)
  std::vector<float>::iterator it = copy.begin() + (size / 2);
  std::nth_element(copy.begin(), it, copy.end());
  return *it;
}

} // namespace RTCP
} // namespace LOFAR
