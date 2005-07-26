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


#include <Transport/Lock.h>
#include <Common/lofar_vector.h>

namespace LOFAR
{

// an item of the CyclicBuffer
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

  // read a block beginning at oldest item + offset and adjust tail and count
  const TYPE GetBlockReadPtr(int offset, int Nelements, int& ID);

  // read oldest item without adjusting tail and count 
  const TYPE GetFirstReadPtr(int& ID);

  // (over)write an item without adjusting head and count
  TYPE GetManualWritePtr(int ID);
  
  // release locks
  void WriteUnlockItem(int ID);
  void ReadUnlockItems(int ID, int NElements);

  // print head, tail and count pointers
  void Dump();

  // maximum number of items in buffer
  int  getSize();
  
  // actual number of written items in buffer
  int  getCount(); 
  
  /* Use setWrittenBeforeReading(uint x) to force that cyclic buffer must 
     contain at least x% of its maximum items before reading from buffer 
     is possible (x=0 means that reading is possible if buffer contains 
     at least 1 item */
  void setWrittenBeforeReading(uint percentage);

 private:

  void ReadLockItems(int ID, int NElements);
  void WriteLockItem(int ID);
  
  vector< BufferItem <TYPE> > itsBuffer;

  int itsHeadIdx;
  int itsTailIdx;
  int itsCount;

  int itsMinCount;

  pthread_mutex_t buffer_mutex;
  pthread_cond_t  data_available;
  pthread_cond_t  space_available;
};

template<class TYPE>
CyclicBuffer<TYPE>::CyclicBuffer() :
    itsHeadIdx(0),
    itsTailIdx(0),
    itsCount(0),
    itsMinCount(0)
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
  return (itsBuffer.size()-1);
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
  while (itsCount >= (int)itsBuffer.size())
  {
    pthread_cond_wait(&space_available, &buffer_mutex);
  }

  // CONDITION: itsCount < itsBuffer.size()
  // space available for at least one element
  
  ID = itsHeadIdx;
  itsBuffer[ID].itsRWLock.WriteLock();
  
  // adjust the head (points to first free position)
  itsHeadIdx++;
  if (itsHeadIdx >= (int)itsBuffer.size())
  {
    itsHeadIdx = 0;
  }
  itsCount++;

  // signal that data has become available 
  if (itsMinCount+1 == itsCount)
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
  while ((itsCount <= itsMinCount)) 
  {
    pthread_cond_wait(&data_available, &buffer_mutex);
  }
  // CONDITION: itsCount > itsMinCount
  
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
const TYPE CyclicBuffer<TYPE>::GetBlockReadPtr(int offset, int Nelements, int& ID)
{
  pthread_mutex_lock(&buffer_mutex);

  // wait until enough elements are available
  while ((itsCount <= itsMinCount) || (itsCount<=offset)) 
  {
    pthread_cond_wait(&data_available, &buffer_mutex);
  }
  // CONDITION: itsCount > itsMinCount &&  itscount > offset
  
  // set offset
  itsTailIdx += offset;
  ID = itsTailIdx;
  
  // lock the elements
  ReadLockItems(ID, Nelements); 

  for (int i=0;i<Nelements;i++) {
    itsBuffer[ID+i].itsRWLock.ReadLock();
 }
 
  // adjust the tail
  itsTailIdx += Nelements;
  if (itsTailIdx >= (int)itsBuffer.size())
  {
    itsTailIdx = itsTailIdx-(int)itsBuffer.size();
  }
  itsCount = itsCount-offset-Nelements;
  
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
  ReadLockItems(ID, 1);
 
  pthread_mutex_unlock(&buffer_mutex);
  
  return itsBuffer[ID].itsItem;
}


template<class TYPE>
TYPE CyclicBuffer<TYPE>::GetManualWritePtr(int ID)
{
  if ( (ID >= (int)itsBuffer.size()) || (ID < 0))
  {
   LOG_TRACE_RTTI("CyclicBuffer::getWriteUserItem: ID has invalid value");
   return 0;
 } 

  pthread_mutex_lock(&buffer_mutex); 

  itsBuffer[ID].itsRWLock.WriteLock();

  pthread_mutex_unlock(&buffer_mutex);

  return itsBuffer[ID].itsItem;
}

template<class TYPE>
void CyclicBuffer<TYPE>::WriteUnlockItem(int ID)
{
  itsBuffer[ID].itsRWLock.WriteUnlock();
}

template<class TYPE>
void CyclicBuffer<TYPE>::ReadUnlockItems(int ID, int Nelements)
{
  for (int i=0; i<Nelements; i++) {
    itsBuffer[ID+i].itsRWLock.ReadUnlock();
  }
}

template<class TYPE>
void CyclicBuffer<TYPE>::ReadLockItems(int ID, int Nelements)
{
  for (int i=0; i<Nelements; i++) {
    itsBuffer[ID+i].itsRWLock.ReadLock();
  }
}

template<class TYPE>
void CyclicBuffer<TYPE>::WriteLockItem(int ID)
{
  itsBuffer[ID].itsRWLock.WriteLock();
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
  return itsBuffer.size();
}

template<class TYPE>
int CyclicBuffer<TYPE>::getCount(void)
{
  return itsCount;
}

template<class TYPE>
void CyclicBuffer<TYPE>::setWrittenBeforeReading(uint percentage)
{
  itsMinCount =  ((float)percentage/100) * itsBuffer.size();
}


}

#endif
