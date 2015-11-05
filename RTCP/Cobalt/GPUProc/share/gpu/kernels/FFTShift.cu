//# FFTShift.cu: multyply odd samples with -1 for correct fft functionality
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

#include "gpu_math.cuh"


#if !(NR_SAMPLES_PER_CHANNEL >= 1)
#error Precondition violated: NR_SAMPLES_PER_CHANNEL >= 1
#endif

#if !(NR_STATIONS >= 1)
#error Precondition violated: NR_STATIONS >= 1
#endif

#if !(NR_POLARIZATIONS == 2)
#error Precondition violated: NR_POLARIZATIONS == 2
#endif

#if !(NR_CHANNELS >= 1)
#error Precondition violated: NR_CHANNELS >= 1
#endif

typedef float2(*DataType)[NR_STATIONS][NR_POLARIZATIONS][NR_CHANNELS][NR_SAMPLES_PER_CHANNEL];

/**
 * Shift the zero-frequency component to the center of the spectrum.
 * This kernel swaps the half-spaces of the channel dimension 
 * so that the negative frequencies are placed to
 * the left of the positive frequencies. We do this by modulating the samples
 * with exp(-j*pi), which results in a shift over pi in the frequency
 * domain. More information can be found in any decent book on digital signal
 * processing. 
 * @param[data] a 4-D array
 *              [station][polarizations][nr_channels][n_samples_channel]
 *              of complex floats.
 *
 * Required preprocessor symbols:
 * - NR_SAMPLES_PER_CHANNEL: > 0
 * - NR_STATIONS           : > 0
 * - NR_POLARIZATIONS      : ==2
 * - NR_CHANNELS           : > 0
 *
 * Execution configuration:
 * - Use a 3D thread block. (sample, station, channel) size < 1024.
 * - Use a 3D grid dim. (sample, station, channel) channel < 64
 */

extern "C" {
__global__ void FFTShift(DataType data)
{
  unsigned sample  = blockIdx.x * blockDim.x + threadIdx.x;
  unsigned station = blockIdx.y * blockDim.y + threadIdx.y;
  unsigned channel = blockIdx.z * blockDim.z + threadIdx.z;

  // Set the odd samples 
  signed factor = 1 - 2 * (sample % 2);  //multiplication that results in -1 or
              // odd samples (faster then an if statement)
  (*data)[station][0][channel][sample] = 
                            (*data)[station][0][channel][sample] * factor;
  (*data)[station][1][channel][sample] = 
                            (*data)[station][1][channel][sample] * factor;

}
}
