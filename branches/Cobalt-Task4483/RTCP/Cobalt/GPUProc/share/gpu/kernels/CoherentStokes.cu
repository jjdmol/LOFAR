//# CoherentStokes.cu
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

typedef float2 (*inputDataType)[NR_TABS][NR_POLARIZATIONS][NR_SAMPLES_PER_CHANNEL][NR_CHANNELS]; 
typedef float (*outputDataType)[NR_TABS][NR_COHERENT_STOKES][NR_SAMPLES_PER_CHANNEL/INTEGRATION_SIZE][NR_CHANNELS];

/*!
 * Computes correlations between all pairs of stations (baselines) and X,Y
 * polarizations. Also computes all station (and pol) auto-correlations.
 # blabla bla
 * \param[out] visibilitiesPtr         2D output array of visibilities. Each visibility contains the 4 polarization pairs, XX, XY, YX, YY, each of complex float type.
 * \param[in]  correctedDataPtr        3D input array of samples. Each sample contains the 2 polarizations X, Y, each of complex float type.
 *
 * Pre-processor input symbols (some are tied to the execution configuration)
 * Symbol                  | Valid Values            | Description
 * ----------------------- | ----------------------- | -----------
 * NR_STATIONS             | >= 1                    | number of antenna fields
 * NR_SAMPLES_PER_CHANNEL  | multiple of BLOCK_SIZE  | number of input samples per channel
 * NR_CHANNELS             | >= 1                    | number of frequency channels per subband
 * Note that for > 1 channels, NR_CHANNELS-1 channels are actually processed,
 * because the second PPF has "corrupted" channel 0. (An inverse PPF can disambiguate.) \n
 * Note that if NR_CHANNELS is low (esp. 1), these kernels perform poorly.
 * Note that this kernel assumes (but does not use) NR_POLARIZATIONS == 2.
 *
 * Execution configuration:
 */
extern "C" __global__ void coherentStokes(void *outputPtr, const void *inputPtr) 
{
  inputDataType input = (inputDataType) inputPtr;
  outputDataType output = (outputDataType) outputPtr;

  // Define the indexes in the data depending on the block and thread idx
  unsigned channel_idx = threadIdx.x;  // The channels are next to each other in mem 
  unsigned time_idx = threadIdx.y;     //
  unsigned tab_idx = threadIdx.z;   // 

  // Step over the complete time line with INTEGRATION_SIZE steps
  for (unsigned idx_stride = time_idx * (NR_SAMPLES_PER_CHANNEL / TIME_PARALLEL_FACTOR) ; 
                idx_stride < (time_idx + 1) * (NR_SAMPLES_PER_CHANNEL / TIME_PARALLEL_FACTOR);
                idx_stride += INTEGRATION_SIZE)
  {
    // We are integrating all values in the current stride, we need local variable to store
    float stokesI = 0;

#   if NR_COHERENT_STOKES == 4
    float stokesQ = 0;
    float halfStokesU = 0;
    float halfStokesV = 0;
#   endif

    for (unsigned idx_step = 0; idx_step < INTEGRATION_SIZE; idx_step++) 
    {
        //[NR_TABS][NR_POLARIZATIONS][NR_SAMPLES_PER_CHANNEL][NR_CHANNELS]
      float2 X = (*input)[tab_idx][0][idx_stride + idx_step][channel_idx];    
      float2 Y = (*input)[tab_idx][1][idx_stride + idx_step][channel_idx];

      float powerX = X.x * X.x + X.y * X.y;
      float powerY = Y.x * Y.x + Y.y * Y.y;
      stokesI += powerX + powerY;

#     if NR_COHERENT_STOKES == 4
      stokesQ += powerX - powerY;
      halfStokesU += X.x * Y.x + X.y * Y.y;
      halfStokesV += X.y * Y.x - X.x * Y.y;
#     endif
    }
    unsigned write_idx = idx_stride / INTEGRATION_SIZE;
    (*output)[tab_idx][0][write_idx][channel_idx] = stokesI;

#   if NR_COHERENT_STOKES == 4
    (*output)[tab_idx][1][write_idx][channel_idx] = stokesQ;
    (*output)[tab_idx][2][write_idx][channel_idx] = 2 * halfStokesU;
    (*output)[tab_idx][3][write_idx][channel_idx] = 2 * halfStokesV;
#   endif

    

  }
}
