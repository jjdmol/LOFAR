typedef __global float2 (*TransposedDataType)[NR_TABS][NR_POLARIZATIONS][NR_SAMPLES_PER_SUBBAND + NR_STATION_FILTER_TAPS - 1][512];
typedef __global float4 (*ComplexVoltagesType)[NR_SUBBANDS][NR_SAMPLES_PER_SUBBAND + NR_STATION_FILTER_TAPS - 1][NR_TABS];


__kernel void UHEP_Transpose(__global void *restrict transposedDataPtr,
                             __global const void *restrict complexVoltagesPtr,
                             __global int reverseSubbandMapping[512])
{
  TransposedDataType transposedData = (TransposedDataType) transposedDataPtr;
  ComplexVoltagesType complexVoltages = (ComplexVoltagesType) complexVoltagesPtr;

  __local float4 tmp[16][17];

  uint tabBase = 16 * get_global_id(1);
  uint sbBase = 16 * get_global_id(2);

  uint tabOffsetR = get_local_id(0) & 15;
  uint tabR = tabBase + tabOffsetR;
  uint sbOffsetR = get_local_id(0) >> 4;
  int sbSourceR = reverseSubbandMapping[sbBase + sbOffsetR];
  bool doR = (NR_TABS % 16 == 0 || tabR < NR_TABS) && sbSourceR >= 0;

  uint tabOffsetW = get_local_id(0) >> 4;
  uint tabW = tabBase + tabOffsetW;
  uint sbOffsetW = get_local_id(0) & 15;
  int sbSourceW = reverseSubbandMapping[sbBase + sbOffsetW];
  bool doW = NR_TABS % 16 == 0 || tabW < NR_TABS;

  for (int time = 0; time < NR_SAMPLES_PER_SUBBAND + NR_STATION_FILTER_TAPS - 1; time++) {
    if (doR)
      tmp[tabOffsetR][sbOffsetR] = (*complexVoltages)[sbSourceR][time][tabR];

    barrier(CLK_LOCAL_MEM_FENCE);

    if (doW) {
      float4 sample = sbSourceW >= 0 ? tmp[tabOffsetW][sbOffsetW] : 0;
      (*transposedData)[tabW][0][time][sbBase + sbOffsetW] = sample.xy;
      (*transposedData)[tabW][1][time][sbBase + sbOffsetW] = sample.zw;
    }

    barrier(CLK_LOCAL_MEM_FENCE);
  }
}
