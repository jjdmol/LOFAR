//# BeamFormer.cu
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

// Some defines used to determine the correct way the process the data
#define MAX(A,B) ((A)>(B) ? (A) : (B))

#define NR_PASSES MAX((NR_STATIONS + 6) / 16, 1) // gives best results on GTX 680

#ifndef NR_STATIONS_PER_PASS  // Allow overriding for testing optimalizations 
  #define NR_STATIONS_PER_PASS ((NR_STATIONS + NR_PASSES - 1) / NR_PASSES)
#endif
#if NR_STATIONS_PER_PASS > 32
#error "need more passes to beam for this number of stations"
#endif

// Defines applying additional weighting. Currently used for correcting constant multiplications
// introduced by FFT and iFFT operations: 
#ifndef GLOBAL_WEIGHT_CORRECTION
  #define TIMES_WEIGHT_CORRECTION //empty define: nop
#else 
  #define TIMES_THE_WEIGHT_CORRECTION * GLOBAL_WEIGHT_CORRECTION
#endif
  

// Typedefs used to map input data on arrays
typedef  float2 (*WeightsType)[NR_STATIONS][NR_CHANNELS][NR_TABS];
typedef  float4 (*BandPassCorrectedType)[NR_STATIONS][NR_CHANNELS][NR_SAMPLES_PER_CHANNEL];
typedef  float2 (*ComplexVoltagesType)[NR_CHANNELS][NR_SAMPLES_PER_CHANNEL][NR_TABS][NR_POLARIZATIONS];

