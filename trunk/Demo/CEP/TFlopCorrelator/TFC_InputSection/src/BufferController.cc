//#  BufferController.cc: Control cyclic buffers for subbands and metadata
//#
//#  Copyright (C) 2002-2005
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
#include <lofar_config.h>

#include <math.h>
#include <stdlib.h>
#include <TFC_InputSection/BufferController.h>
#include <Common/hexdump.h>
#include <Common/LofarLogger.h>

namespace LOFAR {

BufferController::BufferController(int buffersize, int nsubbands, int historySize, int maxCount)
  : itsBufferSize(buffersize),
    itsNSubbands(nsubbands),
    itsHead(buffersize),
    itsTail(buffersize),
    itsOldHead(buffersize),
    itsOldTail(buffersize),
    itsOverwritingAllowed(true),
    itsWriteLockTimer("writeLock"),
    itsWriteTimer("write"),
    itsWriteUnlockTimer("writeUnlock"),
    itsReadLockTimer("readLock"),
    itsReadTimer("read"),
    itsReadUnlockTimer("readUnlock"),
    itsWaitingForDataTimer("waitingForData"),
    itsWaitingForSpaceTimer("waitingForSpace"),
    itsMinCount(historySize),
    itsMaxCount(maxCount)
{
  // create metadata buffer
  itsMetadataBuffer = new MetadataType[itsBufferSize];
  
  // create subbands buffer
  for (int s=0; s<itsNSubbands; s++) {
    itsSubbandBuffer.push_back(new SubbandType[itsBufferSize]);
  }
}

BufferController::~BufferController()
{
  cout<<"\nBufferTimers:"<<endl;
  itsWriteLockTimer.print(cout);
  itsWriteTimer.print(cout);
  itsWriteUnlockTimer.print(cout);
  itsReadLockTimer.print(cout);
  itsReadTimer.print(cout);
  itsReadUnlockTimer.print(cout);
  itsWaitingForDataTimer.print(cout);
  itsWaitingForSpaceTimer.print(cout);

  vector<SubbandType*>::iterator sit = itsSubbandBuffer.begin();
  for (; sit!=itsSubbandBuffer.end(); sit++) {
    delete *sit;
  }
  itsSubbandBuffer.clear();

  delete itsMetadataBuffer;
}
 
timestamp_t BufferController::getOldestStamp(mutex::scoped_lock& sl)
{
  // wait until at least one element is available
  bool amWaiting = false;
  while (getCount() <= 0) 
  {
    if (!amWaiting) {
      itsWaitingForDataTimer.start();
      amWaiting = true;
    }
    data_available.wait(sl);
  }
  if (amWaiting) {
    itsWaitingForDataTimer.stop();
  }
  // CONDITION: Count > 0
  int bid = itsTail.getValue();

  return itsMetadataBuffer[bid].timestamp;
}

timestamp_t BufferController::getNewestStamp(mutex::scoped_lock& sl)
{
  // wait until at least one element is available
  bool amWaiting = false;
  while (getCount() <= 0) 
  {
    if (!amWaiting) {
      itsWaitingForDataTimer.start();
      amWaiting = true;
    }
    data_available.wait(sl);
  }
  if (amWaiting) {
    itsWaitingForDataTimer.stop();
  }
  // CONDITION: Count > 0
  int bid = itsOldHead.getValue() - 1;

  return itsMetadataBuffer[bid].timestamp;
}

timestamp_t BufferController::setStartOffset()
{

  mutex::scoped_lock sl(buffer_mutex);

  // calculate offset
  timestamp_t newestStamp;
  newestStamp.setStamp(getNewestStamp(sl).getSeqId() + 1, 0);
  
  timestamp_t oldestStamp = getOldestStamp(sl);
  int offset = newestStamp - oldestStamp;

  // wait until enough data becomes available
  bool amWaiting = false;
  while (getCount() - offset <= 0)
  {
    if (!amWaiting) {
      itsWaitingForDataTimer.start();
      amWaiting = true;
    }
    data_available.wait(sl);
  }
  if (amWaiting) {
    itsWaitingForDataTimer.stop();
  }

  // This method is called when there is no reader,
  // so tail == oldTail
  itsTail += offset;
  itsOldTail = itsTail;
  int bid = itsTail.getValue();

  space_available.notify_all();
  
  return itsMetadataBuffer[bid].timestamp;
}

void BufferController::setStartOffset(timestamp_t startstamp)
{

  mutex::scoped_lock sl(buffer_mutex);

  // calculate offset
  timestamp_t oldestStamp = getOldestStamp(sl);
  int offset = startstamp - oldestStamp;

  // wait until enough data becomes available
  bool amWaiting = false;
  while (getCount() - offset <= 0)
  {
    if (!amWaiting) {
      itsWaitingForDataTimer.start();
      amWaiting = true;
    }
    data_available.wait(sl);
  }
  if (amWaiting) {
    itsWaitingForDataTimer.stop();
  }
  
  // This method is called when there is no reader,
  // so tail == oldTail
  itsTail += offset;
  itsOldTail = itsTail;

  space_available.notify_all();
}

int BufferController::setReadOffset(timestamp_t startstamp)
{
  mutex::scoped_lock sl(buffer_mutex);
  
  // calculate offset
  timestamp_t oldestStamp = getOldestStamp(sl);
  int offset = startstamp - oldestStamp;

  // check offset
  ASSERTSTR(std::abs(offset) < itsBufferSize , 
	    "BufferController: timestamp offset invalid (startstamp: " << startstamp << "   oldestStamp: " << oldestStamp <<"   newestStamp: " << getNewestStamp(sl) << "   count: " << getCount() << ")");

  // wait until enough data becomes available
  bool amWaiting = false;
  while (getCount() - offset < itsMinCount)
  {
    if (!amWaiting) {
      itsWaitingForDataTimer.start();
      amWaiting = true;
    }
    data_available.wait(sl);
  }
  if (amWaiting) {
    itsWaitingForDataTimer.stop();
  }
  
  // This method is called when there is no reader,
  // so tail == oldTail
  itsTail += offset;
  itsOldTail = itsTail;
  int bid = itsTail.getValue();
   
  return bid;
}

int BufferController::setRewriteOffset(timestamp_t startstamp)
{
  mutex::scoped_lock sl(buffer_mutex);

  // calculate offset
  timestamp_t oldestStamp = getOldestStamp(sl);
  int offset = startstamp - oldestStamp;

  // check if there are enough elements in buffer
  if ((offset >= getCount()) || (offset < 0)) {
    return -1;
  } 
  // CONDITION: offset + nelements < Count
   
  itsOldHead = itsTail + offset;

  return itsOldHead.getValue();
}

int BufferController::writeLockRange(int nelements)
{
  int bid;

  mutex::scoped_lock sl(buffer_mutex);
  
  // wait until space becomes available
  int amWaiting = false;
  while ((itsHead - itsOldTail + nelements >= itsMaxCount) && !itsOverwritingAllowed)
  {
    if (!amWaiting) {
      itsWaitingForSpaceTimer.start();
      amWaiting = true;
    }
    space_available.wait(sl);
  }
  if (amWaiting) {
    itsWaitingForSpaceTimer.stop();
  }
  
  // CONDITION: Count < itsMaxCount 
  bid = itsHead.getValue();
  itsHead+=nelements;

  // if allowed, overwrite previous written elements
  if (((itsHead - itsTail) < nelements) && itsOverwritingAllowed) {
    // push tail forwards
    itsTail+= itsHead-itsTail;
    itsOldTail+= itsHead-itsTail;
  }

  return bid;
}

int BufferController::readLockRange(int nelements)
{
  int bid;

  mutex::scoped_lock sl(buffer_mutex);

  // overwriting not allowed when reading is started
  itsOverwritingAllowed = false;
  
  // wait until enough elements are available
  bool amWaiting = false;
  while (getCount() - nelements < itsMinCount) 
  {
    if (!amWaiting) {
      itsReadLockTimer.stop();
      itsWaitingForDataTimer.start();
      amWaiting = true;
    }
    data_available.wait(sl);
  }
  if (amWaiting) {
    itsWaitingForDataTimer.stop();
    itsReadLockTimer.start();
  }
 
  // CONDITION: Count >= itsMinCount 
  bid = itsTail.getValue();
  itsTail+=nelements;
  
  return bid;
}

void BufferController::releaseWriteBlock()
{
  mutex::scoped_lock sl(buffer_mutex);
  
  // synchronize writepointers
  itsOldHead = itsHead;

  // signal that data has become available 
  data_available.notify_all();
}

void BufferController::releaseReadBlock()
{
  mutex::scoped_lock sl(buffer_mutex);
  
  // synchronize readpointers
  itsOldTail = itsTail;
  
  // signal that space has become available
  space_available.notify_all();
}

void BufferController::releaseRewriteBlock()
{
  mutex::scoped_lock sl(buffer_mutex);
  
  // synchronize writepointers
  itsOldHead = itsHead;

  // signal that data has become available 
  data_available.notify_all();
}

timestamp_t BufferController::startBufferRead()
{
  // start reading so overwriting not allowed anymore 
  itsOverwritingAllowed = false;

  // set offset and return
  return setStartOffset();
}

void BufferController::startBufferRead(timestamp_t startstamp)
{
  // start reading so overwriting not allowed anymore 
  itsOverwritingAllowed = false;
  
  // set offset
  setStartOffset(startstamp);
}

void BufferController::getElements(vector<SubbandType*> buf, int& invalidcount, timestamp_t startstamp, int nelements)
{
  // set offset, get startindex
  int sid = setReadOffset(startstamp);

  // get metadata for requested block
  invalidcount = 0;
  int bid;
  itsReadLockTimer.start();
  bid = readLockRange(nelements);
  itsReadLockTimer.stop();

  itsReadTimer.start();
  CyclicCounter id(itsBufferSize);
  id = bid;
  for (int m=bid; m<nelements; m++, id++) {
    if (itsMetadataBuffer[id.getValue()].invalid != 0) {
      invalidcount++;
    }
  }
  
  // get subbands for requested block
  int n1, n2;
  for (int s=0; s<itsNSubbands; s++) {
    if (sid + nelements  > itsBufferSize) {
      // do copy in 2 blocks because end of subband data array will be crossed 
      n1 = itsBufferSize - sid;
      memmove(buf[s], &(itsSubbandBuffer[s][sid]), n1*sizeof(SubbandType));

      n2 = nelements - n1;
      memmove(&(buf[s][n1]), &(itsSubbandBuffer[s][0]), n2*sizeof(SubbandType));
    }
    else {
      // copy can be executed in one block
      memmove(buf[s], &(itsSubbandBuffer[s][sid]), nelements*sizeof(SubbandType));
    }   
  }
  itsReadTimer.stop();
 
  // reading is done, free block
  itsReadUnlockTimer.start();
  releaseReadBlock();
  itsReadUnlockTimer.stop();
}

void BufferController::writeElements(SubbandType* buf, timestamp_t rspstamp)
{
  itsWriteLockTimer.start();
  // write the metadata
  int bid;
  bid = writeLockRange(1);
  itsWriteLockTimer.stop();
  itsMetadataBuffer[bid].invalid = 0;
  itsMetadataBuffer[bid].timestamp = rspstamp;
  
  
  itsWriteTimer.start();
  // write the subbanddata
  for (int s=0; s<itsNSubbands; s++) {
    itsSubbandBuffer[s][bid] = buf[s];
  }
  itsWriteTimer.stop();
 
  // writing is done, free block
  itsWriteUnlockTimer.start();
  releaseWriteBlock();
  itsWriteUnlockTimer.stop();
}
void BufferController::writeElements(SubbandType* buf, timestamp_t startstamp, int nelements, int stride)
{
  int bid, sid;
  itsWriteLockTimer.start();
  // write the metadata for this dummy block
  bid = writeLockRange(nelements);
  itsWriteLockTimer.stop();
  CyclicCounter id(itsBufferSize);
  id = bid;
  for (int i=0; i<nelements; i++, id++) {
    itsMetadataBuffer[id.getValue()].invalid = 0;
    itsMetadataBuffer[id.getValue()].timestamp = startstamp;
    startstamp++;
  }

  itsWriteTimer.start();
  // write the subbanddata
  for (int e=0; e<nelements; e++) {
    for (int s=0; s<itsNSubbands; s++) {
      itsSubbandBuffer[s][bid+e] = buf[s+e*stride];
    }
  }
  itsWriteTimer.stop();
 
  // writing is done, free block
  itsWriteUnlockTimer.start();
  releaseWriteBlock();
  itsWriteUnlockTimer.stop();
}

void BufferController::writeDummy(SubbandType* dum, timestamp_t startstamp, int nelements)
{
  int bid, sid;

  bid = writeLockRange(nelements);

  CyclicCounter id(itsBufferSize);
  id = bid;
  // write the metadata for this dummy block
  for (int i=0; i<nelements; i++, id++) {
    itsMetadataBuffer[id.getValue()].invalid = 1;
    itsMetadataBuffer[id.getValue()].timestamp = startstamp;
    startstamp++;
  }

  // write the subbanddata
  for (int s=0; s<itsNSubbands; s++) {
    if (bid + nelements  > itsBufferSize) {
      // do copy in 2 blocks because end of subband data array will be crossed 
      int n1 = itsBufferSize - bid;
      memmove(&(itsSubbandBuffer[s][bid]), dum, n1*sizeof(SubbandType));

      int n2 = nelements - n1;
      memmove(&(itsSubbandBuffer[s][0]), &dum[n1], n2*sizeof(SubbandType));
    }
    else {
      // copy can be executed in one block
      memmove(&(itsSubbandBuffer[s][bid]), dum, nelements*sizeof(SubbandType));
    }
  }

  releaseWriteBlock();
}

bool BufferController::rewriteElements(SubbandType* buf, timestamp_t startstamp)
{
  // set offset, get startindex
  int bid = setRewriteOffset(startstamp);

  if (bid == -1) {
    // element not available
    return false;

  } else {
 
    // rewrite the metadata 
    itsMetadataBuffer[bid].invalid = 0;
    
    // rewrite the subbanddata
    for (int s=0; s<itsNSubbands; s++) {
      itsSubbandBuffer[s][bid] = buf[s];
    }

    // rewriting is done, free block
    releaseRewriteBlock();
    
    return true;
  } 
}

void BufferController::clear()
{
  mutex::scoped_lock sl(buffer_mutex);
  itsTail = itsOldHead;
  itsOldTail = itsTail;
  space_available.notify_all();
}


}
