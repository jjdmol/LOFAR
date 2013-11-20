//# DelayAndBandPass.cu
//# Copyright (C) 2012-2013  ASTRON (Netherlands Institute for Radio Astronomy)
//# P.O. Box 2, 7990 AA Dwingeloo, The Netherlands
//#
//# This file is part of the LOFAR software suite.
//# The LOFAR software suite is free software: you can redistribute it and/or
//# modify it under the terms of the GNU General Public License as published
//# by the Free Software Foundation, either version 3 of the License, or
//# (at your option) any later version.
//#
//# The LOFAR software suite is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License along
//# with the LOFAR software suite. If not, see <http://www.gnu.org/licenses/>.
//#
//# $Id$

/** @file
 * This file contains a CUDA implementation of the GPU kernel for the delay
 * and bandpass correction. It can also transpose the data (pol to dim 0).
 *
 * Usually, this kernel will be run after the polyphase filter kernel FIR.cl. In
 * that case, the input data for this kernel is already in floating point format
 * (@c NR_CHANNELS > 1). However, if this kernel is the first in row, then the
 * input data is still in integer format (@c NR_CHANNELS == 1), and this kernel
 * needs to do the integer-to-float conversion. If we do BANDPASS_CORRECTION
 * (implies NR_CHANNELS > 1), then we typically also want to transpose the pol
 * dim to the stride 1 dim (@c DO_TRANSPOSE).
 *
 * @attention The following pre-processor variables must be supplied when
 * compiling this program. Please take the pre-conditions for these variables
 * into account:
 * - @c NR_CHANNELS: 1 or a multiple of 16
 * - if @c NR_CHANNELS == 1 (input data is in integer format):
 *   - @c NR_BITS_PER_SAMPLE: 8 or 16
 *   - @c NR_SAMPLES_PER_SUBBAND: a multiple of 16
 * - if @c NR_CHANNELS > 1 (input data is in floating point format):
 *   - @c NR_SAMPLES_PER_CHANNEL: a multiple of 16
 * - @c NR_SAPS: > 0
 * - @c NR_POLARIZATIONS: 2
 * - @c SUBBAND_BANDWIDTH: a multiple of @c NR_CHANNELS
 *
 * - @c DELAY_COMPENSATION: defined or not
 * - @c BANDPASS_CORRECTION: defined or not
 * - @c DO_TRANSPOSE: defined or not
 */

#include "gpu_math.cuh"

#include "IntToFloat.cuh"

#include <stdio.h>


#if NR_CHANNELS == 1
   //# #chnl==1 && BANDPASS_CORRECTION is rejected on the CPU early, (TODO)
   //# but once here, don't do difficult and adjust cleanly here.
#  undef BANDPASS_CORRECTION
#endif

#if defined DO_TRANSPOSE
typedef  fcomplex (* OutputDataType)[NR_STATIONS][NR_CHANNELS][NR_SAMPLES_PER_CHANNEL][NR_POLARIZATIONS];
#else
typedef  fcomplex (* OutputDataType)[NR_STATIONS][NR_POLARIZATIONS][NR_CHANNELS][NR_SAMPLES_PER_CHANNEL];
#endif

//# TODO: Unify #dims in input type to 4: [NR_SAMPLES_PER_SUBBAND] -> [NR_SAMPLES_PER_CHANNEL][NR_CHANNELS] (see kernel test)
//#       It is technically incorrect, but different dims for the same input type is a real pain to use/supply.
//#       Also unify order of #chn, #sampl to [NR_SAMPLES_PER_CHANNEL][NR_CHANNELS]
#if NR_CHANNELS == 1
#  if NR_BITS_PER_SAMPLE == 16
typedef  short_complex rawSampleType;
typedef  short_complex (* InputDataType)[NR_STATIONS][NR_SAMPLES_PER_SUBBAND][NR_POLARIZATIONS];
#  elif NR_BITS_PER_SAMPLE == 8
typedef  char_complex  rawSampleType;
typedef  char_complex  (* InputDataType)[NR_STATIONS][NR_SAMPLES_PER_SUBBAND][NR_POLARIZATIONS];
#  else
#    error unsupported NR_BITS_PER_SAMPLE
#  endif
#else
typedef  fcomplex (* InputDataType)[NR_STATIONS][NR_POLARIZATIONS][NR_SAMPLES_PER_CHANNEL][NR_CHANNELS];
#endif
typedef  const double (* DelaysType)[NR_SAPS][NR_STATIONS][NR_POLARIZATIONS]; // 2 Polarizations; in seconds
typedef  const double2 (* PhaseOffsetsType)[NR_STATIONS]; // 2 Polarizations; in radians
typedef  const float (* BandPassFactorsType)[NR_CHANNELS];

