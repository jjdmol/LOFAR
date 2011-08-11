//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

#include <Common/Timer.h>

#include <PreCorrelationFlagger.h>

#define APPLY_FLAGS 0

namespace LOFAR {
namespace RTCP {

// FilteredData samples: [nrChannels][nrStations][nrSamplesPerIntegration][NR_POLARIZATIONS]
// FilteredData flags:   std::vector<SparseSet<unsigned> >  flags;

PreCorrelationFlagger::PreCorrelationFlagger(const Parset& parset, const unsigned nrStations, const unsigned nrChannels, const unsigned nrSamplesPerIntegration,
					     const float cutoffThreshold)
:
  Flagger(parset, nrStations, nrChannels, cutoffThreshold, /*baseSentitivity*/ 1.0f, /*firstThreshold*/ 6.0f,
	  getFlaggerType(parset.onlinePreCorrelationFlaggingType(getFlaggerTypeString(FLAGGER_THRESHOLD))), 
	  getFlaggerStatisticsType(parset.onlinePreCorrelationFlaggingStatisticsType(getFlaggerStatisticsTypeString(FLAGGER_STATISTICS_WINSORIZED)))),
    itsNrSamplesPerIntegration(nrSamplesPerIntegration)
{
  itsPowers.resize(boost::extents[itsNrChannels][itsNrSamplesPerIntegration | 2][NR_POLARIZATIONS]);

  LOG_DEBUG_STR("pre correlation flagging type = " << getFlaggerTypeString()
		<< ", statistics type = " << getFlaggerStatisticsTypeString());
}


void PreCorrelationFlagger::flag(FilteredData* filteredData)
{
  float mean;
  float stdDev;
  float median;

  for(unsigned station = 0; station < itsNrStations; station++) {
    calculateStatistics(station, filteredData, itsPowers, mean, stdDev, median);
//    thresholdingFlagger(station, filteredData, itsPowers, mean, stdDev, median);
    integratingThresholdingFlagger(station, filteredData, itsPowers, mean, stdDev, median);
  }
}


void PreCorrelationFlagger::thresholdingFlagger(const unsigned station, FilteredData* filteredData, const MultiDimArray<float,3> &powers, const float /*mean*/, const float stdDev, const float median)
{
  NSTimer thresholdingFlaggerTimer("RFI pre Thresholding flagger", true, true);
  thresholdingFlaggerTimer.start();

  float threshold = median + itsCutoffThreshold * stdDev;
  unsigned realSamplesFlagged = 0;
  unsigned totalSamplesFlagged = 0;
  fcomplex zero = makefcomplex(0, 0);

  for (unsigned channel = 0; channel < itsNrChannels; channel++) {
      for (unsigned time = 0; time < itsNrSamplesPerIntegration; time++) {
        // Always flag poth polarizations as a unit.
        const float powerX = powers[channel][time][0];
        const float powerY = powers[channel][time][1];

        if (powerX > threshold || powerY > threshold) {
          // flag this sample, both polarizations.
#if DETAILED_FLAGS
          filteredData->detailedFlags[channel][station].include(time);
          filteredData->samples[channel][station][time][0] = zero;
          filteredData->samples[channel][station][time][1] = zero;
#else
#if APPLY_FLAGS
	  // register 
          filteredData->flags[station].include(time);
#else
          filteredData->samples[channel][station][time][0] = zero;
          filteredData->samples[channel][station][time][1] = zero;
#endif

#endif
          realSamplesFlagged += 2;
        }
      }
  }

#if APPLY_FLAGS && !DETAILED_FLAGS
  // We have to wipe the flagged samples, the correlator uses them.
  for (unsigned stat = 0; stat < itsNrStations; stat++) {
    for (unsigned time = 0; time < itsNrSamplesPerIntegration; time++) {
      if (filteredData->flags[stat].test(time)) {
        for (unsigned channel = 0; channel < itsNrChannels; channel++) {
          for (unsigned pol = 0; pol < NR_POLARIZATIONS; pol++) {
            filteredData->samples[channel][stat][time][pol] = zero;
          }
          totalSamplesFlagged += 2;
        }
      }
    }
  }
#endif

  thresholdingFlaggerTimer.stop();

  float realPercentageFlagged = (realSamplesFlagged * 100.0f) / (itsNrChannels * itsNrSamplesPerIntegration * NR_POLARIZATIONS);
  float totalPercentageFlagged = (totalSamplesFlagged * 100.0f) / (itsNrChannels * itsNrSamplesPerIntegration * NR_POLARIZATIONS);

  LOG_DEBUG_STR("RFI pre thresholdingFlagger: station " << station << ": really flagged " << realSamplesFlagged << " samples, " << realPercentageFlagged
      << "%, total flagged " << totalSamplesFlagged << " samples, " << totalPercentageFlagged << " %");
}


void PreCorrelationFlagger::integratingThresholdingFlagger(const unsigned station, FilteredData* filteredData, const MultiDimArray<float,3> &powers, const float /*mean*/, const float stdDev, const float median)
{
  MultiDimArray<float,2> integratedPowers;
  integratedPowers.resize(boost::extents[itsNrChannels][NR_POLARIZATIONS]);

  for (unsigned channel = 0; channel < itsNrChannels; channel++) {
    float powerSumX = 0.0f;
    float powerSumY = 0.0f;
    for (unsigned time = 0; time < itsNrSamplesPerIntegration; time++) {
      powerSumX += powers[channel][time][0];
      powerSumY += powers[channel][time][1];
    }
    integratedPowers[channel][0] = powerSumX;
    integratedPowers[channel][1] = powerSumY;
  }

  float integratedMean, integratedMedian, integratedStdDev;
  Flagger::calculateStatistics(integratedPowers.data(), integratedPowers.size(), integratedMean, integratedMedian, integratedStdDev);
  
  LOG_DEBUG_STR("INTEGRATED mean " << integratedMean << ", median " << integratedMedian << ", stddev " << integratedStdDev);

  float threshold = integratedMedian + itsCutoffThreshold * integratedStdDev;
  unsigned realSamplesFlagged = 0;
  unsigned totalSamplesFlagged = 0;
  fcomplex zero = makefcomplex(0, 0);

  for (unsigned channel = 0; channel < itsNrChannels; channel++) {
    // Always flag poth polarizations as a unit.
    const float powerX = integratedPowers[channel][0];
    const float powerY = integratedPowers[channel][1];

    if (powerX > threshold || powerY > threshold) {
      // flag this sample, both polarizations.
#if DETAILED_FLAGS
      filteredData->detailedFlags[channel][station].include(time);
      filteredData->samples[channel][station][time][0] = zero;
      filteredData->samples[channel][station][time][1] = zero;
#else
#if APPLY_FLAGS
      // register 
      filteredData->flags[station].include(0, itsNrSamplesPerIntegration);
#else
      for(unsigned time=0; time<itsNrSamplesPerIntegration; time++) {
	filteredData->samples[channel][station][time][0] = zero;
	filteredData->samples[channel][station][time][1] = zero;
      }
#endif

#endif
      realSamplesFlagged += 2 * itsNrSamplesPerIntegration;
    }
  }

  float realPercentageFlagged = (realSamplesFlagged * 100.0f) / (itsNrChannels * itsNrSamplesPerIntegration * NR_POLARIZATIONS);
  float totalPercentageFlagged = (totalSamplesFlagged * 100.0f) / (itsNrChannels * itsNrSamplesPerIntegration * NR_POLARIZATIONS);

  LOG_DEBUG_STR("RFI pre thresholdingFlagger: station " << station << ": really flagged " << realSamplesFlagged << " samples, " << realPercentageFlagged
      << "%, total flagged " << totalSamplesFlagged << " samples, " << totalPercentageFlagged << " %");
}






// SCALING makes no difference for normal threshold.
#define SCALE 1

void PreCorrelationFlagger::calculateStatistics(unsigned station, FilteredData* filteredData, MultiDimArray<float,3> &powers, float& mean, float& stdDev, float& median)
{
  NSTimer RFIStatsTimer("RFI pre statistics calculations", true, true);
  RFIStatsTimer.start();

  double sum = 0.0;

  float min = 1E10, max = -1E10;

  for (unsigned channel = 0; channel < itsNrChannels; channel++) {
      for (unsigned time = 0; time < itsNrSamplesPerIntegration; time++) {
        for (unsigned pol = 0; pol < NR_POLARIZATIONS; pol++) {
          fcomplex sample = filteredData->samples[channel][station][time][pol];
          float power = real(sample) * real(sample) + imag(sample) * imag(sample);
          sum += power;
          powers[channel][time][pol] = power;

#if SCALE
          if(power < min) min = power;
          if(power > max) max = power;
#endif // SCALE
        }
    }
  }
#if SCALE
  sum = 0.0;
  //scale
  for (unsigned channel = 0; channel < itsNrChannels; channel++) {
      for (unsigned time = 0; time < itsNrSamplesPerIntegration; time++) {
        for (unsigned pol = 0; pol < NR_POLARIZATIONS; pol++) {
          powers[channel][time][pol] /= max;
          sum += powers[channel][time][pol];
        }
    }
  }
#endif // SCALE

  mean = sum / (itsNrChannels * itsNrSamplesPerIntegration * NR_POLARIZATIONS);
  float dummy;
  calculateStdDevAndSum(powers.data(), powers.size(), mean, stdDev, dummy);
  median = calculateMedian(powers.data(), powers.size());

  RFIStatsTimer.stop();

  LOG_DEBUG_STR("RFI pre global stats: min = " << min << ", max = " << max << ", mean = " << mean << ", median = " << median << ", stddev = " << stdDev);
}

} // namespace RTCP
} // namespace LOFAR
