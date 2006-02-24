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
  namespace CS1_InputSection {

    BeamletBuffer::BeamletBuffer(int bufferSize, int nSubbands, int history, int readWriteDelay):
      itsNSubbands(nSubbands),
      itsSize(bufferSize),
      itsLockedRange(bufferSize, readWriteDelay, bufferSize - history, 0),
      itsDroppedItems(0),
      itsDummyItems(0),
      itsWriteTimer("write"),
      itsReadTimer("read")
    {
      for (int sb = 0; sb < nSubbands; sb ++) {
	itsSBBuffers.push_back(new Beamlet[bufferSize]);
      }
      itsInvalidFlags = new bool[bufferSize]; 
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
      delete [] itsInvalidFlags;
    }

    int BeamletBuffer::writeElements(Beamlet* data, TimeStamp begin, int nElements, int stride) {
      TimeStamp end = begin + nElements;
      TimeStamp realBegin = itsLockedRange.writeLock(begin, end);
      itsDroppedItems += realBegin - begin;
     
      itsWriteTimer.start();
      for (TimeStamp i = realBegin; i < end; i++) {
	for (int sb = 0; sb < itsNSubbands; sb++) {
	  itsSBBuffers[sb][mapTime2Index(i)] = data[sb];
	}
	data += stride;
	itsInvalidFlags[mapTime2Index(i)] = false; // these items are not invalid
      }      
      itsWriteTimer.stop();
      itsLockedRange.writeUnlock(end);
      return end - realBegin;
    }

    int BeamletBuffer::getElements(vector<Beamlet*> buffers, int& invalidCount, TimeStamp begin, int nElements) { 
      ASSERTSTR(buffers.size() == itsNSubbands, "BeamletBuffer received wrong number of buffers to write to (in getElements).");
      TimeStamp end = begin + nElements;
      TimeStamp realBegin = itsLockedRange.readLock(begin, end);
      
      itsReadTimer.start();
      ASSERTSTR(realBegin <= end, "requested data no longer present in buffer");
      int startI = mapTime2Index(realBegin);
      int endI = mapTime2Index(end);
      if (endI < startI) {
	  int firstChunk = itsSize - startI;
	  for (int sb = 0; sb < itsNSubbands; sb++) {
	    memcpy(&buffers[sb][0], &(itsSBBuffers[sb])[startI], firstChunk * sizeof(Beamlet));
	    memcpy(&buffers[sb][firstChunk], &(itsSBBuffers[sb])[0], endI * sizeof(Beamlet));
	  }
      } else {
	  for (int sb = 0; sb < itsNSubbands; sb++) {
	    memcpy(&buffers[sb][0], &itsSBBuffers[sb][startI], (endI - startI) * sizeof(Beamlet));
	  }	  
      }

      invalidCount = 0;      
      for (TimeStamp i = realBegin; i < end; i++) {
	if (itsInvalidFlags[mapTime2Index(i)]) {
	  for (int sb = 0; sb < itsNSubbands; sb++) {
	    // for all invalid subbands set the subband to zero, it will later be copied to the output
	    memset(&(itsSBBuffers[sb])[mapTime2Index(i)], 0, sizeof(Beamlet));
	  }
	  invalidCount += 1;
	}
	// invalidate all positions
	// TODO: this assumes every item is read exactly once (no more, no less)
	itsInvalidFlags[mapTime2Index(i)] = true;
      }
      itsDummyItems += invalidCount;

      itsReadTimer.stop();
      itsLockedRange.readUnlock(end);
      return end - realBegin;
    }

    TimeStamp BeamletBuffer::startBufferRead() {
      TimeStamp oldest = itsLockedRange.getReadStart();
      TimeStamp fixPoint(oldest.getSeqId() + 1, 0); 
     
      itsLockedRange.readUnlock(fixPoint);
      return fixPoint;
    }

    TimeStamp BeamletBuffer::startBufferRead(TimeStamp begin) {
      TimeStamp oldest = itsLockedRange.getReadStart();
      TimeStamp realBegin = itsLockedRange.readLock(begin, begin);

      // if begin is no longer in the buffer the oldest possible beginning is returned
      // if begin is not yet in the buffer readLock waits for it

      itsLockedRange.readUnlock(realBegin);
      return realBegin;
    }

  } // namespace CS1_InputSection
} // namespace LOFAR
