#ifndef __STATIONSETTINGS__
#define __STATIONSETTINGS__

#include <Common/LofarLogger.h>
#include "StationID.h"
#include <ostream>

namespace LOFAR {
namespace RTCP {

#define NR_RSPBOARDS 4

struct StationSettings {
private:
  static const unsigned currentVersion = 1;

  unsigned version;

  bool valid() const { return version == currentVersion; }

public:
  struct StationID station;

  unsigned nrBeamlets;

  size_t   nrSamples;

  unsigned nrBoards;
  size_t   nrFlagRanges;

  key_t    dataKey;

  StationSettings();

  // read settings from shared memory, using the given stationID
  StationSettings(struct StationID station);

  bool operator==(const struct StationSettings &other) const {
    return station == other.station
        && nrBeamlets == other.nrBeamlets 
        && nrSamples == other.nrSamples
        && nrBoards == other.nrBoards
        && nrFlagRanges == other.nrFlagRanges
        && dataKey == other.dataKey;
  }

};

StationSettings::StationSettings()
:
  version(currentVersion)
{
}

StationSettings::StationSettings(struct StationID station)
:
  version(currentVersion),
  station(station)
{
  do {
    SharedStruct<struct StationSettings> shm(station.hash(), false);

    *this = shm.get();
  } while (!valid());  

  ASSERT( valid() );
}

std::ostream& operator<<( std::ostream &str, const struct StationSettings &s ) {
  str << s.station << " beamlets: " << s.nrBeamlets << " buffer: " << (1.0 * s.nrSamples / s.station.clock * 1024) << "s";

  return str;
}

}
}

#endif

