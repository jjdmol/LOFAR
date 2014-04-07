#ifndef __SAMPLEBUFFER__
#define __SAMPLEBUFFER__

#include <Common/LofarLogger.h>
#include <Interface/MultiDimArray.h>
#include <Interface/Allocator.h>
#include "BufferSettings.h"
#include "SharedMemory.h"
#include "Ranges.h"
#include "SampleType.h"
#include <string>
#include <vector>
#include <boost/format.hpp>

namespace LOFAR {
namespace RTCP {


template<typename T> class SampleBuffer {
public:
  SampleBuffer( const struct BufferSettings &settings, bool create );

private:
  const std::string logPrefix;
  SharedMemoryArena data;
  SparseSetAllocator allocator;

  struct BufferSettings *initSettings( const struct BufferSettings &localSettings, bool create );

  static size_t dataSize( const struct BufferSettings &settings ) {
    return sizeof settings
         + settings.nrBoards * (Ranges::size(settings.nrFlagRanges) + 8)
         + settings.nrBeamlets * (settings.nrSamples * sizeof(T) + 128);
  }

public:
  struct BufferSettings *settings;

  const size_t nrBeamlets;
  const size_t nrSamples;
  const size_t nrBoards;
  const size_t nrFlagRanges; // width of each flag range

  MultiDimArray<T,2>  beamlets; // [subband][sample]
  std::vector<Ranges> flags;    // [rspboard]
};


template<typename T> SampleBuffer<T>::SampleBuffer( const struct BufferSettings &_settings, bool create )
:
  logPrefix(str(boost::format("[station %s %s board] [SampleBuffer] ") % _settings.station.stationName % _settings.station.antennaField)),
  data(_settings.dataKey, dataSize(_settings), create ? SharedMemoryArena::CREATE_EXCL : SharedMemoryArena::READ),
  allocator(data),
  settings(initSettings(_settings, create)),

  nrBeamlets(settings->nrBeamlets),
  nrSamples(settings->nrSamples),
  nrBoards(settings->nrBoards),
  nrFlagRanges(settings->nrFlagRanges),

  beamlets(boost::extents[nrBeamlets][nrSamples], 128, allocator, false, false),
  flags(nrBoards)
{
  for (size_t f = 0; f < flags.size(); f++) {
    size_t numBytes = Ranges::size(nrFlagRanges);

    flags[f] = Ranges(static_cast<int64*>(allocator.allocate(numBytes, 8)), numBytes, nrSamples, create);
  }

  LOG_INFO_STR( logPrefix << "Initialised" );
}

template<typename T> struct BufferSettings *SampleBuffer<T>::initSettings( const struct BufferSettings &localSettings, bool create )
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

