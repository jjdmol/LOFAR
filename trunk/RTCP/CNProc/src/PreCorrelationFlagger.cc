//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

#include <Common/Timer.h>

#include <PreCorrelationFlagger.h>


namespace LOFAR {
namespace RTCP {

static NSTimer RFIStatsTimer("RFI statistics calculations (pre correlation)", true, true);
static NSTimer thresholdingFlaggerTimer("Thresholding flagger (pre correlation)", true, true);

  // FilteredData samples: [nrChannels][nrStations][nrSamplesPerIntegration][NR_POLARIZATIONS]
  // FilteredData flags:   std::vector<SparseSet<unsigned> >  flags;

  PreCorrelationFlagger::PreCorrelationFlagger(const unsigned nrStations, const unsigned nrChannels, const unsigned nrSamplesPerIntegration, const float cutoffThreshold)
:
  Flagger(nrStations, nrChannels, nrStations * nrChannels * nrSamplesPerIntegration * NR_POLARIZATIONS, cutoffThreshold),
  itsNrSamplesPerIntegration(nrSamplesPerIntegration)
{
  itsPowers.resize(itsTotalNrSamples);
}


void PreCorrelationFlagger::flag(FilteredData* filteredData)
{
  calculateGlobalStatistics(filteredData);
  thresholdingFlagger(filteredData);
}


void PreCorrelationFlagger::thresholdingFlagger(FilteredData* filteredData)
{
  thresholdingFlaggerTimer.start();
  
  float threshold = itsPowerMean + itsCutoffThreshold * itsPowerStdDev;

  unsigned index = 0;
  unsigned totalSamplesFlagged = 0;

  for(unsigned channel = 0; channel < itsNrChannels; channel++) {
    for(unsigned station = 0; station < itsNrStations; station++) {
      for(unsigned time = 0; time < itsNrSamplesPerIntegration; time++) {
	for(unsigned pol = 0; pol< NR_POLARIZATIONS; pol++) {
	  const float power = itsPowers[index++];

	  if(power > threshold) {
	    // flag this sample, both polarizations.
	    // TODO: currently, we can only flag all channels at once! This is a limitation in FilteredData.
	    filteredData->flags[station].include(time, time);
	    totalSamplesFlagged++;
	  }
	}
      }
    }
  }

  thresholdingFlaggerTimer.stop();

  float percentageFlagged = totalSamplesFlagged * 100.0f / itsPowers.size();

  std::cerr << "thresholdingFlagger: flagged " << totalSamplesFlagged << " samples, " << percentageFlagged << " %" << std::endl;
}


void PreCorrelationFlagger::calculateGlobalStatistics(FilteredData* filteredData)
{
  RFIStatsTimer.start();

  float sum = 0.0f;
  unsigned index = 0;

  for(unsigned channel = 0; channel < itsNrChannels; channel++) {
    for(unsigned station = 0; station < itsNrStations; station++) {
      for(unsigned time = 0; time < itsNrSamplesPerIntegration; time++) {
	for(unsigned pol = 0; pol< NR_POLARIZATIONS; pol++) {
	  fcomplex sample = filteredData->samples[channel][station][time][pol];
	  float power = real(sample) * real(sample) + imag(sample) * imag(sample);
	  sum += power;
	  itsPowers[index++] = power;
	}
      }
    }
  }

  itsPowerMean = sum / itsTotalNrSamples;

  itsPowerStdDev = calculateStdDev(itsPowers, itsPowerMean);
  itsPowerMedian = calculateMedian(itsPowers);

  RFIStatsTimer.stop();

  std::cerr << "global RFI stats: mean = " << itsPowerMean << ", median = " << itsPowerMedian << ", stddev = " << itsPowerStdDev << std::endl;
}


} // namespace RTCP
} // namespace LOFAR
