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

typedef float2 *DataType;

/**
 * Shift the zero-frequency component to the center of the spectrum.
 * This kernel swaps the half-spaces of the channel dimension 
 * so that the negative frequencies are placed to
 * the left of the positive frequencies. We do this by modulating the samples
 * with exp(-j*pi), which results in a shift over pi in the frequency
 * domain. More information can be found in any decent book on digital signal
 * processing. 
 *
 * @param[data] a multi-dimensional array with time samples of type complex
 * float in the last dimension.
 * @pre @c The number of data samples must be a multiple of the maximum number
 * of threads per block, typically 1024.
 * @note We will squash the multi-dimensional array to one dimension for reasons
 * of flexibility, because the size of the other dimensions (usually @c[channel]
 * and @c[station]) can vary wildly.
 */

extern "C"
{
  __global__ void FFTShift(DataType data)
  {
    unsigned sample  = blockIdx.x * blockDim.x + threadIdx.x;

    // Multiplication factor: 1 for even samples, -1 for odd samples
    signed factor = 1 - 2 * (sample % 2); 
    data[sample] = data[sample] * factor;
  }
}
