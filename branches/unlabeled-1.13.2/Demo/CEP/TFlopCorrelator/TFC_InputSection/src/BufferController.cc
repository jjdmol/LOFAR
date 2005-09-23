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

BufferController::BufferController(int buffersize, int nsubbands)
  : itsBufferSize(buffersize),
    itsNSubbands(nsubbands),
    itsHead(buffersize),
    itsTail(buffersize),
    itsOldHead(buffersize),
    itsOldTail(buffersize),
    itsOverwritingAllowed(true)
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
  while (getCount() <= 0) 
  {
    data_available.wait(sl);
  }
  // CONDITION: Count > 0
  int bid = itsTail.getValue();

  return itsMetadataBuffer[bid].timestamp;
}

timestamp_t BufferController::getNewestStamp(mutex::scoped_lock& sl)
{
  // wait until at least one element is available
  while (getCount() <= 0) 
  {
    data_available.wait(sl);
  }
  // CONDITION: Count > 0
  int bid = itsOldHead.getValue() - 1;

  return itsMetadataBuffer[bid].timestamp;
}

timestamp_t BufferController::setStartOffset()
{

  mutex::scoped_lock sl(buffer_mutex);

  // calculate offset
  timestamp_t newestStamp = getNewestStamp(sl);
  timestamp_t oldestStamp = getOldestStamp(sl);
  int offset = newestStamp - oldestStamp;

  // wait until enough data becomes available
  while (getCount() - offset <= 0)
  {
    data_available.wait(sl);
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
  while (getCount() - offset <= 0)
  {
    data_available.wait(sl);
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
  ASSERTSTR(std::abs(offset) <= MAX_OFFSET , 
  	       "BufferController: timestamp offset invalid");

  // wait until enough data becomes available
  while (getCount() - offset < MIN_COUNT)
  {
    data_available.wait(sl);
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
  if (offset >= getCount()) {
    return -1;
  } 
  // CONDITION: offset + nelements < Count
   
  itsOldHead = itsTail + offset;

  return itsOldHead.getValue();
}

int BufferController::getWritePtr()
{
  int bid;

  mutex::scoped_lock sl(buffer_mutex);
  
  // wait until space becomes available
  while ((itsHead - itsOldTail >= MAX_COUNT) && !itsOverwritingAllowed)
  {
    space_available.wait(sl);
  }
  
  // CONDITION: Count < MAX_COUNT 
  bid = itsHead.getValue();
  itsHead++;

  // if allowed, overwrite previous written elements
  if (itsHead == itsTail && itsOverwritingAllowed) {
    // push tail forwards
    itsTail++;
    itsOldTail++;
  }

  return bid;
}

int BufferController::getReadPtr()
{
  int bid;

  mutex::scoped_lock sl(buffer_mutex);

  // overwriting not allowed when reading is started
  itsOverwritingAllowed = false;
  
  // wait until enough elements are available
  while (getCount() < MIN_COUNT) 
  {
    data_available.wait(sl);
  }
  
  // CONDITION: Count >= MIN_COUNT 
  bid = itsTail.getValue();
  itsTail++;
  
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
  for (int m=0; m<nelements; m++) {
    bid = getReadPtr(); 
    if (itsMetadataBuffer[bid].invalid != 0) {
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
 
  // reading is done, free block
  releaseReadBlock();
}

void BufferController::writeElements(SubbandType* buf, timestamp_t rspstamp)
{
  // write the metadata
  int bid;
  bid = getWritePtr();
  itsMetadataBuffer[bid].invalid = 0;
  itsMetadataBuffer[bid].timestamp = rspstamp;
  
  
  // write the subbanddata
  for (int s=0; s<itsNSubbands; s++) {
    itsSubbandBuffer[s][bid] = buf[s];
  }
 
  // writing is done, free block
  releaseWriteBlock();
}

void BufferController::writeDummy(SubbandType* dum, timestamp_t startstamp, int nelements)
{
  int bid, sid;

  // write the metadata for this dummy block
  for (int i=0; i<nelements; i++) {
    bid = getWritePtr();
    itsMetadataBuffer[bid].invalid = 1;
    itsMetadataBuffer[bid].timestamp = startstamp;
    if (i==0) {
      sid = bid;
    }
    startstamp++;
  }

  // write the subbanddata
  for (int s=0; s<itsNSubbands; s++) {
    if (sid + nelements  > itsBufferSize) {
      // do copy in 2 blocks because end of subband data array will be crossed 
      int n1 = itsBufferSize - sid;
      memmove(&(itsSubbandBuffer[s][sid]), dum, n1*sizeof(SubbandType));

      int n2 = nelements - n1;
      memmove(&(itsSubbandBuffer[s][0]), &dum[n1], n2*sizeof(SubbandType));
    }
    else {
      // copy can be executed in one block
      memmove(&(itsSubbandBuffer[s][sid]), dum, nelements*sizeof(SubbandType));
    }
  }

  releaseWriteBlock();
}

bool BufferController::rewriteElements(SubbandType* buf, timestamp_t startstamp)
{
  // set offset, get startindex
  int bid = setRewriteOffset(startstamp);

  if (bid != -1) {
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


}
