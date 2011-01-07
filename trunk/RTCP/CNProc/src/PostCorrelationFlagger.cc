//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

#include <Common/Timer.h>

#include <PostCorrelationFlagger.h>
#include <Correlator.h>

namespace LOFAR {
namespace RTCP {

static NSTimer RFIStatsTimer("RFI post statistics calculations", true, true);
static NSTimer thresholdingFlaggerTimer("RFI post Thresholding flagger", true, true);
static NSTimer detectBrokenStationsTimer("RFI post DetectBrokenStations", true, true);

  // CorrelatedData samples: [nrBaselines][nrChannels][NR_POLARIZATIONS][NR_POLARIZATIONS]

  PostCorrelationFlagger::PostCorrelationFlagger(const unsigned nrStations, const unsigned nrChannels, float cutoffThreshold)
:
  Flagger(nrStations, nrChannels, (nrStations * (nrStations + 1) / 2) * nrChannels * NR_POLARIZATIONS * NR_POLARIZATIONS, cutoffThreshold),
  itsNrBaselines((nrStations * (nrStations + 1) / 2))
{
  itsPowers.resize(itsTotalNrSamples);
  itsSummedBaselinePowers.resize(itsNrBaselines);
  itsSummedStationPowers.resize(itsNrStations);
}


void PostCorrelationFlagger::flag(CorrelatedData* correlatedData)
{
  calculateGlobalStatistics(correlatedData);
  thresholdingFlagger(correlatedData);
}


void PostCorrelationFlagger::thresholdingFlagger(CorrelatedData* correlatedData)
{
  thresholdingFlaggerTimer.start();
  
  float threshold = itsPowerMedian + itsCutoffThreshold * itsPowerStdDev;

  unsigned index = 0;
  unsigned totalSamplesFlagged = 0;

  for(unsigned baseline = 0; baseline < itsNrBaselines; baseline++) {
    for(unsigned channel = 0; channel < itsNrChannels; channel++) {
      for(unsigned pol1 = 0; pol1< NR_POLARIZATIONS; pol1++) {
	for(unsigned pol2 = 0; pol2< NR_POLARIZATIONS; pol2++) {
	  const float power = itsPowers[index++];

	  if(power > threshold) {
	    // flag this sample, both polarizations.
	    // TODO: currently, we can only flag all channels at once! This is a limitation in CorrelatedData.
	    //	    correlatedData->flags[station].include(time, time);

#ifdef LOFAR_STMAN_V2      
	    correlatedData->nrValidSamplesV2[baseline] = 0;
#else
	    correlatedData->nrValidSamples[baseline][channel] = 0;
#endif

	    totalSamplesFlagged++;
	  }
	}
      }
    }
  }

  thresholdingFlaggerTimer.stop();

  float percentageFlagged = totalSamplesFlagged * 100.0f / itsPowers.size();

  LOG_DEBUG_STR("RFI post thresholdingFlagger: flagged " << totalSamplesFlagged << " samples, " << percentageFlagged << " %");
}


void PostCorrelationFlagger::detectBrokenStations()
{
  detectBrokenStationsTimer.start();

  // Sum all baselines that involve a station (both horizontally and vertically).
  float total = 0.0f;

  for(unsigned station=0; station < itsNrStations; station++) {
    float sum = 0.0f;
    for(unsigned stationH=station; stationH < itsNrStations; stationH++) {
      for(unsigned stationV=0; stationV < station; stationV++) {
	unsigned baseline = Correlator::baseline(stationH, stationV);
	sum += itsSummedBaselinePowers[baseline];
      }
    } 
    itsSummedStationPowers[station] = sum;
    total += sum;
  }

  float mean = total / itsNrStations;
  float stdDev = calculateStdDev(itsSummedStationPowers, mean);
  float median = calculateMedian(itsSummedStationPowers);
  float threshold = mean + itsCutoffThreshold * stdDev;

  LOG_DEBUG_STR("RFI post detectBrokenStations: mean = " << mean << ", median = " << median << " stdDev = " << stdDev << ", threshold = " << threshold);

  for(unsigned station=0; station < itsNrStations; station++) {
    if(itsSummedStationPowers[station] > threshold) {
	LOG_INFO_STR("RFI post WARNING, station " << station << " seems to be corrupted, total summed power = " << itsSummedStationPowers[station]);
    }
  }

  detectBrokenStationsTimer.stop();
}


void PostCorrelationFlagger::calculateGlobalStatistics(CorrelatedData* correlatedData)
{
  RFIStatsTimer.start();

  float sum = 0.0f;
  unsigned index = 0;

  for(unsigned baseline = 0; baseline < itsNrBaselines; baseline++) {
    float baselineSum = 0.0f;
    for(unsigned channel = 0; channel < itsNrChannels; channel++) {
      for(unsigned pol1 = 0; pol1< NR_POLARIZATIONS; pol1++) {
	for(unsigned pol2 = 0; pol2< NR_POLARIZATIONS; pol2++) {
	  fcomplex sample = correlatedData->visibilities[baseline][channel][pol1][pol2];
	  float power = real(sample) * real(sample) + imag(sample) * imag(sample);
	  itsPowers[index++] = power;
	  baselineSum += power;
	}
      }
    }
    itsSummedBaselinePowers[baseline] = baselineSum;
    sum += baselineSum;
  }
 
  itsPowerMean = sum / itsTotalNrSamples;

  itsPowerStdDev = calculateStdDev(itsPowers, itsPowerMean);
  itsPowerMedian = calculateMedian(itsPowers);

  RFIStatsTimer.stop();

  LOG_DEBUG_STR("RFI post global stats: mean = " << itsPowerMean << ", median = " << itsPowerMedian << ", stddev = " << itsPowerStdDev);
}


} // namespace RTCP
} // namespace LOFAR
