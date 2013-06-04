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
 * This file contains an Cuda implementation of the GPU kernel for the delay
 * and bandpass correction.
 *
 * Usually, this kernel will be run after the polyphase filter kernel FIR.cl. In
 * that case, the input data for this kernel is already in floating point format
 * (@c NR_CHANNELS > 1). However, if this kernel is the first in row, then the
 * input data is still in integer format (@c NR_CHANNELS == 1), and this kernel
 * needs to do the integer-to-float conversion.
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
 * - @c NR_POLARIZATIONS: 2
 * - @c SUBBAND_WIDTH: a multiple of @c NR_CHANNELS
 */

#include <cuda_runtime.h>
#include <cuda.h>

#include "complex.cuh"

#if NR_CHANNELS == 1
#undef BANDPASS_CORRECTION  // TODO: Should this be an assert: this result in unexpected behaviour
#endif

typedef LOFAR::Cobalt::gpu::complex<float> complexfloat;
typedef LOFAR::Cobalt::gpu::complex<short> complexshort;
typedef LOFAR::Cobalt::gpu::complex<char> complexchar;
typedef  complexfloat (* OutputDataType)[NR_STATIONS][NR_CHANNELS][NR_SAMPLES_PER_CHANNEL][NR_POLARIZATIONS];
#if NR_CHANNELS == 1
#if NR_BITS_PER_SAMPLE == 16
typedef  complexshort (* InputDataType)[NR_STATIONS][NR_SAMPLES_PER_SUBBAND][NR_POLARIZATIONS];
#elif NR_BITS_PER_SAMPLE == 8
typedef  complexchar (* InputDataType)[NR_STATIONS][NR_SAMPLES_PER_SUBBAND][NR_POLARIZATIONS];
#else
#error unsupport NR_BITS_PER_SAMPLE
#endif
#else
typedef  complexfloat (* InputDataType)[NR_STATIONS][NR_POLARIZATIONS][NR_SAMPLES_PER_CHANNEL][NR_CHANNELS];
#endif
typedef  const float (* DelaysType)[NR_BEAMS][NR_STATIONS][COMPLEX]; // 2 Polarizations; in seconds
typedef  const float (* PhaseOffsetsType)[NR_STATIONS][COMPLEX]; // 2 Polarizations; in radians
typedef  const float (* BandPassFactorsType)[NR_CHANNELS];

