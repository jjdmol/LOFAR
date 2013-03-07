#include "math.cl"

#if NR_CHANNELS == 1
#undef BANDPASS_CORRECTION
#endif


typedef __global fcomplex2 (*restrict OutputDataType)[NR_STATIONS][NR_CHANNELS][NR_SAMPLES_PER_CHANNEL];
#if NR_CHANNELS == 1
#if NR_BITS_PER_SAMPLE == 16
typedef __global short_complex2 (*restrict InputDataType)[NR_STATIONS][NR_SAMPLES_PER_SUBBAND];
#elif NR_BITS_PER_SAMPLE == 8
typedef __global char_complex2 (*restrict InputDataType)[NR_STATIONS][NR_SAMPLES_PER_SUBBAND];
#else
#error unsupport NR_BITS_PER_SAMPLE
#endif
#else
typedef __global fcomplex (*restrict InputDataType)[NR_STATIONS][NR_POLARIZATIONS][NR_SAMPLES_PER_CHANNEL][NR_CHANNELS];
#endif
typedef __global const float2 (*restrict DelaysType)[NR_BEAMS][NR_STATIONS]; // 2 Polarizations; in seconds
typedef __global const float2 (*restrict PhaseOffsetsType)[NR_STATIONS]; // 2 Polarizations; in radians
typedef __global const float (*restrict BandPassFactorsType)[NR_CHANNELS];


__kernel __attribute__((reqd_work_group_size(16 * 16, 1, 1)))
void applyDelaysAndCorrectBandPass(__global fcomplex *restrict correctedDataPtr,
				   __global const fcomplex *restrict filteredDataPtr,
				   float subbandFrequency,
				   unsigned beam,
				   __global const float2 *restrict delaysAtBeginPtr,
				   __global const float2 *restrict delaysAfterEndPtr,
				   __global const float2 *restrict phaseOffsetsPtr,
				   __global const float *restrict bandPassFactorsPtr)
{
  OutputDataType outputData = (OutputDataType) correctedDataPtr;
  InputDataType inputData = (InputDataType) filteredDataPtr;
  DelaysType delaysAtBegin = (DelaysType) delaysAtBeginPtr;
  DelaysType delaysAfterEnd = (DelaysType) delaysAfterEndPtr;
  PhaseOffsetsType phaseOffsets = (PhaseOffsetsType) phaseOffsetsPtr;

#if NR_CHANNELS > 1
  BandPassFactorsType bandPassFactors = (BandPassFactorsType) bandPassFactorsPtr;

  __local fcomplex2 tmp[16][17]; // one too wide to allow coalesced reads

  uint major	   = get_global_id(0) / 16;
  uint minor	   = get_global_id(0) % 16;
  uint channel	   = get_global_id(1) * 16;
#endif
  uint station	   = get_global_id(2);

#if defined DELAY_COMPENSATION
#if NR_CHANNELS == 1
  float frequency = subbandFrequency;
#else
  float frequency = subbandFrequency - .5f * SUBBAND_BANDWIDTH + (channel + minor) * (SUBBAND_BANDWIDTH / NR_CHANNELS);
#endif
  float2 delayAtBegin = (*delaysAtBegin)[beam][station];
  float2 delayAfterEnd = (*delaysAfterEnd)[beam][station];
  float2 phiBegin = -2 * 3.1415926535f * delayAtBegin;
  float2 phiEnd   = -2 * 3.1415926535f * delayAfterEnd;
  float2 deltaPhi = (phiEnd - phiBegin) / NR_SAMPLES_PER_CHANNEL;
#if NR_CHANNELS == 1
  float2 myPhiBegin = (phiBegin + get_local_id(0) * deltaPhi) * frequency + (*phaseOffsets)[station];
  float2 myPhiDelta = get_local_size(0) * deltaPhi * frequency;
#else
  float2 myPhiBegin = (phiBegin + major * deltaPhi) * frequency + (*phaseOffsets)[station];
  float2 myPhiDelta = 16 * deltaPhi * frequency;
#endif
  fcomplex vX = cexp(myPhiBegin.x);
  fcomplex vY = cexp(myPhiBegin.y);
  fcomplex dvX = cexp(myPhiDelta.x);
  fcomplex dvY = cexp(myPhiDelta.y);
#endif

#if defined BANDPASS_CORRECTION
  float weight = (*bandPassFactors)[channel + minor];
#endif

#if defined DELAY_COMPENSATION && defined BANDPASS_CORRECTION
  vX *= weight;
  vY *= weight;
#endif

#if NR_CHANNELS == 1
  for (uint time = get_local_id(0); time < NR_SAMPLES_PER_SUBBAND; time += get_local_size(0)) {
    fcomplex2 samples = convert_float4((*inputData)[station][time]);
    fcomplex sampleX = samples.s01;
    fcomplex sampleY = samples.s23;
#else
  for (uint time = 0; time < NR_SAMPLES_PER_CHANNEL; time += 16) {
    fcomplex sampleX = (*inputData)[station][0][time + major][channel + minor];
    fcomplex sampleY = (*inputData)[station][1][time + major][channel + minor];
#endif

#if defined DELAY_COMPENSATION
    sampleX = cmul(sampleX, vX);
    sampleY = cmul(sampleY, vY);
    vX = cmul(vY, dvX);
    vY = cmul(vY, dvY);
#elif defined BANDPASS_CORRECTION
    sampleX *= weight;
    sampleY *= weight;
#endif

#if NR_CHANNELS == 1
    (*outputData)[station][0][time] = (float4) (sampleX, sampleY);
#else
    tmp[major][minor] = (float4) (sampleX, sampleY);
    barrier(CLK_LOCAL_MEM_FENCE);

    (*outputData)[station][channel + major][time + minor] = tmp[minor][major];
    barrier(CLK_LOCAL_MEM_FENCE);
#endif
  }
}
