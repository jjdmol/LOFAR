
#include "cuda_runtime.h"
#include <cuda.h>


#define COMPLEX 2       // do not change
#define NR_BITS_PER_SAMPLE 16
#define NR_STATION_FILTER_TAPS  16
#define USE_NEW_CORRELATOR
#define NR_POLARIZATIONS         2 
#define NR_TAPS                 16

#define NR_STATIONS 20
#define NR_SAMPLES_PER_CHANNEL 100
#define NR_CHANNELS 16

#if NR_BITS_PER_SAMPLE == 16
typedef signed short SampleType;
#elif NR_BITS_PER_SAMPLE == 8
typedef signed char SampleType;
#else
#error unsupported NR_BITS_PER_SAMPLE
#endif


typedef SampleType (*SampledDataType)[NR_STATIONS][NR_TAPS - 1 + NR_SAMPLES_PER_CHANNEL][NR_CHANNELS][NR_POLARIZATIONS * COMPLEX];
typedef float (*FilteredDataType)[NR_STATIONS][NR_POLARIZATIONS][NR_SAMPLES_PER_CHANNEL][NR_CHANNELS][COMPLEX];
typedef const float (*WeightsType)[NR_CHANNELS][16];


/*!
 * Applies the Finite Input Response filter defined by the weightsPtr array
 * to the sampledDataPtr array. Output is written into the filteredDataPtr
 * array. The filter works on complex numbers. The weights are real values only.
 *
 * Input values are first converted to (complex) float.
 * The kernel also reorders the polarization dimension and expects the weights
 * per channel in reverse order. If an FFT is applied afterwards, the weights
 * of the odd channels are often supplied negated to get the resulting channels
 * in increasing order of frequency.
 *
 * \param[out] filteredDataPtr         4D output array of floats
 * \param[in]  sampledDataPtr          4D input array of signed chars or shorts
 * \param[in]  weightsPtr              2D per-channel FIR filter coefficient array of floats (considering float16 as a dim)
 *
 * Pre-processor input symbols (some are tied to the execution configuration)
 * Symbol                  | Valid Values                | Description
 * ----------------------- | --------------------------- | -----------
 * NR_STATIONS             | >= 1                        | number of antenna fields
 * NR_TAPS                 | 1--16                       | number of FIR filtering coefficients
 * NR_SAMPLES_PER_CHANNEL  | multiple of NR_TAPS and > 0 | number of input samples per channel
 * NR_BITS_PER_SAMPLE      | 8 or 16                     | number of bits of signed integral value type of sampledDataPtr (TODO: support 4)
 * NR_CHANNELS             | multiple of 16 and > 0      | number of frequency channels per subband
 * NR_POLARIZATIONS        | power of 2                  | number of polarizations
 *
 * Execution configuration: (TODO: enforce using __attribute__ reqd_work_group_size)
 * - Work dim == 2  (can be 1 iff NR_STATIONS == 1)
 *     + Inner dim: the channel, pol, real/imag the thread processes
 *     + Outer dim: the station the thread processes
 * - Work group size: must divide global size, no other kernel restrictions
 * - Global size: (NR_CHANNELS * NR_POLARIZATIONS * 2, NR_STATIONS)
 *
 * TODO: convert complex dim to fcomplex (=float2 in math.cl) in device code and to complex<float> in host code.
 */
