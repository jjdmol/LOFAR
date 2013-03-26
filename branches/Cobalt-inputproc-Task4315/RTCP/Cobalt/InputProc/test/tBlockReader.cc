/* PacketsToBuffer.cc
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

#include <Common/LofarTypes.h>
#include <Common/LofarLogger.h>
#include <Stream/FileStream.h>

#include <SampleType.h>
#include <Station/PacketsToBuffer.h>
#include <Buffer/BlockReader.h>
#include <Buffer/SampleBuffer.h>

#include <UnitTest++.h>

using namespace LOFAR;
using namespace Cobalt;
using namespace std;

// A BufferSettings object to be used for all tests
struct StationID stationID("RS106", "LBA", 200, 16);
struct BufferSettings settings(stationID, false);

TEST(Basic) {
  for (size_t nrBeamlets = 1; nrBeamlets < settings.nrBoards * settings.nrBeamletsPerBoard; nrBeamlets <<= 1) {
    for (size_t blockSize = 1; blockSize < settings.nrSamples; blockSize <<= 1) {
      // Create a buffer
      SampleBuffer< SampleType<i16complex> > buffer(settings, true);

      // Read the beamlets
      std::vector<size_t> beamlets(nrBeamlets);
      for (size_t b = 0; b < beamlets.size(); ++b) {
        beamlets[b] = nrBeamlets - b;
      }

      BlockReader< SampleType<i16complex> > reader(settings, beamlets);

      // Read a few blocks -- from the distant past to prevent unnecessary
      // waiting.
      const TimeStamp from(0, 0, settings.station.clockMHz * 1000000);
      const TimeStamp to(from + 10 * blockSize);
      for (TimeStamp current = from; current + blockSize < to; current += blockSize) {
        SmartPtr<struct BlockReader< SampleType<i16complex> >::Block> block(reader.block(current, current + blockSize, std::vector<ssize_t>(nrBeamlets, 0)));

        // Validate the block
        ASSERT(block->beamlets.size() == beamlets.size());

        for (size_t b = 0; b < beamlets.size(); ++b) {
          struct BlockReader< SampleType<i16complex> >::Block::Beamlet &ib = block->beamlets[b];

          // Beamlets should be provided in the same order
          CHECK_EQUAL(beamlets[b], ib.stationBeamlet);

          switch (ib.nrRanges) {
            case 1:
              CHECK(ib.ranges[0].from < ib.ranges[0].to);

              CHECK_EQUAL((ptrdiff_t)blockSize, ib.ranges[0].to - ib.ranges[0].from);
              break;

            case 2:
              CHECK(ib.ranges[0].from < ib.ranges[0].to);
              CHECK(ib.ranges[1].from < ib.ranges[1].to);

              CHECK_EQUAL((ptrdiff_t)blockSize, (ib.ranges[0].to - ib.ranges[0].from) + (ib.ranges[1].to - ib.ranges[1].from));
              break;

            default:
              ASSERTSTR(false, "nrRanges must be 1 or 2");
              break;
          }
          
          // No samples should be available
          CHECK_EQUAL((int64)blockSize, ib.flagsAtBegin.count());
        }
      }
    }
  }
}

template<typename T>
void test( struct BufferSettings &settings, const std::string &filename )
{
  // Create the buffer to keep it around after transfer.process(), or there
  // will be no subscribers and transfer will delete the buffer automatically,
  // at which point we can't attach anymore.
  SampleBuffer< SampleType<T> > buffer(settings, true);

  // Read packets from file
  FileStream fs(filename);

  // Set up transfer
  PacketsToBuffer transfer(fs, settings, 0);

  // Do transfer
  transfer.process();

  // Determine the timestamps of the packets we've just written
  int64 now = (int64)TimeStamp(time(0) + 1, 0, settings.station.clockMHz * 1000000);
  SparseSet<int64> available = buffer.boards[0].available.sparseSet(0, now);

  const TimeStamp from(available.getRanges()[0].begin, settings.station.clockMHz * 1000000);

  // Read some of the beamlets
  std::vector<size_t> beamlets(2);
  for (size_t b = 0; b < beamlets.size(); ++b)
    beamlets[b] = b;

  BlockReader< SampleType<T> > reader(settings, beamlets);

  // Read the block, plus 16 unavailable samples
  SmartPtr<struct BlockReader< SampleType<T> >::Block> block(reader.block(from, from + available.count() + 16, std::vector<ssize_t>(beamlets.size(),0)));

  // Validate the block
  for (size_t b = 0; b < beamlets.size(); ++b) {
    // We should only detect the 16 unavailable samples
    ASSERT(block->beamlets[b].flagsAtBegin.count() == 16);
  }
}


int main()
{
  INIT_LOGGER( "tBlockReader" );

  // Don't run forever if communication fails for some reason
  alarm(10);

  // Use a fixed key, so the test suite knows what to clean
  settings.dataKey = 0x12345678;

  // Limit the array in size to work on systems with only 32MB SHM
  settings.nrBoards = 1;
  settings.setBufferSize(0.1);

  // Test various modes
  LOG_INFO("Test 16-bit complex");
  settings.station.bitMode = 16;
  settings.nrBeamletsPerBoard = 61;
  test<i16complex>(settings, "tBlockReader.in_16bit");

  LOG_INFO("Test 8-bit complex");
  settings.station.bitMode = 8;
  settings.nrBeamletsPerBoard = 122;
  test<i8complex>(settings, "tBlockReader.in_8bit");

  return UnitTest::RunAllTests();
}

