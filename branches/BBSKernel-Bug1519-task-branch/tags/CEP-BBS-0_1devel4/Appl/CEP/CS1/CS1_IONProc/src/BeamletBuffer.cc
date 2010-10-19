//#  BeamletBuffer.cc: one line description
//#
//#  Copyright (C) 2006
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

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

#include <BeamletBuffer.h>
#include <ION_Allocator.h>


namespace LOFAR {
namespace CS1 {

BeamletBuffer::BeamletBuffer(unsigned bufferSize, unsigned nrSubbands, unsigned history, bool isSynchronous, unsigned maxNetworkDelay)
:
  itsNSubbands(nrSubbands),
  itsSize(bufferSize),
  itsHistorySize(history),
  itsSBBuffers(reinterpret_cast<SampleType *>(ION_Allocator().allocate(nrSubbands * bufferSize * NR_POLARIZATIONS * sizeof(SampleType), 32)), boost::extents[nrSubbands][bufferSize][NR_POLARIZATIONS]),
  itsReadTimer("buffer read", true),
  itsWriteTimer("buffer write", true)
{
  pthread_mutex_init(&itsValidDataMutex, 0);

  if (isSynchronous)
    itsSynchronizedReaderWriter = new SynchronizedReaderAndWriter(bufferSize);
  else
    itsSynchronizedReaderWriter = new TimeSynchronizedReader(maxNetworkDelay);
}


BeamletBuffer::~BeamletBuffer()
{      
  delete itsSynchronizedReaderWriter;
  pthread_mutex_destroy(&itsValidDataMutex);
  ION_Allocator().deallocate(itsSBBuffers.origin());
}


void BeamletBuffer::writeElements(Beamlet *data, const TimeStamp &begin, unsigned nrElements)
{
  static TimeStamp previous;  // cache previous index, to avoid expensive
  static unsigned  previousI; // mapTime2Index()

  TimeStamp end = begin + nrElements;
  itsWriteTimer.start();

  unsigned startI = (begin == previous) ? previousI : mapTime2Index(begin);
  unsigned endI   = startI + nrElements;

  if (endI >= itsSize)
    endI -= itsSize;

  previous  = end;
  previousI = endI;
  itsWriteTimer.stop();

  // in synchronous mode, do not overrun tail of reader
  itsSynchronizedReaderWriter->startWrite(begin, end);
  // do not write in circular buffer section that is being read
  itsLockedRanges.lock(startI, endI, itsSize);

  if (endI < startI) {
    // the data wraps around the allocated memory, so do it in two parts
    
    unsigned chunk1 = itsSize - startI;
    for (unsigned sb = 0; sb < itsNSubbands; sb ++) {
      memcpy(itsSBBuffers[sb][startI].origin(), &data[0]     , sizeof(SampleType[chunk1][NR_POLARIZATIONS]));
      memcpy(itsSBBuffers[sb][0].origin()     , &data[chunk1], sizeof(SampleType[endI][NR_POLARIZATIONS]));
      data += nrElements;		
    }
  } else {
    for (unsigned sb = 0; sb < itsNSubbands; sb ++) {
      memcpy(itsSBBuffers[sb][startI].origin(), data, sizeof(SampleType[endI - startI][NR_POLARIZATIONS]));
      data += nrElements;		
    }
  }

  // forget old ValidData; add new ValidData
  pthread_mutex_lock(&itsValidDataMutex);
  itsValidData.exclude(0, end - itsSize).include(begin, end);
  pthread_mutex_unlock(&itsValidDataMutex);

  itsLockedRanges.unlock(startI, endI, itsSize);
  itsSynchronizedReaderWriter->finishedWrite(end);
}


void BeamletBuffer::startReadTransaction(const TimeStamp &begin, unsigned nrElements)
{
  itsReadTimer.start();

  itsBegin  = begin;
  itsEnd    = begin + nrElements;
  itsStartI = mapTime2Index(begin);
  itsEndI   = mapTime2Index(itsEnd);

  // in synchronous mode, do not overrun writer
  itsSynchronizedReaderWriter->startRead(begin, itsEnd);
  // do not read from circular buffer section that is being written
  itsLockedRanges.lock(itsStartI, itsEndI, itsSize);
}


void BeamletBuffer::sendSubband(TransportHolder *th, unsigned subband) /*const*/
{
  // Align to 32 bytes and make multiple of 32 bytes by prepending/appending
  // extra data.  Always send 32 bytes extra, even if data was already aligned.
  unsigned startI = itsStartI & ~(32 / sizeof(Beamlet) - 1); // round down
  unsigned endI   = (itsEndI + 32 / sizeof(Beamlet)) & ~(32 / sizeof(Beamlet) - 1); // round up, possibly adding 32 bytes

  if (endI < startI) {
    // the data wraps around the allocated memory, so copy in two parts
    unsigned firstChunk = itsSize - startI;

    th->sendBlocking(itsSBBuffers[subband][startI].origin(), sizeof(SampleType[firstChunk][NR_POLARIZATIONS]), 0, 0);
    th->sendBlocking(itsSBBuffers[subband][0].origin(),      sizeof(SampleType[endI][NR_POLARIZATIONS]), 0, 0);
  } else {
    th->sendBlocking(itsSBBuffers[subband][startI].origin(), sizeof(SampleType[endI - startI][NR_POLARIZATIONS]), 0, 0);
  }
}


void BeamletBuffer::readFlags(SparseSet<unsigned> &flags)
{
  pthread_mutex_lock(&itsValidDataMutex);
  SparseSet<TimeStamp> validTimes = itsValidData.subset(itsBegin, itsEnd);
  pthread_mutex_unlock(&itsValidDataMutex);

  flags.reset().include(0, static_cast<unsigned>(itsEnd - itsBegin));

  for (SparseSet<TimeStamp>::const_iterator it = validTimes.getRanges().begin(); it != validTimes.getRanges().end(); it ++)
    flags.exclude(static_cast<unsigned>(it->begin - itsBegin),
		  static_cast<unsigned>(it->end - itsBegin));
}

void BeamletBuffer::stopReadTransaction()
{
  itsLockedRanges.unlock(itsStartI, itsEndI, itsSize);
  itsSynchronizedReaderWriter->finishedRead(itsEnd - (itsHistorySize + 16));
  // subtract 16 extra; due to alignment restrictions and the changing delays,
  // it is hard to predict where the next read will begin.

  itsReadTimer.stop();
}

} // namespace CS1
} // namespace LOFAR