inline __device__ fcomplex sincos_f2f(float phi)
{
  float2 r;

  sincosf(phi, &r.y, &r.x);
  return r;
}

inline __device__ fcomplex sincos_d2f(double phi)
{
  double s, c;

  sincos(phi, &s, &c);
  return make_float2(c, s);
}

/**
 * This kernel performs (up to) three operations on the input data:
 * - Apply a fine delay by doing a per channel phase correction.
 * - Apply a bandpass correction to compensate for the errors introduced by the
 *   polyphase filter that produced the subbands. This error is deterministic,
 *   hence it can be fully compensated for.
 * - Transpose the data so that the time slices for each channel are placed
 *   consecutively in memory.
 *
 * @param[out] correctedDataPtr    pointer to output data of ::OutputDataType,
 *                                 a 3D array [station][channel][sample][complex]
 *                                 of ::complex (2 complex polarizations)
 * @param[in]  filteredDataPtr     pointer to input data; this can either be a
 *                                 4D array [station][polarization][sample][channel][complex]
 *                                 of ::fcomplex, or a 2D array [station][subband][complex]
 *                                 of ::short_complex2 or ::char_complex2,
 *                                 depending on the value of @c NR_CHANNELS
 * @param[in]  subbandFrequency    center freqency of the subband
 * @param[in]  beam                index number of the beam
 * @param[in]  delaysAtBeginPtr    pointer to delay data of ::DelaysType,
 *                                 a 2D array [beam][station] of float2 (real:
 *                                 2 polarizations), containing delays in
 *                                 seconds at begin of integration period
 * @param[in]  delaysAfterEndPtr   pointer to delay data of ::DelaysType,
 *                                 a 2D array [beam][station] of float2 (real:
 *                                 2 polarizations), containing delays in
 *                                 seconds after end of integration period
 * @param[in]  phaseOffsetsPtr     pointer to phase offset data of
 *                                 ::PhaseOffsetsType, a 1D array [station] of
 *                                 float2 (real: 2 polarizations), containing
 *                                 phase offsets in radians
 * @param[in]  bandPassFactorsPtr  pointer to bandpass correction data of
 *                                 ::BandPassFactorsType, a 1D array [channel] of
 *                                 float, containing bandpass correction factors
 */

