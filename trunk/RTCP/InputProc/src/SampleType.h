#ifndef __SAMPLETYPE__
#define __SAMPLETYPE__

#include <complex>
#include <ostream>

namespace LOFAR {
namespace RTCP {


template<typename T> struct SampleType {
  std::complex<T> x;
  std::complex<T> y;
};


template<typename T> std::ostream &operator <<(std::ostream &str, const struct SampleType<T> &sample)
{
  str << "(" << sample.x.real() << " + " << sample.x.imag() << "i, " << sample.y.real() << " + " << sample.y.imag() << "i)";

  return str;
}


}
}

#endif

