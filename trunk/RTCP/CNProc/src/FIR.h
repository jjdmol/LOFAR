#ifndef LOFAR_CNPROC_FIR_H
#define LOFAR_CNPROC_FIR_H

#define USE_ORIGINAL_FILTER 0

#if 0 || !defined HAVE_BGP
#define FIR_C_IMPLEMENTATION
#endif

#include <Common/lofar_complex.h>

#include <Interface/Config.h>
#include <Interface/AlignedStdAllocator.h>

#include <boost/multi_array.hpp>

namespace LOFAR {
namespace RTCP {

enum WindowType { HAMMING, BLACKMAN, GAUSSIAN, KAISER };

class FIR {
  public:

  // This method initializes the weights array.
  static void generate_filter(const unsigned taps, const unsigned channels, const bool verbose);

#if defined FIR_C_IMPLEMENTATION
    FIR();

    fcomplex processNextSample(const fcomplex sample, const unsigned channel);

    fcomplex itsDelayLine[NR_TAPS];
#endif

private:
  // Hamming window function
  static void hamming(const unsigned n, double* d);

  // Blackman window function
  static void blackman(const unsigned n, double* d);

  // Gaussian window function
  static void gaussian(const int n, const double a, double* d);

  // Kaiser window function
  static void kaiser(const int n, const double beta, double* d);

  // helper functions
  static double besselI0(const double x);
  static unsigned next_power_of_2(const unsigned n);
  static void interpolate(const double* x, const double* y, const unsigned xlen, const unsigned n, double* result);
  static void generate_fir_filter(const unsigned n, const double w, const double* window, double* result);

  // The window used for generating the filter
  static const WindowType itsWindowType = KAISER;

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
