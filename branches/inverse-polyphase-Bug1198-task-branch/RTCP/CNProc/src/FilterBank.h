#ifndef LOFAR_CNPROC_FILTER_BANK_H
#define LOFAR_CNPROC_FILTER_BANK_H

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

enum WindowType { HAMMING, BLACKMAN, GAUSSIAN, KAISER, PREDEFINED_FILTER };

// The first subband is from -98 KHz to 98 KHz, rather than from 0 to 195 KHz.
// To avoid that the FFT outputs the channels in the wrong order (from 128 to
// 255 followed by channels 0 to 127), we multiply each second FFT input by -1.
// This is efficiently achieved by negating the FIR filter constants of all
// uneven FIR filters.
// Also, the filter tap constants for a channel are in reverse order. This makes the 
// implentation more efficient.

class FilterBank {
  public:

  // This constructor designs a new filter with the specified parameters, and initializes the weights array.
  FilterBank(const bool verbose, const unsigned taps, const unsigned channels, const WindowType windowType);

  // This constructor creates a filterbank from an already existing set of weights.
  FilterBank(const bool verbose, const unsigned taps, const unsigned channels, float* weights);

  unsigned getNrTaps();

  float* getWeights(unsigned channel);


private:
  // Hamming window function
  void hamming(const unsigned n, double* d);

  // Blackman window function
  void blackman(const unsigned n, double* d);

  // Gaussian window function
  void gaussian(const int n, const double a, double* d);

  // Kaiser window function
  void kaiser(const int n, const double beta, double* d);

  // helper functions
  double besselI0(const double x);
  unsigned next_power_of_2(const unsigned n);
  void interpolate(const double* x, const double* y, const unsigned xlen, const unsigned n, double* result);
  void generate_fir_filter(const unsigned n, const double w, const double* window, double* result);
  void generate_filter();


  // The window used for generating the filter, default is KAISER.
  WindowType itsWindowType;

  const unsigned itsNrTaps;
  const unsigned itsNrChannels;
  const bool itsVerbose;

  // Store the weights in a multiarray, since both the number of channels are not known at compile time.
  boost::multi_array<float, 2, AlignedStdAllocator<float, 32> > weights; // [nrChannels][taps];


#if USE_ORIGINAL_FILTER
  static const float origWeights[256][16];
#endif

};


inline unsigned FilterBank::getNrTaps()
{
	return itsNrTaps;
}


inline float* FilterBank::getWeights(unsigned channel)
{
	return weights[channel].origin();
}

} // namespace RTCP
} // namespace LOFAR

#endif
