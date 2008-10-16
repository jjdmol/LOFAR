//#  FIR.h: header files for CN assembly
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

#ifndef LOFAR_CNPROC_FIR_ASM_H
#define LOFAR_CNPROC_FIR_ASM_H

#if defined HAVE_BGL || defined HAVE_BGP
#include <Interface/Config.h>

#if defined HAVE_BGL
#include <rts.h>
#endif

namespace LOFAR {
namespace RTCP {

struct phase_shift {
  dcomplex v0, dv;
};


template <typename SAMPLE_TYPE> extern void _filter(unsigned nrChannels,
						    const float weights[NR_TAPS],
						    const SAMPLE_TYPE samples[],
						    fcomplex out[],
						    int nr_samples_div_16);

extern "C" {
  void _transpose_4x8(fcomplex *out,
		      const fcomplex *in,
		      int length,
		      int input_stride,
		      int output_stride);

  void _phase_shift_and_transpose(fcomplex *out,
				  const fcomplex *in,
				  const struct phase_shift *,
				  int stride,
				  unsigned nrChannels);

  void _fast_memcpy(void *dst, const void *src, size_t bytes);
  void _memzero(void *dst, size_t bytes); // bytes must be multiple of 128
  void _prefetch(const void *src, size_t count, size_t stride);

  extern struct {
    unsigned nr_taps;
    unsigned nr_polarizations;
  } _FIR_constants_used;

#if defined HAVE_BGL
  void _cn_mutex_lock(CN_Mutex *), _cn_mutex_unlock(CN_Mutex *);
#endif

  unsigned long long _rdtsc();
}

} // namespace RTCP
} // namespace LOFAR

#endif
#endif
