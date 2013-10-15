//# CoherentStokes.cu: Calculate the Stokes parameters
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
//# $Id: CoherentStokes.cu 24553 2013-04-09 14:21:56Z mol $

#if NR_SAMPLES_PER_CHANNEL % INTEGRATION_SIZE != 0 
  #error  unsupported INTEGRATION_SIZE for NR_SAMPLES_PER_CHANNEL
#elif NR_COHERENT_STOKES != 1
  #if NR_COHERENT_STOKES != 4
     #error  unsupported NR_COHERENT_STOKES
  #endif
#elif NR_CHANNELS % 16 != 0
  #if NR_CHANNELS != 1
    #error unsupported NR_CHANNELS
  #endif
#endif

//4D output array of stokes values. Each sample contains 1 or 4 stokes paramters. For each tab, there are NR_STOKES timeseries of channels 
typedef float2 (*inputDataType)[NR_TABS][NR_POLARIZATIONS][NR_SAMPLES_PER_CHANNEL][NR_CHANNELS]; 

//4D input array of complex samples. For each tab and polarization there are timelines with data for each channel
typedef float (*outputDataType)[NR_TABS][NR_COHERENT_STOKES][NR_SAMPLES_PER_CHANNEL/INTEGRATION_SIZE][NR_CHANNELS];

/*!
 * Computes the first or all 4 stokes parameters.
 * http://www.astron.nl/~romein/papers/EuroPar-11/EuroPar-11.pdf 
 * \code
 * I = X * conj(X) + Y * conj(Y)
 * Q = X * conj(X) - Y * conj(Y)
 * U = 2 * real(X * con(Y))
 * V = 2 * imag(X * con(Y))
 * \endcode
 * This reduces to (validated on paper by Wouter and John):
 * \code
 * Px = real(X) * real(X) + imag(X) * imag(X)
 * Py = real(Y) * real(Y) + imag(Y) * imag(Y)
 * I = Px + Py
 * Q = Px - Py
 * U = 2 * (real(X) * real(Y) + imag(X) * imag(Y))
 * V = 2 * (imag(X) * real(Y) - real(X) * imag(Y))
 * \endcode
 * 
 * The kernel's first parallel dimension is on the channels; the second dimension is in time; the third on the tabs.
 * The thread block size based on these factors could be larger then the hardmare max.
 * Therefore<tt> NR_CHANNELS * NR_TABS * TIME_PARALLEL_FACTOR </tt>should not exceed the hardware maximum of threads (1024 on a K10).
 *
 * \param[out] outputPtr         4D output array of stokes values. Each sample contains 1 or 4 stokes paramters. For each tab, there are NR_STOKES timeseries of channels 
 *                               The dimensions are: NR_TABS by NR_COHERENT_STOKES by (NR_SAMPLES_PER_CHANNEL/INTEGRATION_SIZE)  by NR_CHANNELS.
 * \param[in]  inputPtr          4D input array of complex samples. For each tab and polarization there are timelines with data for each channel
 *                               The dimensions are: NR_TABS by NR_POLARIZATIONS by NR_SAMPLES_PER_CHANNEL by NR_CHANNELS
 *
 * Pre-processor input symbols (some are tied to the execution configuration)
 * Symbol                  | Valid Values            | Description
 * ----------------------- | ----------------------- | -----------
 * NR_STATIONS             | >= 1                    | number of antenna fields
 * NR_SAMPLES_PER_CHANNEL  | multiple of INTEGRATION_SIZE | number of input samples per channel
 * NR_CHANNELS             | 1 or 16                 | number of frequency channels per subband
 * NR_TABS                 | >= 1                    | number of tabs to create
 * NR_COHERENT_STOKES      | 1 or 4                  | number of stokes paramters to create
 * INTEGRATION_SIZE        | >= 1                    | amount of samples to integrate to a single output sample
 * TIME_PARALLEL_FACTOR    | >= 1                    | amount of parallel threads to work on a full timerange
 *
 * Note that this kernel assumes and needs NR_POLARIZATIONS == 2 and COMPLEX == 2
 * 
 * The TIME_PARALLEL_FACTOR splits the time range in a number of portions which get worked on by 
 * seperate threads (in parallel).
 *  
 * Execution configuration:
 * - LocalWorkSize = 3 dimensional; (NR_CHANNELS, TIME_PARALLEL_FACTOR, NR_TABS)
 *                   The product of the three should not be larger then max thread size.
 *                   The max thread size depends on the hardware used. 512 For odl hardware. K10 and higher have 1024 threads 
 * - GlobalWorkSize = 3 dimensional; depends on the size of NR_TABS, NR_CHANNELS and the max thread size
 *                   Ideally the work fits in a single block. If not the remainder could
 *                   be computed with a second (differently sized) block
 */
extern "C" __global__ void coherentStokes(void *outputPtr, const void *inputPtr) 
{
  inputDataType input = (inputDataType) inputPtr;
  outputDataType output = (outputDataType) outputPtr;

  // Define the indexes in the data depending on the block and thread idx
  unsigned channel_idx = threadIdx.x;  // If we have channels do the read and write with 16 in parallel
  unsigned time_idx = threadIdx.y;     
  unsigned tab_idx = threadIdx.z;    

  // Step over (part of) the timerange of samples with INTEGRATION_SIZE steps
  // The time_idx determines which part of (or the whole of) the timerange this thread is working on:
  // Work from the start of the time frame (pending your threadIdx.y) untill the next timeframe
  // Step within this timerange with integration size steps. These substeps are done in the inner loop
  for (unsigned idx_stride = time_idx * (NR_SAMPLES_PER_CHANNEL / TIME_PARALLEL_FACTOR) ; 
                   idx_stride < (time_idx + 1) * (NR_SAMPLES_PER_CHANNEL / TIME_PARALLEL_FACTOR)
                && idx_stride < NR_SAMPLES_PER_CHANNEL;
                idx_stride += INTEGRATION_SIZE)
  {
    // We are integrating all values in the current stride
    // local variable
    float stokesI = 0;
#   if NR_COHERENT_STOKES == 4
    float stokesQ = 0;
    float halfStokesU = 0;
    float halfStokesV = 0;
#   endif

    // Do the integration
    for (unsigned idx_step = 0; idx_step < INTEGRATION_SIZE; idx_step++) 
    {
      float2 X = (*input)[tab_idx][0][idx_stride + idx_step][channel_idx];    
      float2 Y = (*input)[tab_idx][1][idx_stride + idx_step][channel_idx];

      // Calculate the partial solutions
      float powerX = X.x * X.x + X.y * X.y;
      float powerY = Y.x * Y.x + Y.y * Y.y;
      stokesI += powerX + powerY;
#     if NR_COHERENT_STOKES == 4
      stokesQ += powerX - powerY;
      halfStokesU += X.x * Y.x + X.y * Y.y;
      halfStokesV += X.y * Y.x - X.x * Y.y;
#     endif
    }
    // We step in the data with INTEGRATION_SIZE
    unsigned write_idx = idx_stride / INTEGRATION_SIZE;

    (*output)[tab_idx][0][write_idx][channel_idx] = stokesI;
#   if NR_COHERENT_STOKES == 4
    (*output)[tab_idx][1][write_idx][channel_idx] = stokesQ;
    (*output)[tab_idx][2][write_idx][channel_idx] = 2 * halfStokesU;
    (*output)[tab_idx][3][write_idx][channel_idx] = 2 * halfStokesV;
#   endif  
    // No baries needed. All computations are fully parallel
  }
}
