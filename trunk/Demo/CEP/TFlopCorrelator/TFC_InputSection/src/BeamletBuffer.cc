//#  BeamletBuffer.cc: one line description
//#
//#  Copyright (C) 2005
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
#include <TFC_InputSection/BeamletBuffer.h>

namespace LOFAR {

    BeamletBuffer::BeamletBuffer(int bufferSize, int nSubbands, int history, int readWriteDelay):
      itsNSubbands(nSubbands),
      itsSize(bufferSize),
      itsLockedRange(bufferSize, readWriteDelay, bufferSize - history, 0),
      itsWriteTimer("write"),
      itsReadTimer("read")
    {
      for (int sb = 0; sb < nSubbands; sb ++) {
	itsSBBuffers.push_back(new SubbandType[bufferSize]);
      }
      itsInvalidFlags = new bool[bufferSize]; 
    }

    BeamletBuffer::~BeamletBuffer()
    {      
      cout<<"\nBeamletBufferTimers:"<<endl;
      itsWriteTimer.print(cout);
      itsReadTimer.print(cout);

      vector<SubbandType*>::iterator bit = itsSBBuffers.begin();
      for (; bit != itsSBBuffers.end(); bit++) {
	delete [] *bit;
      }
      delete [] itsInvalidFlags;
    }

    int BeamletBuffer::writeElements(SubbandType* data, TimeStamp begin, int nElements, int stride, bool valid) {
      TimeStamp end = begin + nElements;
      TimeStamp realBegin = itsLockedRange.writeLock(begin, end);
     
      itsWriteTimer.start();
      for (TimeStamp i = realBegin; i < end; i++) {
	for (int sb = 0; sb < itsNSubbands; sb++) {
	  itsSBBuffers[sb][map(i)] = data[sb];
	}
	data += stride;
	itsInvalidFlags[map(i)] = !valid;
      }      
      itsWriteTimer.stop();
      itsLockedRange.writeUnlock(end);
      return end - realBegin;
    }

    int BeamletBuffer::getElements(vector<SubbandType*> buffers, int& invalidCount, TimeStamp begin, int nElements) { 
      ASSERTSTR(buffers.size() == itsNSubbands, "BeamletBuffer received wrong number of buffers to write to (in getElements).");
      TimeStamp end = begin + nElements;
      TimeStamp realBegin = itsLockedRange.readLock(begin, end);
      
      itsReadTimer.start();
      ASSERTSTR(realBegin <= end, "requested data no longer present in buffer");
      int startI = map(realBegin);
      int endI = map(end);
      if (endI < startI) {
	  int firstChunk = itsSize - startI;
	  for (int sb = 0; sb < itsNSubbands; sb++) {
	    memcpy(&buffers[sb][0], &(itsSBBuffers[sb])[startI], firstChunk * sizeof(SubbandType));
	    memcpy(&buffers[sb][firstChunk], &(itsSBBuffers[sb])[0], endI * sizeof(SubbandType));
	  }
      } else {
	  for (int sb = 0; sb < itsNSubbands; sb++) {
	    memcpy(&buffers[sb][0], &itsSBBuffers[sb][startI], (endI - startI) * sizeof(SubbandType));
	  }	  
      }

      invalidCount = 0;
      for (TimeStamp i = realBegin; i < end; i++) {
	invalidCount += itsInvalidFlags[map(i)] ? 1 : 0;
      }

      itsReadTimer.stop();
      itsLockedRange.readUnlock(end);
      return end - realBegin;
    }

    TimeStamp BeamletBuffer::startBufferRead() {
      TimeStamp begin(0, 0);
      TimeStamp oldest = itsLockedRange.readLock(begin, begin);
      TimeStamp fixPoint(oldest.getSeqId() + 1, 0);
      
      itsLockedRange.readUnlock(fixPoint);
      return fixPoint;
    }

    TimeStamp BeamletBuffer::startBufferRead(TimeStamp begin) {
      TimeStamp realBegin = itsLockedRange.readLock(begin, begin);

      itsLockedRange.readUnlock(realBegin);
      return realBegin;
    }
      
} // namespace LOFAR
