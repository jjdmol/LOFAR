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

  // if attach=true, read settings from shared memory, using the given stationID
  // if attach=false, set sane default values
  BufferSettings(const struct StationID &station, bool attach);

  // Shortcut to set nrSamples to represent `seconds' of buffer.
  void setBufferSize(double seconds);

  size_t flagIdx(unsigned beamlet) const { return beamlet / nrBeamletsPerBoard; }

  bool operator==(const struct BufferSettings &other) const {
    return station == other.station
        && nrBeamletsPerBoard == other.nrBeamletsPerBoard
        && nrSamples == other.nrSamples
        && nrBoards == other.nrBoards
        && nrFlagRanges == other.nrFlagRanges
        && dataKey == other.dataKey;
  }
private:

  // Derive sane values from the station field.
  void deriveDefaultSettings();
};

std::ostream& operator<<( std::ostream &str, const struct BufferSettings &s );

}
}

#endif

