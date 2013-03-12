#include "math.cl"

#define NR_STATIONS_PER_BLOCK   32
#define NR_TIMES_PER_BLOCK      8

#define NR_BASELINES            (NR_STATIONS * (NR_STATIONS + 1) / 2)


typedef __global fcomplex2 (*CorrectedDataType)[NR_STATIONS][NR_CHANNELS][NR_SAMPLES_PER_CHANNEL];
typedef __global fcomplex4 (*VisibilitiesType)[NR_BASELINES][NR_CHANNELS];


__kernel
void correlateTriangleKernel(__global void *visibilitiesPtr,
                             __global const void *correctedDataPtr)
{
  VisibilitiesType visibilities = (VisibilitiesType) visibilitiesPtr;
  CorrectedDataType correctedData = (CorrectedDataType) correctedDataPtr;

  __local fcomplex2 samples[2][NR_TIMES_PER_BLOCK][NR_STATIONS_PER_BLOCK / 2 | 1];
  uint channel = get_global_id(2) + 1;
  uint block = get_global_id(1);

#if NR_STATIONS % NR_STATIONS_PER_BLOCK == 0
  uint firstStation = block * NR_STATIONS_PER_BLOCK;
  uint nrStationsThisBlock = NR_STATIONS_PER_BLOCK;
#else
  uint lastStation = block * NR_STATIONS_PER_BLOCK + NR_STATIONS % NR_STATIONS_PER_BLOCK;
  uint firstStation = block == 0 ? 0 : lastStation - NR_STATIONS_PER_BLOCK;
  uint nrStationsThisBlock = lastStation - firstStation;
#endif

  uint miniBlock = get_local_id(0);
  uint statXoffset = convert_uint_rtz(sqrt(convert_float(8 * miniBlock + 1)) - 0.99999f) / 2;
  uint statYoffset = miniBlock - statXoffset * (statXoffset + 1) / 2;

  statXoffset *= 2, statYoffset *= 2;

  float4 vis_0A_r = 0, vis_0A_i = 0;
  float4 vis_0B_r = 0, vis_0B_i = 0;
  float4 vis_1A_r = 0, vis_1A_i = 0;
  float4 vis_1B_r = 0, vis_1B_i = 0;

  bool doCorrelate = statXoffset < nrStationsThisBlock;

  for (uint major = 0; major < NR_SAMPLES_PER_CHANNEL; major += NR_TIMES_PER_BLOCK) {
    // load data into local memory
    barrier(CLK_LOCAL_MEM_FENCE);

    for (uint i = get_local_id(0); i < nrStationsThisBlock * NR_TIMES_PER_BLOCK; i += get_local_size(0)) {
      uint time = i % NR_TIMES_PER_BLOCK;
      uint stat = i / NR_TIMES_PER_BLOCK;

      samples[stat % 2][time][stat / 2] = (*correctedData)[firstStation + stat][channel][major + time];
    }

    barrier(CLK_LOCAL_MEM_FENCE);

#pragma unroll 1
    for (uint time = 0; time < NR_TIMES_PER_BLOCK; time++) {
      fcomplex2 sample_0, sample_1, sample_A, sample_B;

      if (doCorrelate) {
        sample_0 = samples[0][time][statYoffset / 2];
        sample_A = samples[0][time][statXoffset / 2];
        sample_B = samples[1][time][statXoffset / 2];
        sample_1 = samples[1][time][statYoffset / 2];

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
  }

  int statY = firstStation + statYoffset;
  uint statX = firstStation + statXoffset;
  uint baseline = (statX * (statX + 1) / 2) + statY;

  if (statXoffset < nrStationsThisBlock) {
    (*visibilities)[baseline            ][channel].even = vis_0A_r;
    (*visibilities)[baseline            ][channel].odd = vis_0A_i;
  }

  if (statXoffset < nrStationsThisBlock && statYoffset + 1 < nrStationsThisBlock) {
    (*visibilities)[baseline + 1][channel].even = vis_1A_r;
    (*visibilities)[baseline + 1][channel].odd = vis_1A_i;
  }

  if (statXoffset + 1 < nrStationsThisBlock) {
    (*visibilities)[baseline + statX + 1][channel].even = vis_0B_r;
    (*visibilities)[baseline + statX + 1][channel].odd = vis_0B_i;
    (*visibilities)[baseline + statX + 2][channel].even = vis_1B_r;
    (*visibilities)[baseline + statX + 2][channel].odd = vis_1B_i;
  }
}


__kernel __attribute__((reqd_work_group_size(NR_STATIONS_PER_BLOCK * NR_STATIONS_PER_BLOCK / 4, 1, 1)))
void correlateRectangleKernel(__global void *visibilitiesPtr,
                              __global const void *correctedDataPtr)
{
  VisibilitiesType visibilities = (VisibilitiesType)  visibilitiesPtr;
  CorrectedDataType correctedData = (CorrectedDataType) correctedDataPtr;

  __local fcomplex2 samplesX[2][NR_TIMES_PER_BLOCK][NR_STATIONS_PER_BLOCK / 2 | 1];
  __local fcomplex2 samplesY[2][NR_TIMES_PER_BLOCK][NR_STATIONS_PER_BLOCK / 2 | 1];

  uint block = get_global_id(1);
  uint blockX = convert_uint_rtz(sqrt(convert_float(8 * block + 1)) - 0.99999f) / 2;
  uint blockY = block - blockX * (blockX + 1) / 2;

#if NR_STATIONS % NR_STATIONS_PER_BLOCK == 0
  uint firstStationX = (blockX + 1) * NR_STATIONS_PER_BLOCK;
  uint firstStationY = blockY * NR_STATIONS_PER_BLOCK;
#else
  uint firstStationX = blockX * NR_STATIONS_PER_BLOCK + NR_STATIONS % NR_STATIONS_PER_BLOCK;
  int firstStationY = (blockY - 1) * NR_STATIONS_PER_BLOCK + NR_STATIONS % NR_STATIONS_PER_BLOCK;
#endif

  uint statXoffset = get_local_id(0) / (NR_STATIONS_PER_BLOCK / 2);
  uint statYoffset = get_local_id(0) % (NR_STATIONS_PER_BLOCK / 2);

  float4 vis_0A_r = 0, vis_0A_i = 0;
  float4 vis_0B_r = 0, vis_0B_i = 0;
  float4 vis_1A_r = 0, vis_1A_i = 0;
  float4 vis_1B_r = 0, vis_1B_i = 0;

  uint loadTime = get_local_id(0) % NR_TIMES_PER_BLOCK;
  uint loadStat = get_local_id(0) / NR_TIMES_PER_BLOCK;

  bool doCorrelateLower = NR_STATIONS % NR_STATIONS_PER_BLOCK == 0 || (int) (firstStationY + 2 * statYoffset) >= 0;
  bool doCorrelateUpper = NR_STATIONS % NR_STATIONS_PER_BLOCK == 0 || (int) (firstStationY + 2 * statYoffset) >= -1;
  bool doLoadY = NR_STATIONS % NR_STATIONS_PER_BLOCK == 0 || (int) (firstStationY + loadStat) >= 0;

  uint channel = get_global_id(2) + 1;

  for (uint major = 0; major < NR_SAMPLES_PER_CHANNEL; major += NR_TIMES_PER_BLOCK) {
    // load data into local memory
    fcomplex2 sampleX = (*correctedData)[firstStationX + loadStat][channel][major + loadTime];
    fcomplex2 sampleY;

    if (doLoadY)
      sampleY = (*correctedData)[firstStationY + loadStat][channel][major + loadTime];

    barrier(CLK_LOCAL_MEM_FENCE);

    samplesX[loadStat % 2][loadTime][loadStat / 2] = sampleX;

    if (doLoadY)
      samplesY[loadStat % 2][loadTime][loadStat / 2] = sampleY;

    barrier(CLK_LOCAL_MEM_FENCE);

#pragma unroll 1
    for (uint time = 0; time < NR_TIMES_PER_BLOCK; time++) {
      fcomplex2 sample_0, sample_1, sample_A, sample_B;

      if (doCorrelateLower) {
        sample_0 = samplesY[0][time][statYoffset];
      }

      if (doCorrelateUpper) {
        sample_A = samplesX[0][time][statXoffset];
        sample_B = samplesX[1][time][statXoffset];
        sample_1 = samplesY[1][time][statYoffset];
      }

      if (doCorrelateLower) {
        vis_0A_r += sample_0.xxzz * sample_A.xzxz;
        vis_0A_i += sample_0.yyww * sample_A.xzxz;
        vis_0B_r += sample_0.xxzz * sample_B.xzxz;
        vis_0B_i += sample_0.yyww * sample_B.xzxz;
        vis_0A_r += sample_0.yyww * sample_A.ywyw;
        vis_0A_i -= sample_0.xxzz * sample_A.ywyw;
        vis_0B_r += sample_0.yyww * sample_B.ywyw;
        vis_0B_i -= sample_0.xxzz * sample_B.ywyw;
      }

      if (doCorrelateUpper) {
        vis_1A_r += sample_1.xxzz * sample_A.xzxz;
        vis_1A_i += sample_1.yyww * sample_A.xzxz;
        vis_1B_r += sample_1.xxzz * sample_B.xzxz;
        vis_1B_i += sample_1.yyww * sample_B.xzxz;
        vis_1A_r += sample_1.yyww * sample_A.ywyw;
        vis_1A_i -= sample_1.xxzz * sample_A.ywyw;
        vis_1B_r += sample_1.yyww * sample_B.ywyw;
        vis_1B_i -= sample_1.xxzz * sample_B.ywyw;
      }
    }
  }

  int statY = firstStationY + 2 * statYoffset;
  uint statX = firstStationX + 2 * statXoffset;
  uint baseline = (statX * (statX + 1) / 2) + statY;

  if (doCorrelateLower) {
    (*visibilities)[baseline            ][channel].even = vis_0A_r;
    (*visibilities)[baseline            ][channel].odd = vis_0A_i;
    (*visibilities)[baseline + statX + 1][channel].even = vis_0B_r;
    (*visibilities)[baseline + statX + 1][channel].odd = vis_0B_i;
  }

  if (doCorrelateUpper) {
    (*visibilities)[baseline + 1][channel].even = vis_1A_r;
    (*visibilities)[baseline + 1][channel].odd = vis_1A_i;
    (*visibilities)[baseline + statX + 2][channel].even = vis_1B_r;
    (*visibilities)[baseline + statX + 2][channel].odd = vis_1B_i;
  }
}

////////////////////////////////////////////////////////////////////////////////

void correlateTriangle(VisibilitiesType visibilities,
                       CorrectedDataType correctedData,
                       __local fcomplex2 samples[2][NR_TIMES_PER_BLOCK][NR_STATIONS_PER_BLOCK / 2 | 1],
                       uint block)
{
  uint channel = get_global_id(2) + 1;

#if NR_STATIONS % NR_STATIONS_PER_BLOCK == 0
  uint firstStation = block * NR_STATIONS_PER_BLOCK;
#else
  int firstStation = (block - 1) * NR_STATIONS_PER_BLOCK + NR_STATIONS % NR_STATIONS_PER_BLOCK;
#endif

  bool doCorrelate = false, doAutoCorrelate = false, doNearAutoCorrelate = false;
  uint statXoffset, statYoffset;

  if (get_local_id(0) < 128) {
    uint miniBlock = get_local_id(0);
    uint miniBlockX = convert_uint_rtz(sqrt(convert_float(8 * miniBlock + 1)) - 0.99999f) / 2;
    uint miniBlockY = miniBlock - miniBlockX * (miniBlockX + 1) / 2;

    statXoffset = 2 * miniBlockX + 2;
    statYoffset = 2 * miniBlockY;
    doCorrelate = statXoffset < NR_STATIONS_PER_BLOCK && (NR_STATIONS % NR_STATIONS_PER_BLOCK == 0 || (int) (firstStation + statYoffset) >= 0);
  } else if (get_local_id(0) < 128 + NR_STATIONS_PER_BLOCK / 2) {
    statXoffset = statYoffset = 2 * (get_local_id(0) - 128);
    // actually, it is the visibility one right of statXoffset
    doNearAutoCorrelate = (int) (firstStation + statXoffset) >= 0;
  } else if (get_local_id(0) >= 192 && get_local_id(0) < 192 + NR_STATIONS_PER_BLOCK) {
    statXoffset = statYoffset = get_local_id(0) - 192;
    doAutoCorrelate = NR_STATIONS % NR_STATIONS_PER_BLOCK == 0 || (int) (firstStation + statYoffset) >= 0;
  }

  float4 vis_0A_r = 0, vis_0A_i = 0;
  float4 vis_0B_r = 0, vis_0B_i = 0;
  float4 vis_1A_r = 0, vis_1A_i = 0;
  float4 vis_1B_r = 0, vis_1B_i = 0;

  uint loadTime = get_local_id(0) % NR_TIMES_PER_BLOCK;
  uint loadStat = get_local_id(0) / NR_TIMES_PER_BLOCK;

  bool doLoad = NR_STATIONS % NR_STATIONS_PER_BLOCK == 0 || (int) (firstStation + loadStat) >= 0;

  for (uint major = 0; major < NR_SAMPLES_PER_CHANNEL; major += NR_TIMES_PER_BLOCK) {
    // load data into local memory
    fcomplex2 sample;

    if (doLoad)
      sample = (*correctedData)[firstStation + loadStat][channel][major + loadTime];

    barrier(CLK_LOCAL_MEM_FENCE);

    if (doLoad)
      samples[loadStat % 2][loadTime][loadStat / 2] = sample;

    barrier(CLK_LOCAL_MEM_FENCE);

#pragma unroll 1
    for (uint time = 0; time < NR_TIMES_PER_BLOCK; time++) {
      if (doCorrelate) {
        fcomplex2 sample_0 = samples[0][time][statYoffset / 2];
        fcomplex2 sample_A = samples[0][time][statXoffset / 2];
        fcomplex2 sample_B = samples[1][time][statXoffset / 2];
        fcomplex2 sample_1 = samples[1][time][statYoffset / 2];

        vis_0A_r += sample_0.xxzz * sample_A.xzxz;
        vis_0A_i += sample_0.yyww * sample_A.xzxz;
        vis_0B_r += sample_0.xxzz * sample_B.xzxz;
        vis_0B_i += sample_0.yyww * sample_B.xzxz;
        vis_0A_r += sample_0.yyww * sample_A.ywyw;
        vis_0A_i -= sample_0.xxzz * sample_A.ywyw;
        vis_0B_r += sample_0.yyww * sample_B.ywyw;
        vis_0B_i -= sample_0.xxzz * sample_B.ywyw;

        vis_1A_r += sample_1.xxzz * sample_A.xzxz;
        vis_1A_i += sample_1.yyww * sample_A.xzxz;
        vis_1B_r += sample_1.xxzz * sample_B.xzxz;
        vis_1B_i += sample_1.yyww * sample_B.xzxz;
        vis_1A_r += sample_1.yyww * sample_A.ywyw;
        vis_1A_i -= sample_1.xxzz * sample_A.ywyw;
        vis_1B_r += sample_1.yyww * sample_B.ywyw;
        vis_1B_i -= sample_1.xxzz * sample_B.ywyw;
      }

      if (doAutoCorrelate) {
        fcomplex2 sample_0 = samples[statYoffset % 2][time][statYoffset / 2];
        vis_0A_r.xyw += sample_0.xxz * sample_0.xzz;
        vis_0A_i.y += sample_0.y * sample_0.z;
        vis_0A_r.xyw += sample_0.yyw * sample_0.yww;
        vis_0A_i.y -= sample_0.x * sample_0.w;
      }

      if (doNearAutoCorrelate) {
        fcomplex2 sample_0 = samples[0][time][statYoffset / 2];
        fcomplex2 sample_B = samples[1][time][statXoffset / 2];
        vis_0B_r += sample_0.xxzz * sample_B.xzxz;
        vis_0B_i += sample_0.yyww * sample_B.xzxz;
        vis_0B_r += sample_0.yyww * sample_B.ywyw;
        vis_0B_i -= sample_0.xxzz * sample_B.ywyw;
      }
    }
  }

  if (doAutoCorrelate) {
    vis_0A_r.z = vis_0A_r.y;
    vis_0A_i.z = -vis_0A_i.y;
  }

  int statY = firstStation + statYoffset;
  uint statX = firstStation + statXoffset;
  uint baseline = (statX * (statX + 1) / 2) + statY;

  if (doCorrelate || doAutoCorrelate) {
    (*visibilities)[baseline            ][channel].even = vis_0A_r;
    (*visibilities)[baseline            ][channel].odd = vis_0A_i;
  }

  if (doCorrelate || doNearAutoCorrelate) {
    (*visibilities)[baseline + statX + 1][channel].even = vis_0B_r;
    (*visibilities)[baseline + statX + 1][channel].odd = vis_0B_i;
  }

  if (doCorrelate) {
    (*visibilities)[baseline + 1][channel].even = vis_1A_r;
    (*visibilities)[baseline + 1][channel].odd = vis_1A_i;
    (*visibilities)[baseline + statX + 2][channel].even = vis_1B_r;
    (*visibilities)[baseline + statX + 2][channel].odd = vis_1B_i;
  }
}


void correlateTriangle2(VisibilitiesType visibilities,
                        CorrectedDataType correctedData,
                        __local fcomplex2 samples[2][NR_TIMES_PER_BLOCK][NR_STATIONS_PER_BLOCK / 2 | 1],
                        uint block
                        )
{
  uint channel = get_global_id(2) + 1;

#if NR_STATIONS % NR_STATIONS_PER_BLOCK == 0
  uint firstStation = block * NR_STATIONS_PER_BLOCK;
  //uint lastStation = firstStation + NR_STATIONS_PER_BLOCK;
  uint nrStationsThisBlock = NR_STATIONS_PER_BLOCK;
#else
  uint lastStation = block * NR_STATIONS_PER_BLOCK + NR_STATIONS % NR_STATIONS_PER_BLOCK;
  uint firstStation = block == 0 ? 0 : lastStation - NR_STATIONS_PER_BLOCK;
  uint nrStationsThisBlock = lastStation - firstStation;
#endif

  uint miniBlock = get_local_id(0);
  uint statXoffset = convert_uint_rtz(sqrt(convert_float(8 * miniBlock + 1)) - 0.99999f) / 2;
  uint statYoffset = miniBlock - statXoffset * (statXoffset + 1) / 2;

  statXoffset *= 2, statYoffset *= 2;

  //bool doCorrelate = statXoffset < nrStationsThisBlock;

  float4 vis_0A_r = 0, vis_0A_i = 0;
  float4 vis_0B_r = 0, vis_0B_i = 0;
  float4 vis_1A_r = 0, vis_1A_i = 0;
  float4 vis_1B_r = 0, vis_1B_i = 0;

  uint loadTime = get_local_id(0) % NR_TIMES_PER_BLOCK;
  uint loadStat = get_local_id(0) / NR_TIMES_PER_BLOCK;

  bool doCorrelateLeft = statXoffset < nrStationsThisBlock;
  //bool doCorrelateRight = statXoffset + 1 < nrStationsThisBlock;
  bool doLoad = NR_STATIONS % NR_STATIONS_PER_BLOCK == 0 || loadStat < nrStationsThisBlock;

  for (uint major = 0; major < NR_SAMPLES_PER_CHANNEL; major += NR_TIMES_PER_BLOCK) {
    // load data into local memory
    float4 sample;

    if (doLoad)
      sample = (*correctedData)[firstStation + loadStat][channel][major + loadTime];

    barrier(CLK_LOCAL_MEM_FENCE);

    if (doLoad)
      samples[loadStat % 2][loadTime][loadStat / 2] = sample;

    barrier(CLK_LOCAL_MEM_FENCE);

#pragma unroll 1
    for (uint time = 0; time < NR_TIMES_PER_BLOCK; time++) {
      float4 sample_0, sample_1, sample_A, sample_B;

      if (doCorrelateLeft) {
        sample_0 = samples[0][time][statYoffset / 2];
        sample_A = samples[0][time][statXoffset / 2];
        sample_B = samples[1][time][statXoffset / 2];
        sample_1 = samples[1][time][statYoffset / 2];

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
  }

  int statY = firstStation + statYoffset;
  uint statX = firstStation + statXoffset;
  uint baseline = (statX * (statX + 1) / 2) + statY;

  if (statXoffset < nrStationsThisBlock) {
    (*visibilities)[baseline            ][channel].even = vis_0A_r;
    (*visibilities)[baseline            ][channel].odd = vis_0A_i;
  }

  if (statXoffset < nrStationsThisBlock && statYoffset + 1 < nrStationsThisBlock) {
    (*visibilities)[baseline + 1][channel].even = vis_1A_r;
    (*visibilities)[baseline + 1][channel].odd = vis_1A_i;
  }

  if (statXoffset + 1 < nrStationsThisBlock) {
    (*visibilities)[baseline + statX + 1][channel].even = vis_0B_r;
    (*visibilities)[baseline + statX + 1][channel].odd = vis_0B_i;
    (*visibilities)[baseline + statX + 2][channel].even = vis_1B_r;
    (*visibilities)[baseline + statX + 2][channel].odd = vis_1B_i;
  }
}


void correlateRectangle(VisibilitiesType visibilities,
                        CorrectedDataType correctedData,
                        __local fcomplex2 samplesX[2][NR_TIMES_PER_BLOCK][NR_STATIONS_PER_BLOCK / 2 | 1],
                        __local fcomplex2 samplesY[2][NR_TIMES_PER_BLOCK][NR_STATIONS_PER_BLOCK / 2 | 1],
                        uint blockX,
                        uint blockY
                        )
{
  uint channel = get_global_id(2) + 1;

#if NR_STATIONS % NR_STATIONS_PER_BLOCK == 0
  uint firstStationX = blockX * NR_STATIONS_PER_BLOCK;
  uint firstStationY = blockY * NR_STATIONS_PER_BLOCK;
#else
  uint firstStationX = (blockX - 1) * NR_STATIONS_PER_BLOCK + NR_STATIONS % NR_STATIONS_PER_BLOCK;
  int firstStationY = (blockY - 1) * NR_STATIONS_PER_BLOCK + NR_STATIONS % NR_STATIONS_PER_BLOCK;
#endif

  uint statXoffset = get_local_id(0) / (NR_STATIONS_PER_BLOCK / 2);
  uint statYoffset = get_local_id(0) % (NR_STATIONS_PER_BLOCK / 2);

  float4 vis_0A_r = 0, vis_0A_i = 0;
  float4 vis_0B_r = 0, vis_0B_i = 0;
  float4 vis_1A_r = 0, vis_1A_i = 0;
  float4 vis_1B_r = 0, vis_1B_i = 0;

  uint loadTime = get_local_id(0) % NR_TIMES_PER_BLOCK;
  uint loadStat = get_local_id(0) / NR_TIMES_PER_BLOCK;

  bool doCorrelateLower = NR_STATIONS % NR_STATIONS_PER_BLOCK == 0 || (int) (firstStationY + 2 * statYoffset) >= 0;
  bool doCorrelateUpper = NR_STATIONS % NR_STATIONS_PER_BLOCK == 0 || (int) (firstStationY + 2 * statYoffset) >= -1;
  bool doLoadY = NR_STATIONS % NR_STATIONS_PER_BLOCK == 0 || (int) (firstStationY + loadStat) >= 0;

  for (uint major = 0; major < NR_SAMPLES_PER_CHANNEL; major += NR_TIMES_PER_BLOCK) {
    // load data into local memory
    fcomplex2 sampleX = (*correctedData)[firstStationX + loadStat][channel][major + loadTime];
    fcomplex2 sampleY;

    if (doLoadY)
      sampleY = (*correctedData)[firstStationY + loadStat][channel][major + loadTime];

    barrier(CLK_LOCAL_MEM_FENCE);

    samplesX[loadStat % 2][loadTime][loadStat / 2] = sampleX;

    if (doLoadY)
      samplesY[loadStat % 2][loadTime][loadStat / 2] = sampleY;

    barrier(CLK_LOCAL_MEM_FENCE);

#pragma unroll 1
    for (uint time = 0; time < NR_TIMES_PER_BLOCK; time++) {
      fcomplex2 sample_0, sample_1, sample_A, sample_B;

      if (doCorrelateLower) {
        sample_0 = samplesY[0][time][statYoffset];
      }

      if (doCorrelateUpper) {
        sample_A = samplesX[0][time][statXoffset];
        sample_B = samplesX[1][time][statXoffset];
        sample_1 = samplesY[1][time][statYoffset];
      }

      if (doCorrelateLower) {
        vis_0A_r += sample_0.xxzz * sample_A.xzxz;
        vis_0A_i += sample_0.yyww * sample_A.xzxz;
        vis_0B_r += sample_0.xxzz * sample_B.xzxz;
        vis_0B_i += sample_0.yyww * sample_B.xzxz;
        vis_0A_r += sample_0.yyww * sample_A.ywyw;
        vis_0A_i -= sample_0.xxzz * sample_A.ywyw;
        vis_0B_r += sample_0.yyww * sample_B.ywyw;
        vis_0B_i -= sample_0.xxzz * sample_B.ywyw;
      }

      if (doCorrelateUpper) {
        vis_1A_r += sample_1.xxzz * sample_A.xzxz;
        vis_1A_i += sample_1.yyww * sample_A.xzxz;
        vis_1B_r += sample_1.xxzz * sample_B.xzxz;
        vis_1B_i += sample_1.yyww * sample_B.xzxz;
        vis_1A_r += sample_1.yyww * sample_A.ywyw;
        vis_1A_i -= sample_1.xxzz * sample_A.ywyw;
        vis_1B_r += sample_1.yyww * sample_B.ywyw;
        vis_1B_i -= sample_1.xxzz * sample_B.ywyw;
      }
    }
  }

  int statY = firstStationY + 2 * statYoffset;
  uint statX = firstStationX + 2 * statXoffset;
  uint baseline = (statX * (statX + 1) / 2) + statY;

  if (doCorrelateLower) {
    (*visibilities)[baseline            ][channel].even = vis_0A_r;
    (*visibilities)[baseline            ][channel].odd = vis_0A_i;
    (*visibilities)[baseline + statX + 1][channel].even = vis_0B_r;
    (*visibilities)[baseline + statX + 1][channel].odd = vis_0B_i;
  }

  if (doCorrelateUpper) {
    (*visibilities)[baseline + 1][channel].even = vis_1A_r;
    (*visibilities)[baseline + 1][channel].odd = vis_1A_i;
    (*visibilities)[baseline + statX + 2][channel].even = vis_1B_r;
    (*visibilities)[baseline + statX + 2][channel].odd = vis_1B_i;
  }
}


__kernel __attribute__((reqd_work_group_size(NR_STATIONS_PER_BLOCK * NR_STATIONS_PER_BLOCK / 4, 1, 1)))
void correlate(__global void *visibilitiesPtr,
               __global const void *correctedDataPtr)
{
  __local fcomplex2 samplesX[2][NR_TIMES_PER_BLOCK][NR_STATIONS_PER_BLOCK / 2 | 1];
  __local fcomplex2 samplesY[2][NR_TIMES_PER_BLOCK][NR_STATIONS_PER_BLOCK / 2 | 1];

  uint block = get_global_id(1);
  uint blockX = convert_uint_rtz(sqrt(convert_float(8 * block + 1)) - 0.99999f) / 2;
  uint blockY = block - blockX * (blockX + 1) / 2;

  if (blockX == blockY)
    correlateTriangle2((VisibilitiesType) visibilitiesPtr, (CorrectedDataType) correctedDataPtr, samplesX, blockX);
  else
    correlateRectangle((VisibilitiesType) visibilitiesPtr, (CorrectedDataType) correctedDataPtr, samplesX, samplesY, blockX, blockY);
}
