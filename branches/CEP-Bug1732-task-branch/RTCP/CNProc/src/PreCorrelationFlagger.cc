//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

#include <Common/Timer.h>

#include <PreCorrelationFlagger.h>

#define APPLY_FLAGS 0

namespace LOFAR {
namespace RTCP {

// FilteredData samples: [nrChannels][nrStations][nrSamplesPerIntegration][NR_POLARIZATIONS]
// FilteredData flags:   std::vector<SparseSet<unsigned> >  flags;
// Always flag poth polarizations as a unit.

PreCorrelationFlagger::PreCorrelationFlagger(const Parset& parset, const unsigned nrStations, const unsigned nrChannels, const unsigned nrSamplesPerIntegration,
					     const float cutoffThreshold)
:
  Flagger(parset, nrStations, nrChannels, cutoffThreshold, /*baseSentitivity*/ 1.0f, 
	  getFlaggerStatisticsType(parset.onlinePreCorrelationFlaggingStatisticsType(getFlaggerStatisticsTypeString(FLAGGER_STATISTICS_WINSORIZED)))),
  itsFlaggerType(getFlaggerType(parset.onlinePreCorrelationFlaggingType(getFlaggerTypeString(PRE_FLAGGER_THRESHOLD)))), 
  itsNrSamplesPerIntegration(nrSamplesPerIntegration)
{
  itsPowers.resize(boost::extents[itsNrChannels][itsNrSamplesPerIntegration]);
  itsIntegratedPowers.resize(itsNrChannels);
  itsFlags.resize(boost::extents[itsNrChannels][itsNrSamplesPerIntegration]);
  itsIntegratedFlags.resize(itsNrChannels);

  LOG_DEBUG_STR("pre correlation flagging type = " << getFlaggerTypeString()
		<< ", statistics type = " << getFlaggerStatisticsTypeString());
}


void PreCorrelationFlagger::flag(FilteredData* filteredData)
{
  NSTimer flaggerTimer("RFI pre flagger", true, true);
  flaggerTimer.start();

  for(unsigned station = 0; station < itsNrStations; station++) {
    wipeFlags();

    for (unsigned pol = 0; pol < NR_POLARIZATIONS; pol++) {
      calculatePowers(station, pol, filteredData);

      switch(itsFlaggerType) {
      case PRE_FLAGGER_THRESHOLD:
	thresholdingFlagger2D(itsPowers, itsFlags);
	break;
      case PRE_FLAGGER_INTEGRATED_THRESHOLD:
	integratingThresholdingFlagger(itsPowers, itsIntegratedPowers, itsIntegratedFlags);
	break;
      case PRE_FLAGGER_INTEGRATED_SUM_THRESHOLD:
	integratingSumThresholdFlagger(itsPowers, itsIntegratedPowers, itsIntegratedFlags);
  break;
      default:
	LOG_INFO_STR("ERROR, illegal FlaggerType. Skipping online pre correlation flagger.");
	return;
      }
    }

    applyFlags(filteredData, station);
  }

  flaggerTimer.stop();
}


void PreCorrelationFlagger::integratingThresholdingFlagger(const MultiDimArray<float,2> &powers, vector<float> &integratedPowers, vector<bool> &integratedFlags)
{
  integratePowers(powers, integratedPowers);
  thresholdingFlagger1D(integratedPowers, integratedFlags);
}


void PreCorrelationFlagger::integratingSumThresholdFlagger(const MultiDimArray<float,2> &powers, vector<float> &integratedPowers, vector<bool> &integratedFlags) {
  integratePowers(powers, integratedPowers);
  sumThresholdFlagger1D(integratedPowers, integratedFlags, itsBaseSensitivity);
}


void PreCorrelationFlagger::integratePowers(const MultiDimArray<float,2> &powers, vector<float> &integratedPowers) {
  // sum all powers over time to increase the signal-to-noise-ratio
  for (unsigned channel = 0; channel < itsNrChannels; channel++) {
    float powerSum = 0.0f;
    for (unsigned time = 0; time < itsNrSamplesPerIntegration; time++) {
      powerSum += powers[channel][time];
    }
    integratedPowers[channel] = powerSum;
  }
}


void PreCorrelationFlagger::calculatePowers(unsigned station, unsigned pol, FilteredData* filteredData) {
  for (unsigned channel = 0; channel < itsNrChannels; channel++) {
    for (unsigned time = 0; time < itsNrSamplesPerIntegration; time++) {
      fcomplex sample = filteredData->samples[channel][station][time][pol];
      float power = real(sample) * real(sample) + imag(sample) * imag(sample);
      itsPowers[channel][time] = power;
    }
  }
}


void PreCorrelationFlagger::wipeFlags() {
  for (unsigned channel = 0; channel < itsNrChannels; channel++) {
    for (unsigned time = 0; time < itsNrSamplesPerIntegration; time++) {
      itsFlags[channel][time] = false;
    }
    itsIntegratedFlags[channel] = false;
  }
}


void PreCorrelationFlagger::applyFlags(FilteredData* filteredData, unsigned station) {
  for (unsigned channel = 0; channel < itsNrChannels; channel++) {
    for (unsigned time = 0; time < itsNrSamplesPerIntegration; time++) {
      if(itsFlags[channel][time]) {
	flagSample(filteredData, channel, station, time);
      }
    }
  }

  for (unsigned channel = 0; channel < itsNrChannels; channel++) {
    if(itsIntegratedFlags[channel]) {
      for (unsigned time = 0; time < itsNrSamplesPerIntegration; time++) {
	flagSample(filteredData, channel, station, time);
      }
    }
  }

  wipeFlaggedSamples(filteredData);
}


void PreCorrelationFlagger::flagSample(FilteredData* filteredData, unsigned channel, unsigned station, unsigned time) {
  fcomplex zero = makefcomplex(0, 0);

#if DETAILED_FLAGS
  filteredData->detailedFlags[channel][station].include(time);
  for (unsigned pol = 0; pol < NR_POLARIZATIONS; pol++) {
    filteredData->samples[channel][station][time][pol] = zero;
  }
#else // ! DETAILED_FLAGS
#if APPLY_FLAGS
  // register 
  filteredData->flags[station].include(time);
#endif // APPLY_FLAGS
  for (unsigned pol = 0; pol < NR_POLARIZATIONS; pol++) {
    filteredData->samples[channel][station][time][pol] = zero;
  }
#endif // DETAILED_FLAGS
}


void PreCorrelationFlagger::wipeFlaggedSamples(FilteredData* filteredData) {
#if APPLY_FLAGS && !DETAILED_FLAGS
  fcomplex zero = makefcomplex(0, 0);

  // We have to wipe the flagged samples, the correlator uses them.
  for (unsigned stat = 0; stat < itsNrStations; stat++) {
    for (unsigned time = 0; time < itsNrSamplesPerIntegration; time++) {
      if (filteredData->flags[stat].test(time)) {
        for (unsigned channel = 0; channel < itsNrChannels; channel++) {
          for (unsigned pol = 0; pol < NR_POLARIZATIONS; pol++) {
            filteredData->samples[channel][stat][time][pol] = zero;
          }
        }
      }
    }
  }
#else
  (void)filteredData; // prevent compiler warning
#endif
}

PreCorrelationFlaggerType PreCorrelationFlagger::getFlaggerType(std::string t) {
  if (t.compare("THRESHOLD") == 0) {
    return PRE_FLAGGER_THRESHOLD;
  } else if (t.compare("INTEGRATED_THRESHOLD") == 0) {
    return PRE_FLAGGER_INTEGRATED_THRESHOLD;
  } else if (t.compare("SUM_THRESHOLD") == 0) {
    return PRE_FLAGGER_SUM_THRESHOLD;
  } else if (t.compare("INTEGRATED_SUM_THRESHOLD") == 0) {
    return PRE_FLAGGER_INTEGRATED_SUM_THRESHOLD;
  } else {
    LOG_DEBUG_STR("unknown flagger type, using default THRESHOLD");
    return PRE_FLAGGER_THRESHOLD;
  }
}

std::string PreCorrelationFlagger::getFlaggerTypeString(PreCorrelationFlaggerType t) {
  switch(t) {
  case PRE_FLAGGER_THRESHOLD:
    return "THRESHOLD";
  case PRE_FLAGGER_INTEGRATED_THRESHOLD:
    return "INTEGRATED_THRESHOLD";
  case PRE_FLAGGER_SUM_THRESHOLD:
    return "SUM_THRESHOLD";
  case PRE_FLAGGER_INTEGRATED_SUM_THRESHOLD:
    return "INTEGRATED_SUM_THRESHOLD";
  default:
    return "ILLEGAL FLAGGER TYPE";
  }
}

std::string PreCorrelationFlagger::getFlaggerTypeString() {
  return getFlaggerTypeString(itsFlaggerType);
}

} // namespace RTCP
} // namespace LOFAR
