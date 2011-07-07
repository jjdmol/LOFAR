//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

#include <FIR.h>
#include <cstring>

#include <math.h>
#include <iostream>
#include <cstring>

#include <Common/LofarLogger.h>


namespace LOFAR {
namespace RTCP {

template <typename FIR_SAMPLE_TYPE> FIR<FIR_SAMPLE_TYPE>::FIR()
{
}


template <typename FIR_SAMPLE_TYPE> void FIR<FIR_SAMPLE_TYPE>::initFilter(FilterBank* filterBank, const unsigned channel)
{
  itsFilterBank = filterBank;
  itsChannel = channel;
  itsNrTaps = filterBank->getNrTaps();
  itsWeights = filterBank->getWeights(channel);

  itsDelayLine.resize(itsNrTaps);
  memset(itsDelayLine.data(), 0, sizeof(FIR_SAMPLE_TYPE) * itsNrTaps);
}


template <typename FIR_SAMPLE_TYPE> FIR_SAMPLE_TYPE FIR<FIR_SAMPLE_TYPE>::processNextSample(const FIR_SAMPLE_TYPE sample)
{
  FIR_SAMPLE_TYPE sum = sample * itsWeights[0];
  itsDelayLine[0] = sample;

  for (int tap = itsNrTaps; -- tap > 0;) {
    sum += itsWeights[tap] * itsDelayLine[tap];
    itsDelayLine[tap] = itsDelayLine[tap - 1];
  }

  return sum;
}

template class FIR<float>;
template class FIR<fcomplex>;

} // namespace RTCP
} // namespace LOFAR
