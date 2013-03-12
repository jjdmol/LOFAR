#ifndef __SAMPLEBUFFER__
#define __SAMPLEBUFFER__

#include <Common/LofarLogger.h>
#include <CoInterface/MultiDimArray.h>
#include <CoInterface/Allocator.h>
#include "Buffer/BufferSettings.h"
#include "Buffer/SharedMemory.h"
#include "Buffer/Ranges.h"
#include "SampleType.h"
#include <string>
#include <vector>
#include <boost/format.hpp>

namespace LOFAR
{
  namespace RTCP
  {


    /*
     * Maintains a sample buffer in shared memory, which can be created
     * or attached to.
     *
     * The sample buffer contains the following information:
     *
     *   1. A copy of `settings', against which attaches are verified.
     *   2. A beamlets matrix [subband][sample]
     *   3. A flags vector [board]
     *
     * The IPC key used for the shared memory is settings.dataKey.
     */
    template<typename T>
    class SampleBuffer
    {
    public:
      // Create (create=true) or attach to (create=false) a sample buffer
      // in shared memory.
      SampleBuffer( const struct BufferSettings &settings, bool create );

    private:
      const std::string logPrefix;
      SharedMemoryArena data;
      SparseSetAllocator allocator;

      struct BufferSettings *initSettings( const struct BufferSettings &localSettings, bool create );

      static size_t dataSize( const struct BufferSettings &settings )
      {
        return sizeof settings
               + settings.nrBoards * (Ranges::size(settings.nrFlagRanges) + 8)
               + settings.nrBoards * settings.nrBeamletsPerBoard * (settings.nrSamples * sizeof(T) + 128);
      }

    public:
      struct BufferSettings *settings;

      const size_t nrBeamletsPerBoard;
      const size_t nrSamples;
      const size_t nrBoards;
      const size_t nrFlagRanges; // width of each flag range

      MultiDimArray<T,2>  beamlets; // [subband][sample]
      std::vector<Ranges> flags; // [board]
    };


    template<typename T>
    SampleBuffer<T>::SampleBuffer( const struct BufferSettings &_settings, bool create )
      :
      logPrefix(str(boost::format("[station %s %s board] [SampleBuffer] ") % _settings.station.stationName % _settings.station.antennaField)),
      data(_settings.dataKey, dataSize(_settings), create ? SharedMemoryArena::CREATE : SharedMemoryArena::READ),
      allocator(data),
      settings(initSettings(_settings, create)),

      nrBeamletsPerBoard(settings->nrBeamletsPerBoard),
      nrSamples(settings->nrSamples),
      nrBoards(settings->nrBoards),
      nrFlagRanges(settings->nrFlagRanges),

      beamlets(boost::extents[nrBoards * nrBeamletsPerBoard][nrSamples], 128, allocator, false, false),
      flags(nrBoards)
    {
      for (size_t f = 0; f < flags.size(); f++) {
        size_t numBytes = Ranges::size(nrFlagRanges);

        flags[f] = Ranges(static_cast<int64*>(allocator.allocate(numBytes, 8)), numBytes, nrSamples, create);
      }

      LOG_INFO_STR( logPrefix << "Initialised" );
    }

    template<typename T>
    struct BufferSettings *SampleBuffer<T>::initSettings( const struct BufferSettings &localSettings, bool create )
    {
      struct BufferSettings *sharedSettings = allocator.allocateTyped();

      if (create) {
        // register settings
        LOG_INFO_STR( logPrefix << "Registering " << localSettings.station );
        *sharedSettings = localSettings;
      } else {
        // verify settings
        ASSERT( *sharedSettings == localSettings );
        LOG_INFO_STR( logPrefix << "Connected to " << localSettings.station );
      }

      return sharedSettings;
    }

  }
}

#endif

