#ifndef LOFAR_CNPROC_FIR_H
#define LOFAR_CNPROC_FIR_H

#if 0 || !defined HAVE_BGP
#define FIR_C_IMPLEMENTATION
#endif

#include <Common/lofar_complex.h>

#include <FilterBank.h>

#include <Interface/Config.h>
#include <Interface/AlignedStdAllocator.h>

#include <boost/multi_array.hpp>

namespace LOFAR {
namespace RTCP {

template <typename FIR_SAMPLE_TYPE> class FIR {
  public:

  // We need a default constructor, since we create boost multi-arrays of FIR filters.
  FIR();

  void initFilter(FilterBank* filterBank, const unsigned channel);

  FIR_SAMPLE_TYPE processNextSample(const FIR_SAMPLE_TYPE sample);

  float* getWeights();

private:
  std::vector<FIR_SAMPLE_TYPE> itsDelayLine;
  FilterBank* itsFilterBank;
  unsigned itsChannel;
  unsigned itsNrTaps;
  float* itsWeights; // pointer to weights in the filterBank
};

} // namespace RTCP
} // namespace LOFAR

#endif
