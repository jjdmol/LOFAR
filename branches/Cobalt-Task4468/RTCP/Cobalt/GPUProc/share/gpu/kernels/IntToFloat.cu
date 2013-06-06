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

#if NR_BITS_PER_SAMPLE == 16
typedef short2 SampleType;
__device__ float convertIntToFloat(short x)
{
	return x;
}
#elif NR_BITS_PER_SAMPLE == 8
__device__ float convertIntToFloat(char x)
{
	return x==-128 ? -127 : x;
}
typedef char2 SampleType;
#else
#error unsupport NR_BITS_PER_SAMPLE
#endif

typedef  SampleType (*SampledDataType)[NR_STATIONS][NR_SAMPLES_PER_SUBBAND][NR_POLARIZATIONS];
typedef  float2 (*ConvertedDataType)[NR_STATIONS][NR_POLARIZATIONS][NR_SAMPLES_PER_SUBBAND];



extern "C" {
 __global__ void intToFloat( void * convertedDataPtr,
                          const void * sampledDataPtr)
{
  ConvertedDataType convertedData = (ConvertedDataType) convertedDataPtr;
  SampledDataType sampledData = (SampledDataType) sampledDataPtr;

  uint station = blockIdx.y * blockDim.y + threadIdx.y;
  for (uint time = threadIdx.x; time < NR_SAMPLES_PER_SUBBAND; time += blockDim.x) {
    (*convertedData)[station][0][time] = make_float2(
			convertIntToFloat((*sampledData)[station][time][0].x),
            convertIntToFloat((*sampledData)[station][time][0].y));
    (*convertedData)[station][1][time] = make_float2(
			convertIntToFloat((*sampledData)[station][time][1].x), 
            convertIntToFloat((*sampledData)[station][time][1].y));
    
  }
}
}

