#include "SampleBuffer.h"

#include "BufferSettings.h"
#include "SharedMemory.h"

namespace LOFAR
{
  namespace Cobalt
  {
    void removeSampleBuffers( const BufferSettings &settings )
    {
      // Remove the provided dataKey, as it could be a custom setting
      SharedMemoryArena::remove(settings.dataKey);

      // Remove the keys of all possible configurations
      StationID station = settings.station;

      const unsigned bitmodes[] = { 4, 8, 16 };
      const unsigned clocks[]   = { 160, 200 };

      for (size_t b = 0; b < sizeof bitmodes / sizeof bitmodes[0]; ++b) {
        for (size_t c = 0; c < sizeof clocks / sizeof clocks[0]; ++c) {
          station.bitMode  = bitmodes[b];
          station.clockMHz = clocks[c];
          
          // Remove any lingering buffer for this mode
          SharedMemoryArena::remove(station.hash());
        }
      }
    }
  }
}

