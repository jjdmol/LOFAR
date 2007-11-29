#ifndef LOFAR_APPL_CEP_CS1_CS1_BGL_PROC_FIR_H
#define LOFAR_APPL_CEP_CS1_CS1_BGL_PROC_FIR_H

#if 0 || !defined HAVE_BGL
#define FIR_C_IMPLEMENTATION
#endif

#include <Common/lofar_complex.h>

#include <CS1_Interface/CS1_Config.h>


namespace LOFAR {
namespace CS1 {

class FIR {
  public:
#if defined FIR_C_IMPLEMENTATION
    FIR();

    fcomplex processNextSample(fcomplex sample, const float weights[NR_TAPS]);

    fcomplex itsDelayLine[NR_TAPS];
#endif

    static const float weights[NR_SUBBAND_CHANNELS][NR_TAPS];
};

} // namespace CS1
} // namespace LOFAR

#endif
