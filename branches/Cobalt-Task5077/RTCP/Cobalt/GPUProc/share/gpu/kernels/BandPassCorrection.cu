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
 * BandPassCorrection correction. It can also transposes the data: The FFT produces
 * for each sample x channels in the fastest dimension. The channels and samles
 * are transposed. The samples will end up in the fastest dimension.
 *
 * @attention The following pre-processor variables must be supplied when
 * compiling this program. Please take the pre-conditions for these variables
 * into account:
 * - @c NR_POLARIZATIONS: 2
 * - @c NR_STATIONS: > 0
 * - @c NR_CHANNELS_1: > 1 
 * - @c NR_CHANNELS_2: a multiple of 16
 * - @c NR_SAMPLES_PER_CHANNEL: > a multiple of 16
 * - @c NR_BITS_PER_SAMPLE: 8 or 16
 */

#include "gpu_math.cuh"
#include "IntToFloat.cuh"

#if !(NR_POLARIZATIONS == 2)
#error Precondition violated: NR_POLARIZATIONS == 2
#endif

#if !(NR_STATIONS > 0)
#error Precondition violated: NR_STATIONS > 0
#endif

#if !(NR_CHANNELS_1 > 1)
#error Precondition violated: NR_CHANNELS_1 > 1
#endif

#if !(NR_CHANNELS_2 % 16 == 0)
#error Precondition violated: NR_CHANNELS_2 % 16 
#endif

#if !(NR_SAMPLES_PER_CHANNEL > 0 && NR_SAMPLES_PER_CHANNEL % 16 == 0)
#error Precondition violated: NR_SAMPLES_PER_CHANNEL > 0 && NR_SAMPLES_PER_CHANNEL % 16 == 0
#endif

#if !(NR_BITS_PER_SAMPLE == 16 || NR_BITS_PER_SAMPLE == 8)
#error Precondition violated: NR_BITS_PER_SAMPLE == 16 || NR_BITS_PER_SAMPLE == 8
#endif


typedef  fcomplex (* OutputDataType)[NR_STATIONS][NR_CHANNELS_1 * NR_CHANNELS_2][NR_SAMPLES_PER_CHANNEL][NR_POLARIZATIONS];
typedef  fcomplex (* InputDataType)[NR_STATIONS][NR_POLARIZATIONS][NR_CHANNELS_1][NR_SAMPLES_PER_CHANNEL][NR_CHANNELS_2];
typedef  const float (* BandPassFactorsType)[NR_CHANNELS_1 * NR_CHANNELS_2];

/**
 * This kernel performs on the input data:
 * - Apply a bandpass correction to compensate for the errors introduced by the
 *   polyphase filter that produced the subbands. This error is deterministic,
 *   hence it can be fully compensated for.
 * - Transpose the data so that the samples for each channel are placed
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
 * @param[in]  bandPassFactorsPtr  pointer to bandpass correction data of
 *                                 ::BandPassFactorsType, a 1D array [channel] of
 *                                 float, containing bandpass correction factors
 */
#define SHARED
extern "C" {
__global__ void bandPassCorrection( fcomplex * correctedDataPtr,
                                 const fcomplex * filteredDataPtr,
                                 const float * bandPassFactorsPtr)
{
  
  OutputDataType outputData = (OutputDataType) correctedDataPtr;
  InputDataType inputData   = (InputDataType)  filteredDataPtr;

  // Band pass to apply to the channels  
  BandPassFactorsType bandPassFactors = (BandPassFactorsType) bandPassFactorsPtr;
  
  // fasted dims
  unsigned chan2        = blockIdx.x * blockDim.x + threadIdx.x;
  unsigned sample       = blockIdx.y * blockDim.y + threadIdx.y;
  unsigned station      = blockIdx.z * blockDim.z + threadIdx.z;

  // Shared memory to perform a transpose in shared memory
  // one too wide to avoid bank-conflicts on read
  // 16 by 16 limitation for the channels2 and samples per channel are caused by the
  // dimensions of this array
  // TODO: Increasing to 32 x 32 allows for a speedup of 13%
  __shared__ fcomplex tmp[16][16 + 1][2];

  for (unsigned idx_channel1 = 0; idx_channel1 < NR_CHANNELS_1; ++idx_channel1)
  {
    unsigned combined_channel = idx_channel1 * NR_CHANNELS_2 + chan2;
    float weight((*bandPassFactors)[combined_channel]);

    // Read from memory in the quickest dimension (optimal)
    fcomplex sampleX = (*inputData)[station][0][idx_channel1][sample][chan2];
    fcomplex sampleY = (*inputData)[station][1][idx_channel1][sample][chan2];
    
    sampleX.x *= weight;
    sampleX.y *= weight;
    sampleY.x *= weight;
    sampleY.y *= weight;

    // Write the data to shared memory

    tmp[threadIdx.y][threadIdx.x][0] = sampleX;
    tmp[threadIdx.y][threadIdx.x][1] = sampleY;
    __syncthreads();  // assures all writes are done

    // Now write from shared to global memory.
    unsigned chan_index = idx_channel1 * NR_CHANNELS_2 + blockIdx.x * blockDim.x + threadIdx.y;
    // Use the threadidx.x for the highest array index: coalesced writes
    unsigned sample_index = blockIdx.y * blockDim.y + threadIdx.x;

    (*outputData)[station][chan_index][sample_index][0] = tmp[threadIdx.x][threadIdx.y][0];
    (*outputData)[station][chan_index][sample_index][1] = tmp[threadIdx.x][threadIdx.y][1];
    __syncthreads();  // assure are writes are done. The next for itteration reuses the array
  }
}
}