/*!
 * Performs beamforming to x beam based.
 * The beamformer performs a complex weighted multiply add of the each sample of the
 * provided input data.
 *
 * \param[out] complexVoltagesPtr      4D output array of beams. For each channel a number of Tied Array Beams time serires is created for two polarizations
 * \param[in]  correctedDataPtr        3D input array of samples. A time series for each station and channel pair. Each sample contains the 2 polarizations X, Y, each of complex float type.
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
extern "C" __global__ void beamFormer( void *complexVoltagesPtr,
                                       const void *samplesPtr,
                                       const void *weightsPtr)
{
  ComplexVoltagesType complexVoltages = (ComplexVoltagesType) complexVoltagesPtr;
  BandPassCorrectedType samples = (BandPassCorrectedType) samplesPtr;
  WeightsType weights = (WeightsType) weightsPtr;

  unsigned pol = threadIdx.x;
  unsigned tab = threadIdx.y;
  unsigned channel =  blockDim.z * blockIdx.z + threadIdx.z;  // The paralellization in the channel is controllable with extra blocks

  float2 sample;
  // This union is in shared memory because it is used by all threads in the block
  __shared__ union { // Union: Maps two variables to the same adress space
    float2 samples[NR_STATIONS_PER_PASS][16][NR_POLARIZATIONS];
    float4 samples4[NR_STATIONS_PER_PASS][16];
  } _local;

 

#pragma unroll
  for (unsigned first_station = 0;  // Step over data with NR_STATIONS_PER_PASS stride
       first_station < NR_STATIONS;
       first_station += NR_STATIONS_PER_PASS) 
  { // this for loop spand the whole file
#if NR_STATIONS_PER_PASS >= 1
    float2 weight_00;                     // assign the weights to register variables
    if (first_station + 0 < NR_STATIONS)  // Number of station might be larger then 32: We 
                                          // the do multiple passes to span all stations
      weight_00 = (*weights)[first_station + 0][channel][tab] TIMES_THE_WEIGHT_CORRECTION; // Get data from global mem
#endif
    // Loop onrolling allows usage of registers for weights
#if NR_STATIONS_PER_PASS >= 2
    float2 weight_01;

    if (first_station + 1 < NR_STATIONS)
      weight_01 = (*weights)[first_station + 1][channel][tab] TIMES_THE_WEIGHT_CORRECTION;
#endif

#if NR_STATIONS_PER_PASS >= 3
    float2 weight_02;

    if (first_station + 2 < NR_STATIONS)
      weight_02 = (*weights)[first_station + 2][channel][tab] TIMES_THE_WEIGHT_CORRECTION;
#endif

#if NR_STATIONS_PER_PASS >= 4
    float2 weight_03;

    if (first_station + 3 < NR_STATIONS)
      weight_03 = (*weights)[first_station + 3][channel][tab] TIMES_THE_WEIGHT_CORRECTION;
#endif

#if NR_STATIONS_PER_PASS >= 5
    float2 weight_04;

    if (first_station + 4 < NR_STATIONS)
      weight_04 = (*weights)[first_station + 4][channel][tab] TIMES_THE_WEIGHT_CORRECTION;
#endif

#if NR_STATIONS_PER_PASS >= 6
    float2 weight_05;

    if (first_station + 5 < NR_STATIONS)
      weight_05 = (*weights)[first_station + 5][channel][tab] TIMES_THE_WEIGHT_CORRECTION;
#endif

#if NR_STATIONS_PER_PASS >= 7
    float2 weight_06;

    if (first_station + 6 < NR_STATIONS)
      weight_06 = (*weights)[first_station + 6][channel][tab] TIMES_THE_WEIGHT_CORRECTION;
#endif

#if NR_STATIONS_PER_PASS >= 8
    float2 weight_07;

    if (first_station + 7 < NR_STATIONS)
      weight_07 = (*weights)[first_station + 7][channel][tab] TIMES_THE_WEIGHT_CORRECTION;
#endif

#if NR_STATIONS_PER_PASS >= 9
    float2 weight_08;

    if (first_station + 8 < NR_STATIONS)
      weight_08 = (*weights)[first_station + 8][channel][tab] TIMES_THE_WEIGHT_CORRECTION;
#endif

#if NR_STATIONS_PER_PASS >= 10
    float2 weight_09;

    if (first_station + 9 < NR_STATIONS)
      weight_09 = (*weights)[first_station + 9][channel][tab] TIMES_THE_WEIGHT_CORRECTION;
#endif

#if NR_STATIONS_PER_PASS >= 11
    float2 weight_10;

    if (first_station + 10 < NR_STATIONS)
      weight_10 = (*weights)[first_station + 10][channel][tab] TIMES_THE_WEIGHT_CORRECTION;
#endif

#if NR_STATIONS_PER_PASS >= 12
    float2 weight_11;

    if (first_station + 11 < NR_STATIONS)
      weight_11 = (*weights)[first_station + 11][channel][tab] TIMES_THE_WEIGHT_CORRECTION;
#endif

#if NR_STATIONS_PER_PASS >= 13
    float2 weight_12;

    if (first_station + 12 < NR_STATIONS)
      weight_12 = (*weights)[first_station + 12][channel][tab] TIMES_THE_WEIGHT_CORRECTION;
#endif

#if NR_STATIONS_PER_PASS >= 14
    float2 weight_13;

    if (first_station + 13 < NR_STATIONS)
      weight_13 = (*weights)[first_station + 13][channel][tab] TIMES_THE_WEIGHT_CORRECTION;
#endif

#if NR_STATIONS_PER_PASS >= 15
    float2 weight_14;

    if (first_station + 14 < NR_STATIONS)
      weight_14 = (*weights)[first_station + 14][channel][tab] TIMES_THE_WEIGHT_CORRECTION;
#endif

#if NR_STATIONS_PER_PASS >= 16
    float2 weight_15;

    if (first_station + 15 < NR_STATIONS)
      weight_15 = (*weights)[first_station + 15][channel][tab] TIMES_THE_WEIGHT_CORRECTION;
#endif

#if NR_STATIONS_PER_PASS >= 17
    float2 weight_16;

    if (first_station + 16 < NR_STATIONS)
      weight_16 = (*weights)[first_station + 16][channel][tab] TIMES_THE_WEIGHT_CORRECTION;
#endif

#if NR_STATIONS_PER_PASS >= 18
    float2 weight_17;

    if (first_station + 17 < NR_STATIONS)
      weight_17 = (*weights)[first_station + 17][channel][tab] TIMES_THE_WEIGHT_CORRECTION;
#endif

#if NR_STATIONS_PER_PASS >= 19
    float2 weight_18;

    if (first_station + 18 < NR_STATIONS)
      weight_18 = (*weights)[first_station + 18][channel][tab] TIMES_THE_WEIGHT_CORRECTION;
#endif

#if NR_STATIONS_PER_PASS >= 20
    float2 weight_19;

    if (first_station + 19 < NR_STATIONS)
      weight_19 = (*weights)[first_station + 19][channel][tab] TIMES_THE_WEIGHT_CORRECTION;
#endif

#if NR_STATIONS_PER_PASS >= 21
    float2 weight_20;

    if (first_station + 20 < NR_STATIONS)
      weight_20 = (*weights)[first_station + 20][channel][tab] TIMES_THE_WEIGHT_CORRECTION;
#endif

#if NR_STATIONS_PER_PASS >= 22
    float2 weight_21;

    if (first_station + 21 < NR_STATIONS)
      weight_21 = (*weights)[first_station + 21][channel][tab] TIMES_THE_WEIGHT_CORRECTION;
#endif

#if NR_STATIONS_PER_PASS >= 23
    float2 weight_22;

    if (first_station + 22 < NR_STATIONS)
      weight_22 = (*weights)[first_station + 22][channel][tab] TIMES_THE_WEIGHT_CORRECTION;
#endif

#if NR_STATIONS_PER_PASS >= 24
    float2 weight_23;

    if (first_station + 23 < NR_STATIONS)
      weight_23 = (*weights)[first_station + 23][channel][tab] TIMES_THE_WEIGHT_CORRECTION;
#endif

#if NR_STATIONS_PER_PASS >= 25
    float2 weight_24;

    if (first_station + 24 < NR_STATIONS)
      weight_24 = (*weights)[first_station + 24][channel][tab] TIMES_THE_WEIGHT_CORRECTION;
#endif

#if NR_STATIONS_PER_PASS >= 26
    float2 weight_25;

    if (first_station + 25 < NR_STATIONS)
      weight_25 = (*weights)[first_station + 25][channel][tab] TIMES_THE_WEIGHT_CORRECTION;
#endif

#if NR_STATIONS_PER_PASS >= 27
    float2 weight_26;

    if (first_station + 26 < NR_STATIONS)
      weight_26 = (*weights)[first_station + 26][channel][tab] TIMES_THE_WEIGHT_CORRECTION;
#endif

#if NR_STATIONS_PER_PASS >= 28
    float2 weight_27;

    if (first_station + 27 < NR_STATIONS)
      weight_27 = (*weights)[first_station + 27][channel][tab] TIMES_THE_WEIGHT_CORRECTION;
#endif

#if NR_STATIONS_PER_PASS >= 29
    float2 weight_28;

    if (first_station + 28 < NR_STATIONS)
      weight_28 = (*weights)[first_station + 28][channel][tab] TIMES_THE_WEIGHT_CORRECTION;
#endif

#if NR_STATIONS_PER_PASS >= 30
    float2 weight_29;

    if (first_station + 29 < NR_STATIONS)
      weight_29 = (*weights)[first_station + 29][channel][tab] TIMES_THE_WEIGHT_CORRECTION;
#endif

#if NR_STATIONS_PER_PASS >= 31
    float2 weight_30;

    if (first_station + 30 < NR_STATIONS)
      weight_30 = (*weights)[first_station + 30][channel][tab] TIMES_THE_WEIGHT_CORRECTION;
#endif

#if NR_STATIONS_PER_PASS >= 32
    float2 weight_31;

    if (first_station + 31 < NR_STATIONS)
      weight_31 = (*weights)[first_station + 31][channel][tab] TIMES_THE_WEIGHT_CORRECTION;
#endif

    // Loop over all the samples in time
    // TODO: This is a candidate to be added as an extra paralellization dim.
    // problem: we already have the x,y and z filled with parallel parameters. Make the polarization implicit?
    for (unsigned time = 0; time < NR_SAMPLES_PER_CHANNEL; time += 16)  // Perform the addition for 16 timesteps
    {
      // Optimized memory transver: Threads load paralel memory
      for (unsigned i = threadIdx.x + NR_POLARIZATIONS * threadIdx.y;
                    i < NR_STATIONS_PER_PASS * 16;
                    i += NR_TABS * NR_POLARIZATIONS) 
      {
        unsigned t = i % 16;
        unsigned s = i / 16;

        if (NR_SAMPLES_PER_CHANNEL % 16 == 0 || time + t < NR_SAMPLES_PER_CHANNEL)
          if (NR_STATIONS % NR_STATIONS_PER_PASS == 0 || first_station + s < NR_STATIONS)
            _local.samples4[0][i] = (*samples)[first_station + s][channel][time + t];
      }

       __syncthreads();


      for (unsigned t = 0; 
                    t < (NR_SAMPLES_PER_CHANNEL % 16 == 0 ? 16 : min(16U, NR_SAMPLES_PER_CHANNEL - time));
                    t++) 
      {
        float2 sum = first_station == 0 ? // The first run the sum should be zero, otherwise we need to take the sum of the previous run
                    make_float2(0,0) :
                    (*complexVoltages)[channel][time + t][tab][pol];

        // Calculate the weighted complex sum of the samples
#if NR_STATIONS_PER_PASS >= 1
        if (first_station + 1 <= NR_STATIONS) {  // Remember that the number of stations might not be a multiple of 32. Skip if station does not exist
          sample = _local.samples[ 0][t][pol];
          sum.x += weight_00.x * sample.x;
          sum.y += weight_00.x * sample.y;
          sum.x += weight_00.y * -sample.y;
          sum.y += weight_00.y * sample.x;
        }
#endif

#if NR_STATIONS_PER_PASS >= 2
        if (first_station + 2 <+ NR_STATIONS) {
          sample = _local.samples[ 1][t][pol];
          sum.x += weight_01.x * sample.x;
          sum.y += weight_01.x * sample.y;
          sum.x += weight_01.y * -sample.y;
          sum.y += weight_01.y * sample.x;
        }
#endif


#if NR_STATIONS_PER_PASS >= 3
        if (first_station + 3 <= NR_STATIONS) {
          sample = _local.samples[ 2][t][pol];
          sum.x += weight_02.x * sample.x;
          sum.y += weight_02.x * sample.y;
          sum.x += weight_02.y * -sample.y;
          sum.y += weight_02.y * sample.x;
        }
#endif

#if NR_STATIONS_PER_PASS >= 4
        if (first_station + 4 <= NR_STATIONS) {
          sample = _local.samples[ 3][t][pol];
          sum.x += weight_03.x * sample.x;
          sum.y += weight_03.x * sample.y;
          sum.x += weight_03.y * -sample.y;
          sum.y += weight_03.y * sample.x;
        }
#endif

#if NR_STATIONS_PER_PASS >= 5
        if (first_station + 5 <= NR_STATIONS) {
          sample = _local.samples[ 4][t][pol];
          sum.x += weight_04.x * sample.x;
          sum.y += weight_04.x * sample.y;
          sum.x += weight_04.y * -sample.y;
          sum.y += weight_04.y * sample.x;
        }
#endif

#if NR_STATIONS_PER_PASS >= 6
        if (first_station + 6 <= NR_STATIONS) {
          sample = _local.samples[ 5][t][pol];
          sum.x += weight_05.x * sample.x;
          sum.y += weight_05.x * sample.y;
          sum.x += weight_05.y * -sample.y;
          sum.y += weight_05.y * sample.x;
        }
#endif

#if NR_STATIONS_PER_PASS >= 7
        if (first_station + 7 <= NR_STATIONS) {
          sample = _local.samples[ 6][t][pol];
          sum.x += weight_06.x * sample.x;
          sum.y += weight_06.x * sample.y;
          sum.x += weight_06.y * -sample.y;
          sum.y += weight_06.y * sample.x;
        }
#endif

#if NR_STATIONS_PER_PASS >= 8
        if (first_station + 8 <= NR_STATIONS) {
          sample = _local.samples[ 7][t][pol];
          sum.x += weight_07.x * sample.x;
          sum.y += weight_07.x * sample.y;
          sum.x += weight_07.y * -sample.y;
          sum.y += weight_07.y * sample.x;
        }
#endif

#if NR_STATIONS_PER_PASS >= 9
        if (first_station + 9 <= NR_STATIONS) {
          sample = _local.samples[ 8][t][pol];
          sum.x += weight_08.x * sample.x;
          sum.y += weight_08.x * sample.y;
          sum.x += weight_08.y * -sample.y;
          sum.y += weight_08.y * sample.x;
        }
#endif

#if NR_STATIONS_PER_PASS >= 10
        if (first_station + 10 <= NR_STATIONS) {
          sample = _local.samples[ 9][t][pol];
          sum.x += weight_09.x * sample.x;
          sum.y += weight_09.x * sample.y;
          sum.x += weight_09.y * -sample.y;
          sum.y += weight_09.y * sample.x;
        }
#endif

#if NR_STATIONS_PER_PASS >= 11
        if (first_station + 11 <= NR_STATIONS) {
          sample = _local.samples[10][t][pol];
          sum.x += weight_10.x * sample.x;
          sum.y += weight_10.x * sample.y;
          sum.x += weight_10.y * -sample.y;
          sum.y += weight_10.y * sample.x;
        }
#endif

#if NR_STATIONS_PER_PASS >= 12
        if (first_station + 12 <= NR_STATIONS) {
          sample = _local.samples[11][t][pol];
          sum.x += weight_11.x * sample.x;
          sum.y += weight_11.x * sample.y;
          sum.x += weight_11.y * -sample.y;
          sum.y += weight_11.y * sample.x;
        }
#endif

#if NR_STATIONS_PER_PASS >= 13
        if (first_station + 13 <= NR_STATIONS) {
          sample = _local.samples[12][t][pol];
          sum.x += weight_12.x * sample.x;
          sum.y += weight_12.x * sample.y;
          sum.x += weight_12.y * -sample.y;
          sum.y += weight_12.y * sample.x;
        }
#endif

#if NR_STATIONS_PER_PASS >= 14
        if (first_station + 14 <= NR_STATIONS) {
          sample = _local.samples[13][t][pol];
          sum.x += weight_13.x * sample.x;
          sum.y += weight_13.x * sample.y;
          sum.x += weight_13.y * -sample.y;
          sum.y += weight_13.y * sample.x;
        }
#endif

#if NR_STATIONS_PER_PASS >= 15
        if (first_station + 15 <= NR_STATIONS) {
          sample = _local.samples[14][t][pol];
          sum.x += weight_14.x * sample.x;
          sum.y += weight_14.x * sample.y;
          sum.x += weight_14.y * -sample.y;
          sum.y += weight_14.y * sample.x;
        }
#endif

#if NR_STATIONS_PER_PASS >= 16
        if (first_station + 16 <= NR_STATIONS) {
          sample = _local.samples[15][t][pol];
          sum.x += weight_15.x * sample.x;
          sum.y += weight_15.x * sample.y;
          sum.x += weight_15.y * -sample.y;
          sum.y += weight_15.y * sample.x;
        }
#endif

#if NR_STATIONS_PER_PASS >= 17
        if (first_station + 17 <= NR_STATIONS) {
          sample = _local.samples[16][t][pol];
          sum.x += weight_16.x * sample.x;
          sum.y += weight_16.x * sample.y;
          sum.x += weight_16.y * -sample.y;
          sum.y += weight_16.y * sample.x;
        }
#endif

#if NR_STATIONS_PER_PASS >= 18
        if (first_station + 18 <= NR_STATIONS) {
          sample = _local.samples[17][t][pol];
          sum.x += weight_17.x * sample.x;
          sum.y += weight_17.x * sample.y;
          sum.x += weight_17.y * -sample.y;
          sum.y += weight_17.y * sample.x;
        }
#endif

#if NR_STATIONS_PER_PASS >= 19
        if (first_station + 19 <= NR_STATIONS) {
          sample = _local.samples[18][t][pol];
          sum.x += weight_18.x * sample.x;
          sum.y += weight_18.x * sample.y;
          sum.x += weight_18.y * -sample.y;
          sum.y += weight_18.y * sample.x;
        }
#endif

#if NR_STATIONS_PER_PASS >= 20
        if (first_station + 20 <= NR_STATIONS) {
          sample = _local.samples[19][t][pol];
          sum.x += weight_19.x * sample.x;
          sum.y += weight_19.x * sample.y;
          sum.x += weight_19.y * -sample.y;
          sum.y += weight_19.y * sample.x;
        }
#endif

#if NR_STATIONS_PER_PASS >= 21
        if (first_station + 21 <= NR_STATIONS) {
          sample = _local.samples[20][t][pol];
          sum.x += weight_20.x * sample.x;
          sum.y += weight_20.x * sample.y;
          sum.x += weight_20.y * -sample.y;
          sum.y += weight_20.y * sample.x;
        }
#endif

#if NR_STATIONS_PER_PASS >= 22
        if (first_station + 22 <= NR_STATIONS) {
          sample = _local.samples[21][t][pol];
          sum.x += weight_21.x * sample.x;
          sum.y += weight_21.x * sample.y;
          sum.x += weight_21.y * -sample.y;
          sum.y += weight_21.y * sample.x;
        }
#endif

#if NR_STATIONS_PER_PASS >= 23
        if (first_station + 23 <= NR_STATIONS) {
          sample = _local.samples[22][t][pol];
          sum.x += weight_22.x * sample.x;
          sum.y += weight_22.x * sample.y;
          sum.x += weight_22.y * -sample.y;
          sum.y += weight_22.y * sample.x;
        }
#endif

#if NR_STATIONS_PER_PASS >= 24
        if (first_station + 24 <= NR_STATIONS) {
          sample = _local.samples[23][t][pol];
          sum.x += weight_23.x * sample.x;
          sum.y += weight_23.x * sample.y;
          sum.x += weight_23.y * -sample.y;
          sum.y += weight_23.y * sample.x;
        }
#endif

#if NR_STATIONS_PER_PASS >= 25
        if (first_station + 25 <= NR_STATIONS) {
          sample = _local.samples[24][t][pol];
          sum.x += weight_24.x * sample.x;
          sum.y += weight_24.x * sample.y;
          sum.x += weight_24.y * -sample.y;
          sum.y += weight_24.y * sample.x;
        }
#endif

#if NR_STATIONS_PER_PASS >= 26
        if (first_station + 26 <= NR_STATIONS) {
          sample = _local.samples[25][t][pol];
          sum.x += weight_25.x * sample.x;
          sum.y += weight_25.x * sample.y;
          sum.x += weight_25.y * -sample.y;
          sum.y += weight_25.y * sample.x;
        }
#endif

#if NR_STATIONS_PER_PASS >= 27
        if (first_station + 27 <= NR_STATIONS) {
          sample = _local.samples[26][t][pol];
          sum.x += weight_26.x * sample.x;
          sum.y += weight_26.x * sample.y;
          sum.x += weight_26.y * -sample.y;
          sum.y += weight_26.y * sample.x;
        }
#endif

#if NR_STATIONS_PER_PASS >= 28
        if (first_station + 28 <= NR_STATIONS) {
          sample = _local.samples[27][t][pol];
          sum.x += weight_27.x * sample.x;
          sum.y += weight_27.x * sample.y;
          sum.x += weight_27.y * -sample.y;
          sum.y += weight_27.y * sample.x;
        }
#endif

#if NR_STATIONS_PER_PASS >= 29
        if (first_station + 29 <= NR_STATIONS) {
          sample = _local.samples[28][t][pol];
          sum.x += weight_28.x * sample.x;
          sum.y += weight_28.x * sample.y;
          sum.x += weight_28.y * -sample.y;
          sum.y += weight_28.y * sample.x;
        }
#endif

#if NR_STATIONS_PER_PASS >= 30
        if (first_station + 30 <= NR_STATIONS) {
          sample = _local.samples[29][t][pol];
          sum.x += weight_29.x * sample.x;
          sum.y += weight_29.x * sample.y;
          sum.x += weight_29.y * -sample.y;
          sum.y += weight_29.y * sample.x;
        }
#endif

#if NR_STATIONS_PER_PASS >= 31
        if (first_station + 31 <= NR_STATIONS) {
          sample = _local.samples[30][t][pol];
          sum.x += weight_30.x * sample.x;
          sum.y += weight_30.x * sample.y;
          sum.x += weight_30.y * -sample.y;
          sum.y += weight_30.y * sample.x;
        }
#endif

#if NR_STATIONS_PER_PASS >= 32
        if (first_station + 32 <= NR_STATIONS) {
          sample = _local.samples[31][t][pol];
          sum.x += weight_31.x * sample.x;
          sum.y += weight_31.x * sample.y;
          sum.x += weight_31.y * -sample.y;
          sum.y += weight_31.y * sample.x;
        }
#endif
        // Write data to global mem
        (*complexVoltages)[channel][time + t][tab][pol] = sum;
      }

      __syncthreads();
    }
  }
}

