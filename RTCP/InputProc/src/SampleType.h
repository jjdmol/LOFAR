#ifndef __SAMPLETYPE__
#define __SAMPLETYPE__

#include <complex>
#include <ostream>
#include <Common/LofarTypes.h>

namespace LOFAR {
namespace RTCP {


template<typename T> struct SampleType {
  T x, y;
};

template<> struct SampleType<i16complex>;
template<> struct SampleType<i8complex>;
template<> struct SampleType<i4complex>;


template<typename T> std::ostream &operator <<(std::ostream &str, const struct SampleType<T> &sample)
{
  str << "(" << sample.x.real() << " + " << sample.x.imag() << "i, " << sample.y.real() << " + " << sample.y.imag() << "i)";

  return str;
}


}
}

#endif

