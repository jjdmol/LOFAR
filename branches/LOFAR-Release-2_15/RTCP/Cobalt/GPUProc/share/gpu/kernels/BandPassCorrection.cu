//# BandPassCorrection.cu
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
 * This file contains a CUDA implementation of the GPU kernel for the 
 * BandPassCorrection. It transposes the data: The FFT produces
 * for each sample X channels in the fastest dimension. The channels and samples
 * are transposed to allow faster processing in later stages.
 * The samples will end up in the fastest dimension ( the time line).
 *
 * @attention The following pre-processor variables must be supplied when
 * compiling this program. Please take the pre-conditions for these variables
 * into account:
 * - @c NR_POLARIZATIONS: 2
 * - @c NR_STATIONS: > 0
 * - @c NR_CHANNELS_1: a multiple of 16 
 * - @c NR_CHANNELS_2: > 0 
 * - @c NR_SAMPLES_PER_CHANNEL: > a multiple of 16
 * - @c DO_BANDPASS_CORRECTION: if defined, perform bandpass correction
 */

#include "gpu_math.cuh"

#if !(NR_POLARIZATIONS == 2)
#error Precondition violated: NR_POLARIZATIONS == 2
#endif

#if !(NR_STATIONS > 0)
#error Precondition violated: NR_STATIONS > 0
#endif

#if !(NR_CHANNELS_1 > 0 && NR_CHANNELS_1 % 16 == 0)
#error Precondition violated: NR_CHANNELS_1 > 0 && NR_CHANNELS_1 % 16 == 0
#endif

#if !(NR_CHANNELS_2 > 0)
#error Precondition violated: NR_CHANNELS_2 > 0
#endif

#if !(NR_SAMPLES_PER_CHANNEL > 0)
#error Precondition violated: NR_SAMPLES_PER_CHANNEL > 0
#endif

typedef  fcomplex (* OutputDataType)[NR_STATIONS][NR_CHANNELS_1 * NR_CHANNELS_2][NR_SAMPLES_PER_CHANNEL][NR_POLARIZATIONS];
typedef  fcomplex (* InputDataType)[NR_STATIONS][NR_POLARIZATIONS][NR_CHANNELS_1][NR_SAMPLES_PER_CHANNEL][NR_CHANNELS_2];
typedef  const float (* BandPassFactorsType)[NR_CHANNELS_1 * NR_CHANNELS_2];

/**
 * This kernel performs on the input data:
 * - If the preprocessor variable \c DO_BANDPASS_CORRECTION is defined, apply a
 *   bandpass correction to compensate for the errors introduced by the
 *   polyphase filter that produced the subbands. This error is deterministic,
 *   hence it can be fully compensated for.
 * - Transpose the data so that the samples for each channel are placed
 *   consecutively in memory with both polarization next to each other.
 * - Note: This kernel is optimized for performance in dims samples and channel_1
 *   Previous version was optimized for channel_2 (still supported)
 *   
 *
 * @param[out] correctedDataPtr    pointer to output data of ::OutputDataType,
 *                                 a 4D array  [station][channels1 * channels2][samples][pol]
 *                                 of ::complex (2 complex polarizations)
 * @param[in]  intputDataPtr     pointer to input data; 
 *                               5D array  [station][pol][channels1][samples][channels2]
 * @param[in]  bandPassFactorsPtr  pointer to bandpass correction data of
 *                                 ::BandPassFactorsType, a 1D array [channels1 * channels2] of
 *                                 float, containing bandpass correction factors
 */
extern "C" {
__global__ void bandPassCorrection( fcomplex * outputDataPtr,
                                 const fcomplex * inputDataPtr,
                                 const float * bandPassFactorsPtr)
{ 
  OutputDataType outputData = (OutputDataType) outputDataPtr;
  InputDataType inputData   = (InputDataType)  inputDataPtr;

#if defined DO_BANDPASS_CORRECTION
  // Band pass to apply to the channels  
  BandPassFactorsType bandPassFactors = (BandPassFactorsType) bandPassFactorsPtr;
#endif

  // fasted dims
  unsigned sample = blockIdx.x * blockDim.x + threadIdx.x;
  unsigned idx_channel1 = blockIdx.y * blockDim.y + threadIdx.y;
  unsigned chan2 = blockIdx.z * blockDim.z + threadIdx.z;
  
  for (unsigned station = 0; station < NR_STATIONS; ++station)
  {
    // Read from global memory in the quickest dimension (optimal)
    fcomplex sampleX = (*inputData)[station][0][idx_channel1][sample][chan2];
    fcomplex sampleY = (*inputData)[station][1][idx_channel1][sample][chan2];
    unsigned chan_index = idx_channel1 * NR_CHANNELS_2 + chan2;

#if defined DO_BANDPASS_CORRECTION
    float weight((*bandPassFactors)[chan_index]);
    sampleX.x *= weight;
    sampleX.y *= weight;
    sampleY.x *= weight;
    sampleY.y *= weight;
#endif

    (*outputData)[station][chan_index][sample][0] = sampleX; 
    (*outputData)[station][chan_index][sample][1] = sampleY; 
  }
}
}


