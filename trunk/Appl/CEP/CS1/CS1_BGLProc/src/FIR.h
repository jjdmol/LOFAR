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

#ifndef LOFAR_APPL_CEP_CS1_CS1_BGL_PROC_FIR_H
#define LOFAR_APPL_CEP_CS1_CS1_BGL_PROC_FIR_H

#if defined HAVE_BGL
#include <CS1_Interface/bitset.h>
#include <CS1_Interface/CS1_Config.h>
#include <CS1_Interface/DH_Subband.h>

#include <rts.h>

namespace LOFAR
{
namespace CS1
{

struct phase_shift {
  dcomplex v0, dv;
};


typedef bitset<NR_SUBBAND_CHANNELS> inputFlagsType[NR_STATIONS][NR_TAPS - 1 + NR_SAMPLES_PER_INTEGRATION];
typedef bitset<NR_SAMPLES_PER_INTEGRATION> flagsType[NR_STATIONS];

extern "C" {
  void _filter(fcomplex delayLine[NR_TAPS],
	       const float weights[NR_TAPS],
	       const DH_Subband::SampleType samples[],
	       fcomplex out[],
	       int nr_samples_div_16);

  // void _compute_flags(const inputFlagsType *input, flagsType *flags);

  void _transpose_4x8(fcomplex *out,
		      const fcomplex *in,
		      int length,
		      int input_stride,
		      int output_stride);

  void _phase_shift_and_transpose(fcomplex *out,
				  const fcomplex *in,
				  const struct phase_shift *);

  void _fast_memcpy(void *dst, const void *src, size_t bytes);
  void _memzero(void *dst, size_t bytes); // bytes must be multiple of 128
  void _prefetch(const void *src, size_t count, size_t stride);

  struct {
    unsigned input_type;
    unsigned nr_stations;
    unsigned nr_samples_per_integration;
    unsigned nr_subband_channels;
    unsigned nr_polarizations;
  } _FIR_constants_used;

  void _bgl_mutex_lock(BGL_Mutex *), _bgl_mutex_unlock(BGL_Mutex *);
  unsigned long long _rdtsc();
};

} // namespace CS1
} // namespace LOFAR

#endif
#endif
