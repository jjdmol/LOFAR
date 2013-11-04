//# IncoherentStokesTranspose.cu
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

#include <stdio.h>

//#define NAIVE
#define SHARED_MEM

#if !(TILE_SIZE == 16)
#error Precondition violated: TILE_SIZE == 16
#endif

#if !(NR_CHANNELS >= 1)
#error Precondition violated: NR_CHANNELS >= 1
#endif

#if !(NR_POLARIZATIONS == 2)
#error Precondition violated: NR_POLARIZATIONS == 2
#endif

#if !(NR_SAMPLES_PER_CHANNEL > 0 && NR_SAMPLES_PER_CHANNEL % 16 == 0)
#error Precondition violated: NR_SAMPLES_PER_CHANNEL > 0 && NR_SAMPLES_PER_CHANNEL % 16 == 0
#endif

#if !(NR_STATIONS >= 1)
#error Precondition violated: NR_STATIONS >= 1
#endif

// 3-D input data array of band-pass corrected data. Note that, actually, the
// data is 4-D (<tt>[station][channel][time][pol]</tt>), but the 4th dimension
// has been squashed into a single float4 (i.e., two complex polarizations).
typedef float4 (*InputDataType)[NR_STATIONS][NR_CHANNELS][NR_SAMPLES_PER_CHANNEL];

// 4-D output data array of band-pass corrected data that can be fed into an
// inverse FFT (<tt>[station][pol][time][channel]</tt>).
typedef float2 (*OutputDataType)[NR_STATIONS][NR_POLARIZATIONS][NR_SAMPLES_PER_CHANNEL][NR_CHANNELS];


// Performs data transposition from the output of the beamformer kernel to a
// data order suitable for an inverse FFT. Parallelisation is performed over the
// TABs and number of samples (time).
//
// We have 4 dimensions, but CUDA thread blocks can be up to three. Mangle the
// TAB and sample dimension in to dim 0 (x).
//
// The kernel needs to determine for each thread whether to read and separately
// whether to write back a sample, because the number of TABs may not divide by
// the 16x16 thread arrangement (even though we have a 1D thread block).
//
// \param[out] OutputDataType 4D output array of samples. For each TAB and pol,
// a spectrum per time step of complex floats.
// \param[in] ComplexVoltagesType 3D input array of samples (last dim (pol) is
// implicit). For each channel, the TABs per time step of two complex floats.
//
// Pre-processor input symbols (some are tied to the execution configuration)
// Symbol                  | Valid Values            | Description
// ----------------------- | ----------------------- | -----------
// NR_CHANNELS             | >= 1                    | number of frequency channels per subband
// NR_POLARIZATIONS        | 2                       | number of polarizations
// NR_SAMPLES_PER_CHANNEL  | multiple of 16 and > 0  | number of input samples per channel
// NR_STATIONS             | >= 1                    | number of Tied Array Beams to create
//
// Execution configuration:
//
// - LocalWorkSize = 1 dimensional; (256, 1, 1) is in use. 
//   Multiples of (32, 1, 1) may work too.
// - GlobalWorkSize = 3 dimensional:
//   + inner dim (x): always 1 block
//   + middle dim (y): 16 TABs can be processed in a block.
//     Number of blocks required, rounded-up. eg for 17 tabs we need 2 blocks
//   + outer dim (z): 16 samples per channel can be processed in a block.
//     Number of blocks required (fits exactly). 32 channels is 2 blocks
extern "C"
__global__ void transpose(OutputDataType output,
                          const InputDataType input)
{
  unsigned time, channel;

#if defined(NAIVE)
  // Naive approach: directly reading from and writing to global memory.
  for (int station = 0; station < NR_STATIONS; station++) {
    time = blockIdx.x * blockDim.x + threadIdx.x;
    channel = blockIdx.y * blockDim.y + threadIdx.y;
    if (time < NR_SAMPLES_PER_CHANNEL && channel < NR_CHANNELS) {
      float4 sample = (*input)[station][channel][time];
      (*output)[station][0][time][channel] = make_float2(sample.x, sample.y);
      (*output)[station][1][time][channel] = make_float2(sample.z, sample.w);
    }
  }
#elif defined(SHARED_MEM)
  // Use shared memory to do a block transpose. Both reads and writes to global
  // memory can then be made coalesced.
  __shared__ float4 tmp[TILE_SIZE][TILE_SIZE + 1];

  for (int station = 0; station < NR_STATIONS; station++) {
    time = blockIdx.x * blockDim.x + threadIdx.x;
    channel = blockIdx.y * blockDim.y + threadIdx.y;
    // Inside our data cube?
    if (channel < NR_CHANNELS && time < NR_SAMPLES_PER_CHANNEL) {
      // Read data
      tmp[threadIdx.y][threadIdx.x] =  (*input)[station][channel][time];
    }
    __syncthreads();

    time = blockIdx.x * blockDim.x + threadIdx.y;
    channel = blockIdx.y * blockDim.y + threadIdx.x;
    // Inside our data cube?
    if (channel < NR_CHANNELS && time < NR_SAMPLES_PER_CHANNEL) {
      // Write data
      float4 sample = tmp[threadIdx.x][threadIdx.y];
      (*output)[station][0][time][channel] = make_float2(sample.x, sample.y);
      (*output)[station][1][time][channel] = make_float2(sample.z, sample.w);
    }
    __syncthreads();

  }
#endif

}
