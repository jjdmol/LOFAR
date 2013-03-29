//# ReaderWriterSynchronization.h
//# Copyright (C) 2012-2013  ASTRON (Netherlands Institute for Radio Astronomy)
//# P.O. Box 2, 7990 AA Dwingeloo, The Netherlands
//#
//# This file is part of the LOFAR software suite.
//# The LOFAR software suite is free software: you can redistribute it and/or
//# modify it under the terms of the GNU General Public License as published
//# by the Free Software Foundation, either version 3 of the License, or
//# (at your option) any later version.
//#
//# The LOFAR software suite is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License along
//# with the LOFAR software suite. If not, see <http://www.gnu.org/licenses/>.
//#
//# $Id$

#ifndef LOFAR_GPUPROC_READER_WRITER_SYNCHRONIZATION
#define LOFAR_GPUPROC_READER_WRITER_SYNCHRONIZATION

//# Never #include <config.h> or #include <lofar_config.h> in a header file!

#include <CoInterface/RSPTimeStamp.h>

#include <GPUProc/SlidingPointer.h>
#include "WallClockTime.h"

namespace LOFAR
{
  namespace Cobalt
  {


    class ReaderAndWriterSynchronization
    {
    public:
      virtual ~ReaderAndWriterSynchronization();

      virtual void startRead(const TimeStamp &begin, const TimeStamp &end) = 0;
      virtual void finishedRead(const TimeStamp &advanceTo) = 0;

      virtual void startWrite(const TimeStamp &begin, const TimeStamp &end) = 0;
      virtual void finishedWrite(const TimeStamp &advanceTo) = 0;
    };


    class SynchronizedReaderAndWriter : public ReaderAndWriterSynchronization
    {
    public:
      SynchronizedReaderAndWriter(unsigned bufferSize);
      ~SynchronizedReaderAndWriter();

      virtual void startRead(const TimeStamp &begin, const TimeStamp &end);
      virtual void finishedRead(const TimeStamp &advanceTo);

      virtual void startWrite(const TimeStamp &begin, const TimeStamp &end);
      virtual void finishedWrite(const TimeStamp &advanceTo);

      void         noMoreReading();
      void         noMoreWriting();

    private:
      SlidingPointer<TimeStamp> itsReadPointer, itsWritePointer;
      unsigned itsBufferSize;
    };


    class TimeSynchronizedReader : public ReaderAndWriterSynchronization
    {
    public:
      TimeSynchronizedReader(unsigned maximumNetworkLatency);
      ~TimeSynchronizedReader();

      virtual void  startRead(const TimeStamp &begin, const TimeStamp &end);
      virtual void  finishedRead(const TimeStamp &advanceTo);

      virtual void  startWrite(const TimeStamp &begin, const TimeStamp &end);
      virtual void  finishedWrite(const TimeStamp &advanceTo);

    private:
      WallClockTime itsWallClock;
      unsigned itsMaximumNetworkLatency;
    };



  } // namespace Cobalt
} // namespace LOFAR

#endif

