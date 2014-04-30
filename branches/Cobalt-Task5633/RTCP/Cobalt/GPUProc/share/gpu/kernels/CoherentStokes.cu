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
//# $Id$

#if !(TIME_INTEGRATION_FACTOR >= 1)
#error Precondition violated: TIME_INTEGRATION_FACTOR >= 1
#endif

#if !(NR_CHANNELS >= 1)
#error Precondition violated: NR_CHANNELS >= 1
#endif

#if !(NR_COHERENT_STOKES == 1 || NR_COHERENT_STOKES == 4)
#error Precondition violated: NR_COHERENT_STOKES == 1 || NR_COHERENT_STOKES == 4
#endif

#if !(COMPLEX_VOLTAGES == 0 || NR_COHERENT_STOKES == 4)
#error Precondition violated: COMPLEX_VOLTAGES == 0 || NR_COHERENT_STOKES == 4
#endif

#if !(NR_POLARIZATIONS == 2)
#error Precondition violated: NR_POLARIZATIONS == 2
#endif

#if !(NR_SAMPLES_PER_CHANNEL > 0 && NR_SAMPLES_PER_CHANNEL % TIME_INTEGRATION_FACTOR == 0)
#error Precondition violated: NR_SAMPLES_PER_CHANNEL > 0 && NR_SAMPLES_PER_CHANNEL % TIME_INTEGRATION_FACTOR == 0
#endif

#if !(NR_TABS >= 1)
#error Precondition violated: NR_TABS >= 1
#endif

//4D input array of complex samples. For each tab and polarization there are
//time lines with data for each channel
typedef float2 (*InputDataType)[NR_TABS][NR_POLARIZATIONS][NR_SAMPLES_PER_CHANNEL][NR_CHANNELS]; 

//4D output array of stokes values. Each sample contains 1 or 4 stokes
//paramters. For each tab, there are NR_COHERENT_STOKES timeseries of channels
typedef float (*OutputDataType)[NR_TABS][NR_COHERENT_STOKES][NR_SAMPLES_PER_CHANNEL / TIME_INTEGRATION_FACTOR][NR_CHANNELS];

/*!
 * Computes the Stokes I or IQUV, or outputs the 4 complex voltages (XrXiYrYi).
 * http://www.astron.nl/~romein/papers/EuroPar-11/EuroPar-11.pdf 
 * In case of Stokes:
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
 * The kernel's first parallel dimension is on the channels; the second
 * dimension is in time; the third on the tabs.  The thread block size based on
 * these factors could be larger then the hardmare max.  Therefore<tt>
 * NR_CHANNELS * timeParallelFactor * NR_TABS</tt> should not exceed the
 * hardware maximum of threads per block (1024 on an NVIDIA K10).
 *
 * \param[out] output
 *             4D output array of stokes values. Each sample contains 1 or 4
 *             stokes paramters. For each tab, there are \c NR_COHERENT_STOKES
 *             time series of channels. The dimensions are: \c NR_TABS, \c
 *             NR_COHERENT_STOKES,
 *             <tt>(NR_SAMPLES_PER_CHANNEL / TIME_INTEGRATION_FACTOR)</tt>, \c
 *             NR_CHANNELS.
 * \param[in]  input
 *             4D input array of complex samples. For each tab and polarization
 *             there are time lines with data for each channel. The dimensions
 *             are: \c NR_TABS, \c NR_POLARIZATIONS, \c NR_SAMPLES_PER_CHANNEL,
 *             \c NR_CHANNELS
 * \param[in]  timeIntegrationFactor
 *             immediate value that indicates in how many sub-ranges the sample
 *             range (time) is split for independent processing. Must be >= 1 and:
 *             NR_SAMPLES_PER_CHANNEL / timeParallelFactor must be a multiple of
 *             timeIntegrationFactor.
 *
 * Pre-processor input symbols (some are tied to the execution configuration)
 * Symbol                  | Valid Values  | Description
 * ----------------------- | ------------- | -----------
 * TIME_INTEGRATION_FACTOR | >= 1          | amount of samples to integrate to a single output sample
 * NR_CHANNELS             | >= 1          | number of frequency channels per subband
 * NR_COHERENT_STOKES      | 1 or 4        | number of stokes paramters to create
 * COMPLEX_VOLTAGES        | 1 or 0        | whether we compute complex voltages or coherent stokes
 * NR_POLARIZATIONS        | 2             | number of polarizations
 * NR_SAMPLES_PER_CHANNEL  | multiple of TIME_INTEGRATION_FACTOR | number of input samples per channel
 * NR_TABS                 | >= 1          | number of tabs to create
 * 
 * Execution configuration:
 * - LocalWorkSize = 3 dimensional; (\c NR_CHANNELS, \c timeParallelFactor, \c
 *                   NR_TABS).  The product of the three should not be larger
 *                   then max thread size.  The max thread size depends on the
 *                   hardware used: 512 for old hardware, 1024 for K10 and
 *                   higher.
 * - GlobalWorkSize = 3 dimensional; depends on the size of \c NR_TABS, \c
 *                   NR_CHANNELS and the max thread size. Ideally the work fits
 *                   in a single block. If not the remainder could be computed
 *                   with a second (differently sized) block.
 */
