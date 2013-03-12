#if NR_BITS_PER_SAMPLE == 16
typedef short2 SampleType;
#elif NR_BITS_PER_SAMPLE == 8
typedef char2 SampleType;
#else
#error unsupport NR_BITS_PER_SAMPLE
#endif

typedef __global SampleType (*SampledDataType)[NR_STATIONS][NR_SAMPLES_PER_SUBBAND][NR_POLARIZATIONS];
typedef __global float2 (*ConvertedDataType)[NR_STATIONS][NR_POLARIZATIONS][NR_SAMPLES_PER_SUBBAND];


__kernel void intToFloat(__global void *restrict convertedDataPtr,
                         __global const void *restrict sampledDataPtr)
{
  ConvertedDataType convertedData = (ConvertedDataType) convertedDataPtr;
  SampledDataType sampledData = (SampledDataType) sampledDataPtr;

  uint station = get_global_id(1);

  for (uint time = get_local_id(0); time < NR_SAMPLES_PER_SUBBAND; time += get_local_size(0)) {
    (*convertedData)[station][0][time] = convert_float2((*sampledData)[station][time][0]);
    (*convertedData)[station][1][time] = convert_float2((*sampledData)[station][time][1]);
  }
}
