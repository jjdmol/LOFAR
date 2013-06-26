//# Transepose.cu
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
 *
 * COPY PASTA OF BEAMFORMER DOCUMENTATION 
 *
 *
 *
 * Performs beamforming to x beam based.
 * The beamformer performs a complex weighted multiply add of the each sample of the
 * provided input data.
 *
 * \param[out] transposedDataPtr      4D output array of beams. For each channel a number of Tied Array Beams time serires is created for two polarizations
 * \param[in]  complexVoltagesPtr        3D input array of samples. A time series for each station and channel pair. Each sample contains the 2 polarizations X, Y, each of complex float type.
 * \param[in]  weightsPtr              3d input array of complex valued weights to be applied to the correctData samples. THere is a weight for each station, channel and Tied Array Beam triplet.
 * Pre-processor input symbols (some are tied to the execution configuration)
 * Symbol                  | Valid Values            | Description
 * ----------------------- | ----------------------- | -----------
 * NR_STATIONS             | >= 1                    | number of antenna fields
 * NR_SAMPLES_PER_CHANNEL  | >= 1                    | number of input samples per channel
 * NR_CHANNELS             | >= 1                    | number of frequency channels per subband
 * NR_TABS                 | >= 1                    | number of Tied Array Beams to create
 * ----------------------- | ------------------------| 
 * NR_STATIONS_PER_PASS    | 1 >= && <= 32           | Set to overide default: Parallelization parameter, controls the number stations to beamform in a single pass over the input data. 
 *
 * Note that this kernel assumes  NR_POLARIZATIONS == 2 and COMPLEX == 2
 *
 * Execution configuration:
 * - LocalWorkSize = (NR_POLARIZATIONS, NR_TABS, NR_CHANNELS) Note that for full utilization NR_TABS * NR_CHANNELS % 16 = 0
 */

#ifdef CHANNEL_PARALLEL 
typedef float2 (*ComplexVoltagesType)[NR_CHANNELS][NR_SAMPLES_PER_CHANNEL][NR_TABS][NR_POLARIZATIONS];
typedef float2 (*TransposedDataType)[NR_TABS][NR_POLARIZATIONS][NR_SAMPLES_PER_CHANNEL][NR_CHANNELS];

extern "C" __global__  void transpose( 
                    void * transposedDataPtr,
                    const void * complexVoltagesPtr)
{
  TransposedDataType transposedData = (TransposedDataType) transposedDataPtr;
  ComplexVoltagesType complexVoltages = (ComplexVoltagesType) complexVoltagesPtr;

  __shared__ float2 tmp[16][17][2]; // add one to get coaliesced reads?

  unsigned tabBase = 16 * blockIdx.y; // No use of the block size!!!
  unsigned chBase = 16 * blockIdx.z;

  unsigned tabOffsetR = threadIdx.x & 15;
  unsigned tabR = tabBase + tabOffsetR;
  unsigned chOffsetR = threadIdx.x >> 4;
  unsigned channelR = chBase + chOffsetR;
  bool doR = NR_TABS % 16 == 0 || tabR < NR_TABS;

  unsigned tabOffsetW = threadIdx.x >> 4;
  unsigned tabW = tabBase + tabOffsetW;
  unsigned chOffsetW = threadIdx.x & 15;
  unsigned channelW = chBase + chOffsetW;
  bool doW = NR_TABS % 16 == 0 || tabW < NR_TABS;

  for (int time = 0; time < NR_SAMPLES_PER_CHANNEL; time++) 
  {
    if (doR)  // only do a read and write if we are within our bounds
    {    
      tmp[tabOffsetR][chOffsetR][0] = (*complexVoltages)[channelR][time][tabR][0];
      tmp[tabOffsetR][chOffsetR][1] = (*complexVoltages)[channelR][time][tabR][1];
    }
    __syncthreads();
    if (doW) 
    {
      float2 sample = tmp[tabOffsetW][chOffsetW][0];
      float2 sample2 = tmp[tabOffsetW][chOffsetW][1];
      (*transposedData)[tabW][0][time][channelW] = sample;
      (*transposedData)[tabW][1][time][channelW] = sample2;
    }

    __syncthreads();
  }
}

#else

typedef  float2 (*ComplexVoltagesType)[NR_CHANNELS][NR_SAMPLES_PER_CHANNEL][NR_TABS][NR_POLARIZATIONS]; 
typedef  float2 (*TransposedDataType)[NR_TABS][NR_POLARIZATIONS][NR_CHANNELS][NR_SAMPLES_PER_CHANNEL];

extern "C" __global__  void transpose( void * transposedDataPtr,
                                        const void * complexVoltagesPtr)
{
  TransposedDataType transposedData = (TransposedDataType) transposedDataPtr;
  ComplexVoltagesType complexVoltages = (ComplexVoltagesType) complexVoltagesPtr;

  __shared__ float2 tmp[16][17][2];

  unsigned tabBase = 16 * blockDim.y * blockIdx.y + threadIdx.y;
  unsigned timeBase = 16 * blockDim.z * blockIdx.z + threadIdx.z;

  unsigned tabOffsetR = threadIdx.x & 15;   // use and to get module 16
  unsigned tabR = tabBase + tabOffsetR;
  unsigned timeOffsetR = threadIdx.x >> 4;  // use bitshift to get devision
  unsigned timeR = timeBase + timeOffsetR;
  bool doR = NR_TABS % 16 == 0 || tabR < NR_TABS;

  unsigned tabOffsetW = threadIdx.x >> 4;
  unsigned tabW = tabBase + tabOffsetW;
  unsigned timeOffsetW = threadIdx.x & 15;
  unsigned timeW = timeBase + timeOffsetW;
  bool doW = NR_TABS % 16 == 0 || tabW < NR_TABS;

  for (int channel = 0; channel < NR_CHANNELS; channel++) 
  {
    if (doR)
    {
      tmp[tabOffsetR][timeOffsetR][0] = (*complexVoltages)[channel][timeR][tabR][0];
      tmp[tabOffsetR][timeOffsetR][1] = (*complexVoltages)[channel][timeR][tabR][1];
    }

    __syncthreads();
    if (doW) {
      float2 sample = tmp[tabOffsetW][timeOffsetW][0];
      float2 sample2 = tmp[tabOffsetW][timeOffsetW][1];
      (*transposedData)[tabW][0][channel][timeW] = sample;
      (*transposedData)[tabW][1][channel][timeW] = sample2;
    }

    __syncthreads();
  }
}

#endif
