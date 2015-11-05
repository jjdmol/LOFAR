#if 0
typedef __global float2 (*TransposedDataType)[NR_TABS][NR_POLARIZATIONS][NR_SAMPLES_PER_CHANNEL][NR_CHANNELS];
typedef __global float4 (*ComplexVoltagesType)[NR_CHANNELS][NR_SAMPLES_PER_CHANNEL][NR_TABS];

__kernel void transposeComplexVoltages(__global void *restrict transposedDataPtr, 
				       __global const void *restrict complexVoltagesPtr)
{
  TransposedDataType  transposedData  = (TransposedDataType) transposedDataPtr;
  ComplexVoltagesType complexVoltages = (ComplexVoltagesType) complexVoltagesPtr;

  __local float4 tmp[16][17];

  uint tabBase = 16 * get_global_id(1);
  uint chBase  = 16 * get_global_id(2);

  uint tabOffsetR = get_local_id(0) & 15;
  uint tabR	  = tabBase + tabOffsetR;
  uint chOffsetR  = get_local_id(0) >> 4;
  uint channelR	  = chBase + chOffsetR;
  bool doR	  = NR_TABS % 16 == 0 || tabR < NR_TABS;

  uint tabOffsetW = get_local_id(0) >> 4;
  uint tabW	  = tabBase + tabOffsetW;
  uint chOffsetW  = get_local_id(0) & 15;
  uint channelW	  = chBase + chOffsetW;
  bool doW	  = NR_TABS % 16 == 0 || tabW < NR_TABS;

  for (int time = 0; time < NR_SAMPLES_PER_CHANNEL; time ++) {
    if (doR)
      tmp[tabOffsetR][chOffsetR] = (*complexVoltages)[channelR][time][tabR];

    barrier(CLK_LOCAL_MEM_FENCE);

    if (doW) {
      float4 sample = tmp[tabOffsetW][chOffsetW];
      (*transposedData)[tabW][0][time][channelW] = sample.xy;
      (*transposedData)[tabW][1][time][channelW] = sample.zw;
    }

    barrier(CLK_LOCAL_MEM_FENCE);
  }
}

#else

typedef __global float2 (*TransposedDataType)[NR_TABS][NR_POLARIZATIONS][NR_CHANNELS][NR_SAMPLES_PER_CHANNEL];
typedef __global float4 (*ComplexVoltagesType)[NR_CHANNELS][NR_SAMPLES_PER_CHANNEL][NR_TABS];

__kernel void transposeComplexVoltages(__global void *restrict transposedDataPtr, 
				       __global const void *restrict complexVoltagesPtr)
{
  TransposedDataType  transposedData  = (TransposedDataType) transposedDataPtr;
  ComplexVoltagesType complexVoltages = (ComplexVoltagesType) complexVoltagesPtr;

  __local float4 tmp[16][17];

  uint tabBase = 16 * get_global_id(1);
  uint timeBase  = 16 * get_global_id(2);

  uint tabOffsetR = get_local_id(0) & 15;
  uint tabR	  = tabBase + tabOffsetR;
  uint timeOffsetR  = get_local_id(0) >> 4;
  uint timeR	  = timeBase + timeOffsetR;
  bool doR	  = NR_TABS % 16 == 0 || tabR < NR_TABS;

  uint tabOffsetW = get_local_id(0) >> 4;
  uint tabW	  = tabBase + tabOffsetW;
  uint timeOffsetW  = get_local_id(0) & 15;
  uint timeW	  = timeBase + timeOffsetW;
  bool doW	  = NR_TABS % 16 == 0 || tabW < NR_TABS;

  for (int channel = 0; channel < NR_CHANNELS; channel ++) {
    if (doR)
      tmp[tabOffsetR][timeOffsetR] = (*complexVoltages)[timeR][channel][tabR];

    barrier(CLK_LOCAL_MEM_FENCE);

    if (doW) {
      float4 sample = tmp[tabOffsetW][timeOffsetW];
      (*transposedData)[tabW][0][channel][timeW] = sample.xy;
      (*transposedData)[tabW][1][channel][timeW] = sample.zw;
    }

    barrier(CLK_LOCAL_MEM_FENCE);
  }
}

#endif
