//# IntToFloat.cl
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

#include "IntToFloat.cuh"

#if NR_BITS_PER_SAMPLE == 16
typedef short2 SampleType;
#elif NR_BITS_PER_SAMPLE == 8
typedef char2 SampleType;
#else
#error unsupport NR_BITS_PER_SAMPLE
#endif

typedef  SampleType (*SampledDataType)[NR_STATIONS][NR_SAMPLES_PER_SUBBAND][NR_POLARIZATIONS];
typedef  float2 (*ConvertedDataType)[NR_STATIONS][NR_POLARIZATIONS][NR_SAMPLES_PER_SUBBAND];

/**
 * This kernel performs a conversion of the integer valued input to floats.
 * - It supports both 16 and 8 bits (short and char) input selectable using
 *   the define NR_BITS_PER_SAMPLE
 * - In 8 bit mode the converted chars with value -128 are clamped to a minimum of -127 
 *
 * @param[out] correctedDataPtr    pointer to output data of ::ConvertedDataType,
 *                                 a 4D array [station][polarizations][n_samples_subband][complex]
 *                                 of floats (2 complex polarizations)
 * @param[in]  SampledDataType     pointer to input data; this can either be a
 *                                 4D array [station][n_samples_subband][polarizations][complex]
 *                                 of shorts or chars. depending on NR_BITS_PER_SAMPLE.
 */

extern "C" {
 __global__ void intToFloat( void * convertedDataPtr,
                          const void * sampledDataPtr)
{
  ConvertedDataType convertedData = (ConvertedDataType) convertedDataPtr;
  SampledDataType sampledData = (SampledDataType) sampledDataPtr;
  // Use the y dim for selecting the station. blockDim.y is normally 1
  uint station = blockIdx.y * blockDim.y + threadIdx.y;
  
  // Step data with whole blocks allows for coalesced reads and writes
  for (uint time = threadIdx.x; time < NR_SAMPLES_PER_SUBBAND; time += blockDim.x) {
    // pol 1
    (*convertedData)[station][0][time] = make_float2(
			convertIntToFloat((*sampledData)[station][time][0].x),
            convertIntToFloat((*sampledData)[station][time][0].y));
    // pol 2
    (*convertedData)[station][1][time] = make_float2(
			convertIntToFloat((*sampledData)[station][time][1].x), 
            convertIntToFloat((*sampledData)[station][time][1].y));
    // TODO: Is a sync needed here? Dont think so but..
  }
}
}


