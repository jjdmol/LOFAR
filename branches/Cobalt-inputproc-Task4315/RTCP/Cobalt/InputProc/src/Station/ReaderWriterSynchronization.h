/* ReaderWriterSynchronization.h
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

#ifndef LOFAR_INPUTPROC_READER_WRITER_SYNCHRONIZATION
#define LOFAR_INPUTPROC_READER_WRITER_SYNCHRONIZATION

//# Never #include <config.h> or #include <lofar_config.h> in a header file!

#include <CoInterface/RSPTimeStamp.h>

#include <SlidingPointer.h>

namespace LOFAR
{
  namespace Cobalt
  {
    class SynchronizedReaderAndWriter
    {
    public:
      SynchronizedReaderAndWriter(unsigned bufferSize);
      ~SynchronizedReaderAndWriter();

      class Reader {
      public:
        Reader(SynchronizedReaderAndWriter &srw, const TimeStamp &begin, const TimeStamp &end);
        ~Reader();

      private:
        SynchronizedReaderAndWriter &srw;
        const TimeStamp &begin;
        const TimeStamp &end;
      };

      class Writer {
      public:
        Writer(SynchronizedReaderAndWriter &srw, const TimeStamp &begin, const TimeStamp &end);
        ~Writer();

      private:
        SynchronizedReaderAndWriter &srw;
        const TimeStamp &begin;
        const TimeStamp &end;
      };

      void         noMoreReading();
      void         noMoreWriting();

    private:
      // The read and write positions.
      SlidingPointer<TimeStamp> readPointer, writePointer;

      // The size of the buffer we scan over, to prevent overwriting data due
      // to buffer wrap-around.
      unsigned bufferSize;

      friend class Reader;
      friend class Writer;
    };
  } // namespace Cobalt
} // namespace LOFAR

#endif

