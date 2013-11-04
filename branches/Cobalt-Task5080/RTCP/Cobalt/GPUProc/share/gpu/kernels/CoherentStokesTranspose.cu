//# CoherentStokesTranspose.cu
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
 *
 * \param[out] OutputDataType      4D output array of samples. For each TAB and pol, a spectrum per time step of complex floats.
 * \param[in]  InputDataType       3D input array of samples (last dim (pol) is implicit). For each channel, the TABs per time step of two complex floats.
 *
 * Pre-processor input symbols (some are tied to the execution configuration)
 * Symbol                  | Valid Values            | Description
 * ----------------------- | ----------------------- | -----------
 * NR_SAMPLES_PER_CHANNEL  | multiple of 16 and > 0  | number of input samples per channel
 * NR_CHANNELS             | >= 1                    | number of frequency channels per subband
 * NR_TABS                 | multiple of 16 and > 0  | number of Tied Array Beams to create
 *
 * Note that this kernel assumes  NR_POLARIZATIONS == 2
 *
 * Execution configuration:
 * - LocalWorkSize = 2 dimensional; (16, 16, 1) is in use.
 * - GlobalWorkSize = 3 dimensional:
 *   + inner dim (x): nr tabs (/ 16)
 *   + middle dim (y): nr samples ( /16)
 *   + outer dim (z): number of channels (/1)
 */
#include "gpu_math.cuh"

typedef fcomplex (*OutputDataType)[NR_TABS][NR_POLARIZATIONS][NR_CHANNELS][NR_SAMPLES_PER_CHANNEL]; //last dims of this needs to be swapped

typedef float4 fcomplex2;
// Allows for better memory access
typedef fcomplex2 (*InputDataType)[NR_CHANNELS][NR_SAMPLES_PER_CHANNEL][NR_TABS]; // [NR_POLARIZATIONS];


extern "C"
__global__ void coherentStokesTranspose(void *OutputDataPtr,
                          const void *InputDataPtr)
{ 
  OutputDataType outputData = (OutputDataType) OutputDataPtr;
  InputDataType inputData = (InputDataType) InputDataPtr;
  
  // Reorder the grid block indexing: to prevent global memory access bank conflicts
  // blockIdx.x starts at the diagonal
  unsigned block_x;
  unsigned block_y;
  if ( NR_SAMPLES_PER_CHANNEL == NR_TABS)  // if workload is square
  {
     block_x= (blockIdx.y + blockIdx.x) % gridDim.x; 
     block_y = blockIdx.x;
  } 
  else
  {
    unsigned bid = blockIdx.x + gridDim.x * blockIdx.y;
    block_y = bid % gridDim.y;
    block_x = ((bid % gridDim.y) + block_y) % gridDim.x;    
  }

  unsigned tab           = block_x * blockDim.x + threadIdx.x;
  unsigned sample        = block_y * blockDim.y + threadIdx.y;
  unsigned channel       = blockIdx.z * blockDim.z ;

  // Use shared memory for the transpose
  __shared__ fcomplex2 tmp[16][16 + 1];  // plus one to prevent bank conflicts in shared memory

  tmp[threadIdx.y][threadIdx.x] = (*inputData) [channel][sample][tab];
  __syncthreads();  // assures all writes are done

  // Reassign the tab and sample to allow the threadIdx.x to write in the highest dimension
  tab           = block_x * blockDim.x + threadIdx.y;
  sample        = block_y * blockDim.y + threadIdx.x;

  // Do the write to global mem
  (*outputData)[tab][0][channel][sample] = make_float2(tmp[threadIdx.x][threadIdx.y].x,
    tmp[threadIdx.x][threadIdx.y].y) ;
  (*outputData)[tab][1][channel][sample] = make_float2(tmp[threadIdx.x][threadIdx.y].z,
    tmp[threadIdx.x][threadIdx.y].w) ;

  __syncthreads();  // assures all writes are done
}