__global__ void FIR_filter( void *filteredDataPtr,
                          const void *sampledDataPtr,
                          const void *weightsPtr)
{
  SampledDataType sampledData = (SampledDataType) sampledDataPtr;
  FilteredDataType filteredData = (FilteredDataType) filteredDataPtr;
  WeightsType weightsData = (WeightsType) weightsPtr;

  unsigned cpr = blockIdx.x*blockDim.x+threadIdx.x;
#if 0
  // Straight index calc for NR_CHANNELS == 1
  uint pol_ri = cpr & 3;
  uint channel = cpr >> 2;
  uint ri = cpr & 1;
  uint pol = pol_ri >> 1;
#else
  unsigned ri = cpr & 1;
  unsigned channel = (cpr >> 1) % NR_CHANNELS;
  unsigned pol = (cpr >> 1) / NR_CHANNELS;
  unsigned pol_ri = (pol << 1) | ri;
#endif
  unsigned station = blockIdx.y*blockDim.y+threadIdx.y;

  //const float16 weights = (*weightsData)[channel];
  const float weights_s0 = (*weightsData)[channel][0];
  const float weights_s1 = (*weightsData)[channel][1];
  const float weights_s2 = (*weightsData)[channel][2];
  const float weights_s3 = (*weightsData)[channel][3];
  const float weights_s4 = (*weightsData)[channel][4];
  const float weights_s5 = (*weightsData)[channel][5];
  const float weights_s6 = (*weightsData)[channel][6];
  const float weights_s7 = (*weightsData)[channel][7];
  const float weights_s8 = (*weightsData)[channel][8];
  const float weights_s9 = (*weightsData)[channel][9];
  const float weights_sA = (*weightsData)[channel][10];
  const float weights_sB = (*weightsData)[channel][11];
  const float weights_sC = (*weightsData)[channel][12];
  const float weights_sD = (*weightsData)[channel][13];
  const float weights_sE = (*weightsData)[channel][14];
  const float weights_sF = (*weightsData)[channel][15];

  //float16 delayLine;
  float delayLine_s0, delayLine_s1, delayLine_s2, delayLine_s3, 
        delayLine_s4, delayLine_s5, delayLine_s6, delayLine_s7, 
        delayLine_s8, delayLine_s9, delayLine_sA, delayLine_sB,
        delayLine_sC, delayLine_sD, delayLine_sE, delayLine_sF;
  

  delayLine_s0 = (*sampledData)[station][0][channel][pol_ri];
  delayLine_s1 = (*sampledData)[station][1][channel][pol_ri];
  delayLine_s2 = (*sampledData)[station][2][channel][pol_ri];
  delayLine_s3 = (*sampledData)[station][3][channel][pol_ri];
  delayLine_s4 = (*sampledData)[station][4][channel][pol_ri];
  delayLine_s5 = (*sampledData)[station][5][channel][pol_ri];
  delayLine_s6 = (*sampledData)[station][6][channel][pol_ri];
  delayLine_s7 = (*sampledData)[station][7][channel][pol_ri];
  delayLine_s8 = (*sampledData)[station][8][channel][pol_ri];
  delayLine_s9 = (*sampledData)[station][9][channel][pol_ri];
  delayLine_sA = (*sampledData)[station][10][channel][pol_ri];
  delayLine_sB = (*sampledData)[station][11][channel][pol_ri];
  delayLine_sC = (*sampledData)[station][12][channel][pol_ri];
  delayLine_sD = (*sampledData)[station][13][channel][pol_ri];
  delayLine_sE = (*sampledData)[station][14][channel][pol_ri];
  

  float sum_s0, sum_s1, sum_s2, sum_s3,
        sum_s4, sum_s5, sum_s6, sum_s7,
        sum_s8, sum_s9, sum_sA, sum_sB,
        sum_sC, sum_sD, sum_sE, sum_sF;

  for (unsigned time = 0; time < NR_SAMPLES_PER_CHANNEL; time += NR_TAPS) 
  {
    delayLine_sF = (*sampledData)[station][time + NR_TAPS - 1 + 0][channel][pol_ri];
    sum_s0 = weights_sF * delayLine_s0;
    delayLine_s0 = (*sampledData)[station][time + NR_TAPS - 1 + 1][channel][pol_ri];
    sum_s0 += weights_sE * delayLine_s1;
    sum_s0 += weights_sD * delayLine_s2;
    sum_s0 += weights_sC * delayLine_s3;
    sum_s0 += weights_sB * delayLine_s4;
    sum_s0 += weights_sA * delayLine_s5;
    sum_s0 += weights_s9 * delayLine_s6;
    sum_s0 += weights_s8 * delayLine_s7;
    sum_s0 += weights_s7 * delayLine_s8;
    sum_s0 += weights_s6 * delayLine_s9;
    sum_s0 += weights_s5 * delayLine_sA;
    sum_s0 += weights_s4 * delayLine_sB;
    sum_s0 += weights_s3 * delayLine_sC;
    sum_s0 += weights_s2 * delayLine_sD;
    sum_s0 += weights_s1 * delayLine_sE;
    sum_s0 += weights_s0 * delayLine_sF;
    (*filteredData)[station][pol][time + 0][channel][ri] = sum_s0;

    sum_s1 = weights_sF * delayLine_s1;
    delayLine_s1 = (*sampledData)[station][time + NR_TAPS - 1 + 2][channel][pol_ri];
    sum_s1 += weights_sE * delayLine_s2;
    sum_s1 += weights_sD * delayLine_s3;
    sum_s1 += weights_sC * delayLine_s4;
    sum_s1 += weights_sB * delayLine_s5;
    sum_s1 += weights_sA * delayLine_s6;
    sum_s1 += weights_s9 * delayLine_s7;
    sum_s1 += weights_s8 * delayLine_s8;
    sum_s1 += weights_s7 * delayLine_s9;
    sum_s1 += weights_s6 * delayLine_sA;
    sum_s1 += weights_s5 * delayLine_sB;
    sum_s1 += weights_s4 * delayLine_sC;
    sum_s1 += weights_s3 * delayLine_sD;
    sum_s1 += weights_s2 * delayLine_sE;
    sum_s1 += weights_s1 * delayLine_sF;
    sum_s1 += weights_s0 * delayLine_s0;
    (*filteredData)[station][pol][time + 1][channel][ri] = sum_s1;

    sum_s2 = weights_sF * delayLine_s2;
    delayLine_s2 = (*sampledData)[station][time + NR_TAPS - 1 + 3][channel][pol_ri];
    sum_s2 += weights_sE * delayLine_s3;
    sum_s2 += weights_sD * delayLine_s4;
    sum_s2 += weights_sC * delayLine_s5;
    sum_s2 += weights_sB * delayLine_s6;
    sum_s2 += weights_sA * delayLine_s7;
    sum_s2 += weights_s9 * delayLine_s8;
    sum_s2 += weights_s8 * delayLine_s9;
    sum_s2 += weights_s7 * delayLine_sA;
    sum_s2 += weights_s6 * delayLine_sB;
    sum_s2 += weights_s5 * delayLine_sC;
    sum_s2 += weights_s4 * delayLine_sD;
    sum_s2 += weights_s3 * delayLine_sE;
    sum_s2 += weights_s2 * delayLine_sF;
    sum_s2 += weights_s1 * delayLine_s0;
    sum_s2 += weights_s0 * delayLine_s1;
    (*filteredData)[station][pol][time + 2][channel][ri] = sum_s2;

    sum_s3 = weights_sF * delayLine_s3;
    delayLine_s3 = (*sampledData)[station][time + NR_TAPS - 1 + 4][channel][pol_ri];
    sum_s3 += weights_sE * delayLine_s4;
    sum_s3 += weights_sD * delayLine_s5;
    sum_s3 += weights_sC * delayLine_s6;
    sum_s3 += weights_sB * delayLine_s7;
    sum_s3 += weights_sA * delayLine_s8;
    sum_s3 += weights_s9 * delayLine_s9;
    sum_s3 += weights_s8 * delayLine_sA;
    sum_s3 += weights_s7 * delayLine_sB;
    sum_s3 += weights_s6 * delayLine_sC;
    sum_s3 += weights_s5 * delayLine_sD;
    sum_s3 += weights_s4 * delayLine_sE;
    sum_s3 += weights_s3 * delayLine_sF;
    sum_s3 += weights_s2 * delayLine_s0;
    sum_s3 += weights_s1 * delayLine_s1;
    sum_s3 += weights_s0 * delayLine_s2;
    (*filteredData)[station][pol][time + 3][channel][ri] = sum_s3;

    sum_s4 = weights_sF * delayLine_s4;
    delayLine_s4 = (*sampledData)[station][time + NR_TAPS - 1 + 5][channel][pol_ri];
    sum_s4 += weights_sE * delayLine_s5;
    sum_s4 += weights_sD * delayLine_s6;
    sum_s4 += weights_sC * delayLine_s7;
    sum_s4 += weights_sB * delayLine_s8;
    sum_s4 += weights_sA * delayLine_s9;
    sum_s4 += weights_s9 * delayLine_sA;
    sum_s4 += weights_s8 * delayLine_sB;
    sum_s4 += weights_s7 * delayLine_sC;
    sum_s4 += weights_s6 * delayLine_sD;
    sum_s4 += weights_s5 * delayLine_sE;
    sum_s4 += weights_s4 * delayLine_sF;
    sum_s4 += weights_s3 * delayLine_s0;
    sum_s4 += weights_s2 * delayLine_s1;
    sum_s4 += weights_s1 * delayLine_s2;
    sum_s4 += weights_s0 * delayLine_s3;
    (*filteredData)[station][pol][time + 4][channel][ri] = sum_s4;

    sum_s5 = weights_sF * delayLine_s5;
    delayLine_s5 = (*sampledData)[station][time + NR_TAPS - 1 + 6][channel][pol_ri];
    sum_s5 += weights_sE * delayLine_s6;
    sum_s5 += weights_sD * delayLine_s7;
    sum_s5 += weights_sC * delayLine_s8;
    sum_s5 += weights_sB * delayLine_s9;
    sum_s5 += weights_sA * delayLine_sA;
    sum_s5 += weights_s9 * delayLine_sB;
    sum_s5 += weights_s8 * delayLine_sC;
    sum_s5 += weights_s7 * delayLine_sD;
    sum_s5 += weights_s6 * delayLine_sE;
    sum_s5 += weights_s5 * delayLine_sF;
    sum_s5 += weights_s4 * delayLine_s0;
    sum_s5 += weights_s3 * delayLine_s1;
    sum_s5 += weights_s2 * delayLine_s2;
    sum_s5 += weights_s1 * delayLine_s3;
    sum_s5 += weights_s0 * delayLine_s4;
    (*filteredData)[station][pol][time + 5][channel][ri] = sum_s5;

    sum_s6 = weights_sF * delayLine_s6;
    delayLine_s6 = (*sampledData)[station][time + NR_TAPS - 1 + 7][channel][pol_ri];
    sum_s6 += weights_sE * delayLine_s7;
    sum_s6 += weights_sD * delayLine_s8;
    sum_s6 += weights_sC * delayLine_s9;
    sum_s6 += weights_sB * delayLine_sA;
    sum_s6 += weights_sA * delayLine_sB;
    sum_s6 += weights_s9 * delayLine_sC;
    sum_s6 += weights_s8 * delayLine_sD;
    sum_s6 += weights_s7 * delayLine_sE;
    sum_s6 += weights_s6 * delayLine_sF;
    sum_s6 += weights_s5 * delayLine_s0;
    sum_s6 += weights_s4 * delayLine_s1;
    sum_s6 += weights_s3 * delayLine_s2;
    sum_s6 += weights_s2 * delayLine_s3;
    sum_s6 += weights_s1 * delayLine_s4;
    sum_s6 += weights_s0 * delayLine_s5;
    (*filteredData)[station][pol][time + 6][channel][ri] = sum_s6;

    sum_s7 = weights_sF * delayLine_s7;
    delayLine_s7 = (*sampledData)[station][time + NR_TAPS - 1 + 8][channel][pol_ri];
    sum_s7 += weights_sE * delayLine_s8;
    sum_s7 += weights_sD * delayLine_s9;
    sum_s7 += weights_sC * delayLine_sA;
    sum_s7 += weights_sB * delayLine_sB;
    sum_s7 += weights_sA * delayLine_sC;
    sum_s7 += weights_s9 * delayLine_sD;
    sum_s7 += weights_s8 * delayLine_sE;
    sum_s7 += weights_s7 * delayLine_sF;
    sum_s7 += weights_s6 * delayLine_s0;
    sum_s7 += weights_s5 * delayLine_s1;
    sum_s7 += weights_s4 * delayLine_s2;
    sum_s7 += weights_s3 * delayLine_s3;
    sum_s7 += weights_s2 * delayLine_s4;
    sum_s7 += weights_s1 * delayLine_s5;
    sum_s7 += weights_s0 * delayLine_s6;
    (*filteredData)[station][pol][time + 7][channel][ri] = sum_s7;

    sum_s8 = weights_sF * delayLine_s8;
    delayLine_s8 = (*sampledData)[station][time + NR_TAPS - 1 + 9][channel][pol_ri];
    sum_s8 += weights_sE * delayLine_s9;
    sum_s8 += weights_sD * delayLine_sA;
    sum_s8 += weights_sC * delayLine_sB;
    sum_s8 += weights_sB * delayLine_sC;
    sum_s8 += weights_sA * delayLine_sD;
    sum_s8 += weights_s9 * delayLine_sE;
    sum_s8 += weights_s8 * delayLine_sF;
    sum_s8 += weights_s7 * delayLine_s0;
    sum_s8 += weights_s6 * delayLine_s1;
    sum_s8 += weights_s5 * delayLine_s2;
    sum_s8 += weights_s4 * delayLine_s3;
    sum_s8 += weights_s3 * delayLine_s4;
    sum_s8 += weights_s2 * delayLine_s5;
    sum_s8 += weights_s1 * delayLine_s6;
    sum_s8 += weights_s0 * delayLine_s7;
    (*filteredData)[station][pol][time + 8][channel][ri] = sum_s8;

    sum_s9 = weights_sF * delayLine_s9;
    delayLine_s9 = (*sampledData)[station][time + NR_TAPS - 1 + 10][channel][pol_ri];
    sum_s9 += weights_sE * delayLine_sA;
    sum_s9 += weights_sD * delayLine_sB;
    sum_s9 += weights_sC * delayLine_sC;
    sum_s9 += weights_sB * delayLine_sD;
    sum_s9 += weights_sA * delayLine_sE;
    sum_s9 += weights_s9 * delayLine_sF;
    sum_s9 += weights_s8 * delayLine_s0;
    sum_s9 += weights_s7 * delayLine_s1;
    sum_s9 += weights_s6 * delayLine_s2;
    sum_s9 += weights_s5 * delayLine_s3;
    sum_s9 += weights_s4 * delayLine_s4;
    sum_s9 += weights_s3 * delayLine_s5;
    sum_s9 += weights_s2 * delayLine_s6;
    sum_s9 += weights_s1 * delayLine_s7;
    sum_s9 += weights_s0 * delayLine_s8;
    (*filteredData)[station][pol][time + 9][channel][ri] = sum_s9;

    sum_sA = weights_sF * delayLine_sA;
    delayLine_sA = (*sampledData)[station][time + NR_TAPS - 1 + 11][channel][pol_ri];
    sum_sA += weights_sE * delayLine_sB;
    sum_sA += weights_sD * delayLine_sC;
    sum_sA += weights_sC * delayLine_sD;
    sum_sA += weights_sB * delayLine_sE;
    sum_sA += weights_sA * delayLine_sF;
    sum_sA += weights_s9 * delayLine_s0;
    sum_sA += weights_s8 * delayLine_s1;
    sum_sA += weights_s7 * delayLine_s2;
    sum_sA += weights_s6 * delayLine_s3;
    sum_sA += weights_s5 * delayLine_s4;
    sum_sA += weights_s4 * delayLine_s5;
    sum_sA += weights_s3 * delayLine_s6;
    sum_sA += weights_s2 * delayLine_s7;
    sum_sA += weights_s1 * delayLine_s8;
    sum_sA += weights_s0 * delayLine_s9;
    (*filteredData)[station][pol][time + 10][channel][ri] = sum_sA;

    sum_sB = weights_sF * delayLine_sB;
    delayLine_sB = (*sampledData)[station][time + NR_TAPS - 1 + 12][channel][pol_ri];
    sum_sB += weights_sE * delayLine_sC;
    sum_sB += weights_sD * delayLine_sD;
    sum_sB += weights_sC * delayLine_sE;
    sum_sB += weights_sB * delayLine_sF;
    sum_sB += weights_sA * delayLine_s0;
    sum_sB += weights_s9 * delayLine_s1;
    sum_sB += weights_s8 * delayLine_s2;
    sum_sB += weights_s7 * delayLine_s3;
    sum_sB += weights_s6 * delayLine_s4;
    sum_sB += weights_s5 * delayLine_s5;
    sum_sB += weights_s4 * delayLine_s6;
    sum_sB += weights_s3 * delayLine_s7;
    sum_sB += weights_s2 * delayLine_s8;
    sum_sB += weights_s1 * delayLine_s9;
    sum_sB += weights_s0 * delayLine_sA;
    (*filteredData)[station][pol][time + 11][channel][ri] = sum_sB;

    sum_sC = weights_sF * delayLine_sC;
    delayLine_sC = (*sampledData)[station][time + NR_TAPS - 1 + 13][channel][pol_ri];
    sum_sC += weights_sE * delayLine_sD;
    sum_sC += weights_sD * delayLine_sE;
    sum_sC += weights_sC * delayLine_sF;
    sum_sC += weights_sB * delayLine_s0;
    sum_sC += weights_sA * delayLine_s1;
    sum_sC += weights_s9 * delayLine_s2;
    sum_sC += weights_s8 * delayLine_s3;
    sum_sC += weights_s7 * delayLine_s4;
    sum_sC += weights_s6 * delayLine_s5;
    sum_sC += weights_s5 * delayLine_s6;
    sum_sC += weights_s4 * delayLine_s7;
    sum_sC += weights_s3 * delayLine_s8;
    sum_sC += weights_s2 * delayLine_s9;
    sum_sC += weights_s1 * delayLine_sA;
    sum_sC += weights_s0 * delayLine_sB;
    (*filteredData)[station][pol][time + 12][channel][ri] = sum_sC;

    sum_sD = weights_sF * delayLine_sD;
    delayLine_sD = (*sampledData)[station][time + NR_TAPS - 1 + 14][channel][pol_ri];
    sum_sD += weights_sE * delayLine_sE;
    sum_sD += weights_sD * delayLine_sF;
    sum_sD += weights_sC * delayLine_s0;
    sum_sD += weights_sB * delayLine_s1;
    sum_sD += weights_sA * delayLine_s2;
    sum_sD += weights_s9 * delayLine_s3;
    sum_sD += weights_s8 * delayLine_s4;
    sum_sD += weights_s7 * delayLine_s5;
    sum_sD += weights_s6 * delayLine_s6;
    sum_sD += weights_s5 * delayLine_s7;
    sum_sD += weights_s4 * delayLine_s8;
    sum_sD += weights_s3 * delayLine_s9;
    sum_sD += weights_s2 * delayLine_sA;
    sum_sD += weights_s1 * delayLine_sB;
    sum_sD += weights_s0 * delayLine_sC;
    (*filteredData)[station][pol][time + 13][channel][ri] = sum_sD;

    sum_sE = weights_sF * delayLine_sE;
    delayLine_sE = (*sampledData)[station][time + NR_TAPS - 1 + 15][channel][pol_ri];
    sum_sE += weights_sE * delayLine_sF;
    sum_sE += weights_sD * delayLine_s0;
    sum_sE += weights_sC * delayLine_s1;
    sum_sE += weights_sB * delayLine_s2;
    sum_sE += weights_sA * delayLine_s3;
    sum_sE += weights_s9 * delayLine_s4;
    sum_sE += weights_s8 * delayLine_s5;
    sum_sE += weights_s7 * delayLine_s6;
    sum_sE += weights_s6 * delayLine_s7;
    sum_sE += weights_s5 * delayLine_s8;
    sum_sE += weights_s4 * delayLine_s9;
    sum_sE += weights_s3 * delayLine_sA;
    sum_sE += weights_s2 * delayLine_sB;
    sum_sE += weights_s1 * delayLine_sC;
    sum_sE += weights_s0 * delayLine_sD;
    (*filteredData)[station][pol][time + 14][channel][ri] = sum_sE;

    sum_sF = weights_sF * delayLine_sF;
    sum_sF += weights_sE * delayLine_s0;
    sum_sF += weights_sD * delayLine_s1;
    sum_sF += weights_sC * delayLine_s2;
    sum_sF += weights_sB * delayLine_s3;
    sum_sF += weights_sA * delayLine_s4;
    sum_sF += weights_s9 * delayLine_s5;
    sum_sF += weights_s8 * delayLine_s6;
    sum_sF += weights_s7 * delayLine_s7;
    sum_sF += weights_s6 * delayLine_s8;
    sum_sF += weights_s5 * delayLine_s9;
    sum_sF += weights_s4 * delayLine_sA;
    sum_sF += weights_s3 * delayLine_sB;
    sum_sF += weights_s2 * delayLine_sC;
    sum_sF += weights_s1 * delayLine_sD;
    sum_sF += weights_s0 * delayLine_sE;
    (*filteredData)[station][pol][time + 15][channel][ri] = sum_sF;
  }
}
//
//int main()
//{
//    const int arraySize = 5;
//    const int a[arraySize] = { 1, 2, 3, 4, 5 };
//    const int b[arraySize] = { 10, 20, 30, 40, 50 };
//    int c[arraySize] = { 0 };
//
//    // Add vectors in parallel.
//    cudaError_t cudaStatus = addWithCuda(c, a, b, arraySize);
//    if (cudaStatus != cudaSuccess) {
//        fprintf(stderr, "addWithCuda failed!");
//        return 1;
//    }
//
//    printf("{1,2,3,4,5} + {10,20,30,40,50} = {%d,%d,%d,%d,%d}\n",
//        c[0], c[1], c[2], c[3], c[4]);
//
//    // cudaDeviceReset must be called before exiting in order for profiling and
//    // tracing tools such as Nsight and Visual Profiler to show complete traces.
//    cudaStatus = cudaDeviceReset();
//    if (cudaStatus != cudaSuccess) {
//        fprintf(stderr, "cudaDeviceReset failed!");
//        return 1;
//    }
//
//    return 0;
//}

