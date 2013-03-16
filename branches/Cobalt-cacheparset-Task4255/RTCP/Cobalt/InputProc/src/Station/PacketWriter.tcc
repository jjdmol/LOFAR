/* PacketWriter.tcc
 * Copyright (C) 2013  ASTRON (Netherlands Institute for Radio Astronomy)
 * P.O. Box 2, 7990 AA Dwingeloo, The Netherlands
 *
 * This file is part of the LOFAR software suite.
 * The LOFAR software suite is free software: you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * The LOFAR software suite is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with the LOFAR software suite. If not, see <http://www.gnu.org/licenses/>.
 *
 * $Id: $
 */

#include <ios>
#include <boost/format.hpp>

#include <Common/LofarTypes.h>
#include <Common/LofarConstants.h>
#include <Common/LofarLogger.h>
#include <CoInterface/RSPTimeStamp.h>

namespace LOFAR {
namespace Cobalt {


template<typename T> PacketWriter<T>::PacketWriter( const std::string &logPrefix, SampleBuffer<T> &buffer, unsigned boardNr )
:
  logPrefix(str(boost::format("%s [PacketWriter] ") % logPrefix)),

  buffer(buffer),
  flags(buffer.flags[boardNr]),
  settings(*buffer.settings),
  firstBeamlet(boardNr * settings.nrBeamletsPerBoard),

  nrWritten(0)
{
  // bitmode must coincide with our template
  ASSERTSTR( sizeof(T) == N_POL * 2 * settings.station.bitMode / 8, "PacketWriter created with sample size " << sizeof(T) << " does not match station settings, which are in " << settings.station.bitMode << " bit mode" );

  // we must be able to fit our packets
  ASSERT( firstBeamlet + settings.nrBeamletsPerBoard <= settings.nrBoards * settings.nrBeamletsPerBoard );
}


template<typename T> void PacketWriter<T>::writePacket( const struct RSP &packet )
{
  const uint8 &nrBeamlets  = packet.header.nrBeamlets;
  const uint8 &nrTimeslots = packet.header.nrBlocks;

  // should not exceed the number of beamlets we expect
  ASSERT( nrBeamlets <= settings.nrBeamletsPerBoard );

  const TimeStamp timestamp = packet.timeStamp();

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

