__kernel void computeStokes(__global void *restrict stokesDataPtr, 
			    __global const void *restrict complexVoltagesPtr)
{
  typedef __global float (*StokesType)[NR_TABS][NR_COHERENT_STOKES][NR_SAMPLES_PER_CHANNEL / COHERENT_STOKES_TIME_INTEGRATION_FACTOR][NR_CHANNELS];
  typedef __global float4 (*ComplexVoltagesType)[NR_CHANNELS][NR_SAMPLES_PER_CHANNEL / COHERENT_STOKES_TIME_INTEGRATION_FACTOR][COHERENT_STOKES_TIME_INTEGRATION_FACTOR][NR_TABS];

  StokesType	      stokesData      = (StokesType) stokesDataPtr;
  ComplexVoltagesType complexVoltages = (ComplexVoltagesType) complexVoltagesPtr;

  __local float tmp[NR_COHERENT_STOKES][16][17];

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

  for (uint time = 0; time < NR_SAMPLES_PER_CHANNEL / COHERENT_STOKES_TIME_INTEGRATION_FACTOR; time ++) {
    float stokesI = 0;
#if NR_COHERENT_STOKES == 4
    float stokesQ = 0, halfStokesU = 0, halfStokesV = 0;
#endif

    if (doR) {
      for (uint t = 0; t < COHERENT_STOKES_TIME_INTEGRATION_FACTOR; t ++) {
	float4 sample = (*complexVoltages)[channelR][time][t][tabR];
	float2 X = sample.xy, Y = sample.zw;
	float powerX = X.x * X.x + X.y * X.y;
	float powerY = Y.x * Y.x + Y.y * Y.y;
	stokesI += powerX + powerY;
#if NR_COHERENT_STOKES == 4
	stokesQ += powerX - powerY;
	halfStokesU += X.x * Y.x + X.y * Y.y;
	halfStokesV += X.y * Y.x - X.x * Y.y;
#endif
      }

      tmp[0][tabOffsetR][chOffsetR] = stokesI;
#if NR_COHERENT_STOKES == 4
      tmp[1][tabOffsetR][chOffsetR] = stokesQ;
      tmp[2][tabOffsetR][chOffsetR] = 2 * halfStokesU;
      tmp[3][tabOffsetR][chOffsetR] = 2 * halfStokesV;
#endif
    }

    barrier(CLK_LOCAL_MEM_FENCE);

    if (doW)
      for (uint stokes = 0; stokes < NR_COHERENT_STOKES; stokes ++)
	(*stokesData)[tabW][stokes][time][channelW] = tmp[stokes][tabOffsetW][chOffsetW];

    barrier(CLK_LOCAL_MEM_FENCE);
  }
}


#if 0
__kernel void computeStokes(__global void *restrict stokesDataPtr, 
			    __global const void *restrict dedispersedDataPtr)
{
  typedef __global float (*StokesType)[NR_TABS][NR_COHERENT_STOKES][NR_SAMPLES_PER_CHANNEL / COHERENT_STOKES_TIME_INTEGRATION_FACTOR][NR_CHANNELS];
  typedef __global float2 (*DedispersedDataType)[NR_TABS][NR_POLARIZATIONS][NR_CHANNELS][NR_SAMPLES_PER_CHANNEL];

  StokesType	      stokesData      = (StokesType) stokesDataPtr;
  DedispersedDataType dedispersedData = (DedispersedDataType) dedispersedDataPtr;

  __local float tmp[NR_COHERENT_STOKES][16][17];

  uint timeBase = 16 * get_global_id(1);
  uint chBase  = 16 * get_global_id(2);

  uint timeOffsetR = get_local_id(0) & 15;
  uint timeR	  = timeBase + tabOffsetR;
  uint chOffsetR  = get_local_id(0) >> 4;
  uint channelR	  = chBase + chOffsetR;
  bool doR	  = NR_TABS % 16 == 0 || tabR < NR_TABS;

  uint tabOffsetW = get_local_id(0) >> 4;
  uint tabW	  = tabBase + tabOffsetW;
  uint chOffsetW  = get_local_id(0) & 15;
  uint channelW	  = chBase + chOffsetW;
  bool doW	  = NR_TABS % 16 == 0 || tabW < NR_TABS;

  for (uint time = 0; time < NR_SAMPLES_PER_CHANNEL / COHERENT_STOKES_TIME_INTEGRATION_FACTOR; time ++) {
    float stokesI = 0;
#if NR_COHERENT_STOKES == 4
    float stokesQ = 0, halfStokesU = 0, halfStokesV = 0;
#endif

    if (doR) {
      for (uint t = 0; t < COHERENT_STOKES_TIME_INTEGRATION_FACTOR; t ++) {
	float4 sample = (*complexVoltages)[channelR][time][t][tabR];
	float2 X = sample.xy, Y = sample.zw;
	float powerX = X.x * X.x + X.y * X.y;
	float powerY = Y.x * Y.x + Y.y * Y.y;
	stokesI += powerX + powerY;
#if NR_COHERENT_STOKES == 4
	stokesQ += powerX - powerY;
	halfStokesU += X.x * Y.x + X.y * Y.y;
	halfStokesV += X.y * Y.x - X.x * Y.y;
#endif
      }

      tmp[0][tabOffsetR][chOffsetR] = stokesI;
#if NR_COHERENT_STOKES == 4
      tmp[1][tabOffsetR][chOffsetR] = stokesQ;
      tmp[2][tabOffsetR][chOffsetR] = 2 * halfStokesU;
      tmp[3][tabOffsetR][chOffsetR] = 2 * halfStokesV;
#endif
    }

    barrier(CLK_LOCAL_MEM_FENCE);

    if (doW)
      for (uint stokes = 0; stokes < NR_COHERENT_STOKES; stokes ++)
	(*stokesData)[tabW][stokes][time][channelW] = tmp[stokes][tabOffsetW][chOffsetW];

    barrier(CLK_LOCAL_MEM_FENCE);
  }
}
#endif
