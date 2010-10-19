#ifndef BGL_FIR_H
#define BGL_FIR_H

#include <TFC_Interface/TFC_Config.h>

namespace LOFAR {

struct phase_shift {
  union {
    dcomplex c;
    struct {
      double real, imag;
    } d;
  } v0, dv;
};


extern "C" {
  void _filter(fcomplex delayLine[NR_TAPS],
               const float weights[NR_TAPS],
               const i16complex samples[],
               fcomplex out[],
               int nr_samples_div_16);

  void _transpose_8x4(i16complex *out, const i16complex *in);

  void _transpose_4x8(fcomplex *out,
                      const fcomplex *in,
                      int length,
                      int input_stride,
                      int output_stride);

  void _phase_shift_and_transpose(fcomplex *out,
                                  const fcomplex *in,
                                  const struct phase_shift[2]);

  void _fast_memcpy(void *dst, const void *src, size_t bytes);
  void _fft_16(const fcomplex *in, int in_stride, fcomplex *out, int out_stride);
};

}
#endif
