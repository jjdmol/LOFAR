#include "SampleBuffer.h"

#include <string>
#include <vector>
#include <boost/format.hpp>

#include <Common/LofarLogger.h>
#include "BufferSettings.h"
#include "SharedMemory.h"
#include "Ranges.h"

namespace LOFAR
{
  namespace Cobalt
  {
    template <typename T>
    size_t SampleBuffer<T>::dataSize( const struct BufferSettings &settings )
    {
      return // header
             sizeof settings
             // flags (aligned to ALIGNMENT)
             + settings.nrBoards * (Ranges::size(settings.nrAvailableRanges) + ALIGNMENT)
             // beamlets (aligned to ALIGNMENT)
             + settings.nrBoards * (BoardMode::nrBeamletsPerBoard(T::bitMode()) * settings.nrSamples(T::bitMode()) * sizeof(T) + ALIGNMENT)
             // mode (aligned to ALIGNMENT)
             + settings.nrBoards * (sizeof(struct BoardMode) + ALIGNMENT);
    }


    template<typename T>
    SampleBuffer<T>::SampleBuffer( const struct BufferSettings &_settings, SharedMemoryArena::Mode shmMode )
      :
      logPrefix(str(boost::format("[station %s %s board] [SampleBuffer] ") % _settings.station.stationName % _settings.station.antennaField)),
      data(_settings.dataKey, dataSize(_settings), shmMode),
      allocator(data),
      create(shmMode == SharedMemoryArena::CREATE || shmMode == SharedMemoryArena::CREATE_EXCL),
      settings(initSettings(_settings, create)),
      sync(settings->sync),
      syncLock(settings->syncLock),

      nrSamples(settings->nrSamples(T::bitMode())),
      nrBoards(settings->nrBoards),
      nrAvailableRanges(settings->nrAvailableRanges),

      beamlets(boost::extents[nrBoards * BoardMode::nrBeamletsPerBoard(T::bitMode())][nrSamples], ALIGNMENT, allocator, true, false),
      boards(nrBoards,Board(*this))
    {
      // Check if non-realtime mode is set up correctly
      if (sync) {
        ASSERTSTR(syncLock, "Synced buffer requires syncLock object");
        ASSERTSTR(syncLock->size() == nrBoards, "SHM buffer has " << nrBoards << " RSP boards, but syncLock expects " << syncLock->size() << " boards");
      }

      for (size_t b = 0; b < boards.size(); b++) {
        size_t numBytes = Ranges::size(nrAvailableRanges);

        boards[b].available = Ranges(static_cast<int64*>(allocator.allocate(numBytes, ALIGNMENT)), numBytes, nrSamples, create);
        boards[b].boardNr = b;
        boards[b].mode = allocator.allocateTyped(ALIGNMENT);
      }

      LOG_DEBUG_STR( logPrefix << "Initialised" );
    }
    

    template<typename T>
    struct BufferSettings *SampleBuffer<T>::initSettings( const struct BufferSettings &localSettings, bool create )
    {
      struct BufferSettings *sharedSettings = allocator.allocateTyped(ALIGNMENT);

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
    SampleBuffer<T>::Board::Board( SampleBuffer<T> &buffer, size_t boardNr )
      :
      mode(0),
      boardNr(boardNr),
      buffer(buffer)
    {
    }


    template<typename T>
    void SampleBuffer<T>::Board::changeMode( const struct BoardMode &mode )
    {
      // only act if something changes
      if (mode == *this->mode)
        return;

      // invalidate all data
      available.clear();

      // set the new mode
      *this->mode = mode;
    }


    template<typename T>
    void SampleBuffer<T>::Board::noReadBefore( const TimeStamp &epoch )
    {
      if (buffer.sync) {
        // Free up read intent up until `epoch'.
        (*buffer.syncLock)[boardNr].readPtr.advanceTo(epoch);
      }
    }


    template<typename T>
    void SampleBuffer<T>::Board::startRead( const TimeStamp &begin, const TimeStamp &end )
    {
      // Free up read intent up until `begin'.
      noReadBefore(begin);

      if (buffer.sync) {
        // Wait for writer to finish writing until `end'.
        (*buffer.syncLock)[boardNr].writePtr.waitFor(end);
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
      LOG_DEBUG_STR("[board " << boardNr << "] noMoreReading()");

      // Signal we're done reading

      // Put the readPtr into the far future.
      // We only use this TimeStamp for comparison so clockSpeed does not matter.
      noReadBefore(TimeStamp(0xFFFFFFFFFFFFFFFFULL));
    }


    template<typename T>
    void SampleBuffer<T>::Board::startWrite( const TimeStamp &begin, const TimeStamp &end )
    {
      ASSERT((uint64)end > buffer.nrSamples);

      if (buffer.sync) {
        // Signal write intent, to let reader know we don't have data older than
        // this.
        (*buffer.syncLock)[boardNr].writePtr.advanceTo(begin);

        // Wait for reader to finish what we're about to overwrite
        (*buffer.syncLock)[boardNr].readPtr.waitFor(end - buffer.nrSamples);
      }

      // Mark overwritten range (and everything before it to prevent a mix) as invalid
      available.excludeBefore(end - buffer.nrSamples);
    }


    template<typename T>
    void SampleBuffer<T>::Board::stopWrite( const TimeStamp &end )
    {
      if (buffer.sync) {
        // Signal we're done writing
        (*buffer.syncLock)[boardNr].writePtr.advanceTo(end);
      }
    }


    template<typename T>
    void SampleBuffer<T>::Board::noMoreWriting()
    {
      LOG_DEBUG_STR("[board " << boardNr << "] noMoreWriting()");

      if (buffer.sync) {
        // Signal we're done writing

        // Put the writePtr into the far future.
        // We only use this TimeStamp for comparison so clockSpeed does not matter.
        (*buffer.syncLock)[boardNr].writePtr.advanceTo(TimeStamp(0xFFFFFFFFFFFFFFFFULL));
      }
    }
  }
}

