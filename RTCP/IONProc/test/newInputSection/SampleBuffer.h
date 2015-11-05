#ifndef __SAMPLEBUFFER__
#define __SAMPLEBUFFER__

#include <Common/LofarLogger.h>
#include <Common/LofarConstants.h>
#include <Interface/MultiDimArray.h>
#include <Interface/Allocator.h>
#include "StationSettings.h"
#include "SharedMemory.h"
#include "Ranges.h"
#include <string>
#include <complex>

namespace LOFAR {
namespace RTCP {

template<typename T> class SampleBuffer {
public:
  SampleBuffer( const struct StationSettings &settings, bool create );

  struct SampleType {
    std::complex<T> x;
    std::complex<T> y;
  };

private:
  const std::string logPrefix;
  SharedMemoryArena data;
  SparseSetAllocator allocator;

  struct StationSettings *initSettings( const struct StationSettings &localSettings, bool create );

  static size_t dataSize( const struct StationSettings &settings ) {
    return sizeof settings
         + NR_RSPBOARDS * (Ranges::size(settings.nrFlagRanges) + 8)
         + settings.nrBeamlets * (settings.nrSamples * sizeof(T) + 128);
  }

public:
  struct StationSettings *settings;

  const size_t nrBeamlets;
  const size_t nrSamples;
  const size_t nrFlagRanges;

  MultiDimArray<T,2>  beamlets; // [subband][sample]
  std::vector<Ranges> flags;    // [rspboard]
};


template<typename T> SampleBuffer<T>::SampleBuffer( const struct StationSettings &_settings, bool create )
:
  logPrefix(str(boost::format("[station %s %s board] [SampleBuffer] ") % _settings.station.stationName % _settings.station.antennaSet)),
  data(_settings.dataKey, dataSize(_settings), create ? SharedMemoryArena::CREATE_EXCL : SharedMemoryArena::READ),
  allocator(data),
  settings(initSettings(_settings, create)),

  nrBeamlets(settings->nrBeamlets),
  nrSamples(settings->nrSamples),
  nrFlagRanges(settings->nrFlagRanges),

  beamlets(boost::extents[nrBeamlets][nrSamples], 128, allocator, false, false),
  flags(settings->nrBoards)
{
  // bitmode must coincide with our template
  ASSERT( sizeof(T) == N_POL * 2 * settings->station.bitmode / 8 );

  for (size_t f = 0; f < flags.size(); f++) {
    size_t numBytes = Ranges::size(nrFlagRanges);

    flags[f] = Ranges(static_cast<int64*>(allocator.allocate(numBytes, 8)), numBytes, nrSamples, create);
  }

  LOG_INFO_STR( logPrefix << "Initialised" );
}

template<typename T> struct StationSettings *SampleBuffer<T>::initSettings( const struct StationSettings &localSettings, bool create )
{
  //struct StationSettings *sharedSettings = allocator.allocateTyped<struct StationSettings>();
  struct StationSettings *sharedSettings = allocator.allocateTyped();

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

