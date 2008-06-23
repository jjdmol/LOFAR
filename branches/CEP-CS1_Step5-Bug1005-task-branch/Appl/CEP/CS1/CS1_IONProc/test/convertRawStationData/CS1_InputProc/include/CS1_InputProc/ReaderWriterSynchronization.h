//#  Copyright (C) 2007
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//#  This program is free software; you can redistribute it and/or modify
//#  it under the terms of the GNU General Public License as published by
//#  the Free Software Foundation; either version 2 of the License, or
//#  (at your option) any later version.
//#
//#  This program is distributed in the hope that it will be useful,
//#  but WITHOUT ANY WARRANTY; without even the implied warranty of
//#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//#  GNU General Public License for more details.
//#
//#  You should have received a copy of the GNU General Public License
//#  along with this program; if not, write to the Free Software
//#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#
//#  $Id$

#ifndef LOFAR_APPL_CEP_CS1_CS1_IONPROC_READER_WRITER_SYNCHRONIZATION
#define LOFAR_APPL_CEP_CS1_CS1_IONPROC_READER_WRITER_SYNCHRONIZATION

//# Never #include <config.h> or #include <lofar_config.h> in a header file!

#include <CS1_Interface/RSPTimeStamp.h>
#include <CS1_InputProc/SlidingPointer.h>
#include <CS1_InputProc/WallClockTime.h>

#include <pthread.h>


namespace LOFAR {
namespace CS1 {  


class ReaderAndWriterSynchronization
{
  public:
    virtual	 ~ReaderAndWriterSynchronization();

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
    
  private:
    SlidingPointer<TimeStamp> itsReadPointer, itsWritePointer;
    unsigned		      itsBufferSize;
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
    unsigned	  itsMaximumNetworkLatency;
};



} // namespace CS1
} // namespace LOFAR

#endif
