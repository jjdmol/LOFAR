/* PacketWriter.cc
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

#include <lofar_config.h>

#include <ctime>
#include <string>
#include <vector>

#include <Common/LofarTypes.h>
#include <Common/LofarLogger.h>
#include <Stream/FileStream.h>
#include <CoInterface/SparseSet.h>

#include <SampleType.h>
#include <Buffer/StationID.h>
#include <Buffer/SampleBuffer.h>
#include <Buffer/BufferSettings.h>
#include <Station/PacketReader.h>
#include <Station/PacketWriter.h>
#include <Station/RSP.h>

using namespace LOFAR;
using namespace Cobalt;
using namespace std;

template<typename T>
void test( struct BufferSettings &settings, const std::string &filename )
{
  SampleBuffer< SampleType<T> > buffer(settings, true);
  PacketWriter< SampleType<T> > writer("", buffer, 0);

  // Read packets from file
  FileStream fs(filename);
  PacketReader reader("", fs);

  // There are two packets, transfer them
  struct RSP packet;

  size_t nrValidSamples = 0;

  for (size_t i = 0; i < 2; ++i) {
    // Read the packet
    ASSERT( reader.readPacket(packet) );

    nrValidSamples += packet.header.nrBlocks;

    // Write the packet
    writer.writePacket(packet);

    // Check whether the packet is in the buffer

    // Check the flags
    SparseSet<int64> available = buffer.boards[0].flags.sparseSet((int64)packet.timeStamp(), (int64)packet.timeStamp() + packet.header.nrBlocks);
    ASSERT(available.count() == packet.header.nrBlocks);

    // Check the data
    for (size_t beamlet = 0; beamlet < packet.header.nrBeamlets; ++beamlet) {
      for (size_t sample = 0; sample < packet.header.nrBlocks; ++sample) {
        // Obtain the packet's sample
        complex<int> x = packet.sample(beamlet, sample, 'X');
        complex<int> y = packet.sample(beamlet, sample, 'Y');

        // Obtain the buffer's sample
        // Note: we're the 0th board, so beamlet indices are also absolute
        int64 timestamp = packet.timeStamp() + sample;
        SampleType<T> buf = buffer.beamlets[beamlet][timestamp % settings.nrSamples];

        // Compare them
        ASSERT( x.real() == buf.x.real() );
        ASSERT( x.imag() == buf.x.imag() );
        ASSERT( y.real() == buf.y.real() );
        ASSERT( y.imag() == buf.y.imag() );
      }
    }
  }

  // There should be only nrValidSamples samples in the buffer, nothing more
  int64 now = (int64)TimeStamp(time(0) + 1, 0, settings.station.clockMHz * 1000000);
  SparseSet<int64> available = buffer.boards[0].flags.sparseSet(0, now);
  ASSERT((size_t)available.count() == nrValidSamples);
}


int main()
{
  INIT_LOGGER( "tPacketWriter" );

  // Don't run forever if communication fails for some reason
  alarm(10);

  // Fill a BufferSettings object
  struct StationID stationID("RS106", "LBA", 200, 16);
  struct BufferSettings settings(stationID, false);

  // Use a fixed key, so the test suite knows what to clean
  settings.dataKey = 0x12345678;

  // Test various modes
  LOG_INFO("Test 16-bit complex");
  settings.station.bitMode = 16;
  settings.nrBeamletsPerBoard = 61;
  test<i16complex>(settings, "tPacketWriter.in_16bit");

  LOG_INFO("Test 8-bit complex");
  settings.station.bitMode = 8;
  settings.nrBeamletsPerBoard = 122;
  test<i8complex>(settings, "tPacketWriter.in_8bit");

  return 0;
}

