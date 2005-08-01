//# CyclicBuffer.h: Cyclic buffer interface.
//#
//# Copyright (C) 2000, 2001
//# ASTRON (Netherlands Foundation for Research in Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//# This program is free software; you can redistribute it and/or modify
//# it under the terms of the GNU General Public License as published by
//# the Free Software Foundation; either version 2 of the License, or
//# (at your option) any later version.
//#
//# This program is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License
//# along with this program; if not, write to the Free Software
//# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#
//# $Id$

#ifndef TFLOPCORRELATOR_CYCLICBUFFER_H
#define TFLOPCORRELATOR_CYCLIC_BUFFER_H

#include <CEPFrame/Lock.h>
#include <Common/lofar_vector.h>

// minumum written elements before reading is allowed
#define MIN_COUNT   1 

// maximum of written elements in buffer to avoid that 
// writeptr is catching up readptr
#define MAX_COUNT  (itsBuffer.size()-5)    

namespace LOFAR
{

// Item of the CyclicBuffer
template <class TYPE>
class BufferItem
{
 public:
  BufferItem(TYPE DataItem) :
    itsItem(DataItem)
    {
    }

    // Control single write multiple read access
    ThreadRWLock itsRWLock;
  
    // Pointer to the TYPE object.
    TYPE itsItem;

 private:
    
    // size of the TYPE
    int   itsSize;
    
    // Unique ID of this BufferElement.
    int itsID;
};


template <class TYPE>
class CyclicBuffer
{
 public:

  CyclicBuffer();
  ~CyclicBuffer();
  
  // add item into buffer
  int   AddBufferItem(TYPE DataItem);
  
  // clear item from buffer
  void  RemoveBufferItem(int ID);
  
  // clear the buffer
  void  RemoveItems();

  // write an item and adjust head and count
  TYPE GetAutoWritePtr(int& ID);

  // read oldest item and adjust tail and count
  const TYPE GetAutoReadPtr(int& ID);

  // write a block and adjust tail and count
  const TYPE GetBlockWritePtr(int nelements, int& ID);

  // read a block beginning at oldest item + offset and adjust tail and count
  const TYPE GetBlockReadPtr(int offset, int nelements, int& ID);

  // read oldest item without adjusting tail and count 
  const TYPE GetFirstReadPtr(int& ID);

  // (over)write an item without adjusting head and count
  TYPE GetManualWritePtr(int ID);
  
  // release locks
  void WriteUnlockElements(int ID, int nelements);
  void ReadUnlockElements(int ID, int nelements);

  // print head, tail and count pointers
  void Dump();

  // maximum number of items in buffer
  int  getSize();
  
  // actual number of written items in buffer
  int  getCount(); 

 private:

  void ReadLockElements(int ID, int nelements);
  void WriteLockElements(int ID, int nelements);
  
  vector< BufferItem <TYPE> > itsBuffer;

  int itsHeadIdx;
  int itsTailIdx;
  int itsCount;

