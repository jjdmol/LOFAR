//#  FIR.h: header files for BGL assembly
//#
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//#  This program is free software; you can redistribute it and/or modify
//#  it under the terms of the GNU General Public License as published by
//#  the Free Software Foundation; either version 2 of the License, or
//#  (at your option) any later version.
//#
//#  This program is distributed in the hope that it will be useful,
//#  but WITHOUT ANY WARRANTY; without even the implied warranty of
//#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//#  GNU General Public License for more details.
//#
//#  You should have received a copy of the GNU General Public License
//#  along with this program; if not, write to the Free Software
//#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#
//#  $Id$

#ifndef LOFAR_APPL_CEP_CS1_CS1_BGL_PROC_FIR_ASM_H
#define LOFAR_APPL_CEP_CS1_CS1_BGL_PROC_FIR_ASM_H

#if defined HAVE_BGL || defined HAVE_BGP
#include <CS1_Interface/CS1_Config.h>

#if defined HAVE_BGL
#include <rts.h>
#endif

namespace LOFAR
{
namespace CS1
{

struct phase_shift {
  dcomplex v0, dv;
};


extern "C" {
  void _filter(fcomplex delayLine[NR_TAPS],
	       const float weights[NR_TAPS],
	       const INPUT_SAMPLE_TYPE samples[],
	       fcomplex out[],
	       int nr_samples_div_16);

  void _transpose_4x8(fcomplex *out,
		      const fcomplex *in,
		      int length,
		      int input_stride,
		      int output_stride);

  void _phase_shift_and_transpose(fcomplex *out,
				  const fcomplex *in,
				  const struct phase_shift *,
				  int stride);

  void _fast_memcpy(void *dst, const void *src, size_t bytes);
  void _memzero(void *dst, size_t bytes); // bytes must be multiple of 128
  void _prefetch(const void *src, size_t count, size_t stride);

  extern struct {
    unsigned nr_bits_per_sample;
    unsigned nr_subband_channels;
    unsigned nr_taps;
    unsigned nr_polarizations;
  } _FIR_constants_used;

#if defined HAVE_BGL
  void _bgl_mutex_lock(BGL_Mutex *), _bgl_mutex_unlock(BGL_Mutex *);
#endif

  unsigned long long _rdtsc();

#if NR_BITS_PER_SAMPLE == 4
  extern fcomplex _FIR_fp_table[16][16];
#elif NR_BITS_PER_SAMPLE == 8
  extern fcomplex _FIR_fp_table[256][256];
#elif NR_BITS_PER_SAMPLE == 16
  extern float _FIR_fp_table[65536];
#endif
};

} // namespace CS1
} // namespace LOFAR

#endif
#endif
