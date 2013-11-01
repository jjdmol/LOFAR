//# Transpose.cu
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

/*!
 * Performs data transposition from the output of the beamformer kernel to
 * a data order suitable for an inverse FFT.
 * Parallelisation is performed over the TABs and number of samples (time).
 *
 * We have 4 dimensions, but CUDA thread blocks can be up to three.
 * Mangle the TAB and sample dimension in to dim 0 (x).
 *
 * The kernel needs to determine for each thread whether to read and separately
 * whether to write back a sample, because the number of TABs may not divide by
 * the 16x16 thread arrangement (even though we have a 1D thread block).
 *
 * \param[out] TransposedDataType      4D output array of samples. For each TAB and pol, a spectrum per time step of complex floats.
 * \param[in]  ComplexVoltagesType     3D input array of samples (last dim (pol) is implicit). For each channel, the TABs per time step of two complex floats.
 *
 * Pre-processor input symbols (some are tied to the execution configuration)
 * Symbol                  | Valid Values            | Description
 * ----------------------- | ----------------------- | -----------
 * NR_SAMPLES_PER_CHANNEL  | multiple of 16 and > 0  | number of input samples per channel
 * NR_CHANNELS             | >= 1                    | number of frequency channels per subband
 * NR_TABS                 | >= 1                    | number of Tied Array Beams to create
 *
 * Note that this kernel assumes  NR_POLARIZATIONS == 2
 *
 * Execution configuration:
 * - LocalWorkSize = 1 dimensional; (256, 1, 1) is in use. Multiples of (32, 1, 1) may work too.
 * - GlobalWorkSize = 3 dimensional:
 *   + inner dim (x): always 1 block
 *   + middle dim (y): 16 TABs can be processed in a block. Number of blocks required, rounded-up. eg for 17 tabs we need 2 blocks
 *   + outer dim (z): 16 samples per channel can be processed in a block. Number of blocks required (fits exactly). 32 channels is 2 blocks
 */
#include "gpu_math.cuh"

typedef fcomplex (*OutputDataType)[NR_TABS][NR_POLARIZATIONS][NR_CHANNELS][NR_SAMPLES_PER_CHANNEL]; //last dims of this needs to be swapped

// last dim within float4 is NR_POLARIZATIONS
typedef fcomplex (*InputDataType)[NR_CHANNELS][NR_SAMPLES_PER_CHANNEL][NR_TABS][NR_POLARIZATIONS]; // [NR_POLARIZATIONS];





extern "C"
__global__ void coherentStokesTranspose(void *OutputDataPtr,
                          const void *InputDataPtr)
{

  OutputDataType outputData = (OutputDataType) OutputDataPtr;
  InputDataType inputData = (InputDataType) InputDataPtr;
     
  // fasted dims
  unsigned tab          = blockIdx.x * blockDim.x + threadIdx.x;
  unsigned sample       = blockIdx.y * blockDim.y + threadIdx.y;
  unsigned channel       = blockIdx.z * blockDim.z + threadIdx.z;

  (*outputData)[tab][0][channel][sample] = (*inputData) [channel][sample][tab][0];
  (*outputData)[tab][1][channel][sample] = (*inputData)[channel][sample][tab][1];




  // Shared memory to perform a transpose in shared memory
  //// one too wide to avoid bank-conflicts on read
  //// 16 by 16 limitation for the channels2 and samples per channel are caused by the
  //// dimensions of this array
  //// TODO: Increasing to 32 x 32 allows for a speedup of 13%
  //__shared__ fcomplex tmp[16][16 + 1][2];

  //for (unsigned idx_channel1 = 0; idx_channel1 < NR_CHANNELS_1; ++idx_channel1)
  //{
  //  unsigned combined_channel = idx_channel1 * NR_CHANNELS_2 + chan2;
  //  float weight((*bandPassFactors)[combined_channel]);

  //  // Read from memory in the quickest dimension (optimal)
  //  fcomplex sampleX = (*inputData)[station][0][idx_channel1][sample][chan2];
  //  fcomplex sampleY = (*inputData)[station][1][idx_channel1][sample][chan2];

  //  sampleX.x *= weight;
  //  sampleX.y *= weight;
  //  sampleY.x *= weight;
  //  sampleY.y *= weight;

  //  // Write the data to shared memory

  //  tmp[threadIdx.y][threadIdx.x][0] = sampleX;
  //  tmp[threadIdx.y][threadIdx.x][1] = sampleY;
  //  __syncthreads();  // assures all writes are done

  //  // Now write from shared to global memory.
  //  unsigned chan_index = idx_channel1 * NR_CHANNELS_2 + blockIdx.x * blockDim.x + threadIdx.y;
  //  // Use the threadidx.x for the highest array index: coalesced writes to the global memory
  //  unsigned sample_index = blockIdx.y * blockDim.y + threadIdx.x;

  //  (*outputData)[station][chan_index][sample_index][0] = tmp[threadIdx.x][threadIdx.y][0];  // The threadIdx.y in shared mem is not a problem
  //  (*outputData)[station][chan_index][sample_index][1] = tmp[threadIdx.x][threadIdx.y][1];
  //  __syncthreads();  // assure are writes are done. The next for itteration reuses the array
  //}

#ifdef SKIP
  /*
   41.89%  5.5789ms         1  5.5789ms  5.5789ms  5.5789ms  [CUDA memcpy HtoD]
   38.80%  5.1674ms         1  5.1674ms  5.1674ms  5.1674ms  [CUDA memcpy DtoH]
   19.30%  2.5703ms         1  2.5703ms  2.5703ms  2.5703ms  coherentStokesTranspose
  */
  unsigned tabBase = 16 * blockDim.y * blockIdx.y + threadIdx.y;
  unsigned timeBase = 16 * blockDim.z * blockIdx.z + threadIdx.z;

  unsigned tabOffsetR = threadIdx.x & 15;
  unsigned tabR = tabBase + tabOffsetR;
  unsigned timeOffsetR = threadIdx.x >> 4;
  unsigned timeR = timeBase + timeOffsetR;
  bool doR = NR_TABS % 16 == 0 || tabR < NR_TABS;

  unsigned tabOffsetW = threadIdx.x >> 4;
  unsigned tabW = tabBase + tabOffsetW;
  unsigned timeOffsetW = threadIdx.x & 15;
  unsigned timeW = timeBase + timeOffsetW;
  bool doW = NR_TABS % 16 == 0 || tabW < NR_TABS;

  for (int channel = 0; channel < NR_CHANNELS; channel++) 
  {
    if (doR)
    {
      tmp[tabOffsetR][timeOffsetR] = (*complexVoltages)[channel][timeR][tabR];
    }

    __syncthreads();

    if (doW) {
      float4 sample = tmp[tabOffsetW][timeOffsetW];
      (*transposedData)[tabW][0][channel][timeW] = make_float2(sample.x, sample.y);
      (*transposedData)[tabW][1][channel][timeW] = make_float2(sample.z, sample.w);
    }

    __syncthreads();

  }
  #endif
}
