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

BufferSettings::BufferSettings(struct StationID station)
:
  version(currentVersion),
  station(station)
{
  do {
    SharedStruct<struct BufferSettings> shm(station.hash(), false);

    *this = shm.get();
  } while (!valid());  

  ASSERT( valid() );
}

std::ostream& operator<<( std::ostream &str, const struct BufferSettings &s ) {
  str << s.station << " beamlets: " << s.nrBeamlets << " buffer: " << (1.0 * s.nrSamples / s.station.clock * 1024) << "s";

  return str;
}


}
}
