#ifndef __BUFFERSETTINGS__
#define __BUFFERSETTINGS__

#include <Common/LofarLogger.h>
#include "SharedMemory.h"
#include "StationID.h"
#include <ostream>

namespace LOFAR {
namespace RTCP {

#define NR_RSPBOARDS 4

struct BufferSettings {
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

  BufferSettings();

  // read settings from shared memory, using the given stationID
  BufferSettings(struct StationID station);

  bool operator==(const struct BufferSettings &other) const {
    return station == other.station
        && nrBeamlets == other.nrBeamlets 
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

