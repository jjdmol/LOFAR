/* tSampleBufferSync.cc: Test SampleBuffer operations in non-real time mode.
 * Copyright (C) 2012-2013  ASTRON (Netherlands Institute for Radio Astronomy)
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

#include <unistd.h>

#include <Common/LofarTypes.h>
#include <Common/LofarLogger.h>
#include <CoInterface/SmartPtr.h>

#include <InputProc/SampleType.h>
#include <InputProc/Buffer/StationID.h>
#include <InputProc/Buffer/SampleBuffer.h>
#include <InputProc/Buffer/BufferSettings.h>

#include <UnitTest++.h>
#include <unistd.h>

using namespace LOFAR;
using namespace Cobalt;
using namespace std;

SmartPtr< SampleBuffer< SampleType<i16complex> > > buffer;
SampleBuffer< SampleType<i16complex> >::Board *board;

// Offset to prevent diving into negative timestamps
#define EPOCH 1000000

/*
 * (Re)initialise the global board variable.
 */
void initBoard()
{
  // Fill a BufferSettings object
  struct StationID stationID("RS106", "LBA", 200, 16);
  struct BufferSettings settings(stationID, false);

  // Use a fixed key, so the test suite knows what to clean
  settings.dataKey = 0x10000005;

  // Limit the array in size to work on systems with only 32MB SHM
  settings.nrBoards = 1;
  settings.setBufferSize(0.1);

  // Test non-real-time syncing!
  settings.sync = true;

  // Create a lock set for syncing
  SyncLock syncLock(settings);
  settings.syncLock = &syncLock;

  // Create the buffer
  buffer = new SampleBuffer< SampleType<i16complex> >(settings, true);
  board = &buffer->boards[0];

  board->noReadBefore(TimeStamp(EPOCH));
};

/*
 * Basic in-order read/writes should work.
 */
TEST(OneBlock_InOrder) {
  initBoard();

  // if we write first, reader should succeed immediately
  board->startWrite(TimeStamp(EPOCH + 10), TimeStamp(EPOCH + 20));
  board->stopWrite(TimeStamp(EPOCH + 20));

  // reading should fall through now
  board->startRead(TimeStamp(EPOCH + 10), TimeStamp(EPOCH + 20));
  board->stopRead(TimeStamp(EPOCH + 20));
}

/*
 * Reads before writes should wait.
 */
TEST(OneBlock_OutOfOrder) {
  initBoard();

  bool writtenBlock = false;

# pragma omp parallel sections
  {
#   pragma omp section
    {
      // Start reading -- should block on writer
      board->startRead(TimeStamp(EPOCH + 10), TimeStamp(EPOCH + 20));
      CHECK_EQUAL(true, writtenBlock);
      board->stopRead(TimeStamp(EPOCH + 20));
    }

#   pragma omp section
    {
      // Start writing
      board->startWrite(TimeStamp(EPOCH + 10), TimeStamp(EPOCH + 20));

      // Try to make sure that reader is waiting
      usleep(2000);

      writtenBlock = true;
      board->stopWrite(TimeStamp(EPOCH + 20));
    }
  }
}

/*
 * Content should not be overwritten in a wrap-around if it
 * has not been read yet.
 */
TEST(Overwrite) {
  initBoard();

  unsigned writtenBlock = 0;

# pragma omp parallel sections
  {
#   pragma omp section
    {
      // Wait for first block to be written
      usleep(2000);

      // Start reading -- should block on first block
      board->startRead(TimeStamp(EPOCH + 10), TimeStamp(EPOCH + 20));
      CHECK_EQUAL(1U, writtenBlock);
      board->stopRead(TimeStamp(EPOCH + 20));

      // Now the old data is released, and the new will be written
      board->startRead(TimeStamp(EPOCH + 10 + buffer->nrSamples), TimeStamp(EPOCH + 20 + buffer->nrSamples));
      CHECK_EQUAL(2U, writtenBlock);
      board->stopRead(TimeStamp(EPOCH + 20 + buffer->nrSamples));
    }

#   pragma omp section
    {
      // Start writing
      board->startWrite(TimeStamp(EPOCH + 10), TimeStamp(EPOCH + 20));
      writtenBlock = 1;
      board->stopWrite(TimeStamp(EPOCH + 20));

      // Start overwriting
      board->startWrite(TimeStamp(EPOCH + 10 + buffer->nrSamples), TimeStamp(EPOCH + 20 + buffer->nrSamples));
      writtenBlock = 2;
      board->stopWrite(TimeStamp(EPOCH + 20 + buffer->nrSamples));
    }
  }
}

/*
 * Missing data at the beginning should be handled.
 */
TEST(SkipBegin) {
  initBoard();

  // write 20-30
  board->startWrite(TimeStamp(EPOCH + 20), TimeStamp(EPOCH + 30));
  board->stopWrite(TimeStamp(EPOCH + 30));

  // reading 10-20 should be possible now
  board->startRead(TimeStamp(EPOCH + 10), TimeStamp(EPOCH + 20));
  board->stopRead(TimeStamp(EPOCH + 20));
}

/*
 * Missing data in the middle should be handled.
 */
TEST(SkipMiddle) {
  initBoard();

  // write begin part
  board->startWrite(TimeStamp(EPOCH + 10), TimeStamp(EPOCH + 20));
  board->stopWrite(TimeStamp(EPOCH + 20));

  // write end part
  board->startWrite(TimeStamp(EPOCH + 30), TimeStamp(EPOCH + 40));
  board->stopWrite(TimeStamp(EPOCH + 40));

  // reading middle should be possible now
  board->startRead(TimeStamp(EPOCH + 20), TimeStamp(EPOCH + 30));
  board->stopRead(TimeStamp(EPOCH + 30));
}


/*
 * noMoreReading() should allow writers to write freely.
 */
TEST(noMoreReading) {
  initBoard();

  // write something
  board->startWrite(TimeStamp(EPOCH + 10), TimeStamp(EPOCH + 20));
  board->stopWrite(TimeStamp(EPOCH + 20));

  // signal end of reading
  board->noMoreReading();

  // overwrite the same data, which is allowed if there are no readers
  board->startWrite(TimeStamp(EPOCH + 10 + buffer->nrSamples), TimeStamp(EPOCH + 20 + buffer->nrSamples));
  board->stopWrite(TimeStamp(EPOCH + 20 + buffer->nrSamples));
}


/*
 * noMoreWriting() should allow readers to read freely.
 */
TEST(noMoreWriting) {
  initBoard();

  board->noMoreWriting();

  // reading should fall through now
  board->startRead(TimeStamp(EPOCH + 10), TimeStamp(EPOCH + 20));
  board->stopRead(TimeStamp(EPOCH + 20));
}

int main()
{
  INIT_LOGGER( "tSampleBufferSync" );

  // Don't run forever if communication fails for some reason
  alarm(10);

  return UnitTest::RunAllTests();
}

