#ifndef __BUFFERSETTINGS__
#define __BUFFERSETTINGS__

#include <Common/LofarLogger.h>
#include "SharedMemory.h"
#include "StationID.h"
#include <ostream>

namespace LOFAR {
namespace RTCP {

struct BufferSettings {
private:
  static const unsigned currentVersion = 1;

  unsigned version;

  bool valid() const { return version == currentVersion; }

public:
  struct StationID station;

  unsigned nrBeamletsPerBoard;

  size_t   nrSamples;

  unsigned nrBoards;
  size_t   nrFlagRanges;

  key_t    dataKey;

  BufferSettings();

  // read settings from shared memory, using the given stationID
  BufferSettings(struct StationID station);

  size_t flagIdx(unsigned beamlet) const { return beamlet / nrBeamletsPerBoard; }

  bool operator==(const struct BufferSettings &other) const {
    return station == other.station
        && nrBeamletsPerBoard == other.nrBeamletsPerBoard
        && nrSamples == other.nrSamples
        && nrBoards == other.nrBoards
        && nrFlagRanges == other.nrFlagRanges
        && dataKey == other.dataKey;
  }

};

std::ostream& operator<<( std::ostream &str, const struct BufferSettings &s );

}
}

#endif

