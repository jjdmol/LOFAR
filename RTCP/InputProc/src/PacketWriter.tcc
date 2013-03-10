#include <Common/LofarLogger.h>
#include <Common/LofarConstants.h>
#include <Interface/RSPTimeStamp.h>
#include <Interface/SmartPtr.h>
#include "RSP.h"
#include "SampleBuffer.h"
#include "BufferSettings.h"
#include "Ranges.h"
#include <boost/format.hpp>
#include <string>
#include <ios>

namespace LOFAR {
namespace RTCP {


template<typename T> PacketWriter<T>::PacketWriter( const std::string &logPrefix, SampleBuffer<T> &buffer, const struct BufferSettings &settings, unsigned boardNr )
:
  logPrefix(str(boost::format("%s [PacketWriter] ") % logPrefix)),

  buffer(buffer),
  flags(buffer.flags[boardNr]),
  settings(settings),
  firstBeamlet(boardNr * settings.nrBeamletsPerBoard),

  nrWritten(0)
{
  // bitmode must coincide with our template
  ASSERT( sizeof(T) == N_POL * 2 * settings.station.bitMode / 8 );

  // we must be able to fit our packets
  ASSERT( firstBeamlet + settings.nrBeamletsPerBoard < settings.nrBoards * settings.nrBeamletsPerBoard );
}


template<typename T> void PacketWriter<T>::writePacket( const struct RSP &packet )
{
  const uint8 &nrBeamlets  = packet.header.nrBeamlets;
  const uint8 &nrTimeslots = packet.header.nrBlocks;

  // should not exceed the number of beamlets we expect
  ASSERT( nrBeamlets <= settings.nrBeamletsPerBoard );

  const TimeStamp timestamp(packet.header.timestamp, packet.header.blockSequenceNumber, settings.station.clockMHz * 1000000);

  // determine the time span when cast on the buffer
  const size_t from_offset = (int64)timestamp % settings.nrSamples;
  size_t to_offset = ((int64)timestamp + nrTimeslots) % settings.nrSamples;

  if (to_offset == 0)
    to_offset = settings.nrSamples;

  const size_t wrap = from_offset < to_offset ? 0 : settings.nrSamples - from_offset;

  /*
   * Make sure the buffer and flags are always consistent.
   */

  // mark data we overwrite as invalid
  flags.excludeBefore(timestamp + nrTimeslots - settings.nrSamples);

  // transpose
  const T *beamlets = reinterpret_cast<const T*>(&packet.payload.data);

  for (uint8 b = 0; b < nrBeamlets; ++b) {
    T *dst1 = &buffer.beamlets[firstBeamlet + b][from_offset];

    if (wrap > 0) {
      T *dst2 = &buffer.beamlets[firstBeamlet + b][0];

      memcpy(dst1, beamlets, wrap        * sizeof(T));
      memcpy(dst2, beamlets, to_offset   * sizeof(T));
    } else {
      memcpy(dst1, beamlets, nrTimeslots * sizeof(T));
    }

    beamlets += nrTimeslots;
  }

  // mark as valid
  flags.include(timestamp, timestamp + nrTimeslots);

  ++nrWritten;
}


template<typename T> void PacketWriter<T>::logStatistics()
{
  LOG_INFO_STR( logPrefix << "Written " << nrWritten << " packets");

  nrWritten = 0;
}


}
}

