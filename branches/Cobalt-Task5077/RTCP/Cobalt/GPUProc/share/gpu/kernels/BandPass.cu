//# BandPass.cu
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
 bandpass correction. It can also transpose the data (pol to dim 0).
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
 * - @c NR_POLARIZATIONS: 2
  *
 */

#include "gpu_math.cuh"
#include "IntToFloat.cuh"


typedef  fcomplex (* OutputDataType)[NR_STATIONS][NR_CHANNELS_1 * NR_CHANNELS_2][NR_SAMPLES_PER_CHANNEL][NR_POLARIZATIONS];
typedef  fcomplex (* InputDataType)[NR_STATIONS][NR_POLARIZATIONS][NR_CHANNELS_1][NR_SAMPLES_PER_CHANNEL][NR_CHANNELS_2];
typedef  const float (* BandPassFactorsType)[NR_CHANNELS_1 * NR_CHANNELS_2];

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
 * @param[in]  bandPassFactorsPtr  pointer to bandpass correction data of
 *                                 ::BandPassFactorsType, a 1D array [channel] of
 *                                 float, containing bandpass correction factors
 */
#define SHARED
extern "C" {
 __global__ void correctBandPass( fcomplex * correctedDataPtr,
                                  const fcomplex * filteredDataPtr,
                                  const float * bandPassFactorsPtr)
{
  
  OutputDataType outputData = (OutputDataType) correctedDataPtr;
  InputDataType inputData   = (InputDataType)  filteredDataPtr;

  
  BandPassFactorsType bandPassFactors = (BandPassFactorsType) bandPassFactorsPtr;
  
  // fasted dims
  unsigned chan2        = blockIdx.x * blockDim.x + threadIdx.x;
  unsigned sample       = blockIdx.y * blockDim.y + threadIdx.y;
  unsigned station      = blockIdx.z * blockDim.z + threadIdx.z;



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

#if defined SHARED
    //  4.6 ms
    // Write blocks of memory 16 by 16 in chared
    __shared__ fcomplex tmp[16][16 + 1][2]; // one too wide to avoid bank-conflicts on read
    // 
    tmp[threadIdx.y][threadIdx.x][0] = sampleX;
    tmp[threadIdx.y][threadIdx.x][1] = sampleY;
    __syncthreads();
    // Write data to global with the sample moving the 
    // Use correct coallesced writes: 2 ms
    (*outputData)[station][idx_channel1 * NR_CHANNELS_2 + blockIdx.x * blockDim.x + threadIdx.y][blockIdx.y * blockDim.y + threadIdx.x][0] = tmp[threadIdx.x][threadIdx.y][0];
    (*outputData)[station][idx_channel1 * NR_CHANNELS_2 + blockIdx.x * blockDim.x + threadIdx.y][blockIdx.y * blockDim.y + threadIdx.x][1] = tmp[threadIdx.x][threadIdx.y][1];
    __syncthreads();
#else
    // 5.5 ms
    (*outputData)[station][combined_channel][sample][0] = sampleX; //tmp[minor][major][0];
    (*outputData)[station][combined_channel][sample][1] = sampleY; //tmp[minor][major][1];
#endif
  }
}
}

