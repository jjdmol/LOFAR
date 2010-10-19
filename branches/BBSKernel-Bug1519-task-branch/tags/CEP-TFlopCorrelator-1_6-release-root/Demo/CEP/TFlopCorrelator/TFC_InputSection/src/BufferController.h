//#  BufferController.h: class to control a cyclic buffer containing RSP data
//#
//#  Copyright (C) 2000, 2001
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


#ifndef TFLOPCORRELATOR_BUFFERCONTROLLER_H
#define TFLOPCORRELATOR_BUFFERCONTROLLER_H

#include <Common/Timer.h>
#include <TFC_Interface/RSPTimeStamp.h>
#include <TFC_InputSection/CyclicCounter.h>
#include <APS/ParameterSet.h>
#include <boost/thread.hpp>

namespace LOFAR
{

using namespace boost;
using ACC::APS::ParameterSet;

typedef struct 
{
  i16complex Xpol;
  i16complex Ypol;
} SubbandType;

typedef struct
{
  int invalid;
  timestamp_t timestamp;
} MetadataType;

class BufferController
{
 public:
 
   BufferController(int buffersize, int nsubbands, int historySize, int maxCount);
   ~BufferController();
   void getElements(vector<SubbandType*> buf, int& invalidcount, timestamp_t startstamp, int nelements);
   void writeElements(SubbandType* buf, timestamp_t rspstamp);
   void writeElements(SubbandType* buf, timestamp_t rspstamp, int nelements, int stride);
   void writeDummy(SubbandType* dum, timestamp_t startstamp, int nelements);
   bool rewriteElements(SubbandType* buf, timestamp_t startstamp);
   
   // disable overwriting and remember the newest element
   // return the newest element (to be used on the master)
   timestamp_t startBufferRead();
   // disable overwrite at the given stamp (to be used on the client)
   void startBufferRead(timestamp_t stamp);

   void clear();
   
   void setAllowOverwrite(bool allow);

  private:

   // the buffers
   vector<SubbandType*> itsSubbandBuffer;
   MetadataType* itsMetadataBuffer;

   // index pointers
   CyclicCounter itsHead;
   CyclicCounter itsTail;
   CyclicCounter itsOldHead;
   CyclicCounter itsOldTail;

   int itsBufferSize;
   int itsNSubbands; 

   // permission to overwrite previous written elements
   bool itsOverwritingAllowed;
   
   mutex buffer_mutex;        // lock/unlock shared data
   condition data_available;  // 'buffer not empty' trigger
   condition space_available; // 'buffer not full' trigger

   int getCount();
   int writeLockRange(int nelements);
   int readLockRange(int nelements);
   int setReadOffset(timestamp_t startstamp);
   int setRewriteOffset(timestamp_t startstamp);
   void setStartOffset(timestamp_t startstamp);
   timestamp_t setStartOffset();     
   void releaseWriteBlock();
   void releaseReadBlock();
   void releaseRewriteBlock();
   timestamp_t getOldestStamp(mutex::scoped_lock& sl);
   timestamp_t getNewestStamp(mutex::scoped_lock& sl);

   NSTimer itsWriteLockTimer;
   NSTimer itsWriteTimer;
   NSTimer itsWriteUnlockTimer;
   NSTimer itsReadLockTimer;
   NSTimer itsReadTimer;
   NSTimer itsReadUnlockTimer;
   NSTimer itsWaitingForDataTimer;
   NSTimer itsWaitingForSpaceTimer;

   int itsMinCount;
   int itsMaxCount;
};

inline int BufferController::getCount()
{
  return itsOldHead - itsTail;
}

inline void BufferController::setAllowOverwrite(bool allow)
{
  itsOverwritingAllowed = allow;
}
}
#endif