extern "C" {
 __global__ void applyDelaysAndCorrectBandPass( fcomplex * correctedDataPtr,
                                                const fcomplex * filteredDataPtr,
                                                double subbandFrequency,
                                                unsigned beam,
                                                const double * delaysAtBeginPtr,
                                                const double * delaysAfterEndPtr,
                                                const double * phaseOffsetsPtr,
                                                const float * bandPassFactorsPtr)
{
  OutputDataType outputData = (OutputDataType) correctedDataPtr;
  InputDataType inputData   = (InputDataType)  filteredDataPtr;

  /* The x dimension is 256 wide. */
  const unsigned major   = (blockIdx.x * blockDim.x + threadIdx.x) / 16;
  const unsigned minor   = (blockIdx.x * blockDim.x + threadIdx.x) % 16;

  /* The y dimension is NR_CHANNELS/16 wide (or 1 if NR_CHANNELS == 1) */
  const unsigned channel = NR_CHANNELS == 1 ? 0 : (blockIdx.y * blockDim.y + threadIdx.y) * 16 + minor;

  /* The z dimension is NR_STATIONS wide. */
  const unsigned station =  blockIdx.z * blockDim.z + threadIdx.z;

  const unsigned timeStart = NR_CHANNELS == 1 ? threadIdx.x : major;
  const unsigned timeInc   = NR_CHANNELS == 1 ? blockDim.x  : 16;

#if defined BANDPASS_CORRECTION
  BandPassFactorsType bandPassFactors = (BandPassFactorsType) bandPassFactorsPtr;

  float weight((*bandPassFactors)[channel]);
#endif

#if defined DELAY_COMPENSATION
  DelaysType delaysAtBegin  = (DelaysType) delaysAtBeginPtr;
  DelaysType delaysAfterEnd = (DelaysType) delaysAfterEndPtr;
  PhaseOffsetsType phaseOffsets = (PhaseOffsetsType) phaseOffsetsPtr;

  /*
   * Delay compensation means rotating the phase of each sample BACK.
   *
   * n     = channel number (f.e. 0 .. 255)
   * f_n   = channel frequency of channel n
   * f_ref = base frequency of subband (f.e. 200 MHz)
   * df    = delta frequency of 1 channel (f.e. 768 Hz)
   *
   * f_n := f_ref + n * df
   *
   * m      = sample number (f.e. 0 .. 3071)
   * tau_m  = delay at sample m
   * tau_0  = delayAtBegin (f.e. -2.56us .. +2.56us)
   * dtau   = delta delay for 1 sample (f.e. <= 1.6ns)
   *
   * tau_m := tau_0 + m * dtau
   *
   * Then, the required phase shift is:
   *
   *   phi_mn = -2 * pi * f_n * tau_m
   *          = -2 * pi * (f_ref + n * df) * (tau_0 + m * dtau)
   *          = -2 * pi * (f_ref * tau_0 + f_ref * m * dtau + tau_0 * n * df + m * n * df * dtau)
   *                       -------------   ----------------   --------------   -----------------
   *                           O(100)           O(0.1)            O(0.01)          O(0.001)
   *
   * Finally, we also want to correct for fixed phase offsets per station,
   * as given by the phaseOffsets array.
   */

  const double frequency = NR_CHANNELS == 1
    ? subbandFrequency
    : subbandFrequency - 0.5 * SUBBAND_BANDWIDTH + channel * (SUBBAND_BANDWIDTH / NR_CHANNELS);

  const double2 delayAtBegin  = make_double2((*delaysAtBegin) [beam][station][0], (*delaysAtBegin) [beam][station][1]);
  const double2 delayAfterEnd = make_double2((*delaysAfterEnd)[beam][station][0], (*delaysAfterEnd)[beam][station][1]);

  // Calculate the angles to rotate for for the first and (beyond the) last sample.
  //
  // We need to undo the delay, so we rotate BACK, resulting in a negative constant factor.
  const double2 phiAtBegin  = -2.0 * M_PI * frequency * delayAtBegin  + (*phaseOffsets)[station];
  const double2 phiAfterEnd = -2.0 * M_PI * frequency * delayAfterEnd + (*phaseOffsets)[station];
#endif

  for (unsigned time = timeStart; time < NR_SAMPLES_PER_CHANNEL; time += timeInc)
  {
#if NR_CHANNELS == 1
    const rawSampleType sampleXraw = (*inputData)[station][time][0];
    fcomplex sampleX = make_float2(convertIntToFloat(sampleXraw.x),
                                   convertIntToFloat(sampleXraw.y));
    const rawSampleType sampleYraw = (*inputData)[station][time][1];
    fcomplex sampleY = make_float2(convertIntToFloat(sampleYraw.x),
                                   convertIntToFloat(sampleYraw.y));
#else
    fcomplex sampleX = (*inputData)[station][0][time][channel];
    fcomplex sampleY = (*inputData)[station][1][time][channel];
#endif

#if defined DELAY_COMPENSATION    
    // Offset of this sample between begin and end.
    const double timeOffset = double(time) / NR_SAMPLES_PER_CHANNEL;

    // Interpolate the required phase rotation for this sample.
    //
    // Single precision angle + sincos is measured to be good enough (2013-11-20).
    // Note that we do the interpolation in double precision still.
    // We can afford to if we keep this kernel memory bound.
    const float2 phi  = make_float2( phiAtBegin.x  * (1.0 - timeOffset)
                                   + phiAfterEnd.x *        timeOffset,
                                     phiAtBegin.y  * (1.0 - timeOffset)
                                   + phiAfterEnd.y *        timeOffset );

    sampleX = sampleX * sincos_f2f(phi.x);
    sampleY = sampleY * sincos_f2f(phi.y);
#endif

#if defined BANDPASS_CORRECTION
    sampleX.x *= weight;
    sampleX.y *= weight;
    sampleY.x *= weight;
    sampleY.y *= weight;
#endif

// Support all variants of NR_CHANNELS and DO_TRANSPOSE for testing etc.
// Transpose: data order is [station][channel][time][pol]
#if NR_CHANNELS > 1 && defined DO_TRANSPOSE
    __shared__ fcomplex tmp[16][17][2]; // one too wide to avoid bank-conflicts on read

    tmp[major][minor][0] = sampleX;
    tmp[major][minor][1] = sampleY;
    __syncthreads();
    (*outputData)[station][channel - minor + major][time - major + minor][0] = tmp[minor][major][0];
    (*outputData)[station][channel - minor + major][time - major + minor][1] = tmp[minor][major][1];
    __syncthreads();
#elif NR_CHANNELS == 1 && defined DO_TRANSPOSE
    (*outputData)[station][0][time][0] = sampleX;
    (*outputData)[station][0][time][1] = sampleY;

// No transpose: data order is [station][pol][channel][time]
#elif NR_CHANNELS > 1
    (*outputData)[station][0][channel][time] = sampleX;
    (*outputData)[station][1][channel][time] = sampleY;
#else
    (*outputData)[station][0][0][time] = sampleX;
    (*outputData)[station][1][0][time] = sampleY;
#endif
  }
}
}

