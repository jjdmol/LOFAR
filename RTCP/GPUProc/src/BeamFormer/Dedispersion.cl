#include "math.cl"


//        | DM beam pol ch subtime subch. | SB
// -------+-------------------------------+---
// taper  |                        x      | x
// factor | x           x          x      | x
// sample |    x    x   x  x       x      | x

#define CHANNEL_BANDWIDTH (SUBBAND_BANDWIDTH / NR_CHANNELS)
#define SUB_CHANNEL_BANDWIDTH (CHANNEL_BANDWIDTH / DEDISPERSION_FFT_SIZE)

typedef __global float2 (*BufferType)[NR_TABS][NR_POLARIZATIONS][NR_CHANNELS][NR_SAMPLES_PER_CHANNEL / DEDISPERSION_FFT_SIZE][DEDISPERSION_FFT_SIZE];


__kernel void applyChirp(__global void *bufferPtr,
			 __global float *DMs,
                         float subbandFrequency)
{
  __local float local_DMs[NR_TABS];

  for (int i = get_local_id(0); i < NR_TABS; i += get_local_size(0))
    local_DMs[i] = DMs[i] * 2.0f * (float) M_PI * 4.149e15f;

  barrier(CLK_LOCAL_MEM_FENCE);
  
  BufferType buffer = (BufferType) bufferPtr;

  uint subChannel = get_global_id(0);
  uint time = get_global_id(1);
  uint channel = get_global_id(2);

#if NR_CHANNELS > 1
  float subbandBaseFrequency = subbandFrequency - .5f * (float) SUBBAND_BANDWIDTH;
  float channel0frequency    = subbandBaseFrequency + channel * CHANNEL_BANDWIDTH;
#else
  float channel0frequency    = subbandFrequency;
#endif

  float binFrequency = subChannel * SUB_CHANNEL_BANDWIDTH;

  if (subChannel > DEDISPERSION_FFT_SIZE)
    binFrequency -= CHANNEL_BANDWIDTH;

  float taper = native_rsqrt(1 + pow(binFrequency / (.47f * (float) CHANNEL_BANDWIDTH), 80.0f)) * DEDISPERSION_FFT_SIZE;
  float frequencyDiv = binFrequency / channel0frequency;
  float frequencyFac = frequencyDiv * frequencyDiv / (channel0frequency + binFrequency);

  for (uint tab = 0; tab < NR_TABS; tab ++) {
    float DM = local_DMs[tab];

    /* if (DM > 0) */ {
      float2 sampleX = (*buffer)[tab][0][channel][time][subChannel];
      float2 sampleY = (*buffer)[tab][1][channel][time][subChannel];
      float2 factor  = cexp(DM * frequencyFac) * taper;

      (*buffer)[tab][0][channel][time][subChannel] = cmul(factor, sampleX);
      (*buffer)[tab][1][channel][time][subChannel] = cmul(factor, sampleY);
    }
  }
}
