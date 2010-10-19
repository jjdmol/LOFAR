//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

#include <Common/Timer.h>

#include <BeamFormer.h>

#include <map>

#define VERBOSE 0

namespace LOFAR {
namespace RTCP {

static NSTimer beamFormTimer("BeamFormer::formBeams()", true, true);

BeamFormer::BeamFormer(const unsigned nrStations, const unsigned nrSamplesPerIntegration, 
		       const std::vector<unsigned> &station2BeamFormedStation, const unsigned nrChannels)
:
  itsNrStations(nrStations), 
  itsNrChannels(nrChannels),
  itsNrSamplesPerIntegration(nrSamplesPerIntegration), 
  itsStation2BeamFormedStation(station2BeamFormedStation)
{
  itsNrBeamFormedStations = calcNrBeamFormedStations();
  if(itsNrBeamFormedStations == 0) {
#if VERBOSE
    LOG_ERROR("BeamForming disabled");
#endif
    itsStationMapping = new unsigned[nrStations];
  } else {  
#if VERBOSE
    LOG_ERROR_STR("BeamForming enabled, " << itsNrBeamFormedStations << " station(s)");
#endif
    itsStationMapping = new unsigned[itsNrBeamFormedStations];
  }

  calcMapping();
}

unsigned BeamFormer::calcNrBeamFormedStations()
{
    if(itsStation2BeamFormedStation.size() == 0) return 0;

    unsigned max = 0;
    for(unsigned i=0; i<itsStation2BeamFormedStation.size(); i++) {
	if(itsStation2BeamFormedStation[i] > max) {
	    max = itsStation2BeamFormedStation[i];
	}
    }

    return max + 1;
}

BeamFormer::~BeamFormer()
{
  delete[] itsStationMapping;
}

void BeamFormer::calcMapping()
{
  if(itsNrBeamFormedStations == 0) {
    // beamforming disabled
    for(unsigned i=0; i<itsNrStations; i++) {
      itsStationMapping[i] = i;
    }
  } else {
    unsigned nrStationsInBeam[itsNrBeamFormedStations];
    memset(nrStationsInBeam, 0, itsNrBeamFormedStations * sizeof(unsigned));
    
    itsBeamFormedStations.resize(itsNrBeamFormedStations);
    for(unsigned i=0; i<itsNrStations; i++) {
      unsigned id = itsStation2BeamFormedStation[i];
      
      itsBeamFormedStations[id].resize(nrStationsInBeam[id] + 1);
      itsBeamFormedStations[id][nrStationsInBeam[id]] = i;
      nrStationsInBeam[id]++;
    }

    for(unsigned i=0; i<itsNrBeamFormedStations; i++) {
      itsStationMapping[i] = itsBeamFormedStations[i][0];
    }

#if VERBOSE
  // dump the mapping
  LOG_ERROR("*** BeamForming mapping START");
  std::stringstream logStr;
  for(unsigned i=0; i<itsNrBeamFormedStations; i++) {
    logStr << "BeamFormed Station " << i << ": ";
    for (unsigned j=0; j<itsBeamFormedStations[i].size(); j++) {
      logStr << itsBeamFormedStations[i][j] << " ";
    }
    LOG_ERROR(logStr.str());
  }
  LOG_ERROR("*** BeamForming mapping END");
#endif
  }
}

void BeamFormer::beamFormStation(FilteredData *filteredData, const unsigned beamFormedStation)
{
  const std::vector<unsigned> &stationList = itsBeamFormedStations[beamFormedStation];
  const unsigned destStation = stationList[0];
  const unsigned nrStationsInBeam = stationList.size();
  
#if VERBOSE
  std::stringstream logStr;
  logStr << "Beam forming station " << beamFormedStation << ", size is " << nrStationsInBeam << " (";
  for(unsigned statIndex=0; statIndex<nrStationsInBeam; statIndex++) {
    logStr << stationList[statIndex] << " ";
  }
  logStr << ")";
  LOG_ERROR(logStr.str());
#endif

  // Flagging strategy: if an entire station (or more than x % of a station)
  // is flagged away, just drop that entire station and correct for it.
  // If only a fraction is flagged, do a union of all flags, and flag that data away.
  // Also, we have to set the resulting values to zero, the correlator requires this.

  bool validStation[nrStationsInBeam];
  const unsigned upperBound = static_cast<unsigned>(itsNrSamplesPerIntegration * MAX_FLAGGED_PERCENTAGE);
  unsigned nrValidStations = 0;

  for(unsigned i=0; i<nrStationsInBeam; i++) {
    if(filteredData->flags[stationList[i]].count() > upperBound) {
      // many samples have been flagged away, drop entire station
#if VERBOSE
      LOG_ERROR_STR( "dropping station " << stationList[i] << ", " << filteredData->flags[destStation].count() <<
	" samples were flagged away, upper bound = " << upperBound);
#endif
      validStation[i] = false;
    } else {
      // ok, use station
      validStation[i] = true;
      nrValidStations++;
    }
  }

//  LOG_ERROR_STR("total Stations in beam = " << nrStationsInBeam << ", valid = " << nrValidStations);
  const float factor = 1.0 / nrValidStations;

  // Now, we just flag everything that is flagged in one of the stations away.
  // We only do this for valid stations. Station 0 (the destination) is a special case.
  // If it was not valid, we have to clear its flags.
  if(!validStation[0]) {
    filteredData->flags[destStation].reset();
  }
  for(unsigned statIndex = 1; statIndex < nrStationsInBeam; statIndex++) {
    if(validStation[statIndex]) {
      unsigned other = stationList[statIndex];
      filteredData->flags[destStation] |= filteredData->flags[other];
    }
  }

  // Add the data of all stations to the destination station.
  for (unsigned ch = 0; ch < itsNrChannels; ch ++) {
    if (ch > 0 /* && !itsRFIflags[stat1][ch] && !itsRFIflags[stat2][ch] */) {
      for (unsigned time = 0; time < itsNrSamplesPerIntegration; time ++) {
	if(!filteredData->flags[destStation].test(time)) {
	  for (unsigned pol = 0; pol < NR_POLARIZATIONS; pol ++) {
	    fcomplex sample = filteredData->samples[ch][destStation][time][pol];
	    
	    for (unsigned statIndex = 1; statIndex < nrStationsInBeam; statIndex++) {
	      if(validStation[statIndex]) {
		sample += filteredData->samples[ch][stationList[statIndex]][time][pol];
	      }
	    }
	    
	    // We need to correct the total power here: divide by the number of 
	    // stations in this beam formed station.
	    filteredData->samples[ch][destStation][time][pol] = sample * factor;
	  }
	} else {
	  for (unsigned pol = 0; pol < NR_POLARIZATIONS; pol ++) {
	    filteredData->samples[ch][destStation][time][pol] = makefcomplex(0, 0);
	  }
	}
      }
    }
  }

  // There are two ways of adding the data.
  // 1) load sample, iterate over the stations, add values, store sample.
  // 2) do the additions pair-wise. Just add two stations together.
  // The number of operations is the same, but the latter requires many
  // more memory loads. However, option 2 walks linerarly through memory, and
  // may have a better cache behaviour, or may be easier to prefetch.
  // On a PC, option 1 was much faster.
}

void BeamFormer::formBeams(FilteredData *filteredData)
{
#if VERBOSE
  LOG_ERROR("beam forming START");
#endif

  beamFormTimer.start();

  for (unsigned beamFormedStation=0; beamFormedStation<itsNrBeamFormedStations; beamFormedStation++) {
    if(itsBeamFormedStations[beamFormedStation].size() > 1) {
      beamFormStation(filteredData, beamFormedStation);
    }
  }

  beamFormTimer.stop();

#if VERBOSE
  LOG_ERROR("beam forming DONE");
#endif
}


} // namespace RTCP
} // namespace LOFAR
