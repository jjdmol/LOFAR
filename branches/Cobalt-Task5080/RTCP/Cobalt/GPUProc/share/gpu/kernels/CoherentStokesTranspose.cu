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

typedef float4 fcomplex2;
// last dim within float4 is NR_POLARIZATIONS
typedef fcomplex2 (*InputDataType)[NR_CHANNELS][NR_SAMPLES_PER_CHANNEL][NR_TABS]; // [NR_POLARIZATIONS];

// fcomplex2 speedup 5%

extern "C"
__global__ void coherentStokesTranspose(void *OutputDataPtr,
                          const void *InputDataPtr)
{ 
  OutputDataType outputData = (OutputDataType) OutputDataPtr;
  InputDataType inputData = (InputDataType) InputDataPtr;
  

//#define BLOCKREORDER 1
#ifdef BLOCKREORDER
  // Reorder the grid block indexing: to prevent global memory access bank conflicts
  // blockIdx.x start at the diagonal
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
#else
  unsigned block_x      = blockIdx.x ;
  unsigned block_y      = blockIdx.y ; 
#endif

  unsigned tab           = block_x * blockDim.x + threadIdx.x;
  unsigned sample        = block_y * blockDim.y + threadIdx.y;

  for (unsigned idx = 0; idx < 1; ++idx)  // Do more work in a kernel allows hiding of preparation work
  {
    unsigned channel       = blockIdx.z * blockDim.z + idx;

    // Use shared memory for the transpose
    __shared__ fcomplex2 tmp[16][16 + 1];  // plus one to prevent bank conflicts in shared memory
    
    tmp[threadIdx.y][threadIdx.x] = (*inputData) [channel][sample][tab];
    __syncthreads();  // assures all writes are done

    // Reassign the tab and sample to allow the threadIdx.x to write in the highest dimension
    tab           = block_x * blockDim.x + threadIdx.y;
    sample        = block_y * blockDim.y + threadIdx.x;
    (*outputData)[tab][0][channel][sample] = make_float2(tmp[threadIdx.x][threadIdx.y].x,
                                                         tmp[threadIdx.x][threadIdx.y].y) ;

    (*outputData)[tab][1][channel][sample] = make_float2(tmp[threadIdx.x][threadIdx.y].z,
                                                         tmp[threadIdx.x][threadIdx.y].w) ;

    __syncthreads();  // assures all writes are done
  }
}
    // OPtimal write to global memory
    //  6.43%  757.00us         1  757.00us  757.00us  757.00us  coherentStokesTranspose
    //   6.06%  710.37us         1  710.37us  710.37us  710.37us  coherentStokesTranspose float4
    // 5.51  642.34us       1  642.34us  642.34us  642.34us  coherentStokesTranspose 4 channel in for loop
    // 5.74  671.17us       1  671.17us  671.17us  671.17us  coherentStokesTranspose 8 channel
    // 5.23  607.53us       1  607.53us  607.53us  607.53us  coherentStokesTranspose
    // With blovkr reordering idx = 0  4.70  543.05us       1  543.05us  543.05us  543.05us  coherentStokesTranspose
