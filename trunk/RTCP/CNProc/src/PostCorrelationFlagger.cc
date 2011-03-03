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

PostCorrelationFlagger::PostCorrelationFlagger(const unsigned nrStations, const unsigned nrChannels, float cutoffThreshold) :
    Flagger(nrStations, nrChannels, cutoffThreshold), itsNrBaselines((nrStations * (nrStations + 1) / 2))
{
  itsSummedBaselinePowers.resize(itsNrBaselines);
  itsSummedStationPowers.resize(itsNrStations);
}

void PostCorrelationFlagger::flag(CorrelatedData* correlatedData)
{
  MultiDimArray<float,3> powers;
  powers.resize(boost::extents[itsNrChannels][NR_POLARIZATIONS][NR_POLARIZATIONS]);

  float mean;
  float stdDev;
  float median;

  for (unsigned baseline = 0; baseline < itsNrBaselines; baseline++) {
      calculateStatistics(baseline, correlatedData, powers, mean, stdDev, median);
      thresholdingFlagger(baseline, correlatedData, powers, mean, stdDev, median);
  }
}


void PostCorrelationFlagger::thresholdingFlagger(unsigned baseline, CorrelatedData* correlatedData, MultiDimArray<float,3> &powers, float& mean, float& stdDev, float& median)
{
  thresholdingFlaggerTimer.start();

  float threshold = median + itsCutoffThreshold * stdDev;
  unsigned totalSamplesFlagged = 0;

  for (unsigned channel = 0; channel < itsNrChannels; channel++) {
      for (unsigned pol1 = 0; pol1 < NR_POLARIZATIONS; pol1++) {
	  for (unsigned pol2 = 0; pol2 < NR_POLARIZATIONS; pol2++) {
	      const float power = powers[channel][pol1][pol2];
	      
	      if (power > threshold) {
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
    
  thresholdingFlaggerTimer.stop();

  float percentageFlagged = totalSamplesFlagged * 100.0f / powers.size();

  LOG_DEBUG_STR("RFI post thresholdingFlagger: flagged " << totalSamplesFlagged << " samples, " << percentageFlagged << " %");
}


void PostCorrelationFlagger::detectBrokenStations()
{
  detectBrokenStationsTimer.start();

  // Sum all baselines that involve a station (both horizontally and vertically).
  float total = 0.0f;

  for (unsigned station = 0; station < itsNrStations; station++) {
    float sum = 0.0f;
    for (unsigned stationH = station; stationH < itsNrStations; stationH++) { // also count autocorrelation
        unsigned baseline = Correlator::baseline(station, stationH);
        sum += itsSummedBaselinePowers[baseline];
    }
    for (unsigned stationV = 0; stationV < station; stationV++) {
        unsigned baseline = Correlator::baseline(stationV, station);
        sum += itsSummedBaselinePowers[baseline];
    }
    
    itsSummedStationPowers[station] = sum;
    total += sum;
  }

  float mean = total / itsNrStations;
  float stdDev = calculateStdDev(itsSummedStationPowers.data(), itsSummedStationPowers.size(), mean);
  float median = calculateMedian(itsSummedStationPowers.data(), itsSummedStationPowers.size());
  float threshold = mean + itsCutoffThreshold * stdDev;

  LOG_DEBUG_STR("RFI post detectBrokenStations: mean = " << mean << ", median = " << median << " stdDev = " << stdDev << ", threshold = " << threshold);

  for (unsigned station = 0; station < itsNrStations; station++) {
    LOG_INFO_STR("RFI post detectBrokenStations: station " << station << " total summed power = " << itsSummedStationPowers[station]);
    if (itsSummedStationPowers[station] > threshold) {
      LOG_INFO_STR("RFI post detectBrokenStations: WARNING, station " << station << " seems to be corrupted, total summed power = " << itsSummedStationPowers[station]);
    }
  }

  detectBrokenStationsTimer.stop();
}


void PostCorrelationFlagger::calculateStatistics(unsigned baseline, CorrelatedData* correlatedData, MultiDimArray<float,3> &powers, float& mean, float& stdDev, float& median)
{
  RFIStatsTimer.start();

  float sum = 0.0f;
  for (unsigned channel = 0; channel < itsNrChannels; channel++) {
      for (unsigned pol1 = 0; pol1 < NR_POLARIZATIONS; pol1++) {
	  for (unsigned pol2 = 0; pol2 < NR_POLARIZATIONS; pol2++) {
	      fcomplex sample = correlatedData->visibilities[baseline][channel][pol1][pol2];
	      float power = real(sample) * real(sample) + imag(sample) * imag(sample);
	      powers[channel][pol1][pol2] = power;
	      sum += power;
	  }
      }
  }

  itsSummedBaselinePowers[baseline] = sum;

  mean = sum / ((itsNrChannels-1) * NR_POLARIZATIONS * NR_POLARIZATIONS);

  stdDev = calculateStdDev(powers.data(), powers.size(), mean);
  median = calculateMedian(powers.data(), powers.size());

  RFIStatsTimer.stop();

  LOG_DEBUG_STR("RFI post global stats baseline " << baseline << ": sum = " << sum << ", mean = " << mean << ", median = " << median << ", stddev = " << stdDev);
}

} // namespace RTCP
} // namespace LOFAR
