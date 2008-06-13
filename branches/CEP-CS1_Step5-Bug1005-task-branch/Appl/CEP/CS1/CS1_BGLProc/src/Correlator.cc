//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

#include <Common/Timer.h>

#include <Correlator.h>
#include <CorrelatorAsm.h>
#include <FIR.h>

#include <map>


namespace LOFAR {
namespace CS1 {


static NSTimer computeFlagsTimer("Correlator::computeFlags()", true);
static NSTimer correlateTimer("Correlator::correlate()", true);
static NSTimer weightTimer("Correlator::weight()", true);


Correlator::Correlator(unsigned nrStations, unsigned* stationMapping, unsigned nrSamplesPerIntegration)
:
  itsNrStations(nrStations),
  itsNrBaselines(nrStations * (nrStations + 1) / 2),
  itsNrSamplesPerIntegration(nrSamplesPerIntegration),
  itsStationMapping(stationMapping),
  itsCorrelationWeights(new float[nrSamplesPerIntegration + 1])
{
  itsCorrelationWeights[0] = 0.0;

  for (unsigned i = 1; i <= nrSamplesPerIntegration; i ++) {
#if INPUT_TYPE == I4COMPLEX_TYPE
    itsCorrelationWeights[i] = 1.0 / i;
#else
    itsCorrelationWeights[i] = 1.0e-6 / i;
#endif
  }
}


Correlator::~Correlator()
{
  delete [] itsCorrelationWeights;
}


#if 1

double Correlator::computeCentroidAndValidSamples(const SparseSet<unsigned> &flags, unsigned &nrValidSamples) const
{
  unsigned sq	     = itsNrSamplesPerIntegration * itsNrSamplesPerIntegration;
  unsigned nrSamples = itsNrSamplesPerIntegration;

  for (SparseSet<unsigned>::const_iterator it = flags.getRanges().begin(); it != flags.getRanges().end(); it ++) {
    sq	      -= (it->end - it->begin) * (it->end + it->begin);
    nrSamples -= (it->end - it->begin);
  }

  nrValidSamples = nrSamples;
  return nrSamples > 0 ? (double) sq / (double) (2 * nrSamples) : .5;
}


void Correlator::computeFlagsAndCentroids(const FilteredData *filteredData, CorrelatedData *correlatedData)
{
  computeFlagsTimer.start();

  for (unsigned stat2 = 0; stat2 < itsNrStations; stat2 ++) {
    for (unsigned stat1 = 0; stat1 <= stat2; stat1 ++) {
      unsigned nrValidSamples;
      unsigned bl = baseline(stat1, stat2);

      correlatedData->centroids[bl] = computeCentroidAndValidSamples(filteredData->flags[itsStationMapping[stat1]] | filteredData->flags[itsStationMapping[stat2]], nrValidSamples);
      correlatedData->nrValidSamples[bl][0] = 0; // channel 0 does not contain valid data

      for (unsigned ch = 1; ch < NR_SUBBAND_CHANNELS; ch ++)
	correlatedData->nrValidSamples[bl][ch] = nrValidSamples;
    }
  }

  computeFlagsTimer.stop();
}

#else

void Correlator::computeFlags(const FilteredData *filteredData, CorrelatedData *correlatedData)
{
  computeFlagsTimer.start();

  for (unsigned stat2 = 0; stat2 < itsNrStations; stat2 ++) {
    for (unsigned stat1 = 0; stat1 <= stat2; stat1 ++) {
      unsigned bl             = baseline(stat1, stat2);
      unsigned nrValidSamples = itsNrSamplesPerIntegration - (filteredData->flags[itsStationMapping[stat1]] | filteredData->flags[itsStationMapping[stat2]]).count();

      correlatedData->nrValidSamples[bl][0] = 0; // channel 0 does not contain valid data

      for (unsigned ch = 1; ch < NR_SUBBAND_CHANNELS; ch ++)
	correlatedData->nrValidSamples[bl][ch] = nrValidSamples;
    }
  }

  computeFlagsTimer.stop();
}

#endif


void Correlator::correlate(const FilteredData *filteredData, CorrelatedData *correlatedData)
{
# if 0
  cout << "correlating " << itsNrStations << " stations" << endl;
  for (unsigned stat = 0; stat < itsNrStations; stat ++) {
    cout << "   station " << stat << " -> " << itsStationMapping[stat] << endl;
  }
#endif

  correlateTimer.start();

#if defined CORRELATOR_C_IMPLEMENTATION
  for (unsigned ch = 0; ch < NR_SUBBAND_CHANNELS; ch ++) {
    for (unsigned stat2 = 0; stat2 < itsNrStations; stat2 ++) {
      for (unsigned stat1 = 0; stat1 <= stat2; stat1 ++) {
	unsigned bl = baseline(stat1, stat2), nrValid = 0;

	if (ch > 0 /* && !itsRFIflags[stat1][ch] && !itsRFIflags[stat2][ch] */) {
	  nrValid = correlatedData->nrValidSamples[bl][ch];
	  for (unsigned pol1 = 0; pol1 < NR_POLARIZATIONS; pol1 ++) {
	    for (unsigned pol2 = 0; pol2 < NR_POLARIZATIONS; pol2 ++) {
	      dcomplex sum = makedcomplex(0, 0);
	      for (unsigned time = 0; time < itsNrSamplesPerIntegration; time ++) {
		sum +=      filteredData->samples[ch][itsStationMapping[stat1]][time][pol1] 
                     * conj(filteredData->samples[ch][itsStationMapping[stat2]][time][pol2]);
	      }
	      sum *= itsCorrelationWeights[nrValid] * FIR::bandPassCorrectionFactors[ch];
	      correlatedData->visibilities[bl][ch][pol1][pol2] = sum;
	    }
	  }
	}
    
	if (nrValid == 0) {
	  for (unsigned pol1 = 0; pol1 < NR_POLARIZATIONS; pol1 ++) {
	    for (unsigned pol2 = 0; pol2 < NR_POLARIZATIONS; pol2 ++) {
	      correlatedData->visibilities[bl][ch][pol1][pol2] = makefcomplex(0, 0);
	    }
	  }
	}

	//nrValidSamples[bl][ch] = nrValid;
      }
    }
  }
#else
  // Blue Gene/L assembler version. 

  for (unsigned ch = 1; ch < NR_SUBBAND_CHANNELS; ch ++) {
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

    if (nrValidStations == 0) {
      break;
    }

    // Divide the correlation matrix into blocks of 3x2, 2x2, 3+2, 2+1, and 1x1.

    // do the first (auto)correlation(s) (these are the "left"most 1 or 3
    // squares in the corner of the triangle)
    if (nrValidStations % 2 == 0) {
      unsigned stat10 = map[0], stat11 = map[1];

      _auto_correlate_2(filteredData->samples[ch][itsStationMapping[stat10]].origin(),
			filteredData->samples[ch][itsStationMapping[stat11]].origin(),
			correlatedData->visibilities[baseline(stat10, stat10)][ch].origin(),
			correlatedData->visibilities[baseline(stat10, stat11)][ch].origin(),
			correlatedData->visibilities[baseline(stat11, stat11)][ch].origin(),
			itsNrSamplesPerIntegration);
    } else {
      unsigned stat10 = map[0];

      _auto_correlate_1(filteredData->samples[ch][itsStationMapping[stat10]].origin(),
			correlatedData->visibilities[baseline(stat10, stat10)][ch].origin(),
			itsNrSamplesPerIntegration);
    }

    for (unsigned stat2 = nrValidStations % 2 ? 1 : 2; stat2 < nrValidStations; stat2 += 2) {
      unsigned stat1 = 0;

      // do as many 3x2 blocks as possible
      for (; stat1 + 3 <= stat2; stat1 += 3) { 
	unsigned stat10 = map[stat1], stat11 = map[stat1+1], stat12 = map[stat1+2];
	unsigned stat20 = map[stat2], stat21 = map[stat2+1];

	_correlate_3x2(filteredData->samples[ch][itsStationMapping[stat10]].origin(),
		       filteredData->samples[ch][itsStationMapping[stat11]].origin(),
		       filteredData->samples[ch][itsStationMapping[stat12]].origin(),
		       filteredData->samples[ch][itsStationMapping[stat20]].origin(),
		       filteredData->samples[ch][itsStationMapping[stat21]].origin(),
		       correlatedData->visibilities[baseline(stat10, stat20)][ch].origin(),
		       correlatedData->visibilities[baseline(stat10, stat21)][ch].origin(),
		       correlatedData->visibilities[baseline(stat11, stat20)][ch].origin(),
		       correlatedData->visibilities[baseline(stat11, stat21)][ch].origin(),
		       correlatedData->visibilities[baseline(stat12, stat20)][ch].origin(),
		       correlatedData->visibilities[baseline(stat12, stat21)][ch].origin(),
		       itsNrSamplesPerIntegration);
      }

      // see if a 2x2 block is necessary
      if (stat1 + 2 <= stat2) {
	unsigned stat10 = map[stat1], stat11 = map[stat1+1];
	unsigned stat20 = map[stat2], stat21 = map[stat2+1];

	_correlate_2x2(filteredData->samples[ch][itsStationMapping[stat10]].origin(),
		       filteredData->samples[ch][itsStationMapping[stat11]].origin(),
		       filteredData->samples[ch][itsStationMapping[stat20]].origin(),
		       filteredData->samples[ch][itsStationMapping[stat21]].origin(),
		       correlatedData->visibilities[baseline(stat10, stat20)][ch].origin(),
		       correlatedData->visibilities[baseline(stat10, stat21)][ch].origin(),
		       correlatedData->visibilities[baseline(stat11, stat20)][ch].origin(),
		       correlatedData->visibilities[baseline(stat11, stat21)][ch].origin(),
		       itsNrSamplesPerIntegration);
	stat1 += 2;
      }

      // do the remaining (auto)correlations near the diagonal
      if (stat1 == stat2) {
	unsigned stat10 = map[stat1], stat11 = map[stat1+1];

	_auto_correlate_2(filteredData->samples[ch][itsStationMapping[stat10]].origin(),
			  filteredData->samples[ch][itsStationMapping[stat11]].origin(),
			  correlatedData->visibilities[baseline(stat10,stat10)][ch].origin(),
			  correlatedData->visibilities[baseline(stat10,stat11)][ch].origin(),
			  correlatedData->visibilities[baseline(stat11,stat11)][ch].origin(),
			  itsNrSamplesPerIntegration);
      } else {
	unsigned stat10 = map[stat1], stat11 = map[stat1+1], stat12 = map[stat1+2];

	_auto_correlate_3(filteredData->samples[ch][itsStationMapping[stat10]].origin(),
			  filteredData->samples[ch][itsStationMapping[stat11]].origin(),
			  filteredData->samples[ch][itsStationMapping[stat12]].origin(),
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
#if 0
  for (unsigned bl = 0; bl < itsNrBaselines; bl ++) {
    for (unsigned ch = 0; ch < NR_SUBBAND_CHANNELS; ch ++) {
      for (unsigned pol1 = 0; pol1 < NR_POLARIZATIONS; pol1 ++) {
	for (unsigned pol2 = 0; pol2 < NR_POLARIZATIONS; pol2 ++) {
	  itsCorrelatedData->visibilities[bl][ch][pol1][pol2] *= itsCorrelationWeights[(*nrValidSamples)[bl][ch]];
	}
      }
    }
  }
#else
  _weigh_visibilities(correlatedData->visibilities.origin(), correlatedData->nrValidSamples.origin(), itsCorrelationWeights, FIR::bandPassCorrectionFactors, itsNrBaselines * NR_SUBBAND_CHANNELS);
#endif
  weightTimer.stop();
#endif  

#if 0
static unsigned count;
nrValidSamples[0][0] = itsCurrentSubband;
nrValidSamples[0][1] = TH_MPI::getCurrentRank();
nrValidSamples[0][2] = count ++;
#endif
  correlateTimer.stop();
}


} // namespace CS1
} // namespace LOFAR
