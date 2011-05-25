//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

#include <Common/Timer.h>

#include <Correlator.h>
#include <CorrelatorAsm.h>

#include <Interface/FilteredData.h>

#include <map>

namespace LOFAR {
namespace RTCP {


static NSTimer computeFlagsTimer("Correlator::computeFlags()", true, true);
static NSTimer correlateTimer("Correlator::correlate()", true, true);
static NSTimer weightTimer("Correlator::weight()", true, true);


// nrStations is the number of superstations in case we use TAB.
// Stations that are not beam formed count as a station.
Correlator::Correlator(const std::vector<unsigned> &stationMapping, unsigned nrChannels, unsigned nrSamplesPerIntegration)
:
  itsNrStations(stationMapping.size()),
  itsNrBaselines(itsNrStations * (itsNrStations + 1) / 2),
  itsNrChannels(nrChannels),
  itsNrSamplesPerIntegration(nrSamplesPerIntegration),
  itsCorrelationWeights(nrSamplesPerIntegration + 1),
  itsStationMapping(stationMapping)
{
  itsCorrelationWeights[0] = 0.0;

  for (unsigned i = 1; i <= nrSamplesPerIntegration; i ++)
    itsCorrelationWeights[i] = 1.0e-6 / i;
}


template <typename T> void Correlator::setNrValidSamples(const SampleData<> *sampleData, Matrix<T> &theNrValidSamples)
{
  for (unsigned stat2 = 0; stat2 < itsNrStations; stat2 ++) {
    for (unsigned stat1 = 0; stat1 <= stat2; stat1 ++) {
      unsigned bl             = baseline(stat1, stat2);
      unsigned nrValidSamples = itsNrSamplesPerIntegration - (sampleData->flags[itsStationMapping[stat1]] | sampleData->flags[itsStationMapping[stat2]]).count();

      if (itsNrChannels == 1) {
	theNrValidSamples[bl][0] = nrValidSamples;
      } else {
	theNrValidSamples[bl][0] = 0; // channel 0 does not contain valid data

	for (unsigned ch = 1; ch < itsNrChannels; ch ++)
	  theNrValidSamples[bl][ch] = nrValidSamples;
      }
    }
  }
}


void Correlator::computeFlags(const SampleData<> *sampleData, CorrelatedData *correlatedData)
{
  computeFlagsTimer.start();

  switch (correlatedData->itsNrBytesPerNrValidSamples) {
    case 4 : setNrValidSamples(sampleData, correlatedData->itsNrValidSamples4);
	     break;

    case 2 : setNrValidSamples(sampleData, correlatedData->itsNrValidSamples2);
	     break;

    case 1 : setNrValidSamples(sampleData, correlatedData->itsNrValidSamples1);
	     break;
  }

  computeFlagsTimer.stop();
}



void Correlator::correlate(const SampleData<> *sampleData, CorrelatedData *correlatedData)
{
  ASSERT(sampleData->samples.shape()[0] == itsNrChannels);
  /* sampleData->samples.shape()[1] needs to be valid for any itsStationMapping */
  ASSERT(sampleData->samples.shape()[2] >= itsNrSamplesPerIntegration);
  ASSERT(sampleData->samples.shape()[3] == NR_POLARIZATIONS);

#if 0
  LOG_DEBUG_STR("correlating " << itsNrStations << " stations");
  for (unsigned stat = 0; stat < itsNrStations; stat ++) {
    LOG_DEBUG_STR("   station " << stat << " -> " << itsStationMapping[stat]);
  }
#endif
  correlateTimer.start();

#if defined CORRELATOR_C_IMPLEMENTATION
  for (unsigned ch = 0; ch < itsNrChannels; ch ++) {
    for (unsigned stat2 = 0; stat2 < itsNrStations; stat2 ++) {
      for (unsigned stat1 = 0; stat1 <= stat2; stat1 ++) { 
	unsigned bl	 = baseline(stat1, stat2);
	unsigned nrValid = 0;

	if ((ch > 0 || itsNrChannels == 1) /* && !itsRFIflags[stat1][ch] && !itsRFIflags[stat2][ch] */) {
	  nrValid = correlatedData->nrValidSamples(bl, ch);

	  for (unsigned pol1 = 0; pol1 < NR_POLARIZATIONS; pol1 ++) {
	    for (unsigned pol2 = 0; pol2 < NR_POLARIZATIONS; pol2 ++) {
	      dcomplex sum = makedcomplex(0, 0);

	      for (unsigned time = 0; time < itsNrSamplesPerIntegration; time ++)
		sum += sampleData->samples[ch][itsStationMapping[stat1]][time][pol1] * conj(sampleData->samples[ch][itsStationMapping[stat2]][time][pol2]);

	      sum *= itsCorrelationWeights[nrValid];
	      correlatedData->visibilities[bl][ch][pol1][pol2] = sum;
	    }
	  }
	}
    
	if (nrValid == 0)
	  for (unsigned pol1 = 0; pol1 < NR_POLARIZATIONS; pol1 ++)
	    for (unsigned pol2 = 0; pol2 < NR_POLARIZATIONS; pol2 ++)
	      correlatedData->visibilities[bl][ch][pol1][pol2] = makefcomplex(0, 0);
      }
    }
  }
#else
  // Blue Gene/L assembler version. 

  for (unsigned ch = itsNrChannels == 1 ? 0 : 1; ch < itsNrChannels; ch ++) {
    // build a map of valid stations
    unsigned nrValidStations = 0, map[itsNrStations];

    for (unsigned stat2 = 0; stat2 < itsNrStations; stat2 ++) {
//    if (!itsRFIflags[stat2][ch]) {
	map[nrValidStations ++] = stat2;
//    } else { // clear correlations that involve invalided stations
//	for (unsigned stat1 = 0; stat1 < itsNrStations; stat1 ++) {
//	  unsigned bl = stat1 < stat2 ? baseline(stat1, stat2) :
//	    baseline(stat2, stat1);
//	  //_clear_correlation(&visibilities[bl][ch]);
//	  nrValidSamples[bl][ch] = 0;
//	}
//    }
    }

    if (nrValidStations == 0)
      break;

    // Divide the correlation matrix into blocks of 3x2, 2x2, 3+2, 2+1, and 1x1.

    // do the first (auto)correlation(s) (these are the "left"most 1 or 3
    // squares in the corner of the triangle)
    if (nrValidStations % 2 == 0) {
      unsigned stat10 = map[0], stat11 = map[1];

      _auto_correlate_2(sampleData->samples[ch][itsStationMapping[stat10]].origin(),
			sampleData->samples[ch][itsStationMapping[stat11]].origin(),
			correlatedData->visibilities[baseline(stat10, stat10)][ch].origin(),
			correlatedData->visibilities[baseline(stat10, stat11)][ch].origin(),
			correlatedData->visibilities[baseline(stat11, stat11)][ch].origin(),
			itsNrSamplesPerIntegration);
    } else {
      unsigned stat10 = map[0];

      _auto_correlate_1(sampleData->samples[ch][itsStationMapping[stat10]].origin(),
			correlatedData->visibilities[baseline(stat10, stat10)][ch].origin(),
			itsNrSamplesPerIntegration);
    }

    for (unsigned stat2 = nrValidStations % 2 ? 1 : 2; stat2 < nrValidStations; stat2 += 2) {
      unsigned stat1 = 0;

#if 0 && defined HAVE_BGP
      // do as many 3x2 blocks as possible
      for (; stat1 + 3 <= stat2; stat1 += 3) { 
	unsigned stat10 = map[stat1], stat11 = map[stat1+1], stat12 = map[stat1+2];
	unsigned stat20 = map[stat2], stat21 = map[stat2+1];

	_correlate_3x2(sampleData->samples[ch][itsStationMapping[stat10]].origin(),
		       sampleData->samples[ch][itsStationMapping[stat11]].origin(),
		       sampleData->samples[ch][itsStationMapping[stat12]].origin(),
		       sampleData->samples[ch][itsStationMapping[stat20]].origin(),
		       sampleData->samples[ch][itsStationMapping[stat21]].origin(),
		       correlatedData->visibilities[baseline(stat10, stat20)][ch].origin(),
		       correlatedData->visibilities[baseline(stat10, stat21)][ch].origin(),
		       correlatedData->visibilities[baseline(stat11, stat20)][ch].origin(),
		       correlatedData->visibilities[baseline(stat11, stat21)][ch].origin(),
		       correlatedData->visibilities[baseline(stat12, stat20)][ch].origin(),
		       correlatedData->visibilities[baseline(stat12, stat21)][ch].origin(),
		       itsNrSamplesPerIntegration);
      }
#endif

      // see if a 2x2 block is necessary
      for (; stat1 + 2 <= stat2; stat1 += 2) {
	unsigned stat10 = map[stat1], stat11 = map[stat1+1];
	unsigned stat20 = map[stat2], stat21 = map[stat2+1];

	_correlate_2x2(sampleData->samples[ch][itsStationMapping[stat10]].origin(),
		       sampleData->samples[ch][itsStationMapping[stat11]].origin(),
		       sampleData->samples[ch][itsStationMapping[stat20]].origin(),
		       sampleData->samples[ch][itsStationMapping[stat21]].origin(),
		       correlatedData->visibilities[baseline(stat10, stat20)][ch].origin(),
		       correlatedData->visibilities[baseline(stat10, stat21)][ch].origin(),
		       correlatedData->visibilities[baseline(stat11, stat20)][ch].origin(),
		       correlatedData->visibilities[baseline(stat11, stat21)][ch].origin(),
		       itsNrSamplesPerIntegration);
      }

      // do the remaining (auto)correlations near the diagonal
      if (stat1 == stat2) {
	unsigned stat10 = map[stat1], stat11 = map[stat1+1];

	_auto_correlate_2(sampleData->samples[ch][itsStationMapping[stat10]].origin(),
			  sampleData->samples[ch][itsStationMapping[stat11]].origin(),
			  correlatedData->visibilities[baseline(stat10,stat10)][ch].origin(),
			  correlatedData->visibilities[baseline(stat10,stat11)][ch].origin(),
			  correlatedData->visibilities[baseline(stat11,stat11)][ch].origin(),
			  itsNrSamplesPerIntegration);
      } else {
	unsigned stat10 = map[stat1], stat11 = map[stat1+1], stat12 = map[stat1+2];

	_auto_correlate_3(sampleData->samples[ch][itsStationMapping[stat10]].origin(),
			  sampleData->samples[ch][itsStationMapping[stat11]].origin(),
			  sampleData->samples[ch][itsStationMapping[stat12]].origin(),
			  correlatedData->visibilities[baseline(stat10,stat11)][ch].origin(),
			  correlatedData->visibilities[baseline(stat10,stat12)][ch].origin(),
			  correlatedData->visibilities[baseline(stat11,stat11)][ch].origin(),
			  correlatedData->visibilities[baseline(stat11,stat12)][ch].origin(),
			  correlatedData->visibilities[baseline(stat12,stat12)][ch].origin(),
			  itsNrSamplesPerIntegration);
      }
    }
  }

  weightTimer.start();

  switch (correlatedData->itsNrBytesPerNrValidSamples) {
    case 1 : _weigh_visibilities_1(correlatedData->visibilities.origin(), correlatedData->itsNrValidSamples1.origin(), &itsCorrelationWeights[0], itsNrBaselines, itsNrChannels);
	     break;

    case 2 : _weigh_visibilities_2(correlatedData->visibilities.origin(), correlatedData->itsNrValidSamples2.origin(), &itsCorrelationWeights[0], itsNrBaselines, itsNrChannels);
	     break;

    case 4 : _weigh_visibilities_4(correlatedData->visibilities.origin(), correlatedData->itsNrValidSamples4.origin(), &itsCorrelationWeights[0], itsNrBaselines, itsNrChannels);
	     break;
  }

  weightTimer.stop();
#endif  

  correlateTimer.stop();
}


} // namespace RTCP
} // namespace LOFAR