// Helper function for using CUDA to add vectors in parallel.
cudaError_t FIR_filter_wrapper(float *DevFilteredData,
    float const *DevSampledData,
    float const *DevWeightsData)
{
    cudaError_t cudaStatus;

    // Choose which GPU to run on, change this on a multi-GPU system.
    cudaStatus = cudaSetDevice(0);
    if (cudaStatus != cudaSuccess) {
        //fprintf(stderr, "cudaSetDevice failed!  Do you have a CUDA-capable GPU installed?");
        return cudaStatus;;
    }

    // From here copy pasta of opencl code
    int nrChannelsPerSubband = 8;
    int nrStations = 2; 
    unsigned totalNrThreads = nrChannelsPerSubband * NR_POLARIZATIONS * 2; //ps.nrChannelsPerSubband()
    dim3 globalWorkSize(totalNrThreads, nrStations); //ps.nrStations()

    int MAXNRCUDATHREADS = 512;
    size_t maxNrThreads = MAXNRCUDATHREADS;
    unsigned nrPasses = (totalNrThreads + maxNrThreads - 1) / maxNrThreads;
    dim3 localWorkSize(totalNrThreads / nrPasses, 1); 

    //// Create the needed data
    //unsigned sizeFilteredData = NR_STATIONS * NR_POLARIZATIONS * NR_SAMPLES_PER_CHANNEL * NR_CHANNELS * COMPLEX;
    //float* filteredData = new float[sizeFilteredData];
    //for (unsigned idx = 0; idx < sizeFilteredData; ++idx)
    //{
    //  filteredData[idx] = 0;
    //}
    //
    //unsigned sizeSampledData = NR_STATIONS * (NR_TAPS - 1 + NR_SAMPLES_PER_CHANNEL) * NR_CHANNELS * NR_POLARIZATIONS * COMPLEX;
    //float * sampledData = new float[sizeSampledData];
    //for (unsigned idx = 0; idx < sizeSampledData; ++idx)
    //{
    //  sampledData[idx] = 0;
    //}

    //unsigned sizeWeightsData = NR_CHANNELS * 16;
    //float * weightsData = new float[sizeWeightsData];
    //     for (unsigned idx = 0; idx < sizeWeightsData; ++idx)
    //{
    //  weightsData[idx] = 0;
    //}

    //copy to the gpu
    //float *DevFilteredData;
    //float *DevSampledData;
    //float *DevWeightsData;
    // Allocate GPU buffers for three vectors (two input, one output)    .
    //cudaStatus = cudaMalloc((void**)&DevFilteredData, sizeFilteredData * sizeof(float));
    //if (cudaStatus != cudaSuccess) {
    //    fprintf(stderr, "cudaMalloc failed!");
    //    goto Error;
    //}

    //cudaStatus = cudaMalloc((void**)&DevSampledData, sizeSampledData * sizeof(float));
    //if (cudaStatus != cudaSuccess) {
    //    fprintf(stderr, "cudaMalloc failed!");
    //    goto Error;
    //}

    //cudaStatus = cudaMalloc((void**)&DevWeightsData, sizeWeightsData * sizeof(float));
    //if (cudaStatus != cudaSuccess) {
    //    fprintf(stderr, "cudaMalloc failed!");
    //    goto Error;
    //}    

    //// Copy input vectors from host memory to GPU buffers.
    //cudaStatus = cudaMemcpy(DevWeightsData, weightsData,
    //  sizeWeightsData * sizeof(float), cudaMemcpyHostToDevice);
    //if (cudaStatus != cudaSuccess) {
    //    fprintf(stderr, "cudaMemcpy failed!");
    //    goto Error;
    //}

    //cudaStatus = cudaMemcpy(DevSampledData, sampledData,
    //  sizeSampledData * sizeof(float), cudaMemcpyHostToDevice);
    //if (cudaStatus != cudaSuccess) {
    //    fprintf(stderr, "cudaMemcpy failed!");
    //    goto Error;
    //}


    // Launch a kernel on the GPU with one thread for each element.
    FIR_filter<<<globalWorkSize, localWorkSize>>>(DevFilteredData,
      DevSampledData, DevWeightsData);

    //// cudaDeviceSynchronize waits for the kernel to finish, and returns
    //// any errors encountered during the launch.
    //cudaStatus = cudaDeviceSynchronize();
    //if (cudaStatus != cudaSuccess) {
    //    fprintf(stderr, "cudaDeviceSynchronize returned error code %d after launching addKernel!\n", cudaStatus);
    //    return cudaStatus;;
    //}

    //// Copy output vector from GPU buffer to host memory.
    //cudaStatus = cudaMemcpy(filteredData, DevFilteredData,
    //  sizeFilteredData * sizeof(float), cudaMemcpyDeviceToHost);
    //if (cudaStatus != cudaSuccess) {
    //    fprintf(stderr, "cudaMemcpy failed!");
    //    return cudaStatus;;
    //}

}