  pthread_mutex_t buffer_mutex;
  pthread_cond_t  data_available;
  pthread_cond_t  space_available;
};

template<class TYPE>
CyclicBuffer<TYPE>::CyclicBuffer() :
    itsHeadIdx(0),
    itsTailIdx(0),
    itsCount(0)
{
  pthread_mutex_init(&buffer_mutex, NULL);
  pthread_cond_init (&data_available,  NULL);
  pthread_cond_init (&space_available, NULL);
}

template<class TYPE>
CyclicBuffer<TYPE>::~CyclicBuffer()
{
  itsBuffer.clear();
}

template<class TYPE>
int CyclicBuffer<TYPE>::AddBufferItem(TYPE DataItem)
{
  BufferItem<TYPE> elem(DataItem);

  pthread_mutex_lock(&buffer_mutex);

  // add the element at the back of the deque
  itsBuffer.push_back(elem);

  pthread_mutex_unlock(&buffer_mutex);

  // the ID will be the index in the deque
  return ((int)itsBuffer.size()-1);
}

template<class TYPE>
void CyclicBuffer<TYPE>::RemoveBufferItem(int ID)
{
  // lock the buffer for the duration of this routine
  pthread_mutex_lock(&buffer_mutex);

  // erase the element at the specified position
  itsBuffer.erase(ID);

  pthread_mutex_unlock(&buffer_mutex);
}

template<class TYPE>
void CyclicBuffer<TYPE>::RemoveItems(void)
{
  // lock the buffer for the duration of this routine
  pthread_mutex_lock(&buffer_mutex);

  // clear the buffer
  itsBuffer.clear();

  pthread_mutex_unlock(&buffer_mutex);
}

template<class TYPE>
TYPE CyclicBuffer<TYPE>::GetAutoWritePtr(int& ID)
{

  pthread_mutex_lock(&buffer_mutex);
  
  // wait until space becomes available
  while (itsCount >= MAX_COUNT)
  {
    pthread_cond_wait(&space_available, &buffer_mutex);
  }

  // CONDITION: itsCount < MAX_COUNT
  ID = itsHeadIdx;
  WriteLockElements(ID, 1);
  
  // adjust the head (points to first free position)
  itsHeadIdx++;
  if (itsHeadIdx >= (int)itsBuffer.size())
  {
    itsHeadIdx = 0;
  }
  itsCount++;

  // signal that data has become available 
  if (itsCount >= MIN_COUNT)
  {
    pthread_cond_broadcast(&data_available);
  }
  pthread_mutex_unlock(&buffer_mutex);

  return itsBuffer[ID].itsItem;
}

template<class TYPE>
const TYPE CyclicBuffer<TYPE>::GetAutoReadPtr(int& ID)
{
  pthread_mutex_lock(&buffer_mutex);

  // wait until enough elements are available
  while ((itsCount < MIN_COUNT)) 
  {
    pthread_cond_wait(&data_available, &buffer_mutex);
  }
  
  // CONDITION: itsCount >= MIN_COUNT
  ID = itsTailIdx;
  ReadLockElements(ID, 1);
 
  // adjust the tail
  itsTailIdx++;
  if (itsTailIdx >= (int)itsBuffer.size())
  {
    itsTailIdx = 0;
  }
  itsCount--;
  
  // signal that space has become available
  pthread_cond_broadcast(&space_available);

  pthread_mutex_unlock(&buffer_mutex);
  
  return itsBuffer[ID].itsItem;
}

template<class TYPE>
const TYPE CyclicBuffer<TYPE>::GetBlockWritePtr(int nelements, int& ID)
{
  pthread_mutex_lock(&buffer_mutex);
  
  // wait until enough space becomes available
  while (itsCount + nelements > MAX_COUNT)
  {
    pthread_cond_wait(&space_available, &buffer_mutex);
  }

  // CONDITION: itsCount + nelements <= MAX_COUNT
  ID = itsHeadIdx;
  
  // lock the elements
  WriteLockElements(ID, nelements);

  // adjust the head (point to first free position)
  itsHeadIdx += nelements;
  if (itsHeadIdx >= (int)itsBuffer.size())
  {
    itsHeadIdx -= (int)itsBuffer.size();;
  }
  itsCount += nelements;

  // signal that data has become available 
  if (itsCount >= MIN_COUNT)
  {
    pthread_cond_broadcast(&data_available);
  }
  pthread_mutex_unlock(&buffer_mutex);

  return itsBuffer[ID].itsItem;
}


template<class TYPE>
const TYPE CyclicBuffer<TYPE>::GetBlockReadPtr(int offset, int nelements, int& ID)
{
  pthread_mutex_lock(&buffer_mutex);

  int minelements;
  if (offset >= 0) {
    minelements = offset + nelements;
  } else {
    minelements = nelements;
  }
    
  // wait until enough elements are available
  while (itsCount < MIN_COUNT || itsCount < minelements) 
  {
     pthread_cond_wait(&data_available, &buffer_mutex);
  }
  // CONDITION: itsCount >= MIN_COUNT &&  itscount >= minelements
  
  // set offset (note: offset can be less than 0
  ID = itsTailIdx + offset;
  if (ID >= (int)itsBuffer.size()) {
    ID -= (int)itsBuffer.size();
  }
  if (ID < 0) {
    ID += (int)itsBuffer.size();
  }

  // lock the elements
  ReadLockElements(ID, nelements); 
  
  // adjust the tail
  itsTailIdx += nelements;
  if (itsTailIdx >= (int)itsBuffer.size())
  {
    itsTailIdx -= (int)itsBuffer.size();
  }
  itsCount -= nelements;
  
  // signal that space has become available
  pthread_cond_broadcast(&space_available);

  pthread_mutex_unlock(&buffer_mutex);
  
  return itsBuffer[ID].itsItem;
}

template<class TYPE>
const TYPE CyclicBuffer<TYPE>::GetFirstReadPtr(int& ID)
{
  pthread_mutex_lock(&buffer_mutex);

  // wait until at least one element is available
  while (itsCount <= 0) 
  {
    pthread_cond_wait(&data_available, &buffer_mutex);
  }
  // CONDITION: itsCount > 0
  
  ID = itsTailIdx;
  ReadLockElements(ID, 1);
 
  pthread_mutex_unlock(&buffer_mutex);
  
  return itsBuffer[ID].itsItem;
}

template<class TYPE>
TYPE CyclicBuffer<TYPE>::GetManualWritePtr(int ID)
{
  if ( (ID >= itsBuffer.size()) || (ID < 0))
  {
   LOG_TRACE_RTTI("CyclicBuffer::getWriteUserItem: ID has invalid value");
   return 0;
  } 

  pthread_mutex_lock(&buffer_mutex); 

  // lock the elements
  WriteLockElements(ID, 1);

  pthread_mutex_unlock(&buffer_mutex);

  return itsBuffer[ID].itsItem;
}

template<class TYPE>
void CyclicBuffer<TYPE>::WriteLockElements(int ID, int nelements)
{
  int idx = ID;
  for (int i=0; i<nelements; i++) {
    if (idx >= (int)itsBuffer.size()) {
      idx = 0;
    }
    itsBuffer[idx].itsRWLock.WriteLock();
    idx++;
  }
}

template<class TYPE>
void CyclicBuffer<TYPE>::WriteUnlockElements(int ID, int nelements)
{
  int idx = ID;
  for (int i=0; i<nelements; i++) {
    if (idx >= (int)itsBuffer.size()) {
      idx = 0;
    }
    itsBuffer[idx].itsRWLock.WriteUnlock();
    idx++;
  }
}

template<class TYPE>
void CyclicBuffer<TYPE>::ReadLockElements(int ID, int nelements)
{
  int idx = ID;
  for (int i=0; i<nelements; i++) {
    if (idx >= (int)itsBuffer.size()) {
      idx = 0;
    }
    itsBuffer[idx].itsRWLock.ReadLock();
    idx++;
  }
}

template<class TYPE>
void CyclicBuffer<TYPE>::ReadUnlockElements(int ID, int nelements)
{
  int idx = ID;
  for (int i=0; i<nelements; i++) {
    if (idx >= (int)itsBuffer.size()) {
      idx = 0;
    }
    itsBuffer[idx].itsRWLock.ReadUnlock();
    idx++;
  }
}

template<class TYPE>
void CyclicBuffer<TYPE>::Dump(void)
{
  pthread_mutex_lock(&buffer_mutex);
  
  cerr << "itsHeadIdx = " << itsHeadIdx << endl;
  cerr << "itsTailIdx = " << itsTailIdx << endl;
  cerr << "itsCount   = " << itsCount   << endl;

  pthread_mutex_unlock(&buffer_mutex);
}

template<class TYPE>
int CyclicBuffer<TYPE>::getSize(void)
{
  return (int)itsBuffer.size();
}

template<class TYPE>
int CyclicBuffer<TYPE>::getCount(void)
{
  return itsCount;
}



}

#endif
