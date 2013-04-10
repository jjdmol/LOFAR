#include "SampleBuffer.h"

#include <string>
#include <vector>
#include <boost/format.hpp>

#include <Common/LofarLogger.h>
#include "BufferSettings.h"
#include "SharedMemory.h"
#include "SlidingPointer.h"
#include "Ranges.h"

namespace LOFAR
{
  namespace Cobalt
  {
    template <typename T>
    size_t SampleBuffer<T>::dataSize( const struct BufferSettings &settings )
    {
      return sizeof settings
             + settings.nrBoards * (Ranges::size(settings.nrAvailableRanges) + 8)
             + settings.nrBoards * settings.nrBeamletsPerBoard * (settings.nrSamples * sizeof(T) + 128);
    }


    template<typename T>
    SampleBuffer<T>::SampleBuffer( const struct BufferSettings &_settings, bool create )
      :
      logPrefix(str(boost::format("[station %s %s board] [SampleBuffer] ") % _settings.station.stationName % _settings.station.antennaField)),
      data(_settings.dataKey, dataSize(_settings), create ? SharedMemoryArena::CREATE : SharedMemoryArena::READ),
      allocator(data),
      settings(initSettings(_settings, create)),
      sync(settings->sync),

      nrBeamletsPerBoard(settings->nrBeamletsPerBoard),
      nrSamples(settings->nrSamples),
      nrBoards(settings->nrBoards),
      nrAvailableRanges(settings->nrAvailableRanges),

      beamlets(boost::extents[nrBoards * nrBeamletsPerBoard][nrSamples], 128, allocator, false, false),
      boards(nrBoards, Board(*this))
    {
      for (size_t b = 0; b < boards.size(); b++) {
        size_t numBytes = Ranges::size(nrAvailableRanges);

        boards[b].available = Ranges(static_cast<int64*>(allocator.allocate(numBytes, 8)), numBytes, nrSamples, create);
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


    template<typename T>
    SampleBuffer<T>::Board::Board( SampleBuffer<T> &buffer )
      :
      buffer(buffer)
    {
    }


    template<typename T>
    void SampleBuffer<T>::Board::noReadBefore( const TimeStamp &epoch )
    {
      if (buffer.sync) {
        // Free up read intent up until `epoch'.
        readPtr.advanceTo(epoch);
      }
    }


    template<typename T>
    void SampleBuffer<T>::Board::startRead( const TimeStamp &begin, const TimeStamp &end )
    {
      // Free up read intent up until `begin'.
      noReadBefore(begin);

      if (buffer.sync) {
        // Wait for writer to finish writing until `end'.
        writePtr.waitFor(end);
      }
    }


    template<typename T>
    void SampleBuffer<T>::Board::stopRead( const TimeStamp &end )
    {
      // Signal we're done reading
      noReadBefore(end);
    }


    template<typename T>
    void SampleBuffer<T>::Board::noMoreReading()
    {
      // Signal we're done reading

      // Put the readPtr into the far future.
      // We only use this TimeStamp for comparison so clockSpeed does not matter.
      noReadBefore(TimeStamp(0xFFFFFFFFFFFFFFFFULL));
    }


    template<typename T>
    void SampleBuffer<T>::Board::startWrite( const TimeStamp &begin, const TimeStamp &end )
    {
      ASSERT((uint64)end > buffer.settings->nrSamples);

      if (buffer.sync) {
        // Signal write intent, to let reader know we don't have data older than
        // this.
        writePtr.advanceTo(begin);

        // Wait for reader to finish what we're about to overwrite
        readPtr.waitFor(end - buffer.settings->nrSamples);
      }

      // Mark overwritten range (and everything before it to prevent a mix) as invalid
      available.excludeBefore(end - buffer.settings->nrSamples);
    }


    template<typename T>
    void SampleBuffer<T>::Board::stopWrite( const TimeStamp &end )
    {
      if (buffer.sync) {
        // Signal we're done writing
        writePtr.advanceTo(end);
      }
    }


    template<typename T>
    void SampleBuffer<T>::Board::noMoreWriting()
    {
      if (buffer.sync) {
        // Signal we're done writing

        // Put the writePtr into the far future.
        // We only use this TimeStamp for comparison so clockSpeed does not matter.
        writePtr.advanceTo(TimeStamp(0xFFFFFFFFFFFFFFFFULL));
      }
    }
  }
}

