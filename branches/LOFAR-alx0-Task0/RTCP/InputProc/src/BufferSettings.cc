#include <lofar_config.h>
#include "BufferSettings.h"
#include "SharedMemory.h"
#include <Common/LofarLogger.h>

namespace LOFAR {
namespace RTCP {


BufferSettings::BufferSettings()
:
  version(currentVersion)
{
}

BufferSettings::BufferSettings(const struct StationID &station, bool attach)
:
  version(currentVersion),
  station(station)
{
  if (attach) {
    do {
      SharedStruct<struct BufferSettings> shm(station.hash(), false);

      *this = shm.get();
    } while (!valid());  

    ASSERT( valid() );
  } else {
    deriveDefaultSettings();
  }
}


void BufferSettings::deriveDefaultSettings()
{
  switch (station.bitMode) {
    default:
    case 16:
      nrBeamletsPerBoard = 61;
      break;

    case 8:
      nrBeamletsPerBoard = 122;
      break;

    case 4:
      nrBeamletsPerBoard = 244;
      break;
  }

  // 1 second buffer
  setBufferSize(1.0);

  nrBoards = 4;
  nrFlagRanges = 64;

  dataKey = station.hash();
}


void BufferSettings::setBufferSize(double seconds)
{
  // Make sure nrSamples is a multiple of 16, which
  // is the expected number of samples in a block.
  //
  // Doing so allows the writer to prevent split
  // writes of packets. (TODO: That's not implemented,
  // because the timestamps of the packets are not
  // necessarily a multiple of 16).
  nrSamples = static_cast<size_t>(seconds * (station.clockMHz * 1000000) / 1024) & ~0xFLL;
}


std::ostream& operator<<( std::ostream &str, const struct BufferSettings &s ) {
  str << s.station << " beamlets: " << (s.nrBoards * s.nrBeamletsPerBoard) << " buffer: " << (1.0 * s.nrSamples / s.station.clockMHz / 1000000 * 1024) << "s";

  return str;
}


}
}
