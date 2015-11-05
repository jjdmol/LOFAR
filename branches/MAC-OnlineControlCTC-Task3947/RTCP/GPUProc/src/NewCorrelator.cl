#define NR_STATIONS_PER_BLOCK	32
#define BLOCK_SIZE		8

#define NR_BASELINES		(NR_STATIONS * (NR_STATIONS + 1) / 2)


typedef __global float (*CorrectedDataType)[NR_STATIONS][NR_CHANNELS][NR_SAMPLES_PER_CHANNEL][NR_POLARIZATIONS * 2];
typedef __global float8 (*VisibilitiesType)[NR_BASELINES][NR_CHANNELS];


#if 0
__kernel void correlateTriangles(__global void *visibilitiesPtr,
				 __global const void *correctedDataPtr
)
{
  VisibilitiesType visibilities = (VisibilitiesType) visibilitiesPtr;
  CorrectedDataType correctedData = (CorrectedDataType) correctedDataPtr;

  __local float4 samples[BLOCK_SIZE][NR_STATIONS_PER_BLOCK];

  uint triangle = get_global_id(1);
  uint channel  = get_global_id(2);
  uint firstStation = triangle * NR_STATIONS_PER_BLOCK;

  float4 vis_0A_r = (float4) 0, vis_0A_i = (float4) 0;
  float4 vis_0B_r = (float4) 0, vis_0B_i = (float4) 0;
  float4 vis_1A_r = (float4) 0, vis_1A_i = (float4) 0;
  float4 vis_1B_r = (float4) 0, vis_1B_i = (float4) 0;

  for (uint major = 0; major < NR_SAMPLES_PER_CHANNEL; major += BLOCK_SIZE) {
    // load data into local memory
#pragma unroll 1
    for (uint i = get_local_id(0); i < BLOCK_SIZE * NR_STATIONS_PER_BLOCK; i += get_local_size(0)) {
      uint time = i % BLOCK_SIZE;
      uint stat = i / BLOCK_SIZE;

      if (firstStation + stat < NR_STATIONS)
	samples[time][stat] = (*correctedData)[firstStation + stat][channel][major + time];
    }

    barrier(CLK_LOCAL_MEM_FENCE);

    // compute auto-correlations

    if (firstStation + get_local_id(0) < NR_STATIONS) {
      for (time = 0; time + BLOCK_SIZE; time ++) {
	float sample = samples[time][get_local_id(0)];
      }


    barrier(CLK_LOCAL_MEM_FENCE);
  }
}
#endif


__kernel __attribute__((reqd_work_group_size(NR_STATIONS_PER_BLOCK * NR_STATIONS_PER_BLOCK / 4, 1, 1)))
void correlateRectangles(__global void *visibilitiesPtr,
			 __global const void *correctedDataPtr
)
{
  VisibilitiesType visibilities = (VisibilitiesType) visibilitiesPtr;
  CorrectedDataType correctedData = (CorrectedDataType) correctedDataPtr;

  __local float4 samplesX[2][BLOCK_SIZE][NR_STATIONS_PER_BLOCK / 2 | 1];
  __local float4 samplesY[2][BLOCK_SIZE][NR_STATIONS_PER_BLOCK / 2 | 1];

  uint block	 = get_global_id(1);
  uint channel   = get_global_id(2);
  uint blockX	 = convert_uint_rtz(sqrt(convert_float(8 * block + 1)) - 0.99999f) / 2;
  uint blockY	 = block - blockX * (blockX + 1) / 2;

#if NR_STATIONS % NR_STATIONS_PER_BLOCK == 0
  uint firstStationX = (blockX + 1) * NR_STATIONS_PER_BLOCK;
  uint firstStationY = blockY * NR_STATIONS_PER_BLOCK;
#else
  uint firstStationX = blockX * NR_STATIONS_PER_BLOCK + NR_STATIONS % NR_STATIONS_PER_BLOCK;
  int  firstStationY = (blockY - 1) * NR_STATIONS_PER_BLOCK + NR_STATIONS % NR_STATIONS_PER_BLOCK;
#endif

  uint statXoffset = (get_local_id(0) / (NR_STATIONS_PER_BLOCK / 2));
  uint statYoffset = (get_local_id(0) % (NR_STATIONS_PER_BLOCK / 2));

  float4 vis_0A_r = 0, vis_0A_i = 0;
  float4 vis_0B_r = 0, vis_0B_i = 0;
  float4 vis_1A_r = 0, vis_1A_i = 0;
  float4 vis_1B_r = 0, vis_1B_i = 0;

  for (uint major = 0; major < NR_SAMPLES_PER_CHANNEL; major += BLOCK_SIZE) {
    // load data into local memory
    for (uint i = get_local_id(0); i < 4 * BLOCK_SIZE * NR_STATIONS_PER_BLOCK; i += NR_STATIONS_PER_BLOCK * NR_STATIONS_PER_BLOCK / 4) {
      uint p = i % 4;
      uint time = i / 4 % BLOCK_SIZE;
      uint stat = i / 4 / BLOCK_SIZE;

      ((__local float *) &samplesX[stat % 2][time][stat / 2])[p] = (*correctedData)[firstStationX + stat][channel][major + time][p];

      if (NR_STATIONS % NR_STATIONS_PER_BLOCK == 0 || (int) (firstStationY + stat) >= 0)
	((__local float *) &samplesY[stat % 2][time][stat / 2])[p] = (*correctedData)[firstStationY + stat][channel][major + time][p];
    }

    barrier(CLK_LOCAL_MEM_FENCE);

    if (NR_STATIONS % NR_STATIONS_PER_BLOCK == 0 || (int) (firstStationY + 2 * statYoffset) >= -1) {
      for (uint time = 0; time < BLOCK_SIZE; time ++) {
        float4 sample_0 = samplesY[0][time][statYoffset];
        float4 sample_A = samplesX[0][time][statXoffset];
        float4 sample_B = samplesX[1][time][statXoffset];
        float4 sample_1 = samplesY[1][time][statYoffset];

        vis_0A_r += sample_0.xxzz * sample_A.xzxz;
        vis_0A_i += sample_0.yyww * sample_A.xzxz;
        vis_0B_r += sample_0.xxzz * sample_B.xzxz;
        vis_0B_i += sample_0.yyww * sample_B.xzxz;
        vis_1A_r += sample_1.xxzz * sample_A.xzxz;
        vis_1A_i += sample_1.yyww * sample_A.xzxz;
        vis_1B_r += sample_1.xxzz * sample_B.xzxz;
        vis_1B_i += sample_1.yyww * sample_B.xzxz;

        vis_0A_r += sample_0.yyww * sample_A.ywyw;
        vis_0A_i -= sample_0.xxzz * sample_A.ywyw;
        vis_0B_r += sample_0.yyww * sample_B.ywyw;
        vis_0B_i -= sample_0.xxzz * sample_B.ywyw;
        vis_1A_r += sample_1.yyww * sample_A.ywyw;
        vis_1A_i -= sample_1.xxzz * sample_A.ywyw;
        vis_1B_r += sample_1.yyww * sample_B.ywyw;
        vis_1B_i -= sample_1.xxzz * sample_B.ywyw;
      }
    }

    barrier(CLK_LOCAL_MEM_FENCE);
  }

  int  statY = firstStationY + 2 * statYoffset;
  uint statX = firstStationX + 2 * statXoffset;
  uint baseline = (statX * (statX + 1) / 2) + statY;

  if (NR_STATIONS % NR_STATIONS_PER_BLOCK == 0 || statY >= 0) {
    (*visibilities)[baseline    ][channel].even = vis_0A_r;
    (*visibilities)[baseline    ][channel].odd  = vis_0A_i;
    (*visibilities)[baseline + 1][channel].even = vis_1A_r;
    (*visibilities)[baseline + 1][channel].odd  = vis_1A_i;
  }

  if (NR_STATIONS % NR_STATIONS_PER_BLOCK == 0 || statY >= -1) {
    (*visibilities)[baseline + statX + 1][channel].even = vis_0B_r;
    (*visibilities)[baseline + statX + 1][channel].odd  = vis_0B_i;
    (*visibilities)[baseline + statX + 2][channel].even = vis_1B_r;
    (*visibilities)[baseline + statX + 2][channel].odd  = vis_1B_i;
  }
}