extern "C" __global__ void coherentStokes(OutputDataType output,
                                          const InputDataType input,
                                          unsigned timeParallelFactor)
{
  unsigned channel_idx = blockIdx.x * blockDim.x + threadIdx.x;
  unsigned time_idx = threadIdx.y;     
  unsigned tab_idx = blockIdx.z * blockDim.z + threadIdx.z;    

  // We support all sizes of channels and TABs: skip current thread if not needed.
  if ( channel_idx >= NR_CHANNELS)
    return;
  if ( tab_idx >= NR_TABS)
    return;

  //# Process samples by reading TIME_INTEGRATION_FACTOR samples and writing one
  //# set of Stokes. For parallelism over time, split the sample range in
  //# timeParallelFactor sub-ranges and process these independently.
  //# For complex voltages, we don't compute anything; it's only a transpose.
  //#
  //# TODO: This kernel must be rewritten as if it is a transpose to get efficient global mem read and write accesses.
  //#       This reqs shmem. Note that combining shmem barriers with the two conditional returns above is problematic.
  //# TODO: For very large TIME_INTEGRATION_FACTOR (e.g. 1024), we may need parallel reduction to have enough parallelization. TBD.
  unsigned read_idx  = time_idx * (NR_SAMPLES_PER_CHANNEL / timeParallelFactor);
  unsigned write_idx = read_idx / TIME_INTEGRATION_FACTOR;
  for ( ; read_idx < (time_idx + 1) * (NR_SAMPLES_PER_CHANNEL / timeParallelFactor) &&
          read_idx < NR_SAMPLES_PER_CHANNEL; write_idx++)
  {
    //# Integrate all values in the current stride
#   if COMPLEX_VOLTAGES == 1
      float4 stokes = { 0.0f, 0.0f, 0.0f, 0.0f };
#   else
      float stokesI = 0.0f;
#     if NR_COHERENT_STOKES == 4
        float stokesQ = 0.0f;
        float halfStokesU = 0.0f;
        float halfStokesV = 0.0f;
#     endif
#   endif
    
    //# Do the integration
    for (unsigned stride_read_idx_end = read_idx + TIME_INTEGRATION_FACTOR;
         read_idx < stride_read_idx_end; read_idx++)
    {
      float2 X = (*input)[tab_idx][0][read_idx][channel_idx];
      float2 Y = (*input)[tab_idx][1][read_idx][channel_idx];

#     if COMPLEX_VOLTAGES == 1
        stokes.x += X.x;
        stokes.y += X.y;
        stokes.z += Y.x;
        stokes.w += Y.y;
#     else
        float powerX = X.x * X.x + X.y * X.y;
        float powerY = Y.x * Y.x + Y.y * Y.y;
        stokesI += powerX + powerY;
#       if NR_COHERENT_STOKES == 4
          stokesQ += powerX - powerY;
          halfStokesU += X.x * Y.x + X.y * Y.y;
          halfStokesV += X.y * Y.x - X.x * Y.y;
#       endif
#     endif
    }

#   if COMPLEX_VOLTAGES == 1
      (*output)[tab_idx][0][write_idx][channel_idx] = stokes.x;
      (*output)[tab_idx][1][write_idx][channel_idx] = stokes.y;
      (*output)[tab_idx][2][write_idx][channel_idx] = stokes.z;
      (*output)[tab_idx][3][write_idx][channel_idx] = stokes.w;
#   else
      (*output)[tab_idx][0][write_idx][channel_idx] = stokesI;
#     if NR_COHERENT_STOKES == 4
        (*output)[tab_idx][1][write_idx][channel_idx] = stokesQ;
        (*output)[tab_idx][2][write_idx][channel_idx] = 2.0f * halfStokesU;
        (*output)[tab_idx][3][write_idx][channel_idx] = 2.0f * halfStokesV;
#     endif  
#   endif
  }
}
