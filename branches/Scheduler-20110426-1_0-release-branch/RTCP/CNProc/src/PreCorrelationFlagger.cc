//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

#include <Common/Timer.h>

#include <PreCorrelationFlagger.h>

namespace LOFAR {
namespace RTCP {

// FilteredData samples: [nrChannels][nrStations][nrSamplesPerIntegration][NR_POLARIZATIONS]
// FilteredData flags:   std::vector<SparseSet<unsigned> >  flags;

PreCorrelationFlagger::PreCorrelationFlagger(const unsigned nrStations, const unsigned nrChannels, const unsigned nrSamplesPerIntegration,
					     const float cutoffThreshold)
:
  Flagger(nrStations, nrChannels, cutoffThreshold), itsNrSamplesPerIntegration(nrSamplesPerIntegration)
{
}


void PreCorrelationFlagger::flag(FilteredData* filteredData)
{
  MultiDimArray<float,3> powers;
  powers.resize(boost::extents[itsNrChannels][itsNrSamplesPerIntegration | 2][NR_POLARIZATIONS]);
  float mean;
  float stdDev;
  float median;

  for(unsigned station = 0; station < itsNrStations; station++) {
    calculateStatistics(station, filteredData, powers, mean, stdDev, median);
    thresholdingFlagger(station, filteredData, powers, mean, stdDev, median);
  }
}


void PreCorrelationFlagger::thresholdingFlagger(const unsigned station, FilteredData* filteredData, const MultiDimArray<float,3> &powers, const float mean, const float stdDev, const float median)
{
  NSTimer thresholdingFlaggerTimer("RFI pre Thresholding flagger", true, true);
  thresholdingFlaggerTimer.start();

  float threshold = median + itsCutoffThreshold * stdDev;
  unsigned realSamplesFlagged = 0;
  unsigned totalSamplesFlagged = 0;

  for (unsigned channel = 0; channel < itsNrChannels; channel++) {
      for (unsigned time = 0; time < itsNrSamplesPerIntegration; time++) {
        // Always flag poth polarizations as a unit.
        const float powerX = powers[channel][time][0];
        const float powerY = powers[channel][time][1];

        if (powerX > threshold || powerY > threshold) {
          // flag this sample, both polarizations.
#if DETAILED_FLAGS
          filteredData->detailedFlags[channel][station].include(time, time + 1);
          filteredData->samples[channel][station][time][0] = makefcomplex(0, 0);
          filteredData->samples[channel][station][time][1] = makefcomplex(0, 0);
#else
          filteredData->flags[station].include(time, time + 1);
#endif
          realSamplesFlagged++;
        }
      }
  }

#if !DETAILED_FLAGS
  // We have to wipe the flagged samples, the correlator uses them.
  for (unsigned stat = 0; stat < itsNrStations; stat++) {
    for (unsigned time = 0; time < itsNrSamplesPerIntegration; time++) {
      if (filteredData->flags[stat].test(time)) {
        for (unsigned channel = 0; channel < itsNrChannels; channel++) {
          for (unsigned pol = 0; pol < NR_POLARIZATIONS; pol++) {
            filteredData->samples[channel][stat][time][pol] = makefcomplex(0, 0);
          }
          totalSamplesFlagged++;
        }
      }
    }
  }
#endif

  thresholdingFlaggerTimer.stop();

  float realPercentageFlagged = realSamplesFlagged * 100.0f / powers.size();
  float totalPercentageFlagged = totalSamplesFlagged * 100.0f / powers.size();

  LOG_DEBUG_STR("RFI pre thresholdingFlagger: station " << station << ": really flagged " << realSamplesFlagged << " samples, " << realPercentageFlagged
      << "%, total flagged " << totalSamplesFlagged << " samples, " << totalPercentageFlagged << " %");
}

#define SCALE 0

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
