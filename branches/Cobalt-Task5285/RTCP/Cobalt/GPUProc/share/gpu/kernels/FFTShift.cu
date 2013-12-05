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

typedef float2 (*OuputDataType)[NR_STATIONS][NR_POLARIZATIONS][NR_CHANNELS][NR_SAMPLES_PER_SUBBAND];
typedef float2 (*InputDataType)[NR_STATIONS][NR_POLARIZATIONS][NR_CHANNELS][NR_SAMPLES_PER_SUBBAND];

/**
 * This kernel performs a conversion of the integer valued input to floats and
 * transposes the data to get per station: first all samples with polX, then polY.
 * - It supports 8 and 16 bit (char and short) input, which is selectable using
 *   the define NR_BITS_PER_SAMPLE
 * - In 8 bit mode the converted samples with value -128 are clamped to -127.0f
 *
 * @param[out] convertedDataPtr    pointer to output data of ConvertedDataType,
 *                                 a 4D array [station][polarizations][n_samples_subband][complex]
 *                                 of floats (2 complex polarizations).
 * @param[in]  sampledDataPtr      pointer to input data; this can either be a
 *                                 4D array [station][n_samples_subband][polarizations][complex]
 *                                 of shorts or chars, depending on NR_BITS_PER_SAMPLE.
 *
 * Required preprocessor symbols:
 * - NR_SAMPLES_PER_CHANNEL: > 0
 * - NR_BITS_PER_SAMPLE: 8 or 16
 *
 * Execution configuration:
 * - Use a 1D thread block. No restrictions.
 * - Use a 2D grid dim, where the x dim has 1 block and the y dim represents the
 *   number of stations (i.e. antenna fields).
 */

extern "C" {
__global__ void FFTShift(void *outputDataPtr,
                           const void *inputDataPtr)
{
  InputDataType input = (InputDataType)inputDataPtr;
  OuputDataType output = (OuputDataType)outputDataPtr;

  // fasted dims
  unsigned sample        = blockIdx.x * blockDim.x + threadIdx.x;
  unsigned station       = blockIdx.y * blockDim.y + threadIdx.y;
  unsigned channel      = blockIdx.z * blockDim.z + threadIdx.z;

  //if (false)
  //if (sample % 2 != 0) // if an odd sample
  //{
  //  float2 pol0 = (*input)[station][0][channel][sample];
  //  float2 pol1 = (*input)[station][1][channel][sample];
  //  (*output)[station][0][channel][sample] = make_float2(0,0); // *-1.0f;
  //  (*output)[station][1][channel][sample] = make_float2(0, 0); // *-1.0f;
  //}



}

}

