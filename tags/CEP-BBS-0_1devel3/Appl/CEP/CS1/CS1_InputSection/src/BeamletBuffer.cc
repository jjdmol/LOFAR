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

//# Includes
#include <Common/LofarLogger.h>
#include <CS1_InputSection/BeamletBuffer.h>
#include <Common/lofar_complex.h>
#include <Common/Timer.h>
#include <CS1_Interface/RSPTimeStamp.h>

namespace LOFAR {
namespace CS1 {

BeamletBuffer::BeamletBuffer(int bufferSize, unsigned nSubbands, unsigned history, unsigned readWriteDelay):
  itsNSubbands(nSubbands),
  itsSize(bufferSize),
  itsSBBuffers(boost::extents[nSubbands][bufferSize][NR_POLARIZATIONS]),
  itsLockedRange(bufferSize, readWriteDelay, bufferSize - history, 0),
  itsDroppedItems(0),
  itsDummyItems(0),
  itsSkippedItems(0),
  itsWriteTimer("write"),
  itsReadTimer("read")
{
  mutex::scoped_lock sl(itsFlagsMutex);
  itsFlags.include(0, bufferSize);
}

BeamletBuffer::~BeamletBuffer()
{      
  clog<<"BeamletBuffer did not receive "<<itsDummyItems<<" stamps and received "<<itsDroppedItems<<" items too late. "<<itsSkippedItems<<" items were skipped (but may be received later)."<<endl;
  clog<<"BeamletBufferTimers:"<<endl;
  clog<<itsReadTimer<<endl;
  clog<<itsWriteTimer<<endl;
}

void BeamletBuffer::checkForSkippedData(TimeStamp writeBegin) {
  // flag the data from itsHighestWritten to end
  while ((writeBegin > itsHighestWritten) and (itsHighestWritten > TimeStamp())) {
    // take only the first part, so the buffer won't block
    TimeStamp flagEnd = itsHighestWritten + itsSize/4;

    if (flagEnd > writeBegin)
      flagEnd = writeBegin;

    TimeStamp realBegin = itsLockedRange.writeLock(itsHighestWritten, flagEnd);

    itsWriteTimer.start();
    //cerr<<"BeamletBuffer: skipping "<<itsHighestWritten<<" - "<<flagEnd<<" ("<<flagEnd-itsHighestWritten<<")"<<endl;
    itsSkippedItems += flagEnd - itsHighestWritten;
    unsigned startI = mapTime2Index(realBegin), endI = mapTime2Index(flagEnd);

    {
      mutex::scoped_lock sl(itsFlagsMutex);
      if (endI < startI) {
	itsFlags.include(0, endI).include(startI, itsSize);
      } else {
	itsFlags.include(startI, endI);
      }
    } 

    itsWriteTimer.stop();

    if (itsHighestWritten < flagEnd)
      itsHighestWritten = flagEnd;

    itsLockedRange.writeUnlock(flagEnd);
  }
}

void BeamletBuffer::writeElements(Beamlet *data, TimeStamp begin, unsigned nElements)
{
  // if this part start beyond itsHighestWritten, there is a gap in the data in the buffer
  // so set that data to zero and invalidate it.
  //cerr<<"BeamletBuffer checking for skipped data"<<endl;
  checkForSkippedData(begin);	

  // Now write the normal data
  //cerr<<"BeamletBuffer writing normal data"<<endl;
  TimeStamp end = begin + nElements;
  TimeStamp realBegin = itsLockedRange.writeLock(begin, end);
  //cerr<<"BeamletBuffer writelock received"<<endl;

  if (realBegin < end) {
    itsDroppedItems += realBegin - begin;
    data += realBegin - begin;
    itsWriteTimer.start();

    unsigned startI = mapTime2Index(realBegin), endI = mapTime2Index(end);

    //cerr<<"BeamletBuffer: write from "<<realBegin<<" instead of "<<begin<<endl;
    //cerr<<"BeamletBuffer: Writing from "<<startI<<" to "<<endI<<" timestamp "<<begin<<endl;
    if (endI < startI) {
      // the data wraps around the allocated memory, so do it in two parts
      
      unsigned chunk1 = itsSize - startI;
      for (unsigned sb = 0; sb < itsNSubbands; sb ++) {
	memcpy(itsSBBuffers[sb][startI].origin(), &data[0]     , sizeof(SampleType[chunk1][NR_POLARIZATIONS]));
	memcpy(itsSBBuffers[sb][0].origin()     , &data[chunk1], sizeof(SampleType[endI][NR_POLARIZATIONS]));
	data += nElements;		
      }

      mutex::scoped_lock sl(itsFlagsMutex);
      itsFlags.exclude(startI, itsSize).exclude(0, endI);
    } else {
      for (unsigned sb = 0; sb < itsNSubbands; sb ++) {
	memcpy(itsSBBuffers[sb][startI].origin(), data, sizeof(SampleType[endI - startI][NR_POLARIZATIONS]));
	data += nElements;		
      }

      mutex::scoped_lock sl(itsFlagsMutex);
      itsFlags.exclude(startI, endI);
    }

    itsWriteTimer.stop();

    if (itsHighestWritten < end)
      itsHighestWritten = end;
  }

  itsLockedRange.writeUnlock(end);
}

void BeamletBuffer::getElements(boost::multi_array_ref<SampleType, 3> &buffers, SparseSet<unsigned> &flags, TimeStamp begin, unsigned nElements)
{
  //ASSERTSTR(buffers.size() == itsNSubbands, "BeamletBuffer received wrong number of buffers to write to (in getElements).");
  TimeStamp end = begin + nElements;
  TimeStamp realBegin = itsLockedRange.readLock(begin, end);
  
  itsReadTimer.start();

  unsigned nInvalid = realBegin - begin;
  itsDummyItems += nInvalid * itsNSubbands;
  flags.include(0, nInvalid); // set flags later

  // copy the real data
  unsigned startI = mapTime2Index(begin), endI = mapTime2Index(end);

  if (endI < startI) {
    // the data wraps around the allocated memory, so copy in two parts
    unsigned firstChunk = itsSize - startI;

    for (unsigned sb = 0; sb < itsNSubbands; sb ++) {
      memcpy(buffers[sb].origin()	     , itsSBBuffers[sb][startI].origin(), sizeof(SampleType[firstChunk][NR_POLARIZATIONS]));
      memcpy(buffers[sb][firstChunk].origin(), itsSBBuffers[sb][0].origin(),      sizeof(SampleType[endI][NR_POLARIZATIONS]));
    }

    mutex::scoped_lock sl(itsFlagsMutex);
    flags |= (itsFlags.subset(0,      endI)    += firstChunk);
    flags |= (itsFlags.subset(startI, itsSize) -= startI);
  } else {
    for (unsigned sb = 0; sb < itsNSubbands; sb ++) {
      memcpy(buffers[sb].origin(), itsSBBuffers[sb][startI].origin(), sizeof(SampleType[endI - startI][NR_POLARIZATIONS]));
    }	  
    mutex::scoped_lock sl(itsFlagsMutex);
    flags |= (itsFlags.subset(startI, endI) -= startI);
  }

  //cout<<"BeamletBuffer: getting elements "<<begin<<" - "<<begin+nElements<<": "<<flags<<endl;
 
  itsReadTimer.stop();
  itsLockedRange.readUnlock(end);
}

TimeStamp BeamletBuffer::startBufferRead() {
  TimeStamp oldest = itsLockedRange.getReadStart();
  TimeStamp fixPoint(oldest.getSeqId() + 1, 0); 
 
  TimeStamp realBegin = itsLockedRange.readLock(fixPoint, fixPoint);
  ASSERTSTR(realBegin == fixPoint, "Error in starting up buffer");
  itsLockedRange.readUnlock(fixPoint);
  return fixPoint;
}

TimeStamp BeamletBuffer::startBufferRead(TimeStamp begin) {
  TimeStamp oldest = itsLockedRange.getReadStart();
  TimeStamp realBegin = itsLockedRange.readLock(begin, begin);
  ASSERTSTR(realBegin == begin, "Error in starting up buffer");

  // if begin is no longer in the buffer the oldest possible beginning is returned
  // if begin is not yet in the buffer readLock waits for it

  itsLockedRange.readUnlock(realBegin);
  return realBegin;
}

} // namespace CS1
} // namespace LOFAR
