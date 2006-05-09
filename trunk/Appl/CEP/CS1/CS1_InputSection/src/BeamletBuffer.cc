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

    BeamletBuffer::BeamletBuffer(uint bufferSize, uint nSubbands, uint history, uint readWriteDelay):
      itsNSubbands(nSubbands),
      itsSize(bufferSize),
      itsLockedRange(bufferSize, readWriteDelay, bufferSize - history, 0),
      itsDroppedItems(0),
      itsDummyItems(0),
      itsWriteTimer("write"),
      itsReadTimer("read")
    {
      for (uint sb = 0; sb < nSubbands; sb ++) {
	itsSBBuffers.push_back(new Beamlet[bufferSize]);
      }
      itsFlags.include(0, bufferSize);
    }

    BeamletBuffer::~BeamletBuffer()
    {      
      cout<<"BeamletBuffer did not receive "<<itsDummyItems<<" stamps and received "<<itsDroppedItems<<" items too late."<<endl;
      cout<<"BeamletBufferTimers:"<<endl;
      cout<<itsReadTimer<<endl;
      cout<<itsWriteTimer<<endl;
      cout.flush();

      vector<Beamlet*>::iterator bit = itsSBBuffers.begin();
      for (; bit != itsSBBuffers.end(); bit++) {
	delete [] *bit;
      }
    }

    uint BeamletBuffer::writeElements(Beamlet* data, TimeStamp begin, uint nElements, uint stride)
    {
      // if this part start beyond itsHighestWritten, there is a gap in the data in the buffer
      // so set that data to zero and invalidate it.
      if ((begin > itsHighestWritten) and (itsHighestWritten > TimeStamp())) {
	TimeStamp realBegin = itsLockedRange.writeLock(itsHighestWritten, begin);

	itsWriteTimer.start();
	// we skipped a part, so write zeros there
	uint startI = mapTime2Index(realBegin);
	uint endI = mapTime2Index(begin);

	if (endI < startI) {
	  // the data wraps around the allocated memory, so do it in two parts
	  uint firstChunk = itsSize - startI;
#if defined USE_DEBUG
	  for (uint sb = 0; sb < itsNSubbands; sb++) {
	    memset(&(itsSBBuffers[sb])[startI], 0, firstChunk * sizeof(Beamlet));
	    memset(&(itsSBBuffers[sb])[0], 0, endI * sizeof(Beamlet));
	  }
#endif
	  itsFlags.include(0, endI).include(startI, firstChunk);
	} else {
#if defined USE_DEBUG
	  for (uint sb = 0; sb < itsNSubbands; sb++) {
	    memset(&(itsSBBuffers[sb])[startI], 0, (endI - startI) * sizeof(Beamlet));	    
	  }
#endif
	  itsFlags.include(startI, endI);
	}	
	itsWriteTimer.stop();
	itsLockedRange.writeUnlock(begin);
      }	

      // Now write the normal data
      TimeStamp end = begin + nElements;
      TimeStamp realBegin = itsLockedRange.writeLock(begin, end);

      itsDroppedItems += realBegin - begin;
     
      itsWriteTimer.start();

      uint startI = mapTime2Index(realBegin);
      uint endI = mapTime2Index(end);

      if (endI < startI) {
	// the data wraps around the allocated memory, so do it in two parts
	for (uint i = startI; i < itsSize; i ++) {
	  for (uint sb = 0; sb < itsNSubbands; sb++) {
	    itsSBBuffers[sb][i] = data[sb];
	  }
	  data += stride;
	}      
	for (uint i = 0; i < endI; i ++) {
	  for (uint sb = 0; sb < itsNSubbands; sb++) {
	    itsSBBuffers[sb][i] = data[sb];
	  }
	  data += stride;
	}      
	itsFlags.exclude(startI, itsSize).exclude(0, endI);
      } else {
	for (uint i = startI; i < endI; i ++) {
	  for (uint sb = 0; sb < itsNSubbands; sb++) {
	    itsSBBuffers[sb][i] = data[sb];
	  }
	  data += stride;
	}      
	itsFlags.exclude(startI, endI);
      }

      itsWriteTimer.stop();
      if (itsHighestWritten < end) itsHighestWritten = end;
      itsLockedRange.writeUnlock(end);
      return end - realBegin;
    }

    uint BeamletBuffer::getElements(vector<Beamlet *> buffers, SparseSet *flags, TimeStamp begin, uint nElements)
    {
      ASSERTSTR(buffers.size() == itsNSubbands, "BeamletBuffer received wrong number of buffers to write to (in getElements).");
      TimeStamp end = begin + nElements;
      TimeStamp realBegin = itsLockedRange.readLock(begin, end);
      
      itsReadTimer.start();

      // copy zeros for the part that was already out of the buffer
      uint nInvalid = realBegin - begin;
#if defined USE_DEBUG
      for (uint sb = 0; sb < itsNSubbands; sb++) {
	memset(&buffers[sb][0], 0, nInvalid * sizeof(Beamlet));
      }
#endif
      // set flags later
      itsDummyItems += nInvalid * itsNSubbands;
      flags->include(0, nInvalid);

      // copy the real data
      uint startI = mapTime2Index(realBegin);
      uint endI = mapTime2Index(end);

      if (endI < startI) {
	// the data wraps around the allocated memory, so copy in two parts
	uint firstChunk = itsSize - startI;

	for (uint sb = 0; sb < itsNSubbands; sb++) {
	  memcpy(&buffers[sb][nInvalid],   &itsSBBuffers[sb][startI], firstChunk * sizeof(Beamlet));
	  memcpy(&buffers[sb][firstChunk], &itsSBBuffers[sb][0],      endI       * sizeof(Beamlet));

	  //itsDummyItems += blabla;
	}

	*flags |= (itsFlags.subset(0,      endI)    += nInvalid + firstChunk);
	*flags |= (itsFlags.subset(startI, itsSize) -= nInvalid + startI);
      } else {
	// copy in one part
	for (uint sb = 0; sb < itsNSubbands; sb++) {
	  memcpy(&buffers[sb][nInvalid], &itsSBBuffers[sb][startI], (endI - startI) * sizeof(Beamlet));
	  //flags[sb]->setFlags(itsFlags[blabla]);
	  //itsDummyItems += blabla;
	}	  
	*flags |= (itsFlags.subset(startI, endI) -= nInvalid + startI);
      }
     
      itsReadTimer.stop();
      itsLockedRange.readUnlock(end);
      return end - realBegin;
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
