#ifndef LOFAR_CNPROC_FIR_H
#define LOFAR_CNPROC_FIR_H

#define USE_ORIGINAL_FILTER 0

#if 0 || !(defined HAVE_BGL || defined HAVE_BGP)
#define FIR_C_IMPLEMENTATION
#endif

#include <Common/lofar_complex.h>

#include <Interface/Config.h>
#include <Interface/AlignedStdAllocator.h>

#include <boost/multi_array.hpp>

namespace LOFAR {
namespace RTCP {

class FIR {
  public:

  // This method initializes the weights array.
  static void generate_filter(unsigned taps, unsigned channels);

#if defined FIR_C_IMPLEMENTATION
    FIR();

    fcomplex processNextSample(fcomplex sample, unsigned channel);

    fcomplex itsDelayLine[NR_TAPS];
#endif

private:
  // Hamming window function
  static void hamming(unsigned n, double* d);

  // Blackman window function
  static void blackman(unsigned n, double* d);

  // Gaussian window function
  static void gaussian(int n, double a, double* d);

  static unsigned next_power_of_2(unsigned n);
  static void interpolate(double* x, double* y, unsigned xlen, unsigned n, double* result);
  static void generate_fir_filter(unsigned n, double w, double* window, double* result);

public:

// The first subband is from -98 KHz to 98 KHz, rather than from 0 to 195 KHz.
// To avoid that the FFT outputs the channels in the wrong order (from 128 to
// 255 followed by channels 0 to 127), we multiply each second FFT input by -1.
// This is efficiently achieved by negating the FIR filter constants of all
// uneven FIR filters.
// Also, the constants for a channel are in reverse order. This makes the 
// implentation more efficient.
  static boost::multi_array<float, 2, AlignedStdAllocator<float, 32> > weights; // [nrChannels][NR_TAPS];

#if USE_ORIGINAL_FILTER
  static const float origWeights[256][NR_TAPS];
#endif
};

} // namespace RTCP
} // namespace LOFAR

#endif