/**
 * This kernel perfroms three operations on the input data:
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

#if NR_CHANNELS == 1
#error Support for 1 channel per subband has not yet been implemented due to missing int to float conversion routines.
#endif

extern "C" {
 __global__ void applyDelaysAndCorrectBandPass( complexfloat * correctedDataPtr,
                                                const complexfloat * filteredDataPtr,
                                                float subbandFrequency,
                                                unsigned beam,
                                                const float * delaysAtBeginPtr,
                                                const float * delaysAfterEndPtr,
                                                const float * phaseOffsetsPtr,
                                                const float * bandPassFactorsPtr)
{
  OutputDataType outputData = (OutputDataType) correctedDataPtr;
  InputDataType inputData = (InputDataType) filteredDataPtr;
  DelaysType delaysAtBegin = (DelaysType) delaysAtBeginPtr;
  DelaysType delaysAfterEnd = (DelaysType) delaysAfterEndPtr;
  PhaseOffsetsType phaseOffsets = (PhaseOffsetsType) phaseOffsetsPtr;
#if NR_CHANNELS > 1
  BandPassFactorsType bandPassFactors = (BandPassFactorsType) bandPassFactorsPtr;

  complexfloat tmp[16][17][2]; // one too wide to allow coalesced reads

  unsigned major = (blockIdx.x * blockDim.x + threadIdx.x) / 16;
  unsigned minor = (blockIdx.x * blockDim.x + threadIdx.x) % 16;
  unsigned channel = (blockIdx.y * blockDim.y + threadIdx.y) * 16;
#endif
  unsigned station = blockIdx.z * blockDim.z + threadIdx.z;

#if defined DELAY_COMPENSATION
#if NR_CHANNELS == 1
  float frequency = subbandFrequency;
#else
  float frequency = subbandFrequency - .5f * SUBBAND_BANDWIDTH + (channel + minor) * (SUBBAND_BANDWIDTH / NR_CHANNELS);
#endif
  float2 delayAtBegin = make_float2((*delaysAtBegin)[beam][station][0], (*delaysAtBegin)[beam][station][1]);
  float2 delayAfterEnd = make_float2((*delaysAfterEnd)[beam][station][0], (*delaysAfterEnd)[beam][station][1]);


  // Convert the fraction of sample duration (delayAtBegin/delayAfterEnd) to fractions of a circle.
  // Because we `undo' the delay, we need to rotate BACK.
  float pi2 = -2 * 3.1415926535f;
  float2 phiBegin = make_float2(pi2 * delayAtBegin.x, pi2 * delayAtBegin.y) ;
  float2 phiEnd = make_float2(pi2 * delayAfterEnd.x, pi2 * delayAfterEnd.y) ;

  float2 deltaPhi = make_float2((phiEnd.x - phiBegin.x) / NR_SAMPLES_PER_CHANNEL,
                                (phiEnd.y - phiBegin.y) / NR_SAMPLES_PER_CHANNEL);   
  
#if NR_CHANNELS == 1
  float2 myPhiBegin = make_float2(
                        (phiBegin.x + float(threadIdx.x) * deltaPhi.x) * frequency + (*phaseOffsets)[station][0],
                        (phiBegin.y + float(threadIdx.x) * deltaPhi.y) * frequency + (*phaseOffsets)[station][1]);
  float2 myPhiDelta = make_float2(
                         float(blockDim.x) * deltaPhi.x * frequency.x,
                         float(blockDim.x) * deltaPhi.y * frequency.y);
#else
  float2 myPhiBegin = make_float2(
                          (phiBegin.x + float(major) * deltaPhi.x) * frequency + (*phaseOffsets)[station][0],
                          (phiBegin.y + float(major) * deltaPhi.y) * frequency + (*phaseOffsets)[station][1]);
  float2 myPhiDelta = make_float2(16.0f * deltaPhi.x * frequency,
                                  16.0f * deltaPhi.y * frequency); // Magic constant: 16 is the time step we take in the samples
#endif

  complexfloat vX = LOFAR::Cobalt::gpu::exp(complexfloat(myPhiBegin.x));  // This cast might be costly
  complexfloat vY = LOFAR::Cobalt::gpu::exp(complexfloat(myPhiBegin.y));
  complexfloat dvX = LOFAR::Cobalt::gpu::exp(complexfloat(myPhiDelta.x));
  complexfloat dvY = LOFAR::Cobalt::gpu::exp(complexfloat(myPhiDelta.y));
#endif

#if defined BANDPASS_CORRECTION
  complexfloat weight((*bandPassFactors)[channel + minor]);
#endif

#if defined DELAY_COMPENSATION && defined BANDPASS_CORRECTION
  vX *= weight;
  vY *= weight;
#endif

#if NR_CHANNELS == 1
  for (unsigned time = threadIdx.x; time < NR_SAMPLES_PER_SUBBAND; time += blockDim.x) 
  {
    complexfloat sampleX = (*inputData)[station][time][0];
    complexfloat sampleY = (*inputData)[station][time][1];
#else
  for (unsigned time = 0; time < NR_SAMPLES_PER_CHANNEL; time += 16) 
  {
    complexfloat sampleX = (*inputData)[station][0][time + major][channel + minor];    
    complexfloat sampleY = (*inputData)[station][1][time + major][channel + minor];   
#endif

#if defined DELAY_COMPENSATION    
    sampleX = sampleX * vX;
    sampleY = sampleY * vY;
    vX = vX * dvX;    // The calculation are with exponentional complex for: multiplication for correct phase shift
    vY = vY * dvY;
#elif defined BANDPASS_CORRECTION
    sampleX *= weight;
    sampleY *= weight;
#endif

#if NR_CHANNELS == 1
    (*outputData)[station][0][time][0] = sampleX;
    (*outputData)[station][0][time][1] = sampleY;
#else
    tmp[major][minor][0] = sampleX;
    tmp[major][minor][1] = sampleY;
    __syncthreads(); // Wait till all threads are here: we do a transform of the data
    (*outputData)[station][channel + major][time + minor][0] = tmp[minor][major][0];   
    (*outputData)[station][channel + major][time + minor][1] = tmp[minor][major][1];
    __syncthreads();

#endif
  }
}
}

