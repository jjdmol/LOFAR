/* ReaderWriterSynchronization.cc
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

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

#include "ReaderWriterSynchronization.h"

namespace LOFAR
{
  namespace Cobalt
  {

    SynchronizedReaderAndWriter::SynchronizedReaderAndWriter(unsigned bufferSize)
      :
      bufferSize(bufferSize)
    {
    }


    SynchronizedReaderAndWriter::~SynchronizedReaderAndWriter()
    {
    }


    void SynchronizedReaderAndWriter::Reader::Reader(SynchronizedReaderAndWriter &srw, const TimeStamp &begin, const TimeStamp &end)
    :
      srw(srw),
      begin(begin),
      end(end)
    {
      // First, release any data until 'begin'
      srw.readPointer.advanceTo(begin);

      // Wait until writer is beyond 'end'
      srw.writePointer.waitFor(end);
    }


    void SynchronizedReaderAndWriter::Reader::~Reader()
    {
      // Make the range until 'end' free for writing
      srw.readPointer.advanceTo(end);
    }


    void SynchronizedReaderAndWriter::Writer::Writer(SynchronizedReaderAndWriter &srw, const TimeStamp &begin, const TimeStamp &end)
    :
      srw(srw),
      begin(begin),
      end(end)
    {
      // Advance write pointer to 'begin', in case we skipped data
      srw.writePointer.advanceTo(begin);

      // Also, we can't write if we'd overwrite old data
      srw.readPointer.waitFor(end - srw.bufferSize);
    }


    void SynchronizedReaderAndWriter::Writer::~Writer()
    {
      // Make the range until 'end' available 
      srw.writePointer.advanceTo(end);
    }


    void SynchronizedReaderAndWriter::noMoreReading()
    {
      // advance read pointer to infinity, to unblock thread that waits in startWrite
      readPointer.advanceTo(TimeStamp(0x7FFFFFFFFFFFFFFFLL)); // we only use this TimeStamp for comparison so clockSpeed does not matter
    }


    void SynchronizedReaderAndWriter::noMoreWriting()
    {
      writePointer.advanceTo(TimeStamp(0x7FFFFFFFFFFFFFFFLL));
    }
  } // namespace Cobalt
} // namespace LOFAR

