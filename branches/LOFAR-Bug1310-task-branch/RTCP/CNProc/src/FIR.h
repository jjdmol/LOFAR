#ifndef LOFAR_CNPROC_FIR_H
#define LOFAR_CNPROC_FIR_H

#if 0 || !(defined HAVE_BGL || defined HAVE_BGP)
#define FIR_C_IMPLEMENTATION
#endif

#include <Common/lofar_complex.h>

#include <Interface/Config.h>


namespace LOFAR {
namespace RTCP {

class FIR {
  public:
#if defined FIR_C_IMPLEMENTATION
    FIR();

    fcomplex processNextSample(fcomplex sample, const float weights[NR_TAPS]);

    fcomplex itsDelayLine[NR_TAPS];
#endif

    static const float weights[256][NR_TAPS];
};

} // namespace RTCP
} // namespace LOFAR

#endif
